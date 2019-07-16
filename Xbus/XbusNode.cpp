#include "XbusNode.h"

#include "XbusReader.h"
#include "XbusWriter.h"

XbusNode::XbusNode(uint8_t *packet_ptr)
    : XbusPacket(packet_ptr)
    , _hdr(XbusPacket::payload())
{
    offset_payload +=
            std::tuple_size<guid_t>()
            + sizeof(uint8_t);
}

void XbusNode::read(Request &d)
{
    XbusReader s(_hdr);
    s >> d.guid;
    s >> d.cmd;
}
void XbusNode::write(const Request &d)
{
    XbusWriter s(_hdr);
    s << d.guid;
    s << d.cmd;
}
