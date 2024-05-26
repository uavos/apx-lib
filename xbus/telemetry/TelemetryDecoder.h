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

#include "TelemetryFormat.h"
#include <MandalaMetaBase.h>

namespace xbus {
namespace telemetry {

#pragma pack(1)
typedef struct
{
    mandala::type_id_e type : 4; // value format
    uint8_t _rsv : 3;            // reserved
    bool upd : 1;                // value updated
} dec_flags_s;
#pragma pack()

typedef struct
{
    field_s fields[slots_size];
    dec_flags_s flags[slots_size];
    mandala::raw_t value[slots_size]; // value
} dec_slots_s;
//static_assert(sizeof(dec_slot_s) == 8 * slots_size + 1, "size error");

typedef struct
{
    mandala::raw_t value[xpdr::dataset_size];
    mandala::type_id_e value_type[xpdr::dataset_size];
} dec_xpdr_s;

} // namespace telemetry
} // namespace xbus

class TelemetryDecoder
{
public:
    bool decode(const xbus::pid_s &pid, XbusStreamReader &stream);

    bool decode_format(XbusStreamReader &stream, xbus::telemetry::format_resp_hdr_s *hdr);

    inline bool valid() { return _valid; }
    void reset(bool reset_hash = true);

    // decoded stream current parameters
    inline auto timestamp_ms() const { return _timestamp_ms; }
    inline auto dt_ms() const { return _dt_ms; }
    inline void reset_timestamp(uint64_t v = 0) { _timestamp_ms = v; }

    // stream format
    inline xbus::telemetry::dec_slots_s &dec_slots() { return _slots; }
    inline uint16_t slots_cnt() { return _slots_cnt; }

    // XPDR
    inline auto &xpdr_slots() { return _xpdr_slots; }
    inline uint16_t xpdr_slots_cnt() { return xbus::telemetry::xpdr::dataset_size; }

protected:
    xbus::telemetry::dec_slots_s _slots{};
    uint16_t _slots_cnt{};

    xbus::telemetry::dec_xpdr_s _xpdr_slots{};

    uint64_t _timestamp_ms{};
    uint32_t _dt_ms{};

    uint16_t _ts{};

    bool _valid{};

    xbus::telemetry::hash_s _hash;
    uint8_t _hash_valid{};

    inline bool _is_hash_valid() const { return _hash_valid == 4; }
    uint32_t get_hash(size_t sz);
    bool check_hash(size_t sz);

    bool decode_values(XbusStreamReader &stream, uint8_t pseq);
    bool decode_xpdr(XbusStreamReader &stream);
};
