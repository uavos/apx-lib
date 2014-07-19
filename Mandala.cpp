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
#include "time_ms.h"
//===========================================================================
Mandala::Mandala()
  : MandalaCore()
{
  init();
}
void Mandala::init(void)
{
  memset(&fp,0,sizeof(fp));

  dl_frcnt=0;
  dl_errcnt=0;
  dl_timestamp=0;
  dl_time_s=time;
  dl_ts=0;
  dl_Pdt=0;
  dl_size=0;
  dl_reset=true;

  //------------------------
  //init all vars
#define MVAR(atype,aname,...) \
  aname=0;
#include "MandalaVars.h"
  cas2tas=1.0;

  //------------------------

  //fill strings
  static const char *wt_str_s[wtCnt]={ wt_str_def };
  for(uint i=0;i<wtCnt;i++) wt_str[i]=wt_str_s[i];
  static const char *rwa_str_s[rwaCnt]={ rwa_str_def };
  for(uint i=0;i<rwaCnt;i++) rwa_str[i]=rwa_str_s[i];
}
//===========================================================================
bool Mandala::get_text_names(uint16_t varmsk,const char **name,const char **descr)
{
  uint type;
  void *value_ptr;
  if(!get_ptr(varmsk,&value_ptr,&type))return false;
  *name="";
  *descr="";
  uint8_t m=varmsk>>8;
  if((m!=0xFF)&& ((type==vt_flag && m) || type==vt_enum)){
    switch(varmsk){
    #define MBIT(avarname,abitname,adescr,amask) \
      case idx_##avarname |( avarname##_##abitname <<8): *name=#abitname;*descr=adescr;return true;
    #include "MandalaVars.h"
    }
    if(m) return false;
  }
  //find name and descr (no member)
  switch (varmsk&0xFF) {
  #define SIGDEF(aname,adescr,...) \
    case idx_##aname: *name=#aname;*descr=adescr;return true;
  #define MVAR(atype,aname,adescr, ...) \
    case idx_##aname: *name=#aname;*descr=adescr;return true;
  #include "MandalaVars.h"
  }
  return false;
}
//===========================================================================
uint Mandala::archive(uint8_t *buf,uint size,uint var_idx)
{
  //check for special protocol archiveSize
  if(var_idx==idx_downstream)return archive_downstream(buf,size);
  if(var_idx==idx_flightplan)return archive_flightplan(buf,size);
  if(var_idx<idx_imu)return 0;  //other special var
  //basic vars
  return pack(buf,var_idx);
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
  if(var_idx==idx_downstream)return extract_downstream(buf,size);
  if(var_idx==idx_flightplan)return extract_flightplan(buf,size);

  uint cnt=unpack(buf,size,var_idx);
  if(cnt==size)return cnt;
  if(!cnt)return 0; //error
  if(size<cnt) return cnt;
  size-=cnt;
  var_idx++;
  if(size&&(var_idx>idxPAD)&&(var_idx<idx_vars_top))return cnt+unpack(buf+cnt,size,var_idx);
  else return cnt;
}
//=============================================================================
//=============================================================================
uint Mandala::archive_flightplan(uint8_t *buf,uint bufSize)
{
  //save mandala tmp vars
  uint8_t *sbuf=buf,*buf_top=buf+(bufSize-1);
  uint cnt=0;
  const uint szLLH=4+4+4;
  while(1){
    //write waypoints
    *buf++=wpcnt;
    uint i;
    for(i=0;i<wpcnt;i++){
      if((buf+(szLLH+2+fp.waypoints[i].cmdSize))>buf_top) break;
      buf+=pack_float_f4(buf,&(fp.waypoints[i].LLA[0]));
      buf+=pack_float_f4(buf,&(fp.waypoints[i].LLA[1]));
      buf+=pack_float_f4(buf,&(fp.waypoints[i].LLA[2]));
      *buf++=fp.waypoints[i].type;
      *buf++=fp.waypoints[i].cmdSize;
      memcpy(buf,fp.waypoints[i].cmd,fp.waypoints[i].cmdSize);
      buf+=fp.waypoints[i].cmdSize;
    }
    if(i<wpcnt)break; //overflow
    //write runways
    *buf++=rwcnt;
    for(i=0;i<rwcnt;i++){
      if((buf+(szLLH+6+9))>buf_top) break;
      buf+=pack_float_f4(buf,&(fp.runways[i].LLH[0]));
      buf+=pack_float_f4(buf,&(fp.runways[i].LLH[1]));
      buf+=pack_float_f4(buf,&(fp.runways[i].LLH[2]));
      buf+=pack_point_f2(buf,&(fp.runways[i].dNE));
      *buf++=fp.runways[i].appType;
      buf+=pack_float_f2(buf,&(fp.runways[i].distApp));
      buf+=pack_float_f2(buf,&(fp.runways[i].altApp));
      buf+=pack_float_f2(buf,&(fp.runways[i].distTA));
      buf+=pack_float_f2(buf,&(fp.runways[i].altTA));
    }
    if(i<rwcnt)break; //overflow
    //write flight place parameters
    if((buf+12)>buf_top) break;
    buf+=pack_float_f4(buf,&(fp.safety.altitude));
    buf+=pack_float_f4(buf,&(fp.safety.dHome));
    buf+=pack_float_f4(buf,&(fp.safety.dHomeERS));
    //success: calc number of bytes written
    cnt=buf-sbuf;
    break;
  }
  if(!cnt) fprintf(stderr,"Can't archive flight plan, buffer overflow.\n");
  return cnt;
}
//=============================================================================
uint Mandala::extract_flightplan(uint8_t *buf,uint cnt)
{
  //save mandala tmp vars
  uint8_t *sbuf=buf,*buf_top=buf+cnt;
  int rcnt=0;
  const uint szLLH=4+4+4;
  while(1){
    //unpack waypoints
    if((buf+1)>buf_top) break;
    wpcnt=*buf++;
    if(wpcnt>MAX_WPCNT)break;
    uint i;
    for (i=0;i<wpcnt;i++) {
      if((buf+(szLLH+2))>buf_top) break;
      buf+=unpack_float_f4(buf,&(fp.waypoints[i].LLA[0]));
      buf+=unpack_float_f4(buf,&(fp.waypoints[i].LLA[1]));
      buf+=unpack_float_f4(buf,&(fp.waypoints[i].LLA[2]));
      fp.waypoints[i].type=(_wpt_type)*buf++;
      uint csz=(_wpt_type)*buf++;
      uint sz=csz;
      if(sz>sizeof(_waypoint::cmdSize)) sz=0;
      fp.waypoints[i].cmdSize=sz;
      if((buf+sz)>buf_top) break;
      memcpy(fp.waypoints[i].cmd,buf,sz);
      buf+=csz;
    }
    if(i<wpcnt)break; //overflow
    //unpack runways
    if((buf+1)>buf_top) break;
    rwcnt=*buf++;
    if(rwcnt>MAX_RWCNT)break;
    for (i=0;i<rwcnt;i++) {
      if((buf+(szLLH+6+9))>buf_top) break;
      buf+=unpack_float_f4(buf,&(fp.runways[i].LLH[0]));
      buf+=unpack_float_f4(buf,&(fp.runways[i].LLH[1]));
      buf+=unpack_float_f4(buf,&(fp.runways[i].LLH[2]));
      buf+=unpack_point_f2(buf,&(fp.runways[i].dNE));
      fp.runways[i].appType=(_rw_app)*buf++;
      buf+=unpack_float_f2(buf,&(fp.runways[i].distApp));
      buf+=unpack_float_f2(buf,&(fp.runways[i].altApp));
      buf+=unpack_float_f2(buf,&(fp.runways[i].distTA));
      buf+=unpack_float_f2(buf,&(fp.runways[i].altTA));
    }
    if(i<rwcnt)break; //overflow
    //unpack flight place parameters
    if((buf+12)>buf_top) break;
    buf+=unpack_float_f4(buf,&(fp.safety.altitude));
    buf+=unpack_float_f4(buf,&(fp.safety.dHome));
    buf+=unpack_float_f4(buf,&(fp.safety.dHomeERS));
    //success: calc number of bytes unpacked
    rcnt=buf-sbuf;
    break;
  }
  if(rcnt<(int)cnt)rcnt=0;
  if(rcnt==0){
    fprintf(stderr,"Can't extract flight plan.\n");
    wpcnt=rwcnt=0;
    memset(&fp,0,sizeof(fp));
  }
  return rcnt;
}
//=============================================================================
uint Mandala::archive_downstream(uint8_t *buf,uint maxSize)
{
  // telemetry stream format:
  // <timestampL>,<timestampH>,<bitsig>,<archived data>,[<bitsig>,<data>...]
  // only modified vars are sent
  // timestamp - time in 10ms units & 0x7FFF, MSB = HD stream
  // bitsig - LSBF bitfield, on/off next 8 variables starting from idxPAD
  // skip vars in sig 'dl_filter' as they are calculated by 'extractTelemety'

  //limit maxSize
  if(maxSize){
    if(maxSize>MAX_TELEMETRY)maxSize=MAX_TELEMETRY;
  }else maxSize=sizeof(dl_snapshot);

  bool hd=cmode&cmode_dlhd;

  //watch hd changes
  if(dl_hd_save!=hd){
    dl_hd_save=hd;
    dl_reset=true;
  }

  //pack header
  uint ts=(dl_timestamp/10)&0x7FFF;
  *buf++=ts;
  *buf++=(ts>>8)|(hd?0x80:0x00);

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
  for(uint i=idxPAD;i<idx_local;i++){
    uint sz=hd?pack(dl_var,i):pack_ext(dl_var,i);
    //check if filtered var
    bool filtered=false;//=memchr(dl_filter+1,i,dl_filter[0])!=NULL;
    if((*reset_mask_ptr)&mask) (*reset_mask_ptr)&=~mask; //remove reset flag
    else filtered=(memcmp(snapshot,dl_var,sz)==0);//test changed
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

  if(!(dl_timestamp%dl_reset_interval)) // periodically send everything
    dl_reset=true;

  dl_timestamp+=dl_period?dl_period:(time-dl_time_s);
  dl_time_s=time;
  dl_size=cnt+mask_cnt+2;
  return dl_size;
}
//=============================================================================
uint Mandala::extract_downstream(uint8_t *buf,uint cnt)
{
  //header
  dl_size=cnt;
  dl_frcnt++;
  uint16_t ts=*buf++;
  ts|=(*buf++)<<8;

  //MSB = HD stream
  bool hd=ts&0x8000;
  ts&=0x7FFF;

  //calc delta time
  uint16_t Pdt=(ts-dl_ts)&0x7FFF;
  dl_ts=ts;
  dl_timestamp+=Pdt*10;
  bool bErr=dl_Pdt!=Pdt; //delta different
  dl_Pdt=Pdt;
  if(bErr&&(bErr==dl_e)) dl_errcnt++;
  dl_e=bErr;

  //extract data
  uint tcnt=2;
  if(cnt>2){
    cnt-=2;
    tcnt=unpack_stream(buf,cnt,hd);
    if(!tcnt){
      //fprintf(stderr,"Error extract_downstream\n");
    }
  }
  // calculate vars filtered by sig dl_filter, otherwise calculated by ahrs
  pos_NE=llh2ne(gps_pos);
  gps_altitude=gps_pos[2]-home_pos[2];
  vXY=rotate(gps_vNE,theta[2]);
  // gps_deltaNED,gps_deltaXYZ,gps_distWPT,gps_distHome,
  calc();
  return tcnt;
}
//=============================================================================
//=============================================================================
//=============================================================================
void Mandala::calc(void)
{
  // vars should be filtered by dl_filter
  const Point dNE=cmd_NE-pos_NE;
  //dAlt=cmd_altitude-altitude;
  //_var_vect dNED(dN,dE,-dAlt);
  dXY=rotate(dNE,theta[2]);
  dWPT=distance(dNE);
  dHome=distance(pos_NE);
  wpHDG=heading(dNE);
  homeHDG=heading(pos_NE,true);
  rwDelta=dWPT*sin((wpHDG+180.0-tgHDG)*D2R)-rwAdj;
  rwDV=rotate(gps_vNE,tgHDG)[1];
  gSpeed=distance(gps_vNE);
  //course
  //_var_float crsGPS=heading(gps_vNED);
  //_var_float crsD=boundAngle(crsGPS-theta[2]);
  //_var_float crsK=100.0*((crsD==0)?1.0:limit((90.0/fabs(crsD))/90.0,0,1));
  //course=smoothAngle(crsGPS,course,0.000001);//gSpeed*gSpeed*0.0001/100.0);
}
//=============================================================================
_var_float Mandala::boundAngle(_var_float v,_var_float span) const
{
  const _var_float dspan=span*2.0;
  while (v >= span) v -= dspan;
  while (v < -span) v += dspan;
  return v;
}
//===========================================================================
_var_vect Mandala::boundAngle(const _var_vect &v,_var_float span) const
{
  return _var_vect(boundAngle(v[0],span),boundAngle(v[1],span),boundAngle(v[2],span));
}
//===========================================================================
_var_float Mandala::boundAngle360(_var_float v) const
{
  while(v<0) v+=360.0;
  while(v>=360.0) v-=360.0;
  return v;
}
//===========================================================================
_var_float Mandala::smoothAngle(_var_float v,_var_float v_prev,_var_float speed)
{
  if(speed!=0){
    _var_float vd=boundAngle(v-v_prev);
    if(vd>speed) v=boundAngle(v_prev+speed);
    else if(vd<(-speed)) v=boundAngle(v_prev-speed);
  }
  return v;
}
//===========================================================================
void Mandala::filter_a(const _var_float &v,_var_float *var_p,const _var_float &f)
{
  *var_p=boundAngle(*var_p+boundAngle(v-*var_p)*f);
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
_var_float Mandala::hyst(_var_float err,_var_float ahyst)
{
  if (fabs(err)<=ahyst)return 0.0;
  else return err-((err>0.0)?ahyst:-ahyst);
}
//===========================================================================
_var_float Mandala::limit(const _var_float v,const _var_float vL)
{
  return limit(v,-vL,vL);
}
_var_float Mandala::limit(const _var_float v,const _var_float vMin,const _var_float vMax)
{
  return (v>vMax)?vMax:((v<vMin)?vMin:v);
}
//===========================================================================
_var_float Mandala::heading(const _var_point &ne,bool back) const
{
  return heading(ne[0],ne[1],back);
}
//=============================================================================
_var_float Mandala::heading(const _var_float N,const _var_float E,bool back) const
{
  if(back)return atan2(-E,-N)*R2D;
  else return atan2(E,N)*R2D;
}
//=============================================================================
_var_float Mandala::distance(const _var_point &ne) const
{
  return distance(ne[0],ne[1]);
}
//=============================================================================
_var_float Mandala::distance(const _var_float N,const _var_float E) const
{
  return sqrt(sqr(N)+sqr(E));
}
//=============================================================================
const _var_point Mandala::rotate(const _var_point &v_in,const _var_float psi) const
{
  return rotate(v_in[0],v_in[1],psi);
}
//===========================================================================
const _var_point Mandala::rotate(const _var_float N,const _var_float E,const _var_float psi) const
{
  const _var_float psi_r=psi*D2R;
  _var_float cos_theta=cos(psi_r);
  _var_float sin_theta=sin(psi_r);
  return _var_point(N*cos_theta+E*sin_theta,
                E*cos_theta-N*sin_theta);
}
//===========================================================================
const _var_vect Mandala::rotate(const _var_vect &v_in,const _var_float psi) const
{
  const _var_point v(rotate(v_in[0],v_in[1],psi));
  return _var_vect(v[0],v[1],v_in[2]);
}
//===========================================================================
const _var_vect Mandala::rotate(const _var_vect &v_in,const _var_vect &atheta) const
{
  const _var_float phi=atheta[0]*D2R;
  const _var_float the=atheta[1]*D2R;
  const _var_float psi=atheta[2]*D2R;
  const _var_float cpsi=cos(psi);
  const _var_float cphi=cos(phi);
  const _var_float ctheta=cos(the);
  const _var_float spsi=sin(psi);
  const _var_float sphi=sin(phi);
  const _var_float stheta=sin(the);

  _var_vect eulerDC[3]={_var_vect(cpsi*ctheta,spsi*ctheta,-stheta),
                   _var_vect(-spsi*cphi + cpsi*stheta*sphi,cpsi*cphi + spsi*stheta*sphi,ctheta*sphi),
                   _var_vect(spsi*sphi + cpsi*stheta*cphi,-cpsi*sphi + spsi*stheta*cphi,ctheta*cphi)
                  };
  _var_vect c;
  _var_float s;
  for (uint i=0;i<3;i++) {
    s=0;
    for (uint j=0;j<3;j++) s+=v_in[j]*eulerDC[i][j];
    c[i]=s;
  }
  return c;
}
//=============================================================================
const _var_point Mandala::lla2ne(const _var_vect &lla) const
{
  return llh2ne(_var_vect(lla[0],lla[1],home_pos[2]+lla[2]));
}
//=============================================================================
const _var_point Mandala::llh2ne(const _var_vect llh) const
{
  return llh2ne(llh,home_pos);
}
//===========================================================================
const _var_point Mandala::llh2ne(const _var_vect llh,const _var_vect home_llh) const
{
  return LLH_dist(home_llh,llh,home_llh[0],home_llh[1]);
}
//===========================================================================
const _var_point Mandala::ne2ll(const _var_point &ne) const
{
  return ne2ll(ne,home_pos);
}
//===========================================================================
const _var_point Mandala::ne2ll(const _var_point &ne,const _var_vect &home_llh) const
{
  return ECEF2ll(llh2ECEF(home_llh)+Tangent2ECEF(ne,home_llh[0],home_llh[1]));
}
//===========================================================================
const _var_point Mandala::LLH_dist(const _var_vect &llh1,const _var_vect &llh2,const _var_float lat,const _var_float lon) const
{
  const _var_vect &ecef1(llh2ECEF(llh1));
  const _var_vect &ecef2(llh2ECEF(llh2));
  const _var_vect &diff(ecef2-ecef1);
  return ECEF2Tangent(diff,lat,lon);
}
//=============================================================================
const _var_point Mandala::ECEF_dist(const _var_vect &ecef1,const _var_vect &ecef2,const _var_float lat,const _var_float lon) const
{
  return ECEF2Tangent(ecef1-ecef2,lat,lon);
}
//=============================================================================
const _var_point Mandala::ECEF2Tangent(const _var_vect &ECEF,const _var_float latitude,const _var_float longitude) const
{
  const _var_float lat_r=latitude*D2R,lon_r=longitude*D2R;
  _var_float clat=cos(lat_r);
  _var_float clon=cos(lon_r);
  _var_float slat=sin(lat_r);
  _var_float slon=sin(lon_r);
  _var_float Re2t[2][3];

  Re2t[0][0]=-slat*clon;
  Re2t[0][1]=-slat*slon;
  Re2t[0][2]=clat;
  Re2t[1][0]=-slon;
  Re2t[1][1]=clon;
  Re2t[1][2]=0.0;
  //Re2t[2][0]=-clat*clon;
  //Re2t[2][1]=-clat*slon;
  //Re2t[2][2]=-slat;
  _var_point c;
  _var_float s;
  for (uint i=0;i<2;i++) {
    s=0;
    for (uint j=0;j<3;j++) s+=ECEF[j]*Re2t[i][j];
    c[i]=s;
  }
  return c;
}
//=============================================================================
const _var_vect Mandala::Tangent2ECEF(const _var_point &ne,const _var_float latitude,const _var_float longitude) const
{
  const _var_float lat_r=latitude*D2R,lon_r=longitude*D2R;
  _var_float clat=cos(lat_r);
  _var_float clon=cos(lon_r);
  _var_float slat=sin(lat_r);
  _var_float slon=sin(lon_r);
  _var_float Rt2e[3][2];
  Rt2e[0][0]=-slat*clon;
  Rt2e[1][0]=-slat*slon;
  Rt2e[2][0]=clat;
  Rt2e[0][1]=-slon;
  Rt2e[1][1]=clon;
  Rt2e[2][1]=0.0;
  //Rt2e[0][2]=-clat*clon;
  //Rt2e[1][2]=-clat*slon;
  //Rt2e[2][2]=-slat;
  _var_vect c;
  _var_float s;
  for (uint i=0;i<3;i++) {
    s=0;
    for (uint j=0;j<2;j++) s+=ne[j]*Rt2e[i][j];
    c[i]=s;
  }
  return c;
}
//=============================================================================
_var_float Mandala::sqr(const _var_float x) const
{
  return x*x;
}
const _var_point Mandala::ECEF2ll(const _var_vect &ECEF) const
{
  _var_float X=ECEF[0];
  _var_float Y=ECEF[1];
  _var_float Z=ECEF[2];
  _var_float f=(C_WGS84_a-C_WGS84_b)/C_WGS84_a;
  _var_float e=sqrt(2*f-f*f);
  _var_float h=0;
  _var_float N=C_WGS84_a;
  _var_point ll;
  ll[1]=atan2(Y,X);
  for (int n=0;n<50;++n) {
    _var_float sin_lat=Z/(N*(1-sqr(e))+h);
    ll[0]=atan((Z+e*e*N*sin_lat)/sqrt(X*X+Y*Y));
    N=C_WGS84_a/sqrt(1-sqr(e)*sqr(sin(ll[0])));
    h=sqrt(X*X+Y*Y)/cos(ll[0])-N;
  }
  //ll[2]=h;
  return ll*R2D;
}
//=============================================================================
const _var_vect Mandala::llh2ECEF(const _var_vect &llh) const
{
  const _var_float lat_r=llh[0]*D2R,lon_r=llh[1]*D2R,hmsl=llh[2];
  _var_float f=(C_WGS84_a-C_WGS84_b)/C_WGS84_a;
  _var_float e=sqrt(2*f-f*f);
  _var_float N=C_WGS84_a/sqrt(1-e*e*sqr(sin(lat_r)));
  _var_vect ECEF;
  ECEF[0]=(N+hmsl)*cos(lat_r)*cos(lon_r);
  ECEF[1]=(N+hmsl)*cos(lat_r)*sin(lon_r);
  ECEF[2]=(N*(1-e*e)+hmsl)*sin(lat_r);
  return ECEF;
}
//=============================================================================
_var_float Mandala::wind_triangle(_var_float crs) const
{
  _var_float spd=(airspeed*cas2tas);
  if(spd<=windSpd) return 0.0001;
  _var_float wnd_r=(windHdg)*D2R;
  _var_float Kvel=windSpd/spd;
  _var_float aWTA=crs*D2R-wnd_r;  //fabs??
  _var_float aWCA=asin(Kvel*sin(aWTA));
  _var_float kWS=cos(aWCA)+Kvel*cos(aWTA);
  return kWS;
}
//=============================================================================
_var_float Mandala::wind_circle(_var_float crs,_var_float span,_var_float r) const
{
  _var_float spd=(airspeed*cas2tas);
  if(spd<=windSpd) return 65535;
  _var_float kWSs=0;
  _var_float crs_step=30,crs_e;
  _var_float wnd_r=(windHdg)*D2R;
  _var_float Kvel=windSpd/spd;
  if(span<0) crs_step=-crs_step;
  crs_e=crs+span;
  int sz=span/crs_step;
  _var_float kWS=1;
  while(sz--){
    _var_float aWTA=crs*D2R-wnd_r;
    _var_float aWCA=asin(Kvel*sin(aWTA));
    kWS=cos(aWCA)+Kvel*cos(aWTA);
    kWSs+=1.0/kWS;
    crs+=crs_step;
  }
  if(span<0) span=-(crs_e-crs);
  else span=(crs_e-crs);
  return (kWSs*fabs(crs_step)+span/kWS)*r*(2.0*M_PI/360.0);
}
//=============================================================================
//=============================================================================
const char *Mandala::var_name(uint8_t var_idx)
{
  const char *name;
  const char *descr;
  get_text_names(var_idx|0xFF00,&name,&descr);
  return name;
}
//=============================================================================
uint8_t Mandala::var_index(const char *name)
{
  for(uint i=0;i<idx_vars_top;i++){
    const char *iname;
    const char *descr;
    if(!get_text_names(i|0xFF00,&iname,&descr))continue;
    if(strcmp(iname,name)!=0)continue;
    return i;
  }
  return 0xFF;
}
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
void Mandala::dump(uint8_t var_idx)
{
  uint type;
  void *value_ptr;
  if(!get_ptr(var_idx,&value_ptr,&type))return;
  printf("%s: ",var_name(var_idx));
  switch(type){
    case vt_byte:  printf("%u",(uint)*((_var_byte*)value_ptr));break;
    case vt_uint:  printf("%u",(uint)*((_var_uint*)value_ptr));break;
    case vt_float: printf("%.2f",*((_var_float*)value_ptr));break;
    case vt_vect:  printf("(%.2f,%.2f,%.2f)",(*((_var_vect*)value_ptr))[0],(*((_var_vect*)value_ptr))[1],(*((_var_vect*)value_ptr))[2] );break;
    case vt_sig:{
      printf("+sig+\n");
      _var_signature signature=*(_var_signature*)value_ptr;
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
//=============================================================================



