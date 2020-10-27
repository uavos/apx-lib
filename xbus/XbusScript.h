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
#include "XbusNode.h"
#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

namespace xbus {
namespace script {

typedef char title_t[64];

static constexpr const size_t max_file_size{65535};

struct file_hdr_s
{
    uint32_t code_size;
    uint32_t src_size;

    title_t title;

    static inline uint16_t psize()
    {
        return sizeof(uint32_t) * 2 + sizeof(title_t);
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> code_size;
        *s >> src_size;
        s->read(title, sizeof(title));
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << code_size;
        *s << src_size;
        s->write(title, sizeof(title));
    }
};

} // namespace script
} // namespace xbus
