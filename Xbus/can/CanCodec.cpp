/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#include "CanCodec.h"
#include "CanFormat.h"

#include <modules/node/Node.h>

#include <cstring>

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
    if (!cnt)
        return 0;
    if (!check_crc(dest, cnt))
        return 0;
    return cnt - sizeof(xbus::node::crc_t);
}
bool Codec::check_crc(void *dest, size_t sz)
{
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
    debug("%X", nodeId());
    extid_s extid;
    extid.raw = _extid_defaults.raw;
    extid.pid = 0xFFFF;
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

    size = stream.available();

    txmsg.ext = 1;

    if (size <= 8) {
        txmsg.id = extid.raw;
        txmsg.dlc = size;
        if (size > 0)
            memcpy(txmsg.data, stream.ptr(), size);
        return send_message(txmsg);
    }

    //multi-frame
    xbus::node::crc_t crc = Node::get_crc(data, size);
    size += sizeof(xbus::node::crc_t);
    const uint8_t *src = static_cast<const uint8_t *>(stream.ptr());
    uint8_t frm = frm_seq0;
    while (size > 0) {
        uint8_t dlc;
        if (size > 8) {
            dlc = 8;
            extid.frm = frm;
            frm++;
            if (frm > frm_seq1)
                frm = frm_seq0;
            if (size == 9) {
                memcpy(txmsg.data, src, 7);
                txmsg.data[7] = crc;
            } else {
                memcpy(txmsg.data, src, 8);
            }
        } else {
            dlc = size;
            extid.frm = frm_end;
            if (size > 2) {
                memcpy(txmsg.data, src, size - 2);
                txmsg.data[6] = crc;
                txmsg.data[7] = crc >> 8;
            } else if (size == 2) {
                txmsg.data[6] = crc;
                txmsg.data[7] = crc >> 8;
            } else {
                txmsg.data[7] = crc >> 8;
            }
        }
        txmsg.id = extid.raw;
        txmsg.dlc = dlc;
        if (!send_message(txmsg))
            return false;
        size -= dlc;
        src += dlc;
    }
    return true;
}
