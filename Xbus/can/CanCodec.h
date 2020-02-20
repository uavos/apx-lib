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
#include <containers/QueueBuffer.h>

#ifndef XCAN_POOL_SIZE
#define XCAN_POOL_SIZE (xbus::size_packet_max / 8) //64
#endif

namespace xbus {

class CanCodec : public do_not_copy
{
public:
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
    bool push_message(const CanID &cid, const uint8_t *data, uint8_t cnt);

private:
    /**
     * @brief Extract sender address from can ID.
     * @param extid can ID of the received message.
     * @return Returns sender address.
     */
    uint8_t src_address(uint32_t extid) const;

    /**
     * @brief Extract xbus packet id from can ID.
     * @param extid can ID of the received message.
     * @return Returns packet id.
     */
    xbus::pid_t pid(uint32_t extid) const;

    /**
     * @brief Prepare and send addressing packet.
     */
    void sendAddressing();

    class Pool
    {
    public:
        Pool();
        bool push(uint16_t msgid, uint16_t ext, const uint8_t *data);
        uint16_t pop(uint16_t msgid, QueueBufferBase &queue);
        void remove(uint16_t msgid); //msgid = (src_address:H|pid:L)
        bool checkTimeout(void);
        void updateTimeout(uint16_t msgid); //msgid = (src_address:H|pid:L)

        struct Item // 10 bytes
        {
            uint8_t data[8];
            uint8_t prev;
            uint8_t next;
        } __attribute__((packed));
        struct Tree // 4 bytes
        {
            uint16_t pid;
            uint8_t head;
            uint8_t to;
        } __attribute__((packed));

        Tree tree[16];                         // pids simulaneously [64 bytes]
        Item items[xbus::size_packet_max / 8]; //64 slots [640 bytes]
        uint8_t free;                          //index of free item

    private:
        //13 bytes each item
        uint16_t _ext[XCAN_POOL_SIZE];    // frames left
        uint16_t _msgid[XCAN_POOL_SIZE];  // message id
        uint8_t _data[XCAN_POOL_SIZE][8]; // payload
        uint8_t _to[XCAN_POOL_SIZE];      // timeouts
    };

    Pool pool; // 832 bytes

    uint8_t rx_header[4];

    //QueueBuffer<xbus::size_packet_max> tx_queue;

protected:
    //packets: <src_node_id_8><packet>
    QueueBuffer<xbus::size_packet_max> rx_queue;

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
     * @param pid packet id extracted from multipart stream.
     * @param data packet payload data.
     * @param cnt packet payload size.
     */
    virtual void packetReceived() = 0;

    /**
     * @brief virtual method is called to send simple zero payload can message when addressing.
     * @param extid can ID of the message, MSB=EXTID (Extended ID indication).
     */
    virtual void sendAddressingResponse(const CanID &cid) = 0;

    /**
     * @brief virtual method is called to indicate error.
     */
    virtual void streamError() = 0;
};

} // namespace xbus
