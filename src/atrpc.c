// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/atrpc.h"

#ifdef __cplusplus
  #include <cstdbool>
  #include <cstddef>
  #include <cstdlib>
#else
  #include <stdbool.h>
  #include <stddef.h>
  #include <stdlib.h>
#endif

#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/uartio.h"
#include "azure_c_shared_utility/vector.h"
#include "azure_c_shared_utility/xlogging.h"

typedef enum atrpc_status {
    ATRPC_CLOSED,
    ATRPC_HANDSHAKING,
    ATRPC_NEGOTIATING_AUTOBAUD,
    ATRPC_OPEN,
} atrpc_status;

typedef enum modem_io_status {
    MODEM_IO_CLOSED,
    MODEM_IO_OPEN,
    MODEM_IO_OPENING,
} modem_io_status;

typedef struct ATRPC_INSTANCE_TAG {
    tickcounter_ms_t call_origination_ms;
    char * current_request;
    size_t current_request_length;
    bool current_request_response_outstanding;
    size_t handshake_attempt;
    size_t handshake_index;
    XIO_HANDLE modem_io;
    bool modem_receiving;
    modem_io_status modem_status;
    ON_TA_RESPONSE on_open_complete;
    void * on_open_complete_context;
    ON_TA_RESPONSE on_ta_response;
    void * on_ta_response_context;
    VECTOR_HANDLE response;
    size_t response_machine_index;
    atrpc_status status;
    TICK_COUNTER_HANDLE tick_counter;
    size_t timeout_ms;
} ATRPC_INSTANCE;


static
void
modem_handshake(
    void * context,
    TA_RESULT_CODE result_code,
    const char * response
) {
    (void)response;
    ATRPC_INSTANCE * atrpc = (ATRPC_INSTANCE *)context;

    if (NULL == context) {
        LogError("ERROR: NULL context passed into modem_handshake()!");
    } else {
        switch (result_code) {
          case OK_3GPP:
            ++atrpc->handshake_index;
            break;
          case CONNECT_3GPP:
          case RING_3GPP:
          case NO_CARRIER_3GPP:
          case ERROR_3GPP:
          case ERROR_ATRPC:
          case NO_DIALTONE_3GPP:
          case BUSY_3GPP:
          case NO_ANSWER_3GPP:
          case PROCEEDING_SIMCOM:
          default:
            ++atrpc->handshake_attempt;
            break;
        }

        if (50 <= atrpc->handshake_attempt) {
            /* SRS_UARTIO_27_065: [ During auto-baud negotiation, if 50 or more time-outs occur, then `modem_handshake()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `ERROR_ATRPC` as the `result_code` parameter, and `NULL` as the `response` parameter. ] */
            atrpc->handshake_attempt = 0;
            atrpc->on_open_complete(atrpc->on_open_complete_context, ERROR_ATRPC, NULL);
            LogError("Failed to negotiate handshake before exhausting maximum allowed time-outs!");
        } else {
            switch (atrpc->handshake_index) {
              case 0:
                // Negotiate auto-bauding by sending "AT\r" until the message is acknowledged with an "0\r", "\r\nOK\r\n", "AT\r0\r" or "AT\r\r\nOK\r\n"
                /* SRS_UARTIO_27_056: [ If the ping times-out when negotiating auto-baud, then `on_bytes_received()` shall reissue the ping by calling `(int)xio_send(XIO_HANDLE handle, const void * buffer, size_t size, ON_IO_SEND_COMPLETE on_io_send_complete, void * on_io_send_context)` using the xio handle returned from `xio_create()` for the handle parameter, and `AT\r` for the `buffer` parameter, and `3` for the `size` parameter. ] */
                if (0 != atrpc_attention(context, NULL, 0, 250, modem_handshake, context)) {
                    /* SRS_UARTIO_27_057: [ If `atrpc_attention` returns a non-zero value, then `on_io_open_complete()` shall call the `on_open_complete` callback passed to `atrpc_open()` using the `on_open_complete_context` parameter passed to `atrpc_open()` as the `context` parameter, `ERROR_3GPP` as the `result_code` parameter, and `NULL` as the `response` parameter. ] */
                    atrpc->on_open_complete(atrpc->on_open_complete_context, ERROR_ATRPC, NULL);
                    LogError("XIO ERROR: Unable to negotiate auto-bauding!");
                }
                break;
              case 1:
                // Normalize terminal adapter responses by sending "ATE1V0\r"
                /* SRS_UARTIO_27_061: [ When auto-baud negotiation has completed, then `on_bytes_received()` shall normalize the ta responses by calling `(int)xio_send(XIO_HANDLE handle, const void * buffer, size_t size, ON_IO_SEND_COMPLETE on_io_send_complete, void * on_io_send_context)` using the xio handle returned from `xio_create()` during `atrpc_create()` for the handle parameter, and `ATE1V0\r` for the `buffer` parameter, and `7` for the `size` parameter. ] */
                  if (0 != atrpc_attention(context, "E1V0", (sizeof("E1V0") - 1), 250, modem_handshake, context)) {
                    /* SRS_UARTIO_27_062: [ If the call to `attention()` returns a non-zero value, then `modem_handshake()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `3GPP_ERROR` as the `result_code` parameter, and `NULL` as the `response` parameter. ] */
                    atrpc->on_open_complete(atrpc->on_open_complete_context, ERROR_ATRPC, NULL);
                    LogError("XIO ERROR: Unable to normalize the terminal adapter response syntax!");
                }
                break;
              case 2:
                // Write the settings to the active profile by sending "AT&W\r"
                /* SRS_UARTIO_27_063: [ Once the communication with the modem has been normalized, `on_bytes_received()` shall write the active profile by calling `(int)attention(ATRPC_HANDLE const handle, const char * const command_string, const size_t command_string_length, const size_t timeout_ms, TA_RESPONSE const ta_response, const void * const ta_response_context)`, using the `context` argument for the `handle` parameter, `&W` as the `command_string` parameter, `2` as the `command_string_length` parameter, `0` as the `timeout_ms` parameter, `modem_handshake` as the `ta_response` parameter, and the `context` argument as the `ta_response_context` parameter. ] */
                if (0 != atrpc_attention(context, "&W", (sizeof("&W") - 1), 250, modem_handshake, context)) {
                    /* SRS_UARTIO_27_064: [ If the call to `attention()` returns a non-zero value, then `modem_handshake()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `3GPP_ERROR` as the `result_code` parameter, and "XIO ERROR: Unable to write the active profile!" as the `response` parameter. ] */
                    atrpc->on_open_complete(atrpc->on_open_complete_context, ERROR_ATRPC, NULL);
                    LogError("XIO ERROR: Unable to write the active profile!");
                }
                break;
              case 3:
                atrpc->status = ATRPC_OPEN;
                /* SRS_UARTIO_27_066: [ Once the profile has been successfully stored, then `on_bytes_received()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `3GPP_OK` as the `result_code` parameter, and `NULL` as the `response` parameter. ] */
                atrpc->on_open_complete(atrpc->on_open_complete_context, OK_3GPP, NULL);
                LogInfo("Handshake successful!");
                break;
            }
        }
    }
}


static
void
modem_on_bytes_received(
    void * context_,
    const unsigned char * buffer_,
    size_t size_
) {
    ATRPC_INSTANCE * atrpc = (ATRPC_INSTANCE *)context_;

    if (NULL == context_) {
        LogError("ERROR: NULL context passed into modem_on_bytes_received()!");
    } else {
        if (ATRPC_CLOSED == atrpc->status) {
            /* SRS_UARTIO_27_052: [ If `atrpc_open()` has not been called on the `handle` (passed in the callback context), then `on_bytes_received()` shall discard all bytes. ] */
        } else {
            TA_RESULT_CODE ta_result_code = ERROR_3GPP;
            bool response_complete = false, error = false;
            size_t index = 0;
            for (; index < size_ && !response_complete; ++index) {
                if (ATRPC_NEGOTIATING_AUTOBAUD == atrpc->status) {
                    /* SRS_UARTIO_27_058: [ During auto-baud negotiation, `on_bytes_received()` shall accept "0\r" as a valid response. ] */
                    /* SRS_UARTIO_27_059: [ During auto-baud negotiation, `on_bytes_received()` shall accept "\r\nOK\r\n" as a valid response. ] */
                    switch (atrpc->response_machine_index) {
                      case 0:
                        if ('\r' == buffer_[index]) { ++atrpc->response_machine_index; }
                        break;
                      case 1:
                        if ('0' == buffer_[index]) { ++atrpc->response_machine_index; }
                        else if ('\r' == buffer_[index]) { atrpc->response_machine_index = 4; }
                        else if ('\n' == buffer_[index]) { atrpc->response_machine_index = 3; }
                        else { atrpc->response_machine_index = 0; }
                        break;
                      case 2:
                        if ('\r' == buffer_[index]) {
                            response_complete = true;
                            atrpc->status = ATRPC_HANDSHAKING;
                            ta_result_code = OK_3GPP;
                        }
                        atrpc->response_machine_index = 0;
                        break;
                      case 3:
                        if ('0' == buffer_[index]) { --atrpc->response_machine_index; }
                        else if ('\r' == buffer_[index]) { ++atrpc->response_machine_index; }
                        else { atrpc->response_machine_index = 0; }
                        break;
                      case 4:
                        if ('0' == buffer_[index]) { atrpc->response_machine_index = 2; }
                        else if ('\r' == buffer_[index]) { /* defer */ }
                        else if ('\n' == buffer_[index]) { ++atrpc->response_machine_index; }
                        else { atrpc->response_machine_index = 0; }
                        break;
                      case 5:
                        if ('0' == buffer_[index]) { atrpc->response_machine_index = 2; }
                        else if ('O' == buffer_[index]) { ++atrpc->response_machine_index; }
                        else if ('\r' == buffer_[index]) { --atrpc->response_machine_index; }
                        else { atrpc->response_machine_index = 0; }
                        break;
                      case 6:
                        if ('K' == buffer_[index]) { ++atrpc->response_machine_index; }
                        else { atrpc->response_machine_index = 0; }
                        break;
                      case 7:
                        if ('\r' == buffer_[index]) { ++atrpc->response_machine_index; }
                        else { atrpc->response_machine_index = 0; }
                        break;
                      case 8:
                        if ('0' == buffer_[index]) { atrpc->response_machine_index = 2; }
                        else if ('\r' == buffer_[index]) { atrpc->response_machine_index = 4; }
                        else if ('\n' == buffer_[index]) {
                            response_complete = true;
                            atrpc->status = ATRPC_HANDSHAKING;
                            ta_result_code = OK_3GPP;
                            atrpc->response_machine_index = 0;
                        }
                        else { atrpc->response_machine_index = 0; }
                        break;
                    }
                } else {
                    if (atrpc->current_request_response_outstanding) {
                        if (atrpc->current_request[atrpc->response_machine_index] == buffer_[index]) {
                            ++atrpc->response_machine_index;
                            if (atrpc->current_request_length == atrpc->response_machine_index) {
                                atrpc->current_request_response_outstanding = false;
                                atrpc->response_machine_index = 1;
                            }
                        } else {
                            atrpc->response_machine_index = 0;
                        }
                    } else {
                        /* SRS_UARTIO_27_053: [ If the `on_open_complete()` callback has been called, then `on_bytes_received()` shall capture any bytes following the prefix of the `command_string` parameter passed to `attention()` along with the postfixed `<result code>"\r"`. ] */
                        if (0 != VECTOR_push_back(atrpc->response, &buffer_[index], 1)) {
                            /* SRS_UARTIO_27_055: [ If an error occurs when capturing the bytes, then `on_bytes_received()` shall call the `ta_response` callback passed to `attention()` using the `ta_response_context` as the `context` parameter, the `ERROR_ATRPC` as the `result_code` parameter, and the captured message as the `message` parameter. ] */
                            error = true;
                            response_complete = true;
                        } else {
                            switch (atrpc->response_machine_index) {
                              case 0:
                                if ('\r' == buffer_[index]) { ++atrpc->response_machine_index; }
                                break;
                              case 1:
                                // Accept all valid result codes
                                if ('0' == buffer_[index]
                                 || '1' == buffer_[index]
                                 || '2' == buffer_[index]
                                 || '3' == buffer_[index]
                                 || '4' == buffer_[index]
                                 || '6' == buffer_[index]
                                 || '7' == buffer_[index]
                                 || '8' == buffer_[index]
                                 || '9' == buffer_[index]
                                ) { 
                                    ++atrpc->response_machine_index;
                                }
                                else if ('\r' == buffer_[index]) { /* defer */ }
                                else if ('\n' == buffer_[index]) { atrpc->response_machine_index = 3; }
                                else { atrpc->response_machine_index = 0; }
                                break;
                              case 2:
                                if ('\r' == buffer_[index]) {
                                    response_complete = true;
                                }
                                atrpc->response_machine_index = 0;
                                break;
                              case 3:
                                // Accept all valid result codes
                                if ('0' == buffer_[index]
                                 || '1' == buffer_[index]
                                 || '2' == buffer_[index]
                                 || '3' == buffer_[index]
                                 || '4' == buffer_[index]
                                 || '6' == buffer_[index]
                                 || '7' == buffer_[index]
                                 || '8' == buffer_[index]
                                 || '9' == buffer_[index]
                                ) {
                                    --atrpc->response_machine_index;
                                }
                                else if ('\r' == buffer_[index]) { atrpc->response_machine_index = 1; }
                                else { atrpc->response_machine_index = 0; }
                                break;
                            }
                        }
                    }
                }
                if (response_complete) {
                    char * mutable_buffer = NULL;
                    if (ATRPC_NEGOTIATING_AUTOBAUD != atrpc->status) {
                        mutable_buffer = VECTOR_front(atrpc->response);
                        size_t mutable_buffer_size = VECTOR_size(atrpc->response);
                        if (error) {
                            ta_result_code = ERROR_ATRPC;
                            if (0 == mutable_buffer_size) { mutable_buffer = NULL; }
                            else { mutable_buffer[(mutable_buffer_size - 1)] = '\0'; }
                        } else {
                            ta_result_code = (TA_RESULT_CODE)(mutable_buffer[(mutable_buffer_size - 2)] - 0x30);
                            mutable_buffer[(mutable_buffer_size - 2)] = '\0';
                        }
                    }
                    /* SRS_UARTIO_27_060: [ Once a complete response has been received, then `on_bytes_received()` shall free the stored command string. ] */
                    free(atrpc->current_request);
                    atrpc->current_request = NULL;
                    atrpc->current_request_length = 0;
                    atrpc->current_request_response_outstanding = true;
                    atrpc->timeout_ms = 0;

                    /* SRS_UARTIO_27_054: [ If any bytes where captured, `on_bytes_received()` shall call the `ta_response` callback passed to `attention()` using the `ta_response_context` as the `context` parameter, the captured result code as the `result_code` parameter, and the captured message as the `message` parameter. ] */
                    atrpc->on_ta_response(atrpc->on_ta_response_context, ta_result_code, mutable_buffer);
                    VECTOR_clear(atrpc->response);
                }
            }
        }
    }
    return;
}


static
void
modem_on_io_close_complete(
    void * context_
) {
    ATRPC_INSTANCE * atrpc = (ATRPC_INSTANCE *)context_;

    if (NULL == context_) {
        LogError("ERROR: NULL context passed into modem_on_io_close_complete()!");
    } else {
        atrpc->modem_status = MODEM_IO_CLOSED;
        /* SRS_UARTIO_27_067: [ `on_io_close_complete()` shall call nothing. ] */
    }
    return;
}


static
void
modem_on_io_error(
    void * context_
) {
    ATRPC_INSTANCE * atrpc = (ATRPC_INSTANCE *)context_;

    if (NULL == context_) {
        LogError("ERROR: NULL context passed into modem_on_io_error()!");
    } else {
        /* SRS_UARTIO_27_068: [ `on_io_error()` shall free the stored command string. ] */
        free(atrpc->current_request);
        atrpc->current_request = NULL;
        atrpc->current_request_length = 0;
        atrpc->current_request_response_outstanding = true;
        atrpc->timeout_ms = 0;

        /* SRS_UARTIO_27_069: [ `on_io_error()` shall call the terminal adapter response callback passed as `ta_response` to `attention()` using the `ta_response_context` parameter passed to `attention()` as the `context` parameter, `ERROR_ATRPC` as the `result_code` parameter, and `NULL` as the `message` parameter. ] */
        atrpc->on_ta_response(atrpc->on_ta_response_context, ERROR_ATRPC, NULL);
        LogError("XIO buffer error!");
    }
    return;
}


static
void
modem_on_io_open_complete(
    void * context_,
    IO_OPEN_RESULT open_result_
) {
    ATRPC_INSTANCE * atrpc = (ATRPC_INSTANCE *)context_;

    if (NULL == context_) {
        LogError("ERROR: NULL context passed into modem_on_io_open_complete()!");
    } else {
        if (IO_OPEN_OK != open_result_) {
            atrpc->handshake_attempt = 0;
            atrpc->handshake_index = 0;
            atrpc->modem_status = MODEM_IO_CLOSED;
            atrpc->status = ATRPC_CLOSED;
            /* SRS_UARTIO_27_070: [ If the `open_result` parameter is not `IO_OPEN_OK`, then `on_io_open_complete()` shall call the `on_open_complete` callback passed to `atrpc_open()` using the `on_open_complete_context` parameter passed to `atrpc_open()` as the `context` parameter, `ERROR_ATRPC` as the `result_code` parameter, and `NULL` as the `response` parameter. ] */
            atrpc->on_open_complete(atrpc->on_open_complete_context, ERROR_ATRPC, NULL);
        } else {
            atrpc->modem_status = MODEM_IO_OPEN;
            /* SRS_UARTIO_27_071: [ If the `open_result` parameter is `OPEN_OK`, then `on_io_open_complete()` shall initiate the auto-bauding procedure by calling `(void)atrpc_attention(void * context, const char * const command_string, const size_t command_string_length const size_t timeout_ms, TA_RESPONSE const ta_response, const void * const ta_response_context)` using the incoming `context` parameter as the `handle` parameter, `NULL` as the `command_string` parameter, `0` as the `command_string_length` parameter, `100` as the `timeout_ms` parameter, `modem_handshake` as the `ta_response` parameter, and `ta_response_context` as the `context` parameter. ] */
            modem_handshake(context_, CONNECT_3GPP, NULL);
        }
    }
    return;
}


static
void
modem_on_send_complete(
    void * context_,
    IO_SEND_RESULT send_result_
) {
    ATRPC_INSTANCE * atrpc = (ATRPC_INSTANCE *)context_;

    if (NULL == context_) {
        LogError("ERROR: NULL context passed into modem_on_io_open_complete()!");
    } else {
        atrpc->modem_receiving = false;
        if (IO_SEND_OK != send_result_) {
            /* SRS_UARTIO_27_075: [ If the result of underlying xio `on_io_send_complete()` is not `IO_SEND_OK`, then `on_send_complete()` shall free the command string passed to `attention()`. ] */
            free(atrpc->current_request);
            atrpc->current_request = NULL;
            atrpc->current_request_length = 0;
            atrpc->current_request_response_outstanding = true;
            atrpc->timeout_ms = 0;

            /* SRS_UARTIO_27_074: [ If the result of underlying xio `on_io_send_complete()` is `IO_SEND_CANCELLED`, then `on_send_complete()` shall call the terminal adapter response callback passed as `ta_response` to `attention()` using the `ta_response_context` parameter passed to `attention()` as the `context` parameter, `ERROR_ATRPC` as the `result_code` parameter, and `NULL` as the `message` parameter. ] */
            /* SRS_UARTIO_27_076: [ If the result of underlying xio `on_io_send_complete()` is `IO_SEND_ERROR`, then `on_send_complete()` shall call the terminal adapter response callback passed as `ta_response` to `attention()` using the `ta_response_context` parameter passed to `attention()` as the `context` parameter, `ERROR_ATRPC` as the `result_code` parameter, and `NULL` as the `message` parameter. ] */
            atrpc->on_ta_response(atrpc->on_ta_response_context, ERROR_ATRPC, NULL);
        } else {
            /* SRS_UARTIO_27_073: [ If the result of underlying xio `on_io_send_complete()` is `IO_SEND_OK`, then `on_send_complete()` shall call nothing. ] */
        }
    }
    return;
}


int
atrpc_attention(
    ATRPC_HANDLE handle_,
    const char * command_string_,
    size_t command_string_length_,
    size_t timeout_ms_,
    ON_TA_RESPONSE on_ta_response_,
    void * on_ta_response_context_
) {
    ATRPC_INSTANCE * atrpc = (ATRPC_INSTANCE *)handle_;
    int error;

    if (NULL == handle_) {
        /* SRS_UARTIO_27_000: [ If the `handle` argument is `NULL`, then `atrpc_attention()` shall fail and return a non-zero value. ] */
        error = __LINE__;
    } else if (NULL == on_ta_response_) {
        /* SRS_UARTIO_27_001: [ If the `on_ta_response` argument is `NULL`, then `atrpc_attention()` shall fail and return a non-zero value. ] */
        error = __LINE__;
    } else if (MODEM_IO_OPEN != atrpc->modem_status) {
        /* SRS_UARTIO_27_002: [ If the `on_io_open_complete()` callback from the underlying xio has not been called, then `atrpc_attention()` shall fail and return a non-zero value. ] */
        error = __LINE__;
    } else if (NULL != atrpc->current_request) {
        /* SRS_UARTIO_27_003: [ If a command is currently outstanding, then `atrpc_attention()` shall fail and return a non-zero value. ] */
        error = __LINE__;
    } else {
        atrpc->current_request_length = (command_string_length_ + 3);

        /* SRS_UARTIO_27_004: [ `atrpc_attention()` shall mark the call time, by calling `(int)tickcounter_get_current_ms(TICKCOUNTER_HANDLE handle, tickcounter_ms_t * current_ms)` using the handle returned from `atrpc_create()` as the `handle` parameter. ] */
        if (0 != tickcounter_get_current_ms(atrpc->tick_counter, &atrpc->call_origination_ms)) {
            /* SRS_UARTIO_27_005: [ If the call to `tickcounter_get_current_ms()` returns a non-zero value, then `atrpc_attention()` shall fail and return a non-zero value. ] */
            error = __LINE__;
        /* SRS_UARTIO_27_006: [ `atrpc_attention()` store the command string, by calling `(void *)malloc(size_t size)` using `(command_string_length + 3)` for the `size` parameter. ] */
        } else if (NULL == (atrpc->current_request = malloc(atrpc->current_request_length))) {
            /* SRS_UARTIO_27_007: [ If the call to `malloc()` returns `NULL`, then `atrpc_attention()` shall fail and return a non-zero value. ] */
            error = __LINE__;
        } else {
            /* SRS_UARTIO_27_008: [ `atrpc_attention()` shall call `(int)xio_send(XIO_HANDLE handle, const void * buffer, size_t size, ON_IO_SEND_COMPLETE on_io_send_complete, void * on_io_send_context)` using the xio handle returned from `xio_create()` for the handle parameter, and `AT<command_string>\r` for the `buffer` parameter, and `(command_string_length + 3)` for the `size` parameter. ] */
            atrpc->current_request[0] = 'A';
            atrpc->current_request[1] = 'T';
            (void)strncpy(&atrpc->current_request[2], command_string_, command_string_length_);
            atrpc->current_request[(atrpc->current_request_length - 1)] = '\r';
            atrpc->current_request_response_outstanding = true;
            atrpc->modem_receiving = true;
            atrpc->on_ta_response = on_ta_response_;
            atrpc->on_ta_response_context = on_ta_response_context_;
            atrpc->timeout_ms = timeout_ms_;

            if (0 != xio_send(atrpc->modem_io, atrpc->current_request, atrpc->current_request_length, modem_on_send_complete, atrpc)) {
                /* SRS_UARTIO_27_009: [ If the call to `xio_send()` returns a non-zero value, then `atrpc_attention()` shall fail and return a non-zero value. ] */
                error = __LINE__;
                /* SRS_UARTIO_27_012: [ VALGRIND - If the call to `xio_send()` returns a non-zero value, then the stored command string shall be freed. ] */
                free(atrpc->current_request);
                atrpc->current_request = NULL;
                atrpc->current_request_length = 0;
                atrpc->current_request_response_outstanding = true;
                atrpc->modem_receiving = false;
                atrpc->timeout_ms = 0;
            } else {
                /* SRS_UARTIO_27_010: [ `atrpc_attention()` shall block until the `on_send_complete` callback passed to `xio_send()` returns. ] */
                for (;atrpc->modem_receiving;) {
                    xio_dowork(atrpc->modem_io);
                }

                /* SRS_UARTIO_27_011: [ If no errors are encountered during execution, then `atrpc_attention()` shall return 0. ] */
                error = 0;
            }
        }
    }
    return error;
}


int
atrpc_close(
    ATRPC_HANDLE handle_
) {
    ATRPC_INSTANCE * atrpc = (ATRPC_INSTANCE *)handle_;
    int error;
    
    if (NULL == handle_) {
        /* SRS_UARTIO_27_013: [ If the `handle` argument is `NULL`, then `atrpc_close()` shall fail and return a non-zero value. ] */
        error = __LINE__;
    } else if (ATRPC_CLOSED == atrpc->status) {
        /* SRS_UARTIO_27_014: [ If `atrpc_open()` has not been called on the `handle`, `atrpc_close()` shall do nothing and return 0. ] */
        error = 0;
    /* SRS_UARTIO_27_016: [ `atrpc_close()` shall call `(int)xio_close(XIO_HANDLE handle, ON_IO_CLOSE_COMPLETE on_io_close_complete, void * on_io_close_complete_context)`. ] */
    } else if (0 != xio_close(atrpc->modem_io, modem_on_io_close_complete, handle_)) {
        /* SRS_UARTIO_27_017: [ If the call to `xio_close()` returns a non-zero value, then `atrpc_close()` shall fail and return a non-zero value. ] */
        error = __LINE__;
    } else {
        /* SRS_UARTIO_27_018: [ `atrpc_close()` shall block until the `on_io_close_complete` callback passed to `xio_close()` completes. ] */
        for (;MODEM_IO_CLOSED != atrpc->modem_status;) {
            xio_dowork(atrpc->modem_io);
        }

        if (ATRPC_NEGOTIATING_AUTOBAUD == atrpc->status || ATRPC_HANDSHAKING == atrpc->status) {
            /* SRS_UARTIO_27_015: [ If `atrpc_open()` has been called on the `handle` and the `on_open_complete` callback has not been called, `atrpc_close()` shall call the `(void)on_open_complete(void * context, ta_result_code result_code, char * response)` callback provided to `atrpc_open()`, using the `on_open_complete_context` argument provided to `atrpc_open()` as the `context` parameter, `ERROR_ATRPC` as the `result_code` parameter, and `NULL` as the `response` parameter. ] */
            atrpc->on_open_complete(atrpc->on_open_complete_context, ERROR_ATRPC, NULL);
        }

        atrpc->status = ATRPC_CLOSED;
        /* SRS_UARTIO_27_019: [ If no errors are encountered during execution, then `atrpc_close()` shall return 0. ] */
        error = 0;
    }

    return error;
}


ATRPC_HANDLE
atrpc_create(
    void
) {
    ATRPC_INSTANCE * atrpc;
    const IO_INTERFACE_DESCRIPTION * xio_ifc;
    UARTIO_CONFIG xio_config = {9600, 8};

    /* SRS_UARTIO_27_020: [ `atrpc_create()` shall call `malloc()` to allocate the memory required for the internal data structure. ] */
    if (NULL == (atrpc = (ATRPC_INSTANCE *)malloc(sizeof(ATRPC_INSTANCE)))) {
        /* SRS_UARTIO_27_021: [ If `malloc()` fails to allocate the memory required for the internal data structure, then `atrpc_create()` shall fail and return a `NULL` handle. ] */
    /* SRS_UARTIO_27_022: [ `atrpc_create()` shall create a tickcounter to support timeout functionality by calling `(TICKCOUNTER_HANDLE)tickcounter_create(void)`. ] */
    } else if (NULL == (atrpc->tick_counter = tickcounter_create())) {
        /* SRS_UARTIO_27_023: [ If the call to `tickcounter_create()` returns `NULL`, then `atrpc_create()` shall fail and return `NULL`. ] */
        free(atrpc);
        atrpc = NULL;
    /* SRS_UARTIO_27_024: [ `atrpc_create()` shall acquire an xio interface to a modem chipset by calling `(IO_INTERFACE_DESCRIPTION *)uartio_get_interface_description()`. ] */
    } else if (NULL == (xio_ifc = (IO_INTERFACE_DESCRIPTION *)uartio_get_interface_description())) {
        /* SRS_UARTIO_27_025: [ If the call to `uartio_get_interface_description()` returns `NULL`, then `atrpc_create()` shall fail and return `NULL`. ] */
        tickcounter_destroy(atrpc->tick_counter);
        free(atrpc);
        atrpc = NULL;
    /* SRS_UARTIO_27_026: [ `atrpc_create()` shall create an xio connection to a modem chipset by calling `(XIO_HANDLE)xio_create(const IO_INTERFACE_DESCRIPTION * io_interface_description, const void * io_create_parameters)` using the interface description returned from `uartio_get_interface_description()` for `io_interface_description`. ] */
    } else if (NULL == (atrpc->modem_io = xio_create(xio_ifc, &xio_config))) {
        /* SRS_UARTIO_27_027: [ If the call to `xio_create()` returns `NULL`, then `atrpc_create()` shall fail and return `NULL`. ] */
        tickcounter_destroy(atrpc->tick_counter);
        free(atrpc);
        atrpc = NULL;
    /* SRS_UARTIO_27_028: [ `atrpc_create()` shall create a vector to buffer responses from the terminal adapter. ] */
    } else if (NULL == (atrpc->response = VECTOR_create(sizeof(unsigned char)))) {
        /* SRS_UARTIO_27_029: [ If the call to `VECTOR_create()` returns `NULL`, then `atrpc_create()` shall fail and return `NULL`. ] */
        xio_destroy(atrpc->modem_io);
        tickcounter_destroy(atrpc->tick_counter);
        free(atrpc);
        atrpc = NULL;
    } else {
        // Additional configuration
        atrpc->current_request = NULL;
        atrpc->modem_status = MODEM_IO_CLOSED;
        atrpc->status = ATRPC_CLOSED;
    }

    /* SRS_UARTIO_27_030: [ VALGRIND - When `atrpc_create()` returns a non-zero value, all allocated resources up to that point shall be freed. ] */
    /* SRS_UARTIO_27_031: [ If no errors are encountered during execution, `atrpc_create()` shall return a handle to an AT RPC instance. ] */
    return (ATRPC_HANDLE)atrpc;
}


void
atrpc_destroy(
    ATRPC_HANDLE handle_
) {
    ATRPC_INSTANCE * atrpc = (ATRPC_INSTANCE *)handle_;

    if (NULL == handle_) {
        /* SRS_UARTIO_27_032: [ If the `handle` argument is `NULL`, then `atrpc_destroy()` shall do nothing. ] */
        LogError("NULL handle passed to atrpc_destroy!");
    } else {
        /* SRS_UARTIO_27_033: [ If `atrpc_open()` has previously been called and `atrpc_close()` has not been called on the `handle`, `atrpc_destroy()` shall call `(int)atrpc_close(ATRPC_HANDLE handle)` using the handle `argument` passed to `atrpc_destroy()` as the `handle` parameter. ] */
        if (ATRPC_CLOSED != atrpc->status) {
            LogInfo("ATRPC handle was not closed before atrpc_destroy was called.");
            if (0 != atrpc_close(atrpc)) {
                LogError("atrpc_destroy unable to properly close atrpc!");
            }
        }

        /* SRS_UARTIO_27_034: [ `atrpc_destroy()` shall call `(void)xio_destroy(XIO_HANDLE handle)` using the handle returned from the call to `xio_create()` for the `handle` parameter. ] */
        xio_destroy(atrpc->modem_io);

        /* SRS_UARTIO_27_035: [ `atrpc_destroy()` shall call `(void)VECTOR_destroy(VECTOR_HANDLE handle)` using the handle returned from the call to `VECTOR_create()` for the `handle` parameter. ] */
        VECTOR_destroy(atrpc->response);

        /* SRS_UARTIO_27_036: [ `atrpc_destroy()` shall call `(void)tickcounter_destroy(TICKCOUNTER_HANDLE handle)` using the handle returned from the call to `tickcounter_create()` for the `handle` parameter. ] */
        tickcounter_destroy(atrpc->tick_counter);

        /* SRS_UARTIO_27_037: [ `atrpc_destroy()` shall free the memory required for current request. ] */
        free(atrpc->current_request);

        /* SRS_UARTIO_27_038: [ `atrpc_destroy()` shall free the memory required for the internal data structure. ] */
        free(atrpc);
    }
}


void
atrpc_dowork(
    ATRPC_HANDLE handle_
) {
    ATRPC_INSTANCE * atrpc = (ATRPC_INSTANCE *)handle_;
    tickcounter_ms_t ms = 0;

    if (NULL == handle_) {
        /* SRS_UARTIO_27_039: [ If the `handle` argument is `NULL`, then `atrpc_dowork()` shall do nothing. ] */
    } else {
        /* SRS_UARTIO_27_040: [ `atrpc_dowork()` shall call `(void)xio_dowork(XIO_HANDLE handle)` using the handle returned from the call to `xio_create()` for the `handle` parameter. ] */
        xio_dowork(atrpc->modem_io);

        if (ATRPC_CLOSED != atrpc->status) {
            /* SRS_UARTIO_27_041: [ If `atrpc_open()` has been called on the `handle`, `atrpc_dowork()` shall mark the call time, by calling `(int)tickcounter_get_current_ms(TICKCOUNTER_HANDLE handle, tickcounter_ms_t * current_ms)` using the handle returned from `atrpc_create()` as the `handle` parameter. ] */
            if (0 != tickcounter_get_current_ms(atrpc->tick_counter, &ms)) {
                /* SRS_UARTIO_27_042: [ If `tickcounter_get_current_ms()` returns a non-zero value, then `dowork()` shall not attempt to calculate a timeout. ] */
            } else if (0 != atrpc->timeout_ms && (ms - atrpc->call_origination_ms) >= atrpc->timeout_ms) {
                /* SRS_UARTIO_27_044: [ If `atrpc_open()` has been called on the `handle`, and the timeout value sent as `timeout_ms` to the originating `attention()` call is non-zero and has expired, then `atrpc_dowork()` shall free the stored command string. ] */
                free(atrpc->current_request);
                atrpc->current_request = NULL;
                atrpc->current_request_length = 0;
                atrpc->current_request_response_outstanding = true;
                atrpc->timeout_ms = 0;

                /* SRS_UARTIO_27_045: [ If `atrpc_open()` has been called on the `handle`, and the timeout value sent as `timeout_ms` to the originating `attention()` call is non-zero and has expired, then `atrpc_dowork()` shall call the terminal adapter response callback passed as `on_ta_response` to `attention()` using the `ta_response_context` parameter passed to `attention()` as the `context` parameter, `ERROR_ATRPC` as the `result_code` parameter, and `NULL` as the `message` parameter. ] */
                atrpc->on_ta_response(atrpc->on_ta_response_context, ERROR_ATRPC, NULL);
            } else {
                /* SRS_UARTIO_27_043: [ If `attention()` was called with a timeout of 0, then `dowork()` shall not attempt to calculate a timeout. ] */
            }
        }
    }
}


int
atrpc_open(
    ATRPC_HANDLE handle_,
    ON_TA_RESPONSE on_open_complete,
    void * on_open_complete_context
) {
    ATRPC_INSTANCE * atrpc = (ATRPC_INSTANCE *)handle_;
    int error;
    if (NULL == handle_) {
        /* SRS_UARTIO_27_046: [ If the `handle` argument is `NULL`, then `atrpc_open()` shall fail and return a non-zero value. ] */
        error = __LINE__;
    } else if (NULL == on_open_complete) {
        /* SRS_UARTIO_27_047: [ If the `on_open_complete` argument is `NULL`, then `atrpc_open()` shall fail and return a non-zero value. ] */
        error = __LINE__;
    } else if (ATRPC_CLOSED != atrpc->status) {
        /* SRS_UARTIO_27_048: [ If `atrpc_open()` has been called previously and `atrpc_close()` has not been called on the `handle`, `atrpc_open()` shall fail and return a non-zero value. ] */
        error = __LINE__;
    /* SRS_UARTIO_27_049: [ `atrpc_open()` shall call `(int)xio_open(XIO_HANDLE handle, ON_IO_OPEN_COMPLETE on_io_open_complete, void * on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void * on_bytes_received_context, ON_IO_ERROR on_io_error, void * on_io_error_context)` using the handle returned from `xio_create()` as the `handle` parameter, the incoming `handle` parameter as the `on_bytes_received_context` parameter, and the incoming `handle` parameter as the `on_io_open_complete_context` parameter. ] */
    } else {
        // Must happen before call to `open()`, as these variables are affected by `open()` which can return immediately
        atrpc->handshake_attempt = 0;
        atrpc->handshake_index = 0;
        atrpc->modem_status = MODEM_IO_OPENING;
        atrpc->on_open_complete = on_open_complete;
        atrpc->on_open_complete_context = on_open_complete_context;
        atrpc->response_machine_index = 0;
        atrpc->status = ATRPC_NEGOTIATING_AUTOBAUD;

        if (0 != xio_open(atrpc->modem_io, modem_on_io_open_complete, handle_, modem_on_bytes_received, handle_, modem_on_io_error, handle_)) {
            /* SRS_UARTIO_27_050: [ If `xio_open()` returns a non-zero value, then `atrpc_open()` shall do nothing and return a non-zero value. ] */
            error = __LINE__;
        } else {
            /* SRS_UARTIO_27_051: [ If no errors are encountered, `atrpc_open()` shall return 0. ] */
            error = 0;
        }
    }

    return error;
}

