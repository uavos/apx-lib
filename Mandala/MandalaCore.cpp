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
#include <string.h>

#include "MandalaCore.h"
#include "math.h"
#include "preprocessor.h"

//===========================================================================
const MandalaCore::_vars_list MandalaCore::vars_dlink = {MANDALA_LIST_DLINK, 0};
const MandalaCore::_vars_list MandalaCore::vars_ctr = {MANDALA_LIST_CTR, 0};
//===========================================================================
/*uint32_t MandalaCore::hash32()
{
  uint32_t v=varsCnt<<16;
  #define MVAR(atype,aname,adescr, ...) \
  v=hash32_calc(#aname,v); \
  v=hash32_calc(adescr,v); \

  #define MBIT(avarname,abitname,adescr,amask) \
  v=hash32_calc(#abitname,v); \
  v=hash32_calc(adescr,v); \
  v+=idx_##avarname |( avarname##_##abitname <<8);

  #include "MandalaTemplate.h"
  return v;
}
uint32_t MandalaCore::hash32_calc(const char *str,uint32_t v)
{
  uint32_t sz=strlen(str);
  uint16_t hL=CRC_16_IBM((const uint8_t*)str,sz,v);
  uint16_t hH=CRC_SUM((const uint8_t*)str,sz,v>>16);
  return hL|hH<<16;
}*/
//===========================================================================
_var_float MandalaCore::get_data(uint16_t var_m)
{
    uint32_t type;
    void *value_ptr;
    if (!get_ptr(var_m, &value_ptr, &type))
        return 0;
    return get_data(var_m, type, value_ptr);
}
_var_float MandalaCore::get_data(uint16_t var_m, uint32_t type, void *value_ptr)
{
    uint8_t m = var_m >> 8;
    switch (type) {
    case vt_float:
        return *(_var_float *) value_ptr;
    case vt_vect:
        return (*(_var_vect *) value_ptr)[m];
    case vt_point:
        return (*(_var_point *) value_ptr)[m];
    case vt_byte:
        return *(_var_byte *) value_ptr;
    case vt_uint:
        return *(_var_uint *) value_ptr;
    case vt_flag:
        return m ? (((*(_var_flag *) value_ptr) & m) ? 1 : 0) : (*(_var_flag *) value_ptr);
    case vt_enum:
        return (*(_var_enum *) value_ptr);
    }
    return 0;
}
//=============================================================================
void MandalaCore::set_data(uint16_t var_m, _var_float value)
{
    uint32_t type;
    void *value_ptr;
    if (!get_ptr(var_m, &value_ptr, &type))
        return;
    set_data(var_m, type, value_ptr, value);
}
void MandalaCore::set_data(uint16_t var_m, uint32_t type, void *value_ptr, _var_float value)
{
    uint8_t m = var_m >> 8;
    switch (type) {
    case vt_float:
        *(_var_float *) value_ptr = value;
        break;
    case vt_vect:
        (*(_var_vect *) value_ptr)[m] = value;
        break;
    case vt_point:
        (*(_var_point *) value_ptr)[m] = value;
        break;
    case vt_byte:
        *(_var_byte *) value_ptr = value;
        break;
    case vt_uint:
        *(_var_uint *) value_ptr = value;
        break;
    case vt_flag:
        if (m)
            (*(_var_flag *) value_ptr) = (value <= 0) ? ((*(_var_flag *) value_ptr) & (~m))
                                                      : ((*(_var_flag *) value_ptr) | m);
        else
            *(_var_flag *) value_ptr = value;
        break;
    case vt_enum:
        *(_var_enum *) value_ptr = value;
        break;
    }
}
//=============================================================================
bool MandalaCore::get_ptr(uint8_t var_idx, void **var_ptr, uint32_t *type)
{
    switch (var_idx) {
#define MIDX(aname, ...) \
    case idx_##aname: \
        *type = vt_idx; \
        *var_ptr = NULL; \
        return true;
#define MVAR(atype, aname, adescr, ...) \
    case idx_##aname: \
        *type = vt_##atype; \
        *var_ptr = (void *) &(aname); \
        return true;
#include "MandalaTemplate.h"
    }
    return false;
}
//=============================================================================
uint32_t MandalaCore::pack(uint8_t *buf, uint32_t var_idx)
{
    if (var_idx > 255) {
        //pack as idx_set
        return pack_set(buf, var_idx);
    }
    switch (var_idx) {
#define MVAR(atype, aname, adescr, abytes, atbytes) \
    case idx_##aname: \
        return pack_##atype##_##abytes(buf, (void *) &(aname));
#include "MandalaTemplate.h"
    }
    return 0;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_ext(uint8_t *buf, uint32_t var_idx)
{
    switch (var_idx) {
#define MVAR(atype, aname, adescr, abytes, atbytes) \
    case idx_##aname: \
        return pack_##atype##_##atbytes(buf, (void *) &(aname));
#include "MandalaTemplate.h"
    }
    return 0;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack(const uint8_t *buf, uint32_t cnt, uint32_t var_idx)
{
    if (var_idx < idxPAD) {
        if (var_idx == idx_set)
            return unpack_set(buf, cnt);
        return 0;
    }
    switch (var_idx) {
#define MVAR(atype, aname, adescr, abytes, atbytes) \
    case idx_##aname: \
        return unpack_##atype##_##abytes(buf, (void *) &(aname));
#include "MandalaTemplate.h"
    }
    return 0;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_ext(const uint8_t *buf, uint32_t var_idx)
{
    switch (var_idx) {
#define MVAR(atype, aname, adescr, abytes, atbytes) \
    case idx_##aname: \
        return unpack_##atype##_##atbytes(buf, (void *) &(aname));
#include "MandalaTemplate.h"
    }
    return 0;
}
//==============================================================================
uint32_t MandalaCore::pack_float_s(void *buf, const _var_float &v)
{
    *((int8_t *) buf) = (v > 127) ? 127 : ((v < -128) ? -128 : v);
    return sizeof(int8_t);
}
uint32_t MandalaCore::pack_float_u(void *buf, const _var_float &v)
{
    *((uint8_t *) buf) = (v > 255) ? 255 : v;
    return sizeof(uint8_t);
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_float_s1(void *buf, void *value_ptr)
{
    return pack_float_s(buf, *((_var_float *) value_ptr));
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_float_s01(void *buf, void *value_ptr)
{
    return pack_float_s(buf, *((_var_float *) value_ptr) * 10.0);
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_float_s001(void *buf, void *value_ptr)
{
    return pack_float_s(buf, *((_var_float *) value_ptr) * 100.0);
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_float_s10(void *buf, void *value_ptr)
{
    return pack_float_s(buf, *((_var_float *) value_ptr) / 10.0);
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_float_u1(void *buf, void *value_ptr)
{
    return pack_float_u(buf, *((_var_float *) value_ptr));
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_float_u01(void *buf, void *value_ptr)
{
    return pack_float_u(buf, *((_var_float *) value_ptr) * 10.0);
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_float_u001(void *buf, void *value_ptr)
{
    return pack_float_u(buf, *((_var_float *) value_ptr) * 100.0);
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_float_u10(void *buf, void *value_ptr)
{
    return pack_float_u(buf, *((_var_float *) value_ptr) / 10.0);
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_float_u100(void *buf, void *value_ptr)
{
    return pack_float_u(buf, *((_var_float *) value_ptr) / 100.0);
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_float_f2(void *buf, void *value_ptr)
{ //IEEE 754r
    float f = *((_var_float *) value_ptr);
    if (!matrixmath::f_isvalid(*((_var_float *) value_ptr)))
        *((_var_float *) value_ptr) = 0;
    uint8_t *ptr = (uint8_t *) &f;
    uint32_t x = ptr[0] | ptr[1] << 8 | ptr[2] << 16 | ptr[3] << 24, xs, xe, xm;
    uint16_t hs, he, hm;
    uint16_t *hp = (uint16_t *) buf;
    int hes;
    if ((x & 0x7FFFFFFFu) == 0) {    // Signed zero
        *hp++ = (uint16_t)(x >> 16); // Return the signed zero
    } else {                         // Not zero
        xs = x & 0x80000000u;        // Pick off sign bit
        xe = x & 0x7F800000u;        // Pick off exponent bits
        xm = x & 0x007FFFFFu;        // Pick off mantissa bits
        if (xe == 0) {               // Denormal will underflow, return a signed zero
            *hp++ = (uint16_t)(xs >> 16);
        } else if (xe == 0x7F800000u) { // Inf or NaN (all the exponent bits are set)
            if (xm == 0) {              // If mantissa is zero ...
                *hp++ = (uint16_t)((xs >> 16) | 0x7C00u); // Signed Inf
            } else {
                *hp++ = (uint16_t) 0xFE00u; // NaN, only 1st mantissa bit set
            }
        } else {                                 // Normalized number
            hs = (uint16_t)(xs >> 16);           // Sign bit
            hes = ((int) (xe >> 23)) - 127 + 15; // Exponent unbias the single, then bias the halfp
            if (hes >= 0x1F) {                   // Overflow
                *hp++ = (uint16_t)((xs >> 16) | 0x7C00u); // Signed Inf
            } else if (hes <= 0) {                        // Underflow
                if ((14 - hes) > 24) { // Mantissa shifted all the way off & no rounding possibility
                    hm = (uint16_t) 0u; // Set mantissa to zero
                } else {
                    xm |= 0x00800000u;                    // Add the hidden leading bit
                    hm = (uint16_t)(xm >> (14 - hes));    // Mantissa
                    if ((xm >> (13 - hes)) & 0x00000001u) // Check for rounding
                        hm += (uint16_t) 1u; // Round, might overflow into exp bit, but this is OK
                }
                *hp++ = (hs | hm); // Combine sign bit and mantissa bits, biased exponent is zero
            } else {
                he = (uint16_t)(hes << 10); // Exponent
                hm = (uint16_t)(xm >> 13);  // Mantissa
                if (xm & 0x00001000u)       // Check for rounding
                    *hp++ = (hs | he | hm)
                            + (uint16_t) 1u; // Round, might overflow to inf, this is OK
                else
                    *hp++ = (hs | he | hm); // No rounding
            }
        }
    }
    return 2;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_float_f4(void *buf, void *value_ptr)
{
    float f = *(_var_float *) value_ptr;
    if (!matrixmath::f_isvalid(*(_var_float *) value_ptr))
        *(_var_float *) value_ptr = 0;
    uint8_t *src = (uint8_t *) &f;
    uint8_t *dest = (uint8_t *) buf;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest = *src;
    return 4;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_byte_u1(void *buf, void *value_ptr)
{
    *((uint8_t *) buf) = *((_var_byte *) value_ptr);
    return sizeof(uint8_t);
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_flag_(void *buf, void *value_ptr)
{
    *((uint8_t *) buf) = *((_var_flag *) value_ptr);
    return sizeof(uint8_t);
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_enum_(void *buf, void *value_ptr)
{
    *((uint8_t *) buf) = *((_var_enum *) value_ptr);
    return sizeof(uint8_t);
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_uint_u4(void *buf, void *value_ptr)
{
    _var_uint v = *((_var_uint *) value_ptr);
    uint8_t *ptr = (uint8_t *) buf;
    *ptr++ = v;
    *ptr++ = v >> 8;
    *ptr++ = v >> 16;
    *ptr++ = v >> 24;
    //*((uint32_t*)buf)=*((_var_uint*)value_ptr);
    return sizeof(uint32_t);
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_uint_u2(void *buf, void *value_ptr)
{
    _var_uint v = *((_var_uint *) value_ptr);
    if (v > 0x0000FFFF)
        v = 0x0000FFFF;
    uint8_t *ptr = (uint8_t *) buf;
    *ptr++ = v;
    *ptr++ = v >> 8;
    //*((uint16_t*)buf)=?0x0000FFFF:v;
    return sizeof(uint16_t);
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_vect_s1(void *buf, void *value_ptr)
{
    _var_vect *v = ((_var_vect *) value_ptr);
    uint8_t *ptr = (uint8_t *) buf;
    pack_float_s1(ptr++, &((*v)[0]));
    pack_float_s1(ptr++, &((*v)[1]));
    pack_float_s1(ptr, &((*v)[2]));
    return 3;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_vect_s10(void *buf, void *value_ptr)
{
    _var_vect *v = ((_var_vect *) value_ptr);
    uint8_t *ptr = (uint8_t *) buf;
    pack_float_s10(ptr++, &((*v)[0]));
    pack_float_s10(ptr++, &((*v)[1]));
    pack_float_s10(ptr, &((*v)[2]));
    return 3;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_vect_s001(void *buf, void *value_ptr)
{
    _var_vect *v = ((_var_vect *) value_ptr);
    uint8_t *ptr = (uint8_t *) buf;
    pack_float_s001(ptr++, &((*v)[0]));
    pack_float_s001(ptr++, &((*v)[1]));
    pack_float_s001(ptr, &((*v)[2]));
    return 3;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_vect_f2(void *buf, void *value_ptr)
{
    _var_vect *v = ((_var_vect *) value_ptr);
    uint16_t *ptr = (uint16_t *) buf;
    pack_float_f2(ptr++, &((*v)[0]));
    pack_float_f2(ptr++, &((*v)[1]));
    pack_float_f2(ptr, &((*v)[2]));
    return 6;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_vect_f4(void *buf, void *value_ptr)
{
    _var_vect *v = ((_var_vect *) value_ptr);
    uint32_t *ptr = (uint32_t *) buf;
    pack_float_f4(ptr++, &((*v)[0]));
    pack_float_f4(ptr++, &((*v)[1]));
    pack_float_f4(ptr, &((*v)[2]));
    return 12;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_point_f2(void *buf, void *value_ptr)
{
    _var_point *v = ((_var_point *) value_ptr);
    uint16_t *ptr = (uint16_t *) buf;
    pack_float_f2(ptr++, &((*v)[0]));
    pack_float_f2(ptr, &((*v)[1]));
    return 4;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_point_f4(void *buf, void *value_ptr)
{
    _var_point *v = ((_var_point *) value_ptr);
    uint32_t *ptr = (uint32_t *) buf;
    pack_float_f4(ptr++, &((*v)[0]));
    pack_float_f4(ptr, &((*v)[1]));
    return 8;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::pack_point_u001(void *buf, void *value_ptr)
{
    _var_point *v = ((_var_point *) value_ptr);
    uint8_t *ptr = (uint8_t *) buf;
    pack_float_u001(ptr++, &((*v)[0]));
    pack_float_u001(ptr, &((*v)[1]));
    return 2;
}
//=============================================================================
uint32_t MandalaCore::unpack_float_s1(const void *buf, void *value_ptr)
{
    *((_var_float *) value_ptr) = (_var_float)(*((const int8_t *) buf));
    return 1;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_float_s01(const void *buf, void *value_ptr)
{
    *((_var_float *) value_ptr) = (_var_float)(*((const int8_t *) buf)) / 10.0;
    return 1;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_float_s001(const void *buf, void *value_ptr)
{
    *((_var_float *) value_ptr) = (_var_float)(*((const int8_t *) buf)) / 100.0;
    return 1;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_float_s10(const void *buf, void *value_ptr)
{
    *((_var_float *) value_ptr) = (_var_float)((int32_t) * ((const int8_t *) buf) * 10);
    return 1;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_float_u1(const void *buf, void *value_ptr)
{
    *((_var_float *) value_ptr) = (_var_float)(*((const uint8_t *) buf));
    return 1;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_float_u01(const void *buf, void *value_ptr)
{
    *((_var_float *) value_ptr) = (_var_float)(*((const uint8_t *) buf)) / 10.0;
    return 1;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_float_u001(const void *buf, void *value_ptr)
{
    *((_var_float *) value_ptr) = (_var_float)(*((const uint8_t *) buf)) / 100.0;
    return 1;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_float_u10(const void *buf, void *value_ptr)
{
    *((_var_float *) value_ptr) = (_var_float)((uint32_t) * ((const uint8_t *) buf) * 10);
    return 1;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_float_u100(const void *buf, void *value_ptr)
{
    *((_var_float *) value_ptr) = (_var_float)((uint32_t) * ((const uint8_t *) buf) * 100);
    return 1;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_float_f2(const void *buf, void *value_ptr)
{
    float f = 0;
    uint16_t h = *((const uint16_t *) buf), hs, he, hm;
    uint32_t *xp = (uint32_t *) &f; //(uint32_t*)value_ptr;
    uint32_t xs, xe, xm;
    int32_t xes;
    int e;
    if ((h & 0x7FFFu) == 0) {         // Signed zero
        *xp++ = ((uint32_t) h) << 16; // Return the signed zero
    } else {                          // Not zero
        hs = h & 0x8000u;             // Pick off sign bit
        he = h & 0x7C00u;             // Pick off exponent bits
        hm = h & 0x03FFu;             // Pick off mantissa bits
        if (he == 0) {                // Denormal will convert to normalized
            e = -1; // The following loop figures out how much extra to adjust the exponent
            do {
                e++;
                hm <<= 1;
            } while ((hm & 0x0400u) == 0); // Shift until leading bit overflows into exponent bit
            xs = ((uint32_t) hs) << 16;    // Sign bit
            xes = ((int32_t)(he >> 10)) - 15 + 127
                  - e;                  // Exponent unbias the halfp, then bias the single
            xe = (uint32_t)(xes << 23); // Exponent
            xm = ((uint32_t)(hm & 0x03FFu)) << 13; // Mantissa
            *xp++ = (xs | xe | xm); // Combine sign bit, exponent bits, and mantissa bits
        } else if (he == 0x7C00u) { // Inf or NaN (all the exponent bits are set)
            if (hm == 0) {          // If mantissa is zero ...
                *xp++ = (((uint32_t) hs) << 16) | ((uint32_t) 0x7F800000u); // Signed Inf
            } else {
                *xp++ = (uint32_t) 0xFFC00000u; // NaN, only 1st mantissa bit set
            }
        } else {                        // Normalized number
            xs = ((uint32_t) hs) << 16; // Sign bit
            xes = ((int32_t)(he >> 10)) - 15
                  + 127;                // Exponent unbias the halfp, then bias the single
            xe = (uint32_t)(xes << 23); // Exponent
            xm = ((uint32_t) hm) << 13; // Mantissa
            *xp++ = (xs | xe | xm);     // Combine sign bit, exponent bits, and mantissa bits
        }
    }
    if (!matrixmath::f_isvalid(f))
        f = 0;
    *((_var_float *) value_ptr) = f;
    return 2;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_float_f4(const void *buf, void *value_ptr)
{
    float f;
    const uint8_t *src = (const uint8_t *) buf;
    uint8_t *dest = (uint8_t *) &f;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest++ = *src++;
    *dest = *src;
    if (!matrixmath::f_isvalid(f))
        f = 0;
    *((_var_float *) value_ptr) = f;
    return 4;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_flag_(const void *buf, void *value_ptr)
{
    *((_var_flag *) value_ptr) = *((const uint8_t *) buf);
    return sizeof(uint8_t);
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_enum_(const void *buf, void *value_ptr)
{
    *((_var_enum *) value_ptr) = *((const uint8_t *) buf);
    return sizeof(uint8_t);
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_byte_u1(const void *buf, void *value_ptr)
{
    *((_var_byte *) value_ptr) = *((const uint8_t *) buf);
    return sizeof(uint8_t);
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_uint_u4(const void *buf, void *value_ptr)
{
    const uint8_t *ptr = (const uint8_t *) buf;
    _var_uint v = *ptr++;
    v |= ((_var_uint) *ptr++) << 8;
    v |= ((_var_uint) *ptr++) << 16;
    v |= ((_var_uint) *ptr++) << 24;
    *((_var_uint *) value_ptr) = v;
    //*((_var_uint*)value_ptr)=*((uint32_t*)buf);
    return sizeof(uint32_t);
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_uint_u2(const void *buf, void *value_ptr)
{
    const uint8_t *ptr = (const uint8_t *) buf;
    _var_uint v = *ptr++;
    v |= ((_var_uint) *ptr++) << 8;
    *((_var_uint *) value_ptr) = v;
    //*((_var_uint*)value_ptr)=*((uint16_t*)buf);
    return sizeof(uint16_t);
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_vect_s1(const void *buf, void *value_ptr)
{
    _var_vect *v = ((_var_vect *) value_ptr);
    const uint8_t *ptr = (const uint8_t *) buf;
    unpack_float_s1(ptr++, &((*v)[0]));
    unpack_float_s1(ptr++, &((*v)[1]));
    unpack_float_s1(ptr, &((*v)[2]));
    return 3;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_vect_s10(const void *buf, void *value_ptr)
{
    _var_vect *v = ((_var_vect *) value_ptr);
    const uint8_t *ptr = (const uint8_t *) buf;
    unpack_float_s10(ptr++, &((*v)[0]));
    unpack_float_s10(ptr++, &((*v)[1]));
    unpack_float_s10(ptr, &((*v)[2]));
    return 3;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_vect_s001(const void *buf, void *value_ptr)
{
    _var_vect *v = ((_var_vect *) value_ptr);
    const uint8_t *ptr = (const uint8_t *) buf;
    unpack_float_s001(ptr++, &((*v)[0]));
    unpack_float_s001(ptr++, &((*v)[1]));
    unpack_float_s001(ptr, &((*v)[2]));
    return 3;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_vect_f2(const void *buf, void *value_ptr)
{
    _var_vect *v = ((_var_vect *) value_ptr);
    const uint16_t *ptr = (const uint16_t *) buf;
    unpack_float_f2(ptr++, &((*v)[0]));
    unpack_float_f2(ptr++, &((*v)[1]));
    unpack_float_f2(ptr, &((*v)[2]));
    return 6;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_vect_f4(const void *buf, void *value_ptr)
{
    _var_vect *v = ((_var_vect *) value_ptr);
    const uint32_t *ptr = (const uint32_t *) buf;
    unpack_float_f4(ptr++, &((*v)[0]));
    unpack_float_f4(ptr++, &((*v)[1]));
    unpack_float_f4(ptr, &((*v)[2]));
    return 12;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_point_f2(const void *buf, void *value_ptr)
{
    _var_point *v = ((_var_point *) value_ptr);
    const uint16_t *ptr = (const uint16_t *) buf;
    unpack_float_f2(ptr++, &((*v)[0]));
    unpack_float_f2(ptr, &((*v)[1]));
    return 4;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_point_f4(const void *buf, void *value_ptr)
{
    _var_point *v = ((_var_point *) value_ptr);
    const uint32_t *ptr = (const uint32_t *) buf;
    unpack_float_f4(ptr++, &((*v)[0]));
    unpack_float_f4(ptr, &((*v)[1]));
    return 8;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_point_u001(const void *buf, void *value_ptr)
{
    _var_point *v = ((_var_point *) value_ptr);
    const uint8_t *ptr = (const uint8_t *) buf;
    unpack_float_u001(ptr++, &((*v)[0]));
    unpack_float_u001(ptr, &((*v)[1]));
    return 2;
}
//------------------------------------------------------------------------------
uint32_t MandalaCore::unpack_stream(const uint8_t *buf, uint32_t cnt, bool hd)
{
    uint8_t mask = 1;
    const uint8_t *mask_ptr = buf;
    const uint8_t *ptr = mask_ptr + 1;
    int tcnt = cnt - 1;
    uint32_t idx = idxPAD;
    do {
        while (mask) {
            if ((*mask_ptr) & mask) {
                if (tcnt <= 0)
                    break;
                uint32_t sz = hd ? unpack(ptr, tcnt, idx) : unpack_ext(ptr, idx);
                if (!sz)
                    return 0;
                ptr += sz;
                tcnt -= sz;
            }
            idx++;
            mask <<= 1;
        }
        mask_ptr = ptr;
        mask = 1;
        ptr++;
        tcnt--;
    } while (tcnt > 0);
    return cnt;
}
//-----------------------------------------------------------------------------
uint32_t MandalaCore::pack_set(uint8_t *buf, uint16_t var_m)
{
    *buf++ = var_m;
    *buf++ = var_m >> 8;
    _var_float v = get_data(var_m);
    return pack_float_f4(buf, &v) + 2;
}
uint32_t MandalaCore::unpack_set(const uint8_t *buf, uint32_t cnt)
{
    if (cnt != 2 + 4)
        return 0;
    _var_float v;
    unpack_float_f4(buf + 2, &v);
    set_data((uint16_t) buf[0] | (uint16_t) buf[1] << 8, v);
    return cnt;
}
//=============================================================================
//=============================================================================
void MandalaCore::filter(const _var_float &fv,
                         _var_float *var_p,
                         const _var_float &fS,
                         const _var_float &fL)
{
    _var_float fD = fv - *var_p;
    _var_float fDL = fD / fL;
    _var_float fG = fS + fS * (fDL * fDL);
    if (fG > 1.0)
        fG = 1.0;
    else if (fG < fS)
        fG = fS;
    *var_p += fD * fG;
    if (f_isvalid(*var_p))
        return;
    *var_p = 0;
}
void MandalaCore::filter(const _var_vect &v,
                         _var_vect *var_p,
                         const _var_float &S,
                         const _var_float &L)
{
    for (uint32_t i = 0; i < 3; i++)
        filter(v[i], &((*var_p)[i]), S, L);
}
void MandalaCore::filter_m(const _var_float &v, _var_float *var_p, const _var_float &f)
{
    if (f <= 0 || f >= 1)
        *var_p = v;
    else
        *var_p = (*var_p) * (1.0 - f) + v * f;
    if (f_isvalid(*var_p))
        return;
    *var_p = 0;
}
void MandalaCore::filter_m(const _var_vect &v, _var_vect *var_p, const _var_float &f)
{
    for (uint32_t i = 0; i < 3; i++)
        filter_m(v[i], &((*var_p)[i]), f);
}
//===========================================================================
//=============================================================================
_var_float MandalaCore::inHgToAltitude(_var_float inHg, _var_float inHg_gnd)
{
    if (inHg_gnd == 0)
        return 0;
    return (1.0 - powf(inHg / inHg_gnd, 0.1902632)) * 44330.77;
}
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
#if defined(USE_FLOAT_TYPE) && !defined(MANDALA_FULL)
// Simplified Vector math
Vect::Vect()
{
    this->fill();
}
Vect::Vect(const _var_float &s)
{
    this->fill(s);
}
Vect::Vect(const _var_float &s0, const _var_float &s1, const _var_float &s2)
{
    (*this)[0] = s0;
    (*this)[1] = s1;
    (*this)[2] = s2;
}
void Vect::fill(const _var_float &value)
{
    for (uint32_t i = 0; i < 3; i++)
        this->v[i] = value;
}
_var_float *Vect::array()
{
    return this->v;
}
const _var_float *Vect::array() const
{
    return this->v;
}
_var_float &Vect::operator[](unsigned int index)
{
    return this->v[index];
}
const _var_float &Vect::operator[](unsigned int index) const
{
    return this->v[index];
}
Vect &Vect::operator=(const _var_float value)
{
    for (uint32_t i = 0; i < 3; i++)
        (*this)[i] = value;
    return (*this);
}
bool Vect::operator==(const Vect &value) const
{
    for (uint32_t i = 0; i < 3; i++)
        if ((*this)[i] != value[i])
            return false;
    return true;
}
bool Vect::operator!=(const Vect &value) const
{
    for (uint32_t i = 0; i < 3; i++)
        if ((*this)[i] == value[i])
            return false;
    return true;
}
const Vect Vect::operator+(const Vect &that) const
{
    return Vect(*this) += that;
}
Vect &Vect::operator+=(const Vect &that)
{
    for (uint32_t i = 0; i < 3; i++)
        (*this)[i] += that[i];
    return (*this);
}
const Vect Vect::operator-(const Vect &that) const
{
    return Vect(*this) -= that;
}
Vect &Vect::operator-=(const Vect &that)
{
    for (uint32_t i = 0; i < 3; i++)
        (*this)[i] -= that[i];
    return (*this);
}
const Vect Vect::operator*(const _var_float &scale) const
{
    return Vect(*this) *= scale;
}
Vect &Vect::operator*=(const _var_float &scale)
{
    for (uint32_t i = 0; i < 3; i++)
        (*this)[i] *= scale;
    return (*this);
}
const Vect Vect::operator/(const _var_float &scale) const
{
    return Vect(*this) /= scale;
}
Vect &Vect::operator/=(const _var_float &scale)
{
    for (uint32_t i = 0; i < 3; i++)
        (*this)[i] /= scale;
    return (*this);
}
#endif
//=============================================================================
//=============================================================================
//=============================================================================
