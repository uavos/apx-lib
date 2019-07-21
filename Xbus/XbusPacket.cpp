#include "XbusPacket.h"

#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"


XbusPacket::XbusPacket(uint8_t *packet_ptr)
    : XbusBase(packet_ptr)
{
    offset_payload += sizeof(XbusPacket::pid_t);
}

XbusPacket::pid_t XbusPacket::pid() const
{
    pid_t v;
    XbusStreamReader(_ptr) >> v;
    return v;
}

void XbusPacket::setPid(const pid_t v)
{
    XbusStreamWriter(_ptr) << v;
}
