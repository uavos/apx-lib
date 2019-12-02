/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#include "CanWriter.h"
#include "CanFormat.h"

#include <cstring>

using namespace xbus;

bool CanWriter::sendPacket(uint8_t src_addr, uint16_t pid, const uint8_t *data, uint16_t cnt) const
{
    uint32_t extid = XCAN_SRC(src_addr) | XCAN_PID(pid) | XCAN_PRI_MASK;

    if (cnt <= 8) {
        return sendMessage(extid | XCAN_END_MASK, data, cnt);
    }

    //multi-frame
    uint32_t ext = 0;
    while (cnt > 0) {
        uint32_t dtc;
        if (cnt > 8) {
            dtc = 8;
        } else {
            dtc = cnt;
            ext |= XCAN_END_MASK;
        }
        if (!sendMessage(extid | ext, data, dtc))
            return false;
        cnt -= dtc;
        data += dtc;
        ext++;
    }
    return true;
}
