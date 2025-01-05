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
namespace mission {

#pragma pack(1)

typedef char title_t[32];

struct file_hdr_s
{
    uint32_t size;      // variable length file total size (incl this header)
    uint32_t pld_crc32; // payload CRC (exl this file header)
    // file format version
    static constexpr const uint8_t FORMAT = 1;
    uint8_t format{FORMAT};
    uint8_t _rsv[7]{};
};
static_assert(sizeof(file_hdr_s) == 16, "size");

struct pld_hdr_s
{
    title_t title;

    // payload allocation
    struct pld_item_s
    {
        uint16_t off; // offset in bytes from payload start
        uint16_t cnt; // number of elements
    };

    struct
    {
        // fixed length items
        pld_item_s rw;
        pld_item_s pi;
        pld_item_s wp;
        pld_item_s tw;

        // variable length items
        pld_item_s act; // array of actions
        pld_item_s geo; // array of geofence objects
    } items;

    // unit parameters
    // affect map display and editor
    // data is only used by GCS
    /*struct unit_s
    {
        uint16_t speed; // [m/s] cruise airspeed IAS
        uint16_t turn;  // [m] turn radius at sea level on cruise speed

        uint16_t alt;   // [m] max altitude MSL
        uint16_t endce; // [min] maximum flight endurance

        uint8_t climb; // [m/s] max climbing rate
        uint8_t sink;  // [m/s] max sinking rate

        uint16_t _rsv;

        union {
            uint32_t _raw;
            struct
            {
                bool type : 4; // 0=generic, 1=CTOL, 2=VTOL, 3=HTPL, etc
                bool _rsv : 4;
            };
        } flags;
    };*/
};
// 32 + 6 * 4 = 56 bytes
static_assert(sizeof(pld_hdr_s) == 56, "size");

struct pos_ll_s
{
    uint32_t lat; // [gps] 1e-7 degrees
    uint32_t lon; // [gps] 1e-7 degrees
};

// Waypoint
struct wp_s : pos_ll_s
{
    uint8_t act; // index of action in act array, start from 1
};
static_assert(sizeof(wp_s) == 9, "size");

// Runway
struct rw_s : pos_ll_s
{
    int16_t hmsl;
    int16_t dN;
    int16_t dE;

    enum type_e {
        CLEFT,  // circle left
        CRIGHT, // circle right
    };
    struct
    {
        uint16_t approach : 14; // [m] approach length
        type_e type : 2;        // approach type
    };
    static constexpr const uint16_t APPROACH_MAX = (1 << 14) - 1;
};
static_assert(sizeof(rw_s) == 16, "size");

// Taxiway
struct tw_s : pos_ll_s
{
    // just a sequence of waypoints on the ground
};
static_assert(sizeof(tw_s) == 8, "size");

// Point of Interest
struct pi_s : pos_ll_s
{
    int16_t hmsl;   // altitude [m] MSL to look at
    int16_t radius; // [m] loiter radius, 0=hover at point

    uint8_t loops;    // number of loops to loiter
    uint16_t timeout; // timeout minutes
};
static_assert(sizeof(pi_s) == 15, "size");

// Airspace Geofence
struct geo_s : pos_ll_s
{
    enum type_e {
        CIRCLE,
        POLYGON,
    };
    uint8_t n; // number of points
    struct
    {
        type_e type : 4;
        bool inclusive : 1; // 0=inside, 1=outside
        uint8_t _rsv : 3;
    };
    uint16_t radius; // [m] radius
    // points follow
};
static_assert(sizeof(geo_s) == 12, "size");

// Actions

struct act_s
{
    enum act_e : uint8_t {
        ACT_SEQ,   // actions sequence
        ACT_ALT,   // change altitude immediately
        ACT_TRK,   // change track control immediately
        ACT_SPEED, // change speed immediately
        ACT_POI,   // go to POI after wp reached
        ACT_SCR,   // run script after wp reached
    };
    act_e type : 8;
};

struct act_seq_s : act_s
{
    static constexpr const uint8_t MAX = 7;
    uint8_t next[MAX]; //indices of actions to execute, 0=stop
};
static_assert(sizeof(act_seq_s) == 8, "size");

struct act_alt_s : act_s
{
    int16_t alt; // [m] AGL or AMSL
    struct
    {
        bool amsl : 1; // altitude in AMSL
        bool atrk : 1; // altitude track control
        uint8_t _rsv : 6;
    };
};
static_assert(sizeof(act_alt_s) == 4, "size");

struct act_trk_s : act_s
{
    struct
    {
        bool xtrk : 1; // cross track control
        uint8_t _rsv : 6;
    };
};
static_assert(sizeof(act_trk_s) == 2, "size");

struct act_speed_s : act_s
{
    uint8_t speed; //0=cruise
};
static_assert(sizeof(act_speed_s) == 2, "size");

struct act_poi_s : act_s
{
    uint8_t index; //linked POI [0...n]
};
static_assert(sizeof(act_poi_s) == 2, "size");

struct act_scr_s : act_s
{
    typedef char scr_t[15]; //public func @name
    scr_t scr;
};
static_assert(sizeof(act_scr_s) == 16, "size");

#pragma pack()

} // namespace mission
} // namespace xbus
