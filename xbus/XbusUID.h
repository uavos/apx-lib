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
#include <sys/types.h>

namespace xbus {
namespace cmd {

static constexpr uint16_t uid = 0x0700;

template<uint16_t MASK = 0xFF00>
static constexpr bool match(uint16_t v)
{
    return (v & MASK) == uid;
}

enum class _cmd_e {
    unit,
    telemetry,
    stream,
    sim,
    script,

    node = 15,
};

template<_cmd_e BASE>
struct _base_s
{
    static constexpr uint16_t uid = cmd::uid + (((uint16_t) BASE) << 8);
    static constexpr auto match = cmd::match<0xFFF0>;
};

struct node : _base_s<_cmd_e::node>
{
    enum {
        search = uid,
        ident,
        file,
        reboot,
        msg,
        upd,
        mod,
        usr,
    };
};

} // namespace cmd
} // namespace xbus
