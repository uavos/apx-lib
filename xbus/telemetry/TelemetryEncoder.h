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

typedef void (*enc_codec_f)();

#pragma pack(1)
typedef struct
{
    mandala::type_id_e type : 4; // raw value format
    uint8_t _rsv : 3;            // reserved
    bool upd : 1;                // raw value updated
} enc_flags_s;
#pragma pack()

typedef struct
{
    field_s fields[slots_size];
    uint8_t crc_cobs;
    enc_flags_s flags[slots_size];
    mandala::raw_t value[slots_size];  // raw value
    mandala::raw_t packed[slots_size]; // packed value
} enc_slots_s;
// static_assert(sizeof(enc_slots_s) == 12 * slots_size + 1, "size error");

} // namespace telemetry
} // namespace xbus

class TelemetryEncoder
{
public:
    explicit TelemetryEncoder();

    bool add(const xbus::telemetry::field_s &field);
    void clear();

    bool encode(XbusStreamWriter &stream, uint32_t seq, xbus::telemetry::dt_e dt);
    void encode_format(XbusStreamWriter &stream, uint8_t part);

    void update(const xbus::pid_s &pid, const mandala::spec_s &spec, XbusStreamReader &stream);
    void update(const xbus::pid_s &pid, mandala::raw_t raw, mandala::type_id_e type_id);

    xbus::telemetry::enc_slots_s &enc_slots() { return _slots; }
    inline uint16_t slots_cnt() const { return _slots_cnt; }
    inline uint16_t slots_upd_cnt() const { return _slots_upd_cnt; }

    void reset_feeds();

private:
    xbus::telemetry::enc_slots_s _slots;
    uint16_t _slots_cnt;
    uint16_t _slots_upd_cnt; // re-scheduled slots top limit (before bitfields)

    void _insert(size_t index, const xbus::telemetry::field_s &field);

    xbus::telemetry::hash_s _hash;

    void _set_data(size_t n, const mandala::spec_s &spec, XbusStreamReader &stream);
    void _set_data(size_t n, mandala::raw_t raw, mandala::type_id_e type_id);

    void _update_feeds();

    // fmt feed COBS encoder
    uint8_t _get_feed_fmt();
    uint16_t _fmt_pos;
    uint16_t _fmt_size;

    uint8_t _cobs_code;
    uint8_t _cobs_copy;

    void encode_values(XbusStreamWriter &stream, uint32_t seq);
};
