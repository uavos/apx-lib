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

#include <sys/types.h>

class SerialCodec
{
public:
    // return size of encoded data in buffer
    virtual size_t size() = 0;

    // reset internal state
    virtual void reset() = 0;
};

class SerialEncoder : public SerialCodec
{
public:
    // write and encode data to buffer
    virtual size_t encode(const void *src, size_t sz) = 0;

    // read encoded data from buffer
    virtual size_t read_encoded(void *dest, size_t sz) = 0;
};

class SerialDecoder : public SerialCodec
{
public:
    enum ErrorType {
        DataAccepted,
        DataDropped,

        ErrorOverflow,
        ErrorSize,
        ErrorCRC,
    };

    // decode encoded data and write packet to buffer
    virtual ErrorType decode(const void *src, size_t sz) = 0;

    // read decoded data from buffer
    virtual size_t read_decoded(void *dest, size_t sz) = 0;
};
