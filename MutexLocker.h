#pragma once

#include <pthread.h>

class MutexLocker
{
public:
    explicit MutexLocker(pthread_mutex_t &mutex) :
		_mutex(mutex)
	{
		pthread_mutex_lock(&_mutex);
	}

    MutexLocker(const MutexLocker &other) = delete;
    MutexLocker &operator=(const MutexLocker &other) = delete;

    ~MutexLocker()
	{
		pthread_mutex_unlock(&_mutex);
	}

private:
	pthread_mutex_t &_mutex;
};
