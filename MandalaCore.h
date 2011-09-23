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
//use double for ARM
#ifdef FLOAT_TYPE
typedef float   _var_float;
#else
typedef double  _var_float;
#endif
//=============================================================================
class Vector
{
public:
  Vector();
  Vector(const _var_float &s);
  Vector(const _var_float &s0,const _var_float &s1,const _var_float &s2);
  void fill(const _var_float &value=0);
  _var_float *array();
  const _var_float*array()const;
  _var_float &operator[](unsigned int index);
  const _var_float &operator[](unsigned int index)const;
  Vector & operator=(const _var_float value);
  bool operator==(const Vector &value)const;
  bool operator!=(const Vector &value)const;
  const Vector operator+(const Vector &that)const;
  Vector &operator+=(const Vector &that);
  const Vector operator-(const Vector &that)const;
  Vector &operator-=(const Vector &that);
  const Vector operator*(const _var_float &scale)const;
  Vector &operator*=(const _var_float &scale);
  const Vector operator/(const _var_float &scale)const;
  Vector &operator/=(const _var_float &scale);
protected:
  _var_float v[3];
};
//=============================================================================
typedef Vector _var_vect;
typedef uint    _var_uint;
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
#define VARDEF(atype,aname,aspan,abytes,adescr) idx_##aname,
enum {
  idx_vars_start=idxPAD-1,
  #include "MandalaVars.h"
  idx_vars_top,
  varsCnt=(idx_vars_top-idx_vars_start+1)
};
//-----------------------------------------------------------------------------
//variable parameters
/*#define SIGDEF(aname, adescr, ... ) VARDEFA(sig,aname,VA_NUM_ARGS(__VA_ARGS__),0,1,adescr)
#define VARDEF(atype,aname,aspan,abytes,adescr) VARDEFA(atype,aname,1,aspan,abytes,adescr)
#define VARDEFA(atype,aname,asize,aspan,abytes,adescr) \
  enum{\
      var_type_##aname=vt_##atype, \
      var_bytes_##aname=(aspan<0)?(-abytes):(abytes), \
      var_array_##aname=asize, \
      var_size_##aname=((asize)*(abytes)*((vt_##atype==vt_vect)?3:1)), \
      var_max_##aname=(aspan>0)?(((abytes==1)?0xFF:((abytes==2)?0xFFFF:((abytes==4)?0xFFFFFFFF:0)))): \
                      ( (aspan<0)?((abytes==1)?0x7F:((abytes==2)?0x7FFF:((abytes==4)?0x7FFFFFFF:0))):0 ), \
      var_span100_##aname=(int)(((aspan<0)?(-aspan):(aspan))*100.0),\
      };

#include "MandalaVars.h"
*/
//-----------------------------------------------------------------------------
// variable typedefs
#define VARDEFA(atype,aname,asize,aspan,abytes,adescr) typedef _var_##atype var_typedef_##aname [asize];
#define VARDEF(atype,aname,aspan,abytes,adescr) typedef _var_##atype var_typedef_##aname;
#define SIGDEF(aname, adescr, ... ) typedef _var_signature var_typedef_##aname;
#include "MandalaVars.h"

//bitfield constants
#define BITDEF(avarname,abitname,amask,adescr) enum{ avarname##_##abitname=amask };
#include "MandalaVars.h"
//-----------------------------------------------------------------------------
//=============================================================================
class MandalaCore
{
public:
  MandalaCore();

  //member=mask, if var is a bitfield, or vect idx, or array idx
  _var_float get_value(uint var_idx,uint member_idx);
  void set_value(uint var_idx,uint member_idx,_var_float value);
  //-----------------------------------------------------------------------------
  virtual uint archive(uint8_t *buf,uint var_idx){return do_archive(buf,var_idx);}
  virtual uint extract(uint8_t *buf,uint cnt,uint var_idx){return do_extract(buf,cnt,var_idx);}

  #define VARDEF(atype,aname,aspan,abytes,adescr)         var_typedef_##aname aname;
  #define VARDEFA(atype,aname,asize,aspan,abytes,adescr)  VARDEF( atype,aname,aspan,abytes,adescr )
  #define SIGDEF(aname,adescr,...)                        static var_typedef_##aname aname;
  #include "MandalaVars.h"
  uint do_archive(uint8_t *buf,uint var_idx);
  uint do_extract(uint8_t *buf,uint cnt,uint var_idx);

  struct {
    uint8_t *buf;         //buffer to store/extract
    void    *ptr;         //pointer to local var.VARNAME
    int     sbytes;       //archived bytes cnt (if < 0 => signed)
    _var_float   span;         //variable span (absolute, always >0)
    uint    array;        //count of bytes in array
    uint    type;         //type of variable
    uint32_t max;         //max archived integer value (unsigned)
    uint    size;         //total size of archived data
  }vdsc;
  uint vdsc_fill(uint8_t *buf,uint var_idx);

private:

  uint32_t limit_u(const _var_float v,const uint32_t max);
  uint32_t limit_ui(const uint32_t v,const uint32_t max);
  int32_t limit_s(const _var_float v,const uint32_t max);
  void archive_float(void);
  void archive_uint(void);
  void archive_vect(void);
  void archive_sig();
  void extract_float(void);
  void extract_uint(void);
  void extract_vect(void);
  void extract_sig(uint buf_cnt);
};
//=============================================================================
#endif // MANDALA_CORE_H
