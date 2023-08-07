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
#include <cstddef>

namespace apx {

template<class T, class Tdata = T *>
class list
{
public:
    inline bool empty() const { return link() == nullptr; }

    inline auto const &link() const { return _link; }
    inline auto &link() { return _link; }

    struct Iterator
    {
        using node_t = T *;
        node_t node;
        explicit Iterator(node_t v)
            : node(v)
        {}
        inline operator node_t() const { return node; }
        inline operator node_t &() { return node; }
        inline node_t operator*() const { return node; }

        inline Iterator &operator++()
        {
            if (node)
                node = node->list<T, Tdata>::link();
            return *this;
        }
    };
    inline auto begin() const
    {
        return Iterator(link());
    }
    inline auto end() const { return Iterator(nullptr); }

    // append arg node to the tail
    void add(T *n)
    {
        //"this" is probably the root node
        for (auto i = this;;) {
            if (i == n) // already in list
                return;

            T *next = i->link();
            if (!next) {
                n->list<T, Tdata>::_link = nullptr;
                i->_link = n;
                return;
            }
            i = next;
        }
    }

    // insert 'n' after this node
    void insert(T *n)
    {
        //n must not belong to any exiting list
        //"this" is probably the root node
        n->_link = (T *) _link;
        _link = n;
    }

    // insert 'n' after 'after' node or at head if 'after' is nullptr
    inline void insert_after(T *n, T *after)
    {
        if (after)
            after->insert(n);
        else
            insert(n);
    }

    // search and remove arg node
    void remove(T *n)
    {
        //"this" is probably the root node
        for (auto i = this; i;) {
            T *next = i->link();
            if (next == n) {
                i->_link = (T *) n->list<T, Tdata>::link();
                n->list<T, Tdata>::_link = nullptr;
                return;
            }
            i = next;
        }
    }

    // est linked list size
    size_t size() const
    {
        size_t sz = 0;
        for (auto i : *this) {
            (void) i;
            sz++;
        }
        return sz;
    }

    int indexOf(T *node)
    {
        int index = 0;
        for (auto i : *this) {
            if (i == node)
                return index;
            index++;
        }
        return -1;
    }

private:
    Tdata _link{};
};

} // namespace apx
