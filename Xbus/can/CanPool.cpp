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

using namespace xbus::can;

Pool::Pool()
{
    init();
}
void Pool::init()
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

ErrorType Pool::push(const extid_s &extid, const uint8_t *data, uint8_t dlc)
{
    //debug("%X %d %d", mid, seq_idx, dlc);
    // find free item
    if (free == max_idx) {
        // pool overflow
        timeout();
        remove(extid);
        return ErrorSlotsOverflow;
    }
    frm_e frm = static_cast<frm_e>(extid.frm);
    switch (frm) {
    case frm_single:
        return push_new(extid, data, dlc > 0 ? dlc : 0x0F);
    case frm_seq0:
    case frm_seq1:
        if (dlc != 8)
            return ErrorDLC; //error - size<8, but multipart middle msg
        return push_next(extid, data, dlc);
    case frm_end:
        if (dlc == 0)
            return ErrorDLC; //error - size=0, but multipart end msg
        else {
            ErrorType rv = push_next(extid, data, dlc);
            if (rv != MsgAccepted)
                return rv;
            timeout();
            return PacketAvailable;
        }
    }
    return MsgDropped;
}

ErrorType Pool::push_new(const extid_s &extid, const uint8_t *data, uint8_t dlc)
{
    // start new tree
    remove(extid);
    // find empty tree
    for (auto &t : trees) {
        if (t.head != max_idx)
            continue;
        t.extid.raw = extid.raw;
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
ErrorType Pool::push_next(const extid_s &extid, const uint8_t *data, uint8_t dlc)
{
    // find existing tree and append message
    for (auto &t : trees) {
        if (t.head == max_idx)
            continue;
        if (t.dlc)
            continue;
        if ((t.extid.raw ^ extid.raw) & mf_id_mask)
            continue;
        // tree found - check sequence
        if (t.extid.frm == extid.frm)
            return MsgDropped; //repeated
        // append msg to tree
        uint8_t next = t.head;
        uint8_t cnt = 0;
        while (cnt < max_seq_idx) {
            Item &i = items[next];
            //debug("%d %d", t.head, i.next);
            cnt++;
            if (i.next == max_idx) { // tail
                t.to = XCAN_CODEC_TIMEOUT;
                t.dlc = dlc;
                t.extid.frm = extid.frm; // seqX check
                i.next = free;
                //debug("next %X (%d)", mid, i.next);
                push(data);
                return MsgAccepted;
            }
            next = i.next;
        }
        // stream error
        //debug("err %X %d", mid, seq_idx);
        remove(extid);
        return ErrorSeqIdx;
    }
    // mid part received but no tree is working - drop msg
    remove(extid);
    timeout();
    return ErrorOrphan;
}

void Pool::push(const uint8_t *data)
{
    Item &i = items[free];
    //debug("%d %d", free, i.next);
    free = i.next;
    i.next = max_idx;
    memcpy(i.data, data, 8);
}

void Pool::remove(const extid_s &extid)
{
    for (auto &t : trees) {
        if (t.head == max_idx) // free slot
            continue;
        if (t.dlc) // finished msg
            continue;
        if ((t.extid.raw ^ extid.raw) & mf_id_mask)
            continue;
        remove(t);
    }
}
void Pool::remove(Tree &t)
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
void Pool::report_status()
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
size_t Pool::space() const
{
    size_t cnt = 0;
    for (uint8_t i = free; i != max_idx; i = items[i].next)
        cnt++;
    return cnt;
}

size_t Pool::read_packet(void *dest, size_t sz, uint8_t *src_id)
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
size_t Pool::read_packet(Tree &t, void *dest, size_t sz, uint8_t *src_id)
{
    *src_id = t.extid.src;

    XbusStreamWriter stream(dest, sz);
    stream << t.extid.pid;

    for (uint8_t next = t.head;;) {
        Item &i = items[next];
        next = i.next;
        if (next != max_idx) {
            if ((stream.pos() + 8u) > sz)
                break;
            stream.write(i.data, 8);
            continue;
        }
        //final
        uint8_t dlc = t.dlc;
        if (dlc <= 8) {
            if ((stream.pos() + dlc) > sz)
                break;
            stream.write(i.data, dlc);
        }
        break;
    }
    remove(t); //release buffers
    return stream.pos();
}
bool Pool::timeout()
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
