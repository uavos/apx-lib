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
    frm_seq_max = 4,
    frm_end = 5,
    frm_start = 6,
    frm_single = 7,
};

typedef uint32_t extid_t;

// EXTID format [29 bits]
#pragma pack(1)
union extid_s {
    uint32_t raw;

    struct // 29 bits
    {
        uint8_t frm : 3;   // frame id
        uint16_t pid : 16; // packet identifier
        uint8_t src : 7;   // source node address
        uint8_t net : 3;   // [0,7] network id
    };
};
#pragma pack()

static constexpr const extid_t mf_id_mask = ((1 << 29) - 1) & (~((1 << 3) - 1));

} // namespace can
} // namespace xbus
