/*
 * Copyright (C) 2015 Aliaksei Stratsilatau <sa@uavos.com>
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
#ifndef MANDALA_FIELDS
#define MANDALA_FIELDS
#include <inttypes.h>
#include <sys/types.h>
#include <string.h>
#include "preprocessor.h"
#include <dmsg.h>
//=============================================================================
// peferences
//#define MANDALA_MAX_MSG 1024
//#define MANDALA_FULL
//#undef MANDALA_FULL
#define MANDALA_PACKED_STRUCT   __attribute__((packed))
#define MANDALA_TEXT
//#define MANDALA_FLAGS
//#define MANDALA_DOUBLE
#define MANDALA_ITERATORS

#ifndef USE_FLOAT_TYPE
#define MANDALA_DOUBLE
#endif
//=============================================================================

/*
############################
# HEADER
############################
32bit (29bit CAN)
------------------ < CAN EID START
## 7 bits source node address (127 nodes, 0=reserved=<no addr yet>):
0-6:    can_adr
------------------
## 8 bits specifier for dictionary SDO/PDO:
7:      dtype   (8 data types) [float,uint,int,vect,vec2,...]
8:      dtype
9:      dtype
10:     toggle  (toggle bit each frame to detect errors)
11:     cmd     (4 commands) [data,...]
12:     cmd
13:     prio    (4 redundancies) [0=highest priority]
14:     prio
------------------
## 14 bits dictionary:
15:     field   (32 fields)     [field name]
16:     field
17:     field
18:     field
19:     field
20:     group1  (32 groups)     [subsytem name]
21:     group1
22:     group1
23:     group1
24:     group1
25:     group0  (4 groups)      [sensor,control,state,info]
26:     group0
27:     object  (4 types)       [vehicle,payload,aux,stream=3]
28:     object
------------------ < CAN EID END
## 3 bits node local stats (NO CAN TX):
29:     iface   (8 ifaces)      [registered iface index to trace loops]
30:     iface
31:     iface
############################
# STREAM HEADER
############################
32bit (29bit CAN)
------------------ < CAN EID START
0-6:    can_adr
7:      toggle  (toggle bit each frame to detect errors)
------------------
## 8 bits specifier
8-15:   fcnt    (256 frames counter = 2048 bytes)
------------------
## 11 bits command & flags (dictionary space):
16:     cmd     (256 commands/indexes)
17:     cmd
18:     cmd
19:     cmd
20:     cmd
21:     cmd
22:     cmd
23:     cmd
24:     more?           [1=more to come,0=end of frame mark]
25:     expedited?      [1=yes,0=multiframe]
26:     request?        [1=request,0=response]
------------------
## 2 bits object=stream=3 mark
27:     object=1
28:     object=1
------------------ < CAN EID END
## 3 bits node local stats (NO CAN TX):
29:     iface   (8 ifaces)      [registered index to trace loops]
30:     iface
31:     iface
*/
//=============================================================================
//#if !defined(USE_FLOAT_TYPE) || defined(MANDALA_FULL)
#include "MatrixMath.h"
using namespace matrixmath;
/*#else
// Simplified Vector math
typedef float   _mandala_float;
class Vect
{
public:
  Vect();
  Vect(const _mandala_float &s);
  Vect(const _mandala_float &s0,const _mandala_float &s1,const _mandala_float &s2);
  void fill(const _mandala_float &value=0);
  _mandala_float *array();
  const _mandala_float*array()const;
  _mandala_float &operator[](unsigned int index);
  const _mandala_float &operator[](unsigned int index)const;
  Vect & operator=(const _mandala_float value);
  bool operator==(const Vect &value)const;
  bool operator!=(const Vect &value)const;
  const Vect operator+(const Vect &that)const;
  Vect &operator+=(const Vect &that);
  const Vect operator-(const Vect &that)const;
  Vect &operator-=(const Vect &that);
  const Vect operator*(const _mandala_float &scale)const;
  Vect &operator*=(const _mandala_float &scale);
  const Vect operator/(const _mandala_float &scale)const;
  Vect &operator/=(const _mandala_float &scale);
protected:
  _mandala_float v[3];
};
typedef _mandala_float Point[2];
#endif*/
//=============================================================================
//=============================================================================
struct _mandala_test
{
  _mandala_test & operator =(const float & a)
  {
    m_value = a;
    return *this;
  }
  operator float() const {return m_value;}

  const char *name(){return "test struct";}

  float m_value;
};
class _mandala_test_class_base
{
protected:
  virtual const char *name()=0;
};
class _mandala_test_class : virtual public _mandala_test_class_base
{
public:
  _mandala_test_class & operator = (const float & a)
  {
    m_value = a;
    return *this;
  }
  operator float() const {return m_value;}
  const char *name(){return "test class";}
  float m_value;
  //uint8_t buf[5];
  uint32_t ptr;
}__attribute__((packed));
//=============================================================================
//-----------------------------------------------------------------------------
// basic types
//-----------------------------------------------------------------------------
#ifdef MANDALA_DOUBLE
typedef double  _mandala_float;
#else
typedef float   _mandala_float;
#endif
typedef uint16_t   _mandala_index;
typedef uint32_t   _mandala_uint;
typedef uint8_t    _mandala_byte;
typedef uint8_t    _mandala_bit;
typedef uint8_t    _mandala_enum;
//=============================================================================
class _mandala
{
public:
//-----------------------------------------------------------------------------
// indexes _mandala::index_vehicle_sensor_imu_gyro_x
//-----------------------------------------------------------------------------
#define GRP0_MASK       ((1<<12)-1)     //11-9= 2bit (subsystems)
#define GRP1_MASK       ((1<<10)-1)     //9-4=  5bit (32 groups)
#define GRP2_MASK       ((1<<5)-1)      //      5bit (32 fields)
#define MGRP0_BEGIN(...)        ATPASTE2(index_,MGRP0),ATPASTE3(index_,MGRP0,_next)=(ATPASTE2(index_,MGRP0) &~GRP0_MASK)-1,
#define MGRP0_END               ATPASTE3(index_,MGRP0,_end)=(ATPASTE2(index_,MGRP0) &~GRP0_MASK)+(GRP0_MASK+1),
#define MGRP1_BEGIN(...)        ATPASTE4(index_,MGRP0,_,MGRP1),ATPASTE5(index_,MGRP0,_,MGRP1,_next)=(ATPASTE4(index_,MGRP0,_,MGRP1) &~GRP1_MASK)-1,
#define MGRP1_END               ATPASTE5(index_,MGRP0,_,MGRP1,_end)=(ATPASTE4(index_,MGRP0,_,MGRP1) &~GRP1_MASK)+(GRP1_MASK+1),
#define MGRP2_BEGIN(...)        ATPASTE6(index_,MGRP0,_,MGRP1,_,MGRP2),ATPASTE7(index_,MGRP0,_,MGRP1,_,MGRP2,_next)=(ATPASTE6(index_,MGRP0,_,MGRP1,_,MGRP2) &~GRP2_MASK)-1,
#define MGRP2_END               ATPASTE7(index_,MGRP0,_,MGRP1,_,MGRP2,_end)=(ATPASTE6(index_,MGRP0,_,MGRP1,_,MGRP2) &~GRP2_MASK)+(GRP2_MASK+1),

#define MFIELD_INDEX(aname) ATPASTE8(index_,MGRP0,_,MGRP1,_,MGRP2,_,aname)
#define MFIELD_INDEX_VEC(aname,vname) MFIELD_INDEX(ATPASTE3(aname,_,vname))
#define MFIELD(atype,aname,...) MFIELD_INDEX(aname),
#define MFVECT_IMPL(atype,aname,vname,...) MFIELD_INDEX_VEC(aname,vname),
enum{
#include "MandalaFields.h"
};

//enums
#define MFENUM(aname,ename,descr,value) ATPASTE3(aname,_,ename)=value,
enum{
  #include "MandalaFields.h"
};

//-----------------------------------------------------------------------------
// messages
//-----------------------------------------------------------------------------
//universal message packet after unpacking from CAN or UART
  typedef struct {
    uint8_t size;               //size of data bytes
    _mandala_index index;       //dict ID
    //uint8_t  iface  :3; //interface index
    typedef enum{
      dt_float=0,
      dt_uint,
      dt_int,
      dt_vect,
      dt_vec2,
    }_dtype; //8 types total
    typedef enum{
      vs_180=0, //attitude,acc
      vs_1,     //controls
      vs_10,    //mag
      vs_5000,  //gyro
    }_vscale; //16 types total
    union {
      uint8_t specifier;        //general byte mask (8bit)
      struct {                  //dict data
        _dtype   dtype  :3;
        uint8_t  toggle :1;
        uint8_t  cmd    :2;
        uint8_t  prio   :2;
      }MANDALA_PACKED_STRUCT;
    }MANDALA_PACKED_STRUCT;
    union{                      //payload data
      uint8_t  data[8];
      struct{
        uint32_t d20_1    :20;
        uint32_t d20_2    :20;
        uint32_t d20_3    :20;
        _vscale scale     :4;
      }MANDALA_PACKED_STRUCT vect;
    }MANDALA_PACKED_STRUCT payload;
  }MANDALA_PACKED_STRUCT _message;

  #define MANDALA_MSGHDR_SIZE     (sizeof(_mandala::_message)-sizeof(_mandala::_message::payload))

  static _mandala_float vscale2float(const uint8_t vscale)
  {
    switch(vscale){
      case _message::vs_180:    return 180.0;
      case _message::vs_1:      return 1.0;
      case _message::vs_10:     return 10.0;
      case _message::vs_5000:   return 5000.0;
      default: return 1;
    }
  }

//-----------------------------------------------------------------------------
// interfaces
//-----------------------------------------------------------------------------
  class _field;
  class _group;

  //configurable base classes
  class _field_base
  {
  public:
    #ifdef MANDALA_TEXT
    virtual const char* name()const =0;
    virtual const char* descr()const =0;
    virtual const char* shortname()const =0;
    #endif
    #ifdef MANDALA_ITERATORS
    typedef enum{
      mf_float,
      mf_uint,
      mf_byte,
      mf_bit,
      mf_enum,
    }_mf_type;
    typedef enum{
      fmt_float_f4,
      fmt_float_f2,
      fmt_float_u1,
      fmt_float_u01,
      fmt_float_u001,
      fmt_float_u10,
      fmt_float_u100,
      fmt_float_s1,
      fmt_float_s01,
      fmt_float_s001,
      fmt_float_s10,
      fmt_uint_u4,
      fmt_uint_u2,
      fmt_byte_,
      fmt_enum_,
      fmt_bit_,
    }_ext_fmt;
    virtual _mf_type type(void) const =0;
    virtual _ext_fmt ext_fmt(void) =0;
    #endif
  };
  class _group_base
  {
  public:
    #ifdef MANDALA_TEXT
    virtual const char* name()const =0;
    virtual const char* descr()const =0;
    #endif
    #ifdef MANDALA_ITERATORS
    //iterators
    virtual _group * next_group(const _group *g) =0;
    virtual _field * next_field(_field *f) =0;
    #endif
  };


  //implementation
  class _field : public _field_base
  {
  public:
    virtual _mandala_index index()const =0;
    //value
    virtual _mandala_float toFloat() const =0;
    virtual void fromFloat(const _mandala_float &v) =0;
    virtual _mandala_uint toUInt() const =0;
    virtual void fromUInt(const _mandala_uint &v) =0;
    //messages
    virtual uint pack(_message *data) const =0;
    virtual uint unpack(const _message *data) =0;
  };

  class _group : public _group_base
  {
  public:
    virtual _mandala_index index()const =0;
    virtual _group * group(const _mandala_index idx) =0;
    virtual _field * field(const _mandala_index idx) =0;
    virtual uint unpack(const _message *data) =0;
  };

  class _field_vect
  {
  public:
    uint pack(_message *data,const _field *f1,const _field *f2,const _field *f3,_message::_vscale vscale) const
    { //pack vector bundle
      data->index=f1->index();
      data->size=8;
      data->dtype=_mandala::_message::dt_vect;
      data->payload.vect.scale=vscale;
      int32_t vmult=((1<<19)-1);
      float vdiv=vscale2float(data->payload.vect.scale);
      float v=f1->toFloat();
      if(v>vdiv)v=vdiv;
      else if(v<-vdiv)v=-vdiv;
      data->payload.vect.d20_1=(v/vdiv)*vmult;
      v=f2->toFloat();
      if(v>vdiv)v=vdiv;
      else if(v<-vdiv)v=-vdiv;
      data->payload.vect.d20_2=(v/vdiv)*vmult;
      v=f3->toFloat();
      if(v>vdiv)v=vdiv;
      else if(v<-vdiv)v=-vdiv;
      data->payload.vect.d20_3=(v/vdiv)*vmult;
      return data->size;
    }
    uint pack(_message *data,const _field *f1,const _field *f2) const
    { //pack two floats
      data->index=f1->index();
      data->size=8;
      data->dtype=_mandala::_message::dt_vec2;
      float v=f1->toFloat();
      const uint8_t *src=(const uint8_t *)&v;
      uint8_t *dest=(uint8_t *)data->payload.data;
      *dest++=*src++;
      *dest++=*src++;
      *dest++=*src++;
      *dest++=*src++;
      v=f2->toFloat();
      src=(const uint8_t *)&v;
      *dest++=*src++;
      *dest++=*src++;
      *dest++=*src++;
      *dest++=*src++;
      return data->size;
    }
  };


//field template by value type
  template <class T>
  class _field_t : public _field
  {
  public:
#ifdef MANDALA_FLAGS
    _field_t():_field(),m_value(0){flags.all=0;}
    struct{
      union{
        uint8_t all;
        struct{
          uint8_t changed       :1;
          uint8_t used          :1;
          uint8_t unpacked      :1;
          //unpacking
          uint8_t toggle        :1;
          uint8_t prio          :2;
        };
      };
    }MANDALA_PACKED_STRUCT flags;
    void unchange(void) {flags.changed=0;}
#else
    _field_t():_field(),m_value(0){}
#endif
    T & operator=(const T &v){setValue(v);return m_value;}
    operator T() const {return m_value;}

    const T & value() const {return m_value;}

    //interface
    _mandala_float toFloat() const {return value();}
    void fromFloat(const _mandala_float &v){setValue(v);}
    _mandala_uint toUInt() const {return value();}
    void fromUInt(const _mandala_uint &v){setValue(v);}

    uint unpack(const _message *data)
    {
      const uint8_t *src=(const uint8_t *)data->payload.data;
      const uint8_t sz=data->size;
      switch(data->dtype){
        case _message::dt_float:{
          if(sz!=4)return 0;
          float v;
          uint8_t *dest=(uint8_t *)&v;
          *dest++=*src++;
          *dest++=*src++;
          *dest++=*src++;
          *dest++=*src++;
          setValue(v);
        }break;
        case _message::dt_uint:{
          if(sz==1){
            setValue(*src);
          }else if(sz==2){
            uint16_t v;
            uint8_t *dest=(uint8_t *)&v;
            *dest++=*src++;
            *dest++=*src++;
            setValue(v);
          }else if(sz==4){
            uint32_t v;
            uint8_t *dest=(uint8_t *)&v;
            *dest++=*src++;
            *dest++=*src++;
            *dest++=*src++;
            *dest++=*src++;
            setValue(v);
          }else return 0;
        }break;
        case _message::dt_int:{
          if(sz==1){
            setValue((int8_t)*src);
          }else if(sz==2){
            int16_t v;
            uint8_t *dest=(uint8_t *)&v;
            *dest++=*src++;
            *dest++=*src++;
            setValue(v);
          }else if(sz==2){
            int32_t v;
            uint8_t *dest=(uint8_t *)&v;
            *dest++=*src++;
            *dest++=*src++;
            *dest++=*src++;
            *dest++=*src++;
            setValue(v);
          }else return 0;
        }break;
        case _message::dt_vect:{
          if(sz!=8)return 0;
          uint subidx=index()-data->index;
          if(subidx>2)return 0;
          _mandala_float vmult=_mandala::vscale2float(data->payload.vect.scale);
          vmult/=((1<<19)-1);
          int32_t v;
          switch(subidx){
            case 0: v=data->payload.vect.d20_1;break;
            case 1: v=data->payload.vect.d20_2;break;
            case 2: v=data->payload.vect.d20_3;break;
            default: v=0;
          }
          if(v&(1<<19))v|=~((1<<20)-1);
          setValue(v*vmult);
        }break;
        case _message::dt_vec2:{
          if(sz!=8)return 0;
          uint subidx=index()-data->index;
          if(subidx>1)return 0;
          const uint8_t *src=(const uint8_t *)data->payload.data;
          if(subidx==1)src+=4;
          float v;
          uint8_t *dest=(uint8_t *)&v;
          *dest++=*src++;
          *dest++=*src++;
          *dest++=*src++;
          *dest++=*src++;
          setValue(v);
          dmsg("unpack: %s\n",name());
        }break;
        default:
          return 0;
      }
#ifdef MANDALA_FLAGS
      flags.unpacked=1;
#endif
      return sz;
    }

  protected:
    T m_value;
    void setValue(const T &v)
    {
#ifdef MANDALA_FLAGS
      flags.used=1;
      if(m_value==v)return;
      flags.changed=1;
#endif
      m_value=v;
    }
    uint pack_float(_message *data) const
    {
      data->index=index();
      data->dtype=_message::dt_float;
      data->size=4;
      const uint8_t *src=(const uint8_t *)&m_value;
      uint8_t *dest=(uint8_t *)data->payload.data;
      *dest++=*src++;
      *dest++=*src++;
      *dest++=*src++;
      *dest++=*src++;
      return data->size;
    }
    uint pack_uint(_message *data) const
    {
      data->index=index();
      data->dtype=_message::dt_uint;
      data->size=4;
      const uint8_t *src=(const uint8_t *)&m_value;
      uint8_t *dest=(uint8_t *)data->payload.data;
      *dest++=*src++;
      *dest++=*src++;
      *dest++=*src++;
      *dest++=*src++;
      return data->size;
    }
    uint pack_byte(_message *data) const
    {
      data->index=index();
      data->dtype=_message::dt_uint;
      data->size=1;
      data->payload.data[0]=m_value;
      return data->size;
    }
    uint pack_bit(_message *data) const
    {
      data->index=index();
      data->dtype=_message::dt_uint;
      data->size=1;
      data->payload.data[0]=m_value>0?1:0;
      return data->size;
    }

  };

  //field value types
  class _field_float_t : public _field_t<_mandala_float>
  {
  public:
    uint pack(_message *data) const
    {return _field_t<_mandala_float>::pack_float(data);}
  };
  class _field_uint_t : public _field_t<_mandala_uint>
  {
  public:
    uint pack(_message *data) const
    {return _field_t<_mandala_uint>::pack_uint(data);}
  };
  class _field_byte_t : public _field_t<_mandala_byte>
  {
  public:
    uint pack(_message *data) const
    {return _field_t<_mandala_byte>::pack_byte(data);}
  };
  class _field_bit_t : public _field_t<_mandala_bit>
  {
  public:
    uint pack(_message *data) const
    {return _field_t<_mandala_bit>::pack_bit(data);}
  };
  class _field_enum_t : public _field_t<_mandala_enum>
  {
  public:
    uint pack(_message *data) const
    {return _field_t<_mandala_enum>::pack_byte(data);}
  };

  //generic unnamed field type (for small MCUs)
  class _value : public _field_float_t
  {
  public:
    _mandala_float & operator=(const _mandala_float &v){setValue(v);return m_value;}
    _mandala_index index()const{return 0xFFFF;}
    #ifdef MANDALA_TEXT
    const char* name()const{return "";}
    const char* descr()const{return "";}
    const char* shortname()const{return "";}
    #endif
    #ifdef MANDALA_ITERATORS
    _mf_type type(void)const{return mf_float;}
    _ext_fmt ext_fmt(void) {return fmt_float_f4;}
    #endif
  };


//fields typedefs
#define MFIELD_VAR(aname) MGRP0.MGRP1.MGRP2.aname
#define MFIELD_TYPE(aname) ATPASTE2(_,aname)

#define MFIELD(atype,aname,adescr,afmt,...) \
  MFIELD_IMPL(MFIELD_INDEX(aname),atype,aname,adescr,ASTRINGZ(MFIELD_VAR(aname)),aname,afmt) aname;
#define MFIELD_IMPL(aindex,atype,aname,adescr,afullname,ashortname,afmt) \
  class MFIELD_TYPE(aname) : public _field_##atype##_t { public: \
    _mandala_index index()const{return aindex;} \
    const char* name()const{return afullname;} \
    const char* descr()const{return adescr;} \
    const char* shortname()const{return ASTRINGZ(ashortname);} \
    _mf_type type(void)const{return mf_##atype;} \
    _ext_fmt ext_fmt(void) {return fmt_##atype##_##afmt;} \
    _mandala_##atype & operator=(const _mandala_##atype &v){setValue(v);return m_value;} \
    enum {idx=aindex}; \
  }
#define MFVECT(atype,aname,v1,v2,v3,adescr,afmt,...) \
  class MFIELD_TYPE(aname) : public _field_vect { public: \
    operator Vector<3,atype>()const{return Vector<3,atype>(v1,v2,v3);} \
    MFIELD_TYPE(aname) & operator=(const Vector<3,atype>& v){v1=v[0];v2=v[1];v3=v[2];return *this;} \
    MFIELD_IMPL(MFIELD_INDEX_VEC(aname,v1),atype,v1,adescr,ASTRINGZ(MFIELD_VAR(aname).v1),aname.v1,afmt) v1;\
    MFIELD_IMPL(MFIELD_INDEX_VEC(aname,v2),atype,v2,adescr,ASTRINGZ(MFIELD_VAR(aname).v2),aname.v2,afmt) v2;\
    MFIELD_IMPL(MFIELD_INDEX_VEC(aname,v3),atype,v3,adescr,ASTRINGZ(MFIELD_VAR(aname).v3),aname.v3,afmt) v3;\
    uint pack(_message *data,_message::_vscale vscale) const {return _field_vect::pack(data,&v1,&v2,&v3,vscale);} \
    uint pack2(_message *data) const {return _field_vect::pack(data,&v1,&v2);} \
    uint unpack(const _message *data) { \
      if(data->index!=v1.index())return 0; \
      if(data->dtype==_message::dt_vect)return (v1.unpack(data)&&v2.unpack(data)&&v3.unpack(data))?data->size:0; \
      if(data->dtype==_message::dt_vec2)return (v1.unpack(data)&&v2.unpack(data))?data->size:0; \
      return 0; \
    } \
  }aname;


#define MFVEC2(atype,aname,v1,v2,adescr,afmt,...) \
  class MFIELD_TYPE(aname) : public _field_vect { public: \
    operator Vector<2,atype>()const{return Vector<2,atype>(v1,v2);} \
    MFIELD_TYPE(aname) & operator=(const Vector<2,atype>& v){v1=v[0];v2=v[1];return *this;} \
    MFIELD_IMPL(MFIELD_INDEX_VEC(aname,v1),atype,v1,adescr,ASTRINGZ(MFIELD_VAR(aname).v1),aname.v1,afmt) v1;\
    MFIELD_IMPL(MFIELD_INDEX_VEC(aname,v2),atype,v2,adescr,ASTRINGZ(MFIELD_VAR(aname).v2),aname.v2,afmt) v2;\
    uint pack(_message *data) const {return _field_vect::pack(data,&v1,&v2);} \
    uint unpack(const _message *data) { \
      if(data->index!=v1.index())return 0; \
      if(data->dtype==_message::dt_vec2)return (v1.unpack(data)&&v2.unpack(data))?data->size:0; \
      return 0; \
    } \
  }aname;

#define MGRP_IMPL(aname,adescr) \
  class ATPASTE3(_,aname,_base) : public _group { public: \
    const char* name()const{return ASTRINGZ(aname);} \
    const char* descr()const{return adescr;} \
    uint unpack(const _message *data) \
    { \
      _field *f=field(data->index); \
      if(!f)return 0; \
      if(data->dtype==_message::dt_vect){ \
        for(uint i=0;i<3;i++){ \
          _field *f=field(data->index+i); \
          if(!f)return 0; \
          f->unpack(data); \
        } \
        return data->size; \
      }else if(data->dtype==_message::dt_vec2){ \
        for(uint i=0;i<2;i++){ \
          _field *f=field(data->index+i); \
          if(!f)return 0; \
          f->unpack(data); \
        } \
        return data->size; \
      }else return f->unpack(data); \
    } \

#define MGRP0_BEGIN(adescr,...) MGRP_IMPL(MGRP0,adescr)
#define MGRP0_END               };
#define MGRP1_BEGIN(adescr,...) MGRP_IMPL(MGRP1,adescr)
#define MGRP1_END               };
#define MGRP2_BEGIN(adescr,...) MGRP_IMPL(MGRP2,adescr)
#define MGRP2_END               };

#include "MandalaFields.h"


//grp2
#define MGRP_IMPL(aname,adescr) \
  class ATPASTE3(_,aname,_base_grp2) : public ATPASTE3(_,aname,_base) { public: \

#define MGRP0_BEGIN(adescr,...) MGRP_IMPL(MGRP0,adescr)
#define MGRP0_END               };
#define MGRP1_BEGIN(adescr,...) MGRP_IMPL(MGRP1,adescr)
#define MGRP1_END               };
#define MGRP2_BEGIN(adescr,...) \
  class ATPASTE2(_,MGRP2) : public ATPASTE3(_,MGRP2,_base) { public: \
  _mandala_index index()const{return ATPASTE7(index_,MGRP0,_,MGRP1,_,MGRP2,_next)+1;} \
  _group * group(const _mandala_index idx) { (void)idx; return NULL; } \
  _group * next_group(const _group *g) { (void)g; return NULL; } \
  _field * next_field(_field *f) { \
    if(!f) return field(index()); \
    _mandala_index i=f->index()+1; \
    return field(i); \
  } \
  _field * field(const _mandala_index idx) { \
    switch(idx){default: return NULL;
#define MGRP2_END \
    } } \
  }MGRP2;

#define MFIELD(atype,aname,...) case MFIELD_INDEX(aname): return & (aname);
#define MFVECT_IMPL(atype,aname,vname,...) case MFIELD_INDEX_VEC(aname,vname): return & ((aname).vname);

#include "MandalaFields.h"

//grp1
#define MGRP_IMPL(aname,adescr) \
  class ATPASTE3(_,aname,_base_grp1) : public ATPASTE3(_,aname,_base_grp2) { public: \

#define MGRP0_BEGIN(adescr,...) MGRP_IMPL(MGRP0,adescr)
#define MGRP0_END               };
#define MGRP1_BEGIN(adescr,...) \
class ATPASTE2(_,MGRP1) : public ATPASTE3(_,MGRP1,_base_grp2) { public: \
  _mandala_index index()const{return ATPASTE5(index_,MGRP0,_,MGRP1,_next)+1;} \
  _field * field(const _mandala_index idx) { \
    _group *g=group(idx); \
    if(!g)return NULL; \
    return g->field(idx); \
  } \
  _group * next_group(const _group *g) { \
    if(!g)return group(ATPASTE5(index_,MGRP0,_,MGRP1,_next)+1); \
    return group((g->index()&~GRP2_MASK)+(GRP2_MASK+1)); \
  } \
  _field * next_field(_field *f) { \
    if(!f) return field(index()); \
    _mandala_index i=f->index()+1; \
    f=field(i); \
    if(f)return f; \
    i=(i&(~GRP2_MASK))+(GRP2_MASK+1); \
    return field(i); \
  } \
  _group * group(const _mandala_index idx) { \
    switch(idx&~GRP2_MASK){default: return NULL;

#define MGRP1_END \
    } } \
  }MGRP1;

#define MGRP2_BEGIN(adescr,...)     case ATPASTE7(index_,MGRP0,_,MGRP1,_,MGRP2,_next)+1: return &(MGRP2);

#include "MandalaFields.h"

//grp0
#define MGRP_IMPL(aname,adescr) \
  class ATPASTE3(_,aname,_base_grp0) : public ATPASTE3(_,aname,_base_grp1) { public: \

#define MGRP0_BEGIN(adescr,...) \
  class ATPASTE2(_,MGRP0) : public ATPASTE3(_,MGRP0,_base_grp1) { public: \
  _mandala_index index()const{return ATPASTE3(index_,MGRP0,_next)+1;} \
  _field * field(const _mandala_index idx) { \
    _group *g=group(idx); \
    if(!g)return NULL; \
    g=g->group(idx); \
    if(!g)return NULL; \
    return g->field(idx); \
  } \
  _group * next_group(const _group *g) { \
    if(!g)return group(ATPASTE3(index_,MGRP0,_next)+1); \
    return group((g->index()&~GRP1_MASK)+(GRP1_MASK+1)); \
  } \
  _field * next_field(_field *f) { \
    if(!f) return field(index()); \
    _mandala_index i=f->index()+1; \
    f=field(i); \
    if(f)return f; \
    i=(i&(~GRP2_MASK))+(GRP2_MASK+1); \
    f=field(i); \
    if(f)return f; \
    i=(i&(~GRP1_MASK))+(GRP1_MASK+1); \
    return field(i); \
  } \
  _group * group(const _mandala_index idx) { \
    switch(idx&~GRP1_MASK){default: return NULL;

#define MGRP0_END \
    } } \
  }MGRP0;

#define MGRP1_BEGIN(adescr,...)     case ATPASTE5(index_,MGRP0,_,MGRP1,_next)+1: return &(MGRP1);

#include "MandalaFields.h"




//-----------------------------------------------------------------------------
// METHODS
//-----------------------------------------------------------------------------
public:

  // Iterators
  _field * field(const _mandala_index index)
  {
    _group *g=group(index);
    if(!g)return NULL;
    g=g->group(index);
    if(!g)return NULL;
    g=g->group(index);
    if(!g)return NULL;
    return g->field(index);
  }
  _group * group(const _mandala_index index)
  {
    switch(index&~GRP0_MASK){
      default: return NULL;
      #define MGRP0_BEGIN(adescr,...)     case ATPASTE3(index_,MGRP0,_next)+1: return &MGRP0;
      #include "MandalaFields.h"
    }
  }
  _field * next_field(_field *f)
  {
    _mandala_index i=f?f->index()+1:0;
    f=field(i);
    if(f)return f;
    //try to inc grp2
    i=(i&(~GRP2_MASK))+(GRP2_MASK+1);
    f=field(i);
    if(f)return f;
    //try to inc grp1
    i=(i&(~GRP1_MASK))+(GRP1_MASK+1);
    f=field(i);
    if(f)return f;
    //try to inc grp0
    i=(i&(~GRP0_MASK))+(GRP0_MASK+1);
    return field(i);
  }
  _group * next_group(_group *g)
  {
    if(!g)return group(0);
    return group((g->index()&~GRP0_MASK)+(GRP0_MASK+1)); \
  }

  //pack - unpack & messages
  uint unpack(const _message *data)
  {
    _group *g=group(data->index);
    return g?g->unpack(data):0;
  }
  _mandala_float value(const _mandala_index index)
  {
    _field *f=field(index);
    return f?f->toFloat():0;
  }
  void setValue(const _mandala_index index,const _mandala_float v)
  {
    _field *f=field(index);
    if(!f)return;
    f->fromFloat(v);
  }
  void setValue(const _mandala_index index,const _mandala_uint v)
  {
    _field *f=field(index);
    if(!f)return;
    f->fromUInt(v);
  }
};
#undef MFIELD_TYPE
#undef MFIELD_VAR
#undef MFIELD_INDEX
#undef MFIELD_INDEX_VEC
//-----------------------------------------------------------------------------
//extern _mandala mf;
//=============================================================================
#endif
//=============================================================================
// LIST INCLUDE PART
//=============================================================================
//=============================================================================
#ifndef MGRP0_BEGIN
#define MGRP0_BEGIN(...)
#endif
#ifndef MGRP0_END
#define MGRP0_END
#endif
#ifndef MGRP1_BEGIN
#define MGRP1_BEGIN(...)
#endif
#ifndef MGRP1_END
#define MGRP1_END
#endif
#ifndef MGRP2_BEGIN
#define MGRP2_BEGIN(...)
#endif
#ifndef MGRP2_END
#define MGRP2_END
#endif
#ifndef MFIELD
#define MFIELD(...)
#endif
#ifndef MFVECT_IMPL
#define MFVECT_IMPL(atype,aname,vname,...) MFIELD(atype,vname,__VA_ARGS__)
#endif
#ifndef MFVECT
#define MFVECT(atype,aname,v1,v2,v3,...) \
  MFVECT_IMPL(atype,aname,v1,__VA_ARGS__) \
  MFVECT_IMPL(atype,aname,v2,__VA_ARGS__) \
  MFVECT_IMPL(atype,aname,v3,__VA_ARGS__)
#endif
#ifndef MFVEC2
#define MFVEC2(atype,aname,v1,v2,...) \
  MFVECT_IMPL(atype,aname,v1,__VA_ARGS__) \
  MFVECT_IMPL(atype,aname,v2,__VA_ARGS__)
#endif
#ifndef MFENUM
#define MFENUM(...)
#endif
//=============================================================================
#define MGRP0   vehicle
MGRP0_BEGIN("Vehicle")
//-------------------------
#define MGRP1   sensor
MGRP1_BEGIN("Sensors")

#define MGRP2   imu
MGRP2_BEGIN("Inertial Measurement Unit")
MFVECT(float,  acc,x,y,z, "Acceleration [m/s2]", f2)
MFVECT(float,  gyro,x,y,z,"Angular rate [deg/s]", f2)
MFVECT(float,  mag,x,y,z, "Magnetic field [a.u.]", f2)
MFIELD(float,  temp,      "IMU temperature [C]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   gps
MGRP2_BEGIN("Global Positioning System")
MFVECT(float,  pos,lat,lon,hmsl, "GPS position [deg,deg,m]", f4)
MFVECT(float,  vel,N,E,D,        "GPS velocity [m/s]", f2)
MFIELD(uint,   UTC, "GPS UTC Time from 1970 1st Jan [sec]", u4)
MFIELD(byte,   SV,  "GPS Satellites visible [number]", )
MFIELD(byte,   SU,  "GPS Satellites used [number]", )
MGRP2_END
#undef MGRP2

#define MGRP2   air
MGRP2_BEGIN("Aerodynamic sensors")
MFIELD(float, pt,    "Airspeed [m/s]", f2)
MFIELD(float, ps,    "Barometric altitude [m]", f4)
MFIELD(float, PT,    "Pitot probe temperature [C]", s1)
MFIELD(float, vario, "Barometric variometer [m/s]", f2)
MFIELD(float, slip,  "Slip [deg]", f2)
MFIELD(float, aoa,   "Angle of attack [deg]", f2)
MFIELD(float, buo,   "Blimp ballonet pressure [kPa]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   range
MGRP2_BEGIN("Distance to ground")
MFIELD(float, ultrasonic, "Ultrasonic altimeter [m]", f2)
MFIELD(float, laser,      "Laser altimeter [m]", f2)
MFIELD(float, radio,      "Radio altimeter [m]", f4)
MFIELD(float, proximity,  "Proximity sensor [m]", f2)
MFIELD(float, touch,      "Gear force sensor [N]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   lps
MGRP2_BEGIN("Local Positioning System")
MFVECT(float, pos,x,y,z, "Local position sensor [m]", f2)
MFVECT(float, vel,x,y,z, "Local velocity sensor [m/s]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   ils
MGRP2_BEGIN("Instrument Landing System")
MFIELD(float, HDG,    "ILS heading to VOR1 [deg]", f2)
MFIELD(uint,  DME,    "ILS distance to VOR1 [m]", u2)
MFIELD(float, RSS,    "ILS signal strength [0..1]", f2)
MFVEC2(float, offset,HDG,altitude, "ILS offset [deg,m]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   platform
MGRP2_BEGIN("Landing Platform sensors")
MFVECT(float,  pos,lat,lon,hmsl,   "Platform position [deg,deg,m]", f4)
MFVECT(float,  vel,N,E,D,          "Platform velocity [m/s]", f2)
MFVECT(float,  att,roll,pitch,yaw, "Platform attitude [deg]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   radar
MGRP2_BEGIN("Radar target tracking")
MFIELD(byte,  id,     "Radar target ID", )
MFIELD(float, HDG,    "Radar heading to target [deg]", f2)
MFIELD(uint,  DME,    "Radar distance to target [m]", u2)
MFIELD(float, vel,    "Radar target velocity [m/s]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   engine
MGRP2_BEGIN("Engine sensors")
MFIELD(uint,  rpm,      "Engine RPM [1/min]", u2)
MFIELD(uint,  rpm_prop, "Prop RPM [1/min]", u2)
MFIELD(float, fuel,     "Fuel capacity [l]", f2)
MFIELD(float, frate,    "Fuel flow rate [l/h]", f2)
MFIELD(float, ET,       "Engine temperature [C]", u1)
MFIELD(float, OT,       "Oil temperature [C]", u1)
MFIELD(float, OP,       "Oil pressure [atm]", u01)
MFIELD(float, EGT1,     "Exhaust 1 temperature [C]", u10)
MFIELD(float, EGT2,     "Exhaust 2 temperature [C]", u10)
MFIELD(float, EGT3,     "Exhaust 3 temperature [C]", u10)
MFIELD(float, EGT4,     "Exhaust 4 temperature [C]", u10)
MFIELD(float, MAP,      "MAP pressure [Pa]", f2)
MFIELD(float, IAP,      "Intake air box pressure [kPa]", f2)
MFIELD(bit,   error,    "Engine error/ok", )
MGRP2_END
#undef MGRP2

#define MGRP2   power
MGRP2_BEGIN("System power")
MFIELD(float, Ve,    "System battery voltage [v]", f2)
MFIELD(float, Ie,    "System current [A]", u001)
MFIELD(float, Vs,    "Servo battery voltage [v]", f2)
MFIELD(float, Is,    "Servo current [A]", u01)
MFIELD(float, Vp,    "Payload battery voltage [v]", f2)
MFIELD(float, Ip,    "Payload current [A]", u01)
MFIELD(float, Vm,    "ECU battery voltage [v]", f2)
MFIELD(float, Im,    "ECU current [A]", u1)
MFIELD(bit, shutdown,"System trigger shutdown/on", )
MGRP2_END
#undef MGRP2

#define MGRP2   battery
MGRP2_BEGIN("Battery management")
MFIELD(float, Vbat1,    "Battery 1 voltage [v]", f2)
MFIELD(float, Ib1,      "Battery 1 current [A]", u01)
MFIELD(float, BT1,      "Battery 1 temperature [C]", s1)
MFIELD(bit,   err_b1,   "Battery 1 error/ok", )
MFIELD(float, Vbat2,    "Battery 2 voltage [v]", f2)
MFIELD(float, Ib2,      "Battery 2 current [A]", u01)
MFIELD(float, BT2,      "Battery 2 temperature [C]", s1)
MFIELD(bit,   err_b2,   "Battery 2 error/ok", )
MFIELD(float, Vgen,     "Power generator voltage [v]",f2)
MFIELD(float, Igen,     "Charging current [A]", u01)
MFIELD(float, Ichg,     "Charging current [A]", u01)
MFIELD(bit,   err_gen,  "Power generator error/ok", )
MGRP2_END
#undef MGRP2

#define MGRP2   temp
MGRP2_BEGIN("Temperature sensors")
MFIELD(float, AT,       "Ambient temperature [C]", s1)
MFIELD(float, RT,       "Room temperature [C]", s1)
MGRP2_END
#undef MGRP2

#define MGRP2   datalink
MGRP2_BEGIN("Datalink radio sensors")
MFIELD(float, RSS,    "Modem signal strength [0..1]", f2)
MFIELD(float, MT,     "Modem temperature [C]", s1)
MFIELD(float, HDG,    "Modem heading to transmitter [deg]", f2)
MFIELD(uint,  DME,    "Modem distance to transmitter [m]", u4)
MGRP2_END
#undef MGRP2

#define MGRP2   pilot
MGRP2_BEGIN("Pilot stick sensors")
MFIELD(byte,  override, "RC override [0..255]", )
MFIELD(float, roll,     "RC roll [-1..0..+1]", s001)
MFIELD(float, pitch,    "RC pitch [-1..0..+1]", s001)
MFIELD(float, throttle, "RC throttle [0..1]", s001)
MFIELD(float, yaw,      "RC yaw [-1..0..+1]", s001)
MGRP2_END
#undef MGRP2

#define MGRP2   steering
MGRP2_BEGIN("Steering encoders")
MFVEC2(float, vel,left,right,     "Steering velocity [m/s]", f2)
MFIELD(float, HDG,                "Steering heading [deg]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   ers
MGRP2_BEGIN("ERS sensors")
MFIELD(bit, error,   "ERS error/ok", )
MFIELD(bit, disarm,  "ERS disarmed/armed", )
MGRP2_END
#undef MGRP2

MGRP1_END
#undef MGRP1
//-------------------------
#define MGRP1   control
MGRP1_BEGIN("Control outputs")

#define MGRP2   stability
MGRP2_BEGIN("Stability fast controls")
MFIELD(float,  ail,     "Ailerons [-1..0..+1]", s001)
MFIELD(float,  elv,     "Elevator [-1..0..+1]", s001)
MFIELD(float,  rud,     "Rudder [-1..0..+1]", s001)
MFIELD(float,  collective, "Collective pitch [-1..0..+1]", s001)
MFIELD(float,  steering,   "Steering [-1..0..+1]", s001)
MGRP2_END
#undef MGRP2

#define MGRP2   engine
MGRP2_BEGIN("Engine controls")
MFIELD(float,  thr,     "Throttle [0..1]", u001)
MFIELD(float,  prop,    "Prop pitch [-1..0..+1]", s001)
MFIELD(float,  mix,     "Mixture [0..1]", u001)
MFIELD(float,  tune,    "Engine tuning [-1..0..1]", s001)
MFIELD(float,  vector,  "Thrust vector [-1..0..+1]", s001)
MFIELD(bit,    starter, "Engine starter on/off", )
MFIELD(bit,    rev,     "Thrust reverse on/off", )
MGRP2_END
#undef MGRP2

#define MGRP2   wing
MGRP2_BEGIN("Wing mechanization")
MFIELD(float,  flaps,   "Flaps [0..1]", u001)
MFIELD(float,  airbrk,  "Airbrakes [0..1]", u001)
MFIELD(float,  slats,   "Wing slats [0..1]", u001)
MFIELD(float,  sweep,   "Wing sweep [-1..0..+1]", s001)
MFIELD(float,  buoyancy,"Buoyancy [-1..0..+1]", s001)
MGRP2_END
#undef MGRP2

#define MGRP2   brakes
MGRP2_BEGIN("Brakes system control")
MFIELD(float,  brake,   "Brake [0..1]", u001)
MFIELD(float,  brakeL,  "Left brake [0..1]", u001)
MFIELD(float,  brakeR,  "Right brake [0..1]", u001)
MGRP2_END
#undef MGRP2

#define MGRP2   ers
MGRP2_BEGIN("Emergency Recovery System controls")
MFIELD(bit,  launch,    "ERS on/off", )
MFIELD(bit,  rel,       "Parachute released/locked", )
MGRP2_END
#undef MGRP2

#define MGRP2   gear
MGRP2_BEGIN("Landing Gear controls")
MFIELD(bit,  retract,    "Landing gear retracted/extracted", )
MGRP2_END
#undef MGRP2

#define MGRP2   fuel
MGRP2_BEGIN("Fuel pumps")
MFIELD(bit,  pump,    "Fuel pump on/off", )
MFIELD(bit,  xfeed,   "Crossfeed on/off", )
MGRP2_END
#undef MGRP2

#define MGRP2   power
MGRP2_BEGIN("Power management controls")
MFIELD(bit,  ap,        "Avionics", )
MFIELD(bit,  servo,     "Servo on/off", )
MFIELD(bit,  ignition,  "Engine on/off", )
MFIELD(bit,  payload,   "Payload on/off", )
MFIELD(bit,  agl,       "AGL sensor", )
MFIELD(bit,  xpdr,      "XPDR on/off", )
MFIELD(bit,  satcom,    "Satcom on/off", )
MFIELD(bit,  rfamp,     "RF amplifier on/off", )
MFIELD(bit,  ils,       "Instrument Landing System on/off", )
MFIELD(bit,  ice,       "Anti-ice on/off", )
MGRP2_END
#undef MGRP2

#define MGRP2   light
MGRP2_BEGIN("Lighting")
MFIELD(bit,  beacon,    "Beacon light on/off", )
MFIELD(bit,  landing,   "Landing lights on/off", )
MFIELD(bit,  nav,       "Navigation lights on/off", )
MFIELD(bit,  strobe,    "Strobe light on/off", )
MFIELD(bit,  taxi,      "Taxi lights on/off", )
MGRP2_END
#undef MGRP2

#define MGRP2   doors
MGRP2_BEGIN("Doors")
MFIELD(bit,  main,    "Main door open/locked", )
MFIELD(bit,  drop,    "Drop-off open/locked", )
MGRP2_END
#undef MGRP2

MGRP1_END
#undef MGRP1
//-------------------------
#define MGRP1   state
MGRP1_BEGIN("Current system state")

#define MGRP2   ahrs
MGRP2_BEGIN("Attitude and position estimation")
MFVECT(float, att,roll,pitch,yaw,    "Attitude [deg]", f2)
MFIELD(float, lat,      "Latitude [deg]", f4)
MFIELD(float, lon,      "Longitude [deg]", f4)
MFVEC2(float, NE,N,E,   "Local position [m]", f4)
MFVEC2(float, vel,N,E,  "Local velocity [m/s]", f2)
MFIELD(float, altitude, "Altitude [m]", f4)
MFIELD(float, course,   "Moving direction [deg]", f2)
MFIELD(float, gSpeed,   "Ground speed [m/s]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   flight
MGRP2_BEGIN("Flight parameters")
MFIELD(float, airspeed, "Airspeed [m/s]", f2)
MFIELD(float, vspeed,   "Vertical speed [m/s]", f2)
MFIELD(float, ldratio,  "Glide ratio [Lift/Drag]", f2)
MFIELD(float, venergy,  "Compensated variometer [m/s]", f2)
MFIELD(float, agl,      "Above Ground Level altitude [m]", f2)
MFIELD(float, vcas,     "Airspeed derivative [m/s^2]", f2)
MFIELD(float, denergy,  "Venergy derivative [m/s^2]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   status
MGRP2_BEGIN("Status of subsystems")
MFIELD(bit,  rc,     "RC on/off", )
MFIELD(bit,  gps,    "GPS available/lost", )
MFIELD(bit,  agl,    "AGL available/off", )
MFIELD(bit,  uplink, "Uplink available/lost", )
MFIELD(bit,  touch,  "Landing gear touchdown/floating", )
MGRP2_END
#undef MGRP2

#define MGRP2   error
MGRP2_BEGIN("System errors and warnings")
MFIELD(bit,  fatal,     "Fatal error/ok", )
MFIELD(bit,  power,     "Power supply error/ok", )
MFIELD(bit,  engine,    "Engine error/ok", )
MFIELD(bit,  rpm,       "RPM sensor error/ok", )
MFIELD(bit,  cas,       "CAS error", )
MFIELD(bit,  pstatic,   "Static pressure error/ok", )
MFIELD(bit,  gyro,      "IMU gyros bias", )
MFIELD(bit,  ers,       "ERS error/ok", )
MGRP2_END
#undef MGRP2

#define MGRP2   cmd
MGRP2_BEGIN("Commanded values")
MFVECT(float, att,roll,pitch,yaw,       "Commanded attitude [deg]", f2)
MFVEC2(float, NE,N,E,                   "Commanded position [m]", f4)
MFVECT(float, pos,lat,lon,hmsl,         "Commanded global position [deg,deg,m]", f4)
MFIELD(float, course,   "Commanded course [deg]", f2)
MFIELD(float, altitude, "Commanded altitude [m]", f4)
MFIELD(float, airspeed, "Commanded airspeed [m/s]", f2)
MFIELD(float, vspeed,   "Commanded vspeed [m/s]", f2)
MFIELD(float, slip,     "Commanded slip [deg]", f2)
MFIELD(uint,  rpm,      "Commanded RPM [1/m]", u2)
MGRP2_END
#undef MGRP2

#define MGRP2   maneuver
MGRP2_BEGIN("Maneuver parameters")
MFIELD(enum,  mode,      "flight mode", )
MFENUM(mode,   EMG,     "Realtime control",       0)
MFENUM(mode,   RPV,     "Angles control",         1)
MFENUM(mode,   UAV,     "Heading control",        2)
MFENUM(mode,   WPT,     "Waypoints navigation",   3)
MFENUM(mode,   HOME,    "Go back home",           4)
MFENUM(mode,   STBY,    "Loiter around DNED",     5)
MFENUM(mode,   TAXI,    "Taxi",                   6)
MFENUM(mode,   TAKEOFF, "Takeoff",                7)
MFENUM(mode,   LANDING, "Landing",                8)
MFIELD(byte,  stage,     "Maneuver stage", )
MFIELD(enum,  mtype,     "Mission maneuver type", )
MFENUM(mtype,   hdg,     "Heading navigation",      0)
MFENUM(mtype,   line,    "Line navigation",         1)
MFIELD(byte,  loops,     "Number of remaining turns or loops [0..255]", )
MFIELD(float, turnR,     "Current circle radius [m]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   cmode
MGRP2_BEGIN("Control system mode")
MFIELD(bit,  dlhd,      "High precision downstream on/off", )
MFIELD(bit,  thrcut,    "Throttle cut on/off", )
MFIELD(bit,  throvr,    "Throttle override on/off", )
MFIELD(bit,  hover,     "Stabilization mode hover/run", )
MFIELD(bit,  ahrs,      "AHRS mode inertial/gps", )
MGRP2_END
#undef MGRP2

#define MGRP2   nav
MGRP2_BEGIN("Navigation parameters")
MFVEC2(float, dXY,x,y,   "Bodyframe delta [m]", f2)
MFVEC2(float, vXY,x,y,   "Bodyframe velocity [m/s]", f2)
MFIELD(float, delta,     "General delta (depends on mode) [m]", f2)
MFIELD(float, tgHDG,     "Current tangent heading [deg]", f2)
MFIELD(float, stab,      "Stability [0..1]", u001)
MFIELD(float, corr,      "Correlator output [K]", f2)
MFIELD(float, rwAdj,     "Runway displacement adjust [m]", s1)
MFIELD(float, rwDelta,   "Runway alignment [m]", f2)
MFIELD(float, rwDV,      "Runway alignment velocity [m/s]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   mission
MGRP2_BEGIN("Current waypoint information")
MFIELD(float, DME,    "Distance to waypoint [m]", f2)
MFIELD(float, HDG,    "Current waypoint heading [deg]", f2)
MFIELD(uint,  ETA,    "Estimated time of arrival [s]", u4)
MFIELD(byte,  wpidx,  "Current waypoint [0..255]", )
MFIELD(byte,  rwidx,  "Current runway [0..255]", )
MFIELD(byte,  twidx,  "Current taxiway [0..255]", )
MFIELD(byte,  piidx,  "Current point of interest [0..255]", )
MFIELD(enum,  midx,   "Mission index action", )
MFENUM(midx,   inc,   "Increment mission index", 0)
MFENUM(midx,   dec,   "Decrement mission index", 1)
MGRP2_END
#undef MGRP2

#define MGRP2   ils
MGRP2_BEGIN("Instrument Landing System status")
MFIELD(bit, armed,    "ILS armed/off", )
MFIELD(bit, approach, "ILS approach available/lost", )
MFIELD(bit, offset,   "ILS offset available/lost", )
MFIELD(bit, platform, "ILS platform available/lost", )
MGRP2_END
#undef MGRP2

#define MGRP2   home
MGRP2_BEGIN("Home point")
MFVECT(float,  pos,lat,lon,hmsl,"Home global position [deg,deg,m]", f4)
MFIELD(float,  altps,           "Barometric altitude on ground level [m]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   wind
MGRP2_BEGIN("Wind estimator")
MFIELD(float,  speed,   "wind speed [m/s]", u01)
MFIELD(float,  HDG,     "wind direction to [deg]", f2)
MFIELD(float,  cas2tas, "CAS to TAS multiplier [K]", u001)
MGRP2_END
#undef MGRP2

#define MGRP2   downlink
MGRP2_BEGIN("Downlink parameters")
MFIELD(uint,  period,    "downlink period [ms]", u2)
MFIELD(uint,  timestamp, "downlink timestamp [ms]", u4)
MGRP2_END
#undef MGRP2

MGRP1_END
#undef MGRP1
//-------------------------
#define MGRP1   info
MGRP1_BEGIN("Information to human")

#define MGRP2   ground
MGRP2_BEGIN("Ground Station info")
MFIELD(float, RSS, "GCU modem signal strength [0..1]", f2)
MFIELD(float, Ve,  "GCU system battery voltage [v]", f2)
MFIELD(float, MT,  "GCU modem temperature [C]", s1)
MGRP2_END
#undef MGRP2

#define MGRP2   meteo
MGRP2_BEGIN("Meteo base station info")
MFIELD(float,  windSpd, "wind speed [m/s]", u01)
MFIELD(float,  windHdg, "wind direction to [deg]", f2)
MFIELD(float,  temp,    "Ground outside air temperature [C]", s1)
MFIELD(float,  altps,   "barometric altitude on ground level [m]", f2)
MGRP2_END
#undef MGRP2


MGRP1_END
#undef MGRP1

MGRP0_END
#undef MGRP0
//-----------------------------------------------------------------------------
#define MGRP0   payload
MGRP0_BEGIN("Payload")
#define MGRP1   cam
MGRP1_BEGIN("Camera gimbal")

#define MGRP2   imu
MGRP2_BEGIN("Camera IMU")
MFVECT(float,   acc,x,y,z, "Cam acceleration [m/s2]", f2)
MFVECT(float,   gyro,x,y,z,"Cam angular rate [deg/s]", f2)
MFVECT(float,   mag,x,y,z, "Cam magnetic field [a.u.]", f2)
MFIELD(float,   temp,      "Cam IMU temperature [C]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   state
MGRP2_BEGIN("Camera control system state")
MFVECT(float, att,roll,pitch,yaw, "Cam attitude [deg]", f2)
MFIELD(uint,  timestamp, "Cam timestamp [ms]", u4)
MGRP2_END
#undef MGRP2

#define MGRP2   cmd
MGRP2_BEGIN("Camera commanded values")
MFIELD(enum,  camop,     "Cam control mode", )
MFENUM(camop,   off,     "camera off",                   0)
MFENUM(camop,   fixed,   "fixed position",               1)
MFENUM(camop,   stab,    "gyro stabilization",           2)
MFENUM(camop,   position,"attitude position",            3)
MFENUM(camop,   speed,   "attitude speed control",       4)
MFENUM(camop,   target,  "target position tracking",     5)
MFVECT(float, att,roll,pitch,yaw,       "Commanded cam attitude [deg]", f4)
MFIELD(float, zoom,     "Cam zoom level [0..1]", f2)
MFIELD(float, focus,    "Cam focus [0..1]", f2)
MFIELD(uint,  tperiod,  "Cam period for timestamps [ms]", u2)
MFIELD(byte,  ch,       "Video channel [0..255]", )
MGRP2_END
#undef MGRP2

#define MGRP2   tune
MGRP2_BEGIN("Camera tuning")
MFVECT(float, bias,roll,pitch,yaw,       "Cam stability bias [deg/s]", f4)
MGRP2_END
#undef MGRP2

#define MGRP2   options
MGRP2_BEGIN("Camera options")
MFIELD(bit, PF,     "picture flip on/off", )
MFIELD(bit, NIR,    "NIR filter on/off", )
MFIELD(bit, DSP,    "display information on/off",)
MFIELD(bit, FMI,    "focus mode infinity/auto", )
MFIELD(bit, FM,     "focus manual/auto", )
MGRP2_END
#undef MGRP2

#define MGRP2   control
MGRP2_BEGIN("Camera controls output")
MFVECT(float, servo,roll,pitch,yaw, "Cam servo [-1..0..+1]", s001)
MFIELD(bit,   rec,      "Recording", )
MFIELD(bit,   shot,     "Snapshot", )
MFIELD(bit,   ashot,    "Series snapshots", )
MGRP2_END
#undef MGRP2

#define MGRP2   tracking
MGRP2_BEGIN("Camera tracker")
MFVECT(float,  pos,lat,lon,hmsl,"Tracking position [deg,deg,m]", f4)
MFIELD(float,  DME,             "Distance to tracking object [m]", f2)
MGRP2_END
#undef MGRP2

MGRP1_END
#undef MGRP1
//-------------------------
#define MGRP1   turret
MGRP1_BEGIN("Turret")

#define MGRP2   state
MGRP2_BEGIN("Turret system state")
MFVECT(float, att,roll,pitch,yaw, "Turret attitude [deg]", f2)
MFVECT(float, enc,roll,pitch,yaw, "Turret encoders [deg]", f4)
MGRP2_END
#undef MGRP2

#define MGRP2   cmd
MGRP2_BEGIN("Turret commanded values")
MFIELD(enum,  turret,     "Turret control mode", )
MFENUM(turret,   off,     "turret off",                   0)
MFENUM(turret,   fixed,   "fixed position",               1)
MFENUM(turret,   stab,    "gyro stabilization",           2)
MFENUM(turret,   position,"attitude position",            3)
MFENUM(turret,   speed,   "attitude speed control",       4)
MFVECT(float, att,roll,pitch,yaw,       "Commanded turret attitude [deg]", f4)
MFIELD(byte,  ammo,     "Turret ammo [0..255]", )
MGRP2_END
#undef MGRP2

#define MGRP2   tune
MGRP2_BEGIN("Turret tuning")
MFVECT(float, bias,roll,pitch,yaw,       "Turret stability bias [deg/s]", f4)
MGRP2_END
#undef MGRP2

#define MGRP2   options
MGRP2_BEGIN("Turret options")
MFIELD(bit, armed,     "Turret armed/disarmed",        )
MFIELD(bit, shoot,     "Turret shooting/standby",      )
MFIELD(bit, reload,    "Turret reloading/reloaded",    )
MGRP2_END
#undef MGRP2

#define MGRP2   control
MGRP2_BEGIN("Turret controls output")
MFVECT(float, servo,roll,pitch,yaw, "Turret servo [-1..0..+1]", s001)
MGRP2_END
#undef MGRP2

MGRP1_END
#undef MGRP1
MGRP0_END
#undef MGRP0
//-----------------------------------------------------------------------------
#define MGRP0   aux
MGRP0_BEGIN("Auxilary data")
#define MGRP1   local
MGRP1_BEGIN("Node local data fields")

#define MGRP2   vm
MGRP2_BEGIN("Virtual Machine accessible fields")
MGRP2_END
#undef MGRP2

#define MGRP2   tmp
MGRP2_BEGIN("Temporary data")
MGRP2_END
#undef MGRP2

#define MGRP2   storage
MGRP2_BEGIN("Persistent data")
MGRP2_END
#undef MGRP2

#define MGRP2   config
MGRP2_BEGIN("Configuration data")
MGRP2_END
#undef MGRP2

MGRP1_END
#undef MGRP1
//-------------------------
#define MGRP1   shared
MGRP1_BEGIN("Network shared fields")

#define MGRP2   vm
MGRP2_BEGIN("Virtual Machine accessible fields")
MGRP2_END
#undef MGRP2

#define MGRP2   tmp
MGRP2_BEGIN("Temporary data")
MGRP2_END
#undef MGRP2

MGRP1_END
#undef MGRP1
//-------------------------
#define MGRP1   telemetry
MGRP1_BEGIN("Auxilary telemetry fields")

#define MGRP2   usr
MGRP2_BEGIN("User values")
MFIELD(float, u1,  "User value 1", f2)
MFIELD(float, u2,  "User value 2", f2)
MFIELD(float, u3,  "User value 3", f2)
MFIELD(float, u4,  "User value 4", f2)
MFIELD(float, u5,  "User value 5", f2)
MFIELD(float, u6,  "User value 6", f2)
MFIELD(float, u7,  "User value 7", f2)
MFIELD(float, u8,  "User value 8", f2)
MFIELD(float, u9,  "User value 9", f2)
MFIELD(float, u10, "User value 10", f2)
MFIELD(float, u11, "User value 11", f2)
MFIELD(float, u12, "User value 12", f2)
MFIELD(float, u13, "User value 13", f2)
MFIELD(float, u14, "User value 14", f2)
MFIELD(float, u15, "User value 15", f2)
MFIELD(float, u16, "User value 16", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   trg
MGRP2_BEGIN("Binary user values")
MFIELD(bit, b1,  "User bit 1", )
MFIELD(bit, b2,  "User bit 2", )
MFIELD(bit, b3,  "User bit 3", )
MFIELD(bit, b4,  "User bit 4", )
MFIELD(bit, b5,  "User bit 5", )
MFIELD(bit, b6,  "User bit 6", )
MFIELD(bit, b7,  "User bit 7", )
MFIELD(bit, b8,  "User bit 8", )
MFIELD(bit, b9,  "User bit 9", )
MFIELD(bit, b10, "User bit 10", )
MFIELD(bit, b11, "User bit 11", )
MFIELD(bit, b12, "User bit 12", )
MFIELD(bit, b13, "User bit 13", )
MFIELD(bit, b14, "User bit 14", )
MFIELD(bit, b15, "User bit 15", )
MFIELD(bit, b16, "User bit 16", )
MGRP2_END
#undef MGRP2


MGRP1_END
#undef MGRP1
MGRP0_END
#undef MGRP0

//-----------------------------------------------------------------------------
#define MGRP0   stream
MGRP0_BEGIN("Streamed data")
MGRP0_END
#undef MGRP0

//=============================================================================
//=============================================================================
//=============================================================================
#undef MGRP0_BEGIN
#undef MGRP0_END
#undef MGRP1_BEGIN
#undef MGRP1_END
#undef MGRP2_BEGIN
#undef MGRP2_END
#undef MGRP_IMPL
#undef MFIELD
#undef MFVECT_IMPL
#undef MFVECT
#undef MFVEC2
#undef MFENUM
