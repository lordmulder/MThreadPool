///////////////////////////////////////////////////////////////////////////////
// MThreadPool - MuldeR's Thread Pool
// Copyright (C) 2014 LoRd_MuldeR <MuldeR2@GMX.de>. All rights reserved.
// http://www.muldersoft.com/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version, but always including the *additional*
// restrictions defined in the "License.txt" file.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// http://www.gnu.org/licenses/gpl-2.0.txt
///////////////////////////////////////////////////////////////////////////////

#include "ThreadPool.h"

#include "PlatformSupport.h"

#include <cstdio>

#ifdef _WIN32
#include <vld.h>
#endif

using namespace MTHREADPOOL_NS;

#define LOG(X, ...) fprintf(stderr, "[MThreadPool] " X "\n", __VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////
// Constructor & Destructor
///////////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(const uint32_t &threadCount, const uint32_t &maxQueueLength)
:
	m_threadCount(threadCount ? threadCount : getNumberOfProcessors()),
	m_maxQueueLength(std::max((maxQueueLength ? maxQueueLength : (4 * m_threadCount)), m_threadCount))
{
	//LOG("m_threadCount: %u", m_threadCount);
	//LOG("m_maxQueueLength: %u", m_maxQueueLength);
	
	m_bStopFlag = false;
	m_runningTasks = 0;
	m_nextCondIndex = 0;

	//Create the locks
	MTHREAD_MUTEX_INIT(&m_lockTask);
	MTHREAD_MUTEX_INIT(&m_lockListeners);

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
	MTHREAD_MUTEX_LOCK(&m_lockTask);

	//Do we still have any running/pending tasks?
	if((!m_taskQueue.empty()) || (m_runningTasks > 0))
	{
		LOG("Warning: Destructor called while still have running/pending tasks!");
	}

	MTHREAD_MUTEX_UNLOCK(&m_lockTask);

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
	MTHREAD_MUTEX_DESTROY(&m_lockTask);
	MTHREAD_MUTEX_DESTROY(&m_lockListeners);

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
		MTHREAD_MUTEX_LOCK(&m_lockTask);

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

		MTHREAD_MUTEX_UNLOCK(&m_lockTask);
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
			MTHREAD_MUTEX_LOCK(&m_lockTask);

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

			MTHREAD_MUTEX_UNLOCK(&m_lockTask);
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
		MTHREAD_MUTEX_LOCK(&m_lockTask);

		while((!m_taskQueue.empty()) || (m_runningTasks > 0))
		{
			MTHREAD_COND_WAIT(&m_condAllDone, &m_lockTask);
		}

		MTHREAD_MUTEX_UNLOCK(&m_lockTask);
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
		MTHREAD_MUTEX_LOCK(&m_lockTask);
		
		std::unordered_map<ITask*,pthread_cond_t*>::iterator iter = m_taskList.find(task);

		while(iter != m_taskList.end())
		{
			MTHREAD_COND_WAIT(iter->second, &m_lockTask);
			iter = m_taskList.find(task);
		}

		MTHREAD_MUTEX_UNLOCK(&m_lockTask);
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
// Add or remove listener
///////////////////////////////////////////////////////////////////////////////

bool ThreadPool::addListener(IListener *const listener)
{
	try
	{
		MTHREAD_MUTEX_LOCK(&m_lockListeners);

		if(m_listeners.find(listener) == m_listeners.end())
		{
			m_listeners.insert(listener);
		}
		else
		{
			LOG("Listener %p already registered!", listener);
		}

		MTHREAD_MUTEX_UNLOCK(&m_lockListeners);
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

bool ThreadPool::removeListener(IListener *const listener)
{
	try
	{
		MTHREAD_MUTEX_LOCK(&m_lockListeners);

		std::set<IListener*>::iterator iter = m_listeners.find(listener);

		if(iter != m_listeners.end())
		{
			m_listeners.erase(iter);
		}
		else
		{
			LOG("Listener %p not found!", listener);
		}

		MTHREAD_MUTEX_UNLOCK(&m_lockListeners);
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
	try
	{
		processingLoop(static_cast<ThreadPool*>(arg));
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

///////////////////////////////////////////////////////////////////////////////
// Processing loop
///////////////////////////////////////////////////////////////////////////////

void ThreadPool::processingLoop(ThreadPool* pool)
{
	while(!pool->m_bStopFlag)
	{
		ITask *task = fetchNextTask(pool);

		if(task)
		{
			notifyListeners(pool, task, false);

			try
			{
				task->run();
			}
			catch(...)
			{
				LOG("Task %p encountered an internal error!", task);
			}

			notifyListeners(pool, task, true);
			finalizeTask(pool, task);
		}
	}
}

ITask *ThreadPool::fetchNextTask(MTHREADPOOL_NS::ThreadPool* pool)
{
	ITask *task = NULL;

	MTHREAD_SEM_WAIT(&pool->m_semUsed);
	MTHREAD_MUTEX_LOCK(&pool->m_lockTask);

	if(!(pool->m_taskQueue.empty() || pool->m_bStopFlag))
	{
		task = pool->m_taskQueue.front();
		pool->m_taskQueue.pop();
		MTHREAD_SEM_POST(&pool->m_semFree);
	}

	if(task)
	{
		pool->m_runningTasks++;
	}

	MTHREAD_MUTEX_UNLOCK(&pool->m_lockTask);
	return task;
}

void ThreadPool::finalizeTask(MTHREADPOOL_NS::ThreadPool* pool, ITask* task)
{
	MTHREAD_MUTEX_LOCK(&pool->m_lockTask);

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

	MTHREAD_MUTEX_UNLOCK(&pool->m_lockTask);
}

void ThreadPool::notifyListeners(ThreadPool* pool, ITask* task, const bool &finished)
{
	MTHREAD_MUTEX_LOCK(&pool->m_lockListeners);

	for(std::set<IListener*>::iterator iter = pool->m_listeners.begin(); iter != pool->m_listeners.end(); iter++)
	{
		if(finished)
		{
			(*iter)->taskFinished(task);
		}
		else
		{
			(*iter)->taskLaunched(task);
		}
	}

	MTHREAD_MUTEX_UNLOCK(&pool->m_lockListeners);
}
