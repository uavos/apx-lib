#pragma once

#include <cinttypes>
#include <cstddef>

template<class T>
class ListNode
{
public:
    void setSibling(T sibling) { _list_node_sibling = sibling; }
    const T getSibling() const { return _list_node_sibling; }

protected:
    T _list_node_sibling{nullptr};
};

template<class T>
class List
{
public:
    void add(T node)
    {
        if (!_head) {
            _head = node;
            return;
        }
        for (T i = _head;;) {
            T s = i->getSibling();
            if (!s) {
                i->setSibling(node);
                return;
            }
            i = s;
        }
    }

    bool remove(T node)
    {
        if (node == nullptr || _head == nullptr) {
            return false;
        }

        if (node == _head) {
            _head = _head->getSibling();
        }

        for (T i = _head;;) {
            T s = i->getSibling();
            if (!s)
                return false;
            if (s != node)
                continue;
            i->setSibling(s->getSibling());
            return true;
        }
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
            if (node) {
                node = node->getSibling();
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

        for (auto node = _head; node != nullptr; node = node->getSibling()) {
            sz++;
        }

        return sz;
    }

protected:
    T _head{nullptr};
};
