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

#pragma once

#include "MThreadPoolAPI.h"
#include "ThreadUtils.h"

#include <queue>
#include <unordered_map>
#include <set>

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

		virtual bool addListener(MTHREADPOOL_NS::IListener *const listener);
		virtual bool removeListener(MTHREADPOOL_NS::IListener *const listener);

	private:
		volatile bool m_bStopFlag;

		const uint32_t m_threadCount;
		const uint32_t m_maxQueueLength;
		
		uint32_t m_runningTasks;
		uint32_t m_nextCondIndex;

		pthread_t *m_threads;

		sem_t m_semUsed;
		sem_t m_semFree;

		pthread_mutex_t m_lockTask;
		pthread_mutex_t m_lockListeners;

		pthread_cond_t *m_condTaskDone;
		pthread_cond_t m_condAllDone;

		std::queue<MTHREADPOOL_NS::ITask*> m_taskQueue;
		std::unordered_map<MTHREADPOOL_NS::ITask*,pthread_cond_t*> m_taskList;
		std::set<MTHREADPOOL_NS::IListener*> m_listeners;

		static void *entryPoint(void *arg);
		static void processingLoop(MTHREADPOOL_NS::ThreadPool* pool);

		static inline MTHREADPOOL_NS::ITask *fetchNextTask(MTHREADPOOL_NS::ThreadPool* pool);
		static inline void finalizeTask(MTHREADPOOL_NS::ThreadPool* pool, ITask* task);
		static inline void notifyListeners(MTHREADPOOL_NS::ThreadPool* pool, MTHREADPOOL_NS::ITask* task, const bool &finished);
	};
}
