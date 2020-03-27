/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#pragma once

#include <cstdint>

#include "CanPool.h"

#include <Xbus/XbusNode.h>
#include <Xbus/XbusPacket.h>
#include <common/do_not_copy.h>

namespace xbus {
namespace can {

class Codec : public do_not_copy
{
public:
    explicit Codec();

    /**
     * @brief Push message to pool and process it. Must be EXTID message.
     *        It does:
     *        - check extid address space
     *        - put message to pool and check if the packet can be assembled
     * @param msg CAN message
     * @return Returns ErrorType
     */
    ErrorType push_message(const xcan_msg_s &msg);

    /**
     * @brief Send packet splitting to multiframe messages if necessary.
     *        Calls virtual method sendMessage to send messages.
     * @param data packet payload data.
     * @param size packet payload size.
     * @return Returns false on error.
     */
    bool send_packet(const void *data, size_t size);

    /**
     * @brief Reads available packets to buffer.
     * @param dest destination buffer pointer.
     * @param sz destination buffer size
     * @param src_id pointer to store source node id for packet.
     * @return Returns number of bytes read.
     */
    size_t read_packet(void *dest, size_t sz, uint8_t *src_id);

protected:
    /**
     * @brief virtual method to select different node address.
     * @note called automatically for proper addressing. Must change _extid_defaults::src.
     */
    virtual void updateNodeId() = 0;

    /**
     * @brief virtual method is called to send can message
     * @param cid CanID of the message
     * @param data payload data.
     * @param dlc payload size.
     * @return Returns false on error.
     */
    virtual bool send_message(const xcan_msg_s &msg) = 0;

    virtual void lock_push(bool locked) = 0;

private:
    /**
     * @brief Prepare and send addressing packet.
     */
    void sendAddressing();

    xcan_msg_s txmsg;

    size_t check_crc(void *dest, size_t sz);
    static xbus::node::crc_t get_crc(const void *src, size_t sz);

protected:
    Pool pool; // 832 bytes

    extid_s _extid_defaults;
};

} // namespace can
} // namespace xbus
