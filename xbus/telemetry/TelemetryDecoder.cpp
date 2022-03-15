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
#include "TelemetryDecoder.h"
#include <crc.h>

#include "TelemetryValueUnpack.h"

using namespace xbus::telemetry;

bool TelemetryDecoder::decode(uint8_t pseq, XbusStreamReader &stream)
{
    if (stream.available() < xbus::telemetry::hdr_s::psize())
        return false;

    xbus::telemetry::hdr_s hdr;
    hdr.read(&stream);

    // check hash feed
    uint8_t &h = _hash.byte[pseq & 3];
    if (h != hdr.feed_hash) {
        if (_hash_valid > 0 || h)
            reset();
        h = hdr.feed_hash;
        _hash_valid++;
    } else if (!_is_hash_valid())
        _hash_valid++;

    if (_hash_valid > 4)
        reset(); // this should never happen

    // estimate timestamp
    uint32_t dts = (uint16_t) (hdr.ts - _ts);
    _ts = hdr.ts;

    if (!dts)
        dts = 0x0000FFFF + 1;

    _dt_ms = dts * hdr.ts2ms;
    _timestamp_ms += _dt_ms;

    // validity check
    if (_is_hash_valid() && _slots_cnt) {
        // fmt looks consistent
        _valid = true;

        // get values from packed stream
        // seq bits are used to filter rare IDs only
        return decode_values(stream, pseq);
    }

    // not yet valid stream
    _valid = false;
    return false;
}

void TelemetryDecoder::reset(bool reset_hash)
{
    if (reset_hash) {
        _hash.hash = 0;
        _hash_valid = 0;
    }
    _slots_cnt = 0;
    _valid = false;
    memset(_slots.flags, 0, sizeof(_slots.flags));
    memset(_slots.value, 0, sizeof(_slots.value));
}

uint32_t TelemetryDecoder::get_hash(size_t sz)
{
    return apx::crc32(_slots.fields, sz, xbus::telemetry::fmt_version);
}
bool TelemetryDecoder::check_hash(size_t sz)
{
    uint32_t vhash = get_hash(sz);
    if (!_is_hash_valid() || vhash != _hash.hash)
        return false;
    // fmt array ok
    _slots_cnt = sz / sizeof(field_s);
    _valid = true;
    return true;
}

bool TelemetryDecoder::decode_values(XbusStreamReader &stream, uint8_t pseq)
{
    if (stream.available() == 0)
        return false;

    uint8_t cnt = stream.read<uint8_t>();
    uint8_t code = 0;
    uint8_t code_bit = (1 << 6);

    uint8_t nibble_v = 0;
    uint8_t nibble_n = 0;

    bool upd = false;
    size_t index = 0;
    for (; index < _slots_cnt && stream.available() > 0 && cnt > 0; ++index) {
        auto *f = &_slots.fields[index];

        if (f->fmt == fmt_bit)
            break;

        switch (f->pid.seq) {
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
            code = stream.read<uint8_t>();
            code_bit = 1;
            nibble_n = 0;
            if (code & (1 << 7)) {
                // special code - offset
                uint8_t offset = code & 0x7F;
                if (offset < 7)
                    break;
                int16_t dindex = offset;
                while (offset == 0x7F) {
                    code = 0;
                    offset = stream.read<uint8_t>();
                    if (!(offset & (1 << 7)))
                        break;
                    offset &= 0x7F;
                    code = (1 << 7);
                    dindex += offset;
                }
                if (!(code & (1 << 7)))
                    break;
                if (dindex < 7)
                    break;
                index += dindex;
                if (index >= _slots_cnt)
                    break;
                f = &_slots.fields[index];
                code_bit = (1 << 6);
                code = (1 << 6);
            }
        } else
            code_bit <<= 1;

        if (stream.available() == 0 && nibble_n == 0)
            break;

        if (!(code & code_bit))
            continue;

        auto &value = _slots.value[index];
        mandala::type_id_e type = mandala::type_real; // dummy
        if (nibble_n == 0) {
            size_t sz = unpack_value(stream.ptr(), &value, &type, f->fmt, stream.available());
            if (!sz || sz > stream.available())
                break;
            stream.reset(stream.pos() + sz);
        }
        if (f->fmt == fmt_opt) {
            if (nibble_n == 0) {
                nibble_n = 1;
                nibble_v = value & 0xFF;
                value &= 0x0F;
            } else {
                nibble_n = 0;
                value = nibble_v >> 4;
            }
            type = mandala::type_byte;
        } else {
            nibble_n = 0;
        }
        auto &flags = _slots.flags[index];
        flags.type = type;
        flags.upd = true;
        upd = true;

        cnt--;
        if (cnt == 0)
            break;
    }

    // read appended bitfields
    if (index < _slots_cnt && stream.available() > 0 && cnt == 0) {
        for (; index < _slots_cnt; ++index) {
            if (_slots.fields[index].fmt == fmt_bit)
                break;
        }
        cnt = _slots_cnt - index;
        code_bit = (1 << 7);
        for (; index < _slots_cnt; ++index) {
            auto const &f = _slots.fields[index];
            if (f.fmt != fmt_bit)
                break;

            if (code_bit == (1 << 7)) {
                if (stream.available() == 0)
                    break;
                code = stream.read<uint8_t>();
                code_bit = 1;
            } else
                code_bit <<= 1;

            const uint8_t v = (code & code_bit) ? 1 : 0;
            cnt--;

            auto &value = _slots.value[index];
            //if (value != v) {
            // always update
            value = v;
            auto &flags = _slots.flags[index];
            flags.type = mandala::type_byte;
            flags.upd = true;
            upd = true;
            //}
        }
    }

    if (stream.available() != 0 || cnt != 0) {
        reset();
        return false;
    }
    return upd;
}

bool TelemetryDecoder::decode_format(XbusStreamReader &stream, xbus::telemetry::format_resp_hdr_s *hdr)
{
    do {
        if (stream.available() < xbus::telemetry::format_resp_hdr_s::psize())
            break;

        hdr->read(&stream);
        if (hdr->version != xbus::telemetry::fmt_version)
            break;

        const size_t block_size = hdr->psz;

        if (!block_size)
            break;

        if (hdr->part == 0) {
            // first part
            if (hdr->hash != _hash.hash)
                reset();
            _hash.hash = hdr->hash;
            _hash_valid = sizeof(hdr->hash);
        } else {
            if (hdr->hash != _hash.hash)
                break;
        }

        uint8_t *ptr = reinterpret_cast<uint8_t *>(_slots.fields);
        size_t pos = hdr->part * block_size;
        size_t sz = stream.available();

        bool is_final = (hdr->part + 1) == hdr->pcnt;
        if (is_final && stream.available() > block_size)
            break;
        if (!is_final && stream.available() != block_size)
            break;
        if ((pos + sz) > sizeof(_slots.fields))
            break;
        stream.read(ptr + pos, sz);

        if (is_final) {
            pos += sz;
            if (pos % sizeof(field_s)) {
                break;
            }
            if (check_hash(pos)) {
                return true;
            }
        }

        // mid part
        if (_slots_cnt == 0)
            return true;

        if (check_hash(_slots_cnt * sizeof(*_slots.fields)))
            return true;
        reset(false);
        return true;
    } while (0);

    // error
    reset();
    return false;
}
