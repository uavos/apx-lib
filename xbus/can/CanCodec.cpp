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

#include <crc.h>
#include <cstring>

#define XCAN_CODEC_DEBUG

#ifdef XCAN_CODEC_DEBUG
#include <platform/log.h>
#define debug(...) apxdebug(__VA_ARGS__)

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif

#define MODULE_NAME "xcan"
#else
#define debug(...)
#endif

using namespace xbus::can;

Codec::Codec()
{
    _extid_defaults.raw = 0;
}

size_t Codec::read_packet(void *dest, size_t sz, uint8_t *src_id)
{
    for (;;) {
        lock_push(true);
        size_t cnt = pool.read_packet(dest, sz, src_id);
        lock_push(false);
        if (cnt <= (sizeof(xbus::pid_raw_t) + 8))
            return cnt;
        cnt = check_crc(dest, cnt);
        if (cnt)
            return cnt;
    }
}
size_t Codec::check_crc(void *dest, size_t sz)
{
    sz -= sizeof(uint16_t);
    const uint8_t *p = static_cast<const uint8_t *>(dest) + sz;
    uint16_t packet_crc16 = p[0] | (p[1] << 8);
    uint16_t crc16 = apx::crc32(dest, sz);
    if (packet_crc16 != crc16) {
        debug("err:crc: %X (%X) %d", packet_crc16, crc16, sz);
        return 0;
    }
    return sz;
}

ErrorType Codec::push_message(const xcan_msg_s &msg)
{
    do {
        extid_s extid;
        extid.raw = msg.id;

        if (extid.src == _extid_defaults.src) {
            updateNodeId();
            sendAddressing();
            break;
        }

        if (!accept_filter(extid))
            return MsgAccepted;

        //process message
        return pool.push(extid, msg.data, msg.dlc);

    } while (0);
    return MsgDropped;
}

void Codec::sendAddressing()
{
    debug("%X", _extid_defaults.src);
    extid_s extid;
    extid.raw = _extid_defaults.raw;
    extid.pid = 0xFFFF;
    extid.frm = frm_single;
    txmsg.id = extid.raw;
    txmsg.ext = 1;
    txmsg.dlc = 0;
    send_message(txmsg);
}

bool Codec::send_packet(const void *data, size_t size)
{
    if (size < xbus::pid_s::psize())
        return true;

    XbusStreamReader stream(data, size);

    extid_s extid;
    extid.raw = _extid_defaults.raw;

    xbus::pid_s pid;
    pid.read(&stream);
    extid.pid = pid.raw();

    size_t cnt = stream.available();

    txmsg.ext = 1;

    if (cnt <= 8) {
        extid.frm = frm_single;
        txmsg.id = extid.raw;
        txmsg.dlc = cnt;
        if (cnt > 0)
            memcpy(txmsg.data, stream.ptr(), cnt);
        return send_message(txmsg);
    }

    //multi-frame
    uint16_t crc16 = apx::crc32(data, size);
    cnt += sizeof(uint16_t);
    const uint8_t *src = static_cast<const uint8_t *>(stream.ptr());
    uint8_t frm = frm_start;
    while (cnt > 0) {
        uint8_t dlc;
        if (cnt > 8) {
            dlc = 8;
            extid.frm = frm;
            frm = (frm == frm_start || frm >= frm_seq_max) ? 0 : (frm + 1);
            if (cnt == 9) {
                memcpy(txmsg.data, src, 7);
                txmsg.data[7] = crc16;
            } else {
                memcpy(txmsg.data, src, 8);
            }
        } else {
            dlc = cnt;
            extid.frm = frm_end;
            if (cnt > 2) {
                size_t sz = cnt - 2;
                memcpy(txmsg.data, src, sz);
                txmsg.data[sz] = crc16;
                txmsg.data[sz + 1] = crc16 >> 8;
            } else if (cnt == 2) {
                txmsg.data[0] = crc16;
                txmsg.data[1] = crc16 >> 8;
            } else {
                txmsg.data[0] = crc16 >> 8;
            }
        }
        txmsg.id = extid.raw;
        txmsg.dlc = dlc;
        if (!send_message(txmsg))
            return false;
        cnt -= dlc;
        src += dlc;
    }
    return true;
}
