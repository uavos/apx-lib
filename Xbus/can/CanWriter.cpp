/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#include "CanWriter.h"
#include "CanFormat.h"

#include <cstring>

using namespace xbus;

bool CanWriter::sendPacket(uint8_t src_addr, const void *data, size_t size)
{
    XbusStreamReader stream(data, size);
    if (stream.tail() < sizeof(xbus::pid_t))
        return true;
    xbus::pid_t pid = stream.read<xbus::pid_t>();

    uint32_t extid = XCAN_SRC(src_addr) | XCAN_PID(pid) | XCAN_PRI_MASK | (1 << 31);

    size = stream.tail();
    const uint8_t *buf = static_cast<const uint8_t *>(stream.data());

    if (size <= 8) {
        return sendMessage(extid | XCAN_END_MASK, buf, size);
    }

    //multi-frame
    uint32_t ext = 0;
    while (size > 0) {
        uint8_t dtc;
        if (size > 8) {
            dtc = 8;
        } else {
            dtc = size;
            ext |= XCAN_END_MASK;
        }
        if (!sendMessage(extid | ext, buf, dtc))
            return false;
        size -= dtc;
        buf += dtc;
        ext++;
    }
    return true;
}
