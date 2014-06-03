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

	class MTHREADPOOL_DLL IListener
	{
	public:
		IListener(void) {}
		virtual ~IListener(void) {}

		virtual void taskLaunched(MTHREADPOOL_NS::ITask *const task) = 0; // <-- Must be implemented in user code!
		virtual void taskFinished(MTHREADPOOL_NS::ITask *const task) = 0; // <-- Must be implemented in user code!
	};

	class MTHREADPOOL_DLL IPool
	{
	public:
		IPool(void) {}
		virtual ~IPool(void) {}

		virtual bool schedule(MTHREADPOOL_NS::ITask *const task) = 0;
		virtual bool trySchedule(MTHREADPOOL_NS::ITask *const task) = 0;

		virtual bool wait(void) = 0;
		virtual bool wait(MTHREADPOOL_NS::ITask *const task) = 0;

		virtual bool addListener(MTHREADPOOL_NS::IListener *const listener) = 0;
		virtual bool removeListener(MTHREADPOOL_NS::IListener *const listener) = 0;
	};
}

///////////////////////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////////////////////

namespace MTHREADPOOL_NS
{
	IPool MTHREADPOOL_DLL *allocatePool(const uint32_t &threadCount = 0, const uint32_t &maxQueueLength = 0);
	bool MTHREADPOOL_DLL destroyPool(IPool *pool);
	const char MTHREADPOOL_DLL *getVersionInfo(uint32_t &vMajor, uint32_t &vMinor, uint32_t &vPatch, bool &bDebug);
}

///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////

#endif //MTHREADPOOL_INCLUDED
