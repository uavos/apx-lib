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

#include <XbusPacket.h>

// serial codec over FIFO

class SerialCodec
{
public:
    // return encoded/decoded packet data buffer
    virtual const uint8_t *data() const = 0;
};

class SerialEncoder : public SerialCodec
{
public:
    // encode data to internal buffer:
    // return encoded packet data size;
    // updates status property;
    // each call will always generate packet;
    virtual size_t encode(const void *src, size_t sz) = 0;
};

class SerialDecoder : public SerialCodec
{
public:
    // decode encoded data and write packet to internal buffer:
    // return number of bytes processed which could be less than sz (when pkt is available);
    // updates status property to detect packet is available;
    // packet must be read and released when available to free internal buffer;
    virtual size_t decode(const void *src, size_t sz) = 0;

    // return the size of encoded/decoded packet data bytes
    virtual size_t size() const = 0;

    // decoder status as reported by decode()
    enum SerialDecoderStatus : uint8_t {
        Unknown = 0,

        PacketAvailable,

        DataAccepted,
        DataDropped,

        ErrorOverflow,
        ErrorSize,
        ErrorCRC,
    };

    inline auto status() const { return _status; }

protected:
    SerialDecoderStatus _status{};
};
