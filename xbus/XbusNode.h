/*
 * APX Autopilot project <http://docs.uavos.com>
 *
 * Copyright (c) 2003-2020, Aliaksei Stratsilatau <sa@uavos.com>
 * All rights reserved
 *
 * This file is part of APX Shared Libraries.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include "XbusPacket.h"
#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

namespace xbus {
namespace node {

typedef uint8_t guid_t[12]; //global unique node id

typedef uint32_t hash_t;

//---------------------------
// ident
//---------------------------
namespace ident {

typedef struct
{
    uint32_t format; // protocol format
    hash_t hash;     // structure hash
    union {
        struct
        {
            uint32_t files : 4;  //number of files
            uint32_t busy : 1;   //set to postpone gcs requests
            uint32_t reconf : 1; //set if conf was reset
        } bits;
        uint32_t raw;
    } flags;

    // strings: name, version, hardware, [files]

    static constexpr inline uint16_t psize()
    {
        return sizeof(uint32_t) * 2 + sizeof(hash_t);
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> format;
        *s >> hash;
        *s >> flags.raw;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << format;
        *s << hash;
        *s << flags.raw;
    }
} ident_s;

}; // namespace ident

//---------------------------
// reboot
//---------------------------
namespace reboot {

enum type_e : uint8_t {
    firmware = 0x5A,
    loader = 0xA5,
};

}; // namespace reboot

//---------------------------
// file
//---------------------------
namespace file {

enum op_e : uint8_t {
    idle,
    info,   // re: <info_s>
    ropen,  // re: <info_s>
    wopen,  // re: <info_s>
    close,  // re: <info_s>
    read,   // q: <offset> re: <offset><data>
    write,  // q: <offset><data> re: <offset><size><hash>
    abort,  // re: <info_s>
    extend, // re: <offset><timeout>
};
// all op requests prepended with <name> string

static constexpr const uint8_t reply_op_mask = 0x80;

typedef uint8_t fd_t;
typedef uint32_t offset_t;
typedef uint32_t size_t;
typedef uint32_t time_t;
typedef uint16_t timeout_t;

typedef struct
{
    size_t size;     // file size
    time_t time;     // modified time
    hash_t hash;     // contents hash
    offset_t offset; // start offset when available

    union {
        struct
        {
            uint32_t readable : 1; // readable
            uint32_t writable : 1; // writable
            uint32_t oread : 1;    // opened for read
            uint32_t owrite : 1;   // opened for write
        } bits;
        uint32_t raw;
    } flags;

    static constexpr inline uint16_t psize()
    {
        return sizeof(size) + sizeof(time) + sizeof(hash) + sizeof(offset) + sizeof(flags.raw);
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> size;
        *s >> time;
        *s >> hash;
        *s >> offset;
        *s >> flags.raw;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << size;
        *s << time;
        *s << hash;
        *s << offset;
        *s << flags.raw;
    }
} info_s;

}; // namespace file

//---------------------------
// msg
//---------------------------
namespace msg {

enum type_e : uint8_t {
    info,
    warn,
    err,
};

}; // namespace msg

//---------------------------
// dict
//---------------------------
namespace conf {

typedef uint32_t fid_t; // [idx:24][array idx:8]

typedef uint8_t option_t;
typedef float real_t;
typedef uint8_t byte_t;
typedef uint16_t word_t;
typedef uint32_t dword_t;
typedef char string_t[16];
typedef char text_t[64];
typedef xbus::pid_raw_t bind_t;
typedef xbus::node::hash_t script_t;

static constexpr const uint8_t type_field{4};

enum type_e : uint8_t {
    group = 0,
    command,
    option = type_field,
    real,
    byte,
    word,
    dword,
    bind,
    string,
    text,
    script,

    type_max
};

// field descriptor packed into dict array
#pragma pack(1)
typedef struct
{
    type_e type;
    uint8_t array; // >0 if array
    uint8_t group; // group idx

    // field strings: name, units
    // group strings: name, title
    // command strings: name, title

} field_s;
#pragma pack()
static_assert(sizeof(field_s) == 3, "size error");

constexpr const char *type_to_str(type_e t)
{
    switch (t) {
    case group:
        return "group";
    case command:
        return "command";
    case option:
        return "option";
    case real:
        return "real";
    case byte:
        return "byte";
    case word:
        return "word";
    case dword:
        return "dword";
    case bind:
        return "bind";
    case string:
        return "string";
    case text:
        return "text";
    case script:
        return "script";

    case type_max:
        break;
    }

    return "";
}

// dict array content:
// <hash><fields array>
}; // namespace conf

//---------------------------
// mod - modules tree
//---------------------------
namespace mod {

enum op_e : uint8_t {
    sh,     // shell command, report to terminal
    ls,     // re: <strings> tree
    status, // re: <status_s>
};

}; // namespace mod

//---------------------------
// status
//---------------------------
namespace status {

typedef struct
{
    struct
    {
        uint8_t can;
        uint8_t uart;
        uint8_t hw;
        uint8_t _rsv[5];
    } err;
    struct
    {
        uint8_t rx;
        uint8_t tx;
        uint8_t _rsv[6];
    } cnt;

    static constexpr inline uint16_t psize()
    {
        return sizeof(uint8_t) * 4;
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> err.can;
        *s >> err.uart;
        *s >> err.hw;
        s->read(err._rsv, sizeof(err._rsv));
        *s >> cnt.rx;
        *s >> cnt.tx;
        s->read(cnt._rsv, sizeof(cnt._rsv));
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << err.can;
        *s << err.uart;
        *s << err.hw;
        s->write(err._rsv, sizeof(err._rsv));
        *s << cnt.rx;
        *s << cnt.tx;
        s->write(cnt._rsv, sizeof(cnt._rsv));
    }

} status_s;

}; // namespace status

//---------------------------
// usr
//---------------------------
namespace usr {

typedef uint8_t cmd_t;

}; // namespace usr

} // namespace node
} // namespace xbus
