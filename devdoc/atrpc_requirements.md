atrpc
=============

## Overview
`atrpc` implements a (R)emote (P)rocedure (C)all mechanism for (AT)tention commands used in cellular modems. The cellular modem is expected to be accessible via a serial connection adhering to the xio interface.


## References

AT Commands:  
- The Hayes Modem Technical Reference Manual, v1.0b (no longer hosted)  
- [(ITU-T Recommendation V.250) International Telecommunication Union Telecommunication Standarization Sector; Series V: Data Communication Over the Telephone Network: Control procedures - Serial asynchronous automatic dialling and control](https://www.itu.int/rec/T-REC-V.250-200307-I/en)  
- [(3GPP TS 27.007) 3rd Generation Partnership Project; Technical Specification Group Core Network and Terminals; AT command set for User Equipment (UE) (Release 14)](https://portal.3gpp.org/desktopmodules/Specifications/SpecificationDetails.aspx?specificationId=1515)  
- [SIM800 Series - AT Command Manual, v1.09](http://www.simcomm2m.com/UploadFile/TechnicalFile/SIM800%20Series_AT%20Command%20Manual_V1.09.pdf)  

RPC Info:  
- [Wikipedia: Remote Procedure Call](https://en.wikipedia.org/wiki/Remote_procedure_call)  

XIO Interface:  
- [XIO Interface](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/xio.h)  


## Exposed Types

```c
enum ta_result_code {
    OK = 0,
    CONNECT = 1,
    RING = 2,
    NO_CARRIER = 3,
    ERROR = 4,
    ATRPC_TIMEOUT = 5,
    NO_DIALTONE = 6,
    BUSY = 7,
    NO_ANSWER = 8,
    SIMCOM_PROCEEDING = 9,
};

typedef ATRPC_HANDLE void *;
typedef void(*TA_RESPONSE)(void * const context, const ta_result_code result_code, const char * const response);
```


## Exposed API

### atrpc_attention
`atrpc_attention()` will form an AT transaction and send it to the underlying xio layer.
```c
extern int atrpc_attention (ATRPC_HANDLE const handle, const char * const command_string, const size_t command_string_length, const size_t timeout_ms, TA_RESPONSE const ta_response, const void * const ta_response_context);
```
**SRS_UARTIO_27_000: [** If the `handle` argument is `NULL`, then `atrpc_attention()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_001: [** If the `ta_response` argument is `NULL`, then `atrpc_attention()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_002: [** If the `handle` has a status of `ATRPC_CLOSED`, `atrpc_attention()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_003: [** If a command is currently outstanding, then `atrpc_attention()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_004: [** `atrpc_attention()` shall mark the call time, by calling `(int)tickcounter_get_current_ms(TICKCOUNTER_HANDLE handle, tickcounter_ms_t * current_ms)` using the handle returned from `atrpc_create()` as the `handle` parameter. **]**  
**SRS_UARTIO_27_005: [** If the call to `tickcounter_get_current_ms()` returns a non-zero value, then `atrpc_attention()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_006: [** `atrpc_attention()` shall allocate memory to store the command string, by calling `(void *)malloc(size_t size)` using `(command_string_length + 3)` for the `size` parameter. **]**  
**SRS_UARTIO_27_007: [** If the call to `malloc()` returns `NULL`, then `atrpc_attention()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_008: [** `atrpc_attention()` shall call `(int)xio_send(XIO_HANDLE handle, const void * buffer, size_t )` using the xio handle returned from `xio_create()` in `atrpc_create()` for the handle parameter, and `AT<command_string>\r` for the `buffer` parameter . **]**  
**SRS_UARTIO_27_009: [** If the call to `xio_send()` returns a non-zero value, then `atrpc_attention()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_010: [** `atrpc_attention()` shall block until the `on_send_complete` callback passed to `xio_send()` returns. **]**  
**SRS_UARTIO_27_011: [** If no errors are encountered during execution, then `atrpc_attention()` shall return 0. **]**  


### atrpc_close
`atrpc_close()` will close the AT RPC connection.
```c
extern int atrpc_close (ATRPC_HANDLE const handle);
```
**SRS_UARTIO_27_012: [** If the `handle` argument is `NULL`, then `atrpc_close()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_013: [** If the `handle` has a status of `ATRPC_CLOSED`, `atrpc_close()` shall do nothing and return 0. **]**  
**SRS_UARTIO_27_014: [** If the `handle` has an `ATRPC_OPENING` status, `atrpc_close()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `ATRPC_TIMEOUT` as the `result_code` parameter, and `NULL` as the `response` parameter. **]**  
**SRS_UARTIO_27_015: [** `atrpc_close()` shall call `(int)xio_close(XIO_HANDLE handle, ON_IO_CLOSE_COMPLETE on_io_close_complete, void * on_io_close_complete_context)` . **]**  
**SRS_UARTIO_27_016: [** If the call to `xio_close()` returns a non-zero value, then `atrpc_close()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_017: [** `atrpc_close()` shall block until the `on_io_close_complete` callback passed to `xio_close()` returns. **]**  
**SRS_UARTIO_27_018: [** If no errors are encountered during execution, then`atrpc_close()` shall return 0. **]**  


### atrpc_create
`atrpc_create()` will allocate the memory necessary for all state necesary for AT RPC connection.
```c
extern ATRPC_HANDLE atrpc_create (void);
```
The `io_create_parameters` will be in the form of `UARTIO_CONFIG *`
```c
typedef struct UARTIO_CONFIG_TAG {
    uint32_t baud_rate;
    size_t ring_buffer_size;
} UARTIO_CONFIG;
```
**SRS_UARTIO_27_019: [** `atrpc_create()` shall create a tickcounter to support timeout functionality by calling `(TICKCOUNTER_HANDLE)tickcounter_create(void)`. **]**  
**SRS_UARTIO_27_020: [** If the call to `tickcounter_create()` returns `NULL`, then `atrpc_create()` shall fail and return `NULL`. **]**  
**SRS_UARTIO_27_021: [** `atrpc_create()` shall create an xio connection to the SIM808 by calling `(XIO_HANDLE)xio_create(void *io_create_parameters)` using a `UARTIO_CONFIG` initialized with a `baud_rate` of `9600` and a `ring_buffer_size` of `8`. **]**  
**SRS_UARTIO_27_022: [** If the call to `xio_create()` returns `NULL`, then `atrpc_create()` shall fail and return `NULL`. **]**  
**SRS_UARTIO_27_023: [** If no errors are encountered during execution, `atrpc_create()` shall return a handle to an AT RPC instance. **]**  


### atrpc_destroy
`atrpc_destroy()` will free all memory used by the AT RPC connection.
```c
extern void atrpc_destroy (ATRPC_HANDLE const handle);
```
**SRS_UARTIO_27_024: [** If the `handle` argument is `NULL`, then `atrpc_destroy()` shall do nothing. **]**  
**SRS_UARTIO_27_025: [** If the `handle` does not have a status of `ATRPC_CLOSED`, `atrpc_destroy()` shall call `(int)atrpc_close(ATRPC_HANDLE handle)` using the handle returned from `tickcounter_create()` for the `handle` parameter. **]**  
**SRS_UARTIO_27_026: [** `atrpc_destroy()` shall call `(void)tickcounter_destroy(TICKCOUNTER_HANDLE handle)` using the handle returned from the call to `tickcounter_create()` for the `handle` parameter. **]**  
**SRS_UARTIO_27_027: [** `atrpc_destroy()` shall call `(void)xio_destroy(XIO_HANDLE handle)` using the handle returned from the call to `xio_create()` for the `handle` parameter. **]**  


### atrpc_dowork
`atrpc_dowork()` asynchronously processes transactions from the underlying xio layer.
```c
extern void atrpc_dowork (ATRPC_HANDLE const handle);
```
**SRS_UARTIO_27_028: [** If the `handle` argument is `NULL`, then `atrpc_dowork()` shall do nothing. **]**  
**SRS_UARTIO_27_029: [** If the `handle` has a status of `ATRPC_CLOSED`, `atrpc_dowork()` shall do nothing. **]**  
**SRS_UARTIO_27_030: [** `atrpc_dowork()` shall call `(void)xio_dowork(XIO_HANDLE handle)` using the handle returned from the call to `xio_create()` for the `handle` parameter. **]**  
**SRS_UARTIO_27_031: [** `atrpc_dowork()` shall mark the call time, by calling `(int)tickcounter_get_current_ms(TICKCOUNTER_HANDLE handle, tickcounter_ms_t * current_ms)` using the handle returned from `atrpc_create()` as the `handle` parameter. **]**  
**SRS_UARTIO_27_032: [** If the timeout value sent as `timeout_ms` to the originating `attention()` call is non-zero and has expired, `atrpc_dowork()` shall call the terminal adapter response callback passed as `ta_response` to `attention()` using the `ta_response_context` parameter passed to `attention()` as the `context` parameter, `ATRPC_TIMEOUT` as the `result_code` parameter, and `NULL` as the `message` parameter. **]**  
**SRS_UARTIO_27_033: [** If the `ta_response` callback was called, then `atrpc_dowork()` shall free the stored command string. **]**  


### atrpc_open
`atrpc_open()` will open the AT RPC connection.

```c
extern int atrpc_open (ATRPC_HANDLE const handle, TA_RESPONSE const on_open_complete, const void * const on_open_complete_context);
```

**SRS_UARTIO_27_034: [** If the `handle` argument is `NULL`, then `atrpc_open()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_035: [** If the `handle` does not have a status of `ATRPC_CLOSED`, `atrpc_open()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_036: [** If the `on_open_complete` argument is `NULL`, then `atrpc_open()` shall fail and return a non-zero value. **]**  
**SRS_UARTIO_27_037: [** `atrpc_open()` shall call `(int)xio_open(XIO_HANDLE handle, ON_IO_OPEN_COMPLETE on_io_open_complete, void * on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void * on_bytes_received_context, ON_IO_ERROR on_io_error, void * on_io_error_context)` using the handle returned from `xio_create()` as the `handle` parameter, the incoming `handle` parameter as the `on_bytes_received_context` parameter, and the incoming `handle` parameter as the `on_io_open_complete_context` parameter. **]**  


## Callback behavior


### on_bytes_received
`on_bytes_received()` will be passed to the underlying xio during `xio_open()`.
```c
void on_bytes_received (void * context, const unsigned char * buffer, size_t size);
```
**SRS_UARTIO_27_038: [** If the status of the handle (passed in the callback context) is `ATRPC_CLOSED`, then `on_bytes_received()` shall discard all bytes. **]**  
**SRS_UARTIO_27_039: [** If the status of the handle (passed in the callback context) is `ATRPC_OPEN`, then `on_bytes_received()` shall discard any bytes not prefixed with the `command_string` parameter passed to `attention()`. **]**  
**SRS_UARTIO_27_040: [** If the status of the handle (passed in the callback context) is `ATRPC_OPEN`, then `on_bytes_received()` shall capture any bytes prefixed with the `command_string` parameter passed to `attention()` and postfixed with a `<result code>"\r"` and call the `ta_response` callback passed to `attention()` using the `ta_response_context` as the `context` parameter, the parsed result code as the `result_code` parameter, and the captured bytes as the `message` parameter. **]**  
**SRS_UARTIO_27_041: [** If the status of the handle (passed in the callback context) is `ATRPC_OPENING`, then `on_bytes_received()` shall capture any bytes postfixed with a `<result code>"\r"` and call the `ta_response` callback passed to `attention()` using the `ta_response_context` as the `context` parameter, the parsed result code as the `result_code` parameter, and the captured bytes as the `message` parameter. **]**  
**SRS_UARTIO_27_042: [** If the status of the handle (passed in the callback context) is `ATRPC_OPENING`, then `on_bytes_received()` shall capture any bytes postfixed with a `"\r\n"<result word>"\r\n"` and call the `ta_response` callback passed to `attention()` using the `ta_response_context` as the `context` parameter, the parsed result code as the `result_code` parameter, and the captured bytes as the `message` parameter. **]**  


### on_io_close_complete
`on_io_close_complete()` will be passed to the underlying xio during `xio_close()`.
```c
void on_io_close_complete (void * context);
```
**SRS_UARTIO_27_043: [** `on_io_close_complete()` shall do nothing. **]**  


### on_io_error
`on_io_error()` will be passed to the underlying xio during `xio_open()`.
```c
void on_io_error (void * context);
```
**SRS_UARTIO_27_044: [** `on_io_error()` shall call the terminal adapter response callback passed as `ta_response` to `attention()` using the `ta_response_context` parameter passed to `attention()` as the `context` parameter, `ERROR` as the `result_code` parameter, and "Underlying xio error!" as the `message` parameter. **]**  
**SRS_UARTIO_27_045: [** `on_io_error()` shall free the stored command string. **]**  


### on_io_open_complete
`on_io_open_complete()` will be passed to the underlying xio during `xio_open()`.
```c
void on_io_open_complete (void * context, IO_OPEN_RESULT open_result);
```
**SRS_UARTIO_27_046: [** If the `open_result` parameter is not `OPEN_OK`, then `on_io_open_complete()` shall call the `on_open_complete` callback passed to `atrpc_open()` using the `on_open_complete_context` parameter passed to `atrpc_open()` as the `context` parameter, `ERROR` as the `result_code` parameter, and "Underlying xio failed to open!" as the `message` parameter. **]**  
**SRS_UARTIO_27_047: [** If the `open_result` parameter is `OPEN_OK`, then `on_io_open_complete()` shall initiate the handshaking process by calling `(void)atrpc_handshake(void * context, const char * const command_string, const size_t command_string_length const size_t timeout_ms, TA_RESPONSE const ta_response, const void * const ta_response_context)` using the incoming `context` parameter as the `handle` parameter, `NULL` as the `command_string` parameter, `0` as the `command_string_length` parameter, `100` as the `timeout_ms` parameter, `atrpc_handshake` as the `ta_response` parameter, and `ta_response_context` as the `context` parameter. **]**  


### on_send_complete
`on_send_complete()` will be passed to the underlying xio during `xio_send()`.
```c
void on_send_complete (void * context, IO_SEND_RESULT send_result);
```
**SRS_UARTIO_27_048: [** `on_send_complete()` shall do nothing. **]**  


### atrpc_handshake
`atrpc_handshake` is an internal function called to ensure communication between the devices is well-formed, and, consequently, will move the AT RPC from `ATRPC_OPENING` to `ATRPC_OPEN` status.
```c
void atrpc_handshake (void * context, ta_result_code result_code, char * response);
```
**SRS_UARTIO_27_049: [** `atrpc_handshake()` shall negotiate auto-bauding by calling `(int)attention(ATRPC_HANDLE const handle, const char * const command_string, const size_t command_string_length, const size_t timeout_ms, TA_RESPONSE const ta_response, const void * const ta_response_context)`, using the `context` argument for the `handle` parameter, `NULL` as the `command_string` parameter, `0` as the `command_string_length` parameter, `100` as the `timeout_ms` parameter, `atrpc_handshake` as the `ta_response` parameter, and the `context` argument as the `ta_response_context` parameter, and will continue to do so, until it is called with a successful result code. **]**  
**SRS_UARTIO_27_050: [** If the call to `attention()` returns a non-zero value, then `atrpc_handshake()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `3GPP_ERROR` as the `result_code` parameter, and "XIO ERROR: Unable to negotiate auto-bauding!" as the `response` parameter. **]**  
**SRS_UARTIO_27_051: [** `atrpc_handshake()` shall normalize the terminal adapter response syntax by calling `(int)attention(ATRPC_HANDLE const handle, const char * const command_string, const size_t command_string_length, const size_t timeout_ms, TA_RESPONSE const ta_response, const void * const ta_response_context)`, using the `context` argument for the `handle` parameter, `E1V0` as the `command_string` parameter, `4` as the `command_string_length` parameter, `0` as the `timeout_ms` parameter, `atrpc_handshake` as the `ta_response` parameter, and the `context` argument as the `ta_response_context` parameter, and will continue to do so, until it is called with a successful result code. **]**  
**SRS_UARTIO_27_052: [** If the call to `attention()` returns a non-zero value, then `atrpc_handshake()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `3GPP_ERROR` as the `result_code` parameter, and "XIO ERROR: Unable to normalize the terminal adapter response syntax!" as the `response` parameter. **]**  
**SRS_UARTIO_27_053: [** `atrpc_handshake()` shall write the active profile by calling `(int)attention(ATRPC_HANDLE const handle, const char * const command_string, const size_t command_string_length, const size_t timeout_ms, TA_RESPONSE const ta_response, const void * const ta_response_context)`, using the `context` argument for the `handle` parameter, `&W` as the `command_string` parameter, `2` as the `command_string_length` parameter, `0` as the `timeout_ms` parameter, `atrpc_handshake` as the `ta_response` parameter, and the `context` argument as the `ta_response_context` parameter, and will continue to do so, until it is called with a successful result code. **]**  
**SRS_UARTIO_27_054: [** If the call to `attention()` returns a non-zero value, then `atrpc_handshake()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `3GPP_ERROR` as the `result_code` parameter, and "XIO ERROR: Unable to write the active profile!" as the `response` parameter. **]**  
**SRS_UARTIO_27_055: [** If 25 or more failed result codes are received, then `atrpc_handshake()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `3GPP_ERROR` as the `result_code` parameter, and "Exceeded maximum handshake attempts!" as the `response` parameter. **]**  
**SRS_UARTIO_27_056: [** If each call to `attention()` returns 0 before the maximum number of attempts have been reached, then `atrpc_handshake()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `3GPP_OK` as the `result_code` parameter, and "Handshake successful!" as the `response` parameter. **]**  

