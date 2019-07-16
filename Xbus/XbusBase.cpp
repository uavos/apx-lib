#include "XbusBase.h"

XbusBase::XbusBase(uint8_t *packet_ptr)
    : _ptr(packet_ptr)
    , offset_payload(0)
{

}

bool XbusBase::isValid(const uint16_t packet_size) const
{
    return packet_size>=offset_payload;
}
bool XbusBase::isRequest(const uint16_t packet_size) const
{
    return packet_size==offset_payload; //no payload
}

uint8_t * XbusBase::payload(const uint16_t offset) const
{
    return _ptr + offset_payload + offset;
}

uint16_t XbusBase::payloadOffset() const
{
    return offset_payload;
}
uint16_t XbusBase::payloadSize(const uint16_t packet_size) const
{
    return packet_size>offset_payload?packet_size-offset_payload:0;
}
