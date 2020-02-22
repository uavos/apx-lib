/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#pragma once

#include <cstdint>

namespace xbus {

struct CanMsg
{
    union {
        uint32_t raw;
        struct
        {
            uint32_t id : 27;
            uint32_t ext : 1; // 0= standard ID (11 bit), 1= extended (29 bit)
            uint32_t dlc : 4; // data length code
        };
    } hdr;
    uint8_t data[8];
} __attribute__((packed));

}; // namespace xbus
