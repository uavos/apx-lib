/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#pragma once

#include <common/visibility.h>
#include <sys/types.h>

#pragma pack(1)
struct xcan_msg_s
{
    uint32_t id;
    struct
    {
        uint8_t dlc : 4; // data length code
        uint8_t ext : 1; // 0= standard ID (11 bit), 1= extended (29 bit)
        uint8_t _rsv : 3;
    };
    uint8_t data[8];
};
#pragma pack()
