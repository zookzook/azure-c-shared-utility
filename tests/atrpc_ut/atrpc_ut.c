// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
  #include <cstdbool>
#else
  #include <stdbool.h>
#endif

#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

// Test framework `#includes`
#include "testrunnerswitcher.h"
#include "umock_c.h"
#include "umocktypes_charptr.h"
#include "umocktypes_stdint.h"
#include "umock_c_negative_tests.h"

#define disableNegativeTest(x, y) ((x) |= ((uint64_t)1 << (y)))
#define enableNegativeTest(x, y) ((x) &= ~((uint64_t)1 << (y)))
#define skipNegativeTest(x, y) ((x) & ((uint64_t)1 << (y)))

// External library dependencies

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

#define ENABLE_MOCKS
  // `#include` SDK dependencies here
  #include "azure_c_shared_utility/tickcounter_msp430.h"
  #include "azure_c_shared_utility/uartio.h"
#undef ENABLE_MOCKS

#define MOCK_TICKCOUNTER (TICK_COUNTER_HANDLE)0x19790917
#define MOCK_UARTIO (XIO_HANDLE)0x17091979

// Under test `#includes`
#include "azure_c_shared_utility/atrpc.h"

#define ENABLE_MOCKS
  // define free mocked function(s) (platform, external libraries, etc.)
#undef ENABLE_MOCKS

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(atrpc_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    result = umock_c_init(on_umock_c_error);
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_UMOCK_ALIAS_TYPE(bool, char);
    REGISTER_UMOCK_ALIAS_TYPE(TICK_COUNTER_HANDLE, void *);
    REGISTER_UMOCK_ALIAS_TYPE(XIO_HANDLE, void *);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

TEST_FUNCTION(create_SCENARIO_success)
{
    // Arrange

    // Expected call listing

    // Act

    // Assert
    ASSERT_IS_TRUE(false);

    // Cleanup
}

/* SRS_UARTIO_27_000: [ If the `handle` argument is `NULL`, then `atrpc_attention()` shall fail and return a non - zero value. ] */  
/* SRS_UARTIO_27_001: [ If the `ta_response` argument is `NULL`, then `atrpc_attention()` shall fail and return a non - zero value. ] */  
/* SRS_UARTIO_27_002: [ If `atrpc_open()` has not been called on the `handle`, `atrpc_attention()` shall fail and return a non - zero value. ] */  
/* SRS_UARTIO_27_003: [ If a command is currently outstanding, then `atrpc_attention()` shall fail and return a non - zero value. ] */  
/* SRS_UARTIO_27_004: [ `atrpc_attention()` shall mark the call time, by calling `(int)tickcounter_get_current_ms(TICKCOUNTER_HANDLE handle, tickcounter_ms_t * current_ms)` using the handle returned from `atrpc_create()` as the `handle` parameter. ] */  
/* SRS_UARTIO_27_005: [ If the call to `tickcounter_get_current_ms()` returns a non - zero value, then `atrpc_attention()` shall fail and return a non - zero value. ] */  
/* SRS_UARTIO_27_006: [ `atrpc_attention()` store the command string, by calling `(void *)malloc(size_t size)` using `(command_string_length + 3)` for the `size` parameter. ] */  
/* SRS_UARTIO_27_007: [ If the call to `malloc()` returns `NULL`, then `atrpc_attention()` shall fail and return a non - zero value. ] */  
/* SRS_UARTIO_27_008: [ `atrpc_attention()` shall call `(int)xio_send(XIO_HANDLE handle, const void * buffer, size_t)` using the xio handle returned from `xio_create()` in `atrpc_create()` for the handle parameter, and `AT<command_string>\r` for the `buffer` parameter . ] */  
/* SRS_UARTIO_27_009: [ If the call to `xio_send()` returns a non - zero value, then `atrpc_attention()` shall fail and return a non - zero value. ] */  
/* SRS_UARTIO_27_010: [ `atrpc_attention()` shall block until the `on_send_complete` callback passed to `xio_send()` returns. ] */  
/* SRS_UARTIO_27_011: [ If no errors are encountered during execution, then `atrpc_attention()` shall return 0. ] */  

/* SRS_UARTIO_27_012: [ If the `handle` argument is `NULL`, then `atrpc_close()` shall fail and return a non - zero value. ] */  
/* SRS_UARTIO_27_013: [ If the `handle` has a status of `ATRPC_CLOSED`, `atrpc_close()` shall do nothing and return 0. ] */  
/* SRS_UARTIO_27_014: [ If `atrpc_open()` has been called on the `handle` and the `on_open_complete` callback has not been called, `atrpc_close()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `ATRPC_TIMEOUT` as the `result_code` parameter, and `NULL` as the `response` parameter. ] */  
/* SRS_UARTIO_27_015: [ `atrpc_close()` shall call `(int)xio_close(XIO_HANDLE handle, ON_IO_CLOSE_COMPLETE on_io_close_complete, void * on_io_close_complete_context)` . ] */  
/* SRS_UARTIO_27_016: [ If the call to `xio_close()` returns a non - zero value, then `atrpc_close()` shall fail and return a non - zero value. ] */  
/* SRS_UARTIO_27_017: [ `atrpc_close()` shall block until the `on_io_close_complete` callback passed to `xio_close()` returns. ] */  
/* SRS_UARTIO_27_018: [ If no errors are encountered during execution, then`atrpc_close()` shall return 0. ] */  

/* SRS_UARTIO_27_019: [ `atrpc_create()` shall create a tickcounter to support timeout functionality by calling `(TICKCOUNTER_HANDLE)tickcounter_create(void)`. ] */  
/* SRS_UARTIO_27_020: [ If the call to `tickcounter_create()` returns `NULL`, then `atrpc_create()` shall fail and return `NULL`. ] */  
/* SRS_UARTIO_27_021: [ `atrpc_create()` shall create an xio connection to a modem chipset by calling `(XIO_HANDLE)xio_create(void *io_create_parameters)` using a `UARTIO_CONFIG` initialized with a `baud_rate` of `9600` and a `ring_buffer_size` of `8`. ] */  
/* SRS_UARTIO_27_022: [ If the call to `xio_create()` returns `NULL`, then `atrpc_create()` shall fail and return `NULL`. ] */  
/* SRS_UARTIO_27_023: [ If no errors are encountered during execution, `atrpc_create()` shall return a handle to an AT RPC instance. ] */  

/* SRS_UARTIO_27_024: [ If the `handle` argument is `NULL`, then `atrpc_destroy()` shall do nothing. ] */  
/* SRS_UARTIO_27_025: [ If `atrpc_close()` has not been called on the `handle`, `atrpc_destroy()` shall call `(int)atrpc_close(ATRPC_HANDLE handle)` using the handle `argument` passed to `atrpc_destroy()` as the `handle` parameter. ] */  
/* SRS_UARTIO_27_026: [ `atrpc_destroy()` shall call `(void)tickcounter_destroy(TICKCOUNTER_HANDLE handle)` using the handle returned from the call to `tickcounter_create()` for the `handle` parameter. ] */  
/* SRS_UARTIO_27_027: [ `atrpc_destroy()` shall call `(void)xio_destroy(XIO_HANDLE handle)` using the handle returned from the call to `xio_create()` for the `handle` parameter. ] */  

/* SRS_UARTIO_27_028: [ If the `handle` argument is `NULL`, then `atrpc_dowork()` shall do nothing. ] */  
/* SRS_UARTIO_27_029: [ `atrpc_dowork()` shall call `(void)xio_dowork(XIO_HANDLE handle)` using the handle returned from the call to `xio_create()` for the `handle` parameter. ] */  
/* SRS_UARTIO_27_030: [ If `atrpc_open()` has been called on the `handle`, `atrpc_dowork()` shall mark the call time, by calling `(int)tickcounter_get_current_ms(TICKCOUNTER_HANDLE handle, tickcounter_ms_t * current_ms)` using the handle returned from `atrpc_create()` as the `handle` parameter. ] */  
/* SRS_UARTIO_27_031: [  If `atrpc_open()` has been called on the `handle`, and the timeout value sent as `timeout_ms` to the originating `attention()` call is non - zero and has expired, `atrpc_dowork()` shall call the terminal adapter response callback passed as `ta_response` to `attention()` using the `ta_response_context` parameter passed to `attention()` as the `context` parameter, `ATRPC_TIMEOUT` as the `result_code` parameter, and `NULL` as the `message` parameter. ] */  

/* SRS_UARTIO_27_032: [ If the `handle` argument is `NULL`, then `atrpc_open()` shall fail and return a non - zero value. ] */  
/* SRS_UARTIO_27_033: [ If `atrpc_close()` has not been called on the `handle`, `atrpc_open()` shall fail and return a non - zero value. ] */  
/* SRS_UARTIO_27_034: [ If the `on_open_complete` argument is `NULL`, then `atrpc_open()` shall fail and return a non - zero value. ] */  
/* SRS_UARTIO_27_035: [ `atrpc_open()` shall call `(int)xio_open(XIO_HANDLE handle, ON_IO_OPEN_COMPLETE on_io_open_complete, void * on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void * on_bytes_received_context, ON_IO_ERROR on_io_error, void * on_io_error_context)` using the handle returned from `xio_create()` as the `handle` parameter, the incoming `handle` parameter as the `on_bytes_received_context` parameter, and the incoming `handle` parameter as the `on_io_open_complete_context` parameter. ] */  

/* SRS_UARTIO_27_036: [ If `atrpc_open()` has not been called on the `handle` (passed in the callback context), then `on_bytes_received()` shall discard all bytes. ] */  
/* SRS_UARTIO_27_037: [ If the `on_open_complete()` callback(passed to the `atrpc_open()` of the `handle` (passed in the callback context)) has been called, then `on_bytes_received()` shall discard any bytes not prefixed with the `command_string` parameter passed to `attention()`. ] */  
/* SRS_UARTIO_27_038: [ If the `on_open_complete()` callback(passed to the `atrpc_open()` of the `handle` (passed in the callback context)) has been called, then `on_bytes_received()` shall capture any bytes prefixed with the `command_string` parameter passed to `attention()` and postfixed with a `<result code>"\r"` and call the `ta_response` callback passed to `attention()` using the `ta_response_context` as the `context` parameter, the parsed result code as the `result_code` parameter, and the captured bytes as the `message` parameter. ] */  
/* SRS_UARTIO_27_039: [ If the status of the handle(passed in the callback context) is `ATRPC_OPENING`, then `on_bytes_received()` shall capture any bytes postfixed with a `<result code>"\r"` and call the `ta_response` callback passed to `attention()` using the `ta_response_context` as the `context` parameter, the parsed result code as the `result_code` parameter, and the captured bytes as the `message` parameter. ] */  
/* SRS_UARTIO_27_040: [ If the status of the handle(passed in the callback context) is `ATRPC_OPENING`, then `on_bytes_received()` shall capture any bytes postfixed with a `"\r\n"<result word>"\r\n"` and call the `ta_response` callback passed to `attention()` using the `ta_response_context` as the `context` parameter, the parsed result code as the `result_code` parameter, and the captured bytes as the `message` parameter. ] */  
/* SRS_UARTIO_27_041: [ If the `ta_response` callback was called, then `on_bytes_received()` shall free the stored command string. ] */  

/* SRS_UARTIO_27_042: [ `on_io_close_complete()` shall do nothing. ] */  

/* SRS_UARTIO_27_043: [ `on_io_error()` shall call the terminal adapter response callback passed as `ta_response` to `attention()` using the `ta_response_context` parameter passed to `attention()` as the `context` parameter, `ERROR` as the `result_code` parameter, and "Underlying xio error!" as the `message` parameter. ] */  
/* SRS_UARTIO_27_044: [ `on_io_error()` shall free the stored command string. ] */   

/* SRS_UARTIO_27_045: [ If the `open_result` parameter is not `OPEN_OK`, then `on_io_open_complete()` shall call the `on_open_complete` callback passed to `atrpc_open()` using the `on_open_complete_context` parameter passed to `atrpc_open()` as the `context` parameter, `ERROR` as the `result_code` parameter, and "Underlying xio failed to open!" as the `message` parameter. ] */  
/* SRS_UARTIO_27_046: [ If the `open_result` parameter is `OPEN_OK`, then `on_io_open_complete()` shall initiate the handshaking process by calling `(void)atrpc_handshake(void * context, const char * const command_string, const size_t command_string_length const size_t timeout_ms, TA_RESPONSE const ta_response, const void * const ta_response_context)` using the incoming `context` parameter as the `handle` parameter, `NULL` as the `command_string` parameter, `0` as the `command_string_length` parameter, `100` as the `timeout_ms` parameter, `atrpc_handshake` as the `ta_response` parameter, and `ta_response_context` as the `context` parameter. ] */  

/* SRS_UARTIO_27_047: [ `on_send_complete()` shall do nothing. ] */  

/* SRS_UARTIO_27_048: [ `atrpc_handshake()` shall negotiate auto-bauding by calling `(int)attention(ATRPC_HANDLE const handle, const char * const command_string, const size_t command_string_length, const size_t timeout_ms, TA_RESPONSE const ta_response, const void * const ta_response_context)`, using the `context` argument for the `handle` parameter, `NULL` as the `command_string` parameter, `0` as the `command_string_length` parameter, `100` as the `timeout_ms` parameter, `atrpc_handshake` as the `ta_response` parameter, and the `context` argument as the `ta_response_context` parameter, and will continue to do so, until it is called with a successful result code. ] */  
/* SRS_UARTIO_27_049: [ If the call to `attention()` returns a non-zero value, then `atrpc_handshake()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `3GPP_ERROR` as the `result_code` parameter, and "XIO ERROR: Unable to negotiate auto-bauding!" as the `response` parameter. ] */  
/* SRS_UARTIO_27_050: [ `atrpc_handshake()` shall normalize the terminal adapter response syntax by calling `(int)attention(ATRPC_HANDLE const handle, const char * const command_string, const size_t command_string_length, const size_t timeout_ms, TA_RESPONSE const ta_response, const void * const ta_response_context)`, using the `context` argument for the `handle` parameter, `E1V0` as the `command_string` parameter, `4` as the `command_string_length` parameter, `0` as the `timeout_ms` parameter, `atrpc_handshake` as the `ta_response` parameter, and the `context` argument as the `ta_response_context` parameter, and will continue to do so, until it is called with a successful result code. ] */  
/* SRS_UARTIO_27_051: [ If the call to `attention()` returns a non-zero value, then `atrpc_handshake()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `3GPP_ERROR` as the `result_code` parameter, and "XIO ERROR: Unable to normalize the terminal adapter response syntax!" as the `response` parameter. ] */  
/* SRS_UARTIO_27_052: [ `atrpc_handshake()` shall write the active profile by calling `(int)attention(ATRPC_HANDLE const handle, const char * const command_string, const size_t command_string_length, const size_t timeout_ms, TA_RESPONSE const ta_response, const void * const ta_response_context)`, using the `context` argument for the `handle` parameter, `&W` as the `command_string` parameter, `2` as the `command_string_length` parameter, `0` as the `timeout_ms` parameter, `atrpc_handshake` as the `ta_response` parameter, and the `context` argument as the `ta_response_context` parameter, and will continue to do so, until it is called with a successful result code. ] */  
/* SRS_UARTIO_27_053: [ If the call to `attention()` returns a non-zero value, then `atrpc_handshake()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `3GPP_ERROR` as the `result_code` parameter, and "XIO ERROR: Unable to write the active profile!" as the `response` parameter. ] */  
/* SRS_UARTIO_27_054: [ If 25 or more failed result codes are received, then `atrpc_handshake()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `3GPP_ERROR` as the `result_code` parameter, and "Exceeded maximum handshake attempts!" as the `response` parameter. ] */  
/* SRS_UARTIO_27_055: [ If each call to `attention()` returns 0 before the maximum number of attempts have been reached, then `atrpc_handshake()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `3GPP_OK` as the `result_code` parameter, and "Handshake successful!" as the `response` parameter. ] */  

END_TEST_SUITE(atrpc_unittests)

