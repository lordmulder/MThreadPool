///////////////////////////////////////////////////////////////////////////////
// MThreadPool - MuldeR's Thread Pool
// Copyright (C) 2014 LoRd_MuldeR <MuldeR2@GMX.de>
// All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#include <cstdio>

#include "MThreadPoolAPI.h"

#include <sha1.h>
#include <cstring>
#include <queue>

///////////////////////////////////////////////////////////////////////////////
// Task
///////////////////////////////////////////////////////////////////////////////

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

		for(int i = 0; i < 8*SHRT_MAX; i++)
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
// Main
///////////////////////////////////////////////////////////////////////////////

int wmain(int argc, wchar_t* argv[])
{
	static const int MAX_RUNS = 1000;

	uint32_t vMajor, vMinor, vPatch;
	bool bDebug;
	const char *date = MTHREADPOOL_NS::getVersionInfo(vMajor, vMinor, vPatch, bDebug);

	printf("MThreadPool Test [%s]\n\n", __DATE__);
	printf("Using MThreadPool library v%u.%02u-%u, built on %s, %s\n\n", vMajor, vMinor, vPatch, date, bDebug ? "Debug" : "Release");
	
	for(int j = 0; j < MAX_RUNS; j++)
	{
		printf("[Run %d of %d]\n", j+1, MAX_RUNS);

		std::queue<MTHREADPOOL_NS::ITask*> tasks;
		MTHREADPOOL_NS::IPool *pool = MTHREADPOOL_NS::allocatePool(4);

		for(int i = 0; i < 256; i++)
		{
			tasks.push(new MyTask(i));
			if(!pool->schedule(tasks.back()))
			{
				printf("Scheduling has failed!\n");
			}
		}

		pool->wait();

		MTHREADPOOL_NS::destroyPool(pool);

		while(tasks.empty())
		{
			MTHREADPOOL_NS::ITask *t = tasks.front();
			tasks.pop();
			delete t;
		}

		printf("\n--------\n\n");
	}

	printf("\nDone.\n");
	return 0;
}

