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

#include <stdint.h>

// binary search function
template<typename T>
ssize_t binary_search(T x, T arr[], ssize_t size)
{
    ssize_t s = 0;
    ssize_t e = size - 1;
    while (s <= e) {
        ssize_t mid = (s + e) / 2;
        if (arr[mid] == x)
            return mid;
        else if (arr[mid] < x)
            s = mid + 1;
        else
            e = mid - 1;
    }
    return -1;
}
