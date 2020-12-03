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

#include <List.h>
#include <do_not_copy.h>

namespace apx {

template<typename T>
class Event : public ListNode<apx::Event<T> *>, private do_not_copy
{
public:
    static void signal(apx::Event<T> *arg = nullptr)
    {
        for (auto i : _list) {
            i->event(arg);
        }
    }
    static inline size_t size() { return _list.size(); }
    static inline int indexOf(apx::Event<T> *e) { return _list.indexOf(e); }

    typedef T type_t;

protected:
    explicit Event() { connect(); }
    ~Event() { disconnect(); }
    inline void connect() { _list.add(this); }
    inline void disconnect() { _list.remove(this); }

    virtual void event(apx::Event<T> *) = 0;

private:
    static List<apx::Event<T> *> _list;
};

template<typename T>
List<apx::Event<T> *> apx::Event<T>::_list;

} // namespace apx

#define EVENT(aname) \
    struct _##aname##_t; \
    using aname = apx::Event<_##aname##_t>
