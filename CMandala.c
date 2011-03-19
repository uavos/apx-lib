#include "CMandala.h"
#include <math.h>
//=============================================================================
uint archive_size(const uint8_t *signature)
{
  uint scnt=signature[0];
  signature++;
  uint cnt=0;
  while (scnt--) cnt+=archive_var(0,*signature++);
  return cnt;
}
//=============================================================================
uint archive_sig(uint8_t *buf,const uint8_t *signature)
{
  uint scnt=signature[0];
  signature++;
  uint cnt=0,sz;
  while (scnt--) {
    sz=archive_var(buf,*signature++);
    if (!sz)return 0; //error
    buf+=sz;
    cnt+=sz;
  }
  return cnt;
}
//=============================================================================
uint32_t limit_u(const float v,const uint32_t max)
{
  if (v<0)return 0;
  if (v>max)return max;
  return v;
}
uint32_t limit_ui(const uint32_t v,const uint32_t max)
{
  if (v>max)return max;
  return v;
}
int32_t limit_s(const float v,const uint32_t max)
{
  float min=-((float)max);
  if (v<min)return min;
  if (v>max)return max;
  return v;
}
//=============================================================================
uint archive_f_impl(uint8_t *buf,void* vptr,const int sbytes,const float span,const uint32_t max)
{ //little endian (LSB first)
  float v=*((double*)vptr);
  if (!buf)return (sbytes>0)?sbytes:(-sbytes); //used for archive_size
  if (span!=0.0)v=max*v/span;
  switch (sbytes) {
    case -4: {    //int32
      int32_t vs=limit_s(v,max);
      uint32_t vi=*((uint32_t*)&vs);
      *(buf++)=vi;
      *(buf++)=vi>>8;
      *(buf++)=vi>>16;
      *(buf)=vi>>24;
    }break;
    case -2: {    //int16
      int32_t vs=limit_s(v,max);
      uint32_t vi=*((uint32_t*)&vs);
      *(buf++)=vi;
      *(buf)=vi>>8;
    }break;
    case -1:     //int8
      *((int8_t*)buf)=limit_s(v,max);
      break;
    case 1:     //uint8
      *((uint8_t*)buf)=limit_u(v,max);
      break;
    case 2: {    //uint16
      uint16_t vi=limit_u(v,max);
      *(buf++)=vi;
      *(buf)=vi>>8;
    }break;
    case 4: {    //uint32
      uint32_t vi=limit_u(v,max);
      *(buf++)=vi;
      *(buf++)=vi>>8;
      *(buf++)=vi>>16;
      *(buf)=vi>>24;
    }break;
  }
  return (sbytes>0)?sbytes:(-sbytes);
}
uint archive_u_impl(uint8_t *buf,void* vptr,const int sbytes,const uint32_t max)
{ //little endian (LSB first)
  uint v=*((uint*)vptr);
  if (!buf)return sbytes; //used for archive_size
  switch (sbytes) {
    case 1:     //uint8
      *((uint8_t*)buf)=limit_ui(v,max);
      break;
    case 2: {    //uint16
      uint16_t vi=limit_ui(v,max);
      *(buf++)=vi;
      *(buf)=vi>>8;
    }break;
    case 4: {    //uint32
      uint32_t vi=limit_ui(v,max);
      *(buf++)=vi;
      *(buf++)=vi>>8;
      *(buf++)=vi>>16;
      *(buf)=vi>>24;
    }break;
  }
  return sbytes;
}
uint archive_v_impl(uint8_t *buf,void* vptr,const int sbytes,const float span,const uint32_t max)
{
  Vect *v=((Vect*)vptr);
  uint sz,cnt;
  sz=archive_f_impl(buf,&((*v)[0]),sbytes,span,max);
  cnt=sz;
  buf+=sz;
  sz=archive_f_impl(buf,&((*v)[1]),sbytes,span,max);
  cnt+=sz;
  buf+=sz;
  sz=archive_f_impl(buf,&((*v)[2]),sbytes,span,max);
  return cnt+sz;
}
//-----------------------------------------------------------------------------
#define archive_f(abuf,avalue,aname) archive_f_impl(abuf,avalue,var_bytes_##aname,var_span_##aname,var_max_##aname)
#define archive_u(abuf,avalue,aname) archive_u_impl(abuf,avalue,var_bytes_##aname,var_max_##aname)
#define archive_v(abuf,avalue,aname) archive_v_impl(abuf,avalue,var_bytes_##aname,var_span_##aname,var_max_##aname)
#define USEVAR(aname) case idx_##aname: return \
(var_type_##aname == vt_double)?archive_f(buf,&(var. aname),aname): \
( (var_type_##aname == vt_uint)?archive_u(buf,&(var. aname),aname): \
( (var_type_##aname == vt_Vect)?archive_v(buf,&(var. aname),aname): 0 ) );

uint archive_var(uint8_t *buf,uint var_idx)
{ // idx_gps_lat, idx_gps_lon, idx_gps_hmsl, idx_gps_course, idx_gps_vNED, idx_gps_accuracy
  switch (var_idx) {
#include <mandala_vars.h>
  }
  return 0;
}
#undef USEVAR
//=============================================================================
//=============================================================================
