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
    bool empty() const { return _head == nullptr; }

    T front() const { return _head; }
    T back() const { return _tail; }

    size_t size() const
    {
        size_t sz = 0;

        for (auto node = front(); node != nullptr; node = node->next_queue_node()) {
            sz++;
        }

        return sz;
    }

    void push(T newNode)
    {
        // error, node already queued or already inserted
        if ((newNode->next_queue_node() != nullptr) || (newNode == _tail)) {
            return;
        }

        if (_head == nullptr) {
            _head = newNode;
        }

        if (_tail != nullptr) {
            _tail->set_next_queue_node(newNode);
        }

        _tail = newNode;
    }

    T pop()
    {
        T ret = _head;

        if (!empty()) {
            if (_head != _tail) {
                _head = _head->next_queue_node();

            } else {
                // only one item left
                _head = nullptr;
                _tail = nullptr;
            }

            // clear next in popped (in might be re-inserted later)
            ret->set_next_queue_node(nullptr);
        }

        return ret;
    }

    bool remove(T removeNode)
    {
        if (removeNode == nullptr)
            return false;

        // base case
        if (removeNode == _head) {
            _head = removeNode->next_queue_node();
            removeNode->set_next_queue_node(nullptr);
            return true;
        }

        for (T node = _head; node != nullptr; node = node->next_queue_node()) {
            // is sibling the node to remove?
            if (node->next_queue_node() == removeNode) {
                // replace sibling
                node->set_next_queue_node(removeNode->next_queue_node());
                removeNode->set_next_queue_node(nullptr);
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
    Iterator end() { return Iterator(nullptr); }

private:
    T _head{nullptr};
    T _tail{nullptr};
};

template<class T>
class QueueNode
{
private:
    friend Queue<T>;

    T next_queue_node() const { return _next_queue_node; }
    void set_next_queue_node(T new_next) { _next_queue_node = new_next; }

    T _next_queue_node{nullptr};
};
