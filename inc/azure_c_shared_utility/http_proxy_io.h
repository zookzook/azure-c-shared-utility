// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TLSIO_CYCLONESSL_H
#define TLSIO_CYCLONESSL_H

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif /* __cplusplus */

#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/umock_c_prod.h"

typedef struct HTTP_PROXY_IO_CONFIG_TAG
{
    const char* host_name;
    int port;
    const char* proxy_host_name;
    int proxy_port;
    const char* user_name;
    const char* password;
} HTTP_PROXY_IO_CONFIG;

MOCKABLE_FUNCTION(, const IO_INTERFACE_DESCRIPTION*, http_proxy_io_get_interface_description);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TLSIO_CYCLONESSL_H */
