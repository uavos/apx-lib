#include "XbusVehicle.h"

#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

XbusVehicle::XbusVehicle(uint8_t *packet_ptr)
    : XbusPacket(packet_ptr)
    , _hdr(XbusPacket::payload())
{
    offset_payload += sizeof(squawk_t);
}

XbusVehicle::squawk_t XbusVehicle::squawk() const
{
    squawk_t v;
    XbusStreamReader s(_hdr);
    s >> v;
    return v;
}

void XbusVehicle::setSquawk(const squawk_t v)
{
    XbusStreamWriter(_hdr)<<v;
}
