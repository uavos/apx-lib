#pragma once

#include <Xdb/Xdb.h>

namespace xdb {

struct iface_data_base_t
{
    const uint8_t *data;
    uint16_t size;

    union {
        uint32_t all;
        struct
        {
            uint8_t source : 8;  //source node id (if any)
            uint8_t iface : 8;   //iface index, 0=all, n - send to specific iface
            uint8_t request : 1; //data is received from iface
        };
    } flags;
};

struct iface_data_t : public iface_data_base_t
{
    uid_t pid;
};

XDB_DECLARE(iface_data);

} // namespace xdb
