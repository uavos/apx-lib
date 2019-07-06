//==============================================================================
#include "Datalink.h"
#include <dmsg.h>
#include <string.h>
//==============================================================================
Datalink::Datalink()
{
}
//==============================================================================
uint Datalink::pack_downstream(uint8_t *buf,uint sz)
{

}
//==============================================================================
//==============================================================================
uint Datalink::pack_float_s(void *buf,const _mandala_float &v)
{
  *((int8_t*)buf)=(v>127)?127:((v<-128)?-128:v);
  return sizeof(int8_t);
}
uint Datalink::pack_float_u(void *buf,const _mandala_float &v)
{
  *((uint8_t*)buf)=(v>255)?255:v;
  return sizeof(uint8_t);
}
//------------------------------------------------------------------------------
uint Datalink::pack_float_s1(void *buf,void *value_ptr)
{
  return pack_float_s(buf,*((_mandala_float*)value_ptr));
}
//------------------------------------------------------------------------------
uint Datalink::pack_float_s01(void *buf,void *value_ptr)
{
  return pack_float_s(buf,*((_mandala_float*)value_ptr)*10.0);
}
//------------------------------------------------------------------------------
uint Datalink::pack_float_s001(void *buf,void *value_ptr)
{
  return pack_float_s(buf,*((_mandala_float*)value_ptr)*100.0);
}
//------------------------------------------------------------------------------
uint Datalink::pack_float_s10(void *buf,void *value_ptr)
{
  return pack_float_s(buf,*((_mandala_float*)value_ptr)/10.0);
}
//------------------------------------------------------------------------------
uint Datalink::pack_float_u1(void *buf,void *value_ptr)
{
  return pack_float_u(buf,*((_mandala_float*)value_ptr));
}
//------------------------------------------------------------------------------
uint Datalink::pack_float_u01(void *buf,void *value_ptr)
{
  return pack_float_u(buf,*((_mandala_float*)value_ptr)*10.0);
}
//------------------------------------------------------------------------------
uint Datalink::pack_float_u001(void *buf,void *value_ptr)
{
  return pack_float_u(buf,*((_mandala_float*)value_ptr)*100.0);
}
//------------------------------------------------------------------------------
uint Datalink::pack_float_u10(void *buf,void *value_ptr)
{
  return pack_float_u(buf,*((_mandala_float*)value_ptr)/10.0);
}
//------------------------------------------------------------------------------
uint Datalink::pack_float_u100(void *buf,void *value_ptr)
{
  return pack_float_u(buf,*((_mandala_float*)value_ptr)/100.0);
}
//------------------------------------------------------------------------------
uint Datalink::pack_float_f2(void *buf,void *value_ptr)
{ //IEEE 754r
  float f=*((_mandala_float*)value_ptr);
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
uint Datalink::pack_float_f4(void *buf,void *value_ptr)
{
  float f=*(_mandala_float*)value_ptr;
  uint8_t *src=(uint8_t*)&f;
  uint8_t *dest=(uint8_t*)buf;
  *dest++=*src++;
  *dest++=*src++;
  *dest++=*src++;
  *dest=*src;
  return 4;
}
//------------------------------------------------------------------------------
uint Datalink::pack_byte_u1(void *buf,void *value_ptr)
{
  *((uint8_t*)buf)=*((_var_byte*)value_ptr);
  return sizeof(uint8_t);
}
//------------------------------------------------------------------------------
uint Datalink::pack_bit_(void *buf,void *value_ptr)
{
  *((uint8_t*)buf)=*((_var_flag*)value_ptr);
  return sizeof(uint8_t);
}
//------------------------------------------------------------------------------
uint Datalink::pack_enum_(void *buf,void *value_ptr)
{
  *((uint8_t*)buf)=*((_var_enum*)value_ptr);
  return sizeof(uint8_t);
}
//------------------------------------------------------------------------------
uint Datalink::pack_uint_u4(void *buf,void *value_ptr)
{
  _var_uint v=*((_var_uint*)value_ptr);
  uint8_t *ptr=(uint8_t*)buf;
  *ptr++=v;
  *ptr++=v>>8;
  *ptr++=v>>16;
  *ptr++=v>>24;
  //*((uint32_t*)buf)=*((_var_uint*)value_ptr);
  return sizeof(uint32_t);
}
//------------------------------------------------------------------------------
uint Datalink::pack_uint_u2(void *buf,void *value_ptr)
{
  _var_uint v=*((_var_uint*)value_ptr);
  if(v>0x0000FFFF)v=0x0000FFFF;
  uint8_t *ptr=(uint8_t*)buf;
  *ptr++=v;
  *ptr++=v>>8;
  //*((uint16_t*)buf)=?0x0000FFFF:v;
  return sizeof(uint16_t);
}
//=============================================================================
uint Datalink::unpack_float_s1(const void *buf, void *value_ptr)
{
  *((_mandala_float*)value_ptr)=(_mandala_float)(*((const int8_t*)buf));
  return 1;
}
//------------------------------------------------------------------------------
uint Datalink::unpack_float_s01(const void *buf,void *value_ptr)
{
  *((_mandala_float*)value_ptr)=(_mandala_float)(*((const int8_t*)buf))/10.0;
  return 1;
}
//------------------------------------------------------------------------------
uint Datalink::unpack_float_s001(const void *buf,void *value_ptr)
{
  *((_mandala_float*)value_ptr)=(_mandala_float)(*((const int8_t*)buf))/100.0;
  return 1;
}
//------------------------------------------------------------------------------
uint Datalink::unpack_float_s10(const void *buf,void *value_ptr)
{
  *((_mandala_float*)value_ptr)=(_mandala_float)((const int32_t)*((const int8_t*)buf)*10);
  return 1;
}
//------------------------------------------------------------------------------
uint Datalink::unpack_float_u1(const void *buf,void *value_ptr)
{
  *((_mandala_float*)value_ptr)=(_mandala_float)(*((const uint8_t*)buf));
  return 1;
}
//------------------------------------------------------------------------------
uint Datalink::unpack_float_u01(const void *buf,void *value_ptr)
{
  *((_mandala_float*)value_ptr)=(_mandala_float)(*((const uint8_t*)buf))/10.0;
  return 1;
}
//------------------------------------------------------------------------------
uint Datalink::unpack_float_u001(const void *buf,void *value_ptr)
{
  *((_mandala_float*)value_ptr)=(_mandala_float)(*((const uint8_t*)buf))/100.0;
  return 1;
}
//------------------------------------------------------------------------------
uint Datalink::unpack_float_u10(const void *buf,void *value_ptr)
{
  *((_mandala_float*)value_ptr)=(_mandala_float)((const uint32_t)*((const uint8_t*)buf)*10);
  return 1;
}
//------------------------------------------------------------------------------
uint Datalink::unpack_float_u100(const void *buf,void *value_ptr)
{
  *((_mandala_float*)value_ptr)=(_mandala_float)((const uint32_t)*((const uint8_t*)buf)*100);
  return 1;
}
//------------------------------------------------------------------------------
uint Datalink::unpack_float_f2(const void *buf,void *value_ptr)
{
  float f;
  uint16_t h=*((const uint16_t*)buf),hs,he,hm;
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
  *((_mandala_float*)value_ptr)=f;
  return 2;
}
//------------------------------------------------------------------------------
uint Datalink::unpack_float_f4(const void *buf,void *value_ptr)
{
  float f;
  const uint8_t *src=(const uint8_t*)buf;
  uint8_t *dest=(uint8_t*)&f;
  *dest++=*src++;
  *dest++=*src++;
  *dest++=*src++;
  *dest=*src;
  if(isnan(f))f=0;
  *((_mandala_float*)value_ptr)=f;
  return 4;
}
//------------------------------------------------------------------------------
uint Datalink::unpack_bit_(const void *buf,void *value_ptr)
{
  *((_var_flag*)value_ptr)=*((const uint8_t*)buf);
  return sizeof(uint8_t);
}
//------------------------------------------------------------------------------
uint Datalink::unpack_enum_(const void *buf,void *value_ptr)
{
  *((_var_enum*)value_ptr)=*((const uint8_t*)buf);
  return sizeof(uint8_t);
}
//------------------------------------------------------------------------------
uint Datalink::unpack_byte_u1(const void *buf,void *value_ptr)
{
  *((_var_byte*)value_ptr)=*((const uint8_t*)buf);
  return sizeof(uint8_t);
}
//------------------------------------------------------------------------------
uint Datalink::unpack_uint_u4(const void *buf,void *value_ptr)
{
  const uint8_t *ptr=(const uint8_t*)buf;
  _var_uint v=*ptr++;
  v|=((_var_uint)*ptr++)<<8;
  v|=((_var_uint)*ptr++)<<16;
  v|=((_var_uint)*ptr++)<<24;
  *((_var_uint*)value_ptr)=v;
  //*((_var_uint*)value_ptr)=*((uint32_t*)buf);
  return sizeof(uint32_t);
}
//------------------------------------------------------------------------------
uint Datalink::unpack_uint_u2(const void *buf,void *value_ptr)
{
  const uint8_t *ptr=(const uint8_t*)buf;
  _var_uint v=*ptr++;
  v|=((_var_uint)*ptr++)<<8;
  *((_var_uint*)value_ptr)=v;
  //*((_var_uint*)value_ptr)=*((uint16_t*)buf);
  return sizeof(uint16_t);
}
//=============================================================================
