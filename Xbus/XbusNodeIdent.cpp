#include "XbusNodeIdent.h"

#include "XbusReader.h"
#include "XbusWriter.h"


XbusNodeIdent::XbusNodeIdent(uint8_t *packet_ptr)
    : XbusNode(packet_ptr)
{
}

bool XbusNodeIdent::isValid(const uint16_t packet_size) const
{
    return packet_size==(offset_payload
                         + std::tuple_size<name_t>()
                         + std::tuple_size<version_t>()
                         + std::tuple_size<hardware_t>()
                         + sizeof(flags_t));
}

void XbusNodeIdent::read(Ident &d)
{
    XbusReader s(payload());
    s >> d.name;
    s >> d.version;
    s >> d.hardware;
    s >> d.flags.raw;
}
void XbusNodeIdent::write(const Ident &d)
{
    XbusWriter s(payload());
    s << d.name;
    s << d.version;
    s << d.hardware;
    s << d.flags.raw;
}
