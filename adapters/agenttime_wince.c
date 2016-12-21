// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "azure_c_shared_utility/gballoc.h"

#include <time.h>
#include <windows.h>
#include "azure_c_shared_utility/agenttime.h"

time_t time(time_t* inTT)
{
	SYSTEMTIME sysTimeStruct;
	FILETIME fTime;
	ULARGE_INTEGER int64time;

	time_t locTT = 0;
	
	if (inTT == NULL)
	{
		inTT = &locTT;
	}

	GetSystemTime(&sysTimeStruct);
	if (SystemTimeToFileTime(&sysTimeStruct, &fTime))
	{
		memcpy(&int64time, &fTime, sizeof(FILETIME));
		/* Subtract the value for 1970-01-01 00:00 (UTC) */
		int64time.QuadPart -= 0x19db1ded53e8000;
		
		/* Convert to seconds. */
		int64time.QuadPart /= 10000000;

		*inTT = (time_t)int64time.QuadPart;
	}

	return *inTT;
}

double difftime(time_t stopTime, time_t startTime)
{
    return stopTime - startTime;
}

char* ctime(time_t* timeToGet)
{
    return NULL;
}

time_t get_time(time_t* p)
{
    return time(p);
}

struct tm* get_gmtime(time_t* currentTime)
{
    return NULL;
}

time_t get_mktime(struct tm* cal_time)
{
	return (time_t)0;
}

char* get_ctime(time_t* timeToGet)
{
    return ctime(timeToGet);
}

double get_difftime(time_t stopTime, time_t startTime)
{
    return difftime(stopTime, startTime);
}