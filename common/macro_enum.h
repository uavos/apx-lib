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

#include "macro_expand.h"

#define _ENUM_STRINGIFY(X) #X,
#define _ENUM_STRINGS(...) FOREACH(_ENUM_STRINGIFY, (__VA_ARGS__))

#define ENUM_MACRO(name, ...) \
    enum name { __VA_ARGS__ }; \
    static constexpr const char *name##_strings[] = {_ENUM_STRINGS(__VA_ARGS__)}; \
    static inline constexpr const char *name##_string(int value) { return name##_strings[value]; }
