uartio
=============



## Overview
`uartio_msp430` implements a UART adapter for the MSP430 board using the TI `driverlib` utility.



## References

[MSP Driver Library official page](http://www.ti.com/tool/mspdriverlib)  
[Universal asynchronous receiver/transmitter - UART (generic information)](https://en.wikipedia.org/wiki/Universal_asynchronous_receiver/transmitter)  
[XIO Interface](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/xio.h)  
[OPTIONHANDLER Interface](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/optionhandler.h)  



## Exposed API


### uartio_get_interface_description
```c
extern const IO_INTERFACE_DESCRIPTION * uartio_get_interface_description (void);
```

#### XIO Interface Contract
**SRS_UARTIO_27_001: [** `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_close()`. **]**  
**SRS_UARTIO_27_002: [** `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_create()`. **]**  
**SRS_UARTIO_27_003: [** `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_destroy()`. **]**  
**SRS_UARTIO_27_004: [** `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_dowork()`. **]**  
**SRS_UARTIO_27_005: [** `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_open()`. **]**  
**SRS_UARTIO_27_006: [** `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_retrieveoptions()`. **]**  
**SRS_UARTIO_27_007: [** `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_send()`. **]**  
**SRS_UARTIO_27_008: [** `uartio_get_interface_description()` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains a pointer to `uartio_setoption()`. **]**  

#### UARTIO Specific Implementation



## XIO API


### uartio_cloneoption
`uartio_cloneoption()` is the implementation provided to the option handler instance created as part of `uartio_retrieveoptions()`.
```c
void * uartio_cloneoption (const char * option_name, const void * option_value)
```

#### OPTIONHANDLER Interface Contract
**SRS_UARTIO_27_009: [** If the `option_name` argument is NULL, `uartio_cloneoption()` shall return `NULL`. **]**  
**SRS_UARTIO_27_010: [** If the `option_value` argument is NULL, `uartio_cloneoption()` shall return `NULL`. **]**  
**SRS_UARTIO_27_011: [** If the `option_name` argument indicates an option that is not handled by `uartio`, then `uartio_cloneoption()` shall return `NULL`. **]**  

#### UARTIO Specific Implementation


### uartio_close
`uartio_close()` is the implementation provided via `uartio_get_interface_description()` for the `concrete_io_close` member.
```c
int uartio_close (CONCRETE_IO_HANDLE io_handle, ON_IO_CLOSE_COMPLETE on_io_close_complete, void * on_io_close_complete_context)
```

#### XIO Interface Contract
**SRS_UARTIO_27_012: [** If the argument `io_handle` is NULL, `uartio_close()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_013: [** `uartio_close()` shall initiate closing the UART IO and on success it shall return 0. **]**  
**SRS_UARTIO_27_014: [** On success, `on_io_close_complete()` shall be called while passing as argument `on_io_close_complete_context`. **]**  
**SRS_UARTIO_27_015: [** If called when `uartio` is closed, `uartio_close()` shall fail and return a non-zero value. **]**  

#### UARTIO Specific Implementation
**SRS_UARTIO_27_101: [** If the argument `io_handle` is NOT equal to stored singleton value, `uartio_close()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_102: [** `uartio_close()` shall await any outstanding bytes by polling `EUSCI_A_UART_queryStatusFlags()` with the `UCBUSY` mask until it returns `false`. **]**  
**SRS_UARTIO_27_103: [** `uartio_close()` shall disable the UART interrupt by calling `EUSCI_A_UART_disableInterrupt()`. **]**  
**SRS_UARTIO_27_104: [** `uartio_close()` shall disable the UART by calling `EUSCI_A_UART_disable()`. **]**  


### uartio_create
`uartio_create()` is the implementation provided via `uartio_get_interface_description()` for the `concrete_io_create` member.
```c
CONCRETE_IO_HANDLE uartio_create (void * io_create_parameters);
```
The `io_create_parameters` will be in the form of `UARTIO_CONFIG *`
```c
typedef struct UARTIO_CONFIG_TAG {
    uint32_t baud_rate;
    size_t ring_buffer_size;
} UARTIO_CONFIG;
```

#### XIO Interface Contract
**SRS_UARTIO_27_017: [** If the argument `io_create_parameters` is `NULL`, `uartio_create()` shall fail and return `NULL`. **]**  
**SRS_UARTIO_27_019: [** When `uartio_create()` fails, all allocated resources up to that point shall be freed. **]**  

#### UARTIO Specific Implementation
**SRS_UARTIO_27_105: [** If the stored singleton value is non-`NULL`, then `uartio_create()` shall fail and return `NULL`. **]**  
**SRS_UARTIO_27_106: [** If the `baud_rate` member of the `UARTIO_CONFIG *` parameter is 0, then `uartio_create()` shall fail and return `NULL`. **]**  
**SRS_UARTIO_27_107: [** If the `ring_buffer_size` member of the `UARTIO_CONFIG *` parameter is 0, then `uartio_create()` shall fail and return `NULL`. **]**  
**SRS_UARTIO_27_132: [** If the `ring_buffer_size` member of the `UARTIO_CONFIG *` parameter is not a power of 2, then `uartio_create()` shall fail and return `NULL`. **]**  
**SRS_UARTIO_27_108: [** `uartio_create()` shall store the `baud_rate` value for later use when enabling the underlying UART. **]**  
**SRS_UARTIO_27_109: [** `uartio_create()` shall store the `ring_buffer_size` value minus one, as an index mask. **]**  
**SRS_UARTIO_27_110: [** If allocating memory for the ring buffer fails, then the `uartio_create()` shall fail and return `NULL`. **]**  
**SRS_UARTIO_27_111: [** If allocating memory for the cache buffer fails, then the `uartio_create()` shall fail and return `NULL`. **]**  
**SRS_UARTIO_27_112: [** Otherwise `uartio_create()` has succeeded, and a pointer to the static instance will be stored in the singleton pointer. **]**  


### uartio_destroy
`uartio_destroy()` is the implementation provided via `uartio_get_interface_description()` for the `concrete_io_destroy` member.
```c
void uartio_destroy (CONCRETE_IO_HANDLE io_handle);
```

#### XIO Interface Contract
**SRS_UARTIO_27_021: [** If `io_handle` is `NULL`, `uartio_destroy()` shall do nothing. **]**  
**SRS_UARTIO_27_022: [** All options cached via `uartio_set_option` shall also be freed. **]**  
**SRS_UARTIO_27_023: [** `uartio_destroy()` shall close the IO if it was open before freeing all the resources. **]**  

#### UARTIO Specific Implementation
**SRS_UARTIO_27_113: [** If the argument `io_handle` is NOT equal to stored singleton value, `uartio_destroy()` shall do nothing. **]**  
**SRS_UARTIO_27_114: [** `uartio_destroy()` shall set the singleton pointer to `NULL`. **]**  
**SRS_UARTIO_27_115: [** `uartio_destroy()` shall free the ring buffer. **]**  
**SRS_UARTIO_27_133: [** `uartio_destroy()` shall free the cache buffer. **]**  


### uartio_destroyoption
`uartio_destroyoption()` is the implementation provided to the option handler instance created as part of `uartio_retrieveoptions()`.
```c
void uartio_destroyoption (const char * option_name, const void * option_value)
```

#### OPTIONHANDLER Interface Contract
**SRS_UARTIO_27_024: [** If the `option_name` argument is `NULL`, `uartio_destroyoption()` shall do nothing. **]**  
**SRS_UARTIO_27_025: [** If the `option_value` argument is `NULL`, `uartio_destroyoption()` shall do nothing. **]**  
**SRS_UARTIO_27_026: [** If the `option_name` argument indicates an option that is not handled by `uartio`, then `uartio_destroyoption()` shall do nothing. **]**  

#### UARTIO Specific Implementation


### uartio_dowork
`uartio_dowork()` is the implementation provided via `uartio_get_interface_description()` for the `concrete_io_dowork` member.
```c
void uartio_dowork (CONCRETE_IO_HANDLE io_handle)
```

#### XIO Interface Contract
**SRS_UARTIO_27_027: [** If the `io_handle` argument is `NULL`, `uartio_dowork()` shall do nothing. **]**  
**SRS_UARTIO_27_028: [** If the IO is closed (and open has not been initiated), then `uartio_dowork` shall do nothing. **]**  

#### UARTIO Specific Implementation
**SRS_UARTIO_27_116: [** If the argument `io_handle` is NOT equal to stored singleton value, `uartio_dowork()` shall do nothing. **]**  
**SRS_UARTIO_27_118: [** If the IO is open, `uartio_dowork()` shall read all newly available bytes placed in the ring buffer by the UART interrupt. **]**  
**SRS_UARTIO_27_121: [** `uartio_dowork()` shall await any outstanding bytes by polling `EUSCI_A_UART_queryStatusFlags()` using the `UCBUSY` mask until it returns `false`. **]**  
**SRS_UARTIO_27_122: [** `uartio_dowork()` shall disable the UART interrupt while caching the ring buffer by calling `EUSCI_A_UART_disableInterrupt()`. **]**  
**SRS_UARTIO_27_117: [** If any bytes are in the ring buffer they should be indicated via the `on_bytes_received()` callback passed to `uartio_open()`. **]**  
**SRS_UARTIO_27_119: [** If a UART error is detected, the error shall be indicated by calling the `on_io_error()` callback passed in `uartio_open()`, while passing the `on_io_error_context` to the callback. **]**  
**SRS_UARTIO_27_120: [** If a buffer overflow is detected, the error shall be indicated by calling the `on_io_error()` callback passed in `uartio_open()`, while passing the `on_io_error_context` to the callback. **]**  
**SRS_UARTIO_27_123: [** `uartio_dowork()` shall enable the UART interrupt after caching the ring buffer by calling `EUSCI_A_UART_enableInterrupt()`. **]**  


### uartio_open
`uartio_open()` is the implementation provided via `uartio_get_interface_description()` for the `concrete_io_open` member.
```c
int uartio_open (CONCRETE_IO_HANDLE io_handle, ON_IO_OPEN_COMPLETE on_io_open_complete, void * on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void * on_bytes_received_context, ON_IO_ERROR on_io_error, void * on_io_error_context)
```

#### XIO Interface Contract
**SRS_UARTIO_27_029: [** If the argument `io_handle` is `NULL` then `uartio_open()` shall return a non-zero value. **]**  
**SRS_UARTIO_27_030: [** If the argument `on_io_open_complete()` is `NULL` then `uartio_open()` shall return a non-zero value. **]**  
**SRS_UARTIO_27_031: [** If the argument `on_bytes_received()` is `NULL` then `uartio_open()` shall return a non-zero value. **]**  
**SRS_UARTIO_27_032: [** If the argument `on_io_error()` is `NULL` then `uartio_open()` shall return a non-zero value. **]**  
**SRS_UARTIO_27_033: [** `uartio_open()` shall open the UART io and on success it shall return 0. **]**  
**SRS_UARTIO_27_034: [** If `uartio_open()` succeeds, the callback `on_io_open_complete()` shall be called, while passing `on_io_open_complete_context` and `IO_OPEN_OK` as arguments. **]**  
**SRS_UARTIO_27_035: [** If `uartio_open()` is called while the IO is open, `uartio_open()` shall return a non-zero value without performing any work to open the IO. **]**  

#### UARTIO Specific Implementation
**SRS_UARTIO_27_124: [** If the argument `io_handle` is NOT equal to stored singleton value, `uartio_open()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_125: [** `uartio_open()` shall configure the GPIO for UART by calling `GPIO_setAsPeripheralModuleFunctionOutputPin()`. **]**  
**SRS_UARTIO_27_126: [** `uartio_open()` shall initialize the UART by calling `EUSCI_A_UART_init()`. **]**  
**SRS_UARTIO_27_127: [** `uartio_open()` shall enable the UART by calling `EUSCI_A_UART_enable()`. **]**  
**SRS_UARTIO_27_128: [** `uartio_open()` shall enable the UART interrupt by calling `EUSCI_A_UART_enableInterrupt()`. **]**  
**SRS_UARTIO_27_129: [** `uartio_open()` shall determine the clock speed of the submodule clock to provide the EUSCI parameters by calling `CS_getSMCLK()`. **]**  


### uartio_retrieveoptions
`uartio_retrieveoptions()` is the implementation provided via `uartio_get_interface_description()` for the `concrete_io_retrieveoptions` member.  
```c
static OPTIONHANDLER_HANDLE uartio_retrieveoptions (CONCRETE_IO_HANDLE io_handle)
```

#### OPTIONHANDLER Interface Contract
**SRS_UARTIO_27_036: [** If parameter `io_handle` is `NULL` then `uartio_retrieveoptions()` shall fail and return `NULL`. **]**  
**SRS_UARTIO_27_037: [** `uartio_retrieveoptions()` shall produce an instance of `OPTIONHANDLER_HANDLE`. **]**  
**SRS_UARTIO_27_038: [** If producing the `OPTIONHANDLER_HANDLE` fails, then `uartio_retrieveoptions()` shall fail and return `NULL`. **]**  

#### UARTIO Specific Implementation


### uartio_send
`uartio_send()` is the implementation provided via `uartio_get_interface_description()` for the `concrete_io_send` member.
```c
int uartio_send (CONCRETE_IO_HANDLE io_handle, const void * buffer, size_t buffer_size, ON_SEND_COMPLETE on_send_complete, void * on_send_complete_context)
```

#### XIO Interface Contract
**SRS_UARTIO_27_039: [** If the argument `io_handle` is `NULL`, `uartio_send()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_040: [** If the argument `buffer` is `NULL`, `uartio_send()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_041: [** If `buffer_size` is 0, `uartio_send()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_042: [** `uartio_send()` shall send `buffer_size` bytes, pointed to by `buffer`, and on success it shall return 0. **]**  
**SRS_UARTIO_27_043: [** `on_send_complete()` shall be allowed to be `NULL`. **]**  
**SRS_UARTIO_27_044: [** On success, if a non-`NULL` value was passed for `on_send_complete`, then `on_send_complete()` shall be called while passing to it the `on_send_complete_context` value. **]**  
**SRS_UARTIO_27_045: [** If `uartio_send()` is called when the IO is not open, `uartio_send()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_046: [** If the IO is in an error state (i.e. an error was reported through the `on_io_error()` callback), `uartio_send()` shall fail and return a non-zero value. **]**  

#### UARTIO Specific Implementation
**SRS_UARTIO_27_130: [** If the argument `io_handle` is NOT equal to stored singleton value, `uartio_send()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_131: [** `uartio_send()` shall send each byte of `buffer` by calling `EUSCI_A_UART_transmitData()` `buffer_size` times. **]**  


### uartio_setoption
`uartio_setoption()` is the implementation provided via `uartio_get_interface_description()` for the `concrete_io_setoption` member.
```c
int uartio_setoption (CONCRETE_IO_HANDLE io_handle, const char * option_name, const void * option_value)
```

#### OPTIONHANDLER Interface Contract
**SRS_UARTIO_27_047: [** If the argument `io_handle` is `NULL` `uartio_setoption()` shall return a non-zero value. **]**  
**SRS_UARTIO_27_048: [** If the argument `option_name` is `NULL` `uartio_setoption()` shall return a non-zero value. **]**  
**SRS_UARTIO_27_049: [** If the `option_name` argument indicates an option that is not handled by `uartio`, then `uartio_setoption()` shall return a non-zero value. **]**  

#### UARTIO Specific Implementation

Options that shall be handled by uartio:  
[none]
