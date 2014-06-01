///////////////////////////////////////////////////////////////////////////////
// MThreadPool - MuldeR's Thread Pool
// Copyright (C) 2014 LoRd_MuldeR <MuldeR2@GMX.de>
// All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MThreadPoolAPI.h"
#include "ThreadUtils.h"

#include <queue>

namespace MTHREADPOOL_NS
{
	class ThreadPool : public IPool
	{
	public:
		ThreadPool(const uint32_t &nThreads);
		virtual ~ThreadPool(void);

		virtual bool schedule(ITask *const task);
		virtual bool trySchedule(ITask *const task);

		virtual bool wait(void);

	private:
		const uint32_t m_nThreads;
		uint32_t m_runningTasks;
		volatile bool m_bStopFlag;

		pthread_t *m_threads;
		sem_t m_semUsed, m_semFree;
		pthread_mutex_t m_lock;
		pthread_cond_t m_condDone;

		std::queue<MTHREADPOOL_NS::ITask*> m_taskQueue;

		static void *entryPoint(void *arg);
	};
}
