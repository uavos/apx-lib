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

#include <common/do_not_copy.h>
#include <cstdint>
#include <cstring>

class QueueBufferBase : public do_not_copy
{
public:
    explicit QueueBufferBase(size_t size)
        : _total(size)
    {}

    inline size_t size() const { return _size; }
    inline size_t head() const { return _head; }
    inline size_t tail() const { return _tail; }
    inline size_t total() const { return _total; }
    inline size_t space() const { return _total - _size; }
    inline bool empty() const { return _size == 0; }

    inline void reset() { _head = _tail = _size = 0; }

    virtual size_t write(const void *src, size_t sz) = 0;
    virtual size_t read(void *dest, size_t sz) = 0;

    //packet support
    virtual size_t write_packet(const void *src, size_t sz) = 0;
    virtual size_t read_packet(void *dest, size_t sz) = 0;

    size_t write_word(uint16_t cnt)
    {
        if (space() < 2)
            return 0;
        return write(&cnt, 2);
    }

    // cancel write
    void pop_head(size_t pos)
    {
        if (pos == _head && _size == _total) {
            reset();
            return;
        }
        size_t sz = pos <= _head ? _head - pos : _total - pos + _head;
        _head = pos;
        _size -= sz;
    }
    bool push_head(size_t pos)
    {
        size_t sz = pos >= _head ? pos - _head : _total - _head + pos;
        if (space() < sz)
            return false;
        _head = pos;
        _size += sz;
        return false;
    }

    // read adjust
    void skip_read(size_t sz)
    {
        advance(&_tail, sz);
        _size -= sz;
    }
    void advance(size_t *v, size_t sz) const
    {
        *v += sz;
        if (*v >= _total)
            *v -= _total;
    }

protected:
    size_t _head{0};
    size_t _tail{0};
    size_t _size{0};
    const size_t _total;
};

template<typename T = uint8_t>
class QueueBufferT : public QueueBufferBase
{
public:
    explicit QueueBufferT(T *_buf, size_t size)
        : QueueBufferBase(size)
        , buf(_buf)
    {}

    inline const T *read_ptr() const { return buf + _tail; }
    inline T *write_ptr() const { return buf + _head; }

    size_t write(const void *src, size_t sz) override
    {
        if (space() < sz)
            return 0;
        if (sz == 1) {
            buf[_head] = *static_cast<const T *>(src);
        } else {
            size_t cnt1 = _total - _head;
            if (cnt1 > sz)
                cnt1 = sz;
            memcpy(buf + _head, static_cast<const T *>(src), cnt1 * sizeof(T));
            size_t cnt2 = sz - cnt1;
            if (cnt2 > 0)
                memcpy(buf, static_cast<const T *>(src) + cnt1, cnt2 * sizeof(T));
        }
        advance(&_head, sz);
        _size += sz;
        return sz;
    }

    size_t read(void *dest, size_t sz) override
    {
        if (_size == 0)
            return 0;
        if (sz > _size)
            sz = _size;
        if (sz == 1) {
            *static_cast<T *>(dest) = buf[_tail];
        } else {
            size_t cnt1 = _total - _tail;
            if (cnt1 > sz)
                cnt1 = sz;
            memcpy(static_cast<T *>(dest), read_ptr(), cnt1 * sizeof(T));
            size_t cnt2 = sz - cnt1;
            if (cnt2 > 0)
                memcpy(static_cast<T *>(dest) + cnt1, buf, cnt2 * sizeof(T));
        }
        skip_read(sz);
        return sz;
    }

    //packet support
    size_t write_packet(const void *src, size_t sz) override
    {
        if (space() < (sz + 2))
            return 0;
        write_word(sz);
        write(src, sz);
        return sz;
    }

    size_t read_packet(void *dest, size_t sz) override
    {
        if (empty())
            return 0;
        do {
            if (_size < 3)
                break;
            uint16_t cnt;
            if (read(&cnt, 2) != 2)
                break;
            if (_size < cnt || sz < cnt)
                break;
            return read(dest, cnt);
        } while (0);
        reset();
        return 0;
    }

    size_t read_packet(const uint8_t **dest, size_t *sz)
    {
        if (empty())
            return 0;
        do {
            if (_size < 3)
                break;
            uint16_t cnt;
            if (read(&cnt, 2) != 2)
                break;
            if (_size < cnt)
                break;
            *dest = read_ptr();
            *sz = cnt;
            skip_read(cnt);
            return cnt;
        } while (0);
        reset();
        return 0;
    }

    // specific types
    bool write(const T &v)
    {
        return write(&v, 1);
    }

    const T &read_one()
    {
        size_t i = _tail;
        skip_read(1);
        return buf[i];
    }

    // LIFO
    const T &pop_one()
    {
        _head = _head > 0 ? _head - 1 : _total - 1;
        _size--;
        return buf[_head];
    }

protected:
    T *buf;
};

template<size_t _total, typename T = uint8_t>
class QueueBuffer : public QueueBufferT<T>
{
public:
    explicit QueueBuffer()
        : QueueBufferT<T>(m_buf, _total)
    {}

private:
    T m_buf[_total];
};
