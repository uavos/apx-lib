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
#ifndef DATALINK_H
#define DATALINK_H
//=============================================================================
#include "MandalaFields.h"
#include <inttypes.h>
#include <sys/types.h>
//=============================================================================
class Datalink
{
public:
  Datalink();

  uint pack_downstream(uint8_t *buf,uint sz);

private:
  //pack
  static uint pack_float_s(void *buf,const _mandala_float &v);
  static uint pack_float_u(void *buf,const _mandala_float &v);

  static uint pack_float_s1(void *buf,void *value_ptr);
  static uint pack_float_s01(void *buf,void *value_ptr);
  static uint pack_float_s001(void *buf,void *value_ptr);
  static uint pack_float_s10(void *buf,void *value_ptr);

  static uint pack_float_u1(void *buf,void *value_ptr);
  static uint pack_float_u01(void *buf,void *value_ptr);
  static uint pack_float_u001(void *buf,void *value_ptr);
  static uint pack_float_u10(void *buf,void *value_ptr);
  static uint pack_float_u100(void *buf,void *value_ptr);

  static uint pack_float_f2(void *buf,void *value_ptr);
  static uint pack_float_f4(void *buf,void *value_ptr);

  static uint pack_bit_(void *buf,void *value_ptr);
  static uint pack_enum_(void *buf,void *value_ptr);
  static uint pack_byte_u1(void *buf,void *value_ptr);
  static uint pack_uint_u4(void *buf,void *value_ptr);
  static uint pack_uint_u2(void *buf,void *value_ptr);

  //unpack
  static uint unpack_float_s1(const void *buf,void *value_ptr);
  static uint unpack_float_s01(const void *buf,void *value_ptr);
  static uint unpack_float_s001(const void *buf,void *value_ptr);
  static uint unpack_float_s10(const void *buf,void *value_ptr);

  static uint unpack_float_u1(const void *buf,void *value_ptr);
  static uint unpack_float_u01(const void *buf,void *value_ptr);
  static uint unpack_float_u001(const void *buf,void *value_ptr);
  static uint unpack_float_u10(const void *buf,void *value_ptr);
  static uint unpack_float_u100(const void *buf,void *value_ptr);

  static uint unpack_float_f2(const void *buf,void *value_ptr);
  static uint unpack_float_f4(const void *buf,void *value_ptr);

  static uint unpack_bit_(const void *buf,void *value_ptr);
  static uint unpack_enum_(const void *buf,void *value_ptr);
  static uint unpack_byte_u1(const void *buf,void *value_ptr);
  static uint unpack_uint_u4(const void *buf,void *value_ptr);
  static uint unpack_uint_u2(const void *buf,void *value_ptr);
};
//=============================================================================
#endif
