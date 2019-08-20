#pragma once
#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

namespace xbus {
namespace node {
namespace blackbox {

// Node blackbox read-write

typedef enum { bbr_hdr = 0, bbr_data } commands_t;

struct Header
{
    uint32_t magic;
    uint32_t uptime;
    uint32_t oncnt;
    std::array<uint32_t, 3> evtcnt;
    uint32_t rec_size; //blocks
    std::array<uint8_t, 32 - 20 - 4> padding;
    uint32_t crc;

    static inline uint16_t psize()
    {
        return sizeof(uint32_t) * 5 + sizeof(evtcnt) + sizeof(padding);
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> magic;
        *s >> uptime;
        *s >> oncnt;
        *s >> evtcnt;
        *s >> rec_size;
        *s >> padding;
        *s >> crc;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << magic;
        *s << uptime;
        *s << oncnt;
        *s << evtcnt;
        *s << rec_size;
        *s << padding;
        *s << crc;
    }
};

} // namespace blackbox
} // namespace node
} // namespace xbus
