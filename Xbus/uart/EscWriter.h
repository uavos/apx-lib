#pragma once

#include <containers/QueueBuffer.h>

template<size_t _buf_size, typename T = uint8_t>
class EscWriter : private QueueBuffer<_buf_size, T>
{
public:
    using QueueBuffer<_buf_size, T>::reset;
    using QueueBuffer<_buf_size, T>::read;
    using QueueBuffer<_buf_size, T>::size;
    using QueueBuffer<_buf_size, T>::empty;
    using QueueBuffer<_buf_size, T>::total;
    using QueueBuffer<_buf_size, T>::tail;
    using QueueBuffer<_buf_size, T>::read_ptr;
    using QueueBuffer<_buf_size, T>::skip_read;

    //write and encode data to fifo
    size_t encode(const void *src, size_t sz)
    {
        if (space() < (sz + 2 + 2 + 1)) //estimate
            return 0;
        size_t head_s = head();
        do {
            uint8_t crc = 0;
            if (!write(0x55))
                break;
            if (!write(0x01))
                break;

            const T *buf = static_cast<const T *>(src);
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

private:
    using QueueBuffer<_buf_size, T>::space;
    using QueueBuffer<_buf_size, T>::head;
    using QueueBuffer<_buf_size, T>::pop_head;
    using QueueBuffer<_buf_size, T>::write;
};
