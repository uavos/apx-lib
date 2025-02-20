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
    aux,
    sim,

    node = 15,
};

template<_cmd_e BASE>
struct _base_s
{
    static constexpr uint16_t uid = cmd::uid + (((uint16_t) BASE) << 8);
    static constexpr auto match = cmd::match<0xFFF0>;
};

// nodes network management
struct node : _base_s<_cmd_e::node>
{
    enum {
        search = uid, // Search nodes broadcast request, replies <uid>
        ident,        // Node identification <ident_s><strings:name+version+hardware><filenames>
        file,         // File operations <name><fop_e>[<data>]
        reboot,       // System reboot <type_e>
        msg,          // Text message <type_e><string>
        upd,          // Update parameter <fid_t><data>
        mod,          // Modules tree <op_e>
        usr,          // Node specific command <cmd_s>[<data>]
    };
};

// inter-unit communication
struct unit : _base_s<_cmd_e::unit>
{
    enum {
        ident = uid, // unit identity and squawk assignment
        heartbeat,   // unit heartbeat
    };
};

// telemetry data stream
struct telemetry : _base_s<_cmd_e::telemetry>
{
    enum {
        data = uid, // telemetry data stream
        format,     // telemetry format
        xpdr,       // transponder data
    };
};

// auxiliary data objects
struct aux : _base_s<_cmd_e::aux>
{
    enum {
        vcp = uid, // virtual com port data
        vmexec,    // node script call request
        jsexec,    // GCS JScript call request
    };
};

// simulation data
struct sim : _base_s<_cmd_e::sim>
{
    enum {
        sns = uid, // sensors data
        ctr,       // controls data
        cfg,       // datarefs mapping
    };
};

} // namespace cmd
} // namespace xbus
