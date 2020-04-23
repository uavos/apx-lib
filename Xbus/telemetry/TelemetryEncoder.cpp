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
    if (_slots_cnt >= enc_slots_size)
        return;

    auto &f = _slots.fields[_slots_cnt];

    // found empty slot
    f = field;
    memset(&(_slots.flags[_slots_cnt]), 0, sizeof(*_slots.flags));
    _slots.value[_slots_cnt] = 0;
    _slots.packed[_slots_cnt] = 0;
    //_slots.timeout[_slots_cnt] = _slots_cnt;
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
            return _cobs_code;
        }
        uint8_t v = (_fmt_pos == _fmt_size) ? _slots.crc_cobs : buf[_fmt_pos];
        _fmt_pos++;

        _cobs_code--;
        if (_cobs_code == 0)
            continue;

        return v;
    }
}

void TelemetryEncoder::encode_values(XbusStreamWriter &stream, uint8_t seq)
{
    uint8_t *code_cnt = stream.ptr();
    stream.write<uint8_t>(0);

    size_t pos_s = stream.pos();
    uint8_t *code = stream.ptr();
    uint8_t code_bit = 0x80;

    uint8_t scheduled_index = seq % _slots_cnt;

    for (size_t i = 0; i < _slots_cnt; ++i) {
        auto const &f = _slots.fields[i];

        if (code_bit == 0x80) {
            code = stream.ptr();
            stream.write<uint8_t>(0);
            code_bit = 1;
        } else
            code_bit <<= 1;

        auto &flags = _slots.flags[i];

        //re-schedule according to index
        if (i == scheduled_index) {
            flags.upd = true;
            _slots.packed[i]++; //re-pack
        }

        if (flags.upd && (f.pid.seq & seq) == 0) {
            flags.upd = false;

            auto &value = _slots.value[i];
            auto &buf = _slots.packed[i];
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
