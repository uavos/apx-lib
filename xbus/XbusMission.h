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

typedef char title_t[16];

enum item_e { //4bits
    STOP = 0,
    WP,
    RW,
    TW,
    PI,
    ACT,
    DIS,
    EMG
};

struct hdr_s
{
    uint8_t type;   //wp,rw,scr, ..
    uint8_t option; //left,right,line,hdg, ..

    static inline uint16_t psize() { return sizeof(uint8_t); }
    inline void read(XbusStreamReader *s)
    {
        uint8_t v;
        *s >> v;
        type = v & 0x0F;
        option = v >> 4;
    }
    inline void write(XbusStreamWriter *s) const
    {
        uint8_t v = (type & 0x0F) | ((option << 4) & 0xF0);
        *s << v;
    }
};

struct file_hdr_s
{
    uint32_t size;

    title_t title;

    struct stats_s
    {
        uint16_t wp;
        uint16_t rw;
        uint16_t tw;
        uint16_t pi;
        uint16_t dis;
        uint16_t emg;
    };
    stats_s cnt;
    stats_s off;

    static inline uint16_t psize()
    {
        return sizeof(uint32_t) + sizeof(title_t)
               + sizeof(uint16_t) * 6 * 2;
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> size;
        s->read(title, sizeof(title));
        *s >> cnt.wp;
        *s >> cnt.rw;
        *s >> cnt.tw;
        *s >> cnt.pi;
        *s >> cnt.dis;
        *s >> cnt.emg;
        *s >> off.wp;
        *s >> off.rw;
        *s >> off.tw;
        *s >> off.pi;
        *s >> off.dis;
        *s >> off.emg;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << size;
        s->write(title, sizeof(title));
        *s << cnt.wp;
        *s << cnt.rw;
        *s << cnt.tw;
        *s << cnt.pi;
        *s << cnt.dis;
        *s << cnt.emg;
        *s << off.wp;
        *s << off.rw;
        *s << off.tw;
        *s << off.pi;
        *s << off.dis;
        *s << off.emg;
    }

    inline constexpr uint16_t count(item_e type) const
    {
        switch (type) {
        default:
            return 0;
        case WP:
            return cnt.wp;
        case RW:
            return cnt.rw;
        case TW:
            return cnt.tw;
        case PI:
            return cnt.pi;
        case DIS:
            return cnt.dis;
        case EMG:
            return cnt.emg;
        }
    }
    inline constexpr uint16_t offset(item_e type) const
    {
        switch (type) {
        default:
            return 0;
        case WP:
            return off.wp;
        case RW:
            return off.rw;
        case TW:
            return off.tw;
        case PI:
            return off.pi;
        case DIS:
            return off.dis;
        case EMG:
            return off.emg;
        }
    }
};

struct wp_s
{
    float lat;
    float lon;
    uint16_t alt;

    enum {
        DIRECT,
        TRACK,
    };

    static inline uint16_t psize() { return sizeof(float) * 2 + sizeof(int16_t); }
    inline void read(XbusStreamReader *s)
    {
        *s >> lat;
        *s >> lon;
        *s >> alt;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << lat;
        *s << lon;
        *s << alt;
    }
};

struct rw_s
{
    float lat;
    float lon;
    int16_t hmsl;
    int16_t dN;
    int16_t dE;
    uint16_t approach;

    enum {
        LEFT,
        RIGHT,
    };

    static inline uint16_t psize()
    {
        return sizeof(float) * 2 + sizeof(int16_t) + sizeof(int16_t) + sizeof(int16_t)
               + sizeof(uint16_t);
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> lat;
        *s >> lon;
        *s >> hmsl;
        *s >> dN;
        *s >> dE;
        *s >> approach;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << lat;
        *s << lon;
        *s << hmsl;
        *s << dN;
        *s << dE;
        *s << approach;
    }
};

struct tw_s
{
    float lat;
    float lon;

    static inline uint16_t psize() { return sizeof(float) * 2; }
    inline void read(XbusStreamReader *s)
    {
        *s >> lat;
        *s >> lon;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << lat;
        *s << lon;
    }
};

struct pi_s
{
    float lat;
    float lon;
    int16_t hmsl;
    int16_t radius;
    uint8_t loops;
    uint16_t timeout; // TODO change to minutes

    static inline uint16_t psize()
    {
        return sizeof(float) * 2 + sizeof(int16_t) + sizeof(int16_t) + sizeof(uint8_t)
               + sizeof(uint16_t);
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> lat;
        *s >> lon;
        *s >> hmsl;
        *s >> radius;
        *s >> loops;
        *s >> timeout;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << lat;
        *s << lon;
        *s << hmsl;
        *s << radius;
        *s << loops;
        *s << timeout;
    }
};

struct area_s
{
    float lat;
    float lon;

    static inline uint16_t psize(uint8_t pointsCnt) { return sizeof(float) * 2 * pointsCnt; }
    inline void read(XbusStreamReader *s)
    {
        *s >> lat;
        *s >> lon;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << lat;
        *s << lon;
    }
};

// Actions

enum act_e {
    ACT_SPEED,
    ACT_PI,
    ACT_SCR,
    ACT_LOITER,
    ACT_SHOT,
};

struct act_speed_s
{
    uint8_t speed; //0=cruise

    static inline uint16_t psize() { return sizeof(uint8_t); }
    inline void read(XbusStreamReader *s) { *s >> speed; }
    inline void write(XbusStreamWriter *s) const { *s << speed; }
};

struct act_pi_s
{
    uint8_t index; //linked POI [0...n]

    static inline uint16_t psize() { return sizeof(uint8_t); }
    inline void read(XbusStreamReader *s) { *s >> index; }
    inline void write(XbusStreamWriter *s) const { *s << index; }
};

struct act_scr_s
{
    typedef char scr_t[16]; //public func @name
    scr_t scr;

    static inline uint16_t psize() { return sizeof(scr_t); }
    inline void read(XbusStreamReader *s) { s->read(scr, sizeof(scr)); }
    inline void write(XbusStreamWriter *s) const { s->write(scr, sizeof(scr)); }
};

struct act_loiter_s // TODO deprecated action
{
    int16_t radius;
    uint8_t loops;    //loops to loiter
    uint16_t timeout; //time to loiter [s]

    static inline uint16_t psize() { return sizeof(int16_t) + sizeof(uint8_t) + sizeof(uint16_t); }
    inline void read(XbusStreamReader *s)
    {
        *s >> radius;
        *s >> loops;
        *s >> timeout;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << radius;
        *s << loops;
        *s << timeout;
    }
};

struct act_shot_s
{
    uint16_t dist; //distance for series
    uint8_t opt;   //0=single,1=start,2=stop

    static inline uint16_t psize() { return sizeof(uint16_t); }
    inline void read(XbusStreamReader *s)
    {
        uint16_t v;
        *s >> v;
        dist = v & 0x0FFF;
        opt = v >> 12;
    }
    inline void write(XbusStreamWriter *s) const
    {
        uint16_t v = (dist & 0x0FFF) | ((opt << 12) & 0xF000);
        *s << v;
    }
};

inline constexpr uint16_t action_psize(uint8_t option)
{
    switch (static_cast<act_e>(option)) {
    case ACT_SPEED:
        return act_speed_s::psize();
    case ACT_PI:
        return act_pi_s::psize();
    case ACT_SCR:
        return act_scr_s::psize();
    case ACT_LOITER:
        return act_loiter_s::psize();
    case ACT_SHOT:
        return act_shot_s::psize();
    }
    return 0;
}

inline constexpr uint16_t psize(item_e type, uint8_t option)
{
    switch (type) {
    case WP:
        return wp_s::psize();
    case RW:
        return rw_s::psize();
    case TW:
        return tw_s::psize();
    case PI:
        return pi_s::psize();
    case ACT:
        return action_psize(option);
    case DIS:
    case EMG:
        return area_s::psize(option);
    default:
        return 0;
    }
}

} // namespace mission
} // namespace xbus
