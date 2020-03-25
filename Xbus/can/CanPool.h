/****************************************************************************
 *
 *   Copyright (c) 2019 Aliaksei Stratsilatau. All rights reserved.
 *
 ****************************************************************************/

#pragma once

#include <cstdint>

#include "CanFormat.h"
#include "xcan_msg.h"

#include <Xbus/XbusPacket.h>
#include <common/do_not_copy.h>

namespace xbus {
namespace can {

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

class Pool
{
public:
    Pool();
    void init();

    ErrorType push(const extid_s &extid, const uint8_t *data, uint8_t dlc);
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
        extid_s extid;
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

    ErrorType push_new(const extid_s &extid, const uint8_t *data, uint8_t dlc);
    ErrorType push_next(const extid_s &extid, const uint8_t *data, uint8_t dlc);

    void push(const uint8_t *data);
    void remove(const extid_s &extid);
    void remove(Tree &t);
    size_t read_packet(Tree &t, void *dest, size_t sz, uint8_t *src_id);
};

} // namespace can
} // namespace xbus
