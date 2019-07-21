#include "XbusNodeConf.h"

#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"


XbusNodeConf::XbusNodeConf(uint8_t *payload_ptr)
    : XbusBase(payload_ptr)
{
    offset_payload += sizeof(fid_t);
}

XbusNodeConf::fid_t XbusNodeConf::fid()
{
    fid_t d;
    XbusStreamReader(_ptr) >> d;
    return d;
}
void XbusNodeConf::setFid(const fid_t d)
{
    XbusStreamWriter(_ptr) << d;
}

