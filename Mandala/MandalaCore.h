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
#include <math.h>
#include <string.h>

#include <inttypes.h>
#include <sys/types.h>

#include "MatrixMath.h"

using namespace matrixmath;
//=============================================================================
#ifndef assert
#define assert(...)
#endif
//=============================================================================
#ifdef USE_FLOAT_TYPE
typedef float _var_float;
#else
typedef double _var_float;
#endif
//=============================================================================
typedef Vect _var_vect;
typedef Point _var_point;
typedef uint8_t _var_byte;
typedef uint32_t _var_uint;
typedef uint8_t _var_flag;
typedef uint8_t _var_enum;
//=============================================================================
//get constants
#include "MandalaConstants.h"
#include "MandalaIndexes.h"
//-----------------------------------------------------------------------------
// typedefs and consts
#define MVAR(atype, aname, adescr, ...) typedef _var_##atype var_typedef_##aname;
#include "MandalaTemplate.h"
//=============================================================================
class MandalaCore
{
public:
    uint32_t pack(uint8_t *buf, uint32_t var_idx);
    uint32_t unpack(const uint8_t *buf, uint32_t cnt, uint32_t var_idx);

    // second compression level (for downstream)
    uint32_t pack_ext(uint8_t *buf, uint32_t var_idx);
    uint32_t unpack_ext(const uint8_t *buf, uint32_t var_idx);

    // basic protocols
    uint32_t unpack_stream(const uint8_t *buf, uint32_t cnt, bool hd); //downstream without header
    uint32_t unpack_set(const uint8_t *buf, uint32_t cnt); //unpack component <var_idx><mask><value>
    uint32_t pack_set(uint8_t *buf, uint16_t var_m);       //pack component <var_idx><mask><value>

    bool get_ptr(uint8_t var_idx, void **var_ptr, uint32_t *type);

    static void filter(const _var_float &fv,
                       _var_float *var_p,
                       const _var_float &fS = 0.05 / 100.0,
                       const _var_float &fL = 0.9 / 100.0);
    static void filter(const _var_vect &v,
                       _var_vect *var_p,
                       const _var_float &S = 0.05 / 100.0,
                       const _var_float &L = 0.9 / 100.0);
    static void filter_m(const _var_float &v, _var_float *var_p, const _var_float &f);
    static void filter_m(const _var_vect &v, _var_vect *var_p, const _var_float &f);
    //member=mask, if var is a bitfield, or vect idx
    _var_float get_data(uint16_t var_m);
    _var_float get_data(uint16_t var_m, uint32_t type, void *value_ptr);
    void set_data(uint16_t var_m, _var_float value);
    void set_data(uint16_t var_m, uint32_t type, void *value_ptr, _var_float value);
//-----------------------------------------------------------------------------
#define MVAR(atype, aname, ...) var_typedef_##aname aname;
#include "MandalaTemplate.h"

    typedef const uint8_t _vars_list[];
    static _vars_list vars_dlink;
    static _vars_list vars_ctr;

    //math calculations
    _var_float inHgToAltitude(_var_float inHg, _var_float inHg_gnd);

    //optimizations
    static void memcpy(void *dest, const void *src, uint32_t cnt)
    {
        if (cnt == 1)
            *((uint8_t *) dest) = *((const uint8_t *) src);
        else if (cnt > 1)
            ::memcpy(dest, src, cnt);
    }
    static int memcmp(const void *s1, const void *s2, uint32_t cnt)
    {
        if (cnt == 1)
            return *((const uint8_t *) s1) != *((const uint8_t *) s2);
        else if (cnt > 1)
            return ::memcmp(s1, s2, cnt);
        return -1;
    }

    //pack
    uint32_t pack_float_s(void *buf, const _var_float &v);
    uint32_t pack_float_u(void *buf, const _var_float &v);

    uint32_t pack_float_s1(void *buf, void *value_ptr);
    uint32_t pack_float_s01(void *buf, void *value_ptr);
    uint32_t pack_float_s001(void *buf, void *value_ptr);
    uint32_t pack_float_s10(void *buf, void *value_ptr);

    uint32_t pack_float_u1(void *buf, void *value_ptr);
    uint32_t pack_float_u01(void *buf, void *value_ptr);
    uint32_t pack_float_u001(void *buf, void *value_ptr);
    uint32_t pack_float_u10(void *buf, void *value_ptr);
    uint32_t pack_float_u100(void *buf, void *value_ptr);

    uint32_t pack_float_f2(void *buf, void *value_ptr);
    uint32_t pack_float_f4(void *buf, void *value_ptr);

    uint32_t pack_vect_f2(void *buf, void *value_ptr);
    uint32_t pack_vect_f4(void *buf, void *value_ptr);
    uint32_t pack_vect_s1(void *buf, void *value_ptr);
    uint32_t pack_vect_s10(void *buf, void *value_ptr);
    uint32_t pack_vect_s001(void *buf, void *value_ptr);

    uint32_t pack_point_f2(void *buf, void *value_ptr);
    uint32_t pack_point_f4(void *buf, void *value_ptr);
    uint32_t pack_point_u001(void *buf, void *value_ptr);

    uint32_t pack_flag_(void *buf, void *value_ptr);
    uint32_t pack_enum_(void *buf, void *value_ptr);
    uint32_t pack_byte_u1(void *buf, void *value_ptr);
    uint32_t pack_uint_u4(void *buf, void *value_ptr);
    uint32_t pack_uint_u2(void *buf, void *value_ptr);

    //unpack
    uint32_t unpack_float_s1(const void *buf, void *value_ptr);
    uint32_t unpack_float_s01(const void *buf, void *value_ptr);
    uint32_t unpack_float_s001(const void *buf, void *value_ptr);
    uint32_t unpack_float_s10(const void *buf, void *value_ptr);

    uint32_t unpack_float_u1(const void *buf, void *value_ptr);
    uint32_t unpack_float_u01(const void *buf, void *value_ptr);
    uint32_t unpack_float_u001(const void *buf, void *value_ptr);
    uint32_t unpack_float_u10(const void *buf, void *value_ptr);
    uint32_t unpack_float_u100(const void *buf, void *value_ptr);

    uint32_t unpack_float_f2(const void *buf, void *value_ptr);
    uint32_t unpack_float_f4(const void *buf, void *value_ptr);

    uint32_t unpack_vect_f2(const void *buf, void *value_ptr);
    uint32_t unpack_vect_f4(const void *buf, void *value_ptr);
    uint32_t unpack_vect_s1(const void *buf, void *value_ptr);
    uint32_t unpack_vect_s10(const void *buf, void *value_ptr);
    uint32_t unpack_vect_s001(const void *buf, void *value_ptr);

    uint32_t unpack_point_f2(const void *buf, void *value_ptr);
    uint32_t unpack_point_f4(const void *buf, void *value_ptr);
    uint32_t unpack_point_u001(const void *buf, void *value_ptr);

    uint32_t unpack_flag_(const void *buf, void *value_ptr);
    uint32_t unpack_enum_(const void *buf, void *value_ptr);
    uint32_t unpack_byte_u1(const void *buf, void *value_ptr);
    uint32_t unpack_uint_u4(const void *buf, void *value_ptr);
    uint32_t unpack_uint_u2(const void *buf, void *value_ptr);

    //uint32_t hash32(void); //structure hash
    //uint32_t hash32_calc(const char *str,uint32_t v);
};
//=============================================================================
#endif // MANDALA_CORE_H
