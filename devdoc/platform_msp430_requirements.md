platform
=============

## Overview
`platform_msp430` implements the platform abstraction for the MSP430 board using the TI `driverlib` utility.



## References

[Texas Instruments MSP430FR5969 Microcontroller](http://www.ti.com/lit/pdf/slau367)  
[MSP Driver Library official page](http://www.ti.com/tool/mspdriverlib)  
[Azure IoT SDK `platform.h` Header](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/platform.h)  
[XIO Interface](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/xio.h)  



## API Exposed via `platform.h`


### platform_init
```c
extern int platform_init (void);
```

**SRS_UARTIO_27_000: [** `platform_init()` shall initialize the timer submodule by calling `(int)timer_a3_init(void)` **]**  
**SRS_UARTIO_27_001: [** If the timer submodule fails to initialize properly, then `platform_init()` shall fail immediately and return a non-zero value **]**  
**SRS_UARTIO_27_002: [** `platform_init()` shall create a tickcounter instance by calling `(TICK_COUNTER_HANDLE)tickcounter_create()` **]**  
**SRS_UARTIO_27_003: [** `platform_init()` shall prepare to read the SIM808 status pin by calling `(void)GPIO_setAsInputPin(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P3` for the `selectedPort` and `GPIO_PIN5` as the `selectedPins` **]**  
**SRS_UARTIO_27_004: [** `platform_init()` shall prepare the SIM808 PWRKEY pin by calling `(void)GPIO_setAsOutputPin(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P4` for the `selectedPort` and `GPIO_PIN6` as the `selectedPins` **]**  
**SRS_UARTIO_27_005: [** `platform_init()` shall set the PWRKEY pin to low (inactive) by calling `(void)GPIO_setOutputLowOnPin(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P4` for the `selectedPort` and `GPIO_PIN6` as the `selectedPins` **]**  
**SRS_UARTIO_27_006: [** `platform_init()` shall check the status line of the SIM808 by calling `(uint8_t)GPIO_getInputPinValue(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P3` for the `selectedPort` and `GPIO_PIN5` as the `selectedPins` **]**  
**SRS_UARTIO_27_007: [** If the SIM808 status line equals `GPIO_INPUT_PIN_LOW`, then `platform_init()` shall configure an interrupt by calling `(void)GPIO_selectInterruptEdge(uint8_t selectedPort, uint16_t selectedPins, uint8_t edgeSelect)` using `GPIO_PORT_P3` for the `selectedPort`, `GPIO_PIN5` as the `selectedPins` and `GPIO_LOW_TO_HIGH_TRANSITION` as the `edgeSelect` **]**  
**SRS_UARTIO_27_008: [** If the SIM808 status line equals `GPIO_INPUT_PIN_LOW`, then `platform_init()` shall enable the newly configured interrupt by calling `(void)GPIO_enableInterrupt(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P3` for the `selectedPort` and `GPIO_PIN5` as the `selectedPins` **]**  
**SRS_UARTIO_27_009: [** If the SIM808 status line equals `GPIO_INPUT_PIN_LOW`, then `platform_init()` shall mark time by calling `(int)tickcounter_get_current_ms(TICK_COUNTER_HANDLE handle, tickcounter_ms_t * tick_counter)` **]**  
**SRS_UARTIO_27_010: [** If the SIM808 status line equals `GPIO_INPUT_PIN_LOW`, then `platform_init()` shall poll `(int)tickcounter_get_current_ms(TICK_COUNTER_HANDLE handle, tickcounter_ms_t * tick_counter)` until it returns a result greater than or equal to 550 milliseconds from the mark **]**  
**SRS_UARTIO_27_011: [** If the SIM808 status line equals `GPIO_INPUT_PIN_LOW`, then `platform_init()` shall set the PWRKEY pin to high (active) by calling `(void)GPIO_setOutputHighOnPin(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P4` for the `selectedPort` and `GPIO_PIN6` as the `selectedPins` **]**  
**SRS_UARTIO_27_012: [** If the SIM808 status line equals `GPIO_INPUT_PIN_LOW`, then `platform_init()` shall poll `(int)tickcounter_get_current_ms(TICK_COUNTER_HANDLE handle, tickcounter_ms_t * tick_counter)` until it returns a result greater than or equal to 1100 milliseconds from the mark **]**  
**SRS_UARTIO_27_013: [** If any call to `(int)tickcounter_get_current_ms(TICK_COUNTER_HANDLE handle, tickcounter_ms_t * tick_counter)` returns a non-zero value, then `platform_init()` will exit immediately and return a non-zero value **]**  
**SRS_UARTIO_27_014: [** If the SIM808 status line equals `GPIO_INPUT_PIN_LOW`, then `platform_init()` shall set the PWRKEY pin to low (inactive) by calling `(void)GPIO_setOutputLowOnPin(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P4` for the `selectedPort` and `GPIO_PIN6` as the `selectedPins` **]**  
**SRS_UARTIO_27_015: [** `platform_init()` shall configure the GPIO for UART by calling `(void)GPIO_setAsPeripheralModuleFunctionOutputPin(uint8_t selectedPort, uint16_t selectedPins, uint8_t mode)` using port two, `GPIO_PORT_P2`, pins `GPIO_PIN5 & GPIO_PIN6`, and UART functionality with the `GPIO_SECONDARY_MODULE_FUNCTION` identifier. **]**  
**SRS_UARTIO_27_016: [** `platform_init()` shall destroy the tickcounter instance by calling `(void)tickcounter_destroy(TICK_COUNTER_HANDLE)` **]**  
**SRS_UARTIO_27_017: [** `platform_init()` shall block until the interrupt on `GPIO_PORT_P3`, `GPIO_PIN5` has fired **]**  
**SRS_UARTIO_27_018: [** If no errors occurred during execution, `platform_init()` shall return 0 **]**  


### platform_deinit
```c
extern void platform_deinit (void);
```

**SRS_UARTIO_27_019: [** `platform_deinit()` shall create a tickcounter instance by calling `(TICK_COUNTER_HANDLE)tickcounter_create()` **]**  
**SRS_UARTIO_27_020: [** `platform_deinit()` shall prepare to read the SIM808 status pin by calling `(void)GPIO_setAsInputPin(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P3` for the `selectedPort` and `GPIO_PIN5` as the `selectedPins` **]**  
**SRS_UARTIO_27_021: [** `platform_deinit()` shall prepare the SIM808 PWRKEY pin by calling `(void)GPIO_setAsOutputPin(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P4` for the `selectedPort` and `GPIO_PIN6` as the `selectedPins` **]**  
**SRS_UARTIO_27_022: [** `platform_deinit()` shall set the PWRKEY pin to low (inactive) by calling `(void)GPIO_setOutputLowOnPin(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P4` for the `selectedPort` and `GPIO_PIN6` as the `selectedPins` **]**  
**SRS_UARTIO_27_023: [** `platform_deinit()` shall check the status line of the SIM808 by calling `(uint8_t)GPIO_getInputPinValue(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P3` for the `selectedPort` and `GPIO_PIN5` as the `selectedPins` **]**  
**SRS_UARTIO_27_024: [** If the SIM808 status line equals `GPIO_INPUT_PIN_HIGH`, then `platform_deinit()` shall configure an interrupt by calling `(void)GPIO_selectInterruptEdge(uint8_t selectedPort, uint16_t selectedPins, uint8_t edgeSelect)` using `GPIO_PORT_P3` for the `selectedPort`, `GPIO_PIN5` as the `selectedPins` and `GPIO_HIGH_TO_LOW_TRANSITION` as the `edgeSelect` **]**  
**SRS_UARTIO_27_025: [** If the SIM808 status line equals `GPIO_INPUT_PIN_HIGH`, then `platform_deinit()` shall enable the newly configured interrupt by calling `(void)GPIO_enableInterrupt(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P3` for the `selectedPort` and `GPIO_PIN5` as the `selectedPins` **]**  
**SRS_UARTIO_27_026: [** If the SIM808 status line equals `GPIO_INPUT_PIN_HIGH`, then `platform_deinit()` shall set the PWRKEY pin to high (active) by calling `(void)GPIO_setOutputHighOnPin(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P4` for the `selectedPort` and `GPIO_PIN6` as the `selectedPins` **]**  
**SRS_UARTIO_27_027: [** If the SIM808 status line equals `GPIO_INPUT_PIN_HIGH`, then `platform_deinit()` shall mark time by calling `(int)tickcounter_get_current_ms(TICK_COUNTER_HANDLE handle, tickcounter_ms_t * tick_counter)` **]**  
**SRS_UARTIO_27_028: [** If the SIM808 status line equals `GPIO_INPUT_PIN_HIGH`, then `platform_deinit()` shall poll `(int)tickcounter_get_current_ms(TICK_COUNTER_HANDLE handle, tickcounter_ms_t * tick_counter)` until it returns a result greater than or equal to 1100 milliseconds from the mark **]**  
**SRS_UARTIO_27_029: [** If any call to `(int)tickcounter_get_current_ms(TICK_COUNTER_HANDLE handle, tickcounter_ms_t * tick_counter)` returns a non-zero value, then `platform_deinit()` will exit immediately **]**  
**SRS_UARTIO_27_030: [** If the SIM808 status line equals `GPIO_INPUT_PIN_HIGH`, then `platform_deinit()` shall set the PWRKEY pin to low (inactive) by calling `(void)GPIO_setOutputLowOnPin(uint8_t selectedPort, uint16_t selectedPins)` using `GPIO_PORT_P4` for the `selectedPort` and `GPIO_PIN6` as the `selectedPins` **]**  
**SRS_UARTIO_27_031: [** `platform_deinit()` shall destroy the tickcounter instance by calling `(void)tickcounter_destroy(TICK_COUNTER_HANDLE)` **]**  
**SRS_UARTIO_27_032: [** `platform_deinit()` shall deinitialize the timer submodule by calling `(void)timer_a3_deinit(void)` **]**  
**SRS_UARTIO_27_033: [** `platform_deinit()` shall block until the interrupt on `GPIO_PORT_P3`, `GPIO_PIN5` has fired **]**  


### platform_get_default_tlsio
```c
extern const IO_INTERFACE_DESCRIPTION * platform_get_default_tlsio (void);
```

**SRS_UARTIO_27_034: [** `platform_get_default_tlsio()` shall return a non-`NULL` pointer to an `IO_INTERFACE_DESCRIPTION` structure. **]**  
**SRS_UARTIO_27_035: [** `platform_get_default_tlsio()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `tlsio_close()`. **]**  
**SRS_UARTIO_27_036: [** `platform_get_default_tlsio()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `tlsio_create()`. **]**  
**SRS_UARTIO_27_037: [** `platform_get_default_tlsio()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `tlsio_destroy()`. **]**  
**SRS_UARTIO_27_038: [** `platform_get_default_tlsio()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `tlsio_dowork()`. **]**  
**SRS_UARTIO_27_039: [** `platform_get_default_tlsio()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `tlsio_open()`. **]**  
**SRS_UARTIO_27_040: [** `platform_get_default_tlsio()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `tlsio_retrieveoptions()`. **]**  
**SRS_UARTIO_27_041: [** `platform_get_default_tlsio()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `tlsio_send()`. **]**  
**SRS_UARTIO_27_042: [** `platform_get_default_tlsio()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `tlsio_setoption()`. **]**  


## Indirect Interactions



### PORT3_ISR

```c++
#pragma vector=PORT3_VECTOR
__interrupt void PORT3_ISR(void);
```

**SRS_UARTIO_27_043: [** `PORT3_ISR()` shall disable the interrupt that invoked it by calling void GPIO_disableInterrupt(uint8_t selectedPort, uint16_t selectedPins), using `GPIO_PORT_P3` for `selectedPort` and `GPIO_PIN5` as the `selectedPins` **]**  

