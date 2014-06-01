///////////////////////////////////////////////////////////////////////////////
// MThreadPool - MuldeR's Thread Pool
// Copyright (C) 2014 LoRd_MuldeR <MuldeR2@GMX.de>
// All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#include "MThreadPoolAPI.h"

using namespace MTHREADPOOL_NS;

///////////////////////////////////////////////////////////////////////////////
// Version info
///////////////////////////////////////////////////////////////////////////////

static const uint32_t MTHREADPOOL_VERSION_MAJOR = 1;
static const uint32_t MTHREADPOOL_VERSION_MINOR = 0;
static const uint32_t MTHREADPOOL_VERSION_PATCH = 0;

static const char *MTHREADPOOL_VERSION_DATE = __DATE__;

#if defined(NDEBUG) && (!defined(_DEBUG))
static const bool MTHREADPOOL_VERSION_DEBUG = false;
#else
static const bool MTHREADPOOL_VERSION_DEBUG = true;
#endif

///////////////////////////////////////////////////////////////////////////////
// Allocate new pool
///////////////////////////////////////////////////////////////////////////////

IPool *MTHREADPOOL_NS::allocatePool(const uint32_t &nThreads)
{
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Destroy pool
///////////////////////////////////////////////////////////////////////////////

bool MTHREADPOOL_NS::destroyPool(IPool *pool)
{
	try
	{
		if(pool)
		{
			delete pool;
			return true;
		}
		else
		{
			return false;
		}
	}
	catch(...)
	{
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Version info
///////////////////////////////////////////////////////////////////////////////

const char* MTHREADPOOL_NS::getVersionInfo(uint32_t &vMajor, uint32_t &vMinor, uint32_t &vPatch, bool &bDebug)
{
	vMajor = MTHREADPOOL_VERSION_MAJOR;
	vMinor = MTHREADPOOL_VERSION_MINOR;
	vPatch = MTHREADPOOL_VERSION_PATCH;
	bDebug = MTHREADPOOL_VERSION_DEBUG;

	return MTHREADPOOL_VERSION_DATE;
}
