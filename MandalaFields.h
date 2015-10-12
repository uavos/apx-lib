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
template <class T=_var_float,int tindex=0>
class _mandala_field
{
public:
  _mandala_field(){m_value=0;flags_all=0;}
  inline T & operator=(const T &v){m_value=v;flags_all=1;return m_value;}
  inline operator T() const {return m_value;}
  inline const T & value() const {return m_value;}

  inline bool isChanged(){return flags.changed;}
  inline void save() {flags.changed=0;}
  enum{index=tindex};
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
class _mandala_grp
{
public:
};
//=============================================================================
//=============================================================================
class _mandala_fields
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
#define MFIELD(atype,aname,...) MFIELD_INDEX(aname),
enum{
#include "MandalaFields.h"
};

// struct
#define MGRP0_BEGIN     class ATPASTE2(_,MGRP0) : public _mandala_grp { public:
#define MGRP0_END       } MGRP0;
#define MGRP1_BEGIN     class ATPASTE4(_,MGRP0,_,MGRP1) : public _mandala_grp { public:
#define MGRP1_END       } MGRP1;
#define MGRP2_BEGIN     class ATPASTE6(_,MGRP0,_,MGRP1,_,MGRP2) : public _mandala_grp { public:
#define MGRP2_END       } MGRP2;
#define MFIELD(atype,aname,...)         _mandala_field<_var_##atype,MFIELD_INDEX(aname)> aname;
#include "MandalaFields.h"

#define MFIELD_VAR(aname) MGRP0.MGRP1.MGRP2.aname
// METHODS
public:
  _var_float value(const uint index) const
  {
    #define MFIELD(atype,aname,...) case MFIELD_INDEX(aname): return MFIELD_VAR(aname).value();
    switch(index){
      #include "MandalaFields.h"
    }
    return 0;
  }
  void setValue(const uint index,const _var_float &v)
  {
    #define MFIELD(atype,aname,...) case MFIELD_INDEX(aname): MFIELD_VAR(aname)=v;break;
    switch(index){
      #include "MandalaFields.h"
    }
  }
  void setValue(const uint index,const uint &v)
  {
    #define MFIELD(atype,aname,...) case MFIELD_INDEX(aname): MFIELD_VAR(aname)=v;break;
    switch(index){
      #include "MandalaFields.h"
    }
  }
  const char * name(const uint index) const
  {
    #define MFIELD(atype,aname,...) case MFIELD_INDEX(aname): return ASTRINGZ(MFIELD_VAR(aname));
    switch(index){
      #include "MandalaFields.h"
    }
    return "";
  }
  const char * descr(const uint index) const
  {
    #define MFIELD(atype,aname,adescr,...) case MFIELD_INDEX(aname): return adescr;
    switch(index){
      #include "MandalaFields.h"
    }
    return "";
  }
  uint index_next(const uint index) const
  {
    #define MFIELD(atype,aname,...) return MFIELD_INDEX(aname); case MFIELD_INDEX(aname):
    switch(index){
      case -1:
      #include "MandalaFields.h"
      return 0;
    }
    return 0;
  }
};
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
#ifndef MFIELD_VECT
#define MFIELD_VECT(atype,aname,v1,v2,v3,...) \
  MFIELD(float,v1,__VA_ARGS__) \
  MFIELD(float,v2,__VA_ARGS__) \
  MFIELD(float,v3,__VA_ARGS__)
#endif
//=============================================================================
#define MGRP0   vehicle
MGRP0_BEGIN
//-------------------------
#define MGRP1   sensor
MGRP1_BEGIN
#define MGRP2   imu
MGRP2_BEGIN
MFIELD_VECT(vect,  acc,ax,ay,az,    "Airspeed [m/s]",                    f2)
MFIELD_VECT(vect,  gyro,p,q,r,    "Airspeed [m/s]",                    f2)
MFIELD(float, temp,    "Airspeed [m/s]",                       f2)
MGRP2_END
#undef MGRP2
#define MGRP2   air
MGRP2_BEGIN
MFIELD(float, airspeed,    "Airspeed [m/s]",                       f2)
MFIELD(float, altps,       "Barometric altitude [m]",              0)
MFIELD(float, vario,       "Barometric variometer [m/s]",          0)
MGRP2_END
#undef MGRP2
#define MGRP2   agl
MGRP2_BEGIN
MFIELD(float, ultrasonic,    "Airspeed [m/s]",                       f2)
MFIELD(float, laser,       "Barometric altitude [m]",              0)
MGRP2_END
#undef MGRP2
MGRP1_END
#undef MGRP1
//-------------------------
#define MGRP1   control
MGRP1_BEGIN
#define MGRP2   stability
MGRP2_BEGIN
MFIELD(uint,  ail,    "Airspeed [m/s]",                       f2)
MFIELD(float, elv,       "Barometric altitude [m]",              0)
MFIELD(float, rud,       "Barometric variometer [m/s]",          0)
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
MFIELD(float, temp,    "Airspeed [m/s]",                       f2)
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
#undef MFIELD_VECT
