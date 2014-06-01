///////////////////////////////////////////////////////////////////////////////
// MThreadPool - MuldeR's Thread Pool
// Copyright (C) 2014 LoRd_MuldeR <MuldeR2@GMX.de>
// All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#include "ThreadPool.h"

#include <cstdio>

using namespace MTHREADPOOL_NS;

#define LOG(X, ...) fprintf(stderr, "[MThreadPool] " X "\n", __VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////
// Constructor & Destructor
///////////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(const uint32_t &nThreads)
:
	m_nThreads(nThreads)
{
	m_bStopFlag = false;
	m_runningTasks = 0;

	//Create the lock
	MTHREAD_MUTEX_INIT(&m_lock);

	//Create semaphores
	MTHREAD_SEM_INIT(&m_semFree, nThreads);
	MTHREAD_SEM_INIT(&m_semUsed, 0);

	//Create conditional var
	MTHREAD_COND_INIT(&m_condDone);

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
	//Do we still have any running/pending tasks?
	MTHREAD_MUTEX_LOCK(&m_lock);
	if((!m_taskQueue.empty()) || (m_runningTasks > 0))
	{
		LOG("Warning: Destructor called while still have running/pending tasks!");
	}
	MTHREAD_MUTEX_UNLOCK(&m_lock);

	//Stop all running threads!
	m_bStopFlag = true;
	MTHREAD_SEM_POST(&m_semUsed, m_nThreads);

	//Wait for threads to exit
	for(uint32_t i = 0; i < m_nThreads; i++)
	{
		MTHREAD_JOIN(m_threads[i]);
	}

	//Clear thread data
	memset(m_threads, 0, sizeof(pthread_t) * m_nThreads);

	//Create semaphores
	MTHREAD_SEM_DESTROY(&m_semFree);
	MTHREAD_SEM_DESTROY(&m_semUsed);

	//Destroy conditional var
	MTHREAD_COND_DESTROY(&m_condDone);

	//Destroy the lock
	MTHREAD_MUTEX_DESTROY(&m_lock);

	//Clear pending tasks, if any
	std::queue<MTHREADPOOL_NS::ITask*> empty;
	m_taskQueue.swap(empty);
}

///////////////////////////////////////////////////////////////////////////////
// Schedule next task
///////////////////////////////////////////////////////////////////////////////

bool ThreadPool::schedule(ITask *const task)
{
	try
	{
		MTHREAD_SEM_WAIT(&m_semFree);
		MTHREAD_MUTEX_LOCK(&m_lock);

		m_taskQueue.push(task);

		MTHREAD_MUTEX_UNLOCK(&m_lock);
		MTHREAD_SEM_POST(&m_semUsed);

		return true;
	}
	catch(std::exception &e)
	{
		LOG("Exception error: %s", e.what());
		return false;
	}
	catch(...)
	{
		LOG("Unknown exception error!");
		return false;
	}
}

bool ThreadPool::trySchedule(ITask *const task)
{
	try
	{
		if(MTHREAD_SEM_TRYWAIT(&m_semFree))
		{
			MTHREAD_MUTEX_LOCK(&m_lock);

			m_taskQueue.push(task);

			MTHREAD_MUTEX_UNLOCK(&m_lock);
			MTHREAD_SEM_POST(&m_semUsed);

			return true;
		}
		else
		{
			return false;
		}
	}
	catch(std::exception &e)
	{
		LOG("Exception error: %s", e.what());
		return false;
	}
	catch(...)
	{
		LOG("Unknown exception error!");
		return false;
	}

}

///////////////////////////////////////////////////////////////////////////////
// Wait for pending tasks
///////////////////////////////////////////////////////////////////////////////

bool ThreadPool::wait(void)
{
	try
	{
		MTHREAD_MUTEX_LOCK(&m_lock);

		while((!m_taskQueue.empty()) || (m_runningTasks > 0))
		{
			MTHREAD_COND_WAIT(&m_condDone, &m_lock);
		}

		MTHREAD_MUTEX_UNLOCK(&m_lock);

		return true;
	}
	catch(std::exception &e)
	{
		LOG("Exception error: %s", e.what());
		return false;
	}
	catch(...)
	{
		LOG("Unknown exception error!");
		return false;
	}

}

///////////////////////////////////////////////////////////////////////////////
// Thread entry point
///////////////////////////////////////////////////////////////////////////////

void *ThreadPool::entryPoint(void *arg)
{
	ThreadPool *const pool = static_cast<ThreadPool*>(arg);

	try
	{
		while(!pool->m_bStopFlag)
		{
			ITask *task = NULL;

			// ------------------------------
			// Fetch Next Task
			// ------------------------------

			MTHREAD_SEM_WAIT(&pool->m_semUsed);
			MTHREAD_MUTEX_LOCK(&pool->m_lock);

			if(!pool->m_taskQueue.empty())
			{
				task = pool->m_taskQueue.front();
				pool->m_taskQueue.pop();
				MTHREAD_SEM_POST(&pool->m_semFree);
			}

			if(task)
			{
				pool->m_runningTasks++;
			}

			MTHREAD_MUTEX_UNLOCK(&pool->m_lock);

			// ------------------------------
			// Execute Task
			// ------------------------------

			if(task && (!pool->m_bStopFlag))
			{
				try
				{
					task->run();
				}
				catch(...)
				{
					LOG("Task %p encountered an internal error!", task);
				}
			}

			// ------------------------------
			// Finalize Task
			// ------------------------------

			if(task)
			{
				MTHREAD_MUTEX_LOCK(&pool->m_lock);
			
				pool->m_runningTasks--;
				if(pool->m_runningTasks == 0)
				{
					MTHREAD_COND_BROADCAST(&pool->m_condDone);
				}

				MTHREAD_MUTEX_UNLOCK(&pool->m_lock);
			}
		}
	}
	catch(std::exception &e)
	{
		LOG("Exception error in worker thread: %s", e.what());
	}
	catch(...)
	{
		LOG("Unknown exception error in worker thread!");
	}

	return NULL;
}
