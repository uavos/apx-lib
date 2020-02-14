/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#pragma once

#include <cstdint>

namespace xbus {

typedef struct
{
    union {
        uint32_t raw;
        struct
        {
            uint32_t id : 27;
            uint32_t unused0 : 4;
            uint32_t std : 1; // 1= standard ID (11 bit)
        };
    };
} CanID;

}; // namespace xbus
