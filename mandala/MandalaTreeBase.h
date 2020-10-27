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

#include "MandalaMetaBase.h"
//#include "MandalaStream.h"

namespace mandala {

template<typename _Tree>
struct tree_base_t
{
    static constexpr bool match(uid_t uid)
    {
        return (uid & _Tree::meta.mask) == _Tree::uid;
    }
};

template<typename _Tree>
struct tree_0_t : public tree_base_t<_Tree>
{
};

template<typename _Tree>
struct tree_1_t : public tree_base_t<_Tree>
{
};

template<typename _Tree>
struct tree_2_t : public tree_base_t<_Tree>
{
};

template<typename _DataType, typename _Tree>
class tree_value_t : public tree_base_t<_Tree>
{
public:
    /*constexpr operator _DataType() const { return m_value; }

    constexpr const _DataType &get() const { return m_value; }

    bool set(const _DataType &v)
    {
        if (m_value == v)
            return false;
        m_value = v;
        return true;
    }*/

    /*explicit field_t() {}
    field_t(const field_t &) = delete;
    field_t &operator=(const field_t &) = delete;
    field_t(field_t &&) = delete;
    field_t &operator=(field_t &&) = delete;*/

    /*constexpr inline size_t pack(void *buf) const
    {
        return stream::pack<_Tree::meta.sfmt>(buf, m_value);
    }
    constexpr inline size_t unpack(const void *buf)
    {
        return stream::unpack<_Tree::meta.sfmt>(buf, m_value);
    }*/

    /*inline size_t copy_to(void *buf) const
    {
        memcpy(buf, &m_value, sizeof(_DataType));
        return sizeof(_DataType);
    }
    inline size_t copy_from(const void *buf)
    {
        memcpy(&m_value, buf, sizeof(_DataType));
        return sizeof(_DataType);
    }*/

protected:
    //_DataType m_value{};
};

}; // namespace mandala
