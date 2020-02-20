/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#include "CanCodec.h"
#include "CanFormat.h"

#include <cstring>

using namespace xbus;

bool CanCodec::push_message(const CanID &cid, const uint8_t *data, uint8_t cnt)
{
    for (;;) {
        if (cid.std)
            break;
        uint32_t extid = cid.id;
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
            if (!ecnt) {
                //short msg, one frame only
                pool.remove(pool_msgid); //remove if any from pool
                error = false;
                uint8_t hdr[4];
                XbusStreamWriter stream(hdr);
                stream << src_adr;
                stream << pid(extid);
                rx_queue.write_word(stream.position() + cnt);
                rx_queue.write(stream.buf(), stream.position());
                rx_queue.write(data, cnt);
                packetReceived();

                /*memcpy(stream.data(), data, cnt);
                    stream.reset(stream.position() + cnt);
                    packetReceived(src_adr);*/
                break;
            }
            if (cnt == 0) {
                //multipart msg can't end with empty pld
                pool.remove(pool_msgid); //remove if any from pool
                break;
            }
            //copy payloads from pool
            /*uint8_t hdr[4];
            XbusStreamWriter stream(hdr);
            stream << src_adr;
            stream << pid(extid);
            size_t head_s = rx_queue.head();
            rx_queue.write_word(stream.position() + cnt);
            rx_queue.write(stream.buf(), stream.position());

            rcnt = pool.pop(pool_msgid, stream.data(), xbus::size_packet_max - stream.position());
            if (rcnt == 0)
                break;

            uint16_t found_ecnt = rcnt / 8;
            if (found_ecnt != ecnt) {
                break; // corrupted, or repeated tail
            }
            //copy tail
            if ((rcnt + cnt) > xbus::size_packet_max)
                break;
            stream.reset(stream.position() + rcnt);
            stream.write(data, cnt);
            error = false;
            packetReceived(src_adr);*/
            break;

        } while (0);
        if (error)
            streamError();
        return true; //msg accepted
    }
    return false;
}

uint8_t CanCodec::src_address(uint32_t extid) const
{
    return (extid & XCAN_SRC_MASK) >> XCAN_SRC_SHIFT;
}

xbus::pid_t CanCodec::pid(uint32_t extid) const
{
    return (extid & XCAN_PID_MASK) >> XCAN_PID_SHIFT;
}

void CanCodec::sendAddressing()
{
    CanID cid;
    cid.raw = 0;
    cid.id = XCAN_SRC(nodeId()) | XCAN_PRI_MASK | XCAN_END_MASK | XCAN_NAD_MASK;
    sendAddressingResponse(cid);
}

// Pool

CanCodec::Pool::Pool()
{
    //clear pool
    //memset(_msgid, 0, sizeof(_msgid));

    memset(tree, 0, sizeof(tree));
    memset(items, 0, sizeof(items));
    uint8_t idx = 0;
    for (auto &i : items) {
        i.prev = idx - 1;
        i.next = ++idx;
    }
    items[(sizeof(items) / sizeof(*items)) - 1].next = -1;
    free = 0;
}

bool CanCodec::Pool::push(uint16_t msgid, uint16_t ext, const uint8_t *data)
{
    /*size_t i;
    for (i = 0; (i < XCAN_POOL_SIZE) && (_msgid[i]); i++)
        ; //find emty slot
    if (i >= XCAN_POOL_SIZE)
        return false; //pool overflow
    //push to pool at pos=i
    _ext[i] = ext;
    _msgid[i] = msgid;
    memcpy(_data[i], data, 8);
    updateTimeout(msgid);*/
    for (auto &t : tree) {
        if (t.pid)
            continue;
        t.pid = msgid;
        //find free slot
    }

    return true;
}
void CanCodec::Pool::remove(uint16_t msgid)
{
    for (size_t i = 0; i < XCAN_POOL_SIZE; ++i)
        if (_msgid[i] == msgid)
            _msgid[i] = 0;
}
bool CanCodec::Pool::checkTimeout(void)
{
    bool to = false;
    for (size_t i = 0; i < XCAN_POOL_SIZE; ++i) {
        if (_msgid[i] && (!(_to[i]--))) {
            remove(_msgid[i]);
            to = true;
        }
    }
    return to;
}
void CanCodec::Pool::updateTimeout(uint16_t msgid)
{
    for (uint32_t i = 0; i < XCAN_POOL_SIZE; ++i) {
        if (_msgid[i] == msgid)
            _to[i] = 5;
    }
}

uint16_t CanCodec::Pool::pop(uint16_t msgid, QueueBufferBase &queue)
{
    uint16_t rcnt = 0;
    for (size_t i = 0; i < XCAN_POOL_SIZE; ++i) {
        if (_msgid[i] != msgid)
            continue;
        uint32_t pos = (_ext[i] & 255) << 3;
        _msgid[i] = 0; //release pool slot
        /*if ((pos + 8) > size) {
            remove(msgid);
            return 0; //user read buffer too small
        }
        memcpy(queue + pos, _data[i], 8);*/
        rcnt += 8;
    }
    return rcnt;
}
