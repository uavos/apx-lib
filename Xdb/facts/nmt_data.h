#pragma once

#include "iface_data.h"

namespace xdb {

struct nmt_data_t
{
    const uint8_t *data;
    size_t size;
    uint16_t cmd;
};

XDB_DECLARE(nmt_data);

} // namespace xdb
