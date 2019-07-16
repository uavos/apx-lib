#include "XbusBase.h"

XbusBase::XbusBase(uint8_t *packet_ptr)
    : _ptr(packet_ptr)
{

}

bool XbusBase::isValid(const uint16_t packet_size) const
{
    return packet_size>=offset_payload;
}
bool XbusBase::isRequest(const uint16_t packet_size) const
{
    return packet_size==offset_payload;
}
uint8_t * XbusBase::payload(const uint16_t offset) const
{
    return _ptr + offset_payload + offset;
}
