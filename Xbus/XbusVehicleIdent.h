#pragma once
#include "XbusVehicle.h"

#include <array>

class XbusVehicleIdent: private XbusVehicle
{
public:
    XbusVehicleIdent(uint8_t *packet_ptr);

    typedef std::array<char,16>     callsign_t;
    typedef std::array<uint8_t,12>  vuid_t; //global unique vehicle id
    typedef uint8_t                 vclass_t;

#define VEHICLE_CLASS_LIST UAV,GCU,UGV,USV,SAT,RELAY

    typedef enum {
      VEHICLE_CLASS_LIST
    } vehicle_class_t;


    virtual bool isValid(const uint16_t packet_size) const override;

    struct Ident {
        callsign_t  callsign;
        vuid_t      vuid;
        vclass_t    vclass;
    };

    void read(Ident &d);
    void write(const Ident &d);
};
