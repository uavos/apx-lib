#include "CMandala.h"
#include <math.h>
//=============================================================================
// special signature vars..
#define SIGDEF(aname, adescr, ... )   static const uint8_t signature_##aname [ VA_NUM_ARGS(__VA_ARGS__)+1 ]={VA_NUM_ARGS(__VA_ARGS__), __VA_ARGS__ };
#include "MandalaVars.h"
//-----------------------------------------------------------------------------
//global struct
Mandala var={
#define USEVAR(aname)
#define USESIG(aname)   . aname = signature_##aname,
#include <mandala_vars.h>
#undef USEVAR
#undef USESIG
};
//=============================================================================
typedef struct {
  uint8_t *buf;         //buffer to store/extract
  void    *ptr;         //pointer to local var.VARNAME
  int     sbytes;       //archived bytes cnt (if < 0 => signed)
  float   span;         //variable span (absolute, always >0)
  uint    array;        //count of bytes in array
  uint    type;         //type of variable
  uint32_t max;         //max archived integer value (unsigned)
  uint    size;         //total size of archived data
}_variable_descriptor;
static _variable_descriptor vdsc;
typedef double var_double_array [];
typedef uint var_uint_array [];
typedef Vect var_vect_array [];
//=============================================================================
uint vdsc_fill(uint8_t *buf,uint var_idx)
{
#define USESIG(aname) USEVAR(aname)
#define USEVAR(aname) \
  case idx_##aname:\
    vdsc.ptr=&(var. aname);\
    vdsc.sbytes=var_bytes_##aname;\
    vdsc.span=var_span_##aname;\
    vdsc.array=var_array_##aname;\
    vdsc.type=var_type_##aname;\
    vdsc.max=var_max_##aname;\
    vdsc.size=var_size_##aname;\
  break;
  vdsc.buf=buf;
  switch (var_idx) {
#include <mandala_vars.h>
#undef USEVAR
#undef USESIG
    default:
      return 0;
  }
  return vdsc.size;
}
//=============================================================================
static uint32_t limit_u(const float v,const uint32_t max)
{
  if (v<0)return 0;
  if (v>max)return max;
  return v;
}
static uint32_t limit_ui(const uint32_t v,const uint32_t max)
{
  if (v>max)return max;
  return v;
}
static int32_t limit_s(const float v,const uint32_t max)
{
  float min=-((float)max);
  if (v<min)return min;
  if (v>max)return max;
  return v;
}
//=============================================================================
static void archive_float(void)
{ //little endian (LSB first)
  if (vdsc.array>1) {
    uint ai,sz=vdsc.array;
    vdsc.array=1;
    var_double_array *v=((var_double_array*)vdsc.ptr);
    for (ai=0;ai<sz;ai++) {
      vdsc.ptr=&((*v)[ai]);
      archive_float();
    }
    return;
  }
  float v=*((double*)vdsc.ptr);
  if (vdsc.span!=0.0)v=vdsc.max*v/vdsc.span;
  switch (vdsc.sbytes) {
    case -4: {    //int32
      int32_t vs=limit_s(v,vdsc.max);
      uint32_t vi=*((uint32_t*)&vs);
      *(vdsc.buf++)=vi;
      *(vdsc.buf++)=vi>>8;
      *(vdsc.buf++)=vi>>16;
      *(vdsc.buf++)=vi>>24;
    }break;
    case -2: {    //int16
      int16_t vs=limit_s(v,vdsc.max);
      uint16_t vi=*((uint16_t*)&vs);
      *(vdsc.buf++)=vi;
      *(vdsc.buf++)=vi>>8;
    }break;
    case -1:     //int8
      *((int8_t*)(vdsc.buf++))=limit_s(v,vdsc.max);
      break;
    case 1:     //uint8
      *((uint8_t*)(vdsc.buf++))=limit_u(v,vdsc.max);
      break;
    case 2: {    //uint16
      uint16_t vi=limit_u(v,vdsc.max);
      *(vdsc.buf++)=vi;
      *(vdsc.buf++)=vi>>8;
    }break;
    case 4: {    //uint32
      uint32_t vi=limit_u(v,vdsc.max);
      *(vdsc.buf++)=vi;
      *(vdsc.buf++)=vi>>8;
      *(vdsc.buf++)=vi>>16;
      *(vdsc.buf++)=vi>>24;
    }break;
  }
}
static void archive_uint(void)
{ //little endian (LSB first)
  if (vdsc.array>1) {
    uint ai,sz=vdsc.array;
    vdsc.array=1;
    var_uint_array *v=((var_uint_array*)vdsc.ptr);
    for (ai=0;ai<sz;ai++) {
      vdsc.ptr=&((*v)[ai]);
      archive_uint();
    }
    return;
  }
  uint v=*((uint*)vdsc.ptr);
  switch (vdsc.sbytes) {
    case 1:     //uint8
      *(vdsc.buf++)=limit_ui(v,vdsc.max);
      break;
    case 2: {    //uint16
      uint16_t vi=limit_ui(v,vdsc.max);
      *(vdsc.buf++)=vi;
      *(vdsc.buf++)=vi>>8;
    }break;
    case 4: {    //uint32
      uint32_t vi=limit_ui(v,vdsc.max);
      *(vdsc.buf++)=vi;
      *(vdsc.buf++)=vi>>8;
      *(vdsc.buf++)=vi>>16;
      *(vdsc.buf++)=vi>>24;
    }break;
  }
}
static void archive_vect(void)
{
  if (vdsc.array>1) {
    uint ai,sz=vdsc.array;
    vdsc.array=1;
    var_vect_array *v=((var_vect_array*)vdsc.ptr);
    for (ai=0;ai<sz;ai++) {
      vdsc.ptr=&((*v)[ai][0]);
      archive_vect();
    }
    return;
  }
  Vect *v=((Vect*)vdsc.ptr);
  vdsc.ptr=&((*v)[0]);
  archive_float();
  vdsc.ptr=&((*v)[1]);
  archive_float();
  vdsc.ptr=&((*v)[2]);
  archive_float();
}
static void archive_sig()
{
  const uint8_t *signature=*((const uint8_t **)vdsc.ptr);
  uint sz,cnt=0,scnt=signature[0];
  signature++;
  while (scnt--) {
    sz=archive(vdsc.buf,*signature++);
    if (!sz){ //error, var not found in USEVAR()
      vdsc.size=0;
      return;
    }
    cnt+=sz;
  }
  vdsc.size=cnt;
}
//-----------------------------------------------------------------------------
uint archive(uint8_t *buf,uint var_idx)
{
  if (!vdsc_fill(buf,var_idx))return 0;
  switch (vdsc.type) {
    case vt_double:
      archive_float();
      break;
    case vt_uint:
      archive_uint();
      break;
    case vt_Vect:
      archive_vect();
      break;
    case vt_sig:
      archive_sig();
      break;
  }
  return vdsc.size;
}
//=============================================================================
//=============================================================================
//=============================================================================
static void extract_float(void)
{
  if (vdsc.array>1) {
    uint ai,sz=vdsc.array;
    vdsc.array=1;
    var_double_array *v=((var_double_array*)vdsc.ptr);
    for (ai=0;ai<sz;ai++) {
      vdsc.ptr=&((*v)[ai]);
      extract_float();
    }
    return;
  }
  double *v=(double*)vdsc.ptr;
  switch (vdsc.sbytes) {
    case -4: {    //int32
      uint32_t vi=*(vdsc.buf++);
      vi|=(uint32_t)(*(vdsc.buf++))<<8;
      vi|=(uint32_t)(*(vdsc.buf++))<<16;
      vi|=(uint32_t)(*(vdsc.buf++))<<24;
      *v=*((int32_t*)&vi);
    }break;
    case -2: {    //int16
      uint16_t vi=*(vdsc.buf++);
      vi|=(uint16_t)(*(vdsc.buf++))<<8;
      *v=*((int16_t*)&vi);
    }break;
    case -1:     //int8
      *v=*((int8_t*)(vdsc.buf++));
      break;
    case 1:     //uint8
      *v=*((uint8_t*)(vdsc.buf++));
      break;
    case 2: {    //uint16
      uint16_t vi=*(vdsc.buf++);
      vi|=(uint16_t)(*(vdsc.buf++))<<8;
      *v=vi;
    }break;
    case 4: {    //uint32
      uint32_t vi=*(vdsc.buf++);
      vi|=(uint32_t)(*(vdsc.buf++))<<8;
      vi|=(uint32_t)(*(vdsc.buf++))<<16;
      vi|=(uint32_t)(*(vdsc.buf++))<<24;
      *v=vi;
    }break;
  }
  if (vdsc.span!=0.0)
    *v=(*v)/vdsc.max*vdsc.span;
}
static void extract_uint(void)
{
  if (vdsc.array>1) {
    uint ai,sz=vdsc.array;
    vdsc.array=1;
    var_uint_array *v=((var_uint_array*)vdsc.ptr);
    for (ai=0;ai<sz;ai++) {
      vdsc.ptr=&((*v)[ai]);
      extract_uint();
    }
    return;
  }
  uint *v=(uint*)vdsc.ptr;
  switch (vdsc.sbytes) {
    case 1:     //uint8
      *v=*(vdsc.buf++);
      break;
    case 2: {    //uint16
      *v=*(vdsc.buf++);
      *v|=(uint16_t)(*(vdsc.buf++))<<8;
    }break;
    case 4: {    //uint32
      *v=*(vdsc.buf++);
      *v|=(uint32_t)(*(vdsc.buf++))<<8;
      *v|=(uint32_t)(*(vdsc.buf++))<<16;
      *v|=(uint32_t)(*(vdsc.buf++))<<24;
    }break;
  }
}
static void extract_vect(void)
{
  if (vdsc.array>1) {
    uint ai,sz=vdsc.array;
    vdsc.array=1;
    var_vect_array *v=((var_vect_array*)vdsc.ptr);
    for (ai=0;ai<sz;ai++) {
      vdsc.ptr=&((*v)[ai][0]);
      extract_vect();
    }
    return;
  }
  Vect *v=((Vect*)vdsc.ptr);
  vdsc.ptr=&((*v)[0]);
  extract_float();
  vdsc.ptr=&((*v)[1]);
  extract_float();
  vdsc.ptr=&((*v)[2]);
  extract_float();
}
static void extract_sig(uint buf_cnt)
{
  const uint8_t *signature=*((const uint8_t **)vdsc.ptr);
  uint sz,cnt=0,scnt=signature[0];
  signature++;
  while (scnt--) {
    sz=extract(vdsc.buf,buf_cnt,*signature++);
    if (!sz){ //error, var not found in USEVAR()
      vdsc.size=0;
      return;
    }
    cnt+=sz;
    buf_cnt-=sz;
  }
  vdsc.size=cnt;
}
//-----------------------------------------------------------------------------
uint extract(uint8_t *buf,uint cnt,uint var_idx)
{
  if (!vdsc_fill(buf,var_idx))return 0;
  if (cnt<vdsc.size)return 0;
  switch (vdsc.type) {
    case vt_double:
      extract_float();
      break;
    case vt_uint:
      extract_uint();
      break;
    case vt_Vect:
      extract_vect();
      break;
    case vt_sig:
      extract_sig(cnt);
      break;
  }
  return vdsc.size;
}
//=============================================================================
uint extract_packet(uint8_t *buf,uint cnt)
{
  return extract(buf+1,cnt-1,buf[0]);
}
//=============================================================================
//=============================================================================
double var_get_value(uint var_idx,uint member_idx)
{
  if (!vdsc_fill(0,var_idx))return 0;
  if(vdsc.type==vt_double){
    if (vdsc.array>1) return (*((var_double_array*)vdsc.ptr))[member_idx];
    return *(double*)vdsc.ptr;
  }
  if(vdsc.type==vt_Vect){
    return (*(Vect*)vdsc.ptr)[member_idx];
  }
  if(vdsc.type==vt_uint){
    if (vdsc.array>1) return (*((var_uint_array*)vdsc.ptr))[member_idx];
    if(member_idx) return ((*(uint*)vdsc.ptr)&member_idx)?1.0:0.0;
    return *(uint*)vdsc.ptr;
  }
  return 0;
}
//=============================================================================
void var_set_value(uint var_idx,uint member_idx,double value)
{
  if (!vdsc_fill(0,var_idx))return;
  if(vdsc.type==vt_double){
    if (vdsc.array>1) (*((var_double_array*)vdsc.ptr))[member_idx]=value;
    else *(double*)vdsc.ptr=value;
  }else if(vdsc.type==vt_Vect){
    (*(Vect*)vdsc.ptr)[member_idx]=value;
  }else if(vdsc.type==vt_uint){
    if (vdsc.array>1) (*((var_uint_array*)vdsc.ptr))[member_idx]=value;
    else if(member_idx)
      (*(uint*)vdsc.ptr)=(value==0.0)?((*(uint*)vdsc.ptr)&(~member_idx)):((*(uint*)vdsc.ptr)|member_idx);
    else *(uint*)vdsc.ptr=value;
  }
}
//=============================================================================
//=============================================================================


