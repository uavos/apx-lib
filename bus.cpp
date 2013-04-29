//==============================================================================
#include <string.h>
#include "bus.h"
#include "drv_base.h"
#include "node_conf.h"
#include "MandalaCore.h"
extern _node_info node;
extern MandalaCore var;
//==============================================================================
Bus::Bus()
: mute(false),if_cnt(0),dl_cnt(0),drv_cnt(0)
{
}
//==============================================================================
void Bus::add_interface(_if_base *iface)
{
  if_obj[if_cnt++]=iface;
}
void Bus::add_downlink(_if_base *iface)
{
  dl_obj[dl_cnt++]=iface;
}
void Bus::add_driver(_drv_base *drv)
{
  drv_obj[drv_cnt++]=drv;
}
//==============================================================================
void Bus::flush(void)
{
  for(uint i=0;i<if_cnt;i++)
    (if_obj[i])->flush();
  for(uint i=0;i<dl_cnt;i++)
    (dl_obj[i])->flush();
}
//==============================================================================
bool Bus::check_packet_sn(_node_sn *sn,bool force_sn)
{
  packet_broadcast=false;
  if(packet_cnt<bus_packet_size_hdr_srv)
    return false;
  if(memcmp(sn,packet.srv.sn,sizeof(_node_sn))==0)
    return true;
  if(force_sn)return false;
  //check broadcast (sn=000...)
  uint8_t *ptr=packet.srv.sn;
  uint sz=sizeof(_node_sn);
  while(sz--)
    if(*ptr++)return false;
  packet_broadcast=true;
  return true;
}
//==============================================================================
bool Bus::task()
{
  bool rv=false;
  for(uint i=0;i<if_cnt;i++)
    if((if_obj[i])->task())rv=true;
  for(uint i=0;i<dl_cnt;i++)
    if((dl_obj[i])->task())rv=true;
  return rv;
}
//==============================================================================
bool Bus::drivers_data(void)
{
  if(packet.id==idx_service){ //service command
    for(uint i=0;i<drv_cnt;i++)
      if((drv_obj[i])->service_all(this)) //for mhx
        return true;
    if(check_packet_sn(&node.sn))
      for(uint i=0;i<drv_cnt;i++)
        if((drv_obj[i])->service(this))
          return true;
    return false;
  }
  bool rv=false;
  if (packet_cnt==bus_packet_size_hdr){ //variable request (empty data payload)
    for(uint i=0;i<drv_cnt;i++)
      if((drv_obj[i])->var_request(this))rv=true;
  }else{
    for(uint i=0;i<drv_cnt;i++)
      if((drv_obj[i])->var_update(this))rv=true;
  }
  return rv;
}
void Bus::drivers_task(void)
{
  for(uint i=0;i<drv_cnt;i++)
    (drv_obj[i])->task(this);
}
void Bus::default_task(void)
{
  if(read()) drivers_data();
  drivers_task();
}
//---------------------------
void Bus::drivers_reset_config(void)
{
  for(uint i=0;i<drv_cnt;i++)
    (drv_obj[i])->reset_config();
}
//==============================================================================
//==============================================================================
bool Bus::write(uint8_t id,const uint8_t *buf,uint cnt)
{
  if(mute) return true;
  packet.id=id;
  if(buf!=packet.data){
    if(cnt>sizeof(packet.data))return true;
    memcpy(packet.data,buf,cnt);
  }
  return write_packet(cnt);
}
//==============================================================================
bool Bus::write_tagged(uint8_t tag,const uint8_t *buf,uint cnt)
{
  if(mute) return true;
  packet.id=idx_tagged;
  packet.tagged.tag=tag;
  if(buf!=packet.tagged.packet){
    if(cnt>sizeof(packet.tagged.packet))return true; //error sim sent
    memcpy(packet.tagged.packet,buf,cnt);
  }
  return write_packet(cnt+bus_packet_size_hdr_tagged-bus_packet_size_hdr);
}
//==============================================================================
bool Bus::write_packet(uint data_cnt)
{
  return write_raw((uint8_t*)&packet,data_cnt+bus_packet_size_hdr);
}
//==============================================================================
bool Bus::write_srv(_node_sn *sn,uint8_t cmd,const uint8_t *buf,uint cnt)
{
  if(cnt>sizeof(packet.srv.data))return true; //error sim sent
  packet.id=idx_service;
  packet.srv.cmd=cmd;
  memcpy(packet.srv.sn,sn,sizeof(_node_sn));
  if(buf!=packet.srv.data) memcpy(packet.srv.data,buf,cnt);
  return write_packet(cnt+(bus_packet_size_hdr_srv-bus_packet_size_hdr));
}
bool Bus::write_srv(uint8_t cmd,const uint8_t *buf,uint cnt)
{
  return write_srv(&node.sn,cmd,buf,cnt);
}
//==============================================================================
bool Bus::write_ldr(uint8_t cmd,const uint8_t *buf,uint cnt)
{
  packet.srv.data[0]=cmd;
  if(buf&&cnt) memcpy(&(packet.srv.data[1]),buf,cnt);
  return write_srv(apc_loader,packet.srv.data,cnt+1);
}
//==============================================================================
bool Bus::write_ack(uint8_t cmd)
{
  return write_srv(apc_ack,&cmd,1);
}
bool Bus::write_ack(void)
{
  return write_ack(packet.srv.cmd);
}
//==============================================================================
bool Bus::write_raw(const uint8_t *buf,uint cnt)
{
  if(!cnt)return false;
  bool rv=true;
  for(uint i=0;i<if_cnt;i++)
    if(!(if_obj[i])->writePacket(buf,cnt))rv=false;
  if(dl_cnt&&memchr(&var.autosend[1],buf[0],var.autosend[0])){
    for(uint i=0;i<dl_cnt;i++)
      if(!(dl_obj[i])->writePacket(buf,cnt))rv=false;
  }
  return rv;
}
//==============================================================================
uint Bus::read(void)
{
  for(uint i=0;i<if_cnt;i++){
    uint cnt=(if_obj[i])->readPacket((uint8_t*)&packet,sizeof(_bus_packet));
    if(cnt<bus_packet_size_hdr)continue;
    packet_cnt=cnt;
    unwrap_packet();
    return process_packet();
  }
  for(uint i=0;i<dl_cnt;i++){
    uint cnt=(dl_obj[i])->readPacket((uint8_t*)&packet,sizeof(_bus_packet));
    if(cnt<bus_packet_size_hdr)continue;
    packet_cnt=cnt;
    packet_tag=0;
    packet_wrapped=idx_uplink;
    return process_packet();
  }
  return 0;
}
//==============================================================================
void Bus::unwrap_packet(void)
{
  //unwrap apckets
  packet_wrapped=0;
  packet_tag=0;
  if(packet_cnt>=(bus_packet_size_hdr+1)){
    switch(packet.id){
      case idx_uplink:
      case idx_sim:
        packet_wrapped=packet.id;
        packet_cnt-=bus_packet_size_hdr;
        memcpy(&packet,packet.data,packet_cnt); //unwrap
        break;
      case idx_tagged:
        if(packet_cnt<=bus_packet_size_hdr_tagged)return; //don't allow empty tagged packets
        packet_wrapped=packet.id;
        packet_tag=packet.tagged.tag;
        packet_cnt-=bus_packet_size_hdr_tagged;
        memcpy(&packet,packet.tagged.packet,packet_cnt); //unwrap
        break;
    }
  }
}
//==============================================================================
uint Bus::process_packet(void)
{
  //preocess service packets only
  if((packet.id!=idx_service)||(!check_packet_sn(&node.sn)))
    return packet_cnt;

  //low level basic requests
  uint dcnt=packet_cnt-bus_packet_size_hdr_srv;
  //broadcast SRV packets
  switch(packet.srv.cmd){
    case apc_info:
      if (dcnt)break; // requests only
      write_srv(packet.srv.cmd,(uint8_t*)&(node.fw),sizeof(_fw_info));
      node.status.can_rxc=0;
      break;
    case apc_nstat:
      if (dcnt)break; // requests only
      memcpy(packet.srv.data,(uint8_t*)&(node.fw.name),sizeof(_node_name));
      memcpy(packet.srv.data+sizeof(_node_name),(uint8_t*)&(node.status),sizeof(_node_status));
      write_srv(packet.srv.cmd,packet.srv.data,sizeof(_node_name)+sizeof(_node_status));
      break;
  }
  return packet_cnt; //forward to drivers
}
//==============================================================================
void Bus::debug(const char *string)
{
  //safe check string length..
  for(uint i=0;i<sizeof(packet.data);i++)
    if(string[i]==0){
      //found i=strlen
      write_srv(apc_debug,(const uint8_t*)string,i);
      return;
    }
}
//==============================================================================
