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
#include "XbusPacket.h"

#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

namespace xbus {
namespace unit {

typedef uint16_t squawk_t;

// Unit IDENT
typedef uint8_t uid_t[12]; //global unique id

struct ident_s
{
    union {
        uint32_t _raw;
        struct
        {
            bool gcs : 1; // set if Ground Control
        };
    } flags;

    static inline uint16_t psize() { return sizeof(flags._raw); }
    inline void read(XbusStreamReader *s) { *s >> flags._raw; }
    inline void write(XbusStreamWriter *s) const { *s << flags._raw; }

    // strings: callsign
};

} // namespace unit
} // namespace xbus
