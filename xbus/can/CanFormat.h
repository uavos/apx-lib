/*
 * APX Autopilot project <http://docs.uavos.com>
 *
 * Copyright (c) 2003-2020, Aliaksei Stratsilatau <sa@uavos.com>
 * All rights reserved
 *
 * This file is part of APX Shared Libraries.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <sys/types.h>
#include <visibility.h>

#include <xbus/XbusPacket.h>

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
