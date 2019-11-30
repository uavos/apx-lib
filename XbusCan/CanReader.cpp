/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#include "CanReader.h"

using namespace xbus;

#define XCAN_SRC_SHIFT 20 //source address
#define XCAN_SRC_MASK (255 << XCAN_SRC_SHIFT)
#define XCAN_SRC(n) ((n) << XCAN_SRC_SHIFT)

#define XCAN_NAD_SHIFT 12 //node addressing marker
#define XCAN_NAD_MASK (1 << XCAN_NAD_SHIFT)

#define XCAN_END_SHIFT 11 //end marker
#define XCAN_END_MASK (1 << XCAN_END_SHIFT)

#define XCAN_CNT_SHIFT 0 //multipart counter (256*8=2048 bytes max)
#define XCAN_CNT_MASK (255 << XCAN_CNT_SHIFT)
#define XCAN_CNT(n) ((n) << XCAN_CNT_SHIFT)

#define XCAN_PRI_SHIFT 28 //priority bit (lowest)
#define XCAN_PRI_MASK (1 << XCAN_PRI_SHIFT)

CanReader::CanReader() {}

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
        return true;
    }
    return false;
}

uint8_t CanReader::src_address(uint32_t extid) const
{
    return (extid & XCAN_SRC_MASK) >> XCAN_SRC_SHIFT;
}

void CanReader::sendAddressing() const
{
    uint32_t extid = (1 << 31) | XCAN_SRC(nodeId()) | XCAN_PRI_MASK | XCAN_END_MASK | XCAN_NAD_MASK;
    sendAddressingResponse(extid);
}
