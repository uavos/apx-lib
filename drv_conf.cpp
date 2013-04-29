//==============================================================================
#include <stdio.h>
#include <string.h>
#include "drv_conf.h"
//==============================================================================
Conf::Conf(_node_sn *asn,_fw_info *afw,Files *afiles,bool ais_virtual)
  : sn(asn),fw(afw),files(afiles),is_virtual(ais_virtual)
{
}
//==============================================================================
bool Conf::service_all(Bus *bus)
{
  uint dcnt=bus->packet_cnt-bus_packet_size_hdr_srv;
  if(is_virtual && bus->packet.srv.cmd==apc_info){
      if (dcnt)return false; // requests only
      bus->write_srv(sn,bus->packet.srv.cmd,(uint8_t*)fw,sizeof(_fw_info));
      return false; //don't drop
  }
  if(!bus->check_packet_sn(sn,true))return false;
  switch(bus->packet.srv.cmd){
    //apcfg processing
    case apc_conf_inf:
      if (dcnt)break;
      bus->write_srv(sn,bus->packet.srv.cmd,bus->packet.srv.data,conf_inf(bus->packet.srv.data));
      break;
    case apc_conf_cmds:
      if (dcnt)break;
      bus->write_srv(sn,bus->packet.srv.cmd,bus->packet.srv.data,conf_cmd(bus->packet.srv.data));
      break;
    case apc_conf_dsc:
      if (dcnt==0) bus->packet.srv.data[0]=0xFF;
      bus->write_srv(sn,bus->packet.srv.cmd,bus->packet.srv.data,((bus->packet.srv.data[0]==0xFF)?conf_dsc_all(bus->packet.srv.data+1):conf_dsc(bus->packet.srv.data+1,bus->packet.srv.data[0]))+1);
      break;
    case apc_conf_read:
      if (dcnt==0) bus->packet.srv.data[0]=0xFF;
      bus->write_srv(sn,bus->packet.srv.cmd,bus->packet.srv.data,((bus->packet.srv.data[0]==0xFF)?conf_read_all(bus->packet.srv.data+1):conf_read(bus->packet.srv.data+1,bus->packet.srv.data[0]))+1);
      break;
    case apc_conf_write:{
      if(dcnt==0)break;
      uint8_t num=bus->packet.srv.data[0];
      if(dcnt>1){
        if(num==0xFF)conf_write_all(bus->packet.srv.data+1);
        else conf_write(bus->packet.srv.data+1,num);
      }
      bus->write_srv(sn,bus->packet.srv.cmd,bus->packet.srv.data,1);
      if(num==0xFF){
        conf_save();
        bus->write_srv(sn,apc_conf_write,bus->packet.srv.data,0);
      }
    }break;
    case apc_reconf:
      if (dcnt)break;
      bus->write_ack();
      conf_reset();
      bus->drivers_reset_config();
      conf_save();
      bus->packet.srv.data[0]=0xFF;
      bus->write_srv(sn,apc_conf_read,bus->packet.srv.data,conf_read_all(bus->packet.srv.data+1)+1);
      break;
  }
  return false;
}
//==============================================================================
void Conf::conf_save(void)
{
}
//==============================================================================
void Conf::conf_reset(void)
{
}
//==============================================================================
uint Conf::conf_dsc_all(uint8_t *buf)
{
  uint num=0,sz=0;
  while(1){
    uint s=conf_dsc(buf,num);
    if(!s)return sz;
    buf+=s;
    sz+=s;
    num++;
  }
  return sz;
}
uint Conf::conf_read_all(uint8_t *buf)
{
  uint num=0,sz=0;
  while(1){
    uint s=conf_read(buf,num);
    if(!s)return sz;
    buf+=s;
    sz+=s;
    num++;
  }
  return sz;
}
uint Conf::conf_write_all(uint8_t *buf)
{
  uint num=0,sz=0;
  while(1){
    uint s=conf_write(buf,num);
    if(!s)return sz;
    buf+=s;
    sz+=s;
    num++;
  }
  return sz;
}
//==============================================================================
uint Conf::conf_inf(uint8_t *buf)
{
  _conf_inf inf;
  inf.size=0;
  inf.cnt=0;
  while(1){
    uint s=conf_read(buf,inf.cnt);
    if(!s)break;
    inf.size+=s;
    inf.cnt++;
  }
  //get magic number
  uint32_t mn=0;
  uint8_t *ptr;
  uint num=0;
  while(1){
    uint s=conf_dsc(buf,num++);
    if(!s)break;
    ptr=buf;
    for(uint i=0;i<s;i++)
      mn+=ptr[i]^i;
  }
  ptr=(uint8_t*)sn;
  for(uint i=0;i<sizeof(_node_sn);i++)
    mn+=ptr[i]^i;
  ptr=(uint8_t*)fw;
  for(uint i=0;i<sizeof(_fw_info);i++)
    mn+=ptr[i]^i;
  uint cnt=conf_cmd(buf);
  ptr=buf;
  for(uint i=0;i<cnt;i++)
    mn+=ptr[i]^i;
  inf.mn=mn;
  memcpy(buf,&inf,sizeof(inf));
  return sizeof(inf);
}
//-------------------------------------
uint Conf::conf_cmd(uint8_t *buf)
{
  return 0;
}
//-------------------------------------
uint Conf::conf_dsc(uint8_t *buf,uint8_t num)
{
  return 0;
}
//-------------------------------------
uint Conf::conf_read(uint8_t *buf,uint8_t num)
{
  return 0;
}
//-------------------------------------
uint Conf::conf_write(uint8_t *buf,uint8_t num)
{
  return 0;
}
//==============================================================================
