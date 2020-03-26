/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#include "CanCodec.h"
#include "CanFormat.h"

#include <modules/node/Node.h>

#include <cstring>

#define XCAN_CODEC_DEBUG

#ifdef XCAN_CODEC_DEBUG
#include <platform/log.h>
#define debug(...) apxdebug(__VA_ARGS__)
#define MODULE_NAME "cc"
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
    size_t cnt = pool.read_packet(dest, sz, src_id);
    if (cnt <= (sizeof(xbus::pid_raw_t) + 8))
        return cnt;
    if (!check_crc(dest, cnt))
        return 0;
    return cnt - sizeof(xbus::node::crc_t);
}
bool Codec::check_crc(void *dest, size_t sz)
{
    return true;
    if (sz < (sizeof(xbus::pid_s) + sizeof(xbus::node::crc_t))) {
        debug("no crc %d", sz);
        return false;
    }
    sz -= sizeof(xbus::node::crc_t);
    xbus::node::crc_t crc;
    const uint8_t *p = static_cast<const uint8_t *>(dest) + sz;
    crc = *p++;
    crc |= *p << 8;
    xbus::node::crc_t v = Node::get_crc(dest, sz);
    if (v != crc) {
        debug("crc err: %X (%X)", v, crc);
        return false;
    }
    return true;
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
    if (size < sizeof(xbus::pid_s))
        return true;
    XbusStreamReader stream(data, size);

    extid_s extid;
    extid.raw = _extid_defaults.raw;

    xbus::pid_s pid;
    pid.read(&stream);
    extid.pid = pid._raw;

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
    xbus::node::crc_t crc = Node::get_crc(data, size);
    cnt += sizeof(xbus::node::crc_t);
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
                txmsg.data[7] = crc;
            } else {
                memcpy(txmsg.data, src, 8);
            }
        } else {
            dlc = cnt;
            extid.frm = frm_end;
            if (cnt > 2) {
                size_t sz = cnt - 2;
                memcpy(txmsg.data, src, sz);
                txmsg.data[sz] = crc;
                txmsg.data[sz + 1] = crc >> 8;
            } else if (cnt == 2) {
                txmsg.data[0] = crc;
                txmsg.data[1] = crc >> 8;
            } else {
                txmsg.data[0] = crc >> 8;
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
