/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#pragma once

#include <cstdint>

namespace xbus {

struct CanID
{
    union {
        uint32_t raw;
        struct
        {
            uint32_t id : 27;
            uint32_t std : 1; // 1= standard ID (11 bit)
            uint32_t dlc : 4; // data length code
        };
    };
} __attribute__((packed));

struct CanMsg
{
    CanID cid;
    uint8_t data[8];
} __attribute__((packed));

}; // namespace xbus
