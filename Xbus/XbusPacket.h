#pragma once
#include "XbusBase.h"

class XbusPacket: protected XbusBase
{
public:
    XbusPacket(uint8_t *packet_ptr);

    typedef uint8_t pid_t;

    enum {
        size_packet = 1024,
    };

    pid_t pid() const;
    void setPid(const pid_t v);
};
