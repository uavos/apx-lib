#pragma once
#include "XbusBase.h"

#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

class XbusNodeConf: public XbusBase
{
public:
    XbusNodeConf(uint8_t *payload_ptr);

    typedef uint8_t   fid_t;  //field id

    //readers and writers
    fid_t fid();
    void setFid(const fid_t d);
};
