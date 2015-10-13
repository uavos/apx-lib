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
#ifndef MANDALA_FIELDS
#define MANDALA_FIELDS
#include "MandalaCore.h"
#include "preprocessor.h"
//=============================================================================
typedef _var_float _mandala_float;
typedef uint32_t   _mandala_uint;
typedef uint8_t    _mandala_byte;
typedef uint8_t    _mandala_bit;
typedef uint8_t    _mandala_enum;
//=============================================================================
template <class T=_mandala_float,int tindex=-1>
struct _mandala_field
{
public:
  _mandala_field(){m_value=0;flags_all=0;}
  inline T & operator=(const T &v){m_value=v;flags_all=1;return m_value;}
  inline operator T() const {return m_value;}

  inline const T & value() const {return m_value;}
  inline bool isChanged(){return flags.changed;}
  inline void save() {flags.changed=0;}
  enum{index=tindex};
protected:
  virtual inline const T & getValue(void) const {return m_value;}
  virtual inline void getValue(const T &v){m_value=v;}
private:
  T m_value;
  union{
    struct{
      uint8_t changed     :1;
    }flags;
    uint8_t flags_all;
  };
};
//=============================================================================
class _mandala_grp {};
class _mandala_field_vect {};
//=============================================================================
typedef struct {
  //typedefs
  typedef enum{
    dt_float=0,
    dt_u8,
    dt_u16,
    dt_u32,
    dt_s8,
    dt_s16,
    dt_s32,
    dt_bit
  }_dtype;
  typedef struct{
    uint8_t  nID;         //Node ID
  }_msg_id;
  typedef struct{
    uint16_t data_type  :4; //data type
    uint16_t data_fcnt  :2; //frame cnt
    uint16_t error      :2; //error code
    uint16_t command    :8; //command byte
  }_data_hdr;
  typedef struct{
    uint16_t field :13;   //field ID
    uint16_t dtype :3;    //data payload type (_dtype)
    union{
      uint32_t data32;
      uint8_t  data[4];
    }payload;
  }_data;
  //definitions
  _msg_id       id;     //1
  _data_hdr     hdr;    //2
  _data         data;   //2+4
}__attribute__((packed)) _mandala_msg;
//=============================================================================
class _mandala
{
public:
// index
#define MGRP0_BEGIN     ATPASTE2(index_,MGRP0),ATPASTE3(index_,MGRP0,_next)=(ATPASTE2(index_,MGRP0) &~((1<<11)-1))-1,
#define MGRP0_END       ATPASTE3(index_,MGRP0,_end)=(ATPASTE2(index_,MGRP0) &~((1<<11)-1))+(1<<11),
#define MGRP1_BEGIN     ATPASTE4(index_,MGRP0,_,MGRP1),ATPASTE5(index_,MGRP0,_,MGRP1,_next)=(ATPASTE4(index_,MGRP0,_,MGRP1) &~((1<<9)-1))-1,
#define MGRP1_END       ATPASTE5(index_,MGRP0,_,MGRP1,_end)=(ATPASTE4(index_,MGRP0,_,MGRP1) &~((1<<9)-1))+(1<<9),
#define MGRP2_BEGIN     ATPASTE6(index_,MGRP0,_,MGRP1,_,MGRP2),ATPASTE7(index_,MGRP0,_,MGRP1,_,MGRP2,_next)=(ATPASTE6(index_,MGRP0,_,MGRP1,_,MGRP2) &~((1<<4)-1))-1,
#define MGRP2_END       ATPASTE7(index_,MGRP0,_,MGRP1,_,MGRP2,_end)=(ATPASTE6(index_,MGRP0,_,MGRP1,_,MGRP2) &~((1<<4)-1))+(1<<4),

#define MFIELD_INDEX(aname) ATPASTE8(index_,MGRP0,_,MGRP1,_,MGRP2,_,aname)
#define MFIELD_INDEX_VEC(aname,vname) ATPASTE10(index_,MGRP0,_,MGRP1,_,MGRP2,_,aname,_,vname)
#define MFIELD(atype,aname,...) MFIELD_INDEX(aname),
#define MFVECT(atype,aname,v1,v2,v3,...) \
  MFIELD_INDEX_VEC(aname,v1), \
  MFIELD_INDEX_VEC(aname,v2), \
  MFIELD_INDEX_VEC(aname,v3),
#define MFVEC2(atype,aname,v1,v2,...) \
  MFIELD_INDEX_VEC(aname,v1), \
  MFIELD_INDEX_VEC(aname,v2),
enum{
#include "MandalaFields.h"
};

//fields typedefs
#define MFIELD_TYPE(aname) ATPASTE8(_,MGRP0,_,MGRP1,_,MGRP2,_,aname)
#define MFIELD(atype,aname,...)         typedef _mandala_field<_mandala_##atype,MFIELD_INDEX(aname)> MFIELD_TYPE(aname);
#define MFVECT(atype,aname,v1,v2,v3,...) \
  class MFIELD_TYPE(aname) : public _mandala_field_vect { public: \
    operator Vector<3,atype>()const{return Vector<3,atype>(v1,v2,v3);} \
    MFIELD_TYPE(aname) & operator=(const Vector<3,atype>& v){v1=v[0];v2=v[1];v3=v[2];return *this;} \
    _mandala_field<_mandala_##atype,MFIELD_INDEX_VEC(aname,v1)> v1; \
    _mandala_field<_mandala_##atype,MFIELD_INDEX_VEC(aname,v2)> v2; \
    _mandala_field<_mandala_##atype,MFIELD_INDEX_VEC(aname,v3)> v3; \
  };
#define MFVEC2(atype,aname,v1,v2,...) \
  class MFIELD_TYPE(aname) : public _mandala_field_vect { public: \
    operator Vector<2,atype>()const{return Vector<2,atype>(v1,v2);} \
    MFIELD_TYPE(aname) & operator=(const Vector<2,atype>& v){v1=v[0];v2=v[1];return *this;} \
    _mandala_field<_mandala_##atype,MFIELD_INDEX_VEC(aname,v1)> v1; \
    _mandala_field<_mandala_##atype,MFIELD_INDEX_VEC(aname,v2)> v2; \
  };
#include "MandalaFields.h"


// struct
#define MGRP0_BEGIN     class ATPASTE2(_,MGRP0) : public _mandala_grp { public:
#define MGRP0_END       } MGRP0;
#define MGRP1_BEGIN     class ATPASTE4(_,MGRP0,_,MGRP1) : public _mandala_grp { public:
#define MGRP1_END       } MGRP1;
#define MGRP2_BEGIN     class ATPASTE6(_,MGRP0,_,MGRP1,_,MGRP2) : public _mandala_grp { public:
#define MGRP2_END       } MGRP2;
#define MFIELD(atype,aname,...)           MFIELD_TYPE(aname) aname;
#define MFVECT(atype,aname,v1,v2,v3,...)  MFIELD_TYPE(aname) aname;
#define MFVEC2(atype,aname,v1,v2,...)     MFIELD_TYPE(aname) aname;
#include "MandalaFields.h"

#define MFIELD_VAR(aname) MGRP0.MGRP1.MGRP2.aname
// METHODS
public:
  _mandala_float value(const uint index) const
  {
    #define MFIELD(atype,aname,...) case MFIELD_INDEX(aname): return MFIELD_VAR(aname).value();
    #define MFVECT(atype,aname,v1,v2,v3,...) \
      case MFIELD_INDEX_VEC(aname,v1): return MFIELD_VAR(aname).v1.value(); \
      case MFIELD_INDEX_VEC(aname,v2): return MFIELD_VAR(aname).v2.value(); \
      case MFIELD_INDEX_VEC(aname,v3): return MFIELD_VAR(aname).v3.value();
    #define MFVEC2(atype,aname,v1,v2,v3,...) \
      case MFIELD_INDEX_VEC(aname,v1): return MFIELD_VAR(aname).v1.value(); \
      case MFIELD_INDEX_VEC(aname,v2): return MFIELD_VAR(aname).v2.value();
    switch(index){
      #include "MandalaFields.h"
    }
    return 0;
  }
  void setValue(const uint index,const _mandala_float &v)
  {
    #define MFIELD(atype,aname,...) case MFIELD_INDEX(aname): MFIELD_VAR(aname)=v;break;
    #define MFVECT(atype,aname,v1,v2,v3,...) \
      case MFIELD_INDEX_VEC(aname,v1): MFIELD_VAR(aname).v1=v;break; \
      case MFIELD_INDEX_VEC(aname,v2): MFIELD_VAR(aname).v2=v;break; \
      case MFIELD_INDEX_VEC(aname,v3): MFIELD_VAR(aname).v3=v;break;
    #define MFVEC2(atype,aname,v1,v2,v3,...) \
      case MFIELD_INDEX_VEC(aname,v1): MFIELD_VAR(aname).v1=v;break; \
      case MFIELD_INDEX_VEC(aname,v2): MFIELD_VAR(aname).v2=v;break;
    switch(index){
      #include "MandalaFields.h"
    }
  }
  void setValue(const uint index,const uint &v)
  {
    #define MFIELD(atype,aname,...) case MFIELD_INDEX(aname): MFIELD_VAR(aname)=v;break;
    #define MFVECT(atype,aname,v1,v2,v3,...) \
      case MFIELD_INDEX_VEC(aname,v1): MFIELD_VAR(aname).v1=v;break; \
      case MFIELD_INDEX_VEC(aname,v2): MFIELD_VAR(aname).v2=v;break; \
      case MFIELD_INDEX_VEC(aname,v3): MFIELD_VAR(aname).v3=v;break;
    #define MFVEC2(atype,aname,v1,v2,v3,...) \
      case MFIELD_INDEX_VEC(aname,v1): MFIELD_VAR(aname).v1=v;break; \
      case MFIELD_INDEX_VEC(aname,v2): MFIELD_VAR(aname).v2=v;break;
    switch(index){
      #include "MandalaFields.h"
    }
  }
  const char * name(const uint index) const
  {
    #define MFIELD(atype,aname,...) case MFIELD_INDEX(aname): return ASTRINGZ(MFIELD_VAR(aname));
    #define MFVECT(atype,aname,v1,v2,v3,...) \
      case MFIELD_INDEX_VEC(aname,v1): return ASTRINGZ(MFIELD_VAR(aname).v1); \
      case MFIELD_INDEX_VEC(aname,v2): return ASTRINGZ(MFIELD_VAR(aname).v2); \
      case MFIELD_INDEX_VEC(aname,v3): return ASTRINGZ(MFIELD_VAR(aname).v3);
    #define MFVEC2(atype,aname,v1,v2,...) \
      case MFIELD_INDEX_VEC(aname,v1): return ASTRINGZ(MFIELD_VAR(aname).v1); \
      case MFIELD_INDEX_VEC(aname,v2): return ASTRINGZ(MFIELD_VAR(aname).v2);
    switch(index){
      #include "MandalaFields.h"
    }
    return "";
  }
  const char * shortname(const uint index) const
  {
    #define MFIELD(atype,aname,...) case MFIELD_INDEX(aname): return #aname;
    #define MFVECT(atype,aname,v1,v2,v3,...) \
      case MFIELD_INDEX_VEC(aname,v1): return #aname; \
      case MFIELD_INDEX_VEC(aname,v2): return #aname; \
      case MFIELD_INDEX_VEC(aname,v3): return #aname;
    #define MFVEC2(atype,aname,v1,v2,...) \
      case MFIELD_INDEX_VEC(aname,v1): return #aname; \
      case MFIELD_INDEX_VEC(aname,v2): return #aname;
    switch(index){
      #include "MandalaFields.h"
    }
    return "";
  }
  const char * descr(const uint index) const
  {
    #define MFIELD(atype,aname,adescr,...) case MFIELD_INDEX(aname): return adescr;
    #define MFVECT(atype,aname,v1,v2,v3,adescr,...) \
      case MFIELD_INDEX_VEC(aname,v1): return adescr; \
      case MFIELD_INDEX_VEC(aname,v2): return adescr; \
      case MFIELD_INDEX_VEC(aname,v3): return adescr;
    #define MFVEC2(atype,aname,v1,v2,adescr,...) \
      case MFIELD_INDEX_VEC(aname,v1): return adescr; \
      case MFIELD_INDEX_VEC(aname,v2): return adescr;
    switch(index){
      #include "MandalaFields.h"
    }
    return "";
  }
  uint index_next(const uint index) const
  {
    #define MFIELD(atype,aname,...) return MFIELD_INDEX(aname); case MFIELD_INDEX(aname):
    #define MFVECT(atype,aname,v1,v2,v3,...) \
      return MFIELD_INDEX_VEC(aname,v1); case MFIELD_INDEX_VEC(aname,v1): \
      return MFIELD_INDEX_VEC(aname,v2); case MFIELD_INDEX_VEC(aname,v2): \
      return MFIELD_INDEX_VEC(aname,v3); case MFIELD_INDEX_VEC(aname,v3):
    #define MFVEC2(atype,aname,v1,v2,...) \
      return MFIELD_INDEX_VEC(aname,v1); case MFIELD_INDEX_VEC(aname,v1): \
      return MFIELD_INDEX_VEC(aname,v2); case MFIELD_INDEX_VEC(aname,v2):
    switch(index){
      case -1:
      #include "MandalaFields.h"
      return 0;
    }
    return 0;
  }
  static inline bool is_grp0(const uint index) {return (index&((1<<11)-1))==0;}
  static inline bool is_grp1(const uint index) {return (index&((1<<9)-1))==0;}
  static inline bool is_grp2(const uint index) {return (index&((1<<4)-1))==0;}

};
#undef MFIELD_TYPE
#undef MFIELD_VAR
#undef MFIELD_INDEX
#undef MFIELD_INDEX_VEC
//-----------------------------------------------------------------------------
//=============================================================================
#endif
//=============================================================================
// LIST INCLUDE PART
//=============================================================================
//=============================================================================
#ifndef MGRP0_BEGIN
#define MGRP0_BEGIN
#endif
#ifndef MGRP0_END
#define MGRP0_END
#endif
#ifndef MGRP1_BEGIN
#define MGRP1_BEGIN
#endif
#ifndef MGRP1_END
#define MGRP1_END
#endif
#ifndef MGRP2_BEGIN
#define MGRP2_BEGIN
#endif
#ifndef MGRP2_END
#define MGRP2_END
#endif
#ifndef MFIELD
#define MFIELD(...)
#endif
#ifndef MFVECT
#define MFVECT(atype,aname,v1,v2,v3,...) \
  MFIELD(atype,v1,__VA_ARGS__) \
  MFIELD(atype,v2,__VA_ARGS__) \
  MFIELD(atype,v3,__VA_ARGS__)
#endif
#ifndef MFVEC2
#define MFVEC2(atype,aname,v1,v2,...) \
  MFIELD(atype,v1,__VA_ARGS__) \
  MFIELD(atype,v2,__VA_ARGS__)
#endif
#ifndef MFENUM
#define MFENUM(...)
#endif
//=============================================================================
#define MGRP0   vehicle
MGRP0_BEGIN
//-------------------------
#define MGRP1   sensor
MGRP1_BEGIN

#define MGRP2   imu
MGRP2_BEGIN
MFVECT(float,  acc,x,y,z, "Acceleration [m/s2]", f2)
MFVECT(float,  gyro,p,q,r,"Angular rate [deg/s]", f2)
MFVECT(float,  mag,x,y,z, "Magnetic field [a.u.]", f2)
MFIELD(float,  temp,      "IMU Temperature [C]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   gps
MGRP2_BEGIN
MFVECT(float,  pos,lat,lon,hmsl, "GPS position [deg,deg,m]", f4)
MFVECT(float,  vel,N,E,D,        "GPS velocity [m/s]", f2)
MFIELD(uint,   UTC, "GPS UTC Time from 1970 1st Jan [sec]", u4)
MFIELD(byte,   SV,  "GPS Satellites visible [number]", u1)
MFIELD(byte,   SU,  "GPS Satellites used [number]", u1)
MGRP2_END
#undef MGRP2

#define MGRP2   air
MGRP2_BEGIN
MFIELD(float, pt,    "Airspeed [m/s]", f2)
MFIELD(float, ps,    "Barometric altitude [m]", f4)
MFIELD(float, vario, "Barometric variometer [m/s]", f2)
MFIELD(float, slip,  "Slip [deg]", f2)
MFIELD(float, aoa,   "Angle of attack [deg]", f2)
MFIELD(float, buo,   "Blimp ballonet pressure [kPa]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   range
MGRP2_BEGIN
MFIELD(float, ultrasonic, "Ultrasonic altimeter [m]", f2)
MFIELD(float, laser,      "Laser altimeter [m]", f2)
MFIELD(float, radio,      "Radio altimeter [m]", f4)
MFIELD(float, proximity,  "Proximity sensor [m]", f2)
MFIELD(float, touch,      "Gear force sensor [N]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   lps
MGRP2_BEGIN
MFVECT(float, pos,x,y,z, "Local position sensor [m]", f2)
MFVECT(float, vel,x,y,z, "Local velocity sensor [m/s]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   ils
MGRP2_BEGIN
MFIELD(float, HDG,    "ILS heading to VOR1 [deg]", f2)
MFIELD(uint,  DME,    "ILS distance to VOR1 [m]", u2)
MFIELD(float, RSS,    "ILS signal strength [0..1]", f2)
MFVEC2(float, offset,HDG,altitude, "ILS offset [deg,m]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   platform
MGRP2_BEGIN
MFVECT(float,  pos,lat,lon,hmsl,   "Platform position [deg,deg,m]", f4)
MFVECT(float,  vel,N,E,D,          "Platform velocity [m/s]", f2)
MFVECT(float,  att,roll,pitch,yaw, "Platform attitude [deg]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   radar
MGRP2_BEGIN
MFIELD(byte,  id,     "Radar target ID", u1)
MFIELD(float, HDG,    "Radar heading to target [deg]", f2)
MFIELD(uint,  DME,    "Radar distance to target [m]", u2)
MFIELD(float, vel,    "Radar target velocity [m/s]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   power
MGRP2_BEGIN
MFIELD(float, Ve,    "system battery voltage [v]", f2)
MFIELD(float, Vs,    "servo battery voltage [v]", f2)
MFIELD(float, Vp,    "payload battery voltage [v]", f2)
MFIELD(float, Vm,    "engine battery voltage [v]", f2)
MFIELD(float, Ie,    "system current [A]", u001)
MFIELD(float, Is,    "servo current [A]", u01)
MFIELD(float, Ip,    "payload current [A]", u01)
MFIELD(float, Im,    "engine current [A]", u1)
MGRP2_END
#undef MGRP2

#define MGRP2   engine
MGRP2_BEGIN
MFIELD(uint,  rpm,      "engine RPM [1/min]", u2)
MFIELD(uint,  prop_rpm, "prop RPM [1/min]", u2)
MFIELD(float, fuel,     "fuel capacity [l]", f2)
MFIELD(float, frate,    "fuel flow rate [l/h]", f2)
MFIELD(float, ET,       "engine temperature [C]", u1)
MFIELD(float, EGT,      "exhaust gas temperature [C]", u10)
MFIELD(float, OT,       "oil temperature [C]", u1)
MFIELD(float, OP,       "oil pressure [atm]", u01)
MGRP2_END
#undef MGRP2

#define MGRP2   temp
MGRP2_BEGIN
MFIELD(float, AT,       "ambient temperature [C]", s1)
MFIELD(float, RT,       "room temperature [C]", s1)
MFIELD(float, MT,       "modem temperature [C]", s1)
MGRP2_END
#undef MGRP2

#define MGRP2   datalink
MGRP2_BEGIN
MFIELD(float, RSS,    "Modem signal strength [0..1]", f2)
MFIELD(float, HDG,    "Modem heading to transmitter [deg]", f2)
MFIELD(uint,  DME,    "Modem distance to transmitter [m]", u4)
MGRP2_END
#undef MGRP2

#define MGRP2   pilot
MGRP2_BEGIN
MFIELD(byte,  override, "RC override [0..255]", u1)
MFIELD(float, roll,     "RC roll [-1..0..+1]", s001)
MFIELD(float, pitch,    "RC pitch [-1..0..+1]", s001)
MFIELD(float, throttle, "RC throttle [0..1]", s001)
MFIELD(float, yaw,      "RC yaw [-1..0..+1]", s001)
MGRP2_END
#undef MGRP2

#define MGRP2   steering
MGRP2_BEGIN
MFVEC2(float, vel,left,right,     "Steering velocity [m/s]", f2)
MFIELD(float, HDG,                      "Steering heading [deg]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   controls
MGRP2_BEGIN
MFIELD(float, s1, "Controls sensor 1", f2)
MFIELD(float, s2, "Controls sensor 2", f2)
MFIELD(float, s3, "Controls sensor 3", f2)
MFIELD(float, s4, "Controls sensor 4", f2)
MFIELD(float, s5, "Controls sensor 5", f2)
MFIELD(float, s6, "Controls sensor 6", f2)
MFIELD(float, s7, "Controls sensor 7", f2)
MFIELD(float, s8, "Controls sensor 8", f2)
MFIELD(float, s9, "Controls sensor 9", f2)
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
MGRP2_BEGIN
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
MGRP1_BEGIN

#define MGRP2   stability
MGRP2_BEGIN
MFIELD(float,  ail,    "ailerons [-1..0..+1]", s001)
MFIELD(float,  elv,    "elevator [-1..0..+1]", s001)
MFIELD(float,  rud,    "rudder [-1..0..+1]", s001)
MFIELD(float,  collective, "collective pitch [-1..0..+1]", s001)
MFIELD(float,  steering,   "steering [-1..0..+1]", s001)
MGRP2_END
#undef MGRP2

#define MGRP2   engine
MGRP2_BEGIN
MFIELD(float,  thr,     "throttle [0..1]", u001)
MFIELD(float,  prop,    "prop pitch [-1..0..+1]", s001)
MFIELD(float,  mix,     "mixture [0..1]", u001)
MFIELD(float,  tune,    "engine tuning [0..1]", u001)
MFIELD(float,  vector,  "thrust vector [-1..0..+1]", s001)
MFIELD(bit,    starter, "engine starter on/off", )
MFIELD(bit,    rev,     "engine reverse on/off", )
MGRP2_END
#undef MGRP2

#define MGRP2   wing
MGRP2_BEGIN
MFIELD(float,  flaps,   "flaps [0..1]", u001)
MFIELD(float,  airbrk,  "airbrakes [0..1]", u001)
MFIELD(float,  slats,   "wing slats [0..1]", u001)
MFIELD(float,  sweep,   "wing sweep [-1..0..+1]", s001)
MFIELD(float,  buoyancy,"buoyancy [-1..0..+1]", s001)
MGRP2_END
#undef MGRP2

#define MGRP2   brakes
MGRP2_BEGIN
MFIELD(float,  brake,   "brake [0..1]", u001)
MFIELD(float,  brakeL,  "left brake [0..1]", u001)
MFIELD(float,  brakeR,  "right brake [0..1]", u001)
MGRP2_END
#undef MGRP2

#define MGRP2   ers
MGRP2_BEGIN
MFIELD(bit,  launch,    "ERS on/off", )
MFIELD(bit,  rel,       "Parachute released/locked", )
MGRP2_END
#undef MGRP2

#define MGRP2   gear
MGRP2_BEGIN
MFIELD(bit,  retract,    "Landing gear retracted/extracted", )
MGRP2_END
#undef MGRP2

#define MGRP2   fuel
MGRP2_BEGIN
MFIELD(bit,  pump,    "Fuel pump on/off", )
MFIELD(bit,  xfeed,   "Crossfeed on/off", )
MGRP2_END
#undef MGRP2

#define MGRP2   power
MGRP2_BEGIN
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
MGRP2_BEGIN
MFIELD(bit,  beacon,    "Beacon light on/off", )
MFIELD(bit,  landing,   "Landing lights on/off", )
MFIELD(bit,  nav,       "Navigation lights on/off", )
MFIELD(bit,  strobe,    "Strobe light on/off", )
MFIELD(bit,  taxi,      "Taxi lights on/off", )
MGRP2_END
#undef MGRP2

#define MGRP2   doors
MGRP2_BEGIN
MFIELD(bit,  main,    "Main door open/locked", )
MFIELD(bit,  drop,    "Drop-off open/locked", )
MGRP2_END
#undef MGRP2

#define MGRP2   sw
MGRP2_BEGIN
MFIELD(bit,  sw1,    "switch 1 on/off", )
MFIELD(bit,  sw2,    "switch 2 on/off", )
MFIELD(bit,  sw3,    "switch 3 on/off", )
MFIELD(bit,  sw4,    "switch 4 on/off", )
MGRP2_END
#undef MGRP2

MGRP1_END
#undef MGRP1
//-------------------------
#define MGRP1   state
MGRP1_BEGIN

#define MGRP2   ahrs
MGRP2_BEGIN
MFVECT(float, att,roll,pitch,yaw,    "attitude [deg]", f2)
MFIELD(float, lat,  "Latitude [deg]", f4)
MFIELD(float, lon,  "Longitude [deg]", f4)
MFVEC2(float, NE,N,E, "Local position [m]", f4)
MFVEC2(float, vel,N,E, "Local velocity [m/s]", f2)
MFIELD(float, course,  "Moving direction [deg]", f2)
MFIELD(float, gSpeed,  "Ground speed [m/s]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   flight
MGRP2_BEGIN
MFIELD(float, airspeed, "Airspeed [m/s]", f2)
MFIELD(float, altitude, "Altitude [m]", f4)
MFIELD(float, vspeed,   "Vertical speed [m/s]", f2)
MFIELD(float, ldratio,  "Glide ratio [Lift/Drag]", f2)
MFIELD(float, venergy,  "Compensated variometer [m/s]", f2)
MFIELD(float, agl,      "Above Ground Level altitude [m]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   status
MGRP2_BEGIN
MFIELD(bit,  rc,     "RC on/off", )
MFIELD(bit,  gps,    "GPS available/lost", )
MFIELD(bit,  agl,    "AGL available/off", )
MFIELD(bit,  uplink, "Uplink available/lost", )
MFIELD(bit,  touch,  "Landing gear touchdown/floating", )
MFIELD(float,corr,   "Correlator output [K]", f2)
MFIELD(float,stab,   "Stability [0..1]", u001)
MGRP2_END
#undef MGRP2

#define MGRP2   error
MGRP2_BEGIN
MFIELD(byte,  code,     "error code", u1)
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
MGRP2_BEGIN
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
MGRP2_BEGIN
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
MFIELD(byte,  stage,     "maneuver stage", u1)
MFIELD(byte,  wpidx,     "current waypoint [0..255]", u1)
MFIELD(byte,  rwidx,     "current runway [0..255]", u1)
MFIELD(byte,  twidx,     "current taxiway [0..255]", u1)
MFIELD(byte,  piidx,     "current point of interest [0..255]", u1)
MFIELD(enum,  type,      "Mission maneuver type", )
MFENUM(type,   hdg,     "Heading navigation",      0)
MFENUM(type,   line,    "Line navigation",         1)
MGRP2_END
#undef MGRP2

#define MGRP2   cmode
MGRP2_BEGIN
MFIELD(bit,  dlhd,      "High precision downstream on/off", )
MFIELD(bit,  thrcut,    "Throttle cut on/off", )
MFIELD(bit,  throvr,    "Throttle override on/off", )
MFIELD(bit,  hover,     "Stabilization mode hover/run", )
MFIELD(bit,  ahrs,      "AHRS mode inertial/gps", )
MGRP2_END
#undef MGRP2

#define MGRP2   nav
MGRP2_BEGIN
MFIELD(float, turnR,     "Current circle radius [m]", f2)
MFIELD(float, delta,     "General delta (depends on mode) [m]", f2)
MFIELD(uint,  loops,     "Number of remaining turns or loops [0..255]", u1)
MFIELD(float, rwAdj,     "runway displacement adjust [m]", s1)
MFIELD(float, rwDelta,   "Runway alignment [m]", f2)
MFIELD(float, rwDV,      "Runway alignment velocity [m/s]", f2)
MFVEC2(float, dXY,x,y,   "bodyframe delta [m]", f2)
MFVEC2(float, vXY,x,y,   "bodyframe velocity [m/s]", f2)
MGRP2_END
#undef MGRP2

#define MGRP2   waypoint
MGRP2_BEGIN
MFIELD(float, DME,    "Distance to waypoint [m]", f2)
MFIELD(float, HDG,    "Current waypoint heading [deg]", f2)
MFIELD(float, tgHDG,  "Current tangent heading [deg]", f2)
MFIELD(uint,  ETA,    "Estimated time of arrival [s]", u4)
MGRP2_END
#undef MGRP2

#define MGRP2   ils
MGRP2_BEGIN
MFIELD(bit, armed,    "ILS armed/off", )
MFIELD(bit, approach, "ILS approach available/lost", )
MFIELD(bit, offset,   "ILS offset available/lost", )
MFIELD(bit, platform, "ILS platform available/lost", )
MGRP2_END
#undef MGRP2

#define MGRP2   home
MGRP2_BEGIN
MFVECT(float,  pos,lat,lon,hmsl,"Home global position [deg,deg,m]", f4)
MFIELD(float,  altps,           "barometric altitude on ground level [m]", s2)
MGRP2_END
#undef MGRP2

#define MGRP2   wind
MGRP2_BEGIN
MFIELD(float,  speed,   "wind speed [m/s]", u01)
MFIELD(float,  HDG,     "wind direction to [deg]", f2)
MFIELD(float,  cas2tas, "CAS to TAS multiplier [K]", u001)
MGRP2_END
#undef MGRP2

#define MGRP2   downlink
MGRP2_BEGIN
MFIELD(uint,  period,    "downlink period [ms]", u2)
MFIELD(uint,  timestamp, "downlink timestamp [ms]", u4)
MGRP2_END
#undef MGRP2

MGRP1_END
#undef MGRP1
//-------------------------
#define MGRP1   info
MGRP1_BEGIN

#define MGRP2   ground
MGRP2_BEGIN
MFIELD(float, RSS, "GCU modem signal strength [0..1]", f2)
MFIELD(float, Ve,  "GCU system battery voltage [v]", f2)
MFIELD(float, MT,  "GCU modem temperature [C]", s1)
MGRP2_END
#undef MGRP2

#define MGRP2   meteo
MGRP2_BEGIN
MFIELD(float,  windSpd, "wind speed [m/s]", u01)
MFIELD(float,  windHdg, "wind direction to [deg]", f2)
MFIELD(float,  temp,    "Ground outside air temperature [C]", s1)
MFIELD(float,  altps,   "barometric altitude on ground level [m]", s2)
MGRP2_END
#undef MGRP2


MGRP1_END
#undef MGRP1

MGRP0_END
#undef MGRP0
//-----------------------------------------------------------------------------
#define MGRP0   payload
MGRP0_BEGIN
#define MGRP1   cam
MGRP1_BEGIN
#define MGRP2   imu
MGRP2_BEGIN
//MFIELD(float, temp,    "Airspeed [m/s]",                       f2)
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
#undef MFIELD
#undef MFVECT
#undef MFVEC2
#undef MFENUM
