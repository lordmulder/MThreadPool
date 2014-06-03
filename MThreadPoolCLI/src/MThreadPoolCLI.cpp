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

#include <cstdio>

#include "MThreadPoolAPI.h"

#include <sha1.h>
#include <cstring>
#include <queue>

#ifdef _WIN32
#include <vld.h>
#endif

///////////////////////////////////////////////////////////////////////////////
// Task
///////////////////////////////////////////////////////////////////////////////

#ifdef NDEBUG
static const int TASK_LOOPS = 524288;
#else
static const int TASK_LOOPS = 4096;
#endif

class MyTask : public MTHREADPOOL_NS::ITask
{
public:
	MyTask(const uint32_t &seed)
	:
		m_seed(seed)
	{
		/*nothing to do here*/
	}

	virtual void run()
	{
		char hexstring[41];
		unsigned char hash[20], temp[20];

		memset(hash, 0, 20 * sizeof(unsigned char));
		memset(temp, 0, 20 * sizeof(unsigned char));

		memcpy(temp, &m_seed, sizeof(uint32_t));

		for(int i = 0; i < TASK_LOOPS; i++)
		{
			sha1::calc(temp, 20, hash);
			memcpy(temp, hash, 20 * sizeof(unsigned char));
		}

		sha1::toHexString(hash, hexstring);
		printf("Result is %s\n", hexstring);
	}

protected:
	const uint32_t m_seed;
};

///////////////////////////////////////////////////////////////////////////////
// Listener
///////////////////////////////////////////////////////////////////////////////

class MyListener : public MTHREADPOOL_NS::IListener
{
public:
	virtual void taskLaunched(MTHREADPOOL_NS::ITask *const task)
	{
		printf("Task %p started.\n", task);
	}

	virtual void taskFinished(MTHREADPOOL_NS::ITask *const task)
	{
		printf("Task %p finished.\n", task);
	}
};

///////////////////////////////////////////////////////////////////////////////
// Main
///////////////////////////////////////////////////////////////////////////////

int wmain(int argc, wchar_t* argv[])
{
	static const int MAX_RUNS = 8;
	static const int TASK_COUNT = 256;

	uint32_t vMajor, vMinor, vPatch;
	bool bDebug;
	const char *date = MTHREADPOOL_NS::getVersionInfo(vMajor, vMinor, vPatch, bDebug);

	printf("MThreadPool Test [%s]\n\n", __DATE__);
	printf("Using MThreadPool library v%u.%02u-%u, built on %s, %s\n\n", vMajor, vMinor, vPatch, date, bDebug ? "Debug" : "Release");
	
	MyListener listener;

	MyTask **tasks = new MyTask*[TASK_COUNT];
	for(int i = 0; i < TASK_COUNT; i++)
	{
		tasks[i] = new MyTask(i);
	}

	for(int j = 0; j < MAX_RUNS; j++)
	{
		printf("[Run %d of %d]\n", j+1, MAX_RUNS);

		MTHREADPOOL_NS::IPool *pool = MTHREADPOOL_NS::allocatePool();
		pool->addListener(&listener);

		for(int i = 0; i < TASK_COUNT; i++)
		{
			if(!pool->schedule(tasks[i]))
			{
				printf("Scheduling has failed!\n");
			}
		}

		printf("Synchronizing...\n");
		pool->wait();

		pool->removeListener(&listener);
		MTHREADPOOL_NS::destroyPool(pool);

		printf("\n--------\n\n");
	}

	for(int i = 0; i < TASK_COUNT; i++)
	{
		delete tasks[i];
	}
	delete [] tasks;

	printf("\nDone.\n");
	return 0;
}

