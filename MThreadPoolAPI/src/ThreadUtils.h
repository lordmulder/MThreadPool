///////////////////////////////////////////////////////////////////////////////
// MThreadPool - MuldeR's Thread Pool
// Copyright (C) 2014 LoRd_MuldeR <MuldeR2@GMX.de>
// All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pthread.h>
#include <semaphore.h>
#include <stdexcept>

///////////////////////////////////////////////////////////////////////////////
// Thread
///////////////////////////////////////////////////////////////////////////////

static inline void MTHREAD_CREATE(pthread_t *tid, const pthread_attr_t *attr, void *(PTW32_CDECL *start) (void*), void *arg)
{
	memset(tid, 0, sizeof(pthread_t));
	if(pthread_create(tid, attr, start, arg) != 0)
	{
		throw std::runtime_error("pthread_create() failed!");
	}
}

static inline void MTHREAD_JOIN(const pthread_t thread, void **value_ptr = NULL)
{
	if(pthread_join(thread, value_ptr) != 0)
	{
		throw std::runtime_error("pthread_join() failed!");
	}
}

///////////////////////////////////////////////////////////////////////////////
// Mutex
///////////////////////////////////////////////////////////////////////////////

static inline void MTHREAD_MUTEX_INIT(pthread_mutex_t *const mutex, const pthread_mutexattr_t *attr = NULL)
{
	memset(mutex, 0, sizeof(pthread_mutex_t));
	if(pthread_mutex_init(mutex, NULL) != 0)
	{
		throw std::runtime_error("pthread_mutex_init() failed!");
	}
}

static inline void MTHREAD_MUTEX_LOCK(pthread_mutex_t *const mutex)
{
	if(pthread_mutex_lock(mutex) != 0)
	{
		throw std::runtime_error("pthread_mutex_lock() failed!");
	}
}

static inline void MTHREAD_MUTEX_UNLOCK(pthread_mutex_t *const mutex)
{
	if(pthread_mutex_unlock(mutex) != 0)
	{
		throw std::runtime_error("pthread_mutex_unlock() failed!");
	}
}

static inline void MTHREAD_MUTEX_DESTROY(pthread_mutex_t *const mutex)
{
	if(pthread_mutex_destroy(mutex) != 0)
	{
		throw std::runtime_error("pthread_mutex_destroy() failed!");
	}
	memset(mutex, 0, sizeof(pthread_mutex_t));
}

///////////////////////////////////////////////////////////////////////////////
// Semaphore
///////////////////////////////////////////////////////////////////////////////

static inline void MTHREAD_SEM_INIT(sem_t *const sem, const uint32_t &value)
{
	memset(sem, 0, sizeof(sem_t));
	if(sem_init(sem, 0, value) != 0)
	{
		throw std::runtime_error("sem_init() failed!");
	}
}

static inline void MTHREAD_SEM_WAIT(sem_t *const sem)
{
	if(sem_wait(sem) != 0)
	{
		throw std::runtime_error("sem_wait() failed!");
	}
}

static inline bool MTHREAD_SEM_TRYWAIT(sem_t *const sem)
{
	if(sem_trywait(sem) != 0)
	{
		if(errno != EAGAIN)
		{
			throw std::runtime_error("sem_trywait() failed!");
		}
		return false;
	}
	return true;
}

static inline void MTHREAD_SEM_POST(sem_t *const sem, const uint32_t &count = 1)
{
	if(count > 1)
	{
		if(sem_post_multiple(sem, count) != 0)
		{
			throw std::runtime_error("sem_post_multiple() failed!");
		}
	}
	else if(count == 1)
	{
		if(sem_post(sem) != 0)
		{
			throw std::runtime_error("sem_post() failed!");
		}
	}
}

static inline void MTHREAD_SEM_DESTROY(sem_t *const sem)
{
	if(sem_destroy(sem) != 0)
	{
		throw std::runtime_error("sem_destroy() failed!");
	}
	memset(sem, 0, sizeof(sem_t));
}

///////////////////////////////////////////////////////////////////////////////
// Conditional Var
///////////////////////////////////////////////////////////////////////////////

static inline void MTHREAD_COND_INIT(pthread_cond_t *const cond, const pthread_condattr_t *attr = NULL)
{
	memset(cond, 0, sizeof(pthread_cond_t));
	if(pthread_cond_init(cond, NULL) != 0)
	{
		throw std::runtime_error("pthread_cond_init() failed!");
	}
}

static inline void MTHREAD_COND_WAIT(pthread_cond_t *const cond, pthread_mutex_t *const mutex)
{
	if(pthread_cond_wait(cond, mutex) != 0)
	{
		throw std::runtime_error("pthread_cond_wait() failed!");
	}
}

static inline void MTHREAD_COND_BROADCAST(pthread_cond_t *const cond)
{
	if(pthread_cond_broadcast(cond) != 0)
	{
		throw std::runtime_error("pthread_cond_broadcast() failed!");
	}
}

static inline void MTHREAD_COND_DESTROY(pthread_cond_t *const cond)
{
	if(pthread_cond_destroy(cond) != 0)
	{
		throw std::runtime_error("pthread_cond_destroy() failed!");
	}
	memset(cond, 0, sizeof(pthread_cond_t));
}
