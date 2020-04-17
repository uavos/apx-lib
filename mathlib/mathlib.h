/****************************************************************************
 *
 *   Copyright (C) 2012 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file mathlib.h
 *
 * Common header for mathlib exports.
 */

#pragma once

#ifdef __cplusplus

#include "math/Functions.hpp"
#include "math/Integrator.h"
#include "math/Limits.hpp"
#include "math/SearchMin.hpp"
#include "math/TrajMath.hpp"
#include "math/matrix_alg.h"

#endif

/* Math macro's for float literals. Do not use M_PI et al as they aren't
 * defined (neither C nor the C++ standard define math constants) */
#define M_E_F 2.71828183f
#define M_LOG2E_F 1.44269504f
#define M_LOG10E_F 0.43429448f
#define M_LN2_F 0.69314718f
#define M_LN10_F 2.30258509f
///#define M_PI_F 3.14159265f
#define M_TWOPI_F 6.28318531f
//#define M_PI_2_F 1.57079632f
#define M_PI_4_F 0.78539816f
#define M_3PI_4_F 2.35619449f
#define M_SQRTPI_F 1.77245385f
#define M_1_PI_F 0.31830989f
#define M_2_PI_F 0.63661977f
#define M_2_SQRTPI_F 1.12837917f
#define M_DEG_TO_RAD_F 0.0174532925f
#define M_RAD_TO_DEG_F 57.2957795f
#define M_SQRT2_F 1.41421356f
#define M_SQRT1_2_F 0.70710678f
#define M_LN2LO_F 1.90821484E-10f
#define M_LN2HI_F 0.69314718f
#define M_SQRT3_F 1.73205081f
#define M_IVLN10_F 0.43429448f // 1 / log(10)
#define M_LOG2_E_F 0.69314718f
#define M_INVLN2_F 1.44269504f // 1 / log(2)

#define M_DEG_TO_RAD 0.017453292519943295
#define M_RAD_TO_DEG 57.295779513082323
