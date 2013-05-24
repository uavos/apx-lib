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
#ifndef MANDALA_CORE_H
#define MANDALA_CORE_H
//=============================================================================
//=============================================================================
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>
//=============================================================================
#ifndef assert
#define assert(...)
#endif
//=============================================================================
#ifdef USE_FLOAT_TYPE
typedef float   _var_float;
#else
typedef double  _var_float;
#endif

#if !defined(USE_FLOAT_TYPE) || defined(USE_MATRIX_MATH)
#include "MatrixMath.h"
using namespace matrixmath;
#else
// Simplified Vector math
typedef float   _var_float;
class Vect
{
public:
  Vect();
  Vect(const _var_float &s);
  Vect(const _var_float &s0,const _var_float &s1,const _var_float &s2);
  void fill(const _var_float &value=0);
  _var_float *array();
  const _var_float*array()const;
  _var_float &operator[](unsigned int index);
  const _var_float &operator[](unsigned int index)const;
  Vect & operator=(const _var_float value);
  bool operator==(const Vect &value)const;
  bool operator!=(const Vect &value)const;
  const Vect operator+(const Vect &that)const;
  Vect &operator+=(const Vect &that);
  const Vect operator-(const Vect &that)const;
  Vect &operator-=(const Vect &that);
  const Vect operator*(const _var_float &scale)const;
  Vect &operator*=(const _var_float &scale);
  const Vect operator/(const _var_float &scale)const;
  Vect &operator/=(const _var_float &scale);
protected:
  _var_float v[3];
};
#endif
//=============================================================================
//=============================================================================
typedef Vect    _var_vect;
typedef uint    _var_uint;
typedef uint8_t _var_bits;
typedef const uint8_t*  _var_signature;
//=============================================================================
#include "MandalaVars.h" //get constants
//-----------------------------------------------------------------------------
// enum indexes idx_VARNAME
#define SIGDEF(aname, ... ) idx_##aname,
enum {
  #include "MandalaVars.h"
  sigCnt
};
#define VARDEF(atype,aname,aspan,abytes,atbytes,adescr) idx_##aname,
enum {
  idx_vars_start=idxPAD-1,
  #include "MandalaVars.h"
  idx_vars_top,
  varsCnt=(idx_vars_top-idx_vars_start+1)
};
//-----------------------------------------------------------------------------
// variable typedefs
#define VARDEF(atype,aname,aspan,abytes,atbytes,adescr) typedef _var_##atype var_typedef_##aname;
#define SIGDEF(aname, adescr, ... ) typedef _var_signature var_typedef_##aname;
#include "MandalaVars.h"

//bitfield constants
#define BITDEF(avarname,abitname,amask,adescr) enum{ avarname##_##abitname=amask };
#include "MandalaVars.h"
//=============================================================================
class MandalaCore
{
public:
  MandalaCore();
  uint pack(uint8_t *buf,uint var_idx);
  virtual uint unpack(uint8_t *buf,uint cnt,uint var_idx);
  uint unpack_stream(uint8_t *buf,uint cnt);
  bool get_ptr(uint var_idx,void **value_ptr,uint *type);

  static void filter(const _var_float &fv,_var_float *var_p,const _var_float &fS=0.05/100.0,const _var_float &fL=0.9/100.0);
  static void filter(const _var_vect &v,_var_vect *var_p,const _var_float &S=0.05/100.0,const _var_float &L=0.9/100.0);
  static void filter_m(const _var_float &v,_var_float *var_p,const _var_float &f);
  static void filter_m(const _var_vect &v,_var_vect *var_p,const _var_float &f);
  //member=mask, if var is a bitfield, or vect idx
  _var_float get_value(uint var_m);
  _var_float get_value(uint var_idx,uint member_idx);
  void set_value(uint var_m,_var_float value);
  void set_value(uint var_idx,uint member_idx,_var_float value);
  //-----------------------------------------------------------------------------
  #define VARDEF(atype,aname,aspan,abytes,atbytes,adescr)         var_typedef_##aname aname;
  #define SIGDEF(aname,adescr,...)                        static var_typedef_##aname aname;
  #include "MandalaVars.h"

  // flag to use alternate bytes field for packing vars
  // used for telemetry (smaller size, less precision)
  bool alt_bytecnt;

  //math calculations
  _var_float inHgToAltitude(_var_float inHg,_var_float inHg_gnd);
  _var_float conv_pstatic_altitude(void); //convert and filter, return unfiltered

  //optimizations
  static inline void memcpy(void *dest,const void *src,uint cnt)
  {
    if(cnt==1)*((uint8_t*)dest)=*((const uint8_t*)src);
    else if(cnt>1)::memcpy(dest,src,cnt);
  }
  static inline int memcmp(const void *s1,const void *s2,uint cnt)
  {
    if(cnt==1)return *((const uint8_t*)s1)!=*((const uint8_t*)s2);
    else if(cnt>1)return ::memcmp(s1,s2,cnt);
    return -1;
  }
protected:
  //pack
  uint pack_float_1(void *buf,void *value_ptr,_var_float span);
  uint pack_float_2(void *buf,void *value_ptr,_var_float span);
  uint pack_float_4(void *buf,void *value_ptr,_var_float span);
  uint pack_bits_1(void *buf,void *value_ptr,_var_float span);
  uint pack_uint_1(void *buf,void *value_ptr,_var_float span);
  uint pack_uint_2(void *buf,void *value_ptr,_var_float span);
  uint pack_uint_4(void *buf,void *value_ptr,_var_float span);
  uint pack_vect_1(void *buf,void *value_ptr,_var_float span);
  uint pack_vect_2(void *buf,void *value_ptr,_var_float span);
  uint pack_vect_4(void *buf,void *value_ptr,_var_float span);
  uint pack_sig(void *buf,void *value_ptr);
  //unpack
  uint unpack_float_1(void *buf,void *value_ptr,_var_float span);
  uint unpack_float_2(void *buf,void *value_ptr,_var_float span);
  uint unpack_float_4(void *buf,void *value_ptr,_var_float span);
  uint unpack_bits_1(void *buf,void *value_ptr,_var_float span);
  uint unpack_uint_1(void *buf,void *value_ptr,_var_float span);
  uint unpack_uint_2(void *buf,void *value_ptr,_var_float span);
  uint unpack_uint_4(void *buf,void *value_ptr,_var_float span);
  uint unpack_vect_1(void *buf,void *value_ptr,_var_float span);
  uint unpack_vect_2(void *buf,void *value_ptr,_var_float span);
  uint unpack_vect_4(void *buf,void *value_ptr,_var_float span);
  uint unpack_sig(void *buf,uint cnt,void *value_ptr);

};
//=============================================================================
#endif // MANDALA_CORE_H
