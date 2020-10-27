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

#define STRINGIFY(X)    #X
#define MY_VARIADIC_MACRO(...)    FOREACH(STRINGIFY, (__VA_ARGS__))

*/

#define EXPAND(X) X
#define FIRSTARG(X, ...) (X)
#define RESTARGS(X, ...) (__VA_ARGS__)

#define FOREACH(MACRO, LIST) FOREACH_IMPL(NARGS LIST, MACRO, LIST)

#define FOREACH_IMPL(N, M, LIST) FOREACH_IMPL2(N, M, LIST)
#define FOREACH_IMPL2(N, M, LIST) FOREACH_##N(M, LIST)

#define FOREACH_1(M, LIST) M LIST
#define FOREACH_2(M, LIST) EXPAND(M FIRSTARG LIST) \
FOREACH_1(M, RESTARGS LIST)
#define FOREACH_3(M, LIST) EXPAND(M FIRSTARG LIST) \
FOREACH_2(M, RESTARGS LIST)
#define FOREACH_4(M, LIST) EXPAND(M FIRSTARG LIST) \
FOREACH_3(M, RESTARGS LIST)
#define FOREACH_5(M, LIST) EXPAND(M FIRSTARG LIST) \
FOREACH_4(M, RESTARGS LIST)
#define FOREACH_6(M, LIST) EXPAND(M FIRSTARG LIST) \
FOREACH_5(M, RESTARGS LIST)
#define FOREACH_7(M, LIST) EXPAND(M FIRSTARG LIST) \
FOREACH_6(M, RESTARGS LIST)
#define FOREACH_8(M, LIST) EXPAND(M FIRSTARG LIST) \
FOREACH_7(M, RESTARGS LIST)
#define FOREACH_9(M, LIST) EXPAND(M FIRSTARG LIST) \
FOREACH_8(M, RESTARGS LIST)
#define FOREACH_10(M, LIST) EXPAND(M FIRSTARG LIST) \
FOREACH_9(M, RESTARGS LIST)
#define FOREACH_11(M, LIST) EXPAND(M FIRSTARG LIST) \
FOREACH_10(M, RESTARGS LIST)
#define FOREACH_12(M, LIST) EXPAND(M FIRSTARG LIST) \
FOREACH_11(M, RESTARGS LIST)
#define FOREACH_13(M, LIST) EXPAND(M FIRSTARG LIST) \
FOREACH_12(M, RESTARGS LIST)
#define FOREACH_14(M, LIST) EXPAND(M FIRSTARG LIST) \
FOREACH_13(M, RESTARGS LIST)
#define FOREACH_15(M, LIST) EXPAND(M FIRSTARG LIST) \
FOREACH_14(M, RESTARGS LIST)
#define FOREACH_16(M, LIST) EXPAND(M FIRSTARG LIST) \
FOREACH_15(M, RESTARGS LIST)
#define FOREACH_17(M, LIST) EXPAND(M FIRSTARG LIST) \
FOREACH_16(M, RESTARGS LIST)
#define FOREACH_18(M, LIST) EXPAND(M FIRSTARG LIST) \
FOREACH_17(M, RESTARGS LIST)
#define FOREACH_19(M, LIST) EXPAND(M FIRSTARG LIST) \
FOREACH_18(M, RESTARGS LIST)
#define FOREACH_20(M, LIST) EXPAND(M FIRSTARG LIST) \
FOREACH_19(M, RESTARGS LIST)
