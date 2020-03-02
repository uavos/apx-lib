/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#include "CanCodec.h"
#include "CanFormat.h"

#include <cstring>

#ifdef XCAN_CODEC_DEBUG
#include <platform/log.h>
#define debug(...) apxdebug(__VA_ARGS__)
#define MODULE_NAME "cc"
#else
#define debug(...)
#endif

#define XCAN_CODEC_TIMEOUT 7 // number of valid packets to remove old orphans

using namespace xbus;

size_t CanCodec::read_packet(void *dest, size_t sz, uint8_t *src_id)
{
    return pool.read_packet(dest, sz, src_id);
}

CanCodec::ErrorType CanCodec::push_message(const xcan_msg_t &msg)
{
    do {
        uint32_t extid = msg.hdr.id;
        if (extid & XCAN_NAD_MASK) // addressing packet
            break;

        const pool_id_t mid = extid_to_mid(extid); //src_address|var_idx
        const uint8_t src = mid_to_src(mid);

        if (src == 0) {
            sendAddressing();
            break;
        }
        if (src == nodeId()) {
            updateNodeId();
            sendAddressing();
            break;
        }
        //process message
        uint16_t seq_idx = (extid & XCAN_CNT_MASK) >> XCAN_CNT_SHIFT;
        uint8_t dlc = msg.hdr.dlc;

        if (!(extid & XCAN_END_MASK)) {
            // msg part received
            if (dlc != 8) {
                return ErrorDLC; //error - size<8, but multipart middle msg
            }
            return pool.push(mid, seq_idx, msg.data, 0);
        }
        // whole packet received
        ErrorType rv = pool.push(mid, seq_idx, msg.data, dlc ? dlc : 0xF);
        if (rv != MsgAccepted)
            return rv;
        pool.timeout();
        return PacketAvailable;
    } while (0);
    return MsgDropped;
}

CanCodec::pool_id_t CanCodec::extid_to_mid(const uint32_t extid)
{
    return extid >> XCAN_PID_SHIFT; //src_address|var_idx
}
xbus::pid8_t CanCodec::mid_to_pid(const CanCodec::pool_id_t mid)
{
    return mid & (XCAN_PID_MASK >> XCAN_PID_SHIFT);
}
xbus::pid8_t CanCodec::mid_to_src(const CanCodec::pool_id_t mid)
{
    return (mid & (XCAN_SRC_MASK >> XCAN_PID_SHIFT)) >> (XCAN_SRC_SHIFT - XCAN_PID_SHIFT);
}
xbus::pid8_t CanCodec::extid_to_pid(const uint32_t extid)
{
    return mid_to_pid(extid_to_mid(extid));
}

void CanCodec::sendAddressing()
{
    debug("%X", nodeId());
    txmsg.hdr.raw = 0;
    txmsg.hdr.ext = 1;
    txmsg.hdr.id = XCAN_SRC(nodeId()) | XCAN_PRI_MASK | XCAN_END_MASK | XCAN_NAD_MASK;
    send_message(txmsg);
}

// Pool

CanCodec::Pool::Pool()
{
    init();
}
void CanCodec::Pool::init()
{
    // clear pool
    memset(trees, 0, sizeof(trees));
    memset(items, 0, sizeof(items));
    // all trees are available
    for (auto &t : trees) {
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

CanCodec::ErrorType CanCodec::Pool::push(pool_id_t mid, size_t seq_idx, const uint8_t *data, uint8_t dlc)
{
    //debug("%X %d %d", mid, seq_idx, dlc);
    // find free item
    if (free == max_idx || seq_idx >= max_seq_idx) {
        // pool overflow
        timeout();
        remove(mid);
        return ErrorSlotsOverflow;
    }
    if (seq_idx == 0) {
        remove(mid);
        // find empty tree
        for (auto &t : trees) {
            if (t.head != max_idx)
                continue;
            t.mid = mid;
            t.to = XCAN_CODEC_TIMEOUT;
            t.dlc = dlc;
            t.head = free;
            //debug("new %X", mid);
            push(data);
            return MsgAccepted;
        }
        // no empty tree available
        timeout();
        return ErrorTreeOverflow;
    }
    // find existing tree
    for (auto &t : trees) {
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
                t.to = XCAN_CODEC_TIMEOUT;
                t.dlc = dlc;
                i.next = free;
                //debug("next %X (%d)", mid, i.next);
                push(data);
                return MsgAccepted;
            }
            next = i.next;
        }
        // stream error
        //debug("err %X %d", mid, seq_idx);
        remove(mid);
        return ErrorSeqIdx;
    }
    // mid part received but no tree is working - drop msg
    remove(mid);
    timeout();
    return ErrorOrphan;
}
void CanCodec::Pool::push(const uint8_t *data)
{
    Item &i = items[free];
    //debug("%d %d", free, i.next);
    free = i.next;
    i.next = max_idx;
    memcpy(i.data, data, 8);
}

void CanCodec::Pool::remove(pool_id_t mid)
{
    for (auto &t : trees) {
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
    // release all tree items
    while (t.head != max_idx) {
        uint8_t idx = t.head;
        Item &i = items[idx];
        t.head = i.next;
        i.next = free;
        free = idx;
    }
}
void CanCodec::Pool::report_status()
{
#ifdef XCAN_CODEC_DEBUG
    size_t tcnt = 0;
    for (auto const &t : trees) {
        if (t.head != max_idx)
            tcnt++;
    }
    size_t scnt = space();
    if (scnt < 100 || tcnt > 2) {
        debug("slots: %d/%d", space(), size_items);
        debug("trees: %d/%d", tcnt, size_trees);
    }
#endif
}
size_t CanCodec::Pool::space() const
{
    size_t cnt = 0;
    for (uint8_t i = free; i != max_idx; i = items[i].next)
        cnt++;
    return cnt;
}

size_t CanCodec::Pool::read_packet(void *dest, size_t sz, uint8_t *src_id)
{
    for (auto &t : trees) {
        if (t.head == max_idx)
            continue;
        if (!t.dlc)
            continue;
        // finished msg found in tree t
        size_t cnt = read_packet(t, dest, sz, src_id);
        if (cnt > 0)
            return cnt;
    }
    return 0;
}
size_t CanCodec::Pool::read_packet(Tree &t, void *dest, size_t sz, uint8_t *src_id)
{
    XbusStreamWriter stream(dest);
    stream << CanCodec::mid_to_pid(t.mid);
    *src_id = CanCodec::mid_to_src(t.mid);

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
    for (auto &t : trees) {
        if (t.head == max_idx)
            continue;
        if (t.dlc) // finished msg
            continue;
        if (t.to == 0) {
            debug("%X", t.mid);
            remove(t);
            rv = true;
            continue;
        }
        t.to--;
    }
    return rv;
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
