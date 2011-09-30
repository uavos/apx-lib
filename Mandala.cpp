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
    var_round[i]=0;
    var_bits[i]=0;
    var_span[i]=0;
    var_array[i]=0;
    var_ptr[i]=NULL;
    var_type[i]=vt_void;
  }

  memset(waypoints,0,sizeof(waypoints));
  memset(runways,0,sizeof(runways));
  memset(&cfg,0,sizeof(cfg));

  dl_frcnt=0;
  dl_errcnt=0;
  dl_timestamp=0;
  dl_dt=1000/TELEMETRY_FREQ;
  dl_size=0;
  dl_reset=true;

  derivatives_init=false;
  gps_lat_s=0;
  gps_lon_s=0;

  //------------------------

#define VARDEFX(atype,aname,asize,aspan,abytes,adescr) \
  var_ptr[idx_##aname]=(void*)(& aname); \
  var_type[idx_##aname]=vt_##atype; \
  var_array[idx_##aname]=asize; \
  var_size[idx_##aname]=((asize)*(abytes)*((vt_##atype==vt_vect)?3:1)); \
  var_span[idx_##aname]=aspan; \
  var_name[idx_##aname]=#aname; \
  var_descr[idx_##aname]=adescr;

#define VARDEF(atype,aname,aspan,abytes,adescr) \
  VARDEFX(atype,aname,1,aspan,abytes,adescr) \
  aname=0;

#define VARDEFA(atype,aname,asize,aspan,abytes,adescr) \
  VARDEFX(atype,aname,asize,aspan,abytes,adescr) \
  for(uint i=0;i<asize;i++) aname[i]=0;

#include "MandalaVars.h"

#define SIGDEF(aname,adescr,...) \
  VARDEFX(sig,aname,VA_NUM_ARGS(__VA_ARGS__),0,0,adescr) \
  var_size[idx_##aname]=sig_size(aname);

#include "MandalaVars.h"


#define MODEDEF(aname,adescr) \
  mode_names[fm##aname]=#aname; \
  mode_descr[fm##aname]=adescr;

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
  CFGDEFA(atype,aname,1,aspan,abytes,around,adescr) \

#include "MandalaVarsAP.h"

  //set config signature size
  for(uint i=0;i<cfgCnt;i++)
    var_size[idx_config]+=cfg.var_size[i];


  //fill strings
  static const char *wt_str_s[wtCnt]={ wt_str_def };
  for(uint i=0;i<wtCnt;i++) wt_str[i]=wt_str_s[i];
  static const char *rwt_str_s[rwtCnt]={ rwt_str_def };
  for(uint i=0;i<rwtCnt;i++) rwt_str[i]=rwt_str_s[i];


  //bitfield strings
  uint bidx;
#define BITDEF(avarname,abitname,amask,adescr) var_bits[idx_##avarname]++;
#include "MandalaVars.h"
#define BITDEF(avarname,abitname,amask,adescr) \
  bidx=(int)(log(amask)/log(2));\
  var_bits_name[idx_##avarname][bidx]= #abitname ;\
  var_bits_descr[idx_##avarname][bidx]= adescr ;
#include "MandalaVars.h"
}
//===========================================================================
//===========================================================================
uint Mandala::archive(uint8_t *buf,uint size,uint var_idx)
{
  //check for special protocol archiveSize
  if(var_idx==idx_config)return archive_config(buf,size);
  if(var_idx==idx_downstream)return archive_downstream(buf,size);
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
  if(!do_archive(buf,var_idx))return 0;
  return var_size[var_idx];
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
  if(var_idx==idx_flightplan)return extract_flightplan(buf,size);

  void *ptr=var_ptr[var_idx];
  if(!ptr){
    fprintf(stderr,"Error: extract unknown var  #%u\n",var_idx);
    return 0;
  }
  //printf("extracting: %s\n",var_name[var_idx]);
  uint vsz=var_size[var_idx];
  if((!vsz)||(vsz>size)){
    fprintf(stderr,"Error: extract %s #%u (sz: %u, buf: %u)\n",var_name[var_idx],var_idx,vsz,size);
    return 0;
  }
  //uint cnt=do_extract(buf,size,var_idx);
  if(!do_extract(buf,size,var_idx))return 0; //error
  //if(var_idx==20){
    //printf("cnt:%u var:%u %.2f ",cnt,var_idx,Ve);
    //dump(idx_theta);
    //dump(buf,size);
  //}
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
uint Mandala::archive_config(uint8_t *buf,uint bufSize)
{
  uint sz=var_size[idx_config];
  if(bufSize<sz){
    printf("Can't archive config, wrong buffer size (%u) need (%u).\n",bufSize,sz);
    return 0;
  }
  for(uint i=0;i<cfgCnt;i++){
    fill_config_vdsc(buf,i);
    buf+=do_archive_vdsc();
  }
  return sz;
}
//=============================================================================
uint Mandala::extract_config(uint8_t *buf,uint cnt)
{
  uint sz=var_size[idx_config];
  if(cnt!=sz){
    printf("Can't extract config, wrong data size (%u) need (%u).\n",cnt,sz);
    return 0;
  }
  for(uint i=0;i<cfgCnt;i++){
    fill_config_vdsc(buf,i);
    uint vsz=do_extract_vdsc(cnt);
    buf+=vsz;
    cnt-=vsz;
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
    vdsc.array=asize;\
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
  const uint rwPackedSz=wptPackedSz+(var_size[idx_NED]);
  const uint sz=wptPackedSz*wpcnt+rwPackedSz*rwcnt+2;
  if(bufSize<sz){
    printf("Can't archive flight plan, wrong buffer size (%u) need (%u).\n",bufSize,sz);
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
    *buf++=runways[i].type;
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
  const uint rwPackedSz=wptPackedSz+(var_size[idx_NED]);
  const uint sz=wptPackedSz*buf[0]+rwPackedSz*buf[1]+2;
  if(cnt!=sz){
    printf("Can't extract flight plan, wrong data size (%u) need (%u).\n",cnt,sz);
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
    waypoints[i].LLA=Vector(lat,lon,alt);
    waypoints[i].cmd[0]=0;
    //print wpt stats
    const Vector ned=llh2ned(Vector(lat*D2R,lon*D2R,gps_home_hmsl+alt));
    printf("WPT%u NED(%.0f, %.0f, %.0f) %s\n",i+1,ned[0],ned[1],ned[2],wt_str[waypoints[i].type]);
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
    runways[i].type=(_rw_type)*data++;
    runways[i].dNED=NED;
    printf("Runway%u (%s), NED(%.0f,%.0f,%.0f)\n",i+1,rwt_str[runways[i].type],runways[i].dNED[0],runways[i].dNED[1],runways[i].dNED[2]);
  }
  gps_lat=gps_lat_save;
  gps_lon=gps_lon_save;
  gps_hmsl=gps_hmsl_save;
  NED=NED_save;
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
  for(uint i=idxPAD;i<(idxPAD+128);i++){
    uint sz=var_size[i];
    if(!sz)break; //valid vars end
    sz=archive(buf_var,sz,i);
    //check if filtered var
    bool filtered=memchr(dl_filter+1,i,dl_filter[0])!=NULL;
    if((*reset_mask_ptr)&mask) (*reset_mask_ptr)&=~mask;
    else{
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
uint Mandala::extract_downstream(uint8_t *buf,uint cnt)
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
  uint tcnt=0;
  if(cnt>2){
    tcnt=extract_stream(buf+2,cnt-2);
    if(!tcnt){
      fprintf(stderr,"Error extract_downstream");
      return 0;
    }
  }
  // calculate vars filtered by sig dl_filter
  //check if to calc derivatives (gps fix)
  if((gps_lat_s!=gps_lat)||(gps_lon_s!=gps_lon)){
    gps_lat_s=gps_lat;
    gps_lon_s=gps_lon;
    calcDGPS();
  }
  // gps_deltaNED,gps_deltaXYZ,gps_distWPT,gps_distHome,
  calc();
  return tcnt;
}
//=============================================================================
//=============================================================================
void Mandala::calcDGPS(const double dt)
{
  // calculate NED
  NED=llh2ned(Vector(gps_lat*D2R,gps_lon*D2R,gps_hmsl));

  Vector theta_r=theta*D2R;
  // calculate frame velocities
  //theta_r[2]=gps_course*D2R;
  vXYZ=rotate(gps_vNED,theta_r);

  Vector acc;
  if(derivatives_init)
    acc=rotate((gps_vNED-last_vNED)/dt,theta_r);
  else last_aXYZ=acc;
  last_vNED=gps_vNED;
  Vector tacc(last_aXYZ-acc);
  const double accF=5;
  if((fabs(tacc[0])<accF)&&(fabs(tacc[1])<accF)&&(fabs(tacc[2])<accF)){
    aXYZ=acc;
  }
  last_aXYZ=acc;

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
  dN=cmd_N-NED[0];
  dE=cmd_E-NED[1];
  dAlt=cmd_altitude-altitude;
  Vector dNED(dN,dE,-dAlt);
  dXYZ=rotate(dNED,theta[2]*D2R);
  dWPT=ned2dist(dNED);
  dHome=ned2dist(NED);
  wpHDG=ned2hdg(dNED);
  homeHDG=ned2hdg(NED,true);
  rwDelta=dWPT*sin((wpHDG+180.0-rwHDG)*D2R);
}
//=============================================================================
double Mandala::ned2hdg(const Vector &ned,bool back)
{
  double v=atan2(ned[1],ned[0])*R2D;
  if(back)return boundAngle(v+180.0);
  else return boundAngle(v);
}
//=============================================================================
const Vector Mandala::lla2ned(const Vector &lla)
{
  return llh2ned(Vector(lla[0]*D2R,lla[1]*D2R,gps_home_hmsl+lla[2]));
}
//=============================================================================
double Mandala::ned2dist(const Vector &ned)
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
Vector Mandala::boundAngle(const Vector &v,double span)
{
  return Vector(boundAngle(v[0],span),boundAngle(v[1],span),boundAngle(v[2],span));
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
const Vector Mandala::rotate(const Vector &v_in,const double theta)
{
  double cos_theta=cos(theta);
  double sin_theta=sin(theta);
  return Vector(v_in[0]*cos_theta+v_in[1]*sin_theta,
              v_in[1]*cos_theta-v_in[0]*sin_theta,
              v_in[2]);
}
//===========================================================================
const Vector Mandala::rotate(const Vector &v_in,const Vector &theta)
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

  Vector eulerDC[3]={Vector(cpsi*ctheta,spsi*ctheta,-stheta),
                   Vector(-spsi*cphi + cpsi*stheta*sphi,cpsi*cphi + spsi*stheta*sphi,ctheta*sphi),
                   Vector(spsi*sphi + cpsi*stheta*cphi,-cpsi*sphi + spsi*stheta*cphi,ctheta*cphi)
                  };
  Vector c;
  double s;
  for (uint i=0;i<3;i++) {
    s=0;
    for (uint j=0;j<3;j++) s+=v_in[j]*eulerDC[i][j];
    c[i]=s;
  }
  return c;
}
//=============================================================================
const Vector Mandala::llh2ned(const Vector llh)
{
  return llh2ned(llh,Vector(gps_home_lat*D2R,gps_home_lon*D2R,gps_home_hmsl));
}
//===========================================================================
const Vector Mandala::llh2ned(const Vector llh,const Vector home_llh)
{
  return LLH_dist(home_llh,llh,home_llh[0],home_llh[1]);
}
//===========================================================================
const Vector Mandala::LLH_dist(const Vector &llh1,const Vector &llh2,const double lat,const double lon)
{
  const Vector &ecef1(llh2ECEF(llh1));
  const Vector &ecef2(llh2ECEF(llh2));
  const Vector &diff(ecef2-ecef1);
  return ECEF2Tangent(diff,lat,lon);
}
//=============================================================================
const Vector Mandala::ECEF_dist(const Vector &ecef1,const Vector &ecef2,const double lat,const double lon)
{
  return ECEF2Tangent(ecef1-ecef2,lat,lon);
}
//=============================================================================
const Vector Mandala::ECEF2Tangent(const Vector &ECEF,const double latitude,const double longitude)
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
  Vector c;
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
const Vector Mandala::Tangent2ECEF(const Vector &Local,const double latitude,const double longitude)
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
  Vector c;
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
const Vector Mandala::ECEF2llh(const Vector &ECEF)
{
  double X=ECEF[0];
  double Y=ECEF[1];
  double Z=ECEF[2];
  double f=(C_WGS84_a-C_WGS84_b)/C_WGS84_a;
  double e=sqrt(2*f-f*f);
  double h=0;
  double N=C_WGS84_a;
  Vector llh;
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
const Vector Mandala::llh2ECEF(const Vector &llh)
{
  double f=(C_WGS84_a-C_WGS84_b)/C_WGS84_a;
  double e=sqrt(2*f-f*f);
  double N=C_WGS84_a/sqrt(1-e*e*sqr(sin(llh[0])));
  Vector ECEF;
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
void Mandala::dump(const Vector &v,const char *str)
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
    case vt_vect:   printf("(%.2f,%.2f,%.2f)",(*((Vector*)ptr))[0],(*((Vector*)ptr))[1],(*((Vector*)ptr))[2] );break;
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
  fprintf(stream,"#\tVariableName\tDescription\tType\tArray\tSpan\tPackedSize\n");
  for (uint i=0;i<256;i++) {
    if ((!var_size[i])&&(var_type[i]!=vt_sig))continue;
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
    fprintf(stream,"%u\t%s\t%s\t%s\t%u\t%g\t%u\n",
           i,
           var_name[i],
           var_descr[i],
           vt,
           var_array[i],
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



