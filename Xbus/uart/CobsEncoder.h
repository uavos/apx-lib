#pragma once

#include <containers/QueueBuffer.h>

#include "SerialCodec.h"

template<size_t _buf_size, typename T = uint8_t, T _esc = 0>
class CobsEncoder : private QueueBuffer<_buf_size, T>, public SerialEncoder
{
public:
    using QueueBuffer<_buf_size, T>::head;
    using QueueBuffer<_buf_size, T>::tail;
    using QueueBuffer<_buf_size, T>::size;
    using QueueBuffer<_buf_size, T>::total;
    using QueueBuffer<_buf_size, T>::read_ptr;
    using QueueBuffer<_buf_size, T>::skip_read;

    //write and encode data to fifo
    size_t encode(const void *src, size_t sz) override
    {
        if (space() < (sz + 1 + 2)) //estimate
            return 0;

        size_t head_s = head();
        size_t size_s = size();
        do {
            // start block
            T *code_ptr = write_ptr();
            T code = 1;
            if (!write(code))
                break;

            // data
            const T *ptr = static_cast<const T *>(src);
            const T *end = ptr + sz;
            uint8_t crc = 0;
            while (ptr <= end) {
                if (code != 0xFF) {
                    T c;
                    if (ptr == end) {
                        c = crc;
                        ptr++;
                    } else {
                        c = *ptr++;
                        crc ^= c;
                    }
                    if (c != _esc) {
                        if (!write(c))
                            break;
                        code++;
                        continue;
                    }
                }
                // finish block
                *code_ptr = code;
                // start block
                code_ptr = write_ptr();
                code = 1;
                if (!write(code))
                    break;
            }
            if (ptr <= end) //error
                break;

            // finish block
            *code_ptr = code;

            //append packet delimiter
            if (!write(_esc))
                break;

            // all encoded
            return size() - size_s;
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
    using QueueBuffer<_buf_size, T>::write_ptr;
    using QueueBuffer<_buf_size, T>::read;
};
