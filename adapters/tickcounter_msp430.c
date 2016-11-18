// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/tickcounter_msp430.h"

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xlogging.h"

TICK_COUNTER_HANDLE tickcounter_create(void)
{
    return (TICK_COUNTER_HANDLE)NULL;
}

void timer_a3_deinit(void)
{

}

void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter)
{
    (void)tick_counter;
}

void timer_a3_init(void)
{

}

int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t * current_ms)
{
    (void)tick_counter, current_ms;
    return __LINE__;
}
