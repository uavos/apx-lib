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

#include "CanCodec.h"
#include "CanFormat.h"

#include <cstring>

#define XCAN_CODEC_DEBUG

#ifdef XCAN_CODEC_DEBUG
#include <platform/log.h>
#define debug(...) apxdebug(__VA_ARGS__)
#define MODULE_NAME "xcan"
#else
#define debug(...)
#endif

#define XCAN_CODEC_TIMEOUT 7 // number of valid packets to remove old orphans

using namespace xbus::can;

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
    //debug("%d %d", extid.frm, dlc);
    // find free item
    if (free == max_idx) {
        // pool overflow
        timeout();
        remove(extid);
        return ErrorSlotsOverflow;
    }
    ErrorType rv = MsgDropped;
    switch (extid.frm) {
    case frm_end:
        if (dlc == 0)
            return ErrorDLC; //error - size=0, but multipart end msg
        rv = push_next(extid, data, dlc);
        if (rv != MsgAccepted)
            break;
        timeout();
        return PacketAvailable;

    case frm_start:
        if (dlc != 8)
            return ErrorDLC; //error - size<8, but multipart msg
        return push_new(extid, data, 0);
    case frm_single:
        return push_new(extid, data, dlc > 0 ? dlc : 0x0F);
    default: // sequence
        if (dlc != 8)
            return ErrorDLC; //error - size<8, but multipart middle msg
        return push_next(extid, data, 0);
    }
    return rv;
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
        if (dlc)
            return PacketAvailable;
        t.extid.frm = 0;
        return MsgAccepted;
    }
    // no empty tree available
    timeout();
    return ErrorTreeOverflow;
}

ErrorType Pool::push_next(const extid_s &extid, const uint8_t *data, uint8_t dlc)
{
    // find existing tree and append message
    ErrorType rv = ErrorOrphan;
    for (auto &t : trees) {
        if (t.head == max_idx)
            continue;
        if (t.dlc)
            continue;
        if ((t.extid.raw ^ extid.raw) & mf_id_mask)
            continue;
        // tree found - check sequence
        uint8_t t_frm;
        if (dlc == 0) {
            t_frm = t.extid.frm;
            uint8_t frm = extid.frm;
            if (frm != t_frm) {
                //drop repeated msgs
                t_frm = t_frm > 0 ? t_frm - 1 : frm_seq_max;
                if (frm != t_frm) {
                    debug("frm %d %d", t.extid.frm, extid.frm);
                    rv = ErrorOrphan;
                    break; //orphan
                }
                return MsgDropped; //repeated
            }
        }
        rv = push_next(t, data);
        if (rv != MsgAccepted)
            break;

        if (dlc) {
            t.dlc = dlc;
            return PacketAvailable;
        }

        t_frm++; // seqX check
        if (t_frm > frm_seq_max)
            t.extid.frm = 0;
        else
            t.extid.frm = t_frm;
        return MsgAccepted;
    }
    // part received but no tree is working
    debug("orph %d", extid.frm);
    remove(extid);
    timeout();
    return rv;
}

ErrorType Pool::push_next(Tree &t, const uint8_t *data)
{
    // append msg to tree
    uint8_t next = t.head;
    uint8_t cnt = 0;
    while (cnt < max_seq_idx) {
        Item &i = items[next];
        //debug("%d %d", t.head, i.next);
        cnt++;
        if (i.next == max_idx) { // tail
            t.to = XCAN_CODEC_TIMEOUT;
            i.next = free;
            //debug("next %X (%d)", mid, i.next);
            push(data);
            return MsgAccepted;
        }
        next = i.next;
    }
    // stream error
    debug("tail %u %X", cnt, t.extid);
    return ErrorSeqIdx;
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
        *src_id = t.extid.src;
        size_t cnt = read_packet(t, dest, sz);
        if (cnt > 0)
            return cnt;
    }
    return 0;
}
size_t Pool::read_packet(Tree &t, void *dest, size_t sz)
{
    XbusStreamWriter stream(dest, sz);

    stream.write<xbus::pid_raw_t>(t.extid.pid);

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
            debug("%X", t.extid);
            remove(t);
            rv = true;
            continue;
        }
        t.to--;
    }
    return rv;
}
