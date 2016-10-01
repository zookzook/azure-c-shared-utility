// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <msp430.h>
#include <driverlib.h>

#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/vector.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/xlogging.h"

#define BUFFER_RESULTS 1
#define TURBO_BUTTON 0
#define LUDICROUS_SPEED 0

bool shutdown = false;

static void on_close_complete(void * context) {
	shutdown = true;
}

static void on_send_complete(void * context, IO_SEND_RESULT result) {
	switch (result) {
	  case IO_SEND_OK:
	  case IO_SEND_ERROR:
		(void)xio_close(context, on_close_complete, NULL);
		break;
	}
}

static void on_io_error(void* context)
{
    (void)context;
    LogError("IO reported an error\r\n");
}

static void on_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    (void)context, open_result;
    LogError("Open complete called\r\n");

    if (open_result == IO_OPEN_OK)
    {
        LogError("Sending bytes ...\r\n");
        XIO_HANDLE tlsio = (XIO_HANDLE)context;
        const char to_send[] = "GET / HTTP/1.1\r\n"
            "Host: www.google.com\r\n"
            "\r\n";
        if (xio_send(tlsio, to_send, sizeof(to_send), on_send_complete, context) != 0)
        {
            LogError("Send failed\r\n");
        }
    }
    else
    {
        LogError("Open error\r\n");
    }
}

static void on_io_bytes_received(void* context, const unsigned char* buffer, size_t size)
{
#if BUFFER_RESULTS
	VECTOR_HANDLE vec = (VECTOR_HANDLE)context;
    VECTOR_push_back(vec, buffer, size);
#else
	(void)context, buffer;
#endif
    LogInfo("Received %zu bytes\r\n", size);
}

//*****************************************************************************
//
//!/*CLOCK_INIT*/
//!Brief Clock_Init function will assign clock for ACLK,SMCLK,MCLK
//!
//           MSP430FR59x
//         ---------------
//     /|\|               |
//      | |               |-LFXIN
//      --|RST            |-LFXOUT
//        |               |
//        |               |-HFXIN
//        |               |-HFXOUT
//        |               |
//        |                 |---> LED
//        |               |---> ACLK = 32768Hz
//        |               |---> SMCLK = 8MHz
//! \return None

//*****************************************************************************
int
msp430_init (
    void
) {
    WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer
  #if TURBO_BUTTON
   #if LUDICROUS_SPEED
    (void)CS_setDCOFreq(CS_DCORSEL_1, CS_DCOFSEL_6);
   #else
    (void)CS_setDCOFreq(CS_DCORSEL_1, CS_DCOFSEL_4);
   #endif
    (void)CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_2);
  #else
    // Default values
    (void)CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_6);
    (void)CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_8);
  #endif

    // Initialize Port A
    PAOUT = 0x00;  // Set Port A to LOW
    PADIR = 0x00;  // Set Port A to INPUT

    // Initialize Port B
    PBOUT = 0x00;  // Set Port B to LOW
    PBDIR = 0x00;  // Set Port B to OUTPUT

    PM5CTL0 &= ~LOCKLPM5;        // Disable the GPIO power-on default high-impedance mode to
                                // activate previously configured port settingsGS (affects RTC)
    __bis_SR_register(GIE);        // Enable Global Interrupt

    return platform_init();
}

int main(int argc, void** argv)
{
    int result;
#if BUFFER_RESULTS
    VECTOR_HANDLE tcp_response = VECTOR_create(sizeof(uint8_t));
#endif

    (void)argc, argv;

    if (msp430_init() != 0)
    {
        LogError("Cannot initialize platform.");
        result = __LINE__;
    }
    else
    {
        const IO_INTERFACE_DESCRIPTION* tlsio_interface = platform_get_default_tlsio();
        if (tlsio_interface == NULL)
        {
            LogError("Error getting tlsio interface description.");
            result = __LINE__;
        }
        else
        {
            TLSIO_CONFIG tlsio_config;
            XIO_HANDLE tlsio;

            tlsio_config.hostname = "www.google.com";
            tlsio_config.port = 443;
            tlsio = xio_create(tlsio_interface, &tlsio_config);
            if (tlsio == NULL)
            {
                LogError("Error creating TLS IO.");
                result = __LINE__;
            }
            else if ( xio_setoption(tlsio, "apn", "wholesale") )
            {
            	LogError("Failed to set access point name (\"apn\") for TLS XIO layer\n");
                result = __LINE__;
            }
#if BUFFER_RESULTS
            else if (xio_open(tlsio, on_io_open_complete, tlsio, on_io_bytes_received, tcp_response, on_io_error, tlsio) != 0)
#else
            else if (xio_open(tlsio, on_io_open_complete, tlsio, on_io_bytes_received, tlsio, on_io_error, tlsio) != 0)
#endif
			{
				LogError("Error opening TLS IO.");
				result = __LINE__;
			}
			else
			{
				for (;!shutdown;)
				{
					xio_dowork(tlsio);
				}

				result = 0;
			}

#if BUFFER_RESULTS
            VECTOR_destroy(tcp_response);
#endif
            xio_destroy(tlsio);
        }

        platform_deinit();
    }

    return result;
}
