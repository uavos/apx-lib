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

#include <cinttypes>
#include <sys/types.h>

#include <xbus/XbusPacket.h>

namespace mandala {

typedef uint16_t uid_t;

typedef uint8_t byte_t;
typedef uint16_t word_t;
typedef uint32_t dword_t;
typedef float real_t;

typedef uint32_t raw_t;

enum type_id_e {
    type_byte,
    type_word,
    type_dword,
    type_real,
};

constexpr const char *type_string(type_id_e type)
{
    switch (type) {
    case type_byte:
        return "byte";
    case type_word:
        return "word";
    case type_dword:
        return "dword";
    case type_real:
        return "real";
    }
    return "unknown";
}

template<typename T>
T read(type_id_e type, XbusStreamReader &stream)
{
    switch (type) {
    default:
        return T();
    case type_byte:
        return stream.read<byte_t, T>();
    case type_word:
        return stream.read<word_t, T>();
    case type_dword:
        return stream.read<dword_t, T>();
    case type_real:
        return stream.read<real_t, T>();
    }
}

template<typename T>
void write(const T &v, type_id_e type, XbusStreamWriter &stream)
{
    switch (type) {
    case type_byte:
        return stream.write<byte_t, T>(v);
    case type_word:
        return stream.write<word_t, T>(v);
    case type_dword:
        return stream.write<dword_t, T>(v);
    case type_real:
        return stream.write<real_t, T>(v);
    }
    return stream.write<T>(v);
}

constexpr type_id_e type_id(const byte_t &)
{
    return type_byte;
}
constexpr type_id_e type_id(const word_t &)
{
    return type_word;
}
constexpr type_id_e type_id(const dword_t &)
{
    return type_dword;
}
constexpr type_id_e type_id(const real_t &)
{
    return type_real;
}

constexpr size_t type_size(type_id_e type)
{
    switch (type) {
    case type_byte:
        return sizeof(byte_t);
    case type_word:
        return sizeof(word_t);
    case type_dword:
        return sizeof(dword_t);
    case type_real:
        return sizeof(real_t);
    }
    return 0;
}

template<typename T>
static constexpr inline raw_t to_raw(const T &v)
{
    union raw_u {
        raw_u()
            : raw(0)
        {}
        raw_t raw;
        T v;
    };
    raw_u u;
    u.v = v;
    return u.raw;
}
template<typename T>
static constexpr inline T from_raw(raw_t r)
{
    union raw_u {
        raw_u(raw_t r)
            : raw(r)
        {}
        raw_t raw;
        T v;
    };
    raw_u u(r);
    return u.v;
}
template<typename T>
static constexpr inline T from_raw(raw_t r, type_id_e type_id)
{
    switch (type_id) {
    case type_byte:
        return (T) from_raw<byte_t>(r);
    case type_word:
        return (T) from_raw<word_t>(r);
    case type_dword:
        return (T) from_raw<dword_t>(r);
    case type_real:
        return (T) from_raw<real_t>(r);
    }
    return T();
}

// Data Specifier [1 byte] - first byte after pid for mandala data transfers

#pragma pack(1)
union spec_s {
    uint8_t _raw;

    struct
    {
        type_id_e type : 4; // data format
        uint8_t _rsv : 4;   //
    };

    explicit spec_s()
        : _raw(0)
    {}

    static constexpr inline uint16_t psize() { return sizeof(uint8_t); }
    inline void read(XbusStreamReader *s) { *s >> _raw; }
    inline void write(XbusStreamWriter *s) const { *s << _raw; }
};
static_assert(sizeof(spec_s) == spec_s::psize() && sizeof(spec_s) == sizeof(spec_s::_raw),
              "spec_s size error");
#pragma pack()

struct meta_s
{
    const char *name;
    const char *title;
    const char *descr;
    const char *units;
    const char *path;

    const uid_t uid : sizeof(uid_t) * 8;
    const uid_t mask : sizeof(uid_t) * 8;
    const uint8_t level : 3;
    const bool group : 1;
    const type_id_e type_id : 4;

    const uint8_t units_id;

    // telemetry stream format options
    const uint8_t fmt;
    const uint8_t seq;
};

}; // namespace mandala
