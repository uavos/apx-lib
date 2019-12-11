/**
 * @file List.hpp
 *
 * An intrusive linked list.
 */

#pragma once

#include <cinttypes>

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
    void add(T newNode)
    {
        newNode->setSibling(getHead());
        _head = newNode;
    }

    bool remove(T removeNode)
    {
        if (removeNode == nullptr) {
            return false;
        }

        // base case
        if (removeNode == _head) {
            if (_head != nullptr) {
                _head = _head->getSibling();
            }

            return true;
        }

        for (T node = getHead(); node != nullptr; node = node->getSibling()) {
            // is sibling the node to remove?
            if (node->getSibling() == removeNode) {
                // replace sibling
                if (node->getSibling() != nullptr) {
                    node->setSibling(node->getSibling()->getSibling());

                } else {
                    node->setSibling(nullptr);
                }

                return true;
            }
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
            };

            return *this;
        }
    };

    Iterator begin() const { return Iterator(getHead()); }
    Iterator end() const { return Iterator(nullptr); }

    const T getHead() const { return _head; }

    bool empty() const { return getHead() == nullptr; }

    size_t size() const
    {
        size_t sz = 0;

        for (auto node = getHead(); node != nullptr; node = node->getSibling()) {
            sz++;
        }

        return sz;
    }

    void deleteNode(T node)
    {
        if (remove(node)) {
            // only delete if node was successfully removed
            delete node;
        }
    }

    void clear()
    {
        auto node = getHead();

        while (node != nullptr) {
            auto next = node->getSibling();
            delete node;
            node = next;
        }

        _head = nullptr;
    }

protected:
    T _head{nullptr};
};
