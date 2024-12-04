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

#include "list.hpp"
#include <do_not_copy.h>

namespace apx {

template<typename T>
class Event : public list<Event<T>>, private do_not_copy
{
public:
    // call all connected slots, arg can be 'this' to filter local class
    static void signal(Event<T> *arg)
    {
        for (auto i : _list) {
            if (i != arg)
                i->event(arg);
        }
    }
    static void signal()
    {
        Event<T> *arg = {};
        for (auto i : _list) {
            i->event(arg);
        }
    }

    static inline size_t size() { return _list.size(); }
    static inline int indexOf(Event<T> *e) { return _list.indexOf(e); }

    typedef T type_t;

protected:
    explicit Event() { connect(); }
    ~Event() { disconnect(); }
    inline void connect() { _list.add(this); }
    inline void disconnect() { _list.remove(this); }

    virtual void event(Event<T> *) = 0;

private:
    static list<Event<T>> _list;
};

template<typename T>
list<Event<T>> Event<T>::_list;

template<typename T>
class EventValue
{
public:
    typedef Event<EventValue<T>> evt;

    void set(const T v)
    {
        _value = v;
        evt::signal(this);
    }

    inline T value() const { return _value; }
    inline operator T() const { return value(); }

private:
    T _value{};
};

} // namespace apx

#define EVENT(_name) \
    struct _##_name##_t; \
    using _name = apx::Event<_##_name##_t>
