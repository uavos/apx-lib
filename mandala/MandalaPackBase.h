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

#include <mandala/MandalaConstants.h>

namespace mandala {

// telemetry stream element format descriptor
enum fmt_e { // 5 bits
    fmt_none,

    // raw
    fmt_u8,
    fmt_u16,
    fmt_u32,
    fmt_f32,

    // packed bitfields
    fmt_bit, // one bit
    fmt_u4,  // 4 bits

    // packed real numbers (13)
    fmt_f16,      // float16
    fmt_s8,       // signed byte
    fmt_s8_10,    // signed/10
    fmt_s8_01,    // signed*10
    fmt_s8_001,   // signed*100
    fmt_u8_10,    // unsigned/10
    fmt_u8_01,    // unsigned*10
    fmt_u8_001,   // unsigned*100
    fmt_s16_rad,  // radians -PI..+PI
    fmt_s16_rad2, // radians -PI/2..+PI/2
    fmt_u8_u,     // units 0..1
    fmt_s8_u,     // signed units -1..+1
    fmt_u16_10,   // [rpm] unsigned/10
};

// values are send on change only
// to limit bandwidth of frequently changed values use seq param
enum seq_e {      // 2 bits
    seq_always,   // transmit every frame on change
    seq_skip,     // transmit every second frame
    seq_rare,     // transmit every fourth frame
    seq_scheduled // transmit in scheduled slots (rare)
};

// datasets for telemetry stream
enum dataset_e {
    ds_aux,     // only transmit when selected in aux config
    ds_minimal, // everything displayed in GCS instruments
    ds_normal,  // normal op: some extra data for investigations
    ds_heavy,   // experimental flights extra data
    ds_auto,    // not in a dataset, but can be added automatically to the stream
};

constexpr const char *fmt_string(fmt_e fmt)
{
    switch (fmt) {
    case fmt_none:
        return "none";

    case fmt_u8:
        return "u8";
    case fmt_u16:
        return "u16";
    case fmt_u32:
        return "u32";
    case fmt_f32:
        return "f32";

    case fmt_bit:
        return "bit";
    case fmt_u4:
        return "u4";

    case fmt_f16:
        return "f16";
    case fmt_s8:
        return "s8";
    case fmt_s8_10:
        return "s8_10";
    case fmt_s8_01:
        return "s8_01";
    case fmt_s8_001:
        return "s8_001";
    case fmt_u8_10:
        return "u8_10";
    case fmt_u8_01:
        return "u8_01";
    case fmt_u8_001:
        return "u8_001";
    case fmt_s16_rad:
        return "s16_rad";
    case fmt_s16_rad2:
        return "s16_rad2";
    case fmt_u8_u:
        return "u8_u";
    case fmt_s8_u:
        return "s8_u";
    case fmt_u16_10:
        return "u16_10";
    }
    return "unknown";
}

constexpr fmt_e default_fmt(type_id_e type, units_e units)
{
    switch (units) {
    case units_none:
        break;

    case units_bit:
        return fmt_bit;

    case units_opt:
        return fmt_u4;

    case units_A:
        return fmt_f16;

    case units_Ah:
        return fmt_u8;

    case units_bar:
        return fmt_u8_01;

    case units_C:
        return fmt_s8;

    case units_kg:
        return fmt_f16;

    case units_rad:
        return fmt_s16_rad;

    case units_radps:
        return fmt_s8_001;

    case units_radps_sq:
        return fmt_s8_001;

    case units_K:
        return fmt_f16;

    case units_kgpm_cb:
        return fmt_f16;

    case units_kPa:
        return fmt_f16;

    case units_LpD:
        return fmt_f16;

    case units_m:
        return fmt_f16;

    case units_km:
        break;

    case units_mbar:
        return fmt_f16;

    case units_mps:
        return fmt_f16;

    case units_mps_sq:
        return fmt_s8;

    case units_ms:
        break;

    case units_N:
        return fmt_f16;

    case units_Nm:
        return fmt_u8;

    case units_Pa:
        return fmt_f16;

    case units_percent:
        return fmt_u8;

    case units_percentph:
        return fmt_u8;

    case units_rad_sq:
        return fmt_u8_01;

    case units_rpm:
        return fmt_u16_10;

    case units_rpmps:
        return fmt_u16;

    case units_s:
        break;

    case units_min:
        break;

    case units_su:
        return fmt_s8_u;

    case units_u:
        return fmt_u8_u;

    case units_V:
        return fmt_f16;

    case units_gps:
        break;
    }

    switch (type) {
    case type_byte:
        return fmt_u8;

    case type_word:
        return fmt_u16;

    case type_dword:
        return fmt_u32;

    case type_real:
        return fmt_f16;
    }

    return fmt_none;
}

constexpr seq_e default_seq(type_id_e type, units_e units)
{
    (void) type;
    switch (units) {
    default:
        break;

    case units_C:
        return seq_rare;
    }

    return seq_always;
}

// hard coded uint32 data element format descriptior
#pragma pack(1)
union fmt_s {
    struct
    {
        uid_t uid;
        type_id_e type_id : 2;
        dataset_e ds : 3;
        uint8_t _rsv5 : 3;
        fmt_e fmt : 6;
        seq_e seq : 2;
    };

    uint32_t _raw;
};
static_assert(sizeof(fmt_s) == sizeof(fmt_s::_raw), "fmt_s size error");
#pragma pack()

constexpr const fmt_s &fmt_lookup(uid_t uid, const fmt_s *list, ssize_t size)
{
    // binary search
    ssize_t l = 0, r = size - 1;
    while (l <= r) {
        ssize_t m = l + (r - l) / 2;
        const auto &fmt = list[m];
        auto vuid = fmt.uid;
        if (vuid == uid)
            return fmt;
        if (vuid < uid)
            l = m + 1;
        else
            r = m - 1;
    }
    return *list; // not found - return zero uid dmmy element
}

}; // namespace mandala
