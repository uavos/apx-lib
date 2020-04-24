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
    fmt_bit, // one bit
    fmt_opt, // 4 bits

    //packed real numbers
    fmt_f16,       // float16
    fmt_sbyte,     // signed byte
    fmt_sbyte_10,  // signed/10
    fmt_sbyte_01,  // signed*10
    fmt_sbyte_001, // signed*100
    fmt_byte_10,   // unsigned/10
    fmt_byte_01,   // unsigned*10
    fmt_byte_001,  // unsigned*100
    fmt_rad,       // radians -PI..+PI
    fmt_rad2,      // radians -PI/2..+PI/2
    fmt_byte_u,    // units 0..1
    fmt_sbyte_u,   // signed units -1..+1
};

typedef struct
{
    xbus::pid_s pid; // seq = pid seq skip mask
    fmt_e fmt : 8;
    //uint8_t _rsv : 4;
} field_s;
static_assert(sizeof(field_s) == 3, "size error");

typedef union {
    uint32_t hash;
    uint8_t byte[4];
} hash_s;
static_assert(sizeof(hash_s) == 4, "size error");

static constexpr const size_t fmt_block_size = 64;

// stream header
typedef struct
{
    uint16_t ts;       // timestamp [100's of ms]
    uint8_t feed_hash; // dataset structure hash_32 (pid.seq = byte no)
    uint8_t feed_fmt;  // dataset format COBS encoded feed

    static constexpr inline uint16_t psize()
    {
        return 4;
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> ts;
        *s >> feed_hash;
        *s >> feed_fmt;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << ts;
        *s << feed_hash;
        *s << feed_fmt;
    }
} stream_s;
static_assert(sizeof(stream_s) == stream_s::psize(), "size error");

#pragma pack()

} // namespace telemetry
} // namespace xbus
