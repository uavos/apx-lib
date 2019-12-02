/****************************************************************************
 *
 *   Copyright (C) 2019 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

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

    sem_t	_sem_head;
    sem_t	_sem_tail;

	T _data[N] {};

	size_t _head{0};
	size_t _tail{0};

};
