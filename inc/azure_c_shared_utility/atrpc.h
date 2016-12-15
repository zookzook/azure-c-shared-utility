// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AT_RPC_H
#define AT_RPC_H

#include "azure_c_shared_utility/umock_c_prod.h"

enum ta_result_code {
    3GPP_OK = 0,
    3GPP_CONNECT = 1,
    3GPP_RING = 2,
    3GPP_NO_CARRIER = 3,
    3GPP_ERROR = 4,
    ATRPC_TIMEOUT = 5,
    3GPP_NO_DIALTONE = 6,
    3GPP_BUSY = 7,
    3GPP_NO_ANSWER = 8,
    SIMCOM_PROCEEDING = 9,
};

typedef ATRPC_HANDLE void *;
typedef void(*TA_RESPONSE)(void * const context, const ta_result_code result_code, const char * const response);

MOCKABLE_FUNCTION(, int, atrpc_attention, ATRPC_HANDLE, const handle, const char * const, command_string, const size_t, command_string_length, const size_t, timeout_ms, TA_RESPONSE const, ta_response, void * const, ta_response_context);
MOCKABLE_FUNCTION(, void, atrpc_close, ATRPC_HANDLE const, handle);
MOCKABLE_FUNCTION(, ATRPC_HANDLE, atrpc_create);
MOCKABLE_FUNCTION(, void, atrpc_destroy, ATRPC_HANDLE const, handle);
MOCKABLE_FUNCTION(, void, atrpc_dowork, ATRPC_HANDLE const, handle);
MOCKABLE_FUNCTION(, int, atrpc_open, ATRPC_HANDLE const, handle, TA_RESPONSE const, on_open_complete, void * const, on_open_complete_context);

#endif /* AT_RPC_H */
