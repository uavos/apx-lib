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
#include "TelemetryEncoder.h"

#include "TelemetryValuePack.h"

#include <crc.h>
#include <cstdio>

using namespace xbus::telemetry;

TelemetryEncoder::TelemetryEncoder()
{
    clear();
}

bool TelemetryEncoder::add(const field_s &field)
{
    _inserted_index = -1;

    if (_slots_cnt >= slots_size)
        return false;

    auto uid = field.pid.uid;

    // find dups
    auto idx = xbus::telemetry::field_lookup(uid, _slots.fields, _slots_cnt);
    if (idx >= 0) {
        return true;
    }

    // find sorted index
    size_t index = 0;

    if (field.fmt == fmt_bit) { // bitfields at end
        while (index < _slots_cnt) {
            auto const &f = _slots.fields[index];
            if (f.fmt == fmt_bit)
                break;
            index++;
        }
        // sort by uid
        while (index < _slots_cnt) {
            auto const &f = _slots.fields[index];
            if (f.pid.uid > field.pid.uid)
                break;
            index++;
        }
    } else if (field.fmt == fmt_u4) { // opts at end before bitfields
        while (index < _slots_cnt) {
            auto const &f = _slots.fields[index];
            if (f.fmt == fmt_bit)
                break;
            if (f.fmt == fmt_u4)
                break;
            index++;
        }
        // sort by uid
        while (index < _slots_cnt) {
            auto const &f = _slots.fields[index];
            if (f.fmt == fmt_bit)
                break;
            if (f.pid.uid > field.pid.uid)
                break;
            index++;
        }
    } else {
        // sort by uid
        while (index < _slots_cnt) {
            auto const &f = _slots.fields[index];
            if (f.fmt == fmt_bit)
                break;
            if (f.fmt == fmt_u4)
                break;
            if (f.pid.uid > field.pid.uid)
                break;
            index++;
        }
    }

    _insert(index, field);
    _inserted_index = index;

    for (_slots_upd_cnt = 0; _slots_upd_cnt < _slots_cnt; ++_slots_upd_cnt) {
        auto const &f = _slots.fields[_slots_upd_cnt];
        if (f.fmt == fmt_bit)
            break;
    }

    return true;
}
void TelemetryEncoder::_insert(size_t index, const xbus::telemetry::field_s &field)
{
    if (index < _slots_cnt) {
        // shift upper half
        for (size_t src = _slots_cnt - 1, dest = _slots_cnt; dest > index; --src, --dest) {
            _slots.fields[dest] = _slots.fields[src];
            _slots.flags[dest] = _slots.flags[src];
            _slots.value[dest] = _slots.value[src];
            _slots.packed[dest] = _slots.packed[src];
        }
    }
    _slots.fields[index] = field;
    _slots.flags[index] = {};
    _slots.value[index] = 0;
    _slots.packed[index] = 0;

    _slots_cnt++;

    _update_feeds();
}
void TelemetryEncoder::clear()
{
    _slots_cnt = _slots_upd_cnt = 0;
    _slots = {};
    _update_feeds();
}

TelemetryEncoder::result_e TelemetryEncoder::update(const xbus::pid_s &pid, mandala::raw_t raw, mandala::type_id_e type_id, bool sync)
{
    const auto uid = pid.uid;

    // called by MandalaDataBroker

    auto idx = xbus::telemetry::field_lookup(uid, _slots.fields, _slots_cnt);
    if (idx >= 0) {
        _set_data(idx, raw, type_id);
        return ok;
    }

    if (!sync)
        return skipped;

    uint16_t pos = _slots_cnt + _sync_cnt;

    // check for already pending sync - update value data
    idx = xbus::telemetry::field_lookup(uid, _slots.fields + _slots_cnt, _sync_cnt);
    if (idx >= 0) {
        _set_data(idx + _slots_cnt, raw, type_id);
        return pos >= slots_size ? sync_ovf : skipped;
    }

    // add new slot as pending
    if (pos >= slots_size)
        return sync_ovf;

    auto fmt = mandala::fmt(uid);
    if (fmt.ds == mandala::ds_aux)
        return skipped;

    pos++;
    _sync_cnt++;

    auto &f = _slots.fields[pos];

    f.pid = xbus::pid_s(uid, xbus::pri_final, fmt.seq);
    f.fmt = fmt.fmt;
    _set_data(pos, raw, type_id);

    return pos >= slots_size ? sync_ovf : synced;
}

void TelemetryEncoder::sync_flush()
{
    auto pos = _slots_cnt + _sync_cnt;
    for (auto i = _slots_cnt; i < pos; ++i) {
        auto f = _slots.fields[i];
        auto flags = _slots.flags[i];
        auto value = _slots.value[i];
        auto packed = _slots.packed[i];
        if (!add(f))
            break;

        auto dest = _inserted_index;
        if (dest >= 0) {
            _slots.flags[dest] = flags;
            _slots.value[dest] = value;
            _slots.packed[dest] = packed;
        }
    }
    _sync_cnt = 0;
}

void TelemetryEncoder::_set_data(size_t n, mandala::raw_t raw, mandala::type_id_e type_id)
{
    auto &prev = _slots.value[n];
    auto &flags = _slots.flags[n];

    if (prev == raw && flags.type == type_id)
        return;
    prev = raw;
    flags.type = type_id;

    auto &f = _slots.fields[n];
    if (f.pid.seq != seq_scheduled)
        flags.upd = true;
}

bool TelemetryEncoder::encode(XbusStreamWriter &stream, uint8_t pseq, uint64_t timestamp_ms)
{
    hdr_s hdr;
    hdr.ts = timestamp_ms / hdr.ts2ms;

    hdr.feed_hash = _hash.byte[pseq & 3];

    hdr.write(&stream);

    encode_values(stream, pseq);

    return true;
}

void TelemetryEncoder::_update_feeds()
{
    const uint16_t size = _slots_cnt * sizeof(*_slots.fields);

    uint32_t vhash = apx::crc32(_slots.fields, size, xbus::telemetry::fmt_version);
    _hash.byte[0] = vhash;
    _hash.byte[1] = vhash >> 8;
    _hash.byte[2] = vhash >> 16;
    _hash.byte[3] = vhash >> 24;
}

void TelemetryEncoder::encode_values(XbusStreamWriter &stream, uint8_t pseq)
{
    uint8_t *code_cnt = stream.ptr();
    stream.write<uint8_t>(0);

    size_t pos_data = stream.pos();
    uint8_t *code = stream.ptr();
    uint8_t code_bit = (1 << 6);
    uint8_t code_zero = 0;
    uint8_t code_index = 0;

    uint8_t *nibble = nullptr;
    uint8_t nibble_n = 0;

    // continuously enforce slots update
    _slots.flags[_enforced_upd].upd = true;
    _slots.packed[_enforced_upd] += 0x01010101; //re-pack
    _enforced_upd++;
    if (_enforced_upd >= _slots_cnt)
        _enforced_upd = 0;
    else if (_enforced_upd >= _slots_upd_cnt)
        _enforced_upd = 0;

    // pack updated slots values
    size_t index = 0;
    for (; index < _slots_cnt; ++index) {
        auto const &f = _slots.fields[index];
        if (f.fmt == fmt_bit)
            break;

        switch (f.pid.seq) {
        case seq_always:
            break;
        case seq_skip:
            if (pseq & 1)
                continue;
            break;
        case seq_rare:
            if (pseq & 3)
                continue;
            break;
        case seq_scheduled:
            break;
        }

        if (code_bit == (1 << 6)) {
            if (code_zero == 0)
                code_index = index;
            code_zero++;
            code_bit = 1;
            code = stream.ptr();
            stream.write<uint8_t>(0);
            nibble_n = 0;
        } else
            code_bit <<= 1;

        auto &flags = _slots.flags[index];
        if (flags.upd) {
            flags.upd = false;

            auto &value = _slots.value[index];
            auto &buf = _slots.packed[index];
            size_t sz = xbus::telemetry::pack_value(value, flags.type, &buf, f.fmt);
            if (sz) {
                if (code_zero >= 2) {
                    // two or more consequtive zero codes
                    stream.reset(stream.pos() - code_zero);
                    uint8_t dindex = index - code_index;
                    uint8_t offset = dindex;
                    while (dindex > 0 || offset == 0x7F) {
                        offset = dindex;
                        if (offset > 0x7F)
                            offset = 0x7F;
                        stream.write<uint8_t>(offset | 0x80);
                        dindex -= offset;
                    }
                    code_bit = 1 << 6;
                } else {
                    (*code) |= code_bit;
                }

                if (f.fmt == fmt_u4) {
                    uint8_t v = buf & 0x0F;
                    if (nibble_n == 0) {
                        nibble_n = 1;
                        nibble = stream.ptr();
                        stream.write<uint8_t>(v);
                        pos_data = stream.pos();
                    } else {
                        nibble_n = 0;
                        (*nibble) |= v << 4;
                    }
                } else {
                    nibble_n = 0;
                    stream.write(&buf, sz);
                    pos_data = stream.pos();
                }
                (*code_cnt)++;
                code_zero = 0;
            }
        }
    }

    stream.reset(pos_data);

    //append all bitfields
    code_bit = (1 << 7);
    for (; index < _slots_cnt; ++index) {
        if (code_bit == (1 << 7)) {
            code = stream.ptr();
            stream.write<uint8_t>(0);
            code_bit = 1;
        } else
            code_bit <<= 1;
        auto &value = _slots.value[index];
        if (value) {
            (*code) |= code_bit;
        }
    }
}

void TelemetryEncoder::encode_format(XbusStreamWriter &stream, uint8_t part, const uint8_t size)
{
    xbus::telemetry::format_resp_hdr_s h;

    h.version = xbus::telemetry::fmt_version;
    h.hash = _hash.hash;

    h.psz = size;

    size_t cnt = _slots_cnt * sizeof(*_slots.fields);

    h.pcnt = cnt / size;
    if (cnt % size)
        h.pcnt++;

    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(_slots.fields);
    size_t sz = part < h.pcnt ? cnt - (part * size) : 0;
    if (sz > size)
        sz = size;

    h.part = part;

    h.write(&stream);

    if (sz > 0)
        stream.write(ptr + part * size, sz);
}
