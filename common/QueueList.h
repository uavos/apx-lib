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

#include <sys/types.h>

template<class T>
class Queue
{
public:
    bool empty() const { return !_head; }

    T front() const { return _head; }
    T back() const { return _tail; }

    size_t size() const
    {
        size_t sz = 0;

        for (auto node = front(); node; node = node->next_queue_node()) {
            sz++;
        }

        return sz;
    }

    void push(T newNode)
    {
        // error, node already queued or already inserted
        if (newNode->next_queue_node() || (newNode == _tail)) {
            return;
        }

        if (!_head) {
            _head = newNode;
        }

        if (_tail) {
            _tail->set_next_queue_node(newNode);
        }

        _tail = newNode;
    }

    void push_front(T newNode)
    {
        newNode->set_next_queue_node(_head);
        if (!_head)
            _tail = newNode;
        _head = newNode;
    }

    void insert_after(T prev, T newNode)
    {
        if (!_head || prev == _tail) {
            push(newNode);
        } else {
            newNode->set_next_queue_node(prev->next_queue_node());
            prev->set_next_queue_node(newNode);
        }
    }

    T pop()
    {
        T ret = _head;

        if (!empty()) {
            if (_head != _tail) {
                _head = _head->next_queue_node();

            } else {
                // only one item left
                _head = {};
                _tail = {};
            }

            // clear next in popped (in might be re-inserted later)
            ret->set_next_queue_node({});
        }

        return ret;
    }

    bool remove(T removeNode)
    {
        if (!removeNode)
            return false;

        // base case
        if (removeNode == _head) {
            _head = removeNode->next_queue_node();
            removeNode->set_next_queue_node({});
            return true;
        }

        for (T node = _head; node; node = node->next_queue_node()) {
            // is sibling the node to remove?
            if (node->next_queue_node() == removeNode) {
                // replace sibling
                node->set_next_queue_node(removeNode->next_queue_node());
                removeNode->set_next_queue_node({});
                return true;
            }
        }

        return false;
    }

    struct Iterator
    {
        T node;
        Iterator(T v)
            : node(v)
        {}

        operator T() const { return node; }
        operator T &() { return node; }
        T operator*() const { return node; }
        Iterator &operator++()
        {
            if (node) {
                node = node->next_queue_node();
            };

            return *this;
        }
    };

    Iterator begin() { return Iterator(_head); }
    Iterator end() { return Iterator({}); }

private:
    T _head{};
    T _tail{};
};

template<class T>
class QueueNode
{
protected:
    friend Queue<T>;

    T next_queue_node() const { return _next_queue_node; }
    void set_next_queue_node(T new_next) { _next_queue_node = new_next; }

private:
    T _next_queue_node{};
};
