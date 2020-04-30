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
        node->setSibling(nullptr);
        if (!_head) {
            _head = node;
            return;
        }
        for (auto i = _head; i != nullptr; i = i->getSibling()) {
            if (i->getSibling())
                continue;
            i->setSibling(node);
            return;
        }
    }

    void insert(T node, T after)
    {
        if (!after) {
            node->setSibling(_head);
            _head = node;
            return;
        }
        node->setSibling(after->getSibling());
        after->setSibling(node);
    }

    bool remove(T node)
    {
        if (node == nullptr || _head == nullptr) {
            return false;
        }

        if (node == _head) {
            _head = _head->getSibling();
            return true;
        }

        for (auto i = _head; i != nullptr; i = i->getSibling()) {
            if (i->getSibling() != node)
                continue;
            i->setSibling(node->getSibling());
            return true;
        }
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
