#pragma once
#include "XbusNode.h"

class XbusNodeConf: protected XbusNode
{
public:
    XbusNodeConf(uint8_t *packet_ptr);

};
