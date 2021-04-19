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

#include <QueueBuffer.h>

#include "SerialCodec.h"

template<size_t _buf_size, typename T = uint8_t>
class EscDecoder : public SerialDecoder
{
public:
    explicit EscDecoder()
        : SerialDecoder(_buf, _buf_size)
    {}

    //decode ESC encoded data and write packet to fifo
    ErrorType decode(const void *src, size_t sz) override
    {
        if (sz == 0)
            return DataDropped;
        //restore fifo with partially read packet
        if (_rcnt > 0) {
            push_head(_head_pending_s);
        }

        ErrorType ret = DataAccepted;
        const T *rbuf = static_cast<const T *>(src);
        size_t cnt = sz;
        while (cnt--) {
            T v = *rbuf++;
            switch (_state) {
            case 0: //wait for start sequence
                if (v == 0x55)
                    _state = 3;
                continue;
            case 1: //data
                if (v == 0x55) {
                    _state = 2;
                    continue;
                }
            case_DATA:
                if (!space()) {
                    ret = ErrorOverflow;
                    break;
                }
                if (_rcnt == 0) {
                    _head_s = head();
                    if (!write_word(0)) {
                        ret = ErrorOverflow;
                        break;
                    }
                }
                _rcnt++;
                _crc += v;
                if (!write(v)) {
                    ret = ErrorOverflow;
                    break;
                }
                continue;
            case 2: //escape
                if (v == 0x02) {
                    v = 0x55;
                    _state = 1;
                    goto case_DATA;
                }
                if (v == 0x03) {
                    if (_rcnt <= 1) {
                        ret = ErrorSize;
                        break; //no data
                    }
                    T v_crc = pop_one();
                    _crc -= v_crc;
                    if (_crc != v_crc) {
                        ret = ErrorCRC;
                        break;
                    }
                    //frame received...
                    size_t h = head();
                    pop_head(_head_s);
                    write_word(_rcnt - 1);
                    push_head(h);
                    _state = 0;
                    _rcnt = 0;
                    ret = DataAccepted;
                    continue;
                }
                if (v == 0x55) {
                    _state = 3;
                    continue;
                }
                error();
                //fallthru
            case 3: // start..
                if (v == 0x01) {
                    start();
                    continue;
                }
                break;
            }
            //error
            error();
        }
        if (_rcnt > 0) {
            //block fifo until packet read done
            _head_pending_s = head();
            pop_head(_head_s);
        }
        return ret; //always accept all bytes
    }

    inline size_t read_decoded(void *dest, size_t sz) override
    {
        return QueueBuffer<_buf_size, T>::read_packet(dest, sz);
    }

private:
    uint8_t _buf[_buf_size];

    uint8_t _state{0};
    size_t _rcnt{0};
    T _crc;
    size_t _head_s;
    size_t _head_pending_s;

    inline void start()
    {
        _state = 1;
        _crc = 0;
        _rcnt = 0;
    }
    inline void error()
    {
        _state = 0;
        if (_rcnt > 0) {
            _rcnt = 0;
            pop_head(_head_s);
        }
    }
};
