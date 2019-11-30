/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#pragma once

#include <cstdint>

#include <common/do_not_copy.h>

namespace xbus {

class CanReader : public do_not_copy
{
public:
    explicit CanReader();

    /**
     * @brief Push message to pool and process it.
     *        It does:
     *        - check extid address space
     *        - put message to pool and check if the packet can be assembled
     *        - call PacketAvailableCallback when multipart packet is received
     * @param extid can ID of the message, MSB=EXTID (Extended ID indication).
     * @param data Pointer to the message payload.
     * @param cnt number of payload bytes.
     * @return Returns false if message is not accepted.
     */
    bool push_message(uint32_t extid, const uint8_t *data, uint8_t cnt);

private:
    /**
     * @brief Extract sender address from can ID.
     * @param extid can ID of the received message.
     * @return Returns sender address.
     */
    uint8_t src_address(uint32_t extid) const;

    /**
     * @brief EPrepare and send addressing packet
     */
    void sendAddressing() const;

protected:
    /**
     * @brief virtual method to get the local node address value.
     * @note can be set automatically for proper addressing.
     * @return Returns local node address, must be non zero.
     */
    virtual uint8_t nodeId() const = 0;

    /**
     * @brief virtual method to select different node address.
     * @note called automatically for proper addressing. Must change nodeId.
     */
    virtual void updateNodeId() = 0;

    /**
     * @brief virtual method is called when packet is available for read.
     * @param src_addr address of sender.
     * @param pid packet id extracted from multipart stream.
     * @param size packet size.
     */
    virtual void packetAvailable(uint8_t src_addr, uint16_t pid, uint16_t size) const = 0;

    /**
     * @brief virtual method is called to send simple zero payload can message when addressing.
     * @param extid can ID of the message, MSB=EXTID (Extended ID indication).
     */
    virtual void sendAddressingResponse(uint32_t extid) const = 0;
};

} // namespace xbus
