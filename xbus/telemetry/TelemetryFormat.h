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

#include <XbusPacket.h>
#include <XbusStreamReader.h>
#include <XbusStreamWriter.h>

#include <cmath>

#include <MandalaMetaTree.h>
#include <MandalaPack.h>

namespace xbus {
namespace telemetry {

// constants
static constexpr const uint8_t fmt_version = 2;    // increase this number for GCS to be incompatible with AP
static constexpr const size_t fmt_block_size = 64; // encoder will reply blocks (size bytes) of the fields array with this default
static constexpr const size_t slots_size{240};     // max number of fields in the stream (<255)

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

// Transponder data
namespace xpdr {

static constexpr const uint8_t version = 1; // dataset version (hdr.feed_hash)

struct field_s
{
    constexpr field_s(mandala::uid_t uid, mandala::fmt_e fmt)
        : uid{uid}
        , fmt{fmt}
    {}
    constexpr field_s(mandala::uid_t uid)
        : uid{uid}
        , fmt{mandala::fmt(uid).fmt}
    {}

    mandala::uid_t uid;
    mandala::fmt_e fmt : 8;
};

static constexpr const field_s dataset[] = {
    {mandala::est::nav::pos::lat::uid},
    {mandala::est::nav::pos::lon::uid},
    {mandala::est::nav::pos::hmsl::uid},
    {mandala::est::nav::pos::speed::uid},
    {mandala::est::nav::pos::bearing::uid, mandala::fmt_s8_rad},
    {mandala::est::nav::pos::vspeed::uid, mandala::fmt_s8},
    {mandala::cmd::nav::proc::mode::uid},
};
static constexpr const size_t dataset_size = sizeof(dataset) / sizeof(dataset[0]);
} // namespace xpdr

} // namespace telemetry
} // namespace xbus
