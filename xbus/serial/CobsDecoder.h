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

#include <crc.h>

#include "SerialCodec.h"

template<
    size_t _packet_size = xbus::size_packet_max,
    typename T = uint8_t,
    T _esc = 0>
class CobsDecoder : public SerialDecoder
{
public:
    size_t decode(const void *src, size_t sz) override
    {
        _status = Unknown;

        const T *data = static_cast<const T *>(src);
        size_t cnt = sz;
        while (cnt--) {
            T c = *data++;
            if (c == _esc) {
                //packet delimiter
                if (_copy) {
                    // error, escape character in data
                    _status = ErrorSize;
                    restart();
                    continue;
                }
                if (_rcnt <= sizeof(uint16_t)) {
                    // too short packet with crc16 only
                    _status = ErrorSize;
                    restart();
                    continue;
                }
                _rcnt -= sizeof(uint16_t);
                const uint8_t *p = &_buf[_rcnt];
                uint16_t packet_crc16 = p[0] | (p[1] << 8);
                uint16_t crc16 = apx::crc32(_buf, _rcnt);
                if (packet_crc16 != crc16) {
                    _status = ErrorCRC;
                    restart();
                    continue;
                }
                // valid packet is received
                sz -= cnt;
                _rsize = _rcnt;
                _status = PacketAvailable;
                restart(); // packet valid until next decode() call
                return sz;
            }
            if (_copy != 0) {
                if (!write_decoded_byte(c)) {
                    _status = ErrorOverflow;
                    restart();
                    continue;
                }
            } else {
                if (_code != 0xFF) {
                    if (!write_decoded_byte(_esc)) {
                        _status = ErrorOverflow;
                        restart();
                        continue;
                    }
                }
                _copy = _code = c;
            }
            _copy--;
        }

        // all bytes are processed
        if (_status == Unknown)
            _status = sz ? DataAccepted : DataDropped;

        return sz;
    }

    const uint8_t *data() const override
    {
        return _buf;
    }

    size_t size() const override
    {
        return _rsize;
    }

    void reset() override
    {
        _status = Unknown;
        restart();
    }

    // allow buffer sharing
    auto buf() { return _buf; }
    auto buf_size() const { return sizeof(_buf); }

private:
    static constexpr size_t overhead()
    {
        // +crc16
        return sizeof(uint16_t);
    }

    T _buf[_packet_size + overhead()];
    size_t _rsize{};

    size_t _head_pending_s;
    size_t _head_s;
    size_t _rcnt{};

    T _code{0xFF};
    T _copy{0};

    inline void restart()
    {
        _code = 0xFF;
        _copy = 0;
        _rcnt = 0;
    }

    inline constexpr bool write_decoded_byte(T c)
    {
        if (_rcnt >= _packet_size) {
            return false;
        }
        _buf[_rcnt++] = c;
        return true;
    }
};
