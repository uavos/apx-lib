/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#pragma once

#include <cstdint>

#include <Xbus/XbusPacket.h>
#include <common/do_not_copy.h>

namespace xbus {

class CanWriter : public do_not_copy
{
public:
    /**
     * @brief Send packet splitting to multiframe messages if necessary.
     *        Calls virtual method sendMessage to send messages.
     * @param src_addr address of sender.
     * @param data packet payload data.
     * @param size packet payload size.
     * @return Returns false on error.
     */
    bool sendPacket(uint8_t src_addr, const void *data, size_t size);

private:
protected:
    /**
     * @brief virtual method is called to send simple zero payload can message when addressing.
     * @param extid can ID of the message, MSB=EXTID (Extended ID indication).
     * @param data payload data.
     * @param dlc payload size.
     * @return Returns false on error.
     */
    virtual bool sendMessage(uint32_t extid, const uint8_t *data, uint8_t dlc) = 0;
};

} // namespace xbus
