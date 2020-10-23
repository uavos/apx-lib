/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#pragma once

#include <xbus/XbusPacket.h>
#include <common/visibility.h>
#include <sys/types.h>

#define XCAN_SRC_SHIFT 20 //source address
#define XCAN_SRC_MASK (255 << XCAN_SRC_SHIFT)
#define XCAN_SRC(n) (((n) << XCAN_SRC_SHIFT) & XCAN_SRC_MASK)

#define XCAN_PID_SHIFT 12 //packet id
#define XCAN_PID_MASK (255 << XCAN_PID_SHIFT)
#define XCAN_PID(n) (((n) << XCAN_PID_SHIFT) & XCAN_PID_MASK)

#define XCAN_NAD_SHIFT 11 //node addressing marker
#define XCAN_NAD_MASK (1 << XCAN_NAD_SHIFT)

#define XCAN_END_SHIFT 10 //end marker
#define XCAN_END_MASK (1 << XCAN_END_SHIFT)

#define XCAN_CNT_SHIFT 0 //multipart counter (256*8=2048 bytes max)
#define XCAN_CNT_MASK (255 << XCAN_CNT_SHIFT)
#define XCAN_CNT(n) (((n) << XCAN_CNT_SHIFT) & XCAN_CNT_MASK)

#define XCAN_PRI_SHIFT 28 //priority bit (lowest)
#define XCAN_PRI_MASK (1 << XCAN_PRI_SHIFT)

namespace xbus {
namespace can {

// EXTID format [29 bits]
begin_packed_struct struct extid_s
{
    union {
        uint32_t raw;

        struct
        {
            uint32_t frm : 2;     // 0 = single, 1,2 = toggle seq, 3 = end transfer
            pid_s pid;            // 16 bits packet identifier
            uint32_t adr : 7;     // source node address
            uint32_t network : 4; // [0,15] network id
        };
    };
} end_packed_struct;

}; // namespace can
}; // namespace xbus
