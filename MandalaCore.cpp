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
#include "math.h"
//===========================================================================
// static signatures
#define SIGDEF(aname, adescr, ... )   \
  static const uint8_t signature_##aname [ VA_NUM_ARGS(__VA_ARGS__)+1 ]={VA_NUM_ARGS(__VA_ARGS__), __VA_ARGS__ }; \
  var_typedef_##aname MandalaCore:: aname = signature_##aname ;
#include "MandalaVars.h"
//=============================================================================
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
  //_variable_descriptor
  #define VARDEF(atype,aname,aspan,abytes,adescr)         VARDEFA(atype,aname,1,aspan,abytes,adescr)
  #define SIGDEF(aname,adescr,...)                        VARDEFA(sig,aname,VA_NUM_ARGS(__VA_ARGS__),0,1,adescr)

  #define VARDEFA(atype,aname,asize,aspan,abytes,adescr) \
  case idx_##aname: { \
      vdsc.ptr=(void*)&(aname);\
      vdsc.sbytes=(aspan<0)?(-abytes):(abytes);\
      vdsc.span=(aspan<0)?(-aspan):(aspan);\
      vdsc.array=asize;\
      vdsc.type=vt_##atype;\
      vdsc.max=(aspan>=0)?(((abytes==1)?0xFF:((abytes==2)?0xFFFF:((abytes==4)?0xFFFFFFFF:0)))): \
                        ( (aspan<0)?((abytes==1)?0x7F:((abytes==2)?0x7FFF:((abytes==4)?0x7FFFFFFF:0))):0 );\
      vdsc.size=((asize)*(abytes)*((vt_##atype==vt_vect)?3:1));\
    } break;
    vdsc.buf=buf;
    vdsc.prec1000=0;
    switch (var_idx) {
      #include "MandalaVars.h"
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
  uint8_t *buf=vdsc.buf;
  uint32_t max=vdsc.max;
  _var_float v=*((_var_float*)vdsc.ptr);
  if (vdsc.span!=0.0)v=vdsc.max*v/vdsc.span;
  switch (vdsc.sbytes) {
    case -4: {    //int32
        int32_t vs=limit_s(v,max);
        uint32_t vi=*((uint32_t*)&vs);
        *(buf++)=vi;
        *(buf++)=vi>>8;
        *(buf++)=vi>>16;
        *(buf++)=vi>>24;
    }break;
    case -2: {    //int16
        int16_t vs=limit_s(v,max);
        uint16_t vi=*((uint16_t*)&vs);
        *(buf++)=vi;
        *(buf++)=vi>>8;
    }break;
    case -1:     //int8
        *((int8_t*)(buf++))=limit_s(v,max);
        break;
    case 1:     //uint8
        *((uint8_t*)(buf++))=limit_u(v,max);
        break;
    case 2: {    //uint16
        uint16_t vi=limit_u(v,max);
        *(buf++)=vi;
        *(buf++)=vi>>8;
    }break;
    case 4: {    //uint32
        uint32_t vi=limit_u(v,max);
        *(buf++)=vi;
        *(buf++)=vi>>8;
        *(buf++)=vi>>16;
        *(buf++)=vi>>24;
    }break;
  }
  vdsc.buf=buf;
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
  uint8_t *buf=vdsc.buf;
  uint32_t max=vdsc.max;
  _var_uint v=*((_var_uint*)vdsc.ptr);
  switch (vdsc.sbytes) {
    case 1:     //uint8
        *(buf++)=limit_ui(v,max);
        break;
    case 2: {    //uint16
        uint16_t vi=limit_ui(v,max);
        *(buf++)=vi;
        *(buf++)=vi>>8;
    }break;
    case 4: {    //uint32
        uint32_t vi=limit_ui(v,max);
        *(buf++)=vi;
        *(buf++)=vi>>8;
        *(buf++)=vi>>16;
        *(buf++)=vi>>24;
    }break;
  }
  vdsc.buf=buf;
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
    sz=do_archive(vdsc.buf,*signature++);
    if (!sz){ //error, var not found in USEVAR()
      vdsc.size=0;
      return;
    }
    cnt+=sz;
  }
  vdsc.size=cnt;
}
//-----------------------------------------------------------------------------
uint MandalaCore::do_archive(uint8_t *buf,uint var_idx)
{
  if (!vdsc_fill(buf,var_idx))return 0;
  return do_archive_vdsc();
}
uint MandalaCore::do_archive_vdsc(void)
{
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
  uint8_t *buf=vdsc.buf;
  _var_float *v=(_var_float*)vdsc.ptr;
  switch (vdsc.sbytes) {
    case -4: {    //int32
      uint32_t vi=*(buf++);
      vi|=(uint32_t)(*(buf++))<<8;
      vi|=(uint32_t)(*(buf++))<<16;
      vi|=(uint32_t)(*(buf++))<<24;
      *v=*((int32_t*)&vi);
    }break;
    case -2: {    //int16
      uint16_t vi=*(buf++);
      vi|=(uint16_t)(*(buf++))<<8;
      *v=*((int16_t*)&vi);
    }break;
    case -1:     //int8
      *v=*((int8_t*)(buf++));
      break;
    case 1:     //uint8
      *v=*((uint8_t*)(buf++));
      break;
    case 2: {    //uint16
      uint16_t vi=*(buf++);
      vi|=(uint16_t)(*(buf++))<<8;
      *v=vi;
    }break;
    case 4: {    //uint32
      uint32_t vi=*(buf++);
      vi|=(uint32_t)(*(buf++))<<8;
      vi|=(uint32_t)(*(buf++))<<16;
      vi|=(uint32_t)(*(buf++))<<24;
      *v=vi;
    }break;
  }
  vdsc.buf=buf;
  if (vdsc.span!=0.0)
    *v=(*v)/(_var_float)vdsc.max*vdsc.span;
  if(vdsc.prec1000){
    _var_float prec=(_var_float)vdsc.prec1000/1000.0;
    //uint64_t vp=*v/prec;
    *v=round(*v/prec)*prec;
  }
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
  uint8_t *buf=vdsc.buf;
  _var_uint *v=(_var_uint*)vdsc.ptr;
  switch (vdsc.sbytes) {
    case 1:     //uint8
      *v=*(buf++);
      break;
    case 2: {    //uint16
      *v=*(buf++);
      *v|=(uint16_t)(*(buf++))<<8;
    }break;
    case 4: {    //uint32
      *v=*(buf++);
      *v|=(uint32_t)(*(buf++))<<8;
      *v|=(uint32_t)(*(buf++))<<16;
      *v|=(uint32_t)(*(buf++))<<24;
    }break;
  }
  vdsc.buf=buf;
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
    sz=do_extract(vdsc.buf,buf_cnt,*signature++);
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
uint MandalaCore::do_extract(uint8_t *buf,uint cnt,uint var_idx)
{
  if (!vdsc_fill(buf,var_idx))return 0;
  return do_extract_vdsc(cnt);
}
uint MandalaCore::do_extract_vdsc(uint cnt)
{
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
uint MandalaCore::extract_stream(uint8_t *buf,uint cnt)
{
  uint mask=1;
  uint8_t *mask_ptr=buf;
  uint8_t *ptr=mask_ptr+1;
  int tcnt=cnt;
  uint idx=idxPAD;
  do{
    for(uint i=0;i<8;i++){
      if((*mask_ptr)&mask){
        if(tcnt<=0)return 0;
        uint sz=do_extract(ptr,tcnt,idx);
        if(!sz)return 0;
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
  return cnt;
}
//=============================================================================
//=============================================================================
_var_float MandalaCore::get_value(uint var_idx,uint member_idx)
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
void MandalaCore::set_value(uint var_idx,uint member_idx,_var_float value)
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
void MandalaCore::filter(const _var_float &v,_var_float *var_p,const _var_float &S,const _var_float &L)
{
  const _var_float D=v-*var_p;
  _var_float G=S+S*(D/L)*(D/L);
  if (*var_p==0.0)*var_p=v;
  else {
    if (G>1)G=1;
    else if (G<S)G=S;
    *var_p=*var_p+D*G;
  }
}
void MandalaCore::filter(const _var_vect &v,_var_vect *var_p,const _var_float &S,const _var_float &L)
{
  for(uint i=0;i<3;i++)filter(v[i],&((*var_p)[i]),S,L);
}
//===========================================================================
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
const Vector Vector::operator*(const _var_float &scale)const
{return Vector(*this)*=scale;}
Vector &Vector::operator*=(const _var_float &scale)
{for(uint i=0;i<3;i++)(*this)[i]*=scale;return (*this);}
const Vector Vector::operator/(const _var_float &scale)const
{return Vector(*this)/=scale;}
Vector &Vector::operator/=(const _var_float &scale)
{for(uint i=0;i<3;i++)(*this)[i]/=scale;return (*this);}
//=============================================================================
//=============================================================================
//=============================================================================
