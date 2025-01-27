/*
 * APX Autopilot project <http://docs.uavos.com>
 *
 * Copyright (c) 2003-2020, Aliaksei Stratsilatau <sa@uavos.com>
 * All rights reserved
 *
 * This file is part of APX Shared Libraries.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <cstdint>

#include "xcan_msg.h"
#include <XbusPacket.h>

namespace xbus {

class CanCodec
{
public:
    enum ErrorType {
        PacketAvailable = 0,
        MsgAccepted,
        MsgDropped,

        ErrorDLC,
        ErrorSlotsOverflow,
        ErrorTreeOverflow,
        ErrorSeqIdx,
        ErrorOrphan,
    };

    // unique id of the message <src|pid>
    typedef uint16_t pool_id_t;

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
     * @param src_addr address of sender.
     * @param data packet payload data.
     * @param size packet payload size.
     * @return Returns false on error.
     */
    bool send_packet(uint8_t src_addr, const void *data, size_t size);

    /**
     * @brief Reads available packets to buffer.
     * @param dest destination buffer pointer.
     * @param sz destination buffer size
     * @param src_id pointer to store source node id for packet.
     * @return Returns number of bytes read.
     */
    size_t read_packet(void *dest, size_t sz, uint8_t *src_id);

    /**
     * @brief Reads mid from msg id. mid is used as ID in pool.
     * @param extid is CAN message EXT ID.
     * @return Returns <src_address|var_idx>.
     */
    static pool_id_t extid_to_mid(const uint32_t extid);

    /**
     * @brief Reads pid from pool id.
     * @param mid is pool ID.
     * @return Returns xbus::pid_t.
     */
    static xbus::pid_t mid_to_pid(const pool_id_t mid);

    /**
     * @brief Reads stc_id from msg id.
     * @param mid is pool ID.
     * @return Returns src node address.
     */
    static xbus::pid_t mid_to_src(const pool_id_t mid);

    /**
     * @brief Reads pid from CAN EXTID.
     * @param extid is CAN message EXT ID.
     * @return Returns xbus::pid_t.
     */
    static xbus::pid_t extid_to_pid(const uint32_t extid);

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
     * @brief virtual method is called to send can message
     * @param cid CanID of the message
     * @param data payload data.
     * @param dlc payload size.
     * @return Returns false on error.
     */
    virtual bool send_message(const xcan_msg_s &msg) = 0;

private:
    /**
     * @brief Prepare and send addressing packet.
     */
    void sendAddressing();

    xcan_msg_s txmsg;

protected:
    class Pool
    {
    public:
        Pool();
        void init();

        ErrorType push(pool_id_t mid, size_t seq_idx, const uint8_t *data, uint8_t dlc);
        size_t read_packet(void *dest, size_t sz, uint8_t *src_id);
        bool timeout();

        size_t space() const; //return number of free slots for debug

        void report_status();

    private:
        static constexpr const uint8_t size_items = (xbus::size_packet_max / 8);
        static constexpr const uint8_t size_trees = 16; // pids simulaneously [64 bytes]
        static constexpr const uint8_t max_idx = 0xFF;
        static constexpr const uint8_t max_seq_idx = size_items;

        struct Item // 9 bytes
        {
            uint8_t next;
            uint8_t data[8];
        } __attribute__((packed));
        struct Tree // 4 bytes
        {
            pool_id_t mid;
            uint8_t head;
            struct
            {
                uint8_t to : 4;  // timeout
                uint8_t dlc : 4; // last msg dlc, >0 means tree is done, =0xF means zero length
            };
        } __attribute__((packed));

        Tree trees[size_trees];
        Item items[size_items]; // message data slots
        uint8_t free;           // index of free item

        void push(const uint8_t *data);
        void remove(pool_id_t mid);
        void remove(Tree &t);
        size_t read_packet(Tree &t, void *dest, size_t sz, uint8_t *src_id);
    };
    Pool pool; // 832 bytes
};

} // namespace xbus
