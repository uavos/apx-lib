#pragma once

#include <cstdint>
#include <cstring>

template<size_t _buf_size, typename T = uint8_t>
class QueueBufferT
{
public:
    explicit QueueBufferT(T *_buf)
        : buf(_buf)
    {}

    virtual void reset() { _head = _tail = _size = 0; }

    inline size_t size() const { return _size; }
    inline size_t head() const { return _head; }
    inline size_t tail() const { return _tail; }
    inline size_t total() const { return _buf_size; }
    inline size_t space() const { return _buf_size - _size; }

    inline bool empty() const { return _size == 0; }
    inline const T *read_ptr() const { return buf + _tail; }

    size_t write(const T &v)
    {
        return write(&v, sizeof(T));
    }

    virtual size_t write(const void *src, size_t sz)
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
        advance(_head, sz);
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
            memcpy(static_cast<T *>(dest), read_ptr(), cnt1 * sizeof(T));
            size_t cnt2 = sz - cnt1;
            if (cnt2 > 0)
                memcpy(static_cast<T *>(dest) + cnt1, buf, cnt2 * sizeof(T));
        }
        skip_read(sz);
        return sz;
    }

    //packet support
    virtual size_t write_packet(const void *src, size_t sz)
    {
        if (space() < (sz + 2))
            return 0;
        write_packet_cnt(sz);
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

    // DMA fifo support
    bool push_head(size_t pos)
    {
        size_t sz = pos >= _head ? pos - _head : _buf_size - _head + pos;
        if (space() < sz)
            return false;
        _head = pos;
        _size += sz;
        return false;
    }

    // cancel write
    void pop_head(size_t pos)
    {
        if (pos == _head && _size == _buf_size) {
            reset();
            return;
        }
        size_t sz = pos <= _head ? _head - pos : _buf_size - pos + _head;
        _head = pos;
        _size -= sz;
    }

    // LIFO
    const T &pop_one()
    {
        _head = _head > 0 ? _head - 1 : _buf_size - 1;
        _size--;
        return buf[_head];
    }

    // read adjust
    void skip_read(size_t sz)
    {
        advance(_tail, sz);
        _size -= sz;
    }

protected:
    T *buf;

    size_t _head{0};
    size_t _tail{0};
    size_t _size{0};

    void advance(size_t &v, size_t sz) const
    {
        v += sz;
        if (v >= _buf_size)
            v -= _buf_size;
    }

    size_t write_packet_cnt(uint16_t cnt)
    {
        if (space() < 2)
            return 0;
        return write(&cnt, 2);
    }
};

template<size_t _buf_size, typename T = uint8_t>
class QueueBuffer : public QueueBufferT<_buf_size, T>
{
public:
    explicit QueueBuffer()
        : QueueBufferT<_buf_size, T>(m_buf)
    {}

private:
    T m_buf[_buf_size];
};
