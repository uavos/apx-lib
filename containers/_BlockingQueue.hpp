#pragma once

#include <semaphore.h>

#include <MutexLocker.h>

template<class T, size_t N>
class BlockingQueue
{
public:
    BlockingQueue()
    {
        sem_init(&_sem_head, 0, N);
        sem_init(&_sem_tail, 0, 0);
    }

    ~BlockingQueue()
    {
        sem_destroy(&_sem_head);
        sem_destroy(&_sem_tail);
    }

    void push(T newItem)
    {
        sem_wait(&_sem_head);

        _data[_tail] = newItem;
        _tail = (_tail + 1) % N;

        sem_post(&_sem_tail);
    }

    T pop()
    {
        sem_wait(&_sem_tail);

        T ret = _data[_head];
        _head = (_head + 1) % N;

        sem_post(&_sem_head);

        return ret;
    }

private:
    sem_t _sem_head;
    sem_t _sem_tail;

    T _data[N]{};

    size_t _head{0};
    size_t _tail{0};
};
