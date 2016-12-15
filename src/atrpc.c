// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility\atrpc.h"

#ifdef __cplusplus
  #include "cstddef"
  #include "cstdlib"
#else
  #include "stddef.h"
  #include "stdlib.h"
#endif

#include "azure_c_shared_utility\tickcounter.h"
#include "azure_c_shared_utility\uartio.h"

typedef enum atrpc_status {
    ATRPC_CLOSED,
    ATRPC_OPEN,
    ATRPC_OPENING,
} atrpc_status;

typedef struct ATRPC_INSTANCE_TAG {
    tickcounter_ms_t call_origination_ms;
    char * current_request;
    size_t current_request_length;
    size_t handshake_attempt;
    size_t handshake_index;
    TA_RESPONSE on_open_complete;
    void * on_open_complete_context;
    TA_RESPONSE response;
    void * response_context;
    atrpc_status status;
    TICK_COUNTER_HANDLE tick_counter;
    size_t timeout_ms;
    XIO_HANDLE uartio;
} ATRPC_INSTANCE;


int
atrpc_attention(
    ATRPC_HANDLE handle_,
    const char * command_string_,
    size_t command_string_length_,
    size_t timeout_ms_,
    TA_RESPONSE ta_response_,
    void * ta_response_context_
) {
    (void)handle_, command_string_, command_string_length_, timeout_ms_, ta_response_, ta_response_context_;
    //TODO: Fail if handle parameter equals NULL
    //TODO: Fail if reponse parameter equals NULL
    //TODO: Fail if another command is processing (i.e. at_rpc->response does not equal NULL)
    //TODO: Record call time
    //TODO: Send "AT<command_string>\r" to the underlying UART xio layer
    return __LINE__;
}


void
atrpc_close(
    ATRPC_HANDLE handle_
) {
    (void)handle_;
    //TODO: Close the uartio xio layer
}


ATRPC_HANDLE
atrpc_create(
    void
) {
    //TODO: Create tickcounter
    //TODO: Create uartio xio layer
    return (ATRPC_HANDLE)NULL;
}


void
atrpc_destroy(
    ATRPC_HANDLE handle_
) {
    (void)handle_;
    //TODO: Destroy uartio xio layer
    //TODO: Destroy tickcounter
}


void
atrpc_dowork(
    ATRPC_HANDLE handle_
) {
    (void)handle_;
    //TODO: Call tickcounter_get_current_ms to check for timeout condition
    //TODO: Call ta_response callback with `result_code` of 5 to indicate a timeout occurred
    //TODO: Discard bytes that are not prefixed with current_request
    //TODO: Find the current_request in byte stream to detect beginning of the response
        //TODO: Find the result code <ta_result_code><CR> to detect the end of the response
    //TODO: Call ta_response callback with the parsed value supplied to current_reqeust
    //TODO: Clear the callback and context once the callback has been called
}

static
void
atrpc_handshake(
    void * context,
    TA_RESULT_CODE result_code,
    const char * response
) {
    (void)response;
    ATRPC_INSTANCE * atrpc = (ATRPC_INSTANCE *)context;

    switch (result_code) {
      case OK_3GPP:
        ++atrpc->handshake_index;
        break;
      case CONNECT_3GPP:
      case RING_3GPP:
      case NO_CARRIER_3GPP:
      case ERROR_3GPP:
      case ATRPC_TIMEOUT:
      case NO_DIALTONE_3GPP:
      case BUSY_3GPP:
      case NO_ANSWER_3GPP:
      case PROCEEDING_SIMCOM:
      default:
        ++atrpc->handshake_attempt;
        break;
    }

    if (25 <= atrpc->handshake_attempt) {
        atrpc->on_open_complete(atrpc->on_open_complete_context, ERROR_3GPP, "Exceeded maximum handshake attempts!");
    } else {
        switch (atrpc->handshake_index) {
          case 0:
            if (0 != atrpc_attention(context, NULL, 0, 100, atrpc_handshake, context)) {
                atrpc->on_open_complete(atrpc->on_open_complete_context, ERROR_3GPP, "XIO ERROR: Unable to negotiate auto-bauding!");
            }
            break;
          case 1:
            if (0 != atrpc_attention(context, "E1V0", (sizeof("E1V0") - 1), 0, atrpc_handshake, context)) {
                atrpc->on_open_complete(atrpc->on_open_complete_context, ERROR_3GPP, "XIO ERROR: Unable to normalize the terminal adapter response syntax!");
            }
            break;
          case 2:
            atrpc->status = ATRPC_OPEN;
            if (0 != atrpc_attention(context, "&W", (sizeof("&W") - 1), 0, atrpc_handshake, context)) {
                atrpc->on_open_complete(atrpc->on_open_complete_context, ERROR_3GPP, "XIO ERROR: Unable to write the active profile!");
            }
            break;
          case 3:
              atrpc->on_open_complete(atrpc->on_open_complete_context, OK_3GPP, "Handshake successful!");
            break;
        }
    }
}


int
atrpc_open(
    ATRPC_HANDLE handle_,
    TA_RESPONSE on_open_complete,
    void * on_open_complete_context
) {
    (void)handle_, on_open_complete, on_open_complete_context;
    //TODO: Open the uartio xio layer
    //TODO: Negotiate auto-bauding by sending "AT\r" until the message is acknowledged with an "0\r", "\r\nOK\r\n", "AT\r0\r" or "AT\r\r\nOK\r\n"
    //TODO: Ensure expected response types by send "ATE1V0\r"
    //TODO: Write the settings to the active profile by sending "AT&W\r"
    return __LINE__;
}

