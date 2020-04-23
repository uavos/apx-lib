#include "TelemetryEncoder.h"

#include "TelemetryValuePack.h"

#include <crc/crc.h>

using namespace xbus::telemetry;

TelemetryEncoder::TelemetryEncoder()
{
    memset(&_slots, 0, sizeof(_slots));
    _update_feeds();
}

void TelemetryEncoder::add(const field_s &field)
{
    for (size_t i = 0; i < enc_slots_size; ++i) {
        auto &f = _slots.fields[i];
        if (f.fmt)
            continue;

        // found empty slot
        f = field;
        memset(&(_slots.flags[i]), 0, sizeof(*_slots.flags));
        memset(&(_slots.value[i]), 0, sizeof(*_slots.value));
        memset(&(_slots.buf[i]), 0, sizeof(*_slots.buf));
        _update_feeds();

        return;
    }
}

bool TelemetryEncoder::update(const xbus::pid_s &pid, const mandala::spec_s &spec, XbusStreamReader &stream)
{
    // called by MandalaDataBroker
    for (size_t i = 0; i < enc_slots_size; ++i) {
        auto const &f = _slots.fields[i];
        if (!f.fmt)
            break;
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

bool TelemetryEncoder::encode(XbusStreamWriter &stream, uint8_t seq, xbus::telemetry::rate_e rate)
{
    stream_s hdr;
    hdr.spec.seq = seq >> 2;
    hdr.spec.rate = rate;

    hdr.feed_hash = _hash.byte[seq & 3];
    hdr.feed_fmt = _get_feed_fmt();

    hdr.write(&stream);

    encode_values(stream, seq);

    return true;
}

void TelemetryEncoder::_update_feeds()
{
    uint16_t size = 0;
    for (auto &f : _slots.fields) {
        if (!f.fmt)
            break;
        size += sizeof(f);
    }
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

    _fmt_size = size; // append crc
    _fmt_pos = _fmt_size;
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
        return _cobs_code;
    }
    uint8_t v = _fmt_pos == _fmt_size ? _slots.crc_cobs : buf[_fmt_pos];
    _fmt_pos++;
    _cobs_code--;
    if (_cobs_code == 0) {
        return _get_feed_fmt();
    }
    return v;
}

void TelemetryEncoder::encode_values(XbusStreamWriter &stream, uint8_t seq)
{
    uint8_t *code_cnt = stream.ptr();
    stream.write<uint8_t>(0);

    size_t pos_s = stream.pos();
    uint8_t *code = stream.ptr();
    uint8_t code_bit = 0x80;

    for (size_t i = 0; i < enc_slots_size; ++i) {
        auto const &f = _slots.fields[i];
        if (!f.fmt)
            break;

        if (code_bit == 0x80) {
            code = stream.ptr();
            stream.write<uint8_t>(0);
            code_bit = 1;
        } else
            code_bit <<= 1;

        auto &flags = _slots.flags[i];
        if (flags.upd && (f.pid.seq & seq) == 0) {
            flags.upd = false;

            auto &value = _slots.value[i];
            auto &buf = _slots.buf[i];
            size_t sz = xbus::telemetry::pack_value(&value, &buf, flags.type, f.fmt);
            if (sz) {
                stream.write(&buf, sz);
                (*code) |= code_bit;
                (*code_cnt)++;
                pos_s = stream.pos();
            }
        }
    }
    stream.reset(pos_s);
}
