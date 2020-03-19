#pragma once

#include "XbusPacket.h"
#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

namespace xbus {
namespace node {

typedef uint8_t guid_t[12]; //global unique node id

typedef uint16_t crc_t;
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

    static inline uint16_t psize()
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
    info,  // re: <info_s>
    ropen, // re: <info_s>
    wopen, // re: <info_s>
    close, // re: <info_s>
    read,  // q: <offset> re: <offset><data>
    write, // q: <offset><data> re: <offset><size><hash>
    abort, // re: <info_s>
};
// all op requests prepended with <name> string

static constexpr const uint8_t reply_op_mask = 0x80;

typedef uint8_t fd_t;
typedef uint32_t offset_t;
typedef uint32_t size_t;
typedef uint32_t time_t;

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

    static inline uint16_t psize()
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

typedef enum {
    info,
    warn,
    err,
} type_e;

typedef uint8_t type_t;

}; // namespace msg

//---------------------------
// dict
//---------------------------
namespace conf {

typedef uint16_t fid_t;

typedef uint8_t option_t;
typedef float real_t;
typedef uint8_t byte_t;
typedef uint16_t word_t;
typedef uint32_t dword_t;
typedef char string_t[16];
typedef char text_t[64];

enum type_e : uint8_t {
    group = 0,
    command,
    option = 4,
    real,
    byte,
    word,
    dword,
    string,
    text,
};

// field descriptor packed into dict array
typedef struct
{
    uint8_t type : 4;
    uint8_t array : 4; // >0 if array
    uint8_t group;     // group idx

    // field strings: name, descr, units
    // group strings: name, title, descr
    // command strings: name, title

} field_s;

static constexpr size_t size(type_e type)
{
    switch (type) {
    case group:
    case command:
        return 0;
    case option:
        return sizeof(option_t);
    case real:
        return sizeof(real_t);
    case byte:
        return sizeof(byte_t);
    case word:
        return sizeof(word_t);
    case dword:
        return sizeof(dword_t);
    case string:
        return sizeof(string_t);
    case text:
        return sizeof(text_t);
    }
}

// dict array content:
// <hash><fields array>

}; // namespace conf

} // namespace node
} // namespace xbus
