// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AT_RPC_H
#define AT_RPC_H

#include "azure_c_shared_utility/umock_c_prod.h"

#ifdef __cplusplus
  extern "C" {
#endif

typedef struct ATRPC_INSTANCE_TAG * ATRPC_HANDLE;

#define TA_RESULT_CODE_VALUES \
    OK_3GPP = 0, \
    CONNECT_3GPP = 1, \
    RING_3GPP = 2, \
    NO_CARRIER_3GPP = 3, \
    ERROR_3GPP = 4, \
    ERROR_ATRPC = 5, \
    NO_DIALTONE_3GPP = 6, \
    BUSY_3GPP = 7, \
    NO_ANSWER_3GPP = 8, \
    PROCEEDING_SIMCOM = 9

DEFINE_ENUM(TA_RESULT_CODE, TA_RESULT_CODE_VALUES);

typedef void(*ON_TA_RESPONSE)(void * context, TA_RESULT_CODE result_code, const char * response);

MOCKABLE_FUNCTION(, int, atrpc_attention, ATRPC_HANDLE, handle, const char *, command_string, size_t, command_string_length, size_t, timeout_ms, ON_TA_RESPONSE, on_ta_response, void *, ta_response_context);
MOCKABLE_FUNCTION(, int, atrpc_close, ATRPC_HANDLE, handle);
MOCKABLE_FUNCTION(, ATRPC_HANDLE, atrpc_create);
MOCKABLE_FUNCTION(, void, atrpc_destroy, ATRPC_HANDLE, handle);
MOCKABLE_FUNCTION(, void, atrpc_dowork, ATRPC_HANDLE, handle);
MOCKABLE_FUNCTION(, int, atrpc_open, ATRPC_HANDLE, handle, ON_TA_RESPONSE, on_open_complete, void *, on_open_complete_context);

#ifdef __cplusplus
  }
#endif

#endif /* AT_RPC_H */
