///////////////////////////////////////////////////////////////////////////////
// MThreadPool - MuldeR's Thread Pool
// Copyright (C) 2014 LoRd_MuldeR <MuldeR2@GMX.de>
// All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pthread.h>
#include <stdexcept>

static inline void MTHREAD_CREATE(pthread_t *tid, const pthread_attr_t *attr, void *(PTW32_CDECL *start) (void*), void *arg)
{
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
