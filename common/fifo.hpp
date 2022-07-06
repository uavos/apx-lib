/*
 * APX Autopilot project <http://docs.uavos.com>
 *
 * Copyright (c) 2003-2022, Aliaksei Stratsilatau <sa@uavos.com>
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

#include <atomic>
#include <cstdint>
#include <cstring>

namespace apx {

// Circular buffer.
// Thread safe when used as pipe with single write and single read entries.
// Interrupt safe when used as pipe with single write and single read entries.
// Useful to share data between threads and interrupts without locks.

template<typename T>
class fifoT
{
public:
    explicit fifoT(T *buf, size_t size)
        : _buf(buf)
        , _size(size)
    {}

    inline bool empty() const { return _r == _w; }

    // return used number of elements
    size_t used() const
    {
        size_t w = _w;
        size_t r = _r;
        return _used(r, w);
    }

    // return free space available
    size_t free() const
    {
        size_t w = _w;
        size_t r = _r;
        return _free(r, w);
    }

    // reset buffer to default state (no data)
    // called on error and is not safe
    void reset()
    {
        _w = 0;
        _r = 0;
    }

    // write data to fifo and return number of elements written
    size_t write(const void *src, size_t sz)
    {
        size_t w = _w;
        size_t r = _r;

        // ensure data fits in buffer with (_size-1) as max free space
        size_t cnt = _free(r, w);
        if (sz > cnt)
            sz = cnt;
        cnt = _write(&w, r, src, sz);
        _w = w; // confirm write
        return cnt;
    }
    // write one element to fifo
    bool write(const T &src)
    {
        size_t w = _w;
        size_t r = _r;

        // ensure data fits in buffer with (_size-1) as max free space
        size_t cnt = _free(r, w);
        if (!cnt)
            return false;

        _buf[w++] = src;
        if (w >= _size)
            w = 0;

        _w = w; // confirm write
        return true;
    }

    // read data elements from buffer and release space
    size_t read(void *dest, size_t sz)
    {
        size_t w = _w;
        size_t r = _r;
        if (r == w)
            return 0;

        if (w > r) {
            // just one continuous block
            size_t cnt = w - r;
            if (cnt > sz)
                cnt = sz;
            _cpy(dest, &_buf[r], cnt);
            r += cnt;
            _r = r;
            return cnt;
        }

        // two linked blocks
        size_t cnt = _size - r;
        if (cnt >= sz) {
            // just one block fits
            _cpy(dest, &_buf[r], sz);
            r += cnt;
            _r = r;
            return sz;
        }
        _cpy(dest, &_buf[r], cnt);
        sz -= cnt;

        size_t rcnt = cnt; // total read cnt

        cnt = w;
        if (cnt > sz)
            cnt = sz;
        _cpy(dest, &_buf[0], cnt);

        _r = cnt;

        return cnt + rcnt;
    }
    // read one element
    void read(T *dest)
    {
        *dest = *rptr();
        skip_read(1);
    }

    // ---------------------------------------------
    // -- special helpers for DMA and LLD drivers --

    // get current read pointer
    inline const T *rptr() const
    {
        return &_buf[_r];
    }
    // get continous block read length
    size_t rsize() const
    {
        size_t w = _w;
        size_t r = _r;
        if (w > r)
            return w - r;
        if (r > w)
            return _size - r;
        return 0;
    }
    // advance read pointer and free/release memory (useful for dma write)
    void skip_read(size_t sz)
    {
        size_t r = _r;
        r += sz;
        if (r >= _size)
            r -= _size;
        _r = r;
    }

    // get current write pointer
    /*const T *wptr() const
    {
        return &_buf[_w];
    }
    // get continous block write length
    size_t wsize() const
    {
        size_t w = _w;
        size_t r = _r;
        if (w >= r)
            return (r == 0) ? (_size - w - 1) : (_size - w);
        return r - w - 1;
    }

    // advance write pointer
    size_t skip_write(size_t len)
    {
        size_t w = _w;
        size_t r = _r;

        size_t cnt = _free(r, w);
        if (len > cnt)
            len = cnt;

        w += len;
        if (w >= _size)
            w -= _size;

        _w = w;

        return len;
    }*/

protected:
    T *_buf;
    const size_t _size;

    // write and read pointers
    // when w=r the buffer is empty
    // i.e. max data size is (_size-1)
    std::atomic<size_t> _w{};
    std::atomic<size_t> _r{};

    // calculate used number of elements (optimization for concurrent access)
    size_t constexpr _used(size_t r, size_t w) const
    {
        return (w >= r) ? (w - r) : (w + _size - r);
    }

    // calculate free space (optimization for concurrent access)
    size_t constexpr _free(size_t r, size_t w) const
    {
        return (r > w) ? (r - w - 1) : (r + _size - w - 1);
    }

    // copy elements of data to buffer implementation
    static inline void _cpy(void *dest, const void *src, size_t elements)
    {
        memcpy(dest, src, sizeof(T) > 1 ? elements * sizeof(T) : elements);
    }

    // internal write buffer without bounds checking
    size_t _write(size_t *w_ptr, size_t r, const void *src, size_t sz)
    {
        size_t &w = *w_ptr;
        if (w >= r) {
            size_t cnt = _size - w;
            if (cnt >= sz) {
                // all data fits in one block
                _cpy(&_buf[w], src, sz);
                w += sz;
                if (w >= _size)
                    w = 0;
                return sz;
            }
            // data fits in two blocks of nonzero size
            _cpy(&_buf[w], src, cnt);
            sz -= cnt;
            _cpy(&_buf[0], &((const T *) src)[cnt], sz);
            cnt += sz;
            w = sz;
            return cnt;
        }
        // just one free block is available as (w < r)
        _cpy(&_buf[w], src, sz);
        w += sz;
        return sz;
    }
};

// FIFO with static allocation bufffer base template.
template<const size_t SIZE, typename T, typename F>
class fifo_staticT : public F
{
public:
    fifo_staticT()
        : F(_buffer, SIZE)
    {}

private:
    T _buffer[SIZE];
};

using fifo = fifoT<uint8_t>;

// Statically allocated FIFO buffer shortcut
template<const size_t SIZE, typename T = uint8_t>
using fifo_static = fifo_staticT<SIZE, T, fifoT<T>>;

// FIFO data packets support:
//   Each packet is prepended with size value (word) of elements in a packet.
//   Packet size is limited to 16 bit.
template<typename T = uint8_t>
class fifo_packet : public fifoT<T>
{
public:
    fifo_packet(T *buf, size_t size)
        : fifoT<T>(buf, size)
    {}

    // write packet to buffer
    bool write_packet(const void *src, size_t sz)
    {
        size_t w = fifoT<T>::_w;
        size_t r = fifoT<T>::_r;

        // Ensure whole pkt fits in buffer with additional packet size word.
        const size_t pkt_sz = sz + 2;
        const size_t fcnt = fifoT<T>::_free(r, w);
        if (pkt_sz > fcnt)
            return false;

        // write whole packet, then modify write pointer
        // to support concurrent reads
        fifoT<T>::_write(&w, r, &sz, 2);  // Write packet size
        fifoT<T>::_write(&w, r, src, sz); // Write packet data
        fifoT<T>::_w = w;                 // confirm write
        return true;
    }

    // read packet from buffer and release space
    size_t read_packet(void *dest, size_t sz)
    {
        size_t w = fifoT<T>::_w;
        size_t r = fifoT<T>::_r;

        size_t used_cnt = fifoT<T>::_used(r, w);
        if (used_cnt < 2)
            return 0;

        used_cnt -= 2; // remove packet size word from length count

        size_t cnt;
        fifoT<T>::read(&cnt, 2);
        if (cnt < 1 || cnt > used_cnt) {
            // packet corrupted
            fifoT<T>::reset(); // try to reset buffer (at least one packet lost)
            return 0;
        }
        if (cnt > sz) {
            // no space for packet
            fifoT<T>::skip_read(cnt);
            return 0;
        }
        return fifoT<T>::read(dest, cnt);
    }
};
template<const size_t SIZE, typename T = uint8_t>
using fifo_packet_static = fifo_staticT<SIZE, T, fifo_packet<T>>;

} // namespace apx
