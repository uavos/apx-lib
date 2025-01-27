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

#include "CanFormat.h"
#include "xcan_msg.h"

#include <XbusPacket.h>

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
    void init();

    ErrorType push(const extid_s &extid, const uint8_t *data, uint8_t dlc);
    size_t read_packet(void *dest, size_t sz, uint8_t *src_id);
    bool timeout();

    size_t space() const; //return number of free slots for debug

    void report_status();

private:
    static constexpr const uint8_t size_items = (xbus::size_packet_max / 8) * 1.2;
    static constexpr const uint8_t size_trees = 8; // pids simultaneously
    static constexpr const uint8_t max_idx = 0xFF;
    static constexpr const uint8_t max_seq_idx = size_items;

#pragma pack(1)
    struct Item // 9 bytes
    {
        uint8_t next;
        uint8_t data[8];
    };

    struct Tree
    {
        extid_s extid;
        uint8_t head;
        uint8_t dlc; // last msg dlc, >0 means tree is done, =0xFF means zero length
        uint8_t to;  // timeout
    };
#pragma pack()

    Tree trees[size_trees];
    Item items[size_items]; // message data slots
    uint8_t free;           // index of free item

    ErrorType push_new(const extid_s &extid, const uint8_t *data, uint8_t dlc);
    ErrorType push_next(const extid_s &extid, const uint8_t *data, uint8_t dlc);
    ErrorType push_next(Tree &t, const uint8_t *data);

    void push(const uint8_t *data);
    void remove(const extid_s &extid);
    void remove(Tree &t);
    size_t read_packet(Tree &t, void *dest, size_t sz);
};

} // namespace can
} // namespace xbus
