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
  : MandalaCore()
{
  for (uint i=0;i<256;i++) {
    var_name[i]="";
    var_descr[i]="";
    var_bits[i]=0;
    var_span[i]=0;
    var_ptr[i]=NULL;
    var_type[i]=vt_void;
  }

  memset(waypoints,0,sizeof(waypoints));
  memset(runways,0,sizeof(runways));
  memset(&cfg,0,sizeof(cfg));

  dl_frcnt=0;
  dl_errcnt=0;
  dl_timestamp=0;
  dl_ts=0;
  dl_Pdt=0;
  dl_size=0;
  dl_reset=true;

  gps_lat_s=0;
  gps_lon_s=0;
  //------------------------

#define VARDEF(atype,aname,aspan,abytes,atbytes,adescr) \
  var_ptr[idx_##aname]=(void*)(& aname); \
  var_type[idx_##aname]=vt_##atype; \
  var_size[idx_##aname]=((abytes)*((vt_##atype==vt_vect)?3:1)); \
  var_span[idx_##aname]=aspan; \
  var_name[idx_##aname]=#aname; \
  var_descr[idx_##aname]=adescr; \
  aname=0;
#include "MandalaVars.h"

#define SIGDEF(aname,adescr,...) \
  var_ptr[idx_##aname]=(void*)(& aname); \
  var_type[idx_##aname]=vt_sig; \
  var_size[idx_##aname]=sig_size(aname); \
  var_name[idx_##aname]=#aname; \
  var_descr[idx_##aname]=adescr;
#include "MandalaVars.h"



#define REGDEF(aname,adescr) \
  reg_names[reg##aname]=#aname; \
  reg_descr[reg##aname]=adescr;

#define CFGDEFA(atype,aname,asize,aspan,abytes,around,adescr) \
  cfg.var_ptr[idx_cfg_##aname]=& (cfg. aname ); \
  cfg.var_type[idx_cfg_##aname]=vt_##atype; \
  cfg.var_array[idx_cfg_##aname]=asize; \
  cfg.var_size[idx_cfg_##aname]=((asize)*(abytes)*((vt_##atype==vt_vect)?3:1)); \
  cfg.var_name[idx_cfg_##aname]=#aname; \
  cfg.var_descr[idx_cfg_##aname]=adescr; \
  cfg.var_round[idx_cfg_##aname]=around; \
  cfg.var_span[idx_cfg_##aname]=aspan;

#define CFGDEF(atype,aname,aspan,abytes,around,adescr) \
  CFGDEFA(atype,aname,1,aspan,abytes,around,adescr)

#include "MandalaVarsAP.h"

  //set config signature size
  for(uint i=0;i<cfgCnt;i++)
    var_size[idx_config]+=cfg.var_size[i];


  //fill strings
  static const char *wt_str_s[wtCnt]={ wt_str_def };
  for(uint i=0;i<wtCnt;i++) wt_str[i]=wt_str_s[i];
  static const char *rwt_str_s[rwtCnt]={ rwt_str_def };
  for(uint i=0;i<rwtCnt;i++) rwt_str[i]=rwt_str_s[i];
  static const char *rwa_str_s[rwaCnt]={ rwa_str_def };
  for(uint i=0;i<rwaCnt;i++) rwa_str[i]=rwa_str_s[i];


  //bitfield strings
  uint bit_var=256,bit_idx=0;
  memset(var_bits_mask,0,sizeof(var_bits_mask));
  memset(var_bits_name,0,sizeof(var_bits_name));
  memset(var_bits_descr,0,sizeof(var_bits_descr));
//  bidx=(int)(log(amask)/log(2));
#define BITDEF(avarname,abitname,amask,adescr) var_bits[idx_##avarname]++;
#include "MandalaVars.h"

#define BITDEF(avarname,abitname,amask,adescr) \
  if(bit_var!=idx_##avarname){bit_var=idx_##avarname;bit_idx=0;}\
  var_bits_mask[idx_##avarname][bit_idx]= avarname##_##abitname ;\
  var_bits_name[idx_##avarname][bit_idx]= #abitname ;\
  var_bits_descr[idx_##avarname][bit_idx]= adescr ;\
  bit_idx++;
#include "MandalaVars.h"
}
//===========================================================================
//===========================================================================
uint Mandala::archive(uint8_t *buf,uint size,uint var_idx)
{
  //check for special protocol archiveSize
  if(var_idx==idx_config)return archive_config(buf,size);
  if(var_idx==idx_downstream)return archive_downstream(buf,size);
  if(var_idx==idx_downstream_hd)return archive_downstream_hd(buf,size);
  if(var_idx==idx_flightplan)return archive_flightplan(buf,size);

  void *ptr=var_ptr[var_idx];
  if(!ptr){
    fprintf(stderr,"Error: archive unknown var  #%u\n",var_idx);
    return 0;
  }
  if(var_size[var_idx]>size){
    fprintf(stderr,"Error: archive %s  #%u (sz: %u, buf: %u)\n",var_name[var_idx],var_idx,var_size[var_idx],size);
    return 0;
  }
  //archive var
  uint cnt=do_archive(buf,var_idx);
  if(!cnt)return 0;
  return cnt;//var_size[var_idx];
}
//=============================================================================
uint Mandala::extract(uint8_t *buf,uint size)
{
  if(!size)return 0;
  return extract(buf+1,size-1,buf[0]);
}
//=============================================================================
uint Mandala::extract(uint8_t *buf,uint size,uint var_idx)
{
  //check for special protocol archiveSize
  if(var_idx==idx_config)return extract_config(buf,size);
  if(var_idx==idx_downstream)return extract_downstream(buf,size);
  if(var_idx==idx_downstream_hd)return extract_downstream_hd(buf,size);
  if(var_idx==idx_flightplan)return extract_flightplan(buf,size);
  if(var_idx==idx_msg)return 1; //nothing to do
  if(var_idx==idx_service)return 1; //nothing to do
  if(var_idx==idx_setb)return extract_setb(buf,size);
  if(var_idx==idx_clrb)return extract_clrb(buf,size);

  void *ptr=var_ptr[var_idx];
  if(!ptr){
    fprintf(stderr,"Error: extract unknown var  #%u\n",var_idx);
    return 0;
  }
  //printf("extracting: %s\n",var_name[var_idx]);
  uint vsz=var_size[var_idx];
  if((!alt_bytecnt)&&((!vsz)||(vsz>size))){
    fprintf(stderr,"Error: extract %s #%u (sz: %u, buf: %u)\n",var_name[var_idx],var_idx,vsz,size);
    return 0;
  }
  uint cnt=do_extract(buf,size,var_idx);
  if(!cnt)return 0; //error
  //if(var_idx==20){
    //printf("cnt:%u var:%u %.2f ",cnt,var_idx,Ve);
    //dump(idx_theta);
    //dump(buf,size);
  //}
  vsz=cnt;
  if(size<vsz){
    fprintf(stderr,"Error: telemetry tail extra.\n");
    return cnt;
  }
  size-=vsz;
  var_idx++;
  if(size&&(var_idx>idxPAD)&&(var_idx<idx_vars_top))return vsz+extract(buf+vsz,size,var_idx);
  else return vsz;
}
//=============================================================================
//=============================================================================
uint Mandala::sig_size(_var_signature signature)
{
  uint scnt=signature[0];
  signature++;
  uint cnt=0;
  while (scnt--) cnt+=var_size[*signature++];
  return cnt;
}
//=============================================================================
//=============================================================================
//=============================================================================
typedef _var_float _var_float_array [];
typedef _var_uint _var_uint_array [];
typedef _var_vect _var_vect_array [];
uint Mandala::archive_config(uint8_t *buf,uint bufSize)
{
  uint sz=var_size[idx_config];
  if(bufSize<sz){
    fprintf(stderr,"Can't archive config, wrong buffer size (%u) need (%u).\n",bufSize,sz);
    return 0;
  }
  for(uint i=0;i<cfgCnt;i++){
    fill_config_vdsc(buf,i);
    uint asz=cfg.var_array[i];
    if(asz>1){
      switch(cfg.var_type[i]){
        case vt_float:{
          _var_float_array *v=((_var_float_array*)vdsc.ptr);
          for(uint ai=0;ai<asz;ai++){
            vdsc.ptr=&((*v)[ai]);
            do_archive_vdsc();
          }
        }break;
        case vt_vect:{
          _var_vect_array *v=((_var_vect_array*)vdsc.ptr);
          for(uint ai=0;ai<asz;ai++){
            vdsc.ptr=&((*v)[ai][0]);
            do_archive_vdsc();
          }
        }break;
        case vt_uint:{
          _var_uint_array *v=((_var_uint_array*)vdsc.ptr);
          for(uint ai=0;ai<asz;ai++){
            vdsc.ptr=&((*v)[ai]);
            do_archive_vdsc();
          }
        }break;
      }
    }else do_archive_vdsc();
    buf+=vdsc.size;
  }
  return sz;
}
//=============================================================================
uint Mandala::extract_config(uint8_t *buf,uint cnt)
{
  uint sz=var_size[idx_config];
  if(cnt!=sz){
    fprintf(stderr,"Can't extract config, wrong data size (%u) need (%u).\n",cnt,sz);
    return 0;
  }
  for(uint i=0;i<cfgCnt;i++){
    fill_config_vdsc(buf,i);
    uint asz=cfg.var_array[i];
    if(asz>1){
      vdsc.size/=asz;
      switch(cfg.var_type[i]){
        case vt_float:{
          _var_float_array *v=((_var_float_array*)vdsc.ptr);
          for(uint ai=0;ai<asz;ai++){
            vdsc.ptr=&((*v)[ai]);
            uint vsz=do_extract_vdsc(cnt);
            buf+=vsz;
            cnt-=vsz;
          }
        }break;
        case vt_vect:{
          _var_vect_array *v=((_var_vect_array*)vdsc.ptr);
          for(uint ai=0;ai<asz;ai++){
            vdsc.ptr=&((*v)[ai][0]);
            uint vsz=do_extract_vdsc(cnt);
            buf+=vsz;
            cnt-=vsz;
          }
        }break;
        case vt_uint:{
          _var_uint_array *v=((_var_uint_array*)vdsc.ptr);
          for(uint ai=0;ai<asz;ai++){
            vdsc.ptr=&((*v)[ai]);
            uint vsz=do_extract_vdsc(cnt);
            buf+=vsz;
            cnt-=vsz;
          }
        }break;
      }
    }else{
      uint vsz=do_extract_vdsc(cnt);
      buf+=vsz;
      cnt-=vsz;
    }
  }
  return sz;
}
//=============================================================================
void Mandala::fill_config_vdsc(uint8_t *buf,uint i)
{
  //_variable_descriptor
  #define CFGDEF(atype,aname,aspan,abytes,around,adescr) CFGDEFA(atype,aname,1,aspan,abytes,around,adescr)
  #define CFGDEFA(atype,aname,asize,aspan,abytes,around,adescr) \
  case idx_cfg_##aname:\
    vdsc.ptr=&(cfg. aname );\
    vdsc.sbytes=(aspan<0)?(-abytes):(abytes);\
    vdsc.span=(aspan<0)?(-aspan):(aspan);\
    vdsc.type=vt_##atype;\
    vdsc.max=(aspan>0)?(((abytes==1)?0xFF:((abytes==2)?0xFFFF:((abytes==4)?0xFFFFFFFF:0)))): \
                      ( (aspan<0)?((abytes==1)?0x7F:((abytes==2)?0x7FFF:((abytes==4)?0x7FFFFFFF:0))):0 );\
    vdsc.size=((asize)*(abytes)*((vt_##atype==vt_vect)?3:1));\
    vdsc.prec1000=around*1000.0;\
    break;
    vdsc.buf=buf;
    switch (i) {
      #include "MandalaVarsAP.h"
      default:
        return;
    }
}
//=============================================================================
//=============================================================================
uint Mandala::archive_flightplan(uint8_t *buf,uint bufSize)
{
  const uint wptPackedSz=(var_size[idx_gps_lat])+(var_size[idx_gps_lon])+(var_size[idx_gps_hmsl])+1;
  const uint rwPackedSz=wptPackedSz+(var_size[idx_NED])+5;
  const uint sz=wptPackedSz*wpcnt+rwPackedSz*rwcnt+2;
  if(bufSize<sz){
    fprintf(stderr,"Can't archive flight plan, wrong buffer size (%u) need (%u).\n",bufSize,sz);
    return 0;
  }
  uint8_t *sbuf=buf;
  *buf++=wpcnt;
  *buf++=rwcnt;
  _var_float gps_lat_save=gps_lat,gps_lon_save=gps_lon,gps_hmsl_save=gps_hmsl;
  _var_vect NED_save=NED;
  for(uint i=0;i<wpcnt;i++){
    gps_lat=waypoints[i].LLA[0];
    gps_lon=waypoints[i].LLA[1];
    gps_hmsl=waypoints[i].LLA[2];
    buf+=archive(buf,bufSize,idx_gps_lat);
    buf+=archive(buf,bufSize,idx_gps_lon);
    buf+=archive(buf,bufSize,idx_gps_hmsl);
    *buf++=waypoints[i].type;
  }
  for(uint i=0;i<rwcnt;i++){
    gps_lat=runways[i].LLA[0];
    gps_lon=runways[i].LLA[1];
    gps_hmsl=runways[i].LLA[2];
    NED[0]=runways[i].dNED[0];
    NED[1]=runways[i].dNED[1];
    NED[2]=runways[i].dNED[2];
    buf+=archive(buf,bufSize,idx_gps_lat);
    buf+=archive(buf,bufSize,idx_gps_lon);
    buf+=archive(buf,bufSize,idx_gps_hmsl);
    buf+=archive(buf,bufSize,idx_NED);
    *buf++=runways[i].rwType;
    *buf++=runways[i].appType;
    *buf++=limit(runways[i].distApp/10,0,2550);
    *buf++=limit(runways[i].altApp/10,0,2550);
    *buf++=limit(runways[i].distTA/10,0,2550);
    *buf++=limit(runways[i].altTA/10,0,2550);
  }
  gps_lat=gps_lat_save;
  gps_lon=gps_lon_save;
  gps_hmsl=gps_hmsl_save;
  NED=NED_save;
  return buf-sbuf;
}
//=============================================================================
uint Mandala::extract_flightplan(uint8_t *buf,uint cnt)
{
  const uint wptPackedSz=(var_size[idx_gps_lat])+(var_size[idx_gps_lon])+(var_size[idx_gps_hmsl])+1;
  const uint rwPackedSz=wptPackedSz+(var_size[idx_NED])+5;
  const uint sz=wptPackedSz*buf[0]+rwPackedSz*buf[1]+2;
  if(cnt!=sz){
    fprintf(stderr,"Can't extract flight plan, wrong data size (%u) need (%u).\n",cnt,sz);
    return 0;
  }
  wpcnt=buf[0];
  rwcnt=buf[1];
  uint8_t *data=buf+2;
  _var_float gps_lat_save=gps_lat,gps_lon_save=gps_lon,gps_hmsl_save=gps_hmsl;
  _var_vect NED_save=NED;
  double lat,lon,alt;
  // unpack [cnt] waypoints from [*data]
  for (uint i=0;i<wpcnt;i++) {
    data+=extract(data,var_size[idx_gps_lat],idx_gps_lat);
    data+=extract(data,var_size[idx_gps_lon],idx_gps_lon);
    data+=extract(data,var_size[idx_gps_hmsl],idx_gps_hmsl);
    lat=gps_lat;
    lon=gps_lon;
    alt=gps_hmsl;
    waypoints[i].type=(_wpt_type)*data++;
    waypoints[i].LLA=_var_vect(lat,lon,alt);
    waypoints[i].cmd[0]=0;
    //print wpt stats
    //const _var_vect ned=llh2ned(_var_vect(lat*D2R,lon*D2R,gps_home_hmsl+alt));
    //printf("WPT%u NED(%.0f, %.0f, %.0f) %s\n",i+1,ned[0],ned[1],ned[2],wt_str[waypoints[i].type]);
  }
  //unpack runways
  for (uint i=0;i<rwcnt;i++) {
    data+=extract(data,var_size[idx_gps_lat],idx_gps_lat);
    data+=extract(data,var_size[idx_gps_lon],idx_gps_lon);
    data+=extract(data,var_size[idx_gps_hmsl],idx_gps_hmsl);
    data+=extract(data,var_size[idx_NED],idx_NED);
    runways[i].LLA[0]=gps_lat;
    runways[i].LLA[1]=gps_lon;
    runways[i].LLA[2]=gps_hmsl;
    runways[i].rwType=(_rw_type)*data++;
    runways[i].appType=(_rw_app)*data++;
    runways[i].distApp=*data++*10;
    runways[i].altApp=*data++*10;
    runways[i].distTA=*data++*10;
    runways[i].altTA=*data++*10;
    runways[i].dNED=NED;
    //printf("Runway%u (%s), NED(%.0f,%.0f,%.0f)\n",i+1,rwt_str[runways[i].type],runways[i].dNED[0],runways[i].dNED[1],runways[i].dNED[2]);
  }
  gps_lat=gps_lat_save;
  gps_lon=gps_lon_save;
  gps_hmsl=gps_hmsl_save;
  NED=NED_save;
  return data-buf;
}
//=============================================================================
uint Mandala::archive_downstream_hd(uint8_t *buf,uint maxSize)
{
  // telemetry stream format:
  // <uav>,<timestampL>,<timestampH>,<bitsig>,<archived data>,[<bitsig>,<data>...]
  // only modified vars are sent
  // uav - dynamically assigned UAV ID
  // timestamp - time in 10ms units
  // bitsig - LSBF bitfield, on/off next 8 variables starting from idxPAD
  // skip vars in sig 'dl_filter' as they are calculated by 'extractTelemety'

  //limit maxSize
  if(maxSize>MAX_TELEMETRY)maxSize=MAX_TELEMETRY;

  //watch alt_bytecnt changes
  if(dl_hd_save!=alt_bytecnt){
    dl_hd_save=alt_bytecnt;
    dl_reset=true;
  }
  
  //pack header
  *buf++=id.id;
  uint ts=dl_timestamp/10;
  *buf++=ts;
  *buf++=ts>>8;
  
  //pack stream
  if(dl_reset){
    memset(dl_reset_mask,0xFF,sizeof(dl_reset_mask));
    dl_reset=false;
  }
  uint8_t *reset_mask_ptr=dl_reset_mask;
  uint cnt=0,mask_cnt=0,mask_cnt_zero=1;
  uint8_t *snapshot=dl_snapshot;
  uint mask=1;
  uint8_t *mask_ptr=buf;      //start of data
  uint8_t *ptr=mask_ptr+1;
  *mask_ptr=0;
  for(uint i=idxPAD;i<(idxPAD+128);i++){
    uint sz=var_size[i];
    if(!sz)break; //valid vars end
    sz=archive(dl_var,sizeof(dl_var),i);
    //check if filtered var
    bool filtered=memchr(dl_filter+1,i,dl_filter[0])!=NULL;
    if((*reset_mask_ptr)&mask) (*reset_mask_ptr)&=~mask; //remove reset flag
    else{
      //test changed
      filtered|=(memcmp(snapshot,dl_var,sz)==0);
    }
    //pack if not filtered
    if(!filtered){
      //check buf overflow
      if((cnt+mask_cnt+mask_cnt_zero+sz+2)>=maxSize){
        (*reset_mask_ptr)|=mask; //recover reset flag (send later)
        break;
      }
      //post variable
      memcpy(snapshot,dl_var,sz);
      memcpy(ptr,dl_var,sz);
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
  alt_bytecnt=false;

  if(!(dl_timestamp%dl_reset_interval)) // periodically send everything
    dl_reset=true;

  dl_timestamp+=dl_period?dl_period:(1000/TELEMETRY_FREQ);
  dl_size=cnt+mask_cnt+3;
  return dl_size;
}
uint Mandala::archive_downstream(uint8_t *buf,uint maxSize)
{
  alt_bytecnt=true;    //for var pack size
  return archive_downstream_hd(buf,maxSize);
}
//=============================================================================
uint Mandala::extract_downstream_hd(uint8_t *buf,uint cnt)
{
  //header
  dl_size=cnt;
  dl_id=*buf++;
  dl_frcnt++;
  uint16_t ts=*buf++;
  ts|=(*buf++)<<8;
  uint16_t Pdt=ts-dl_ts;
  dl_ts=ts;
  dl_timestamp+=Pdt*10;
  bool bErr=dl_Pdt!=Pdt; //delta different
  dl_Pdt=Pdt;
  if(bErr&&(bErr==dl_e)) dl_errcnt++;
  dl_e=bErr;
  
  //extract data
  uint tcnt=2;
  if(cnt>2){
    tcnt=extract_stream(buf,cnt-2);
    if(!tcnt){
      fprintf(stderr,"Error extract_downstream");
    }
  }
  alt_bytecnt=false;
  // calculate vars filtered by sig dl_filter
  //check if to calc derivatives (gps fix)
  if((gps_lat_s!=gps_lat)||(gps_lon_s!=gps_lon)){
    gps_lat_s=gps_lat;
    gps_lon_s=gps_lon;
    NED=llh2ned(_var_vect(gps_lat*D2R,gps_lon*D2R,gps_hmsl));
  }
  // gps_deltaNED,gps_deltaXYZ,gps_distWPT,gps_distHome,
  calc();
  return tcnt;
}
uint Mandala::extract_downstream(uint8_t *buf,uint cnt)
{
  alt_bytecnt=true;    //for var pack size
  return extract_downstream_hd(buf,cnt);
}
//=============================================================================
uint Mandala::extract_setb(uint8_t *buf,uint cnt)
{
  uint var_idx=buf[0];
  bool bChk=true;
  bChk=bChk&&(var_type[var_idx]==vt_uint);
  bChk=bChk&&(var_bits[var_idx]);
  bChk=bChk&&(var_bits_mask[var_idx][0]);
  bChk=bChk&&(cnt==(1+var_size[var_idx]));
  _var_uint *ptr;
  _var_uint old_v;
  if(bChk){
    ptr=(_var_uint*)var_ptr[var_idx];
    old_v=*ptr;
    bChk=bChk&&extract(buf+1,cnt-1,var_idx);
  }
  if(!bChk){
    fprintf(stderr,"Can't extract 'set_bit'. Integrity check error.");
    return 0;
  }
  *ptr|=old_v;
  return cnt;
}
//-----------------------------------------------------------------------------
uint Mandala::extract_clrb(uint8_t *buf,uint cnt)
{
  uint var_idx=buf[0];
  bool bChk=true;
  bChk=bChk&&(var_type[var_idx]==vt_uint);
  bChk=bChk&&(var_bits[var_idx]);
  bChk=bChk&&(var_bits_mask[var_idx][0]);
  bChk=bChk&&(cnt==(1+var_size[var_idx]));
  _var_uint *ptr;
  _var_uint old_v;
  if(bChk){
    ptr=(_var_uint*)var_ptr[var_idx];
    old_v=*ptr;
    bChk=bChk&&extract(buf+1,cnt-1,var_idx);
  }
  if(!bChk){
    fprintf(stderr,"Can't extract 'clr_bit'. Integrity check error.");
    return 0;
  }
  *ptr=old_v&~(*ptr);
  return cnt;
}
//=============================================================================
//=============================================================================
//=============================================================================
void Mandala::calc(void)
{
  // vars should be filtered by dl_filter
  dN=cmd_N-NED[0];
  dE=cmd_E-NED[1];
  dAlt=cmd_altitude-altitude;
  _var_vect dNED(dN,dE,-dAlt);
  dXYZ=rotate(dNED,theta[2]*D2R);
  dWPT=ned2dist(dNED);
  dHome=ned2dist(NED);
  wpHDG=ned2hdg(dNED);
  homeHDG=ned2hdg(NED,true);
  rwDelta=dWPT*sin((wpHDG+180.0-rwHDG)*D2R)-rwAdj;
  rwDV=rotate(gps_vNED,rwHDG*D2R)[1];
  gSpeed=ned2dist(gps_vNED);
}
//=============================================================================
double Mandala::ned2hdg(const _var_vect &ned,bool back)
{
  double v=atan2(ned[1],ned[0])*R2D;
  if(back)return boundAngle(v+180.0);
  else return boundAngle(v);
}
//=============================================================================
const _var_vect Mandala::lla2ned(const _var_vect &lla)
{
  return llh2ned(_var_vect(lla[0]*D2R,lla[1]*D2R,gps_home_hmsl+lla[2]));
}
//=============================================================================
double Mandala::ned2dist(const _var_vect &ned)
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
_var_vect Mandala::boundAngle(const _var_vect &v,double span)
{
  return _var_vect(boundAngle(v[0],span),boundAngle(v[1],span),boundAngle(v[2],span));
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
const _var_vect Mandala::rotate(const _var_vect &v_in,const double theta)
{
  double cos_theta=cos(theta);
  double sin_theta=sin(theta);
  return _var_vect(v_in[0]*cos_theta+v_in[1]*sin_theta,
              v_in[1]*cos_theta-v_in[0]*sin_theta,
              v_in[2]);
}
//===========================================================================
const _var_vect Mandala::rotate(const _var_vect &v_in,const _var_vect &theta)
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

  _var_vect eulerDC[3]={_var_vect(cpsi*ctheta,spsi*ctheta,-stheta),
                   _var_vect(-spsi*cphi + cpsi*stheta*sphi,cpsi*cphi + spsi*stheta*sphi,ctheta*sphi),
                   _var_vect(spsi*sphi + cpsi*stheta*cphi,-cpsi*sphi + spsi*stheta*cphi,ctheta*cphi)
                  };
  _var_vect c;
  double s;
  for (uint i=0;i<3;i++) {
    s=0;
    for (uint j=0;j<3;j++) s+=v_in[j]*eulerDC[i][j];
    c[i]=s;
  }
  return c;
}
//=============================================================================
const _var_vect Mandala::llh2ned(const _var_vect llh)
{
  return llh2ned(llh,_var_vect(gps_home_lat*D2R,gps_home_lon*D2R,gps_home_hmsl));
}
//===========================================================================
const _var_vect Mandala::llh2ned(const _var_vect llh,const _var_vect home_llh)
{
  return LLH_dist(home_llh,llh,home_llh[0],home_llh[1]);
}
//===========================================================================
const _var_vect Mandala::ned2llh(const _var_vect &ned)
{
  return ned2llh(ned,_var_vect(gps_home_lat*D2R,gps_home_lon*D2R,gps_home_hmsl));
}
//===========================================================================
const _var_vect Mandala::ned2llh(const _var_vect &ned,const _var_vect &home_llh)
{
  return ECEF2llh(llh2ECEF(home_llh)+Tangent2ECEF(ned,home_llh[0],home_llh[1]));
}
//===========================================================================
const _var_vect Mandala::LLH_dist(const _var_vect &llh1,const _var_vect &llh2,const double lat,const double lon)
{
  const _var_vect &ecef1(llh2ECEF(llh1));
  const _var_vect &ecef2(llh2ECEF(llh2));
  const _var_vect &diff(ecef2-ecef1);
  return ECEF2Tangent(diff,lat,lon);
}
//=============================================================================
const _var_vect Mandala::ECEF_dist(const _var_vect &ecef1,const _var_vect &ecef2,const double lat,const double lon)
{
  return ECEF2Tangent(ecef1-ecef2,lat,lon);
}
//=============================================================================
const _var_vect Mandala::ECEF2Tangent(const _var_vect &ECEF,const double latitude,const double longitude)
{
  double clat=cos(latitude);
  double clon=cos(longitude);
  double slat=sin(latitude);
  double slon=sin(longitude);
  double Re2t[3][3];

  Re2t[0][0]=-slat*clon;
  Re2t[0][1]=-slat*slon;
  Re2t[0][2]=clat;
  Re2t[1][0]=-slon;
  Re2t[1][1]=clon;
  Re2t[1][2]=0.0;
  Re2t[2][0]=-clat*clon;
  Re2t[2][1]=-clat*slon;
  Re2t[2][2]=-slat;
  _var_vect c;
  double s;
  for (uint i=0;i<3;i++) {
    s=0;
    for (uint j=0;j<3;j++) s+=ECEF[j]*Re2t[i][j];
    c[i]=s;
  }
  return c;
}
//=============================================================================
const _var_vect Mandala::Tangent2ECEF(const _var_vect &Local,const double latitude,const double longitude)
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
  _var_vect c;
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
const _var_vect Mandala::ECEF2llh(const _var_vect &ECEF)
{
  double X=ECEF[0];
  double Y=ECEF[1];
  double Z=ECEF[2];
  double f=(C_WGS84_a-C_WGS84_b)/C_WGS84_a;
  double e=sqrt(2*f-f*f);
  double h=0;
  double N=C_WGS84_a;
  _var_vect llh;
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
const _var_vect Mandala::llh2ECEF(const _var_vect &llh)
{
  double f=(C_WGS84_a-C_WGS84_b)/C_WGS84_a;
  double e=sqrt(2*f-f*f);
  double N=C_WGS84_a/sqrt(1-e*e*sqr(sin(llh[0])));
  _var_vect ECEF;
  ECEF[0]=(N+llh[2])*cos(llh[0])*cos(llh[1]);
  ECEF[1]=(N+llh[2])*cos(llh[0])*sin(llh[1]);
  ECEF[2]=(N*(1-e*e)+llh[2])*sin(llh[0]);
  return ECEF;
}
//=============================================================================
double Mandala::inHgToAltitude(double inHg)
{
  return (1.0-powf(inHg/pstatic_gnd,0.190284))*(145366.45*0.3048);
}
//=============================================================================
//=============================================================================
void Mandala::dump(const uint8_t *ptr,uint cnt,bool hex)
{
  //printf("\n");
  for (uint i=0;i<cnt;i++)printf(hex?"%.2X ":"%u ",*ptr++);
  printf("\n");
}
void Mandala::dump(const _var_vect &v,const char *str)
{
  printf("%s: %.2f\t%.2f\t%.2f\n",str,v[0],v[1],v[2]);
}
void Mandala::dump(const uint var_idx)
{
  printf("%s: ",var_name[var_idx]);
  void *ptr=var_ptr[var_idx];
  switch(var_type[var_idx]){
    case vt_uint:   printf("%u",*((uint*)ptr));break;
    case vt_float: printf("%.2f",*((double*)ptr));break;
    case vt_vect:   printf("(%.2f,%.2f,%.2f)",(*((_var_vect*)ptr))[0],(*((_var_vect*)ptr))[1],(*((_var_vect*)ptr))[2] );break;
    case vt_sig:{
      printf("+sig+\n");
      _var_signature signature=*(_var_signature*)ptr;
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
  fprintf(stream,"#\tVariableName\tDescription\tType\tSpan\tPackedSize\n");
  for (uint i=0;i<256;i++) {
    if ((!var_size[i])&&(var_type[i]!=vt_sig))continue;
    if (var_type[i]==vt_void)continue;
    //if (i==0) fprintf(stream,"== Signature Variables (internal use only) ==\n");
    //if (i==idxCFG) fprintf(stream,"======= Configuration Variables =============\n");
    const char *vt="";
    switch(var_type[i]){
      case vt_void:   vt="UNKNOWN";break;
      case vt_uint:   vt="uint";break;
      case vt_float:  vt="float";break;
      case vt_vect:   vt="vector";break;
      case vt_sig:    vt="signature";break;
    }
    fprintf(stream,"%u\t%s\t%s\t%s\t%g\t%u\n",
           i,
           var_name[i],
           var_descr[i],
           vt,
           var_span[i],
           var_size[i]
    );
  }
  fprintf(stream,"=========== Bitfields =======================\n");
  fprintf(stream,"VariableName\tBitNumber\tBitName\tDescription\n");
  for (uint i=0;i<256;i++) {
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
  for (uint i=0;i<idxPAD;i++) {
    if(!var_ptr[i])break;
    _var_signature sig=*(_var_signature*)var_ptr[i];
    if ((var_type[i]!=vt_sig)||(!sig[0]))continue;
    fprintf(stream,"%s\t",var_name[i]);
    for(uint ib=0;ib<sig[0];ib++)
      fprintf(stream,"%s,",var_name[sig[ib+1]]);
    fprintf(stream,"\n");
  }
}
//=============================================================================
//=============================================================================
//=============================================================================



