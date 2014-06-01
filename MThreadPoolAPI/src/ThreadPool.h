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

	private:
		const uint32_t m_nThreads;
		pthread_t *m_threads;
		std::queue<MTHREADPOOL_NS::ITask*> m_taskQueue;
		volatile bool m_bStopFlag;

		static void *entryPoint(void *arg);
	};
}
