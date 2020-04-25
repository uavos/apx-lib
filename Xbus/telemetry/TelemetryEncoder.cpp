#include "TelemetryEncoder.h"

#include "TelemetryValuePack.h"

#include <crc/crc.h>

using namespace xbus::telemetry;

TelemetryEncoder::TelemetryEncoder()
{
    memset(&_slots, 0, sizeof(_slots));
    _update_feeds();
}

bool TelemetryEncoder::add(const field_s &field)
{
    if (_slots_cnt >= slots_size)
        return false;

    // find dups
    for (size_t i = 0; i < _slots_cnt; ++i) {
        auto const &f = _slots.fields[i];
        if (f.pid.uid != field.pid.uid)
            continue;
        if (f.pid.pri != field.pid.pri)
            continue;
        return false;
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
    } else if (field.fmt == fmt_opt) { // opts at end before bitfields
        while (index < _slots_cnt) {
            auto const &f = _slots.fields[index];
            if (f.fmt == fmt_bit)
                break;
            if (f.fmt == fmt_opt)
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
            if (f.fmt == fmt_opt)
                break;
            if (f.pid.uid > field.pid.uid)
                break;
            index++;
        }
    }

    _insert(index, field);
    return true;
}
void TelemetryEncoder::_insert(size_t index, const xbus::telemetry::field_s &field)
{
    if (index < _slots_cnt) {
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

bool TelemetryEncoder::update(const xbus::pid_s &pid, const mandala::spec_s &spec, XbusStreamReader &stream)
{
    // called by MandalaDataBroker
    for (size_t i = 0; i < _slots_cnt; ++i) {
        auto const &f = _slots.fields[i];
        if (f.pid.uid != pid.uid)
            continue;
        if (f.pid.pri != pid.pri)
            continue;
        _set_data(i, spec, stream);
        return true;
    }
    return false;
}

void TelemetryEncoder::_set_data(size_t n, const mandala::spec_s &spec, XbusStreamReader &stream)
{
    mandala::raw_t v = 0;
    stream.read(&v, mandala::type_size(spec.type));

    auto &value = _slots.value[n];

    if (value == v)
        return;
    value = v;

    auto &flags = _slots.flags[n];
    flags.type = spec.type;
    flags.upd = true;
}

bool TelemetryEncoder::encode(XbusStreamWriter &stream, uint8_t seq, uint16_t ts)
{
    stream_s hdr;
    hdr.ts = ts;

    hdr.feed_hash = _hash.byte[seq & 3];
    hdr.feed_fmt = _get_feed_fmt();

    hdr.write(&stream);

    encode_values(stream, seq);

    return true;
}

void TelemetryEncoder::_update_feeds()
{
    const uint16_t size = _slots_cnt * sizeof(*_slots.fields);

    uint32_t vhash = CRC_32_APX(_slots.fields, size, 0);
    _hash.byte[0] = vhash;
    _hash.byte[1] = vhash >> 8;
    _hash.byte[2] = vhash >> 16;
    _hash.byte[3] = vhash >> 24;

    //fields fmt feed
    uint8_t crc_cobs = 0;
    for (size_t i = 0; i < size; ++i)
        crc_cobs ^= reinterpret_cast<const uint8_t *>(_slots.fields)[i];
    _slots.crc_cobs = crc_cobs;

    _fmt_size = size;
    _fmt_pos = _fmt_size + 1;
    _cobs_code = 0;
}

uint8_t TelemetryEncoder::_get_feed_fmt()
{
    if (_fmt_size == 0)
        return 0;
    if (_fmt_pos > _fmt_size) {
        _fmt_pos = 0;
        _cobs_code = 0;
        return 0;
    }

    const uint8_t *buf = reinterpret_cast<const uint8_t *>(_slots.fields);

    for (;;) {
        // push v to COBS stream
        if (_cobs_code == 0) {
            //find and return code, relpos of zero
            _cobs_code = 1;
            for (uint16_t i = _fmt_pos; i <= _fmt_size; ++i) {
                uint8_t v = i < _fmt_size ? buf[i] : _slots.crc_cobs;
                if (v == 0)
                    break;
                _cobs_code++;
                if (_cobs_code == 0xFF)
                    break;
            }
            _cobs_copy = _cobs_code;
            return _cobs_code;
        }
        uint8_t v = (_fmt_pos >= _fmt_size) ? _slots.crc_cobs : buf[_fmt_pos];

        _cobs_code--;
        if (_cobs_code == 0) {
            if (_cobs_copy != 0xFF)
                _fmt_pos++;
            continue;
        }

        _fmt_pos++;
        return v;
    }
}

void TelemetryEncoder::encode_values(XbusStreamWriter &stream, uint8_t seq)
{
    uint8_t *code_cnt = stream.ptr();
    stream.write<uint8_t>(0);

    size_t pos_data = stream.pos();
    uint8_t *code = stream.ptr();
    uint8_t code_bit = 0x80;

    uint8_t scheduled_index = seq % (_slots_cnt + 1);

    size_t index = 0;
    for (; index < _slots_cnt; ++index) {
        auto const &f = _slots.fields[index];
        if (f.fmt == fmt_bit)
            break;
        //if (f.fmt == fmt_opt)
        //    break;

        if (code_bit == 0x80) {
            code = stream.ptr();
            stream.write<uint8_t>(0);
            code_bit = 1;
        } else
            code_bit <<= 1;

        auto &flags = _slots.flags[index];

        //re-schedule according to index
        if (index == scheduled_index) {
            flags.upd = true;
            _slots.packed[index] = ~_slots.packed[index]; //re-pack
        }

        if (flags.upd && ((f.pid.seq & seq) == 0)) {
            flags.upd = false;

            auto &value = _slots.value[index];
            auto &buf = _slots.packed[index];
            size_t sz = xbus::telemetry::pack_value(&value, &buf, flags.type, f.fmt);
            if (sz) {
                stream.write(&buf, sz);
                pos_data = stream.pos();
                (*code) |= code_bit;
                (*code_cnt)++;
            }
        }
    }

    //append opt nibbles
    code_bit = 0x80;
    uint8_t *nibble = nullptr;
    uint8_t nibble_n = 0;
    for (; index < _slots_cnt; ++index) {
        auto const &f = _slots.fields[index];
        if (f.fmt != fmt_opt)
            break;
        if (f.fmt == fmt_bit)
            break;

        if (code_bit == 0x80) {
            code = stream.ptr();
            stream.write<uint8_t>(0);
            code_bit = 1;
            nibble_n = 0;
        } else
            code_bit <<= 1;

        auto &flags = _slots.flags[index];

        //re-schedule according to index
        if (index == scheduled_index) {
            flags.upd = true;
            _slots.packed[index]++; //re-pack
        }

        if (flags.upd) {
            flags.upd = false;

            auto &value = _slots.value[index];
            auto &buf = _slots.packed[index];
            size_t sz = xbus::telemetry::pack_value(&value, &buf, flags.type, f.fmt);
            if (sz) {
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
                (*code) |= code_bit;
                (*code_cnt)++;
            }
        }
    }

    stream.reset(pos_data);

    //append all bitfields
    code_bit = 0x80;
    for (; index < _slots_cnt; ++index) {
        if (code_bit == 0x80) {
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

void TelemetryEncoder::encode_format(XbusStreamWriter &stream, uint8_t part)
{
    size_t size = _slots_cnt * sizeof(*_slots.fields);

    uint8_t parts = size / fmt_block_size;
    if (size % fmt_block_size)
        parts++;

    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(_slots.fields);
    size_t sz = part < parts ? size - (part * fmt_block_size) : 0;
    if (sz > fmt_block_size)
        sz = fmt_block_size;

    stream << part;
    stream << parts;

    if (part == 0) {
        //prepend hash on first part
        stream << _hash.hash;
    }

    if (sz > 0)
        stream.write(ptr + part * fmt_block_size, sz);
}
