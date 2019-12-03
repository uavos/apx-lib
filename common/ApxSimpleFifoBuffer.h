#pragma once

#include <cstdint>
#include <string.h>

//==============================================================================
template<const uint16_t fifo_size, class T = uint8_t>
class ApxSimpleFifoBuffer
{
public:
    ApxSimpleFifoBuffer()
        : pos_write(0)
        , pos_read(0)
        , load_cnt(0)
        , load_cnt_save(0)
    {}
    //============================================================================
    inline uint16_t size() { return fifo_size; }
    inline uint16_t fifo_cnt()
    {
        return load_cnt; //pos_read<=pos_write?pos_write-pos_read:fifo_size-pos_read+pos_write;
    }
    //============================================================================
    inline void clear()
    {
        pos_write = pos_read = load_cnt = load_cnt_save = 0;
        memset(pool, 0, sizeof(T));
        memset(pool + 1, 0, sizeof(T));
    }
    //============================================================================
    inline bool is_empty()
    {
        return !fifo_cnt(); //pos_read==pos_write;
    }
    //============================================================================
    inline T *element(const uint16_t i)
    {
        if (is_empty())
            return nullptr;
        return pool + (pos_read + i) % fifo_size;
    }
    //============================================================================
    inline void update_write(uint16_t pos) //update write pos from ext writter (DMA)
    {
        pos_write = pos;
        load_cnt = pos_read <= pos_write ? pos_write - pos_read : fifo_size - pos_read + pos_write;
        load_cnt += load_cnt_save;
    }
    //============================================================================
    bool write(const T *buf, uint32_t cnt) //copy buffer to fifo
    {
        //check if FIFO available.
        if ((size() - fifo_cnt()) < (int) cnt)
            return false;
        //copy buf to FIFO
        if (cnt == 1 && sizeof(T) == 1) { //speedup
            pool[pos_write] = buf[0];
        } else {
            uint32_t cnt_left = fifo_size - pos_write;
            if (cnt_left > cnt)
                cnt_left = cnt;
            memcpy(pool + pos_write, buf, cnt_left * sizeof(T));
            memcpy(pool, buf + cnt_left, (cnt - cnt_left) * sizeof(T));
        }
        //set next iterator
        pos_write = (pos_write + cnt) % fifo_size;
        load_cnt += cnt;
        return true;
    }
    //============================================================================
    bool write_one(const T v) //copy byte to fifo
    {
        if (size() == fifo_cnt())
            return false;
        pool[pos_write++] = v;
        if (pos_write >= fifo_size)
            pos_write = 0;
        load_cnt++;
        return true;
    }
    //============================================================================
    //read pointer from fifo
    //don't release elements until next read
    //return number of elements
    uint32_t read(T **ptr)
    {
        //release fifo from previous read
        //pos_read=pos_read_save;
        load_cnt -= load_cnt_save;
        load_cnt_save = 0;
        //continue
        if (is_empty()) { //nothing to send
            return 0;
        }
        //data to send is waiting
        *ptr = pool + pos_read;
        uint16_t cnt = load_cnt;
        uint16_t max_cnt = fifo_size - pos_read;
        if (cnt > max_cnt)
            cnt = max_cnt;
        pos_read = (pos_read + cnt) % fifo_size;
        load_cnt_save = cnt;
        return cnt;
    }
    //============================================================================
    //read one element to buffer, release element
    bool read_one(T *buf)
    {
        T *p;
        uint16_t pos_s = pos_read;
        uint32_t cnt = read(&p);
        if (!cnt)
            return false;
        if (sizeof(T) == 1)
            *buf = *p;
        else
            memcpy(buf, p, sizeof(T));
        pos_read = pos_s + 1;
        if (pos_read >= fifo_size)
            pos_read = 0;
        ///pos_read_save=pos_read;
        load_cnt--;
        load_cnt_save = 0;
        return true;
    }
    //============================================================================
    //read elements to buffer, release elements
    uint32_t read(T *buf, uint32_t sz)
    {
        uint32_t rcnt = 0;
        while (sz) {
            T *p;
            uint32_t cnt = read(&p);
            if (!cnt)
                break;
            if (cnt > sz) {
                uint16_t d = cnt - sz;
                cnt = sz;
                pos_read = pos_read >= d ? pos_read - d : fifo_size - d;
                load_cnt_save = cnt;
            }
            memcpy(buf, p, cnt * sizeof(T));
            rcnt += cnt;
            sz -= cnt;
            buf += cnt;
            if (!sz) {
                load_cnt -= load_cnt_save;
                load_cnt_save = 0;
            }
        }
        return rcnt;
    }
    //============================================================================
    // VARIABLE LENGTH PACKETS FIFO
    //============================================================================
    //copy packet to fifo, return true on success
    bool write_packet(const uint8_t *buf, uint32_t cnt)
    {
        if (!write_packet_start(cnt))
            return false;
        write_packet_data(buf, cnt);
        write_packet_done(cnt);
        return true;
    }
    inline bool write_packet_start(uint32_t cnt)
    {
        if (!cnt)
            return false;
        // check overflow
        uint32_t pos_top = pos_write + cnt + 2;
        uint32_t pos_rt = pos_read;
        if (pool[pos_rt]) { //check only if tx not empty
            if (pos_write > pos_rt)
                pos_rt += fifo_size;
            if (pos_top >= pos_rt) {
                //node.status.can_err++;
                //LED_ON();
                return false;
            }
        }
        pos_top %= fifo_size; //pos in FIFO after packet write
        //write header
        // <cntH|0x80>,<cntL>,<data..>
        pos_write_save = pos_write;
        pos_write = (pos_write + 1) % fifo_size; //skip first byte
        pool[pos_write] = cnt;
        pos_write = (pos_write + 1) % fifo_size;
        return true;
    }
    inline void write_packet_data(const uint8_t *buf, uint32_t cnt)
    {
        uint32_t xcnt = fifo_size - pos_write;
        if (xcnt > cnt)
            xcnt = cnt;
        memcpy(&(pool[pos_write]), buf, xcnt * sizeof(T));
        uint32_t tcnt = cnt - xcnt;
        if (tcnt)
            memcpy(&(pool[0]), &(buf[xcnt]), tcnt * sizeof(T));
        pos_write = (pos_write + cnt) % fifo_size;
    }
    inline void write_packet_done(uint32_t cnt)
    {
        pool[pos_write] = 0; // stop byte for reader
        //fix first byte, allow write by INT
        pool[pos_write_save] = (cnt >> 8) | 0x80;
        load_cnt += cnt + 2;
    }
    //============================================================================
    //read packet from fifo to buffer, return packet fifo_size
    uint32_t read_packet(uint8_t *buf, uint32_t sz)
    {
        (void) sz;
        //check for new tx packet from fifo
        if (pool[pos_read] == 0)
            return 0; //fifo empty
        //fifo contains data
        uint32_t tx_p = pos_read;
        uint32_t cnt = pool[tx_p++] << 8;
        tx_p %= fifo_size;
        cnt |= pool[tx_p++];
        tx_p %= fifo_size;
        cnt &= 0x7FFF; //clear MSB (fifo flag)
        uint32_t xcnt = fifo_size - tx_p;
        if (xcnt > cnt)
            xcnt = cnt;
        memcpy(buf, &(pool[tx_p]), xcnt * sizeof(T));
        memcpy(&(buf[xcnt]), pool, (cnt - xcnt) * sizeof(T));
        pos_read = (tx_p + cnt) % fifo_size; //round inc,release fifo
        load_cnt -= cnt - 2;
        return cnt;
    }
    //============================================================================

    //data
    __attribute__((aligned(32))) T pool[fifo_size];

private:
    uint16_t pos_write, pos_read, load_cnt;
    uint16_t pos_write_save; //used for packet write
    uint16_t load_cnt_save;
};
