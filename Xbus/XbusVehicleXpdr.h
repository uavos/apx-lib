#pragma once
#include "XbusVehicle.h"

class XbusVehicleXpdr: private XbusVehicle
{
public:
    XbusVehicleXpdr(uint8_t *packet_ptr);

    virtual bool isValid(const uint16_t packet_size) const override;

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
    };

    void read(Xpdr &d);
    void write(const Xpdr &d);
};
