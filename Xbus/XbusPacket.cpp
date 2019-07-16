#include "XbusPacket.h"

#include "XbusReader.h"
#include "XbusWriter.h"


XbusPacket::XbusPacket(uint8_t *packet_ptr)
    : XbusBase(packet_ptr)
{
    offset_payload += sizeof(XbusPacket::pid_t);
}

XbusPacket::pid_t XbusPacket::pid() const
{
    pid_t v;
    XbusReader(_ptr) >> v;
    return v;
}

void XbusPacket::setPid(const pid_t v)
{
    XbusWriter(_ptr) << v;
}
