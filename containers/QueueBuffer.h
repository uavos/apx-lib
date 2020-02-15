#pragma once

#include <cstdint>
#include <cstring>

template<size_t _buf_size, class T = uint8_t>
class QueueBuffer
{
public:
    inline size_t size() { return _size; }
    inline bool empty() { return _size == 0; }
    inline size_t space() { return _buf_size - _size; }

    size_t write(const void *src, size_t sz)
    {
        if (space() < sz)
            return 0;
        if (sz == 1) {
            buf[_head] = *static_cast<const T *>(src);
        } else {
            size_t cnt1 = _buf_size - _head;
            if (cnt1 > sz)
                cnt1 = sz;
            memcpy(buf + _head, static_cast<const T *>(src), cnt1 * sizeof(T));
            size_t cnt2 = sz - cnt1;
            if (cnt2 > 0)
                memcpy(buf, static_cast<const T *>(src) + cnt1, cnt2 * sizeof(T));
        }
        seek(_head, sz);
        _size += sz;
        return sz;
    }

    size_t read(void *dest, size_t sz)
    {
        if (_size == 0)
            return 0;
        if (sz > _size)
            sz = _size;
        if (sz == 1) {
            *static_cast<T *>(dest) = buf[_tail];
        } else {
            size_t cnt1 = _buf_size - _tail;
            if (cnt1 > sz)
                cnt1 = sz;
            memcpy(static_cast<T *>(dest), buf + _tail, cnt1 * sizeof(T));
            size_t cnt2 = sz - cnt1;
            if (cnt2 > 0)
                memcpy(static_cast<T *>(dest) + cnt1, buf, cnt2 * sizeof(T));
        }
        seek(_tail, sz);
        _size -= sz;
        return sz;
    }

    void reset()
    {
        _head = _tail = _size = 0;
    }

    //packet support
    size_t write_packet(const void *src, size_t sz)
    {
        if (space() < (sz + 2))
            return 0;
        uint16_t cnt = sz;
        write(&cnt, 2);
        write(src, sz);
        return sz;
    }

    size_t read_packet(void *dest, size_t sz)
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

private:
    T buf[_buf_size];

    size_t _head{0};
    size_t _tail{0};
    size_t _size{0};

    void seek(size_t &v, size_t sz) const
    {
        v += sz;
        if (v >= _buf_size)
            v -= _buf_size;
    }
};
