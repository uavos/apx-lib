#pragma once
#include "XbusPacket.h"

class XbusVehicle: protected XbusPacket
{
public:
    XbusVehicle(uint8_t *packet_ptr);

    typedef uint16_t      squawk_t;

public:
    squawk_t squawk() const;
    void setSquawk(const squawk_t v);

private:
    uint8_t *_hdr;
};
