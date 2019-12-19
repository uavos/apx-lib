#pragma once

#include "iface_data.h"

namespace xdb {

struct nmt_data_t : public iface_data_base_t
{
    uint16_t cmd;
};

XDB_DECLARE(nmt_data);

} // namespace xdb
