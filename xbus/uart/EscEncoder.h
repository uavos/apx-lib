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
/*
template<size_t _buf_size, typename T = uint8_t>
class EscEncoder : private QueueBuffer<_buf_size, T>, public SerialEncoder
{
public:
    using QueueBuffer<_buf_size, T>::reset;
    using QueueBuffer<_buf_size, T>::read;
    using QueueBuffer<_buf_size, T>::size;
    using QueueBuffer<_buf_size, T>::empty;
    using QueueBuffer<_buf_size, T>::total;
    using QueueBuffer<_buf_size, T>::tail;
    using QueueBuffer<_buf_size, T>::head;
    using QueueBuffer<_buf_size, T>::read_ptr;
    using QueueBuffer<_buf_size, T>::skip_read;

    //write and encode data to fifo
    size_t encode(const void *src, size_t sz) override
    {
        if (space() < (sz + 2 + 2 + 1)) //estimate
            return 0;
        size_t head_s = head();
        do {
            if (!write(0x55))
                break;
            if (!write(0x01))
                break;

            const T *buf = static_cast<const T *>(src);
            uint8_t crc = 0;
            size_t cnt = sz;
            while (cnt > 0) {
                T v = *buf++;
                crc += v;
                if (!write(v))
                    break;
                if (v == 0x55 && !write(0x02))
                    break;
                cnt--;
            }
            if (cnt > 0)
                break;
            if (!write(crc))
                break;
            if (crc == 0x55 && !write(0x02))
                break;
            if (!write(0x55))
                break;
            if (!write(0x03))
                break;
            return sz;
        } while (0);
        //error - fifo overflow
        pop_head(head_s);
        return 0;
    }

    inline size_t read_encoded(void *dest, size_t sz) override
    {
        return QueueBuffer<_buf_size, T>::read(dest, sz);
    }
    inline size_t size() override
    {
        return QueueBuffer<_buf_size, T>::size();
    }
    inline void reset() override
    {
        QueueBuffer<_buf_size, T>::reset();
    }

private:
    using QueueBuffer<_buf_size, T>::space;
    using QueueBuffer<_buf_size, T>::pop_head;
    using QueueBuffer<_buf_size, T>::write;
};
*/
