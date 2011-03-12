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
#ifndef MANDALAMATH_H
#define MANDALAMATH_H
#include "MandalaTypes.h"
#include "MandalaVars.h"
#include <string.h>
#include <sys/types.h>
//=============================================================================
template<class T>
class vt
{
protected:
  T v;
public:
  vt(){v=0;bytes=0;span=0;name="";descr="";index=0;}
  //vt & operator=(const T &value) {v=value;return(*this);}
  //vt & operator+(const T &value) const {return v+value;}
  //vt & operator-(const T &value) const {return v-value;}
  //vt & operator*(const T &value) const {return v*value;}
  //vt & operator*(const T value) const {T rv=(*this);return rv*value;}
  //vt & operator+=(const T &value) {v+=value;return(*this);}
  //vt & operator-=(const T &value) {v-=value;return(*this);}
  //vt & operator*=(const T &value) {v*=value;return(*this);}
  //vt & operator/=(const T &value) {v/=value;return(*this);}
  //vt & operator&=(const T &value) {v&=value;return(*this);}
  //vt & operator|=(const T &value) {v|=value;return(*this);}
  //vt & operator++(const T &value) {v++;return(*this);}
  //bool operator==(const T &value) {return v==value;}
  operator T(void) const {return (T)v;}

  const char *name;
  const char *descr;
  uint index,bytes;
  T span;
};
//=============================================================================
class Flag: public vt<bool>
{
protected:
public:
  Flag() :vt<bool>() {mask=0;}
  Flag & operator=(const uint value){v=value;return(*this);}
  uint mask;
};
//=============================================================================
class Uint: public vt<uint>
{
protected:
public:
  Uint & operator=(const uint value) {v=value;return(*this);}
  Uint & operator&=(const uint value) {v&=value;return(*this);}
  Uint & operator|=(const uint value) {v|=value;return(*this);}
  Uint & operator++() {++v;return(*this);}
  Uint   operator++(int unused) {Uint rv=(*this);v++;return(rv);}
  Uint & operator--() {--v;return(*this);}
  Uint   operator--(int unused) {Uint rv=(*this);v--;return(rv);}
};
//=============================================================================
class Float: public vt<double>
{
protected:
public:
  Float & operator=(const double value) {v=value;return(*this);}
  //Float & operator*(const double value) const {Float rv=(*this);return rv*value;}
};
//=============================================================================
typedef Float _vt_double;
typedef Vect _vt_Vect;
typedef Uint _vt_uint;
typedef Flag _vt_flag;
//=============================================================================
#endif