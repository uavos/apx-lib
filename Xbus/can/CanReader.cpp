/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#include "CanReader.h"
#include "CanFormat.h"

#include <cstring>

using namespace xbus;

bool CanReader::push_message(uint32_t extid, const uint8_t *data, uint8_t cnt)
{
    for (;;) {
        if (!(extid & (1 << 31)))
            break;
        extid &= ~(1 << 31);
        if (extid & XCAN_NAD_MASK)
            break;
        const uint8_t src_adr = src_address(extid);
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
        uint16_t pool_msgid = extid >> XCAN_PID_SHIFT; //src_address|var_idx
        uint16_t ext = extid & (XCAN_CNT_MASK | XCAN_END_MASK);

        uint32_t rcnt = 0;
        bool error = true;
        do {
            uint16_t ecnt = (ext & XCAN_CNT_MASK) >> XCAN_CNT_SHIFT;
            if (!(ext & XCAN_END_MASK)) {
                //msg part received
                if (cnt != 8)
                    break; //error - size<8, but multipart middle msg
                if (ecnt == 0)
                    pool.remove(pool_msgid); //first in sequence, remove all previous traces
                //put part to pool
                if (!pool.push(pool_msgid, ext, data))
                    break;
                error = false;
                break;
            }
            //whole msg received - extract rest from pool if any
            XbusStreamWriter stream(_rxdata);
            stream << pid(extid);
            if (!ecnt) {
                //short msg, one frame only
                if (cnt > 0) {
                    memcpy(stream.data(), data, cnt);
                    stream.reset(stream.position() + cnt);
                }
                pool.remove(pool_msgid); //remove if any from pool
                error = false;
                packetReceived(src_adr, stream.buf(), stream.position());
                break;
            }
            if (cnt == 0) {
                //multipart msg can't end with empty pld
                pool.remove(pool_msgid); //remove if any from pool
                break;
            }
            //copy payloads from pool
            rcnt = pool.pop(pool_msgid, stream.data(), XCAN_PACKET_MAX_SIZE - stream.position());
            if (rcnt == 0)
                break;

            uint16_t found_ecnt = rcnt / 8;
            if (found_ecnt != ecnt) {
                break; // corrupted, or repeated tail
            }
            //copy tail
            if ((rcnt + cnt) > XCAN_PACKET_MAX_SIZE)
                break;
            stream.reset(stream.position() + rcnt);
            memcpy(stream.data(), data, cnt);
            stream.reset(stream.position() + cnt);
            error = false;
            packetReceived(src_adr, stream.buf(), stream.position());
            break;

        } while (0);
        if (error)
            streamError();
        return true; //msg accepted
    }
    return false;
}

uint8_t CanReader::src_address(uint32_t extid) const
{
    return (extid & XCAN_SRC_MASK) >> XCAN_SRC_SHIFT;
}

xbus::pid_t CanReader::pid(uint32_t extid) const
{
    return (extid & XCAN_PID_MASK) >> XCAN_PID_SHIFT;
}

void CanReader::sendAddressing()
{
    uint32_t extid = (1 << 31) | XCAN_SRC(nodeId()) | XCAN_PRI_MASK | XCAN_END_MASK | XCAN_NAD_MASK;
    sendAddressingResponse(extid);
}

// Pool

CanReader::Pool::Pool()
{
    memset(_msgid, 0, sizeof(_msgid)); //clear pool
}

bool CanReader::Pool::push(uint16_t msgid, uint16_t ext, const uint8_t *data)
{
    int i;
    for (i = 0; (i < XCAN_POOL_SIZE) && (_msgid[i]); i++)
        ; //find emty slot
    if (i >= XCAN_POOL_SIZE)
        return false; //pool overflow
    //push to pool at pos=i
    _ext[i] = ext;
    _msgid[i] = msgid;
    memcpy(_data[i], data, 8);
    updateTimeout(msgid);
    return true;
}
void CanReader::Pool::remove(uint16_t msgid)
{
    for (int i = 0; i < XCAN_POOL_SIZE; ++i)
        if (_msgid[i] == msgid)
            _msgid[i] = 0;
}
bool CanReader::Pool::checkTimeout(void)
{
    bool to = false;
    for (int i = 0; i < XCAN_POOL_SIZE; ++i) {
        if (_msgid[i] && (!(_to[i]--))) {
            remove(_msgid[i]);
            to = true;
        }
    }
    return to;
}
void CanReader::Pool::updateTimeout(uint16_t msgid)
{
    for (uint32_t i = 0; i < XCAN_POOL_SIZE; ++i) {
        if (_msgid[i] == msgid)
            _to[i] = 5;
    }
}

uint16_t CanReader::Pool::pop(uint16_t msgid, uint8_t *data, uint16_t size)
{
    uint16_t rcnt = 0;
    for (int i = 0; i < XCAN_POOL_SIZE; ++i) {
        if (_msgid[i] != msgid)
            continue;
        uint32_t pos = (_ext[i] & 255) << 3;
        _msgid[i] = 0; //release pool slot
        if ((pos + 8) > size) {
            remove(msgid);
            return 0; //user read buffer too small
        }
        memcpy(data + pos, _data[i], 8);
        rcnt += 8;
    }
    return rcnt;
}
