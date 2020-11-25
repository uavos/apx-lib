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

#include <cinttypes>
#include <do_not_copy.h>
#include <sys/types.h>

template<class T>
class ListNode
{
public:
    inline constexpr void setSibling(T sibling) { _list_node_sibling = sibling; }
    inline constexpr const T getSibling() const { return _list_node_sibling; }

protected:
    T _list_node_sibling{nullptr};
};

struct ListGuard
{
};

template<class T, typename GUARD = ListGuard>
class List : private do_not_copy
{
public:
    void add(T node)
    {
        GUARD guard;
        (void) guard;

        if (!_head) {
            _head = node;
            node->ListNode<T>::setSibling(nullptr);
            return;
        }
        for (auto i = _head;; i = i->ListNode<T>::getSibling()) {
            if (i == node) // already added
                return;
            if (i->ListNode<T>::getSibling())
                continue;
            i->ListNode<T>::setSibling(node);
            node->ListNode<T>::setSibling(nullptr);
            return;
        }
        node->ListNode<T>::setSibling(nullptr);
    }

    void insert(T node, T after)
    {
        GUARD guard;
        (void) guard;

        for (auto i = _head; i; i = i->ListNode<T>::getSibling()) {
            if (i == node) { // already added
                return;
            }
        }

        if (!after) {
            node->ListNode<T>::setSibling(_head);
            _head = node;
            return;
        }
        node->ListNode<T>::setSibling(after->ListNode<T>::getSibling());
        after->ListNode<T>::setSibling(node);
    }

    bool remove(T node)
    {
        GUARD guard;
        (void) guard;

        if (node == nullptr || _head == nullptr) {
            return false;
        }

        if (node == _head) {
            _head = node->ListNode<T>::getSibling();
            node->ListNode<T>::setSibling(nullptr);
            return true;
        }

        for (auto i = _head; i != nullptr; i = i->ListNode<T>::getSibling()) {
            if (i->ListNode<T>::getSibling() != node)
                continue;
            i->ListNode<T>::setSibling(node->ListNode<T>::getSibling());
            node->ListNode<T>::setSibling(nullptr);
            return true;
        }
        node->ListNode<T>::setSibling(nullptr);
        return false;
    }

    struct Iterator
    {
        T node;
        explicit Iterator(T v)
            : node(v)
        {}

        operator T() const { return node; }
        operator T &() { return node; }
        T operator*() const { return node; }
        Iterator &operator++()
        {
            GUARD guard;
            (void) guard;

            if (node) {
                node = node->ListNode<T>::getSibling();
            }

            return *this;
        }
    };

    inline Iterator begin() const
    {
        return Iterator(_head);
    }
    inline Iterator end() const
    {
        return Iterator(nullptr);
    }

    inline bool empty() const
    {
        return _head == nullptr;
    }

    size_t size() const
    {
        size_t sz = 0;

        GUARD guard;
        (void) guard;

        for (auto node = _head; node != nullptr; node = node->ListNode<T>::getSibling()) {
            sz++;
        }

        return sz;
    }

    int indexOf(T node)
    {
        GUARD guard;
        (void) guard;

        int index = 0;
        for (auto i = _head; i != nullptr; i = i->ListNode<T>::getSibling()) {
            if (i == node)
                return index;
        }
        return -1;
    }

protected:
    T _head{nullptr};
};
