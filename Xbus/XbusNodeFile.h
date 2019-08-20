#pragma once
#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

namespace xbus {
namespace node {
namespace file {

// Node file read-write
enum {
    size_block = 256,
};

typedef struct
{
    uint32_t start_address;
    uint32_t size;   // available size in bytes
    uint8_t xor_crc; // all file data XORed

    static inline uint16_t psize() { return sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint8_t); }
    inline void read(XbusStreamReader *s)
    {
        *s >> start_address;
        *s >> size;
        *s >> xor_crc;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << start_address;
        *s << size;
        *s << xor_crc;
    }
} file_t;

typedef struct
{
    uint32_t start_address;
    uint16_t data_size;

    static inline uint16_t psize() { return sizeof(uint32_t) + sizeof(uint16_t); }
    inline void read(XbusStreamReader *s)
    {
        *s >> start_address;
        *s >> data_size;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << start_address;
        *s << data_size;
    }
} file_data_hdr_t;

} // namespace file
} // namespace node
} // namespace xbus
