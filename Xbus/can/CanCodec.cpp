/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#include "CanCodec.h"
#include "CanFormat.h"

#include <cstring>

#ifdef CAN_CODEC_DEBUG
#include <platform/log.h>
#define debug(...) apxdebug(__VA_ARGS__)
#define MODULE_NAME "cc"
#endif

using namespace xbus;

size_t CanCodec::read_packet(void *dest, size_t sz, uint8_t *src_id)
{
    return pool.read_packet(dest, sz, src_id);
}
void CanCodec::rx_timeout_task()
{
    if (pool.timeout()) {
        rx_error();
    }
}

bool CanCodec::push_message(const CanMsg &msg)
{
    for (;;) {
        if (!msg.hdr.ext)
            break;
        uint32_t extid = msg.hdr.id;
        if (extid & XCAN_NAD_MASK)
            break;
        const uint8_t src_adr = (extid & XCAN_SRC_MASK) >> XCAN_SRC_SHIFT;
        uint8_t node_address = nodeId();
        if (src_adr == 0) {
            sendAddressing();
            break;
        }
        if (src_adr == node_address) {
            updateNodeId();
            sendAddressing();
            break;
        }
        //process message
        uint16_t mid = extid >> XCAN_PID_SHIFT; //src_address|var_idx
        uint16_t ext = extid & (XCAN_CNT_MASK | XCAN_END_MASK);
        uint16_t seq_idx = (ext & XCAN_CNT_MASK) >> XCAN_CNT_SHIFT;
        uint8_t dlc = msg.hdr.dlc;

        if (!(ext & XCAN_END_MASK)) {
            // msg part received
            if (dlc != 8) {
                rx_error();
                return true; //error - size<8, but multipart middle msg
            }
            return pool.push(mid, seq_idx, msg.data, 0);
        }
        // whole packet received
        if (pool.push(mid, seq_idx, msg.data, dlc ? dlc : 0xF)) {
            rx_done();
            return true;
        }
        rx_error();
        return true;
    }
    return false;
}

void CanCodec::sendAddressing()
{
    txmsg.hdr.raw = 0;
    txmsg.hdr.ext = 1;
    txmsg.hdr.id = XCAN_SRC(nodeId()) | XCAN_PRI_MASK | XCAN_END_MASK | XCAN_NAD_MASK;
    send_message(txmsg);
}

// Pool

CanCodec::Pool::Pool()
{
    // clear pool
    memset(tree, 0, sizeof(tree));
    memset(items, 0, sizeof(items));
    // all trees are available
    for (auto &t : tree) {
        t.head = max_idx;
    }
    // build chain of free items
    uint8_t idx = 0;
    for (auto &i : items) {
        i.next = ++idx;
    }
    items[size_items - 1].next = max_idx;
    // index of first free item
    free = 0;
}

bool CanCodec::Pool::push(mid_t mid, size_t seq_idx, const uint8_t *data, uint8_t dlc)
{
    //debug("%X %d %d", mid, seq_idx, dlc);
    // find free item
    if (free == max_idx || seq_idx >= max_seq_idx) {
        // pool overflow
        remove(mid);
        return false;
    }
    if (seq_idx == 0) {
        remove(mid);
        // find empty tree
        for (auto &t : tree) {
            if (t.head != max_idx)
                continue;
            t.mid = mid;
            t.to = 10;
            t.dlc = dlc;
            t.head = free;
            //debug("new %X", mid);
            push(data);
            return true;
        }
        // no empty tree available
        return false;
    }
    // find existing tree
    for (auto &t : tree) {
        if (t.head == max_idx)
            continue;
        if (t.mid != mid)
            continue;
        if (t.dlc)
            continue;
        // append msg to tree
        uint8_t next = t.head;
        while (seq_idx > 0) {
            Item &i = items[next];
            //debug("%d %d", t.head, i.next);
            seq_idx--;
            if (i.next == max_idx) {
                if (seq_idx != 0)
                    break; // gap between tail and msg
                t.to = 10;
                t.dlc = dlc;
                i.next = free;
                //debug("next %X (%d)", mid, i.next);
                push(data);
                return true;
            }
            next = i.next;
        }
        // stream error
        //debug("err %X %d", mid, seq_idx);
        remove(mid);
        return false;
    }
    // mid part received but no tree is working - drop msg
    return false;
}
void CanCodec::Pool::push(const uint8_t *data)
{
    Item &i = items[free];
    //debug("%d %d", free, i.next);
    free = i.next;
    i.next = max_idx;
    memcpy(i.data, data, 8);
}

void CanCodec::Pool::remove(mid_t mid)
{
    for (auto &t : tree) {
        if (t.head == max_idx)
            continue;
        if (t.mid != mid)
            continue;
        if (t.dlc) // finished msg
            continue;
        remove(t);
    }
}
void CanCodec::Pool::remove(Tree &t)
{
    //debug("space: %d/%d", space(), size_items);
    // iterate and release all tree items
    //debug("%X", t.mid);
    while (t.head != max_idx) {
        uint8_t idx = t.head;
        Item &i = items[idx];
        //debug("%d %d %d", idx, i.next, free);
        t.head = i.next;
        i.next = free;
        free = idx;
    }
    //debug("space: %d/%d", space(), size_items);
}

size_t CanCodec::Pool::read_packet(void *dest, size_t sz, uint8_t *src_id)
{
    for (auto &t : tree) {
        if (t.head == max_idx)
            continue;
        if (!t.dlc)
            continue;
        // finished msg found in tree t
        size_t cnt = read_packet(t, dest, sz, src_id);
        if (cnt > 0)
            return cnt;
    }
    return false;
}
size_t CanCodec::Pool::read_packet(Tree &t, void *dest, size_t sz, uint8_t *src_id)
{
    XbusStreamWriter stream(dest);
    stream.write<xbus::pid8_t>(t.mid & (XCAN_PID_MASK >> XCAN_PID_SHIFT));
    *src_id = t.mid & (XCAN_SRC_MASK >> XCAN_PID_SHIFT);

    for (uint8_t next = t.head;;) {
        Item &i = items[next];
        next = i.next;
        if (next != max_idx) {
            if ((stream.position() + 8u) > sz)
                break;
            stream.write(i.data, 8);
            continue;
        }
        //final
        uint8_t dlc = t.dlc;
        if (dlc <= 8) {
            if ((stream.position() + dlc) > sz)
                break;
            stream.write(i.data, dlc);
        }
        break;
    }
    remove(t); //release buffers
    return stream.position();
}
bool CanCodec::Pool::timeout()
{
    bool rv = false;
    for (auto &t : tree) {
        if (t.head == max_idx)
            continue;
        if (t.dlc) // finished msg
            continue;
        if (t.to == 0) {
            remove(t);
            rv = true;
            continue;
        }
        t.to--;
    }
    return rv;
}
size_t CanCodec::Pool::space() const
{
    size_t cnt = 0;
    for (uint8_t i = free; i != max_idx; i = items[i].next)
        cnt++;
    return cnt;
}

bool CanCodec::send_packet(uint8_t src_addr, const void *data, size_t size)
{
    if (size < sizeof(xbus::pid8_t))
        return true;
    XbusStreamReader stream(data, size);
    xbus::pid8_t pid;
    stream >> pid;

    uint32_t extid = XCAN_SRC(src_addr) | XCAN_PID(pid) | XCAN_PRI_MASK;
    size = stream.tail();

    txmsg.hdr.raw = 0;
    txmsg.hdr.ext = 1;

    if (size <= 8) {
        txmsg.hdr.id = extid | XCAN_END_MASK;
        txmsg.hdr.dlc = size;
        if (size > 0)
            memcpy(txmsg.data, stream.data(), size);
        return send_message(txmsg);
    }

    //multi-frame
    const uint8_t *src = static_cast<const uint8_t *>(stream.data());
    uint32_t ext = 0;
    while (size > 0) {
        uint8_t dlc;
        if (size > 8) {
            dlc = 8;
        } else {
            dlc = size;
            ext |= XCAN_END_MASK;
        }
        txmsg.hdr.id = extid | ext;
        txmsg.hdr.dlc = dlc;
        if (dlc > 0)
            memcpy(txmsg.data, src, dlc);
        if (!send_message(txmsg))
            return false;
        size -= dlc;
        src += dlc;
        ext++;
    }
    return true;
}
