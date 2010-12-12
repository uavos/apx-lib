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
    var_bytes[i]=0;
    var_array[i]=0;
    var_ptr[i]=(void*)(& var_void);
    var_type[i]=vt_uint;
  }
  for (uint i=0;i<cmdCnt;i++) {
    cmd_alias[i]="";
  }

  uint idx=0;
  //------------------------  idx_##aname=idx;

#define VARDEFX(atype,aname,asize,aspan,abytes,adescr) \
  var_name[idx]=#aname; \
  var_descr[idx]=adescr; \
  var_span[idx]=aspan; \
  var_bytes[idx]=abytes; \
  var_array[idx]=asize; \
  var_ptr[idx]=(void*)(& aname); \
  var_type[idx]=vt_##atype; \
  name_##aname=#aname; \
  descr_##aname=adescr; \
  idx++;

#define VARDEF(atype,aname,aspan,abytes,adescr) \
  VARDEFX(atype,aname,1,aspan,abytes,adescr) \
  aname=0;

#define VARDEFA(atype,aname,asize,aspan,abytes,adescr) \
  VARDEFX(atype,aname,asize,aspan,abytes,adescr) \
  for(uint i=0;i<asize;i++) aname[i]=0; \
  var_ptr[idx-1]=(void*)(aname);

#define CMDDEF(aname,aargs,aalias,adescr) \
  cmd_name[cmd##aname]=#aname; \
  cmd_alias[cmd##aname]=#aalias; \
  cmd_size[cmd##aname]=aargs; \
  cmd_descr[cmd##aname]=adescr;

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

  idx=idxSIG;
#define SIGDEF(aname,...) \
  VARDEFX(sig,aname,VA_NUM_ARGS(__VA_ARGS__),0,archiveSize(aname),#__VA_ARGS__) \
  var_sig[idx-1]=aname;

#include "MandalaVars.h"

  //create signatures..
  memset(sig_telemetry,0,sizeof(sig_telemetry));
  memset(sig_cfg,0,sizeof(sig_cfg));
  for (uint i=idxCFG;i<maxVars;i++){
    if (!var_bytes[i])break;
    sig_cfg[1+sig_cfg[0]++]=i;
  }
}
//===========================================================================
#define SIGDEF(aname,...) uint8_t Mandala:: aname []={ VA_NUM_ARGS(__VA_ARGS__), __VA_ARGS__ };
#include "MandalaVars.h"
//===========================================================================
uint Mandala::archiveValue(uint8_t *ptr,uint i,double v)
{
  double span=var_span[i];
  if (span==0.0) {
    switch (var_bytes[i]) {
      case 1:
        *((uint8_t*)ptr)=(uint8_t)rint(v);
        break;
      case 2:
        *((uint16_t*)ptr)=(uint16_t)rint(v);
        break;
      case 4:
        *((uint32_t*)ptr)=(uint32_t)rint(v);
        break;
    }
    return var_bytes[i];
  }
  uint signd=0;
  if (span<0) {
    signd=1;
    if (v<span)v=span;
    span=-span;
    if (v>span)v=span;
  } else {
    if (v<0.0)v=0.0;
    else if (v>span)v=span;
  }
  if (signd)
    switch (var_bytes[i]) {
      case 1:
        *((int8_t*)ptr)=(int8_t)rint(((double)0x7F)*v/span);
        break;
      case 2:
        *((int16_t*)ptr)=(int16_t)rint(((double)0x7FFF)*v/span);
        break;
      case 4:
        *((int32_t*)ptr)=(int32_t)rint(((double)0x7FFFFFFF)*v/span);
        break;
    }
  else
    switch (var_bytes[i]) {
      case 1:
        *((uint8_t*)ptr)=(uint8_t)rint(((double)0xFF)*v/span);
        break;
      case 2:
        *((uint16_t*)ptr)=(uint16_t)rint(((double)0xFFFF)*v/span);
        break;
      case 4:
        *((uint32_t*)ptr)=(uint32_t)rint(((double)0xFFFFFFFF)*v/span);
        break;
    }
  return var_bytes[i];
}
//=============================================================================
double Mandala::extractValue(const uint8_t *ptr,uint i)
{
  double v=0,span=var_span[i];
  if (span==0.0) {
    switch (var_bytes[i]) {
      case 1:
        v=(*(uint8_t*)ptr);
        break;
      case 2:
        v=(*(uint16_t*)ptr);
        break;
      case 4:
        v=(*(uint32_t*)ptr);
        break;
    }
    return v;
  }
  switch (var_bytes[i]) {
    case 1:
      v=(span<0.0)?
        -span*(*((int8_t*)ptr))/(double)0x7F
        :span*(*(uint8_t*)ptr)/(double)0xFF;
      break;
    case 2:
      v=(span<0.0)?
        -span*(*((int16_t*)ptr))/(double)0x7FFF
        :span*(*(uint16_t*)ptr)/(double)0xFFFF;
      break;
    case 4:
      v=(span<0.0)?
        -span*(*((int32_t*)ptr))/(double)0x7FFFFFFF
        :span*(*(uint32_t*)ptr)/(double)0xFFFFFFFF;
      break;
  }
  double prec=var_round[i];
  if (prec==0.0)return v;
  return round(v/prec)*prec;
}
//=============================================================================
uint Mandala::extractMandala(const uint8_t *buf,const uint8_t *signature)
{
  uint scnt=signature[0],cnt=0,sz;
  signature++;
  while (scnt--){
    sz=extractVar(buf,*signature++);
    buf+=sz;
    cnt+=sz;
  }
  return cnt;
}
//=============================================================================
uint Mandala::extractVar(const uint8_t *buf,uint var_idx)
{
  uint cnt=0,sz=var_bytes[var_idx];
  switch (var_type[var_idx]) {
    case vt_uint:
      for (uint ai=0;ai<var_array[var_idx];ai++) {
        ((uint*)var_ptr[var_idx])[ai]=(uint)extractValue(buf,var_idx);
        buf+=sz;
        cnt+=sz;
      }
      break;
    case vt_double:
      for (uint ai=0;ai<var_array[var_idx];ai++) {
        ((double*)var_ptr[var_idx])[ai]=extractValue(buf,var_idx);
        buf+=sz;
        cnt+=sz;
      }
      break;
    case vt_Vect:
      for (uint ai=0;ai<var_array[var_idx];ai++) {
        for (uint iv=0;iv<3;iv++) {
          (((Vect*)var_ptr[var_idx])[ai])[iv]=extractValue(buf,var_idx);
          buf+=sz;
          cnt+=sz;
        }
      }
      break;
    case vt_sig:{
      uint s=extractMandala(buf,var_sig[var_idx]);
      buf+=s;
      cnt+=s;
    }
    break;
    default: break;
  }
  return cnt;
}
//=============================================================================
uint Mandala::archiveMandala(uint8_t *buf,const uint8_t *signature)
{
  uint scnt=signature[0];
  signature++;
  uint cnt=0;
  while (scnt--) {
    uint i=(uint8_t)*signature++;
    switch (var_type[i]) {
      case vt_uint:
        for (uint ai=0;ai<var_array[i];ai++)
          buf+=archiveValue(buf,i,((uint*)(var_ptr[i]))[ai]);
        cnt+=var_bytes[i]*var_array[i];
        break;
      case vt_double:
        for (uint ai=0;ai<var_array[i];ai++)
          buf+=archiveValue(buf,i,((double*)(var_ptr[i]))[ai]);
        cnt+=var_bytes[i]*var_array[i];
        break;
      case vt_Vect:
        for (uint ai=0;ai<var_array[i];ai++)
          for (uint iv=0;iv<3;iv++)
            buf+=archiveValue(buf,i,(((Vect*)(var_ptr[i]))[ai])[iv]);
        cnt+=var_bytes[i]*3*var_array[i];
        break;
      default: break;
    }
  }
  //dump(sbuf-cnt,cnt);
  return cnt;
}
//=============================================================================
uint Mandala::archiveSize(const uint8_t *signature)
{
  uint scnt=signature[0];
  signature++;
  uint cnt=0;
  while (scnt--) {
    uint i=(uint8_t)*signature++;
    if (var_type[i]==vt_Vect)
      cnt+=var_bytes[i]*3*var_array[i];
    else cnt+=var_bytes[i]*var_array[i];
  }
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
bool Mandala::checkCommandBuf(const uint8_t *data,uint cnt)
{
  uint cmd=data[0];
  if (cmd>=cmdCnt) {
    printf("Command not defined (%u)\n",cmd);
    return false;
  }
  int size=cmd_size[cmd];
  // guess some known command sizes..
  if (cmd==cmdMandalaReq)size=data[1]+1;
  else if (cmd==cmdMandalaSet)size=data[1]+1+archiveSize(data+1);
  else if (cmd==cmdWPT)size=data[1]*(4+4+2)+1;
  else if (cmd==cmdTelemetrySet)size=data[1]+1;
  // check size..
  if ((size>=0) && ((size+1)!=(int)cnt))
    printf("Wrong %s size (%u) need %u\n",cmd_name[cmd],cnt-1,size);
  else return true;
  return false;
}
//=============================================================================
void Mandala::dump(const uint8_t *ptr,uint cnt,bool hex)
{
  //printf("\n");
  for (uint i=0;i<cnt;i++)printf(hex?"%.2X ":"%u ",*ptr++);
  printf("\n");
}
//=============================================================================
//=============================================================================
double Mandala::boundAngle(double v,double span)
{
  double dspan=span*2.0;
  while (v > span) v -= dspan;
  while (v <= -span) v += dspan;
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
double Mandala::limit(double v,double vL)
{
  return limit(v,-vL,vL);
}
//===========================================================================
double Mandala::limit(double v,double vMin,double vMax)
{
  return (v>vMax)?vMax:((v<vMin)?vMin:v);
}
//===========================================================================
const Vect Mandala::rotate(const Vect &v_in,const double theta)
{
  static double cos_theta=1.0;
  static double sin_theta=0.0;
  static double last_theta=0.0;
  if (last_theta!=theta) {
    last_theta=theta;
    cos_theta=cos(theta);
    sin_theta=sin(theta);
  }
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
  double lat=gps_home_Lat*D2R,lon=gps_home_Lon*D2R;
  return LLH_dist(Vect(lat,lon,gps_home_HMSL),llh,lat,lon);
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
static inline double sqr(double x) {
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
void Mandala::print_report(void)
{
  printf("======= Mandala Variables ===================\n");
  printf("#\tVariableName\tDescription\tType\tArray\tSpan\tBytes\n");
  for (uint i=0;i<maxVars;i++) {
    if (!var_bytes[i])continue;
    if (i==idxSIG) printf("== Signature Variables (internal use only) ==\n");
    if (i==idxCFG) printf("======= Configuration Variables =============\n");
    const char *vt="";
    switch(var_type[i]){
      case vt_void:   vt="UNKNOWN";break;
      case vt_uint:   vt="uint";break;
      case vt_double: vt="double";break;
      case vt_Vect:   vt="Vect";break;
      case vt_sig:    vt="signature";break;
    }
    printf("%u\t%s\t%s\t%s\t%u\t%g\t%u\n",
           i,
           var_name[i],
           var_descr[i],
           vt,
           var_array[i],
           var_span[i],
           var_bytes[i]
          );
  }
}
//=============================================================================
//=============================================================================
//=============================================================================



