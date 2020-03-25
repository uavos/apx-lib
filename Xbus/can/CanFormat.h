/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#pragma once

#include <common/visibility.h>
#include <sys/types.h>

#include <Xbus/XbusPacket.h>

namespace xbus {
namespace can {

enum frm_e : uint8_t {
    frm_single,
    frm_seq0,
    frm_seq1,
    frm_end
};

typedef uint32_t extid_t;

// EXTID format [29 bits]
#pragma pack(1)
union extid_s {
    uint32_t raw;

    struct // 29 bits
    {
        uint8_t frm : 2;   // 0 = single, 1,2 = toggle seq, 3 = end transfer
        uint16_t pid : 16; // packet identifier
        uint8_t src : 7;   // source node address
        uint8_t net : 4;   // [0,15] network id
    };
};
#pragma pack()

static constexpr const extid_t mf_id_mask = ((1 << 29) - 1) & (~((1 << 2) - 1));

} // namespace can
} // namespace xbus
