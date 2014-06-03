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

#include "MThreadPoolAPI.h"
#include "ThreadPool.h"

using namespace MTHREADPOOL_NS;

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////

static const uint32_t MTHREADPOOL_VERSION_MAJOR = 1;
static const uint32_t MTHREADPOOL_VERSION_MINOR = 1;
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

IPool *MTHREADPOOL_NS::allocatePool(const uint32_t &threadCount, const uint32_t &maxQueueLength)
{
	IPool *pool = NULL;

	try
	{
		pool = new ThreadPool(threadCount, maxQueueLength);
	}
	catch(...)
	{
		pool = NULL;
	}

	return pool;
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
