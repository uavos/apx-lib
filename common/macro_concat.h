/*
 * APX Autopilot project <http://docs.uavos.com>
 *
 * Copyright (c) 2003-2020, Aliaksei Stratsilatau <sa@uavos.com>
 * All rights reserved
 *
 * This file is part of APX Shared Libraries.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include "macro_nargs.h"

/*

#define MY_MACRO    CONCAT(x,y,z)

will expand to xyz

*/

#define CONCAT(...) CONCAT_IMPL((__VA_ARGS__))

#define CONCAT_IMPL(LIST) CONCAT_IMPL1(NARGS LIST, LIST)
#define CONCAT_IMPL1(N, LIST) CONCAT_IMPL2(N, LIST)
#define CONCAT_IMPL2(N, LIST) CONCAT_##N LIST

#define CONCAT_S(a) #a

#define CONCAT_1(a) a
#define CONCAT_2(a, b) a##b
#define CONCAT_3(a, b, c) a##b##c
#define CONCAT_4(a, b, c, d) a##b##c##d
#define CONCAT_5(a, b, c, d, e) a##b##c##d##e
#define CONCAT_6(a, b, c, d, e, f) a##b##c##d##e##f
#define CONCAT_7(a, b, c, d, e, f, g) a##b##c##d##e##f##g
#define CONCAT_8(a, b, c, d, e, f, g, h) a##b##c##d##e##f##g##h
