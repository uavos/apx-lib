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
#include "preprocessor.h"
#include "MandalaCore.h"
//===========================================================================
// special signature vars..
#define SIGDEF(aname, adescr, ... )   static const uint8_t signature_##aname [ VA_NUM_ARGS(__VA_ARGS__)+1 ]={VA_NUM_ARGS(__VA_ARGS__), __VA_ARGS__ };
#include "MandalaVars.h"

#define USEVAR(aname)
#define USESIG(aname)   var_typedef_##aname MandalaCore:: aname = signature_##aname ;
#include <mandala_vars.h>
#undef USEVAR
#undef USESIG
//=============================================================================
typedef struct {
  uint8_t *buf;         //buffer to store/extract
  void    *ptr;         //pointer to local var.VARNAME
  int     sbytes;       //archived bytes cnt (if < 0 => signed)
  _var_float   span;         //variable span (absolute, always >0)
  uint    array;        //count of bytes in array
  uint    type;         //type of variable
  uint32_t max;         //max archived integer value (unsigned)
  uint    size;         //total size of archived data
}_variable_descriptor;
static _variable_descriptor vdsc;
typedef _var_float _var_float_array [];
typedef _var_uint _var_uint_array [];
typedef _var_vect _var_vect_array [];
//===========================================================================
MandalaCore::MandalaCore()
{
}
//===========================================================================
uint MandalaCore::vdsc_fill(uint8_t *buf,uint var_idx)
{
  #define USESIG(aname) USEVAR(aname)
  #define USEVAR(aname) \
  case idx_##aname:\
    vdsc.ptr=&(aname);\
    vdsc.sbytes=var_bytes_##aname;\
    vdsc.span=var_span100_##aname/100.0;\
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
uint32_t MandalaCore::limit_u(const _var_float v,const uint32_t max)
{
  if(max==0)return v;
  if (v<0)return 0;
  if (v>max)return max;
  return v;
}
uint32_t MandalaCore::limit_ui(const uint32_t v,const uint32_t max)
{
  if(max==0)return v;
  if (v>max)return max;
  return v;
}
int32_t MandalaCore::limit_s(const _var_float v,const uint32_t max)
{
  if(max==0)return v;
  _var_float min=-((_var_float)max);
  if (v<min)return min;
  if (v>max)return max;
  return v;
}
//=============================================================================
void MandalaCore::archive_float(void)
{ //little endian (LSB first)
  if (vdsc.array>1) {
    uint ai,sz=vdsc.array;
    vdsc.array=1;
    _var_float_array *v=((_var_float_array*)vdsc.ptr);
    for (ai=0;ai<sz;ai++) {
      vdsc.ptr=&((*v)[ai]);
      archive_float();
    }
    return;
  }
  _var_float v=*((_var_float*)vdsc.ptr);
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
void MandalaCore::archive_uint(void)
{ //little endian (LSB first)
  if (vdsc.array>1) {
    uint ai,sz=vdsc.array;
    vdsc.array=1;
    _var_uint_array *v=((_var_uint_array*)vdsc.ptr);
    for (ai=0;ai<sz;ai++) {
      vdsc.ptr=&((*v)[ai]);
      archive_uint();
    }
    return;
  }
  uint v=*((_var_uint*)vdsc.ptr);
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
void MandalaCore::archive_vect(void)
{
  if (vdsc.array>1) {
    uint ai,sz=vdsc.array;
    vdsc.array=1;
    _var_vect_array *v=((_var_vect_array*)vdsc.ptr);
    for (ai=0;ai<sz;ai++) {
      vdsc.ptr=&((*v)[ai][0]);
      archive_vect();
    }
    return;
  }
  _var_vect *v=((_var_vect*)vdsc.ptr);
  vdsc.ptr=&((*v)[0]);
  archive_float();
  vdsc.ptr=&((*v)[1]);
  archive_float();
  vdsc.ptr=&((*v)[2]);
  archive_float();
}
void MandalaCore::archive_sig()
{
  _var_signature signature=*((_var_signature*)vdsc.ptr);
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
uint MandalaCore::archive(uint8_t *buf,uint var_idx)
{
  if (!vdsc_fill(buf,var_idx))return 0;
  switch (vdsc.type) {
    case vt_float:
      archive_float();
      break;
    case vt_uint:
      archive_uint();
      break;
    case vt_vect:
      archive_vect();
      break;
    case vt_sig:
      archive_sig();
      break;
  }
  return vdsc.size;
}
//=============================================================================
void MandalaCore::extract_float(void)
{
  if (vdsc.array>1) {
    uint ai,sz=vdsc.array;
    vdsc.array=1;
    _var_float_array *v=((_var_float_array*)vdsc.ptr);
    for (ai=0;ai<sz;ai++) {
      vdsc.ptr=&((*v)[ai]);
      extract_float();
    }
    return;
  }
  _var_float *v=(_var_float*)vdsc.ptr;
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
void MandalaCore::extract_uint(void)
{
  if (vdsc.array>1) {
    uint ai,sz=vdsc.array;
    vdsc.array=1;
    _var_uint_array *v=((_var_uint_array*)vdsc.ptr);
    for (ai=0;ai<sz;ai++) {
      vdsc.ptr=&((*v)[ai]);
      extract_uint();
    }
    return;
  }
  uint *v=(_var_uint*)vdsc.ptr;
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
void MandalaCore::extract_vect(void)
{
  if (vdsc.array>1) {
    uint ai,sz=vdsc.array;
    vdsc.array=1;
    _var_vect_array *v=((_var_vect_array*)vdsc.ptr);
    for (ai=0;ai<sz;ai++) {
      vdsc.ptr=&((*v)[ai][0]);
      extract_vect();
    }
    return;
  }
  _var_vect *v=((_var_vect*)vdsc.ptr);
  vdsc.ptr=&((*v)[0]);
  extract_float();
  vdsc.ptr=&((*v)[1]);
  extract_float();
  vdsc.ptr=&((*v)[2]);
  extract_float();
}
void MandalaCore::extract_sig(uint buf_cnt)
{
  _var_signature signature=*((_var_signature *)vdsc.ptr);
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
uint MandalaCore::extract(uint8_t *buf,uint cnt,uint var_idx)
{
  if (!vdsc_fill(buf,var_idx))return 0;
  if (cnt<vdsc.size)return 0;
  switch (vdsc.type) {
    case vt_float:
      extract_float();
      break;
    case vt_uint:
      extract_uint();
      break;
    case vt_vect:
      extract_vect();
      break;
    case vt_sig:
      extract_sig(cnt);
      break;
  }
  return vdsc.size;
}
//=============================================================================
//=============================================================================
double MandalaCore::get_value(uint var_idx,uint member_idx)
{
  if (!vdsc_fill(0,var_idx))return 0;
  if(vdsc.type==vt_float){
    if (vdsc.array>1) return (*((_var_float_array*)vdsc.ptr))[member_idx];
    return *(_var_float*)vdsc.ptr;
  }
  if(vdsc.type==vt_vect){
    return (*(_var_vect*)vdsc.ptr)[member_idx];
  }
  if(vdsc.type==vt_uint){
    if (vdsc.array>1) return (*((_var_uint_array*)vdsc.ptr))[member_idx];
    if(member_idx) return ((*(_var_uint*)vdsc.ptr)&member_idx)?1.0:0.0;
    return *(_var_uint*)vdsc.ptr;
  }
  return 0;
}
//=============================================================================
void MandalaCore::set_value(uint var_idx,uint member_idx,double value)
{
  if (!vdsc_fill(0,var_idx))return;
  if(vdsc.type==vt_float){
    if (vdsc.array>1) (*((_var_float_array*)vdsc.ptr))[member_idx]=value;
    else *(_var_float*)vdsc.ptr=value;
  }else if(vdsc.type==vt_vect){
    (*(_var_vect*)vdsc.ptr)[member_idx]=value;
  }else if(vdsc.type==vt_uint){
    if (vdsc.array>1) (*((_var_uint_array*)vdsc.ptr))[member_idx]=value;
    else if(member_idx)
      (*(_var_uint*)vdsc.ptr)=(value==0.0)?((*(_var_uint*)vdsc.ptr)&(~member_idx)):((*(_var_uint*)vdsc.ptr)|member_idx);
    else *(_var_uint*)vdsc.ptr=value;
  }
}
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
Vector::Vector()
{this->fill();}
Vector::Vector(const _var_float &s)
{this->fill(s);}
Vector::Vector(const _var_float &s0,const _var_float &s1,const _var_float &s2)
{(*this)[0]=s0;(*this)[1]=s1;(*this)[2]=s2;}
void Vector::fill(const _var_float &value)
{for(uint i=0;i<3;i++)this->v[i]=value;}
_var_float *Vector::array()
{return this->v;}
const _var_float*Vector::array()const
{return this->v;}
_var_float &Vector::operator[](unsigned int index)
{return this->v[index];}
const _var_float &Vector::operator[](unsigned int index)const
{return this->v[index];}
Vector & Vector::operator=(const _var_float value)
{for(uint i=0;i<3;i++)(*this)[i]=value;return(*this);}
bool Vector::operator==(const Vector &value)const
{for(uint i=0;i<3;i++)if((*this)[i]!=value[i])return false;return true;}
bool Vector::operator!=(const Vector &value)const
{for(uint i=0;i<3;i++)if((*this)[i]==value[i])return false;return true;}
const Vector Vector::operator+(const Vector &that)const
{return Vector(*this)+=that;}
Vector &Vector::operator+=(const Vector &that)
{for(uint i=0;i<3;i++)(*this)[i]+=that[i];return(*this);}
const Vector Vector::operator-(const Vector &that)const
{return Vector(*this)-=that;}
Vector &Vector::operator-=(const Vector &that)
{for(uint i=0;i<3;i++)(*this)[i]-=that[i];return(*this);}
//=============================================================================
//=============================================================================
//=============================================================================
