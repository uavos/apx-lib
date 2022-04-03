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
#include <mandala/MandalaMetaBase.h>

namespace xbus {
namespace telemetry {

#pragma pack(1)
typedef struct
{                                // TODO telemetry stream vars auto requests
    mandala::type_id_e type : 2; // raw value format
    bool upd : 1;                // raw value updated since transmission
    bool aux : 1;                // aux slot from config
    uint8_t to : 4;              // value update timeout counter
} enc_flags_s;
#pragma pack()

typedef struct
{
    field_s fields[slots_size];
    enc_flags_s flags[slots_size];
    mandala::raw_t value[slots_size];  // raw value, as received
    mandala::raw_t packed[slots_size]; // packed value to monitor changes
} enc_slots_s;
static_assert(sizeof(enc_slots_s) == 12 * slots_size, "size error");

} // namespace telemetry
} // namespace xbus

class TelemetryEncoder
{
public:
    explicit TelemetryEncoder();

    bool add(const xbus::telemetry::field_s &field);
    void clear();

    bool encode(XbusStreamWriter &stream, uint8_t pseq, uint64_t timestamp_ms);
    void encode_format(XbusStreamWriter &stream, uint8_t part, const uint8_t size = xbus::telemetry::fmt_block_size);

    enum result_e {
        ok,
        skipped,
        synced,
        sync_ovf
    };

    result_e update(const xbus::pid_s &pid, mandala::raw_t raw, mandala::type_id_e type_id, bool sync = false);

    inline auto &enc_slots() { return _slots; }
    inline auto slots_cnt() const { return _slots_cnt; }
    inline auto slots_upd_cnt() const { return _slots_upd_cnt; }
    ssize_t slot_lookup(mandala::uid_t uid);

    inline auto sync_cnt() const { return _sync_cnt; }
    void sync_flush();

private:
    xbus::telemetry::enc_slots_s _slots;
    uint16_t _slots_cnt;
    uint16_t _slots_upd_cnt; // re-scheduled slots top limit (before bitfields)

    uint16_t _enforced_upd{}; // re-scheduling index counter

    uint16_t _sync_cnt{};
    ssize_t _inserted_index;

    void _insert(size_t index, const xbus::telemetry::field_s &field);

    xbus::telemetry::hash_s _hash;

    void _set_data(size_t n, mandala::raw_t raw, mandala::type_id_e type_id);

    void _update_feeds();

    void encode_values(XbusStreamWriter &stream, uint8_t pseq);
};
