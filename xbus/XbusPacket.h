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

#include <common/visibility.h>
#include <sys/types.h>

#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

namespace xbus {

typedef uint16_t pid_raw_t;

constexpr const size_t size_packet_max = 512;

enum pri_e {
    pri_final = 0,
    pri_primary,
    pri_secondary,
    pri_failsafe,
    pri_auxilary,

    // special cases

    // nmt
    pri_response = 6, // response not request
    pri_request = 7,  // request not response

    // redundancy
    pri_any = 7,    // telemetry value accepts any priority
    pri_select = 7, // sub selects priority automatically
};

constexpr const char *pri_text(uint8_t pri)
{
    switch (pri) {
    default:
        break;
    case pri_final:
        return "local";
    case pri_primary:
        return "primary";
    case pri_secondary:
        return "secondary";
    case pri_failsafe:
        return "failsafe";
    case pri_auxilary:
        return "auxilary";
    }
    return "";
}

// Packet identifier [16 bits]
#pragma pack(1)
union pid_s {
    pid_raw_t _raw;

    struct
    {
        uint16_t uid : 11; // dictionary

        pri_e pri : 3;   // [0,7] sub index, 1=request not response
        uint8_t seq : 2; // sequence counter
    };

    explicit pid_s()
        : _raw(0)
    {}

    constexpr inline pid_raw_t raw()
    {
        return _raw;
    }

    // _seq!=0 indicates valid uid (_raw!=0)
    constexpr explicit pid_s(uint16_t _uid, pri_e _pri, uint8_t _seq)
        : uid(_uid)
        , pri(_pri)
        , seq(_seq)
    {}

    static constexpr inline uint16_t psize()
    {
        return sizeof(pid_raw_t);
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> _raw;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << _raw;
    }

    constexpr pid_s(const pid_raw_t &v)
        : _raw(v)
    {}

    constexpr pid_s &operator=(const pid_raw_t &v)
    {
        _raw = v;
        return *this;
    }
    inline constexpr bool match(const pid_s &v) const { return uid == v.uid; }
};
static_assert(sizeof(pid_s) == sizeof(pid_raw_t), "pid_s size error");
#pragma pack()

} // namespace xbus
