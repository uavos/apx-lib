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
namespace vehicle {

typedef uint16_t squawk_t;

// Vehicle IDENT
typedef uint8_t uid_t[12]; //global unique vehicle id

struct ident_s
{
    union {
        uint32_t _raw;
        struct
        {
            bool gcs : 1; // set if Ground Control
        };
    } flags;

    static inline uint16_t psize()
    {
        return sizeof(flags._raw);
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> flags._raw;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << flags._raw;
    }

    // strings: callsign
};

// Vehicle transponder data
typedef float lat_t;
typedef float lon_t;
typedef int16_t alt_t;
typedef uint16_t speed_t;
typedef int16_t course_t;
typedef uint8_t mode_t;

struct xpdr_s
{
    lat_t lat;
    lon_t lon;
    float alt;
    float speed;
    float course;
    mode_t mode;

    static inline uint16_t psize()
    {
        return sizeof(lat_t)
               + sizeof(lon_t)
               + sizeof(alt_t)
               + sizeof(speed_t)
               + sizeof(course_t)
               + sizeof(mode_t);
    }

    inline void read(XbusStreamReader *s)
    {
        *s >> lat;
        *s >> lon;
        alt = s->read<alt_t, float>();
        speed = s->read<speed_t, float>() / 100.0f;
        course = s->read<course_t, float>() / (32768.0f / 180.0f);
        *s >> mode;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << lat;
        *s << lon;
        s->write<alt_t, float>(alt);
        s->write<speed_t, float>(speed * 100.0f);
        s->write<course_t, float>(course * (32768.0f / 180.0f));
        *s << mode;
    }
};

} // namespace vehicle
} // namespace xbus
