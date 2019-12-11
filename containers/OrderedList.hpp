#pragma once

#include "List.hpp"

#include <cinttypes>

template<class T>
class OrderedListNode : public ListNode<T>
{
public:
    virtual int id() const = 0;
};

template<class T>
class OrderedList : public List<T>
{
public:
    void add_ordered(T newNode)
    {
        //sorted linked list
        if (!List<T>::_head) {
            List<T>::_head = newNode;
        } else {
            int id = newNode->id();
            T i = List<T>::_head;
            T i0 = nullptr;
            for (;;) {
                if (i->id() > id) {
                    newNode->setSibling(i);
                    if (i0) {
                        i0->setSibling(newNode);
                    } else {
                        List<T>::_head = newNode;
                    }
                    break;
                }
                T is = i->getSibling();
                if (!is) {
                    i->setSibling(newNode);
                    break;
                }
                i0 = i;
                i = is;
            }
        }
    }
};
