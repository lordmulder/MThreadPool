///////////////////////////////////////////////////////////////////////////////
// MThreadPool - MuldeR's Thread Pool
// Copyright (C) 2014 LoRd_MuldeR <MuldeR2@GMX.de>
// All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#ifndef MTHREADPOOL_INCLUDED
#define MTHREADPOOL_INCLUDED

#ifdef _MSC_VER
#  ifdef MTHREADPOOL_EXPORTS
#    define MTHREADPOOL_DLL __declspec(dllexport)
#  else
#    define MTHREADPOOL_DLL __declspec(dllimport)
#  endif
#else
#  ifdef __GNUG__
#    define MTHREADPOOL_DLL __attribute__ ((visibility ("default")))
#  else
#    define MTHREADPOOL_DLL
#  endif
#endif

#define MTHREADPOOL_NS MThreadPool_r1

#include <cstdlib>
#include <cstdint>

///////////////////////////////////////////////////////////////////////////////
// Interfaces
///////////////////////////////////////////////////////////////////////////////

namespace MTHREADPOOL_NS
{
	class MTHREADPOOL_DLL ITask
	{
	public:
		ITask(void) {}
		virtual ~ITask(void) {}

		virtual void run(void) = 0; // <-- Must be implemented in user code!
	};

	class MTHREADPOOL_DLL IPool
	{
	public:
		IPool(void) {}
		virtual ~IPool(void) {}

		virtual bool schedule(ITask *const task) = 0;
		virtual bool trySchedule(ITask *const task) = 0;

		virtual bool wait(void) = 0;
	};
}

///////////////////////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////////////////////

namespace MTHREADPOOL_NS
{
	IPool MTHREADPOOL_DLL *allocatePool(const uint32_t &nThreads = 0);
	bool MTHREADPOOL_DLL destroyPool(IPool *pool);
	const char MTHREADPOOL_DLL *getVersionInfo(uint32_t &vMajor, uint32_t &vMinor, uint32_t &vPatch, bool &bDebug);
}

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////

#endif //MTHREADPOOL_INCLUDED
