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

ThreadPool::ThreadPool(const uint32_t &threadCount, const uint32_t &maxQueueLength)
:
	m_threadCount(threadCount),
	m_maxQueueLength(std::max((maxQueueLength ? maxQueueLength : (4 * m_threadCount)), m_threadCount))
{
	//LOG("m_threadCount: %u", m_threadCount);
	//LOG("m_maxQueueLength: %u", m_maxQueueLength);
	
	m_bStopFlag = false;
	m_runningTasks = 0;
	m_nextCondIndex = 0;

	//Create the lock
	MTHREAD_MUTEX_INIT(&m_lock);

	//Create semaphores
	MTHREAD_SEM_INIT(&m_semFree, m_maxQueueLength);
	MTHREAD_SEM_INIT(&m_semUsed, 0);

	//Create global conditional var
	MTHREAD_COND_INIT(&m_condAllDone);

	//Allocate per-task conditional vars
	m_condTaskDone = new pthread_cond_t[m_threadCount + m_maxQueueLength];
	memset(m_condTaskDone, 0, sizeof(pthread_cond_t) * (m_threadCount + m_maxQueueLength));
	for(uint32_t i = 0; i < (m_threadCount + m_maxQueueLength); i++)
	{
		MTHREAD_COND_INIT(&m_condTaskDone[i]);
	}

	//Create the threads
	m_threads = new pthread_t[m_threadCount];
	memset(m_threads, 0, sizeof(pthread_t) * m_threadCount);
	for(uint32_t i = 0; i < m_threadCount; i++)
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
	MTHREAD_SEM_POST(&m_semUsed, m_threadCount);

	//Wait for threads to exit
	for(uint32_t i = 0; i < m_threadCount; i++)
	{
		MTHREAD_JOIN(m_threads[i]);
	}

	//Delete thread array
	if(m_threads)
	{
		delete [] m_threads;
		m_threads = NULL;
	}

	//Destroy conditional vars
	for(uint32_t i = 0; i < (m_threadCount + m_maxQueueLength); i++)
	{
		MTHREAD_COND_DESTROY(&m_condTaskDone[i]);
	}

	//Delete conditional var array
	if(m_condTaskDone)
	{
		delete [] m_condTaskDone;
		m_condTaskDone = NULL;
	}

	//Destroy conditional var
	MTHREAD_COND_DESTROY(&m_condAllDone);

	//Destroy semaphores
	MTHREAD_SEM_DESTROY(&m_semFree);
	MTHREAD_SEM_DESTROY(&m_semUsed);

	//Destroy the lock
	MTHREAD_MUTEX_DESTROY(&m_lock);

	//Clear pending tasks
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

		if(m_taskList.find(task) == m_taskList.end())
		{
			m_taskList.insert(std::make_pair(task, &m_condTaskDone[m_nextCondIndex]));
			m_nextCondIndex = (m_nextCondIndex + 1) % (m_threadCount + m_maxQueueLength);
			m_taskQueue.push(task);
			MTHREAD_SEM_POST(&m_semUsed);
		}
		else
		{
			LOG("Task %p has already been scheduled!", task);
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

bool ThreadPool::trySchedule(ITask *const task)
{
	try
	{
		if(MTHREAD_SEM_TRYWAIT(&m_semFree))
		{
			MTHREAD_MUTEX_LOCK(&m_lock);

			if(m_taskList.find(task) == m_taskList.end())
			{
				m_taskList.insert(std::make_pair(task, &m_condTaskDone[m_nextCondIndex]));
				m_nextCondIndex = (m_nextCondIndex + 1) % (m_threadCount + m_maxQueueLength);
				m_taskQueue.push(task);
				MTHREAD_SEM_POST(&m_semUsed);
			}
			else
			{
				LOG("Task %p has already been scheduled!", task);
			}

			MTHREAD_MUTEX_UNLOCK(&m_lock);
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
			MTHREAD_COND_WAIT(&m_condAllDone, &m_lock);
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

bool ThreadPool::wait(MTHREADPOOL_NS::ITask *const task)
{
	try
	{
		MTHREAD_MUTEX_LOCK(&m_lock);
		
		std::unordered_map<ITask*,pthread_cond_t*>::iterator iter = m_taskList.find(task);

		while(iter != m_taskList.end())
		{
			MTHREAD_COND_WAIT(iter->second, &m_lock);
			iter = m_taskList.find(task);
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
				std::unordered_map<ITask*,pthread_cond_t*>::iterator iter = pool->m_taskList.find(task);

				if(iter != pool->m_taskList.end())
				{
					MTHREAD_COND_BROADCAST(iter->second);
					pool->m_taskList.erase(iter);
				}

				if(pool->m_runningTasks == 0)
				{
					MTHREAD_COND_BROADCAST(&pool->m_condAllDone);
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
