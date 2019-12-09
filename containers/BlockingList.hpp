/**
 * @file BlockingList.hpp
 *
 * A blocking intrusive linked list.
 */

#pragma once

#include "List.hpp"
#include <MutexLocker.h>

#include <pthread.h>
#include <stdlib.h>

template<class T>
class BlockingList : public List<T>
{
public:
    ~BlockingList()
    {
        pthread_mutex_destroy(&_mutex);
        pthread_cond_destroy(&_cv);
    }

    void add(T newNode)
    {
        MutexLocker lg{_mutex};
        List<T>::add(newNode);
    }

    bool remove(T removeNode)
    {
        MutexLocker lg{_mutex};
        return List<T>::remove(removeNode);
    }

    size_t size()
    {
        MutexLocker lg{_mutex};
        return List<T>::size();
    }

    void clear()
    {
        MutexLocker lg{_mutex};
        List<T>::clear();
    }

    pthread_mutex_t &mutex() { return _mutex; }

private:
    pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t _cv = PTHREAD_COND_INITIALIZER;
};
