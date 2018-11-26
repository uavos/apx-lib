//==============================================================================
#include "Mission.h"
#include <dmsg.h>
#include <string.h>
#include <Mandala>
//==============================================================================
Mission::Mission()
  : size(0)
{
}
void Mission::clear(void)
{
  data[0]=0;
  size=0;
}
//==============================================================================
void Mission::reset_current(void)
{
  memset(&current,0,sizeof(current));
}
//==============================================================================
bool Mission::update(uint8_t *buf,uint cnt)
{
  if(cnt>(int)sizeof(data)){
    dmsg("Error in mission, buffer overflow.\n");
    return false;
  }
  //check consistency
  int pos=0;
  while(pos<(int)cnt){
    int npos=next(pos,Mission::mi_stop,buf,cnt);
    if(npos<0)break;
    pos=npos;
  }
  if(buf[size-1] || (pos+1)!=(int)cnt){
    dmsg("Error in mission (%u/%u).\n",pos,cnt);
    return false;
  }
  size=cnt;
  if(buf!=data)memcpy(data,buf,cnt);
  //default home_pos from mission
  if(!(var.status&status_home)){
    bool bSet=false;
    _item_rw *rw0=rw(0);
    if(rw0){
      var.home_pos=Vect(rw0->lat,rw0->lon,rw0->hmsl);
      bSet=true;
    }else{
      _item_wp *wp0=wp(0);
      if(wp0){
        var.home_pos=Vect(wp0->lat,wp0->lon,wp0->alt);
        bSet=true;
      }
    }
    if(bSet){
      if(var.gps_pos.isNull())
        var.gps_pos=var.home_pos;
      //dmsg("Home set from mission.\n");
    }
  }
  return true;
}
//==============================================================================
int Mission::next(int pos, _item_type type)
{
  return next(pos,type,data,size);
}
int Mission::next(int pos, _item_type type, const uint8_t *data_ptr, const uint data_size)
{
  bool bFirst;
  if(pos<0){
    pos=0;
    bFirst=true;
  }else bFirst=false;
  while(pos<(int)data_size){
    if(!bFirst){
      const _item_hdr *hdr=(const _item_hdr*)(data_ptr+pos);
      switch(hdr->type){
        case Mission::mi_wp:
          pos+=sizeof(_item_wp);
          break;
        case Mission::mi_rw:
          pos+=sizeof(_item_rw);
          break;
        case Mission::mi_tw:
          pos+=sizeof(_item_tw);
          break;
        case Mission::mi_pi:
          pos+=sizeof(_item_pi);
          break;
        case Mission::mi_action:
          pos+=action_size(hdr->option);
          break;
        case Mission::mi_restricted:
        case Mission::mi_emergency:
          pos+=area_size(((const _item_area*)hdr)->pointsCnt);
          break;
        default:
          return -1;
      }
    }else bFirst=false;
    const _item_hdr *hdr=(const _item_hdr*)(data_ptr+pos);
    if(type==Mission::mi_stop || type==hdr->type)
      return pos;
  }
  return -1;
}
//==============================================================================
Mission::_item_wp * Mission::wp(int idx)
{
  //find waypoint by idx
  current.wp=NULL;
  int pos=-1,wpidx=0;
  while(1){
    pos=next(pos,Mission::mi_wp);
    if(pos<0)break;
    if(idx!=wpidx++)continue;
    current.wp=(_item_wp*)(data+pos);
    return current.wp;
  }
  return NULL;
}
//==============================================================================
Mission::_item_rw * Mission::rw(int idx)
{
  //find runway by idx
  int pos=-1,rwidx=0;
  while(1){
    pos=next(pos,Mission::mi_rw);
    if(pos<0)break;
    if(idx!=rwidx++)continue;
    return (_item_rw*)(data+pos);
  }
  return NULL;
}
//==============================================================================
Mission::_item_tw * Mission::tw(int idx)
{
  //find taxiway by idx
  int pos=-1,twidx=0;
  while(1){
    pos=next(pos,Mission::mi_tw);
    if(pos<0)break;
    if(idx!=twidx++)continue;
    return (_item_tw*)(data+pos);
  }
  return NULL;
}
//==============================================================================
Mission::_item_pi * Mission::pi(int idx)
{
  //find point of interest by idx
  current.pi=NULL;
  int pos=-1,piidx=0;
  while(1){
    pos=next(pos,Mission::mi_pi);
    if(pos<0)break;
    if(idx!=piidx++)continue;
    current.pi=(_item_pi*)(data+pos);
    return current.pi;
  }
  return NULL;
}
//==============================================================================
Mission::_item_action * Mission::action(int idx)
{
  if(!current.wp)return NULL;
  int pos=(uint8_t*)current.wp-data,aidx=0;
  while(1){
    pos=next(pos,Mission::mi_stop);
    if(pos<0)break;
    _item_action *a=(_item_action*)(data+pos);
    if(a->hdr.type!=mi_action)break;
    if(idx!=aidx++)continue;
    return a;
  }
  return NULL;
}
//==============================================================================
Mission::_item_area * Mission::restricted(int idx)
{
  //find restricted area by idx
  int pos=-1,oidx=0;
  while(1){
    pos=next(pos,Mission::mi_restricted);
    if(pos<0)break;
    if(idx!=oidx++)continue;
    return (_item_area*)(data+pos);
  }
  return NULL;
}
//==============================================================================
Mission::_item_area_point * Mission::area_point(_item_area *area,int idx)
{
  if(idx>=area->pointsCnt)return NULL;
  _item_area_point *p=(_item_area_point*)((uint8_t*)area+sizeof(_item_area));
  return &(p[idx]);
}
//==============================================================================

