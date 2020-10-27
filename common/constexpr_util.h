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

static inline int _constexpr_assert_failure(const char *msg)
{
    (void) msg;
    // we do 2 things that the compiler will refuse to execute at compile-time
    // (and therefore trigger a compilation error):
    // - define a local static variable
    // - declare the method as non constexpr
    static int i = 0;
    return i;
}

/**
 * Assertion that fails compilation if used in a constexpr context (that is executed at
 * compile-time).
 *
 * Important: you need to ensure the code is executed at compile-time, e.g. by
 * assigning the returned value of a constexpr method (where the assert is used)
 * to a variable marked as constexpr. Otherwise the compiler might silently move
 * execution to runtime.
 *
 * If executed at runtime, it has no effect other than slight runtime overhead.
 */
#define constexpr_assert(expr, msg) \
    if (!(expr)) { \
        _constexpr_assert_failure(msg); \
    }
