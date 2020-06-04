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

        node->ListNode<T>::setSibling(nullptr);
        if (!_head) {
            _head = node;
            return;
        }
        for (auto i = _head; i != nullptr; i = i->ListNode<T>::getSibling()) {
            if (i->ListNode<T>::getSibling())
                continue;
            i->ListNode<T>::setSibling(node);
            return;
        }
    }

    void insert(T node, T after)
    {
        GUARD guard;
        (void) guard;

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

protected:
    T _head{nullptr};
};
