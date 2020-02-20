#pragma once

#include <containers/QueueBuffer.h>

#include "SerialCodec.h"

template<size_t _buf_size, typename T = uint8_t>
class EscDecoder : private QueueBuffer<_buf_size, T>, public SerialDecoder
{
public:
    using QueueBuffer<_buf_size, T>::size;
    using QueueBuffer<_buf_size, T>::empty;
    using QueueBuffer<_buf_size, T>::read_packet;

    //decode ESC encoded data and write packet to fifo
    size_t decode(const void *src, size_t sz) override
    {
        if (sz == 0)
            return 0;
        //restore fifo with partially read packet
        if (_rcnt > 0) {
            push_head(_head_pending_s);
        }

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
                if (!space())
                    break;
                if (_rcnt == 0) {
                    _head_s = head();
                    if (!write_word(0))
                        break;
                }
                _rcnt++;
                _crc += v;
                if (!write(v))
                    break;
                continue;
            case 2: //escape
                if (v == 0x02) {
                    v = 0x55;
                    _state = 1;
                    goto case_DATA;
                }
                if (v == 0x03) {
                    if (_rcnt <= 1)
                        break; //no data
                    T v_crc = pop_one();
                    _crc -= v_crc;
                    if (_crc != v_crc) {
                        break;
                    }
                    //frame received...
                    size_t h = head();
                    pop_head(_head_s);
                    write_word(_rcnt - 1);
                    push_head(h);
                    _state = 0;
                    _rcnt = 0;
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
        return sz; //always accept all bytes
    }

    inline size_t read_decoded(void *dest, size_t sz) override
    {
        return QueueBuffer<_buf_size, T>::read_packet(dest, sz);
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

    using QueueBuffer<_buf_size, T>::space;
    using QueueBuffer<_buf_size, T>::head;
    using QueueBuffer<_buf_size, T>::write;
    using QueueBuffer<_buf_size, T>::pop_head;
    using QueueBuffer<_buf_size, T>::push_head;
    using QueueBuffer<_buf_size, T>::write_word;
    using QueueBuffer<_buf_size, T>::pop_one;
};
