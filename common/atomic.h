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

#include <atomic>
#include <stdbool.h>
#include <stdint.h>

namespace apx {

template<typename T>
class atomic_std : public std::atomic<T>
{
public:
    // Ensure that all operations are lock-free, so that 'atomic' can be used from
    // IRQ handlers. This might not be required everywhere though.
    static_assert(__atomic_always_lock_free(sizeof(T), 0), "atomic is not lock-free for the given type T");

    /*explicit atomic_std(T value)
    {
        std::atomic_init(this, value);
    }*/
};

template<typename T>
class atomic
{
public:
    // Ensure that all operations are lock-free, so that 'atomic' can be used from
    // IRQ handlers. This might not be required everywhere though.
    static_assert(__atomic_always_lock_free(sizeof(T), 0), "atomic is not lock-free for the given type T");

    explicit atomic(T value)
        : _value(value)
    {}

    /**
	 * Atomically read the current value
	 */
    inline T load() const
    {
        return __atomic_load_n(&_value, __ATOMIC_SEQ_CST);
    }

    /**
	 * Atomically store a value
	 */
    inline void store(T value)
    {
        __atomic_store(&_value, &value, __ATOMIC_SEQ_CST);
    }

    /**
	 * Atomically add a number and return the previous value.
	 * @return value prior to the addition
	 */
    inline T fetch_add(T num)
    {
        return __atomic_fetch_add(&_value, num, __ATOMIC_SEQ_CST);
    }

    /**
	 * Atomically substract a number and return the previous value.
	 * @return value prior to the substraction
	 */
    inline T fetch_sub(T num)
    {
        return __atomic_fetch_sub(&_value, num, __ATOMIC_SEQ_CST);
    }

    /**
	 * Atomic AND with a number
	 * @return value prior to the operation
	 */
    inline T fetch_and(T num)
    {
        return __atomic_fetch_and(&_value, num, __ATOMIC_SEQ_CST);
    }

    /**
	 * Atomic XOR with a number
	 * @return value prior to the operation
	 */
    inline T fetch_xor(T num)
    {
        return __atomic_fetch_xor(&_value, num, __ATOMIC_SEQ_CST);
    }

    /**
	 * Atomic OR with a number
	 * @return value prior to the operation
	 */
    inline T fetch_or(T num)
    {
        return __atomic_fetch_or(&_value, num, __ATOMIC_SEQ_CST);
    }

    /**
	 * Atomic NAND (~(_value & num)) with a number
	 * @return value prior to the operation
	 */
    inline T fetch_nand(T num)
    {
        return __atomic_fetch_nand(&_value, num, __ATOMIC_SEQ_CST);
    }

    /**
	 * Atomic compare and exchange operation.
	 * This compares the contents of _value with the contents of *expected. If
	 * equal, the operation is a read-modify-write operation that writes desired
	 * into _value. If they are not equal, the operation is a read and the current
	 * contents of _value are written into *expected.
	 * @return If desired is written into _value then true is returned
	 */
    inline bool compare_exchange(T *expected, T num)
    {
        return __atomic_compare_exchange(&_value, expected, num, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    }

private:
    T _value;

private:
    // Disallow copy construction and move assignment
    atomic(const atomic &) = delete;
    atomic &operator=(const atomic &) = delete;
    atomic(atomic &&) = delete;
    atomic &operator=(atomic &&) = delete;
};

} // namespace apx
