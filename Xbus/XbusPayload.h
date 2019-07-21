#pragma once
#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

#include <array>

#define VEHICLE_CLASS_LIST \
    UAV,GCU,UGV,USV,SAT,RELAY

struct XbusVehiclePayload
{

    // Vehicle IDENT

    typedef std::array<char,16>     callsign_t;
    typedef std::array<uint8_t,12>  vuid_t; //global unique vehicle id
    typedef uint8_t                 vclass_t;


    typedef enum {
        VEHICLE_CLASS_LIST
    } vehicle_class_t;

    struct Ident {
        callsign_t  callsign;
        vuid_t      vuid;
        vclass_t    vclass;

        static uint16_t psize()
        {
            return std::tuple_size<callsign_t>()
                    + std::tuple_size<vuid_t>()
                    + sizeof(vclass_t);
        }
    };
    static void read(const uint8_t *p, Ident &d)
    {
        XbusStreamReader s(p);
        s >> d.callsign;
        s >> d.vuid;
        s >> d.vclass;
    }
    static void write(uint8_t *p, const Ident &d)
    {
        XbusStreamWriter s(p);
        s << d.callsign;
        s << d.vuid;
        s << d.vclass;
    }


    // Vehicle transponder data

    typedef float       lat_t;
    typedef float       lon_t;
    typedef int16_t     alt_t;
    typedef uint16_t    speed_t;
    typedef int16_t     course_t;
    typedef uint8_t     mode_t;

    struct Xpdr {
        lat_t       lat;
        lon_t       lon;
        float       alt;
        float       speed;
        float       course;
        mode_t      mode;

        static uint16_t psize()
        {
            return sizeof(lat_t)
                    + sizeof(lon_t)
                    + sizeof(alt_t)
                    + sizeof(speed_t)
                    + sizeof(course_t)
                    + sizeof(mode_t);

        }
    };
    static void read(const uint8_t *p, Xpdr &d)
    {
        XbusStreamReader s(p);
        s >> d.lat;
        s >> d.lon;
        d.alt=s.read<alt_t,float>();
        d.speed=s.read<speed_t,float>()/100.0f;
        d.course=s.read<course_t,float>()/(32768.0f/180.0f);
        s >> d.mode;
    }
    static void write(uint8_t *p, const Xpdr &d)
    {
        XbusStreamWriter s(p);
        s << d.lat;
        s << d.lon;
        s.write<alt_t>(d.alt);
        s.write<speed_t>(d.speed*100.0f);
        s.write<course_t>(d.course*(32768.0f/180.0f));
        s << d.mode;
    }

};

