#pragma once
#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

namespace xbus {
namespace node {
namespace blackbox {

// Node blackbox read-write

typedef enum { bbr_hdr = 0,
               bbr_data } commands_t;

struct Header
{
    uint32_t magic;
    uint32_t uptime;
    uint32_t oncnt;
    uint32_t evtcnt[3];
    uint32_t rec_size;   //blocks
    uint32_t block_size; //bytes
    uint8_t padding[32 - 20 - 4 - 4];
    uint32_t crc;

    static inline uint16_t psize()
    {
        return sizeof(uint32_t) * 6 + sizeof(evtcnt) + sizeof(padding);
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> magic;
        *s >> uptime;
        *s >> oncnt;
        s->read(evtcnt, sizeof(evtcnt));
        *s >> rec_size;
        *s >> block_size;
        s->read(padding, sizeof(padding));
        *s >> crc;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << magic;
        *s << uptime;
        *s << oncnt;
        s->write(evtcnt, sizeof(evtcnt));
        *s << rec_size;
        *s << block_size;
        s->write(padding, sizeof(padding));
        *s << crc;
    }
};

} // namespace blackbox
} // namespace node
} // namespace xbus
