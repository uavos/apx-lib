#include "TelemetryDecoder.h"
#include <crc/crc.h>

#include "TelemetryValueUnpack.h"

using namespace xbus::telemetry;

TelemetryDecoder::TelemetryDecoder()
{
    memset(&_slots, 0, sizeof(_slots));
}

bool TelemetryDecoder::decode(const xbus::pid_s &pid, XbusStreamReader &stream)
{
    if (stream.available() < xbus::telemetry::stream_s::psize())
        return false;

    xbus::telemetry::stream_s hdr;
    hdr.read(&stream);

    uint8_t seq = pid.seq; // | static_cast<uint8_t>(hdr.spec.seq << 2);
    uint8_t dseq = (seq - _seq) & 3;
    _seq = seq;
    bool seq_ok = dseq == 1;

    _dts = hdr.ts - _ts;
    _ts = hdr.ts;

    uint8_t &h = _hash.byte[seq & 3];
    if (h != hdr.feed_hash) {
        if (h)
            reset();
        h = hdr.feed_hash;
        _hash_valid++;
    } else if (!_is_hash_valid())
        _hash_valid++;

    // check seq and fmt feed
    if (!seq_ok) {
        _fmt_pos = 0;
        _cobs_code = 0;
    }
    _set_feed_fmt(hdr.feed_fmt);

    _valid = false;

    if (!_is_hash_valid())
        return false;

    if (!_slots_cnt)
        return false;

    // fmt looks consistent
    _valid = true;
    return decode_values(stream);
}

float TelemetryDecoder::rate()
{
    if (_dts > 100 || _dts == 0)
        return 0;
    return 10.f / _dts;
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
    return CRC_32_APX(_slots.fields, sz, 0);
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

void TelemetryDecoder::_set_feed_fmt(uint8_t v)
{
    if (_fmt_pos == 0 && _cobs_code == 0) {
        if (v != 0) // wait for SOF
            return;
        _cobs_code = 0xFF;
        _cobs_copy = 0;
        return;
    }
    uint8_t *buf = reinterpret_cast<uint8_t *>(_slots.fields);

    if (v == 0) {
        // packet delimiter
        do {
            if (_cobs_copy)
                break;
            if (_fmt_pos < (sizeof(field_s) + 1) || ((_fmt_pos - 1) % sizeof(field_s)))
                break;
            // check crc_xor
            uint8_t crc_xor = 0;
            for (size_t i = 0; i < _fmt_pos; ++i)
                crc_xor ^= buf[i];
            if (crc_xor)
                break;
            buf[--_fmt_pos] = 0;
            // check hash
            if (!check_hash(_fmt_pos))
                break;
            // fmt array ok
            _fmt_pos = 0;
            _cobs_code = 0;
            return;
        } while (0);
        // packet error
        if (_fmt_pos) {
            _slots_cnt = 0;
            _fmt_pos = 0;
        }
        _cobs_code = 0;
        return;
    }
    bool skip = false;
    if (_cobs_copy == 0) {
        skip = _cobs_code == 0xFF;
        _cobs_copy = _cobs_code = v;
        v = 0;
    }
    _cobs_copy--;

    if (skip)
        return;

    if (buf[_fmt_pos] != v) {
        buf[_fmt_pos] = v;
        if (_fmt_pos < (_slots_cnt * sizeof(field_s)))
            _slots_cnt = 0;
    }
    _fmt_pos++;
}

bool TelemetryDecoder::decode_values(XbusStreamReader &stream)
{
    if (stream.available() == 0)
        return false;

    uint8_t cnt = stream.read<uint8_t>();
    uint8_t code = 0;
    uint8_t code_bit = 0x80;

    bool upd = false;
    for (size_t i = 0; i < _slots_cnt; ++i) {
        auto const &f = _slots.fields[i];

        if (!cnt)
            break;

        if (code_bit == 0x80) {
            code = stream.read<uint8_t>();
            code_bit = 1;
        } else
            code_bit <<= 1;

        if (stream.available() == 0)
            break;

        if (!(code & code_bit))
            continue;

        auto &value = _slots.value[i];
        mandala::type_id_e type;
        size_t sz = unpack_value(stream.ptr(), &value, &type, f.fmt, stream.available());
        if (!sz || sz > stream.available())
            break;
        stream.reset(stream.pos() + sz);

        auto &flags = _slots.flags[i];
        flags.type = type;
        flags.upd = true;
        upd = true;

        cnt--;

        if (stream.available() == 0 || cnt == 0)
            break;
    }

    if (stream.available() != 0 || cnt != 0) {
        reset();
        return false;
    }
    return upd;
}

bool TelemetryDecoder::decode_format(uint8_t part, uint8_t parts, XbusStreamReader &stream)
{
    if (part == 0) {
        //prepend hash on first part
        uint32_t hash;
        stream >> hash;
        if (hash != _hash.hash)
            reset();
        _hash.hash = hash;
        _hash_valid = sizeof(hash);
    }
    uint8_t *ptr = reinterpret_cast<uint8_t *>(_slots.fields);
    size_t pos = part * fmt_block_size;
    size_t sz = stream.available();
    bool is_final = (part + 1) == parts;
    do {
        if (is_final && stream.available() > fmt_block_size)
            break;
        if (!is_final && stream.available() != fmt_block_size)
            break;
        if ((pos + sz) > sizeof(_slots.fields))
            break;
        stream.read(ptr + pos, sz);
        if (is_final) {
            pos += sz;
            if (pos % sizeof(field_s)) {
                break;
            }
            if (check_hash(pos))
                return true;
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
