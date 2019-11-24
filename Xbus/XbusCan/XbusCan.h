/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#pragma once

#include <cstdint>

#include <common/do_not_copy.h>

namespace xbus {

class XbusCanReader : public do_not_copy
{
public:
    /**
     * @brief callback function is called when packet is available for read
     * @param src_addr address of sender.
     * @param pid packet id extracted from multipart stream.
     * @param size packet size.
     */
    typedef void (*PacketAvailableCallback)(uint8_t src_addr, uint16_t pid, uint16_t size);

    explicit XbusCanReader(PacketAvailableCallback *cb);

    /**
     * @brief Push message to pool and process it.
     *        It does:
     *        - check extid address space
     *        - put message to pool and check if the packet can be assembled
     *        - call PacketAvailableCallback when multipart packet is received
     * @param extid can ID of the message.
     * @param data Pointer to the message payload.
     * @param cnt number of payload bytes.
     * @return Returns false if message is not accepted.
     */
    bool push_message(uint32_t extid, const uint8_t *data, uint8_t cnt);

    uint16_t read_packet_data(uint8_t *data, uint8_t cnt);

    /**
     * @brief Extract sender address from can ID
     * @param extid can ID of the received message.
     * @return Returns sender address
     */
    uint8_t src_address(uint32_t extid) const;

private:
    PacketAvailableCallback *_cb;
};

} // namespace xbus
