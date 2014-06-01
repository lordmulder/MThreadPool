///////////////////////////////////////////////////////////////////////////////
// MThreadPool - MuldeR's Thread Pool
// Copyright (C) 2014 LoRd_MuldeR <MuldeR2@GMX.de>
// All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#include "ThreadPool.h"

using namespace MTHREADPOOL_NS;

///////////////////////////////////////////////////////////////////////////////
// Constructor & Destructor
///////////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(const uint32_t &nThreads)
:
	m_nThreads(nThreads)
{
	m_bStopFlag = false;

	//Allocate threads
	m_threads = new pthread_t[nThreads];
	memset(m_threads, 0, sizeof(pthread_t) * nThreads);
	for(uint32_t i = 0; i < nThreads; i++)
	{
		MTHREAD_CREATE(&m_threads[i], NULL, entryPoint, this);
	}
}

ThreadPool::~ThreadPool(void)
{
	//Stop all running threads!
	m_bStopFlag = true;
	for(uint32_t i = 0; i < m_nThreads; i++)
	{
		MTHREAD_JOIN(m_threads[i]);
	}

	//Clear thread data
	memset(m_threads, 0, sizeof(pthread_t) * m_nThreads);
}

///////////////////////////////////////////////////////////////////////////////
// Schedule next task
///////////////////////////////////////////////////////////////////////////////

bool ThreadPool::schedule(ITask *const task)
{
	return false;
}

bool ThreadPool::trySchedule(ITask *const task)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// Thread entry point
///////////////////////////////////////////////////////////////////////////////

void *ThreadPool::entryPoint(void *arg)
{
	ThreadPool const* pool = static_cast<ThreadPool*>(arg);

	while(!pool->m_bStopFlag)
	{
		volatile int x = rand();
	}

	return NULL;
}
