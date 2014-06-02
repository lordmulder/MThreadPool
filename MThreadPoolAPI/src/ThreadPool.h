///////////////////////////////////////////////////////////////////////////////
// MThreadPool - MuldeR's Thread Pool
// Copyright (C) 2014 LoRd_MuldeR <MuldeR2@GMX.de>
// All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MThreadPoolAPI.h"
#include "ThreadUtils.h"

#include <queue>
#include <unordered_map>

namespace MTHREADPOOL_NS
{
	class ThreadPool : public IPool
	{
	public:
		ThreadPool(const uint32_t &threadCount = 0, const uint32_t &maxQueueLength = 0);
		virtual ~ThreadPool(void);

		virtual bool schedule(MTHREADPOOL_NS::ITask *const task);
		virtual bool trySchedule(MTHREADPOOL_NS::ITask *const task);

		virtual bool wait(void);
		virtual bool wait(MTHREADPOOL_NS::ITask *const task);

	private:
		const uint32_t m_threadCount;
		const uint32_t m_maxQueueLength;
		
		uint32_t m_runningTasks;
		uint32_t m_nextCondIndex;
		
		volatile bool m_bStopFlag;

		pthread_t *m_threads;
		sem_t m_semUsed, m_semFree;
		pthread_mutex_t m_lock;
		pthread_cond_t *m_condTaskDone;
		pthread_cond_t m_condAllDone;

		std::queue<MTHREADPOOL_NS::ITask*> m_taskQueue;
		std::unordered_map<MTHREADPOOL_NS::ITask*,pthread_cond_t*> m_taskList;

		static void *entryPoint(void *arg);
	};
}
