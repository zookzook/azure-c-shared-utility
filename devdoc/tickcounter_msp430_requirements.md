tickcounter
=============

## Overview
`tickcounter_msp430` implements a tick counter for the MSP430 board using the TI `driverlib` utility.



## References

[Texas Instruments MSP430FR5969 Microcontroller](http://www.ti.com/lit/pdf/slau367)  
[MSP Driver Library official page](http://www.ti.com/tool/mspdriverlib)  
[Azure IoT SDK `tickcounter.h` Header](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/tickcounter.h)  



## API Exposed via `tickcounter.h`


### tickcounter_create
```c
extern TICK_COUNTER_HANDLE tickcounter_create (void);
```

**SRS_UARTIO_27_000: [** `tickcounter_create()` shall allocate its context by calling `(void *)malloc(void)`. **]**  
**SRS_UARTIO_27_001: [** If the call to `malloc()` fails, `tickcounter_create()` shall fail and return `NULL`. **]**  
**SRS_UARTIO_27_002: [** If the call to `malloc()` succeeds, `tickcounter_create()` shall query its creation offset by calling `Timer_A_getCounterValue()`. **]**  


### tickcounter_destroy
```c
extern void tickcounter_destroy (TICK_COUNTER_HANDLE tick_counter);
```

**SRS_UARTIO_27_003: [** If a `NULL` handle is provided, `tickcounter_destroy()` shall do nothing. **]**  
**SRS_UARTIO_27_004: [** `tickcounter_destroy()` shall release the context of the specified by `TICK_COUNTER_HANDLE` by calling `(void)free(void *)` using the handle value. **]**  


### tickcounter_get_current_ms
```c
extern int tickcounter_get_current_ms (TICK_COUNTER_HANDLE tick_counter, tick_t * current_ms);
```

**SRS_UARTIO_27_005: [** If a `NULL` handle is provided, `tickcounter_get_current_ms()` shall fail and return a non zero value. **]**  
**SRS_UARTIO_27_006: [** If a `NULL` is passed in `current_ms`, `tickcounter_get_current_ms()` shall fail and return a non zero value. **]**  
**SRS_UARTIO_27_007: [** `tickcounter_get_current_ms()` shall query the current tick count by calling `(uint16_t)Timer_A_getCounterValue(uint16_t baseAddress)` using `TIMER_A3_BASE` as `baseAddress`. **]**  
**SRS_UARTIO_27_008: [** `tickcounter_get_current_ms()` shall return the number of milliseconds between that have passed between the time `tickcounter_create()` was called and now. **]**  



## API Exposed via `tickcounter_msp430.h` extension  


### timer_a3_deinit
```c
extern void timer_a3_deinit (void);
```

**SRS_UARTIO_27_009: [** `timer_a3_deinit()` shall disable the interrupt by calling `(void)Timer_A_disableInterrupt(uint16_t baseAddress)` using `TIMER_A3_BASE` as `baseAddress`. **]**  
**SRS_UARTIO_27_010: [** `timer_a3_deinit()` shall stop the underlying timer by calling `(void)Timer_A_stop(uint16_t baseAddress)` using `TIMER_A3_BASE` as `baseAddress`. **]**  


### timer_a3_init
```c
extern void timer_a3_init (void);
```

**SRS_UARTIO_27_011: [** `timer_a3_init()` shall enable clock request for the auxillary clock by calling `(void)CS_enableClockRequest(uint8_t selectClock)` using the `CS_ACLK` identifier. **]**  
**SRS_UARTIO_27_012: [** `timer_a3_init()` shall determine the clock speed of the auxillary clock by calling `(uint32_t)CS_getACLK(void)`. **]**  
**SRS_UARTIO_27_013: [** `timer_a3_init()` shall call `(void)Timer_A_initContinuousMode(uint16_t baseAddress, Timer_A_initContinuousModeParam *param)` using `TIMER_A_CLOCKSOURCE_ACLK` as the first member of the initialization parameters. **]**  
**SRS_UARTIO_27_014: [** `timer_a3_init()` shall call `(void)Timer_A_initContinuousMode(uint16_t baseAddress, Timer_A_initContinuousModeParam *param)` using `TIMER_A_CLOCKSOURCE_DIVIDER_16` as the second member of the initialization parameters. **]**  
**SRS_UARTIO_27_015: [** `timer_a3_init()` shall call `(void)Timer_A_initContinuousMode(uint16_t baseAddress, Timer_A_initContinuousModeParam *param)` using `TIMER_A_TAIE_INTERRUPT_ENABLE` as the third member of the initialization parameters. **]**  
**SRS_UARTIO_27_016: [** `timer_a3_init()` shall call `(void)Timer_A_initContinuousMode(uint16_t baseAddress, Timer_A_initContinuousModeParam *param)` using `TIMER_A_SKIP_CLEAR` as the fourth member of the initialization parameters. **]**  
**SRS_UARTIO_27_017: [** `timer_a3_init()` shall call `(void)Timer_A_initContinuousMode(uint16_t baseAddress, Timer_A_initContinuousModeParam *param)` using `true` as the fifth member of the initialization parameters. **]**  

