/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#pragma once

#include <cstdint>

#include "CanMsg.h"
#include <Xbus/XbusPacket.h>
#include <common/do_not_copy.h>

namespace xbus {

class CanCodec : public do_not_copy
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

    /**
     * @brief Push message to pool and process it.
     *        It does:
     *        - check extid address space
     *        - put message to pool and check if the packet can be assembled
     *        - call virtual method packetReceived when whole packet is received
     * @param cid can ID of the message
     * @param data Pointer to the message payload.
     * @param cnt number of payload bytes.
     * @return Returns false if message is not accepted.
     */
    ErrorType push_message(const CanMsg &msg);

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

    void report_status();

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
    virtual bool send_message(const CanMsg &msg) = 0;

private:
    /**
     * @brief Prepare and send addressing packet.
     */
    void sendAddressing();

    CanMsg txmsg;

    class Pool
    {
    public:
        Pool();

        typedef uint16_t mid_t; // src_address:H | pid:L

        ErrorType push(mid_t mid, size_t seq_idx, const uint8_t *data, uint8_t dlc);
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
            mid_t mid;
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
        void remove(mid_t mid);
        void remove(Tree &t);
        size_t read_packet(Tree &t, void *dest, size_t sz, uint8_t *src_id);
    };
    Pool pool; // 832 bytes
};

} // namespace xbus
