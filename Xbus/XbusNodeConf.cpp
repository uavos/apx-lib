#include "XbusNodeConf.h"

#include "XbusReader.h"
#include "XbusWriter.h"


XbusNodeConf::XbusNodeConf(uint8_t *packet_ptr)
    : XbusNode(packet_ptr)
    , _hdr(XbusNode::payload())
{
    offset_payload += sizeof(fid_t);
}


void XbusNodeConf::read(fid_t &d)
{
    XbusReader(_hdr) >> d;
}
void XbusNodeConf::write(const fid_t d)
{
    XbusWriter(_hdr) << d;
}

void XbusNodeConf::read(conf_info_t &d)
{
    XbusReader s(payload());
    s >> d.cnt;
    s >> d.size;
    s >> d.mn;
}
void XbusNodeConf::write(const conf_info_t &d)
{
    XbusWriter s(payload());
    s << d.cnt;
    s << d.size;
    s << d.mn;
}

void XbusNodeConf::read(ft_script_t &d)
{
    XbusReader s(payload());
    s >> d.size;
    s >> d.code_size;
    s >> d.crc;
    s >> d.name;
}
void XbusNodeConf::write(const ft_script_t &d)
{
    XbusWriter s(payload());
    s << d.size;
    s << d.code_size;
    s << d.crc;
    s << d.name;
}


