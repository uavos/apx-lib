#pragma once

#include <Xbus/XbusPacket.h>
#include <Xbus/XbusStreamReader.h>
#include <Xbus/XbusStreamWriter.h>

namespace xbus {
namespace telemetry {

#pragma pack(1)

// data element format descriptor
enum fmt_e {
    fmt_none,

    //raw
    fmt_real,
    fmt_dword,
    fmt_word,
    fmt_byte,

    //packed bitfields
    fmt_bit,  // one bit
    fmt_bit4, // nibble

    //packed real numbers
    fmt_f16,       // float16
    fmt_sbyte,     // signed byte
    fmt_sbyte_10,  // signed/10
    fmt_sbyte_01,  // signed*10
    fmt_sbyte_001, // signed*100
    fmt_byte_10,   // unsigned/10
    fmt_byte_01,   // unsigned*10
    fmt_byte_001,  // unsigned*100
};

enum class cap_e {
    bit,
    byte,
    word,
    dword,
};

enum class span_e {
    real,
    uint,

    sint, // float from signed integer
    f16,  // float 16 bits
    rad,  // radians -PI..+PI
    rad2, // radians -PI/2..+PI/2
    u,    // units 0..1
    su,   // signed units -1..+1
    u10,  // 10's
    u01,  // 0.1's
    u001, // 0.01's
};

typedef struct
{
    xbus::pid_s pid; // seq = pid seq skip mask
    fmt_e fmt : 4;
    uint8_t _rsv : 4;
} field_s;
static_assert(sizeof(field_s) == 3, "size error");

typedef union {
    uint32_t hash;
    uint8_t byte[4];
} hash_s;
static_assert(sizeof(hash_s) == 4, "size error");

enum rate_e {
    rate_10Hz,
    rate_5Hz,
    rate_1Hz,
    rate_ts, // timestamp_32 follows
};

// stream header
typedef struct
{
    union {
        uint8_t _raw;
        struct
        {
            uint8_t seq : 6; // sequence MSB[2,7] (+pid.seq LSB[0,1])
            rate_e rate : 2;
        };
    } spec;
    uint8_t feed_hash; // dataset structure hash_32 (pid.seq = byte no)
    uint8_t feed_fmt;  // dataset format COBS encoded feed

    static constexpr inline uint16_t psize()
    {
        return 3;
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> spec._raw;
        *s >> feed_hash;
        *s >> feed_fmt;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << spec._raw;
        *s << feed_hash;
        *s << feed_fmt;
    }
} stream_s;
static_assert(sizeof(stream_s) == stream_s::psize(), "size error");

#pragma pack()

} // namespace telemetry
} // namespace xbus
