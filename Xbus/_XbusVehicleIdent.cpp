#include "XbusVehicleIdent.h"

#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

XbusVehicleIdent::XbusVehicleIdent(uint8_t *packet_ptr)
    : XbusVehicle(packet_ptr)
{

}

bool XbusVehicleIdent::isValid(const uint16_t packet_size) const
{
    return packet_size==(offset_payload
                         + std::tuple_size<callsign_t>()
                         + std::tuple_size<vuid_t>()
                         + sizeof(vclass_t));
}


void XbusVehicleIdent::read(Ident &d)
{
    XbusStreamReader s(payload());
    s >> d.callsign;
    s >> d.vuid;
    s >> d.vclass;
}
void XbusVehicleIdent::write(const Ident &d)
{
    XbusStreamWriter s(payload());
    s << d.callsign;
    s << d.vuid;
    s << d.vclass;
}
