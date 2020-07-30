#pragma once

#include <cinttypes>
#include <sys/types.h>

#include <Xbus/XbusPacket.h>

namespace mandala {

typedef uint16_t uid_t;

typedef float real_t;
typedef uint32_t dword_t;
typedef uint16_t word_t;
typedef uint8_t byte_t;
typedef uint8_t option_t;

typedef uint32_t raw_t;

enum type_id_e {
    // integral formats [max 7]
    type_void,
    type_real,
    type_dword,
    type_word,
    type_byte,
    type_option,

    // bundle formats
    type_bundle = 10,
    type_vec2, // <x>,<y>
    type_vec3, // <x>,<y>,<z>
};

template<typename T>
T read(type_id_e type, XbusStreamReader &stream)
{
    switch (type) {
    default:
        return T();
    case type_real:
        return stream.read<real_t, T>();
    case type_dword:
        return stream.read<dword_t, T>();
    case type_word:
        return stream.read<word_t, T>();
    case type_byte:
        return stream.read<byte_t, T>();
    case type_option:
        return stream.read<option_t, T>();
    }
}

template<typename T>
void write(const T &v, type_id_e type, XbusStreamWriter &stream)
{
    switch (type) {
    default:
        return stream.write<T>(v);
    case type_real:
        return stream.write<real_t, T>(v);
    case type_dword:
        return stream.write<dword_t, T>(v);
    case type_word:
        return stream.write<word_t, T>(v);
    case type_byte:
        return stream.write<byte_t, T>(v);
    case type_option:
        return stream.write<option_t, T>(v);
    }
}

constexpr type_id_e type_id(const real_t &)
{
    return type_real;
}
constexpr type_id_e type_id(const dword_t &)
{
    return type_dword;
}
constexpr type_id_e type_id(const word_t &)
{
    return type_word;
}
constexpr type_id_e type_id(const byte_t &)
{
    return type_byte;
}

constexpr size_t type_size(type_id_e type)
{
    switch (type) {
    default:
        return 0;
    case type_real:
        return sizeof(real_t);
    case type_dword:
        return sizeof(dword_t);
    case type_word:
        return sizeof(word_t);
    case type_byte:
        return sizeof(byte_t);
    case type_option:
        return sizeof(option_t);
    }
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
    case type_real:
        return (T) from_raw<real_t>(r);
    case type_dword:
        return (T) from_raw<dword_t>(r);
    case type_word:
        return (T) from_raw<word_t>(r);
    case type_byte:
        return (T) from_raw<byte_t>(r);
    default:
        return T();
    }
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

    static constexpr inline uint16_t psize()
    {
        return sizeof(uint8_t);
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> _raw;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << _raw;
    }
};
static_assert(sizeof(spec_s) == spec_s::psize() && sizeof(spec_s) == sizeof(spec_s::_raw), "spec_s size error");
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
};

}; // namespace mandala
