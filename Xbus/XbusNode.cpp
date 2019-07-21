#include "XbusNode.h"

#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

XbusNode::XbusNode(uint8_t *packet_ptr)
    : XbusPacket(packet_ptr)
    , _hdr(XbusPacket::payload())
{
    offset_payload +=
            std::tuple_size<guid_t>()
            + sizeof(uint8_t);
}

void XbusNode::read(Header &d)
{
    XbusStreamReader s(_hdr);
    s >> d.guid;
    s >> d.cmd;
}
void XbusNode::write(const Header &d)
{
    XbusStreamWriter s(_hdr);
    s << d.guid;
    s << d.cmd;
}
