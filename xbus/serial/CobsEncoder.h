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

// Consistant Overhead Byte Stuffing (COBS) encoder
// Packetization protocol:
// - benefit is to have a single byte for each byte of data and minimum overhead (just 1 byte max for xbus)
// - https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
// - added CRC16 to each packet at the end

template<
    size_t _packet_size = xbus::size_packet_max,
    typename T = uint8_t,
    T _esc = 0>
class CobsEncoder : public SerialEncoder
{
public:
    // encode packet and return its size
    size_t encode(const void *src, size_t sz) override
    {
        uint8_t *dest = _buf;

        // start block
        T *code_ptr = dest;
        T code = 1;
        *dest++ = code;

        // copy data first
        const T *data = static_cast<const T *>(src);
        for (auto cnt = sz; cnt;) {
            if (code != 0xFF) {
                auto c = *data++;
                cnt--;
                if (c != _esc) {
                    *dest++ = c;
                    code++;
                    continue;
                }
            }
            // finish block
            *code_ptr = code;
            // start block
            code_ptr = dest;
            code = 1;
            *dest++ = code;
        }
        // copy crc16 at the end
        uint16_t crc16 = apx::crc32(src, sz);
        const uint8_t *crc16_ptr = reinterpret_cast<const uint8_t *>(&crc16);
        for (auto cnt = 2; cnt;) {
            if (code != 0xFF) {
                auto c = *crc16_ptr++;
                cnt--;
                if (c != _esc) {
                    *dest++ = c;
                    code++;
                    continue;
                }
            }
            // finish block
            *code_ptr = code;
            // start block
            code_ptr = dest;
            code = 1;
            *dest++ = code;
        }

        // finish block
        *code_ptr = code;

        //append packet delimiter
        *dest++ = _esc;

        // all encoded
        return dest - _buf;
    }

    const uint8_t *data() const override
    {
        return _buf;
    }

    // allow buffer sharing
    auto buf() { return _buf; }
    auto buf_size() const { return sizeof(_buf); }

private:
    // any packet will always fit in buffer
    // there's no overflow checks for performance reasons
    static constexpr size_t overhead(size_t packet_size)
    {
        // Worst case - no _esc bytes anywhere in the stream: the
        // payload plus the appended crc16 encode as blocks of up to
        // 254 data bytes, each led by one code byte, plus the packet
        // delimiter. NOTE the divisor is 254 (a 0xFF code covers 254
        // data bytes), not 255, and the crc16 rides INSIDE the encoded
        // stream - either mistake shaves bytes off the buffer that a
        // full-size packet then silently overruns.
        const size_t stream = packet_size + sizeof(uint16_t); // payload + crc16
        return sizeof(uint16_t)       // the crc16 bytes themselves
               + (stream + 253) / 254 // one code byte per started block
               + 1;                   // packet delimiter
    }

    // any packet will always fit in buffer
    // there's no overflow checks for performance reasons
    T _buf[_packet_size + overhead(_packet_size)];

    // independently derived worst-case write count of encode():
    // 1 initial code byte + data + crc16 + one extra code byte after
    // every full 254-byte run + the delimiter. Guards overhead()
    // against a future "optimization" - encode() has no bounds checks.
    static_assert(sizeof(_buf) / sizeof(T)
                      >= 1 + _packet_size + sizeof(uint16_t)
                             + (_packet_size + sizeof(uint16_t) - 1) / 254 + 1,
                  "COBS worst case must fit the buffer");
};
