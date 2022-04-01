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

#include <xbus/XbusPacket.h>
#include <xbus/XbusStreamReader.h>
#include <xbus/XbusStreamWriter.h>

#include <cmath>

#include <mandala/MandalaMetaTree.h>

namespace xbus {
namespace telemetry {

// constants
static constexpr const uint8_t fmt_version = 2;    // increase this number for GCS to be incompatible with AP
static constexpr const size_t fmt_block_size = 64; // encoder will reply blocks (size bytes) of the fields array with this default
static constexpr const size_t slots_size{240};     // max number of fields in the stream

// telemetry field descriptor
#pragma pack(1)
struct field_s
{
    xbus::pid_s pid{}; // seq = seq_e skip mode
    mandala::fmt_e fmt : 8;
};
static_assert(sizeof(field_s) == 3, "size error");

union hash_s {
    uint32_t hash;
    uint8_t byte[4];
};
static_assert(sizeof(hash_s) == 4, "size error");
#pragma pack()

constexpr ssize_t field_lookup(mandala::uid_t uid, const field_s *list, ssize_t size)
{
    // binary search
    ssize_t l = 0, r = size - 1;
    while (l <= r) {
        size_t m = l + (r - l) / 2;
        const auto &f = list[m];
        auto v = f.pid.uid;
        if (v == uid)
            return m;
        if (v < uid)
            l = m + 1;
        else
            r = m - 1;
    }
    return -1;
}

// stream header
struct hdr_s
{
    uint16_t ts;       // [ms*20] 20 mins ovf
    uint8_t feed_hash; // dataset structure hash_32 (pid.seq = byte no) stream

    static constexpr const uint32_t ts2ms{20};

    static constexpr inline uint16_t psize() { return 3; }
    inline void read(XbusStreamReader *s)
    {
        *s >> ts;
        *s >> feed_hash;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << ts;
        *s << feed_hash;
    }
};

// format requests
struct format_req_s
{
    uint8_t version; // must match fmt_version constant
    uint8_t part;    // part requested

    static constexpr inline uint16_t psize() { return 2; }
    inline void read(XbusStreamReader *s)
    {
        *s >> version;
        *s >> part;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << version;
        *s << part;
    }
};

struct format_resp_hdr_s
{
    uint8_t version; // current format version
    uint8_t part;    // current part number [0..n]
    uint8_t pcnt;    // number of parts total
    uint8_t psz;     // part size [bytes]
    uint32_t hash;   // format data hash

    static constexpr inline uint16_t psize() { return 8; }
    inline void read(XbusStreamReader *s)
    {
        *s >> version;
        *s >> part;
        *s >> pcnt;
        *s >> psz;
        *s >> hash;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << version;
        *s << part;
        *s << pcnt;
        *s << psz;
        *s << hash;
    }
};

// Vehicle transponder data
struct xpdr_s
{
    static constexpr const uint8_t current_version = 1; // protocol version

    uint8_t version;
    float lat;
    float lon;
    float hmsl;
    float speed;
    float vspeed;
    float bearing;
    uint8_t mode;

    typedef int16_t hmsl_t;
    typedef uint16_t speed_t;
    typedef int16_t bearing_t;
    typedef int8_t vspeed_t;

    static inline uint16_t psize()
    {
        return 1 + sizeof(float) * 2 + sizeof(hmsl_t) + sizeof(speed_t) + sizeof(bearing_t)
               + 1 + 1;
    }

    inline void read(XbusStreamReader *s)
    {
        *s >> version;
        *s >> lat;
        *s >> lon;
        hmsl = s->read<hmsl_t, float>();
        speed = s->read<speed_t, float>() / 10.0f;
        vspeed = s->read<vspeed_t, float>() / 2.f;
        bearing = s->read<bearing_t, float>() / (32768.0f / M_PI);
        *s >> mode;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << version;
        *s << lat;
        *s << lon;
        s->write<hmsl_t, float>(hmsl);
        s->write<speed_t, float>(speed * 10.0f);
        s->write<vspeed_t, float>(vspeed * 2.f);
        s->write<bearing_t, float>(bearing * (32768.0f / M_PI));
        *s << mode;
    }
};

} // namespace telemetry
} // namespace xbus
