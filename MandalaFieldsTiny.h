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
//=============================================================================
#undef MANDALA_FULL
#define MANDALA_PACKED_STRUCT   //__attribute__((packed))
#define MANDALA_INLINE
//#ifdef USE_FLOAT_TYPE
typedef float   _mandala_float;
//#else
//typedef double  _mandala_float;
//#endif
typedef uint16_t   _mandala_index;
typedef uint32_t   _mandala_uint;
typedef uint8_t    _mandala_byte;
typedef uint8_t    _mandala_bit;
typedef uint8_t    _mandala_enum;
#define GRP0_MASK       ((1<<11)-1)
#define GRP1_MASK       ((1<<9)-1)
#define GRP2_MASK       ((1<<4)-1)
//=============================================================================
#include "MatrixMath.h"
using namespace matrixmath;
//=============================================================================
typedef struct {
  //typedefs
  typedef struct{
    uint8_t  nID;         //Node ID
  }MANDALA_PACKED_STRUCT _msg_id;
  typedef struct{
    uint16_t data_type  :4; //data type
    uint16_t data_fcnt  :2; //frame cnt
    uint16_t error      :2; //error code
    uint16_t command    :8; //command byte
  }MANDALA_PACKED_STRUCT _data_hdr;
  //definitions
  _msg_id       id;     //1
  _data_hdr     hdr;    //2
}MANDALA_PACKED_STRUCT _mandala_msg;
//=============================================================================
class _mandala
{
public:
// index
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

//messages
  typedef struct{
    uint8_t size;       //size of data bytes
    uint16_t index :13; //field ID
    typedef enum{
      dt_float=0,
      dt_uint,
      dt_int,
    }_dtype; //8 types total
    _dtype   dtype :3;  //data payload type
    union{
      uint32_t data32;
      uint8_t  data[4];
    }payload;
  }MANDALA_PACKED_STRUCT _field_msg;

//interfaces
  class _field
  {
  public:
    //descr
    //virtual _mandala_index index()const =0;
    /*virtual const char* name()const =0;
    virtual const char* descr()const =0;
    virtual const char* shortname()const =0;
    //value
    virtual const _mandala_float toFloat() const =0;
    virtual void fromFloat(const _mandala_float &v) =0;
    virtual const _mandala_uint toUInt() const =0;
    virtual void fromUInt(const _mandala_uint &v) =0;
    //messages
    virtual uint unpack(const _field_msg *data) =0;
    virtual uint pack(_field_msg *data) const =0;*/
    //type
    typedef enum{
      mf_float,
      mf_uint,
      mf_byte,
      mf_bit,
      mf_enum,
    }_mf_type;
    //virtual _mf_type type(void) const =0;

    //ext packing (telemetry)
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
    //virtual _ext_fmt ext_fmt(void) =0;
  };

  class _group2
  {
  public:
    virtual _mandala_index index()const =0;
    virtual const char* name()const =0;
    virtual const char* descr()const =0;
    virtual uint unpack(const _field_msg *data) =0;
  };

  class _group1
  {
  public:
    virtual _mandala_index index()const =0;
    virtual const char* name()const =0;
    virtual const char* descr()const =0;
    virtual _group2 * group(const _mandala_index idx) =0;
    virtual _group2 * next_group(const _group2 *g) =0;
    virtual uint unpack(const _field_msg *data) =0;
  };

  class _group0
  {
  public:
    virtual _mandala_index index()const =0;
    virtual const char* name()const =0;
    virtual const char* descr()const =0;
    virtual _group1 * group(const _mandala_index idx) =0;
    virtual _group1 * next_group(const _group1 *g) =0;
    virtual uint unpack(const _field_msg *data) =0;
  };

  class _group
  {
  public:
    /*virtual _mandala_index index()const =0;
    virtual const char* name()const =0;
    virtual const char* descr()const =0;
    virtual _group * group(const _mandala_index idx) =0;
    virtual _group * next_group(const _group *g) =0;
    virtual _field * field(const _mandala_index idx) =0;
    virtual _field * next_field(_field *f) =0;
    virtual uint unpack(const _field_msg *data) =0;*/
  };


//field template
  template <class T>
  class _field_t : public _field
  {
  public:
    _field_t():_field(),m_value(0){}
    MANDALA_INLINE T & operator=(const T &v){setValue(v);return m_value;}
    MANDALA_INLINE operator T() const {return m_value;}

    MANDALA_INLINE const T & value() const {return m_value;}

    //interface
    MANDALA_INLINE const _mandala_float toFloat() const {return value();}
    MANDALA_INLINE void fromFloat(const _mandala_float &v){setValue(v);}
    MANDALA_INLINE const _mandala_uint toUInt() const {return value();}
    MANDALA_INLINE void fromUInt(const _mandala_uint &v){setValue(v);}

    uint unpack(const _field_msg *data)
    {
      const uint8_t *src=(const uint8_t *)data->payload.data;
      const uint8_t sz=data->size;
      switch(data->dtype){
        case _field_msg::dt_float:{
          if(sz!=4)break;
          float v;
          uint8_t *dest=(uint8_t *)&v;
          *dest++=*src++;
          *dest++=*src++;
          *dest++=*src++;
          *dest++=*src++;
          setValue(v);
        }return sz;
        case _field_msg::dt_uint:{
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
          }else break;
        }return sz;
        case _field_msg::dt_int:{
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
          }else break;
        }return sz;
        default:
          return 0;
      }
      return 0;
    }

  protected:
    T m_value;
    MANDALA_INLINE void setValue(const T &v)
    {
      m_value=v;
    }
    MANDALA_INLINE uint pack_float(_field_msg *data) const
    {
      //data->index=index();
      data->dtype=_field_msg::dt_float;
      data->size=4;
      const uint8_t *src=(const uint8_t *)&m_value;
      uint8_t *dest=(uint8_t *)data->payload.data;
      *dest++=*src++;
      *dest++=*src++;
      *dest++=*src++;
      *dest++=*src++;
      return data->size;
    }
    MANDALA_INLINE uint pack_uint(_field_msg *data) const
    {
      //data->index=index();
      data->dtype=_field_msg::dt_uint;
      data->size=4;
      const uint8_t *src=(const uint8_t *)&m_value;
      uint8_t *dest=(uint8_t *)data->payload.data;
      *dest++=*src++;
      *dest++=*src++;
      *dest++=*src++;
      *dest++=*src++;
      return data->size;
    }
    MANDALA_INLINE uint pack_byte(_field_msg *data) const
    {
      //data->index=index();
      data->dtype=_field_msg::dt_uint;
      data->size=1;
      data->payload.data[0]=m_value;
      return data->size;
    }
    MANDALA_INLINE uint pack_bit(_field_msg *data) const
    {
      //data->index=index();
      data->dtype=_field_msg::dt_uint;
      data->size=1;
      data->payload.data[0]=m_value>0?1:0;
      return data->size;
    }
    MANDALA_INLINE uint pack_enum(_field_msg *data) const
    {
      return pack_byte(data);
    }
  };

  //field value types
  class _field_float_t : public _field_t<_mandala_float>
  {
  public:
    uint pack(_field_msg *data) const
    {return _field_t<_mandala_float>::pack_float(data);}
  };
  class _field_uint_t : public _field_t<_mandala_uint>
  {
  public:
    uint pack(_field_msg *data) const
    {return _field_t<_mandala_uint>::pack_uint(data);}
  };
  class _field_byte_t : public _field_t<_mandala_byte>
  {
  public:
    uint pack(_field_msg *data) const
    {return _field_t<_mandala_byte>::pack_byte(data);}
  };
  class _field_bit_t : public _field_t<_mandala_bit>
  {
  public:
    uint pack(_field_msg *data) const
    {return _field_t<_mandala_bit>::pack_bit(data);}
  };
  class _field_enum_t : public _field_t<_mandala_enum>
  {
  public:
    uint pack(_field_msg *data) const
    {return _field_t<_mandala_enum>::pack_byte(data);}
  };

  typedef _field_float_t _field_float_test;
  class _test : public _field_float_test
  {
  public:
    uint v;

  };

//fields typedefs
#define MFIELD_VAR(aname) MGRP0.MGRP1.MGRP2.aname
#define MFIELD_TYPE(aname) ATPASTE2(_,aname)

#define MFIELD_IMPL(aindex,atype,aname,adescr,afullname,ashortname,afmt) \
  typedef _mandala_##atype MFIELD_TYPE(aname); MFIELD_TYPE(aname)

#define MFIELD(atype,aname,adescr,afmt,...) \
  MFIELD_IMPL(MFIELD_INDEX(aname),atype,aname,adescr,ASTRINGZ(MFIELD_VAR(aname)),aname,afmt) aname;

#define MFVECT(atype,aname,v1,v2,v3,adescr,afmt,...) \
  struct MFIELD_TYPE(aname) { public: \
    operator Vector<3,atype>()const{return Vector<3,atype>(v1,v2,v3);} \
    MFIELD_TYPE(aname) & operator=(const Vector<3,atype>& v){v1=v[0];v2=v[1];v3=v[2];return *this;} \
    MFIELD_IMPL(MFIELD_INDEX_VEC(aname,v1),atype,v1,adescr,ASTRINGZ(MFIELD_VAR(aname).v1),aname.v1,afmt) v1;\
    MFIELD_IMPL(MFIELD_INDEX_VEC(aname,v2),atype,v2,adescr,ASTRINGZ(MFIELD_VAR(aname).v2),aname.v2,afmt) v2;\
    MFIELD_IMPL(MFIELD_INDEX_VEC(aname,v3),atype,v3,adescr,ASTRINGZ(MFIELD_VAR(aname).v3),aname.v3,afmt) v3;\
  }aname;
#define MFVEC2(atype,aname,v1,v2,adescr,afmt,...) \
  struct MFIELD_TYPE(aname) { public: \
    operator Vector<2,atype>()const{return Vector<2,atype>(v1,v2);} \
    MFIELD_TYPE(aname) & operator=(const Vector<2,atype>& v){v1=v[0];v2=v[1];return *this;} \
    MFIELD_IMPL(MFIELD_INDEX_VEC(aname,v1),atype,v1,adescr,ASTRINGZ(MFIELD_VAR(aname).v1),aname.v1,afmt) v1;\
    MFIELD_IMPL(MFIELD_INDEX_VEC(aname,v2),atype,v2,adescr,ASTRINGZ(MFIELD_VAR(aname).v2),aname.v2,afmt) v2;\
  }aname;


//groups (_base) typedefs
#define MGRP_IMPL(aname,adescr) \
  struct ATPASTE3(_,aname,_base) : public _group { public: \
    MANDALA_INLINE const char* name()const{return ASTRINGZ(aname);} \
    MANDALA_INLINE const char* descr()const{return adescr;} \

#define MGRP0_BEGIN(adescr,...) MGRP_IMPL(MGRP0,adescr)
#define MGRP0_END               };
#define MGRP1_BEGIN(adescr,...) MGRP_IMPL(MGRP1,adescr)
#define MGRP1_END               };
#define MGRP2_BEGIN(adescr,...) MGRP_IMPL(MGRP2,adescr)
#define MGRP2_END               };

#include "MandalaFields.h"


//grp2
#define MGRP_IMPL(aname,adescr) \
  struct ATPASTE3(_,aname,_base_grp2) : public ATPASTE3(_,aname,_base) { public: \

#define MGRP0_BEGIN(adescr,...) MGRP_IMPL(MGRP0,adescr)
#define MGRP0_END               };
#define MGRP1_BEGIN(adescr,...) MGRP_IMPL(MGRP1,adescr)
#define MGRP1_END               };
#define MGRP2_BEGIN(adescr,...) \
  struct ATPASTE2(_,MGRP2) : public ATPASTE3(_,MGRP2,_base) { public: \
    MANDALA_INLINE _mandala_index index()const{return ATPASTE7(index_,MGRP0,_,MGRP1,_,MGRP2,_next)+1;} \
    MANDALA_INLINE _group * group(const _mandala_index idx) { return NULL; } \
    MANDALA_INLINE _group * next_group(const _group *g) { return NULL; } \
    void * ptr(const _mandala_index idx) { \
      switch(idx){ default: return NULL;

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
  MANDALA_INLINE _mandala_index index()const{return ATPASTE5(index_,MGRP0,_,MGRP1,_next)+1;}
#define MGRP1_END \
  }MGRP1;


#include "MandalaFields.h"

//grp0
#define MGRP_IMPL(aname,adescr) \
  class ATPASTE3(_,aname,_base_grp0) : public ATPASTE3(_,aname,_base_grp1) { public: \

#define MGRP0_BEGIN(adescr,...) \
  class ATPASTE2(_,MGRP0) : public ATPASTE3(_,MGRP0,_base_grp1) { public: \
  MANDALA_INLINE _mandala_index index()const{return ATPASTE3(index_,MGRP0,_next)+1;}
#define MGRP0_END \
  }MGRP0;

#include "MandalaFields.h"


//generic unnamed field types for small MCUs
  /*class _value : public _field_float_t
  {
  public:
    MANDALA_INLINE _mandala_float & operator=(const _mandala_float &v){setValue(v);return m_value;}
    MANDALA_INLINE _mandala_index index()const{return 0xFFFF;}
    MANDALA_INLINE const char* name()const{return "";}
    MANDALA_INLINE const char* descr()const{return "";}
    MANDALA_INLINE const char* shortname()const{return "";}
    MANDALA_INLINE _mf_type type(void)const{return mf_float;}
    MANDALA_INLINE _ext_fmt ext_fmt(void) {return fmt_float_f4;}
  };*/



// METHODS
public:

};
#undef MFIELD_TYPE
#undef MFIELD_VAR
#undef MFIELD_INDEX
#undef MFIELD_INDEX_VEC
//-----------------------------------------------------------------------------
extern _mandala mf;
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

#define MGRP2   power
MGRP2_BEGIN("Power measurements")
MFIELD(float, Ve,    "System battery voltage [v]", f2)
MFIELD(float, Vs,    "Servo battery voltage [v]", f2)
MFIELD(float, Vp,    "Payload battery voltage [v]", f2)
MFIELD(float, Vm,    "Engine battery voltage [v]", f2)
MFIELD(float, Ie,    "System current [A]", u001)
MFIELD(float, Is,    "Servo current [A]", u01)
MFIELD(float, Ip,    "Payload current [A]", u01)
MFIELD(float, Im,    "Engine current [A]", u1)
MGRP2_END
#undef MGRP2

#define MGRP2   engine
MGRP2_BEGIN("Engine sensors")
MFIELD(uint,  rpm,      "Engine RPM [1/min]", u2)
MFIELD(uint,  rpm_prop, "Prop RPM [1/min]", u2)
MFIELD(float, fuel,     "Fuel capacity [l]", f2)
MFIELD(float, frate,    "Fuel flow rate [l/h]", f2)
MFIELD(float, ET,       "Engine temperature [C]", u1)
MFIELD(float, EGT,      "Exhaust temperature [C]", u10)
MFIELD(float, OT,       "Oil temperature [C]", u1)
MFIELD(float, OP,       "Oil pressure [atm]", u01)
MGRP2_END
#undef MGRP2

#define MGRP2   temp
MGRP2_BEGIN("Temperature sensors")
MFIELD(float, AT,       "Ambient temperature [C]", s1)
MFIELD(float, RT,       "Room temperature [C]", s1)
MFIELD(float, MT,       "Modem temperature [C]", s1)
MGRP2_END
#undef MGRP2

#define MGRP2   datalink
MGRP2_BEGIN("Datalink radio sensors")
MFIELD(float, RSS,    "Modem signal strength [0..1]", f2)
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

#define MGRP2   controls
MGRP2_BEGIN("Controls feedback sensors")
MFIELD(float, s1,  "Controls sensor 1", f2)
MFIELD(float, s2,  "Controls sensor 2", f2)
MFIELD(float, s3,  "Controls sensor 3", f2)
MFIELD(float, s4,  "Controls sensor 4", f2)
MFIELD(float, s5,  "Controls sensor 5", f2)
MFIELD(float, s6,  "Controls sensor 6", f2)
MFIELD(float, s7,  "Controls sensor 7", f2)
MFIELD(float, s8,  "Controls sensor 8", f2)
MFIELD(float, s9,  "Controls sensor 9", f2)
MFIELD(float, s10, "Controls sensor 10", f2)
MFIELD(float, s11, "Controls sensor 11", f2)
MFIELD(float, s12, "Controls sensor 12", f2)
MFIELD(float, s13, "Controls sensor 13", f2)
MFIELD(float, s14, "Controls sensor 14", f2)
MFIELD(float, s15, "Controls sensor 15", f2)
MFIELD(float, s16, "Controls sensor 16", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   user
MGRP2_BEGIN("User sensors")
MFIELD(float, s1, "User sensor 1", f2)
MFIELD(float, s2, "User sensor 2", f2)
MFIELD(float, s3, "User sensor 3", f2)
MFIELD(float, s4, "User sensor 4", f2)
MFIELD(float, s5, "User sensor 5", f2)
MFIELD(float, s6, "User sensor 6", f2)
MFIELD(float, s7, "User sensor 7", f2)
MFIELD(float, s8, "User sensor 8", f2)
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
MFIELD(float,  tune,    "Engine tuning [0..1]", u001)
MFIELD(float,  vector,  "Thrust vector [-1..0..+1]", s001)
MFIELD(bit,    starter, "Engine starter on/off", )
MFIELD(bit,    rev,     "Engine reverse on/off", )
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

#define MGRP2   sw
MGRP2_BEGIN("Switches")
MFIELD(bit,  sw1,    "Switch 1 on/off", )
MFIELD(bit,  sw2,    "Switch 2 on/off", )
MFIELD(bit,  sw3,    "Switch 3 on/off", )
MFIELD(bit,  sw4,    "Switch 4 on/off", )
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
MFIELD(byte,  code,     "Error code", )
MFIELD(bit,  fatal,     "Fatal error/ok", )
MFIELD(bit,  power,     "Power supply error/ok", )
MFIELD(bit,  cas,       "CAS error", )
MFIELD(bit,  pstatic,   "Static pressure error/ok", )
MFIELD(bit,  gyro,      "IMU gyros bias", )
MFIELD(bit,  engine,    "Engine error/ok", )
MFIELD(bit,  rpm,       "RPM sensor error/ok", )
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
MFIELD(bit, sw1,       "Turret switch1 on/off",        )
MFIELD(bit, sw2,       "Turret switch2 on/off",        )
MFIELD(bit, sw3,       "Turret switch3 on/off",        )
MFIELD(bit, sw4,       "Turret switch4 on/off",        )
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
