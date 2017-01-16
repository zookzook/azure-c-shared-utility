// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/http_proxy_io.h"
#include "azure_c_shared_utility/base64.h"

typedef enum HTTP_PROXY_IO_STATE_TAG
{
    HTTP_PROXY_IO_STATE_CLOSED,
    HTTP_PROXY_IO_STATE_OPENING_UNDERLYING_IO,
    HTTP_PROXY_IO_STATE_WAITING_FOR_CONNECT_RESPONSE,
    HTTP_PROXY_IO_STATE_OPEN
} HTTP_PROXY_IO_STATE;

typedef struct HTTP_PROXY_IO_INSTANCE_TAG
{
    HTTP_PROXY_IO_STATE http_proxy_io_state;
    ON_BYTES_RECEIVED on_bytes_received;
    void* on_bytes_received_context;
    ON_IO_ERROR on_io_error;
    void* on_io_error_context;
    ON_IO_OPEN_COMPLETE on_io_open_complete;
    void* on_io_open_complete_context;
    char* host_name;
    int port;
    char* proxy_host_name;
    int proxy_port;
    char* user_name;
    char* password;
    XIO_HANDLE underlying_io;
    unsigned char* receive_buffer;
    size_t receive_buffer_size;
} HTTP_PROXY_IO_INSTANCE;

static CONCRETE_IO_HANDLE http_proxy_io_create(void* io_create_parameters)
{
    HTTP_PROXY_IO_INSTANCE* result;

    if (io_create_parameters == NULL)
    {
        result = NULL;
        LogError("NULL io_create_parameters.");
    }
    else
    {
        HTTP_PROXY_IO_CONFIG* http_proxy_io_config = io_create_parameters;

        if (http_proxy_io_config->host_name == NULL)
        {
            result = NULL;
            LogError("NULL host_name in the HTTP proxy IO configuration.");
        }
        else
        {
            result = malloc(sizeof(HTTP_PROXY_IO_INSTANCE));
            if (result == NULL)
            {
                LogError("Failed allocating HTTP proxy IO instance.");
            }
            else
            {
                if (mallocAndStrcpy_s(&result->host_name, http_proxy_io_config->host_name) != 0)
                {
                    LogError("Failed to copy the host_name.");
                    free(result);
                    result = NULL;
                }
                else
                {
                    if (mallocAndStrcpy_s(&result->proxy_host_name, http_proxy_io_config->proxy_host_name) != 0)
                    {
                        LogError("Failed to copy the proxy_host_name.");
                        free(result->host_name);
                        free(result);
                        result = NULL;
                    }
                    else
                    {
                        result->user_name = NULL;
                        result->password = NULL;

                        if ((http_proxy_io_config->user_name != NULL) && (mallocAndStrcpy_s(&result->user_name, http_proxy_io_config->user_name) != 0))
                        {
                            LogError("Failed to copy the user_name.");
                            free(result->proxy_host_name);
                            free(result->host_name);
                            free(result);
                            result = NULL;
                        }
                        else
                        {
                            if ((http_proxy_io_config->password != NULL) && (mallocAndStrcpy_s(&result->password, http_proxy_io_config->password) != 0))
                            {
                                LogError("Failed to copy the passowrd.");
                                free(result->user_name);
                                free(result->proxy_host_name);
                                free(result->host_name);
                                free(result);
                                result = NULL;
                            }
                            else
                            {
                                const IO_INTERFACE_DESCRIPTION* underlying_io_interface = socketio_get_interface_description();
                                if (underlying_io_interface == NULL)
                                {
                                    LogError("Unable to get the socket IO interface description.");
                                    free(result->password);
                                    free(result->user_name);
                                    free(result->proxy_host_name);
                                    free(result->host_name);
                                    free(result);
                                    result = NULL;
                                }
                                else
                                {
                                    SOCKETIO_CONFIG socket_io_config;

                                    socket_io_config.hostname = http_proxy_io_config->proxy_host_name;
                                    socket_io_config.port = http_proxy_io_config->proxy_port;

                                    result->underlying_io = xio_create(underlying_io_interface, &socket_io_config);
                                    if (result->underlying_io == NULL)
                                    {
                                        LogError("Unable to create the underlying IO.");
                                        free(result->password);
                                        free(result->user_name);
                                        free(result->proxy_host_name);
                                        free(result->host_name);
                                        free(result);
                                        result = NULL;
                                    }
                                    else
                                    {
                                        result->port = http_proxy_io_config->port;
                                        result->proxy_port = http_proxy_io_config->proxy_port;
                                        result->receive_buffer = NULL;
                                        result->receive_buffer_size = 0;
                                        result->http_proxy_io_state = HTTP_PROXY_IO_STATE_CLOSED;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return result;
}

static void http_proxy_io_destroy(CONCRETE_IO_HANDLE http_proxy_io)
{
    if (http_proxy_io == NULL)
    {
        LogError("NULL http_proxy_io.");
    }
    else
    {
        HTTP_PROXY_IO_INSTANCE* http_proxy_io_instance = (HTTP_PROXY_IO_INSTANCE*)http_proxy_io;

        free(http_proxy_io_instance->receive_buffer);
        xio_destroy(http_proxy_io_instance->underlying_io);
        free(http_proxy_io_instance->host_name);
        free(http_proxy_io_instance);
    }
}

static void indicate_open_complete_error_and_close(HTTP_PROXY_IO_INSTANCE* http_proxy_io_instance)
{
    http_proxy_io_instance->http_proxy_io_state = HTTP_PROXY_IO_STATE_CLOSED;
    http_proxy_io_instance->on_io_open_complete(http_proxy_io_instance->on_io_open_complete_context, IO_OPEN_ERROR);
    xio_close(http_proxy_io_instance->underlying_io, NULL, NULL);
}

static void on_underlying_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    if (context == NULL)
    {
        LogError("NULL context in on_underlying_io_open_complete");
    }
    else
    {
        HTTP_PROXY_IO_INSTANCE* http_proxy_io_instance = (HTTP_PROXY_IO_INSTANCE*)context;
        switch (http_proxy_io_instance->http_proxy_io_state)
        {
        default:
            break;

        case HTTP_PROXY_IO_STATE_OPENING_UNDERLYING_IO:
            if (open_result != IO_OPEN_OK)
            {
                LogError("Underlying IO open failed");
                indicate_open_complete_error_and_close(http_proxy_io_instance);
            }
            else
            {
                STRING_HANDLE encoded_auth_string;
                const char* auth_string_payload;

                /* Send connect */
                http_proxy_io_instance->http_proxy_io_state = HTTP_PROXY_IO_STATE_WAITING_FOR_CONNECT_RESPONSE;

                if (http_proxy_io_instance->user_name != NULL)
                {
                    STRING_HANDLE plain_auth_string;
                    plain_auth_string = STRING_construct_sprintf("%s:%s", http_proxy_io_instance->user_name, (http_proxy_io_instance->password == NULL) ? "" : http_proxy_io_instance->password);
                    if (plain_auth_string == NULL)
                    {
                        LogError("Cannot construct auth string");
                        encoded_auth_string = NULL;
                    }
                    else
                    {
                        size_t auth_string_length = STRING_length(plain_auth_string);
                        const char* plain_auth_string_bytes = STRING_c_str(plain_auth_string);

                        encoded_auth_string = Base64_Encode_Bytes((const unsigned char*)plain_auth_string_bytes, auth_string_length);
                        if (encoded_auth_string == NULL)
                        {
                            LogError("Cannot Base64 encode auth string");
                        }

                        STRING_delete(plain_auth_string);
                    }
                }
                else
                {
                    encoded_auth_string = NULL;
                }

                if ((http_proxy_io_instance->user_name != NULL) &&
                    (encoded_auth_string == NULL))
                {
                    LogError("Cannot create authorization header");
                }
                else
                {
                    STRING_HANDLE connect_request;
                    if (http_proxy_io_instance->user_name != NULL)
                    {
                        auth_string_payload = STRING_c_str(encoded_auth_string);
                    }
                    else
                    {
                        auth_string_payload = "";
                    }

                    connect_request = STRING_construct_sprintf("CONNECT %s:%d HTTP/1.1\r\nHost:%s:%d%s%s\r\n\r\n",
                        http_proxy_io_instance->host_name,
                        http_proxy_io_instance->port,
                        http_proxy_io_instance->host_name,
                        http_proxy_io_instance->port,
                        (http_proxy_io_instance->user_name != NULL) ? "\r\nProxy-authorization: Basic " : "",
                        auth_string_payload);

                    if (connect_request == NULL)
                    {
                        LogError("Could not construct CONNECT request");
                        indicate_open_complete_error_and_close(http_proxy_io_instance);
                    }
                    else
                    {
                        const char* connect_request_bytes = STRING_c_str(connect_request);
                        size_t connect_request_length = STRING_length(connect_request);

                        if (xio_send(http_proxy_io_instance->underlying_io, connect_request_bytes, connect_request_length, NULL, NULL) != 0)
                        {
                            LogError("Could not send CONNECT request");
                            indicate_open_complete_error_and_close(http_proxy_io_instance);
                        }

                        STRING_delete(connect_request);
                    }
                }

                if (encoded_auth_string != NULL)
                {
                    STRING_delete(encoded_auth_string);
                }
            }

            break;
        }
    }
}

static void on_underlying_io_error(void* context)
{
    HTTP_PROXY_IO_INSTANCE* http_proxy_io_instance = (HTTP_PROXY_IO_INSTANCE*)context;

    switch (http_proxy_io_instance->http_proxy_io_state)
    {
    default:
        break;

    case HTTP_PROXY_IO_STATE_OPENING_UNDERLYING_IO:
    case HTTP_PROXY_IO_STATE_WAITING_FOR_CONNECT_RESPONSE:
        http_proxy_io_instance->http_proxy_io_state = HTTP_PROXY_IO_STATE_CLOSED;
        indicate_open_complete_error_and_close(http_proxy_io_instance);
        break;

    case HTTP_PROXY_IO_STATE_OPEN:
        http_proxy_io_instance->on_io_error(http_proxy_io_instance->on_io_error_context);
        break;
    }
}

/*the following function does the same as sscanf(pos2, "%d", &sec)*/
/*this function only exists because some of platforms do not have sscanf. */
static int ParseStringToDecimal(const char *src, int* dst)
{
    int result;
    char* next;

    (*dst) = strtol(src, &next, 0);
    if ((src == next) || ((((*dst) == LONG_MAX) || ((*dst) == LONG_MIN)) && (errno != 0)))
    {
        result = __LINE__;
    }
    else
    {
        result = 0;
    }

    return result;
}

/*the following function does the same as sscanf(buf, "HTTP/%*d.%*d %d %*[^\r\n]", &ret) */
/*this function only exists because some of platforms do not have sscanf. This is not a full implementation; it only works with well-defined HTTP response. */
static int ParseHttpResponse(const char* src, int* dst)
{
    int result;
    static const char HTTPPrefix[] = "HTTP/";
    bool fail;
    const char* runPrefix;

    if ((src == NULL) || (dst == NULL))
    {
        result = __LINE__;
    }
    else
    {
        fail = false;
        runPrefix = HTTPPrefix;

        while ((*runPrefix) != '\0')
        {
            if ((*runPrefix) != (*src))
            {
                fail = true;
                break;
            }
            src++;
            runPrefix++;
        }

        if (!fail)
        {
            while ((*src) != '.')
            {
                if ((*src) == '\0')
                {
                    fail = true;
                    break;
                }
                src++;
            }
        }

        if (!fail)
        {
            while ((*src) != ' ')
            {
                if ((*src) == '\0')
                {
                    fail = true;
                    break;
                }
                src++;
            }
        }

        if (fail)
        {
            result = __LINE__;
        }
        else
        {
            if (ParseStringToDecimal(src, dst) != 0)
            {
                result = __LINE__;
            }
            else
            {
                result = 0;
            }
        }
    }

    return result;
}

static void on_underlying_io_bytes_received(void* context, const unsigned char* buffer, size_t size)
{
    HTTP_PROXY_IO_INSTANCE* http_proxy_io_instance = (HTTP_PROXY_IO_INSTANCE*)context;

    if (http_proxy_io_instance->http_proxy_io_state == HTTP_PROXY_IO_STATE_WAITING_FOR_CONNECT_RESPONSE)
    {
        unsigned char* new_receive_buffer = realloc(http_proxy_io_instance->receive_buffer, http_proxy_io_instance->receive_buffer_size + size + 1);
        if (new_receive_buffer != NULL)
        {
            http_proxy_io_instance->receive_buffer = new_receive_buffer;
            memcpy(http_proxy_io_instance->receive_buffer + http_proxy_io_instance->receive_buffer_size, buffer, size);
            http_proxy_io_instance->receive_buffer_size += size;
        }

        if (http_proxy_io_instance->receive_buffer_size >= 4)
        {
            char* request_end_ptr;

            http_proxy_io_instance->receive_buffer[http_proxy_io_instance->receive_buffer_size] = 0;

            if ((http_proxy_io_instance->receive_buffer_size >= 4) &&
                ((request_end_ptr = strstr((const char*)http_proxy_io_instance->receive_buffer, "\r\n\r\n")) != NULL))
            {
                int status_code;

                /* This part should really be done with the HTTPAPI, but that has to be done as a separate step
                as the HTTPAPI has to expose somehow the underlying IO and currently this would be a too big of a change. */

                if (ParseHttpResponse((const char*)http_proxy_io_instance->receive_buffer, &status_code) != 0)
                {
                    LogError("Cannot decode HTTP response");
                    indicate_open_complete_error_and_close(http_proxy_io_instance);
                }
                else if (status_code != 200)
                {
                    LogError("Bad status (%d) received in CONNECT response", status_code);
                    indicate_open_complete_error_and_close(http_proxy_io_instance);
                }
                else
                {
                    size_t length_remaining = http_proxy_io_instance->receive_buffer - ((const unsigned char *)request_end_ptr + 4);

                    http_proxy_io_instance->http_proxy_io_state = HTTP_PROXY_IO_STATE_OPEN;
                    http_proxy_io_instance->on_io_open_complete(http_proxy_io_instance->on_io_open_complete_context, IO_OPEN_OK);

                    if (length_remaining > 0)
                    {
                        http_proxy_io_instance->on_bytes_received(http_proxy_io_instance->on_bytes_received_context, (const unsigned char*)request_end_ptr + 4, length_remaining);
                    }
                }
            }
        }
    }
    else
    {
        /* bubble up received bytes */
        http_proxy_io_instance->on_bytes_received(http_proxy_io_instance->on_bytes_received_context, buffer, size);
    }
}

static int http_proxy_io_open(CONCRETE_IO_HANDLE http_proxy_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    int result;

    if ((http_proxy_io == NULL) ||
        (on_io_open_complete == NULL) ||
        (on_bytes_received == NULL) ||
        (on_io_error == NULL))
    {
        LogError("NULL http_proxy_io.");
        result = __LINE__;
    }
    else
    {
        HTTP_PROXY_IO_INSTANCE* http_proxy_io_instance = (HTTP_PROXY_IO_INSTANCE*)http_proxy_io;

        if (http_proxy_io_instance->http_proxy_io_state != HTTP_PROXY_IO_STATE_CLOSED)
        {
            LogError("Invalid tlsio_state. Expected state is HTTP_PROXY_IO_STATE_CLOSED.");
            result = __LINE__;
        }
        else
        {
            http_proxy_io_instance->on_bytes_received = on_bytes_received;
            http_proxy_io_instance->on_bytes_received_context = on_bytes_received_context;

            http_proxy_io_instance->on_io_error = on_io_error;
            http_proxy_io_instance->on_io_error_context = on_io_error_context;

            http_proxy_io_instance->on_io_open_complete = on_io_open_complete;
            http_proxy_io_instance->on_io_open_complete_context = on_io_open_complete_context;

            http_proxy_io_instance->http_proxy_io_state = HTTP_PROXY_IO_STATE_OPENING_UNDERLYING_IO;

            if (xio_open(http_proxy_io_instance->underlying_io, on_underlying_io_open_complete, http_proxy_io_instance, on_underlying_io_bytes_received, http_proxy_io_instance, on_underlying_io_error, http_proxy_io_instance) != 0)
            {
                http_proxy_io_instance->http_proxy_io_state = HTTP_PROXY_IO_STATE_CLOSED;
                LogError("Cannot open the underlying IO.");
                result = __LINE__;
            }
            else
            {
                result = 0;
            }
        }
    }

    return result;
}

static int http_proxy_io_close(CONCRETE_IO_HANDLE http_proxy_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* on_io_close_complete_context)
{
    int result = 0;

    if (http_proxy_io == NULL)
    {
        result = __LINE__;
        LogError("NULL http_proxy_io.");
    }
    else
    {
        HTTP_PROXY_IO_INSTANCE* http_proxy_io_instance = (HTTP_PROXY_IO_INSTANCE*)http_proxy_io;

        if (http_proxy_io_instance->http_proxy_io_state != HTTP_PROXY_IO_STATE_OPEN)
        {
            result = __LINE__;
            LogError("Invalid tlsio_state. Expected state is HTTP_PROXY_IO_STATE_OPEN.");
        }
        else
        {
            (void)xio_close(http_proxy_io_instance->underlying_io, NULL, NULL);
            http_proxy_io_instance->http_proxy_io_state = HTTP_PROXY_IO_STATE_CLOSED;

            on_io_close_complete(on_io_close_complete_context);

            result = 0;
        }
    }

    return result;
}

static int http_proxy_io_send(CONCRETE_IO_HANDLE http_proxy_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* on_send_complete_context)
{
    int result;

    if ((http_proxy_io == NULL) ||
        (buffer == NULL) ||
        /* Codes_SRS_TLSIO_CYCLONESSL_01_044: [ If size is 0, tlsio_cyclonessl_send shall fail and return a non-zero value. ]*/
        (size == 0))
    {
        result = __LINE__;
        LogError("NULL http_proxy_io.");
    }
    else
    {
        HTTP_PROXY_IO_INSTANCE* http_proxy_io_instance = (HTTP_PROXY_IO_INSTANCE*)http_proxy_io;

        if (http_proxy_io_instance->http_proxy_io_state != HTTP_PROXY_IO_STATE_OPEN)
        {
            result = __LINE__;
            LogError("Invalid tlsio_state. Expected state is HTTP_PROXY_IO_STATE_OPEN.");
        }
        else
        {
            if (xio_send(http_proxy_io_instance->underlying_io, buffer, size, on_send_complete, on_send_complete_context) != 0)
            {
                result = __LINE__;
                LogError("Underlying xio_send failed.");
            }
            else
            {
                result = 0;
            }
        }
    }

    return result;
}

static void http_proxy_io_dowork(CONCRETE_IO_HANDLE http_proxy_io)
{
    if (http_proxy_io == NULL)
    {
        LogError("NULL http_proxy_io.");
    }
    else
    {
        HTTP_PROXY_IO_INSTANCE* http_proxy_io_instance = (HTTP_PROXY_IO_INSTANCE*)http_proxy_io;

        if (http_proxy_io_instance->http_proxy_io_state != HTTP_PROXY_IO_STATE_CLOSED)
        {
            xio_dowork(http_proxy_io_instance->underlying_io);
        }
    }
}

static int http_proxy_io_setoption(CONCRETE_IO_HANDLE http_proxy_io, const char* optionName, const void* value)
{
    int result;

    /* Tests_SRS_TLSIO_CYCLONESSL_01_057: [ If any of the arguments tls_io or option_name is NULL tlsio_cyclonessl_setoption shall return a non-zero value. ]*/
    if ((http_proxy_io == NULL) || (optionName == NULL))
    {
        LogError("NULL http_proxy_io");
        result = __LINE__;
    }
    else
    {
        HTTP_PROXY_IO_INSTANCE* http_proxy_io_instance = (HTTP_PROXY_IO_INSTANCE*)http_proxy_io;

        if (xio_setoption(http_proxy_io_instance->underlying_io, optionName, value) != 0)
        {
            LogError("Unrecognized option");
            result = __LINE__;
        }
        else
        {
            result = 0;
        }
    }

    return result;
}

static OPTIONHANDLER_HANDLE http_proxy_io_retrieve_options(CONCRETE_IO_HANDLE http_proxy_io)
{
    OPTIONHANDLER_HANDLE result;

    if (http_proxy_io == NULL)
    {
        LogError("invalid parameter detected: CONCRETE_IO_HANDLE handle=%p", http_proxy_io);
        result = NULL;
    }
    else
    {
        HTTP_PROXY_IO_INSTANCE* http_proxy_io_instance = (HTTP_PROXY_IO_INSTANCE*)http_proxy_io;

        result = xio_retrieveoptions(http_proxy_io_instance->underlying_io);
        if (result == NULL)
        {
            LogError("unable to create option handler");
        }
    }
    return result;
}

static const IO_INTERFACE_DESCRIPTION http_proxy_io_interface_description =
{
    http_proxy_io_retrieve_options,
    http_proxy_io_create,
    http_proxy_io_destroy,
    http_proxy_io_open,
    http_proxy_io_close,
    http_proxy_io_send,
    http_proxy_io_dowork,
    http_proxy_io_setoption
};

const IO_INTERFACE_DESCRIPTION* http_proxy_io_get_interface_description(void)
{
    return &http_proxy_io_interface_description;
}
