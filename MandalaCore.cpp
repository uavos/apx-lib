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
  static  uint8_t signature_##aname [ VA_NUM_ARGS(__VA_ARGS__)+1 ]={VA_NUM_ARGS(__VA_ARGS__), __VA_ARGS__ }; \
  var_typedef_##aname MandalaCore:: aname = signature_##aname ;
#include "MandalaVars.h"
//=============================================================================
MandalaCore::MandalaCore()
  : alt_bytecnt(false)
{
}
//===========================================================================
_var_float MandalaCore::get_value(uint var_m)
{
  return get_value(var_m&0xFF,var_m>>8);
}
_var_float MandalaCore::get_value(uint var_idx,uint member_idx)
{
  uint type;
  void *value_ptr;
  if(!get_ptr(var_idx,&value_ptr,&type))return 0;
  switch(type){
    case vt_float:return *(_var_float*)value_ptr;
    case vt_vect: return (*(_var_vect*)value_ptr)[member_idx];
    case vt_uint: return *(_var_uint*)value_ptr;
    case vt_bits:
      if(member_idx) return ((*(_var_bits*)value_ptr)&member_idx)?1:0;
      else return *(_var_bits*)value_ptr;
  }
  return 0;
}
//=============================================================================
void MandalaCore::set_value(uint var_m,_var_float value)
{
  set_value(var_m&0xFF,var_m>>8,value);
}
void MandalaCore::set_value(uint var_idx,uint member_idx,_var_float value)
{
  uint type;
  void *value_ptr;
  if(!get_ptr(var_idx,&value_ptr,&type))return;
  switch(type){
    case vt_float:*(_var_float*)value_ptr=value;break;
    case vt_vect: (*(_var_vect*)value_ptr)[member_idx]=value;break;
    case vt_uint: *(_var_uint*)value_ptr=value;break;
    case vt_bits:
      if(member_idx) (*(_var_bits*)value_ptr)=(value<=0.0)?((*(_var_bits*)value_ptr)&(~member_idx)):((*(_var_bits*)value_ptr)|member_idx);
      else *(_var_bits*)value_ptr=value;
      break;
  }
}
//=============================================================================
bool MandalaCore::get_ptr(uint var_idx,void **value_ptr,uint*type)
{
  switch (var_idx) {
    #define VARDEF(atype,aname,aspan,abytes,atbytes,adescr) \
      case idx_##aname: *type=vt_##atype;*value_ptr=(void*)&(aname);return true;
    #include "MandalaVars.h"
  }
  return false;
}
//=============================================================================
uint MandalaCore::pack(uint8_t *buf,uint var_idx)
{
  if(alt_bytecnt){
    switch (var_idx) {
      #define SIGDEF(aname,adescr,...) \
        case idx_##aname: return pack_sig(buf,(void*)&(aname));
      #define VARDEF(atype,aname,aspan,abytes,atbytes,adescr) \
        case idx_##aname: return pack_##atype##_##atbytes (buf,(void*)&(aname),aspan);
      #include "MandalaVars.h"
    }
  }else{
    switch (var_idx) {
      #define SIGDEF(aname,adescr,...) \
        case idx_##aname: return pack_sig(buf,(void*)&(aname));
      #define VARDEF(atype,aname,aspan,abytes,atbytes,adescr) \
        case idx_##aname: return pack_##atype##_##abytes (buf,(void*)&(aname),aspan);
      #include "MandalaVars.h"
    }
  }
  return 0;
}
//------------------------------------------------------------------------------
uint MandalaCore::unpack(uint8_t *buf,uint cnt,uint var_idx)
{
  if(alt_bytecnt){
    switch (var_idx) {
      #define SIGDEF(aname,adescr,...) \
        case idx_##aname: return unpack_sig(buf,cnt,(void*)&(aname));
      #define VARDEF(atype,aname,aspan,abytes,atbytes,adescr) \
        case idx_##aname: return (cnt>=atbytes)?unpack_##atype##_##atbytes (buf,(void*)&(aname),aspan):0;
      #include "MandalaVars.h"
    }
  }else{
    switch (var_idx) {
      #define SIGDEF(aname,adescr,...) \
        case idx_##aname: return unpack_sig(buf,cnt,(void*)&(aname));
      #define VARDEF(atype,aname,aspan,abytes,atbytes,adescr) \
        case idx_##aname: return (cnt>=abytes)?unpack_##atype##_##abytes (buf,(void*)&(aname),aspan):0;
      #include "MandalaVars.h"
    }
  }
  return 0;
}
//==============================================================================
uint MandalaCore::pack_float_1(void *buf,void *value_ptr,_var_float span)
{
  _var_float f=*((_var_float*)value_ptr);
  if (span==0){
    uint32_t v=f;
    if(v>0xFF)v=0xFF;
    *((uint8_t*)buf)=v;
  }else if (span==-127){
    int32_t v=f;
    if(v>127)v=127;
    else if(v<-128)v=-128;
    *((int8_t*)buf)=v;
  }else if (span==-12.7){
    int32_t v=f*10.0;
    if(v>127)v=127;
    else if(v<-128)v=-128;
    *((int8_t*)buf)=v;
  }else if (span==25.5){
    int32_t v=f*10.0;
    if(v>0xFF)v=0xFF;
    else if(v<0)v=0;
    *((uint8_t*)buf)=v;
  }else if (span>0){
    if(f>span)f=span;
    else if(f<-span)f=-span;
    uint32_t v=0xFF*f/span;
    if(v>0xFF)v=0xFF;
    *((uint8_t*)buf)=v;
  }else if(span<0){
    if(f>-span)f=-span;
    else if(f<span)f=span;
    int32_t v=0x7F*f/(-span);
    if(v>0x7F)v=0x7F;
    else if(v<-(int)0x80)v=-(int)0x80;
    *((int8_t*)buf)=v;
  }
  return 1;
}
//------------------------------------------------------------------------------
uint MandalaCore::pack_float_2(void *buf,void *value_ptr,_var_float span)
{ //IEEE 754r
  (void)span;
  float f=*((_var_float*)value_ptr);
  uint8_t *ptr=(uint8_t*)&f;
  uint32_t x = ptr[0]|ptr[1]<<8|ptr[2]<<16|ptr[3]<<24,xs,xe,xm;
  uint16_t hs,he,hm;
  uint16_t *hp=(uint16_t*)buf;
  int hes;
  if( (x & 0x7FFFFFFFu) == 0 ) {  // Signed zero
    *hp++ = (uint16_t) (x >> 16);  // Return the signed zero
  } else { // Not zero
    xs = x & 0x80000000u;  // Pick off sign bit
    xe = x & 0x7F800000u;  // Pick off exponent bits
    xm = x & 0x007FFFFFu;  // Pick off mantissa bits
    if( xe == 0 ) {  // Denormal will underflow, return a signed zero
      *hp++ = (uint16_t) (xs >> 16);
    } else if( xe == 0x7F800000u ) {  // Inf or NaN (all the exponent bits are set)
      if( xm == 0 ) { // If mantissa is zero ...
        *hp++ = (uint16_t) ((xs >> 16) | 0x7C00u); // Signed Inf
      } else {
        *hp++ = (uint16_t) 0xFE00u; // NaN, only 1st mantissa bit set
      }
    } else { // Normalized number
      hs = (uint16_t) (xs >> 16); // Sign bit
      hes = ((int)(xe >> 23)) - 127 + 15; // Exponent unbias the single, then bias the halfp
      if( hes >= 0x1F ) {  // Overflow
        *hp++ = (uint16_t) ((xs >> 16) | 0x7C00u); // Signed Inf
      } else if( hes <= 0 ) {  // Underflow
        if( (14 - hes) > 24 ) {  // Mantissa shifted all the way off & no rounding possibility
          hm = (uint16_t) 0u;  // Set mantissa to zero
        } else {
          xm |= 0x00800000u;  // Add the hidden leading bit
          hm = (uint16_t) (xm >> (14 - hes)); // Mantissa
          if( (xm >> (13 - hes)) & 0x00000001u ) // Check for rounding
            hm += (uint16_t) 1u; // Round, might overflow into exp bit, but this is OK
        }
        *hp++ = (hs | hm); // Combine sign bit and mantissa bits, biased exponent is zero
      } else {
        he = (uint16_t) (hes << 10); // Exponent
        hm = (uint16_t) (xm >> 13); // Mantissa
        if( xm & 0x00001000u ) // Check for rounding
          *hp++ = (hs | he | hm) + (uint16_t) 1u; // Round, might overflow to inf, this is OK
          else
            *hp++ = (hs | he | hm);  // No rounding
      }
    }
  }
  return 2;
}
//------------------------------------------------------------------------------
uint MandalaCore::pack_float_4(void *buf,void *value_ptr,_var_float span)
{
  (void)span;
  float f=*(_var_float*)value_ptr;
  uint8_t *src=(uint8_t*)&f;
  uint8_t *dest=(uint8_t*)buf;
  *dest++=*src++;
  *dest++=*src++;
  *dest++=*src++;
  *dest=*src;
  return 4;
}
//------------------------------------------------------------------------------
uint MandalaCore::pack_uint_1(void *buf,void *value_ptr,_var_float span)
{
  (void)span;
  uint v=*((_var_uint*)value_ptr);
  if(v>0xFF)v=0xFF;
  *((uint8_t*)buf)=v;
  return 1;
}
//------------------------------------------------------------------------------
uint MandalaCore::pack_bits_1(void *buf,void *value_ptr,_var_float span)
{
  (void)span;
  *((uint8_t*)buf)=*((_var_bits*)value_ptr);
  return 1;
}
//------------------------------------------------------------------------------
uint MandalaCore::pack_uint_2(void *buf,void *value_ptr,_var_float span)
{
  (void)span;
  uint v=*((_var_uint*)value_ptr);
  if(v>0xFFFF)v=0xFFFF;
  *((uint16_t*)buf)=v;
  return 2;
}
//------------------------------------------------------------------------------
uint MandalaCore::pack_uint_4(void *buf,void *value_ptr,_var_float span)
{
  (void)span;
  uint v=*((_var_uint*)value_ptr);
  *((uint32_t*)buf)=v;
  return 4;
}
//------------------------------------------------------------------------------
uint MandalaCore::pack_vect_1(void *buf,void *value_ptr,_var_float span)
{
  _var_vect *v=((_var_vect*)value_ptr);
  uint8_t *ptr=(uint8_t*)buf;
  pack_float_1(ptr++,&((*v)[0]),span);
  pack_float_1(ptr++,&((*v)[1]),span);
  pack_float_1(ptr,&((*v)[2]),span);
  return 3;
}
//------------------------------------------------------------------------------
uint MandalaCore::pack_vect_2(void *buf,void *value_ptr,_var_float span)
{
  _var_vect *v=((_var_vect*)value_ptr);
  uint16_t *ptr=(uint16_t*)buf;
  pack_float_2(ptr++,&((*v)[0]),span);
  pack_float_2(ptr++,&((*v)[1]),span);
  pack_float_2(ptr,&((*v)[2]),span);
  return 6;
}
//------------------------------------------------------------------------------
uint MandalaCore::pack_vect_4(void *buf,void *value_ptr,_var_float span)
{
  _var_vect *v=((_var_vect*)value_ptr);
  uint32_t *ptr=(uint32_t*)buf;
  pack_float_4(ptr++,&((*v)[0]),span);
  pack_float_4(ptr++,&((*v)[1]),span);
  pack_float_4(ptr,&((*v)[2]),span);
  return 12;
}
//------------------------------------------------------------------------------
uint MandalaCore::pack_sig(void *buf,void *value_ptr)
{
  _var_signature signature=*((_var_signature*)value_ptr);
  uint cnt=0,scnt=signature[0];
  signature++;
  uint8_t *ptr=(uint8_t*)buf;
  while (scnt--) {
    uint sz=pack(ptr,*signature++);
    if(!sz)break;
    ptr+=sz;
    cnt+=sz;
  }
  return cnt;
}
//=============================================================================
uint MandalaCore::unpack_float_1(void *buf,void *value_ptr,_var_float span)
{
  _var_float v=0;
  if (span==0){
    v=*((uint8_t*)buf);
  }else if (span==-127){
    v=(_var_float)(*((int8_t*)buf));
  }else if (span==-12.7){
    v=(_var_float)(*((int8_t*)buf))*0.1;
  }else if (span==25.5){
    v=(_var_float)(*((uint8_t*)buf))*0.1;
  }else if (span>0){
    v=(_var_float)(*((uint8_t*)buf))*(span/(_var_float)0xFF);
  }else if(span<0){
    v=(_var_float)(*((int8_t*)buf))*(span/(_var_float)(-0x80));
  }
  *((_var_float*)value_ptr)=v;
  return 1;
}
//------------------------------------------------------------------------------
uint MandalaCore::unpack_float_2(void *buf,void *value_ptr,_var_float span)
{
  (void)span;
  float f;
  uint16_t h=*((uint16_t*)buf),hs,he,hm;
  uint32_t *xp=(uint32_t*)&f;//(uint32_t*)value_ptr;
  uint32_t xs,xe,xm;
  int32_t xes;
  int e;
  if( (h & 0x7FFFu) == 0 ) {  // Signed zero
    *xp++ = ((uint32_t) h) << 16;  // Return the signed zero
  } else { // Not zero
    hs = h & 0x8000u;  // Pick off sign bit
    he = h & 0x7C00u;  // Pick off exponent bits
    hm = h & 0x03FFu;  // Pick off mantissa bits
    if( he == 0 ) {  // Denormal will convert to normalized
      e = -1; // The following loop figures out how much extra to adjust the exponent
      do {
        e++;
        hm <<= 1;
      } while( (hm & 0x0400u) == 0 ); // Shift until leading bit overflows into exponent bit
      xs = ((uint32_t) hs) << 16; // Sign bit
      xes = ((int32_t) (he >> 10)) - 15 + 127 - e; // Exponent unbias the halfp, then bias the single
      xe = (uint32_t) (xes << 23); // Exponent
      xm = ((uint32_t) (hm & 0x03FFu)) << 13; // Mantissa
      *xp++ = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
    } else if( he == 0x7C00u ) {  // Inf or NaN (all the exponent bits are set)
      if( hm == 0 ) { // If mantissa is zero ...
        *xp++ = (((uint32_t) hs) << 16) | ((uint32_t) 0x7F800000u); // Signed Inf
      } else {
        *xp++ = (uint32_t) 0xFFC00000u; // NaN, only 1st mantissa bit set
      }
    } else { // Normalized number
      xs = ((uint32_t) hs) << 16; // Sign bit
      xes = ((int32_t) (he >> 10)) - 15 + 127; // Exponent unbias the halfp, then bias the single
      xe = (uint32_t) (xes << 23); // Exponent
      xm = ((uint32_t) hm) << 13; // Mantissa
      *xp++ = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
    }
  }
  if(isnan(f))f=0;
  *((_var_float*)value_ptr)=f;
  return 2;
}
//------------------------------------------------------------------------------
uint MandalaCore::unpack_float_4(void *buf,void *value_ptr,_var_float span)
{
  (void)span;
  float f;
  uint8_t *src=(uint8_t*)buf;
  uint8_t *dest=(uint8_t*)&f;
  *dest++=*src++;
  *dest++=*src++;
  *dest++=*src++;
  *dest=*src;
  if(isnan(f))f=0;
  *((_var_float*)value_ptr)=f;
  return 4;
}
//------------------------------------------------------------------------------
uint MandalaCore::unpack_bits_1(void *buf,void *value_ptr,_var_float span)
{
  (void)span;
  *((_var_bits*)value_ptr)=*((uint8_t*)buf);
  return 1;
}
//------------------------------------------------------------------------------
uint MandalaCore::unpack_uint_1(void *buf,void *value_ptr,_var_float span)
{
  (void)span;
  *((_var_uint*)value_ptr)=*((uint8_t*)buf);
  return 1;
}
//------------------------------------------------------------------------------
uint MandalaCore::unpack_uint_2(void *buf,void *value_ptr,_var_float span)
{
  (void)span;
  *((_var_uint*)value_ptr)=*((uint16_t*)buf);
  return 2;
}
//------------------------------------------------------------------------------
uint MandalaCore::unpack_uint_4(void *buf,void *value_ptr,_var_float span)
{
  (void)span;
  *((_var_uint*)value_ptr)=*((uint32_t*)buf);
  return 4;
}
//------------------------------------------------------------------------------
uint MandalaCore::unpack_vect_1(void *buf,void *value_ptr,_var_float span)
{
  _var_vect *v=((_var_vect*)value_ptr);
  uint8_t *ptr=(uint8_t*)buf;
  unpack_float_1(ptr++,&((*v)[0]),span);
  unpack_float_1(ptr++,&((*v)[1]),span);
  unpack_float_1(ptr,&((*v)[2]),span);
  return 3;
}
//------------------------------------------------------------------------------
uint MandalaCore::unpack_vect_2(void *buf,void *value_ptr,_var_float span)
{
  _var_vect *v=((_var_vect*)value_ptr);
  uint16_t *ptr=(uint16_t*)buf;
  unpack_float_2(ptr++,&((*v)[0]),span);
  unpack_float_2(ptr++,&((*v)[1]),span);
  unpack_float_2(ptr,&((*v)[2]),span);
  return 6;
}
//------------------------------------------------------------------------------
uint MandalaCore::unpack_vect_4(void *buf,void *value_ptr,_var_float span)
{
  _var_vect *v=((_var_vect*)value_ptr);
  uint32_t *ptr=(uint32_t*)buf;
  unpack_float_4(ptr++,&((*v)[0]),span);
  unpack_float_4(ptr++,&((*v)[1]),span);
  unpack_float_4(ptr,&((*v)[2]),span);
  return 12;
}
//------------------------------------------------------------------------------
uint MandalaCore::unpack_sig(void *buf,uint cnt,void *value_ptr)
{
  _var_signature signature=*((_var_signature*)value_ptr);
  uint rcnt=0,scnt=signature[0];
  signature++;
  uint8_t *ptr=(uint8_t*)buf;
  while ((scnt--)&&cnt) {
    uint sz=unpack(ptr,cnt,*signature++);
    if(!sz)break;
    ptr+=sz;
    rcnt+=sz;
    cnt-=sz;
  }
  return rcnt;
}
//------------------------------------------------------------------------------
uint MandalaCore::unpack_stream(uint8_t *buf,uint cnt)
{
  uint mask=1;
  uint8_t *mask_ptr=buf;
  uint8_t *ptr=mask_ptr+1;
  int tcnt=cnt-1;
  uint idx=idxPAD;
  do{
    for(uint i=0;i<8;i++){
      if((*mask_ptr)&mask){
        if(tcnt<=0)break;
        uint sz=unpack(ptr,tcnt,idx);
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
void MandalaCore::filter(const _var_float &fv,_var_float *var_p,const _var_float &fS,const _var_float &fL)
{
  _var_float fD=fv-*var_p;
  _var_float fDL=fD/fL;
  _var_float fG=fS+fS*(fDL*fDL);
  if (fG>1.0)fG=1.0;
  else if (fG<fS)fG=fS;
  *var_p+=fD*fG;
}
void MandalaCore::filter(const _var_vect &v,_var_vect *var_p,const _var_float &S,const _var_float &L)
{
  for(uint i=0;i<3;i++)filter(v[i],&((*var_p)[i]),S,L);
}
void MandalaCore::filter_m(const _var_float &v,_var_float *var_p,const _var_float &f)
{
  *var_p=(*var_p)*(1.0-f)+v*f;
}
void MandalaCore::filter_m(const _var_vect &v,_var_vect *var_p,const _var_float &f)
{
  for(uint i=0;i<3;i++)filter_m(v[i],&((*var_p)[i]),f);
}
//===========================================================================
//=============================================================================
_var_float MandalaCore::inHgToAltitude(_var_float inHg,_var_float inHg_gnd)
{
  if(inHg_gnd==0) return 0;
  return (1.0-powf(inHg/inHg_gnd,0.1902632))*44330.77;
}
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
#if defined(USE_FLOAT_TYPE) && !defined(USE_MATRIX_MATH)
// Simplified Vector math
Vect::Vect()
{this->fill();}
Vect::Vect(const _var_float &s)
{this->fill(s);}
Vect::Vect(const _var_float &s0,const _var_float &s1,const _var_float &s2)
{(*this)[0]=s0;(*this)[1]=s1;(*this)[2]=s2;}
void Vect::fill(const _var_float &value)
{for(uint i=0;i<3;i++)this->v[i]=value;}
_var_float *Vect::array()
{return this->v;}
const _var_float*Vect::array()const
{return this->v;}
_var_float &Vect::operator[](unsigned int index)
{return this->v[index];}
const _var_float &Vect::operator[](unsigned int index)const
{return this->v[index];}
Vect & Vect::operator=(const _var_float value)
{for(uint i=0;i<3;i++)(*this)[i]=value;return(*this);}
bool Vect::operator==(const Vect &value)const
{for(uint i=0;i<3;i++)if((*this)[i]!=value[i])return false;return true;}
bool Vect::operator!=(const Vect &value)const
{for(uint i=0;i<3;i++)if((*this)[i]==value[i])return false;return true;}
const Vect Vect::operator+(const Vect &that)const
{return Vect(*this)+=that;}
Vect &Vect::operator+=(const Vect &that)
{for(uint i=0;i<3;i++)(*this)[i]+=that[i];return(*this);}
const Vect Vect::operator-(const Vect &that)const
{return Vect(*this)-=that;}
Vect &Vect::operator-=(const Vect &that)
{for(uint i=0;i<3;i++)(*this)[i]-=that[i];return(*this);}
const Vect Vect::operator*(const _var_float &scale)const
{return Vect(*this)*=scale;}
Vect &Vect::operator*=(const _var_float &scale)
{for(uint i=0;i<3;i++)(*this)[i]*=scale;return (*this);}
const Vect Vect::operator/(const _var_float &scale)const
{return Vect(*this)/=scale;}
Vect &Vect::operator/=(const _var_float &scale)
{for(uint i=0;i<3;i++)(*this)[i]/=scale;return (*this);}
#endif
//=============================================================================
//=============================================================================
//=============================================================================
