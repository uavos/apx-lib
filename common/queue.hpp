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

namespace apx {

template<class T>
using queue_item = list<T>;

template<class T>
class queue : private list<T>
{
public:
    inline T *front() const { return link(); }
    inline T *back() const { return _back; }

    using list<T>::empty;

    // append arg node to the tail
    void push(T *n)
    {
        // error, node already queued or already inserted
        if (n->link() || (n == back())) {
            return;
        }
        if (empty()) {
            link() = n;
            _back = n;
            return;
        }
        back()->link() = n;
        _back = n;
    }

    void push_front(T *n)
    {
        n->link() = front();
        if (empty())
            _back = n;
        link() = n;
    }

    void insert_after(T *prev, T *n)
    {
        if (empty() || prev == back()) {
            push(n);
        } else {
            n->link() = (T *) prev->link();
            prev->link() = n;
        }
    }

    T *pop()
    {
        auto n = front();

        if (!empty()) {
            if (n != back()) {
                // more than one item queued
                link() = (T *) n->link();
            } else {
                // only one item left
                link() = nullptr;
                _back = nullptr;
            }

            n->link() = nullptr;
        }

        return n;
    }

    bool remove(T *n)
    {
        // unlink node to make sure iterator breaks
        T *next = n->link();
        n->link() = nullptr;

        // base case
        if (n == front()) {
            if (next) {
                // more than one item queued
                link() = next;
                n->link() = nullptr;
            } else {
                // only one item left
                link() = nullptr;
                _back = nullptr;
            }
            return true;
        }

        // find node parent
        for (auto i : *this) {
            if (i->link() == n) {
                // replace sibling
                i->link() = next;
                n->link() = nullptr;
                if (n == back())
                    _back = i;
                return true;
            }
        }

        // node not found in queue
        return false;
    }

    bool contains(const T *n) const
    {
        for (auto i : *this) {
            if (i == n)
                return true;
        }
        return false;
    }

    // allow queue iterator access
    using list<T>::begin;
    using list<T>::end;

private:
    T *_back{};

    using list<T>::link;
};

} // namespace apx
