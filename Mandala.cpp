/*
 * Copyright (C) 2011 Aliaksei Stratsilatau <sa@uavos.com>
 *
 * This file is part of the UAV Open System Project
 *  http://www.uavos.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "Mandala.h"
//===========================================================================
Mandala::Mandala()
{
  for (uint i=0;i<maxVars;i++) {
    var_name[i]="";
    var_descr[i]="";
    var_span[i]=0;
    var_round[i]=0;
    var_mult[i]=0;
    var_bytes[i]=0;
    var_size[i]=0;
    var_array[i]=0;
    var_ptr[i]=NULL;//(void*)(& var_void);
    var_type[i]=vt_uint;
    var_bits[i]=0;
  }

  memset(waypoints,0,sizeof(waypoints));
  memset(runways,0,sizeof(runways));

  dl_frcnt=0;
  dl_errcnt=0;
  dl_timestamp=0;
  dl_dt=1000/TELEMETRY_FREQ;
  dl_size=0;
  dl_reset=true;

  derivatives_init=false;
  gps_lat_s=0;
  gps_lon_s=0;

  uint idx=0;
  //------------------------

#define VARDEFX(atype,aname,asize,aspan,abytes,adescr) \
  var_name[idx]=#aname; \
  var_descr[idx]=adescr; \
  var_span[idx]=aspan; \
  var_bytes[idx]=abytes; \
  var_mult[idx]=(aspan>0)?(((abytes==1)?0xFF:((abytes==2)?0xFFFF:((abytes==4)?0xFFFFFFFF:0)))/((aspan==0)?1:(double)aspan)): \
  ( (aspan<0)?(((abytes==1)?0x7F:((abytes==2)?0x7FFF:((abytes==4)?0x7FFFFFFF:0)))/((aspan==0)?1:(double)-aspan)):0 ); \
  var_array[idx]=asize; \
  var_ptr[idx]=(void*)(& aname); \
  var_type[idx]=vt_##atype; \
  name_##aname=#aname; \
  descr_##aname=adescr; \
  var_size[idx]=var_bytes[idx]*var_array[idx]; \
  if(var_type[idx]==vt_vect)var_size[idx]*=3; \
  idx++;

#define VARDEF(atype,aname,aspan,abytes,adescr) \
  VARDEFX(atype,aname,1,aspan,abytes,adescr) \
  aname=0;

#define VARDEFA(atype,aname,asize,aspan,abytes,adescr) \
  VARDEFX(atype,aname,asize,aspan,abytes,adescr) \
  for(uint i=0;i<asize;i++) aname[i]=0; \
  var_ptr[idx-1]=(void*)(aname);

#define MODEDEF(aname,adescr) \
  mode_names[fm##aname]=#aname; \
  mode_descr[fm##aname]=adescr;

#define REGDEF(aname,adescr) \
  reg_names[reg##aname]=#aname; \
  reg_descr[reg##aname]=adescr;

#include "MandalaVars.h"


  idx=idxCFG;
#define CFGDEF(atype,aname,aspan,abytes,around,adescr) \
  VARDEFX(atype,cfg_##aname,1,aspan,abytes,adescr) \
  var_round[idx-1]=around; \
  cfg_##aname=0;

#define CFGDEFA(atype,aname,asize,aspan,abytes,around,adescr) \
  VARDEFX(atype,cfg_##aname,asize,aspan,abytes,adescr) \
  for(uint i=0;i<asize;i++) cfg_##aname[i]=0; \
  var_round[idx-1]=around; \
  var_ptr[idx-1]=(void*)(cfg_##aname);

#include "MandalaVars.h"

//signatures..#__VA_ARGS__
  idx=idxSIG;
#define SIGDEF(aname,adescr,...) \
  VARDEFX(sig,aname,VA_NUM_ARGS(__VA_ARGS__),0,0,adescr) \
  var_sig[idx-1]=aname; \
  const uint8_t aname##_t []={ __VA_ARGS__ };\
  aname[0]=var_array[idx-1]; \
  for(uint i=0;i<aname[0];i++)aname[i+1]=aname##_t[i]; \
  var_bytes[idx-1]=size(aname);\
  var_size[idx-1]=var_bytes[idx-1];

#include "MandalaVars.h"


  //init config signature..
  config[0]=0;
  for (uint i=idxCFG;i<maxVars;i++){
    if (!var_bytes[i])break;
    config[++(config[0])]=i;
  }
  var_bytes[idx_config]=size(config);
  var_size[idx_config]=var_bytes[idx_config];

  //fill strings
  static const char *wt_str_s[wtCnt]={ wt_str_def };
  for(uint i=0;i<wtCnt;i++) wt_str[i]=wt_str_s[i];
  static const char *rwt_str_s[rwtCnt]={ rwt_str_def };
  for(uint i=0;i<rwtCnt;i++) rwt_str[i]=rwt_str_s[i];


  //bitfield strings
#define BITDEF(avarname,abitname,amask,adescr) var_bits[idx_##avarname]++;
#include "MandalaVars.h"
#define BITDEF(avarname,abitname,amask,adescr) \
  idx=(int)(log(amask)/log(2));\
  var_bits_name[idx_##avarname][idx]= #abitname ;\
  var_bits_descr[idx_##avarname][idx]= adescr ;
#include "MandalaVars.h"
}
//===========================================================================
//===========================================================================
uint Mandala::archive(uint8_t *buf,uint size,uint var_idx)
{
  if(!var_ptr[var_idx]){
    fprintf(stderr,"Error: archive unknown var  #%u\n",var_idx);
    return 0;
  }
  //check for special protocol archiveSize
  if(var_idx==idx_downstream)return archive_downstream(buf,size);
  if(var_idx==idx_flightplan)return archive_flightplan(buf,size);

  if(var_size[var_idx]>size){
    fprintf(stderr,"Error: archive %s  #%u (sz: %u, buf: %u)\n",var_name[var_idx],var_idx,var_size[var_idx],size);
    return 0;
  }
  //archive var
  switch (var_type[var_idx]) {
    case vt_uint:
      for (uint ai=0;ai<var_array[var_idx];ai++)
        buf+=archive_u(buf,((uint*)(var_ptr[var_idx]))[ai],var_idx);
      break;
    case vt_float:
      for (uint ai=0;ai<var_array[var_idx];ai++)
        buf+=archive_f(buf,((double*)(var_ptr[var_idx]))[ai],var_idx);
      break;
    case vt_vect:
      for (uint ai=0;ai<var_array[var_idx];ai++)
        for (uint iv=0;iv<3;iv++)
          buf+=archive_f(buf,(((Vect*)(var_ptr[var_idx]))[ai])[iv],var_idx);
      break;
    case vt_sig:
      buf+=archive_sig(buf,size,var_sig[var_idx]);
      break;
    default: break;
  }
  return var_size[var_idx];
}
//=============================================================================
uint Mandala::extract(const uint8_t *buf,uint size)
{
  if(!size)return 0;
  return extract(buf+1,size-1,buf[0]);
}
//=============================================================================
uint Mandala::extract(const uint8_t *buf,uint size,uint var_idx)
{
  if(!var_ptr[var_idx]){
    fprintf(stderr,"Error: extract unknown var  #%u\n",var_idx);
    return 0;
  }
  //check for special protocol archiveSize
  if(var_idx==idx_downstream)return extract_downstream(buf,size);
  if(var_idx==idx_flightplan)return extract_flightplan(buf,size);

  //printf("extracting: %s\n",var_name[var_idx]);
  uint vsz=var_size[var_idx];
  if((!vsz)||(vsz>size)){
    fprintf(stderr,"Error: extract %s  #%u (sz: %u, buf: %u)\n",var_name[var_idx],var_idx,vsz,size);
    return 0;
  }
  //extract var
  switch (var_type[var_idx]) {
    case vt_uint:
      for (uint ai=0;ai<var_array[var_idx];ai++){
        ((uint*)var_ptr[var_idx])[ai]=(uint)extract_u(buf,var_idx);
        buf+=var_bytes[var_idx];
      }
      break;
    case vt_float:
      for (uint ai=0;ai<var_array[var_idx];ai++){
        ((double*)var_ptr[var_idx])[ai]=extract_f(buf,var_idx);
        buf+=var_bytes[var_idx];
      }
      break;
    case vt_vect:
      for (uint ai=0;ai<var_array[var_idx];ai++)
        for (uint iv=0;iv<3;iv++){
          (((Vect*)var_ptr[var_idx])[ai])[iv]=extract_f(buf,var_idx);
          buf+=var_bytes[var_idx];
        }
      break;
    case vt_sig:{
      //printf("extracting sig: %s\n",var_name[var_idx]);
      vsz=extract_sig(buf,size,var_sig[var_idx]);
      buf+=vsz;
    }
    break;
    default: break;
  }
  if(!vsz)return 0; //error from extract_sig
  size-=vsz;
  var_idx++;
  if(size&&(var_idx<idxSIG))return vsz+extract(buf,size,var_idx);
  else return vsz;
}
//=============================================================================
uint Mandala::extract_sig(const uint8_t *buf,uint size,const uint8_t *signature)
{
  uint scnt=signature[0],cnt=0,sz;
  signature++;
  while (scnt--){
    uint var_idx=*signature++;
    sz=var_size[var_idx];
    if(sz>size)sz=0; //error
    sz=extract(buf,sz,var_idx);
    if(!sz)return 0; //error
    buf+=sz;
    cnt+=sz;
    size-=sz;
  }
  return cnt;
}
//=============================================================================
uint Mandala::archive_sig(uint8_t *buf,uint size,const uint8_t *signature)
{
  uint scnt=signature[0];
  signature++;
  uint cnt=0,sz;
  while (scnt--) {
    sz=archive(buf,size,*signature++);
    if(!sz)return 0; //error
    buf+=sz;
    cnt+=sz;
    size-=sz;
  }
  return cnt;
}
//=============================================================================
uint Mandala::archive_f(uint8_t *buf,const double v,const uint var_idx)
{
  double span=var_span[var_idx];
  if(span==0.0)    return archive_u(buf,(uint)round(v<0?0:v),var_idx);
  else if(span>0.0)return archive_u(buf,(uint)round(var_mult[var_idx]*limit(v,0.0,span)),var_idx);
  else return archive_s(buf,(int)round(var_mult[var_idx]*limit(v,-span)),var_idx);
}
//----------------------------------
uint Mandala::archive_u(uint8_t *buf,const uint v,const uint var_idx)
{
  switch (var_bytes[var_idx]) {
    case 1:
      *((uint8_t*)buf)=(uint8_t)limit_u(v,0xFF);
      break;
    case 2:
      *((uint16_t*)buf)=(uint16_t)limit_u(v,0xFFFF);
      break;
    case 4:
      *((uint32_t*)buf)=(uint32_t)limit_u(v,0xFFFFFFFF);
      break;
  }
  return var_bytes[var_idx];
}
//----------------------------------
uint Mandala::archive_s(uint8_t *buf,const int v,const uint var_idx)
{
  switch (var_bytes[var_idx]) {
    case 1:
      *((int8_t*)buf)=(int8_t)limit_s(v,0x7F);
      break;
    case 2:
      *((int16_t*)buf)=(int16_t)limit_s(v,0x7FFF);
      break;
    case 4:
      *((int32_t*)buf)=(int32_t)limit_s(v,0x7FFFFFFF);
      break;
  }
  return var_bytes[var_idx];
}
//----------------------------------
uint Mandala::limit_u(const uint v,const uint vL)
{
  return (v>vL)?vL:v;
}
int Mandala::limit_s(const int v,const int vL)
{
  return (v>(vL))?(vL):((v<(-vL))?(-vL):v);
}
//=============================================================================
double Mandala::extract_f(const uint8_t *buf,const uint var_idx)
{
  double span=var_span[var_idx];
  if(span==0.0) return (double)extract_u(buf,var_idx);
  double v;
  if(span>0.0)v=limit((double)extract_u(buf,var_idx)/var_mult[var_idx],span);
  else v=limit((double)extract_s(buf,var_idx)/var_mult[var_idx],-span);
  double prec=var_round[var_idx];
  if (prec==0.0)return v;
  return round(v/prec)*prec;
}
//----------------------------------
uint Mandala::extract_u(const uint8_t *buf,const uint var_idx)
{
  switch (var_bytes[var_idx]) {
    case 1:
      return *(uint8_t*)buf;
    case 2:
      return *(uint16_t*)buf;
    case 4:
      return *(uint32_t*)buf;
    default: return 0;
  }
}
//----------------------------------
int Mandala::extract_s(const uint8_t *buf,const uint var_idx)
{
  switch (var_bytes[var_idx]) {
    case 1:
      return *(int8_t*)buf;
    case 2:
      return *(int16_t*)buf;
    case 4:
      return *(int32_t*)buf;
    default: return 0;
  }
}
//=============================================================================
uint Mandala::size(const uint8_t *signature)
{
  uint scnt=signature[0];
  signature++;
  uint cnt=0;
  while (scnt--) cnt+=var_size[*signature++];
  return cnt;
}
//===========================================================================
uint Mandala::size(void)
{
  uint cnt=0;
  for (uint i=0;i<maxVars;i++)
    cnt+=var_bytes[i];
  return cnt;
}
//=============================================================================
//=============================================================================
uint Mandala::status_set(uint mask,bool value)
{
  if(value)status|=mask; else status&=~mask;
  return status;
}
uint Mandala::status_clear(uint mask)
{
  return status_set(mask,false);
}
bool Mandala::status_get(uint mask)
{
  return status&mask;
}
//=============================================================================
//=============================================================================
uint Mandala::archive_flightplan(uint8_t *buf,uint bufSize)
{
  const uint wptPackedSz=var_bytes[idx_gps_lat]+var_bytes[idx_gps_lon]+var_bytes[idx_gps_hmsl]+1;
  const uint rwPackedSz=wptPackedSz+var_bytes[idx_NED]*3;
  const uint sz=wptPackedSz*wpcnt+rwPackedSz*rwcnt+2;
  if(bufSize<sz){
    printf("Can't archive flight plan, wrong buffer size (%u) need (%u).\n",bufSize,sz);
    return 0;
  }
  uint8_t *sbuf=buf;
  *buf++=wpcnt;
  *buf++=rwcnt;
  for(uint i=0;i<wpcnt;i++){
    buf+=archive_f(buf,waypoints[i].LLA[0],idx_gps_lat);
    buf+=archive_f(buf,waypoints[i].LLA[1],idx_gps_lon);
    buf+=archive_f(buf,waypoints[i].LLA[2],idx_gps_hmsl);
    *buf++=waypoints[i].type;
  }
  for(uint i=0;i<rwcnt;i++){
    buf+=archive_f(buf,runways[i].LLA[0],idx_gps_lat);
    buf+=archive_f(buf,runways[i].LLA[1],idx_gps_lon);
    buf+=archive_f(buf,runways[i].LLA[2],idx_gps_hmsl);
    *buf++=runways[i].type;
    buf+=archive_f(buf,runways[i].dNED[0],idx_NED);
    buf+=archive_f(buf,runways[i].dNED[1],idx_NED);
    buf+=archive_f(buf,runways[i].dNED[2],idx_NED);
  }
  return buf-sbuf;
}
//=============================================================================
uint Mandala::extract_flightplan(const uint8_t *buf,uint cnt)
{
  const uint wptPackedSz=var_bytes[idx_gps_lat]+var_bytes[idx_gps_lon]+var_bytes[idx_gps_hmsl]+1;
  const uint rwPackedSz=wptPackedSz+var_bytes[idx_NED]*3;
  const uint sz=wptPackedSz*buf[0]+rwPackedSz*buf[1]+2;
  if(cnt!=sz){
    printf("Can't extract flight plan, wrong data size (%u) need (%u).\n",cnt,sz);
    return 0;
  }
  wpcnt=buf[0];
  rwcnt=buf[1];
  const uint8_t *data=buf+2;
  double lat,lon,alt;
  // unpack [cnt] waypoints from [*data]
  for (uint i=0;i<wpcnt;i++) {
    lat=extract_f(data,idx_gps_lat);
    data+=var_bytes[idx_gps_lat];
    lon=extract_f(data,idx_gps_lon);
    data+=var_bytes[idx_gps_lon];
    alt=extract_f(data,idx_gps_hmsl);
    data+=var_bytes[idx_gps_hmsl];
    waypoints[i].type=(_wpt_type)*data++;
    waypoints[i].LLA=Vect(lat,lon,alt);
    waypoints[i].cmd[0]=0;
    //print wpt stats
    const Vect ned=llh2ned(Vect(lat*D2R,lon*D2R,gps_home_hmsl+alt));
    printf("WPT%u NED(%.0f, %.0f, %.0f) %s\n",i+1,ned[0],ned[1],ned[2],wt_str[waypoints[i].type]);
  }
  //unpack runways
  for (uint i=0;i<rwcnt;i++) {
    runways[i].LLA[0]=extract_f(data,idx_gps_lat);
    data+=var_bytes[idx_gps_lat];
    runways[i].LLA[1]=extract_f(data,idx_gps_lon);
    data+=var_bytes[idx_gps_lon];
    runways[i].LLA[2]=extract_f(data,idx_gps_hmsl);
    data+=var_bytes[idx_gps_hmsl];
    runways[i].type=(_rw_type)*data++;
    runways[i].dNED[0]=extract_f(data,idx_NED);
    data+=var_bytes[idx_NED];
    runways[i].dNED[1]=extract_f(data,idx_NED);
    data+=var_bytes[idx_NED];
    runways[i].dNED[2]=extract_f(data,idx_NED);
    data+=var_bytes[idx_NED];
    printf("Runway%u (%s)\n",i+1,rwt_str[runways[i].type]);
  }
  return data-buf;
}
//=============================================================================
uint Mandala::archive_downstream(uint8_t *buf,uint maxSize)
{
  // telemetry stream format:
  // <frcnt>,<timestamp>,<bitsig>,<archived data>,[<bitsig>,<data>...]
  // only modified vars are sent
  // frcnt - inc if packet size>0 and is sent (errors checking)
  // timestamp - every call increments by 10/TELEMETRY_FREQ (time in 100ms units)
  // bitsig - LSBF bitfield, on/off next 8 variables starting from 0
  // skip vars in sig 'dl_filter' as they are calculated by 'extractTelemety'

  //limit maxSize
  if(maxSize>MAX_TELEMETRY)maxSize=MAX_TELEMETRY;

  if(dl_reset){
    memset(dl_reset_mask,0xFF,sizeof(dl_reset_mask));
    dl_reset=false;
  }
  uint8_t *reset_mask_ptr=dl_reset_mask;
  uint cnt=0,mask_cnt=0,mask_cnt_zero=1;
  uint8_t *snapshot=dl_snapshot;
  uint8_t *buf_var=dl_var;
  uint mask=1;
  uint8_t *mask_ptr=buf+2;      //start of data
  uint8_t *ptr=mask_ptr+1;
  *mask_ptr=0;
  for(uint i=0;i<128;i++){
    uint sz=var_size[i];
    if(!sz)break; //valid vars end
    sz=archive(buf_var,sz,i);
    //check if filtered var
    bool filtered=memchr(dl_filter+1,i,dl_filter[0])!=NULL;
    if((*reset_mask_ptr)&mask) (*reset_mask_ptr)&=~mask;
    else{
      filtered|=(dl_frcnt%dl_slow_factor)&&memchr(dl_slow+1,i,dl_slow[0]);
      //test changed
      filtered|=(memcmp(snapshot,buf_var,sz)==0);
    }
    //pack if not filtered
    if(!filtered){
      //check buf overflow
      if((cnt+mask_cnt+mask_cnt_zero+sz+2)>=maxSize)break;
      //post variable
      memcpy(snapshot,buf_var,sz);
      memcpy(ptr,buf_var,sz);
      ptr+=sz;
      cnt+=sz;
      mask_cnt+=mask_cnt_zero;
      mask_cnt_zero=0;
      (*mask_ptr)|=mask;
    }
    snapshot+=sz;
    mask<<=1;
    if(mask&0x0100){
      mask=1;
      mask_cnt_zero++;
      mask_ptr=ptr;
      *mask_ptr=0;
      reset_mask_ptr++;
      ptr++;
    }
  }

  if(!(dl_timestamp%dl_reset_interval)) // periodically send everything
    dl_reset=true;

  /*if(!cnt){
    dl_timestamp+=dl_dt;
    return 0;
  }*/
  buf[0]=dl_frcnt++;
  buf[1]=dl_timestamp/100;
  dl_timestamp+=1000/TELEMETRY_FREQ;
  dl_size=cnt+mask_cnt+2;
  return dl_size;
}
//=============================================================================
uint Mandala::extract_downstream(const uint8_t *buf,uint cnt)
{
  //get timestamps and stats
  dl_size=cnt;
  uint fr_d=((buf[0]-dl_frcnt)&0xFF);
  if(fr_d>1)dl_errcnt+=fr_d-1;
  dl_frcnt+=fr_d;
  dl_dt=((buf[1]-dl_timestamp/100)&0xFF)*100;
  dl_timestamp+=dl_dt;
  if(!dl_dt)dl_dt=100; //default
  //extract data
  uint mask=1;
  const uint8_t *mask_ptr=buf+2;
  const uint8_t *ptr=mask_ptr+1;
  int tcnt=cnt-2;
  uint idx=0;
  do{
    for(uint i=0;i<8;i++){
      if((*mask_ptr)&mask){
        if(tcnt<=0){
          fprintf(stderr,"Error extractTelemety: sz:%u (%s)\n",cnt,var_name[idx]);
          return 0;
        }
        uint sz=extract(ptr,var_size[idx],idx);
        ptr+=sz;
        tcnt-=sz;
      }
      idx++;
      mask<<=1;
    }
    mask_ptr=ptr;
    mask=1;
    ptr++;
    tcnt--;
  }while(tcnt>0);
  // calculate vars filtered by sig dl_filter:
  // gps_velXYZ,gps_accXYZ,gps_crsRate
  //check if to calc derivatives (gps fix)
  if((gps_lat_s!=gps_lat)||(gps_lon_s!=gps_lon)){
    gps_lat_s=gps_lat;
    gps_lon_s=gps_lon;
    calcDGPS();
    //printf("gpsd\n");
  }
  // gps_deltaNED,gps_deltaXYZ,gps_distWPT,gps_distHome,
  calc();
  return ptr-buf;
}
//=============================================================================
//=============================================================================
void Mandala::calcDGPS(const double dt)
{
  // calculate NED
  NED=llh2ned(Vect(gps_lat*D2R,gps_lon*D2R,gps_hmsl));

  Vect theta_r=theta*D2R;
  // calculate frame velocities
  theta_r[2]=gps_course*D2R;
  vXYZ=rotate(gps_vNED,theta_r);

  if(derivatives_init)
    aXYZ=rotate((gps_vNED-last_vNED)/dt,theta_r);
  last_vNED=gps_vNED;

  //calc course rate (derivative)
  if(derivatives_init)
    crsRate=boundAngle(gps_course-last_course)/dt;
  last_course=gps_course;

  derivatives_init=true;
}
//=============================================================================
void Mandala::calc(void)
{
  // vars should be filtered by dl_filter
  dNED=cmd_NED-NED;
  dXYZ=rotate(dNED,theta[2]*D2R);
  dWPT=ned2dist(dNED);
  dHome=ned2dist(NED);
  wpHDG=ned2hdg(dNED);
  homeHDG=ned2hdg(NED,true);
  rwDelta=dWPT*sin((wpHDG+180.0-rwHDG)*D2R);
}
//=============================================================================
double Mandala::ned2hdg(const Vect &ned,bool back)
{
  double v=atan2(ned[1],ned[0])*R2D;
  if(back)return boundAngle(v+180.0);
  else return boundAngle(v);
}
//=============================================================================
const Vect Mandala::lla2ned(const Vect &lla)
{
  return llh2ned(Vect(lla[0]*D2R,lla[1]*D2R,gps_home_hmsl+lla[2]));
}
//=============================================================================
double Mandala::ned2dist(const Vect &ned)
{
  return sqrt(pow(ned[0],2)+pow(ned[1],2));
}
//=============================================================================
double Mandala::boundAngle(double v,double span)
{
  double dspan=span*2.0;
  while (v >= span) v -= dspan;
  while (v < -span) v += dspan;
  return v;
}
//===========================================================================
Vect Mandala::boundAngle(const Vect &v,double span)
{
  return Vect(boundAngle(v[0],span),boundAngle(v[1],span),boundAngle(v[2],span));
}
//===========================================================================
uint Mandala::snap(uint v, uint snapv)
{
  uint vs=v%snapv;
  v-=vs;
  if (vs>=(snapv/2))v+=snapv;
  return v;
}
//===========================================================================
void Mandala::filterValue(double v,double *vLast,double S,double L)
{
  double D=v-*vLast;
  double G=S+S*(D/L)*(D/L);
  if (*vLast==0.0)*vLast=v;
  else {
    if (G>1)G=1;
    else if (G<S)G=S;
    *vLast=*vLast+D*G;
  }
}
//===========================================================================
double Mandala::hyst(double err,double hyst)
{
  if (fabs(err)<=hyst)return 0.0;
  else return err-((err>0.0)?hyst:-hyst);
}
//===========================================================================
double Mandala::limit(const double v,const double vL)
{
  return limit(v,-vL,vL);
}
double Mandala::limit(const double v,const double vMin,const double vMax)
{
  return (v>vMax)?vMax:((v<vMin)?vMin:v);
}
//===========================================================================
const Vect Mandala::rotate(const Vect &v_in,const double theta)
{
  double cos_theta=cos(theta);
  double sin_theta=sin(theta);
  return Vect(v_in[0]*cos_theta+v_in[1]*sin_theta,
              v_in[1]*cos_theta-v_in[0]*sin_theta,
              v_in[2]);
}
//===========================================================================
const Vect Mandala::rotate(const Vect &v_in,const Vect &theta)
{
  const double &phi=theta[0];
  const double &the=theta[1];
  const double &psi=theta[2];
  const double cpsi=cos(psi);
  const double cphi=cos(phi);
  const double ctheta=cos(the);
  const double spsi=sin(psi);
  const double sphi=sin(phi);
  const double stheta=sin(the);

  Vect eulerDC[3]={Vect(cpsi*ctheta,spsi*ctheta,-stheta),
                   Vect(-spsi*cphi + cpsi*stheta*sphi,cpsi*cphi + spsi*stheta*sphi,ctheta*sphi),
                   Vect(spsi*sphi + cpsi*stheta*cphi,-cpsi*sphi + spsi*stheta*cphi,ctheta*cphi)
                  };
  Vect c;
  double s;
  for (uint i=0;i<3;i++) {
    s=0;
    for (uint j=0;j<3;j++) s+=v_in[j]*eulerDC[i][j];
    c[i]=s;
  }
  return c;
}
//=============================================================================
const Vect Mandala::llh2ned(const Vect llh)
{
  return llh2ned(llh,Vect(gps_home_lat*D2R,gps_home_lon*D2R,gps_home_hmsl));
}
//===========================================================================
const Vect Mandala::llh2ned(const Vect llh,const Vect home_llh)
{
  return LLH_dist(home_llh,llh,home_llh[0],home_llh[1]);
}
//===========================================================================
const Vect Mandala::LLH_dist(const Vect &llh1,const Vect &llh2,const double lat,const double lon)
{
  const Vect &ecef1(llh2ECEF(llh1));
  const Vect &ecef2(llh2ECEF(llh2));
  const Vect &diff(ecef2-ecef1);
  return ECEF2Tangent(diff,lat,lon);
}
//=============================================================================
const Vect Mandala::ECEF_dist(const Vect &ecef1,const Vect &ecef2,const double lat,const double lon)
{
  return ECEF2Tangent(ecef1-ecef2,lat,lon);
}
//=============================================================================
const Vect Mandala::ECEF2Tangent(const Vect &ECEF,const double latitude,const double longitude)
{
  double clat=cos(latitude);
  double clon=cos(longitude);
  double slat=sin(latitude);
  double slon=sin(longitude);
  double Re2t[3][3];
  //libmat::Matrix<3,3>   Re2t;

  Re2t[0][0]=-slat*clon;
  Re2t[0][1]=-slat*slon;
  Re2t[0][2]=clat;
  Re2t[1][0]=-slon;
  Re2t[1][1]=clon;
  Re2t[1][2]=0.0;
  Re2t[2][0]=-clat*clon;
  Re2t[2][1]=-clat*slon;
  Re2t[2][2]=-slat;
  Vect c;
  double s;
  for (uint i=0;i<3;i++) {
    s=0;
    for (uint j=0;j<3;j++) s+=ECEF[j]*Re2t[i][j];
    c[i]=s;
  }
  return c;
  //return Re2t*ECEF;
}
//=============================================================================
const Vect Mandala::Tangent2ECEF(const Vect &Local,const double latitude,const double longitude)
{
  double clat=cos(latitude);
  double clon=cos(longitude);
  double slat=sin(latitude);
  double slon=sin(longitude);
  double Rt2e[3][3];
  Rt2e[0][0]=-slat*clon;
  Rt2e[1][0]=-slat*slon;
  Rt2e[2][0]=clat;
  Rt2e[0][1]=-slon;
  Rt2e[1][1]=clon;
  Rt2e[2][1]=0.0;
  Rt2e[0][2]=-clat*clon;
  Rt2e[1][2]=-clat*slon;
  Rt2e[2][2]=-slat;
  Vect c;
  double s;
  for (uint i=0;i<3;i++) {
    s=0;
    for (uint j=0;j<3;j++) s+=Local[j]*Rt2e[i][j];
    c[i]=s;
  }
  return c;
}
//=============================================================================
double Mandala::sqr(double x) {
  return x*x;
}
const Vect Mandala::ECEF2llh(const Vect &ECEF)
{
  double X=ECEF[0];
  double Y=ECEF[1];
  double Z=ECEF[2];
  double f=(C_WGS84_a-C_WGS84_b)/C_WGS84_a;
  double e=sqrt(2*f-f*f);
  double h=0;
  double N=C_WGS84_a;
  Vect llh;
  llh[1]=atan2(Y,X);
  for (int n=0;n<50;++n) {
    double sin_lat=Z/(N*(1-sqr(e))+h);
    llh[0]=atan((Z+e*e*N*sin_lat)/sqrt(X*X+Y*Y));
    N=C_WGS84_a/sqrt(1-sqr(e)*sqr(sin(llh[0])));
    h=sqrt(X*X+Y*Y)/cos(llh[0])-N;
  }
  llh[2]=h;
  return llh;
}
//=============================================================================
const Vect Mandala::llh2ECEF(const Vect &llh)
{
  double f=(C_WGS84_a-C_WGS84_b)/C_WGS84_a;
  double e=sqrt(2*f-f*f);
  double N=C_WGS84_a/sqrt(1-e*e*sqr(sin(llh[0])));
  Vect ECEF;
  ECEF[0]=(N+llh[2])*cos(llh[0])*cos(llh[1]);
  ECEF[1]=(N+llh[2])*cos(llh[0])*sin(llh[1]);
  ECEF[2]=(N*(1-e*e)+llh[2])*sin(llh[0]);
  return ECEF;
}
//=============================================================================
//=============================================================================
//=============================================================================
void Mandala::dump(const uint8_t *ptr,uint cnt,bool hex)
{
  //printf("\n");
  for (uint i=0;i<cnt;i++)printf(hex?"%.2X ":"%u ",*ptr++);
  printf("\n");
}
void Mandala::dump(const Vect &v,const char *str)
{
  printf("%s: %.2f\t%.2f\t%.2f\n",str,v[0],v[1],v[2]);
}
void Mandala::dump(const uint var_idx)
{
  printf("%s: ",var_name[var_idx]);
  switch(var_type[var_idx]){
    case vt_uint:   printf("%u",*((uint*)(var_ptr[var_idx])));break;
    case vt_float: printf("%.2f",*((double*)(var_ptr[var_idx])));break;
    case vt_vect:   printf("(%.2f,%.2f,%.2f)",(*((Vect*)(var_ptr[var_idx])))[0],(*((Vect*)(var_ptr[var_idx])))[1],(*((Vect*)(var_ptr[var_idx])))[2] );break;
    case vt_sig:{
      printf("+sig+\n");
      uint8_t *signature=var_sig[var_idx];
      uint scnt=signature[0];
      signature++;
      while (scnt--) dump(*signature++);
      return;
    }
    default: break;
  }
  printf("\n");
}
//=============================================================================
//=============================================================================
void Mandala::print_report(FILE *stream)
{
  fprintf(stream,"======= Mandala Variables ===================\n");
  fprintf(stream,"#\tVariableName\tDescription\tType\tArray\tSpan\tBytes\n");
  for (uint i=0;i<maxVars;i++) {
    if ((!var_bytes[i])&&(var_type[i]!=vt_sig))continue;
    if (i==idxSIG) fprintf(stream,"== Signature Variables (internal use only) ==\n");
    if (i==idxCFG) fprintf(stream,"======= Configuration Variables =============\n");
    const char *vt="";
    switch(var_type[i]){
      case vt_void:   vt="UNKNOWN";break;
      case vt_uint:   vt="uint";break;
      case vt_float: vt="double";break;
      case vt_vect:   vt="Vect";break;
      case vt_sig:    vt="signature";break;
    }
    fprintf(stream,"%u\t%s\t%s\t%s\t%u\t%g\t%u\n",
           i,
           var_name[i],
           var_descr[i],
           vt,
           var_array[i],
           var_span[i],
           var_bytes[i]
    );
  }
  fprintf(stream,"=========== Bitfields =======================\n");
  fprintf(stream,"VariableName\tBitNumber\tBitName\tDescription\n");
  for (uint i=0;i<maxVars;i++) {
    if (!var_bits[i])continue;
    for(uint ib=0;ib<var_bits[i];ib++)
      fprintf(stream,"%s\t%u\t%s\t%s\n",
             var_name[i],
             ib,
             var_bits_name[i][ib],
             var_bits_descr[i][ib]
      );
  }
  fprintf(stream,"=========== Content of Signature Variables ==\n");
  fprintf(stream,"VariableName\tContent\n");
  for (uint i=0;i<maxVars;i++) {
    if ((var_type[i]!=vt_sig)||(!var_sig[i][0]))continue;
    fprintf(stream,"%s\t",var_name[i]);
    for(uint ib=0;ib<var_sig[i][0];ib++)
      fprintf(stream,"%s,",var_name[var_sig[i][ib]]);
    fprintf(stream,"\n");
  }
}
//=============================================================================
//=============================================================================
//=============================================================================



