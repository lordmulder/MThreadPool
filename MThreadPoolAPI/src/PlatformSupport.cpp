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

#include "PlatformSupport.h"

#include <cstdio>

#define LOG(X, ...) fprintf(stderr, "[MThreadPool] " X "\n", __VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////
// WIN32
///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace MTHREADPOOL_NS
{
	uint32_t getNumberOfProcessors(void)
	{
		uint32_t numberOfProcessors = 0;

		//Get the CPU affinity mask
		DWORD_PTR affinityMaskPrc = 0, affinityMaskSys = 0;
		if(GetProcessAffinityMask(GetCurrentProcess(), &affinityMaskPrc, &affinityMaskSys))
		{
			for(DWORD_PTR testBit = 1; testBit; testBit <<= 1)
			{
				if(affinityMaskPrc & testBit) numberOfProcessors++;
			}
		}
		else
		{
			LOG("Failed to determine number of processors!");
			numberOfProcessors = 1;
		}

		//If no CPU found, enforce at least one!
		if(numberOfProcessors < 1)
		{
			numberOfProcessors = 1;
		}
	
		return numberOfProcessors;
	}
}

#endif //_WIN32

///////////////////////////////////////////////////////////////////////////////
// LINUX
///////////////////////////////////////////////////////////////////////////////

#ifdef __linux__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <sched.h>

namespace MTHREADPOOL_NS
{
	uint32_t getNumberOfProcessors(void)
	{
		uint32_t numberOfProcessors = 0;

		//Get the CPU affinity mask
		cpu_set_t cpuSet;
		if(sched_getaffinity(0, sizeof(cpu_set_t), &cpuSet) == 0)
		{
			//Count the number of active CPU's
			numberOfProcessors = CPU_COUNT_S(sizeof(cpu_set_t), &cpuSet);
		}
		else
		{
			LOG("Failed to determine number of processors!");
			numberOfProcessors = 1;
		}

		//If no CPU found, enforce at least one!
		if(numberOfProcessors < 1)
		{
			numberOfProcessors = 1;
		}
	
		return numberOfProcessors;
	}
}

#endif //__linux__
