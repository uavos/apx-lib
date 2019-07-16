#include "XbusVehicle.h"

#include "XbusReader.h"
#include "XbusWriter.h"

XbusVehicle::XbusVehicle(uint8_t *packet_ptr)
    : XbusPacket(packet_ptr)
    , _hdr(XbusPacket::payload())
{
    offset_payload=sizeof(squawk_t);
}

XbusVehicle::squawk_t XbusVehicle::squawk() const
{
    squawk_t v;
    XbusReader s(_hdr);
    s >> v;
    return v;
}

void XbusVehicle::setSquawk(const squawk_t v)
{
    XbusWriter(_hdr)<<v;
}
