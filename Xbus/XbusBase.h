#pragma once
#include <inttypes.h>
#include <sys/types.h>

class XbusBase
{
public:
    XbusBase(uint8_t *packet_ptr);
    virtual ~XbusBase(){}

    virtual bool isValid(const uint16_t packet_size) const;
    virtual bool isRequest(const uint16_t packet_size) const;

protected:
    uint8_t *_ptr;
    uint16_t offset_payload;

    virtual uint8_t * payload(const uint16_t offset=0) const;
};
