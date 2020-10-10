#pragma once
#include "XbusNode.h"
#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

namespace xbus {
namespace script {

typedef char title_t[16];

static constexpr const size_t max_file_size{65535};

struct file_hdr_s
{
    uint32_t code_size;
    uint32_t src_size;

    title_t title;

    static inline uint16_t psize()
    {
        return sizeof(uint32_t) + sizeof(xbus::node::hash_t) + sizeof(title_t) + sizeof(uint32_t);
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> code_size;
        *s >> src_size;
        s->read(title, sizeof(title));
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << code_size;
        *s << src_size;
        s->write(title, sizeof(title));
    }
};

} // namespace script
} // namespace xbus
