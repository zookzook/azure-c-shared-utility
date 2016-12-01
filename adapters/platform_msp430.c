// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/platform.h"

#ifdef __cplusplus
  #include <cstddef>
#else
  #include <stddef.h>
#endif // __cplusplus

#include <driverlib.h>

#ifdef _MSC_VER
#pragma warning(disable:4068)
#endif

/******************************************************************************
 * Interrupt to signal SIM808 UART module has become available
 ******************************************************************************/
#pragma vector=PORT3_VECTOR
__interrupt void PORT3_ISR(void)
{
  #pragma diag_push
  /*
   * (ULP 10.1) ISR Sim808_Ready calls function GPIO_disableInterrupt.
   *
   * Recommend moving function call away from ISR, or inlining the function, or using pragmas
   */
  //TODO: Confirm decision or address suppression diagnostic message
  #pragma diag_suppress 1538
	//(void)GPIO_disableInterrupt(GPIO_PORT_P3,GPIO_PIN5);
  #pragma diag_pop
}


int
platform_init (
	void
) {
	return __LINE__;
}


void
platform_deinit (
	void
) {
}


const IO_INTERFACE_DESCRIPTION *
platform_get_default_tlsio (
	void
) {
	return (const IO_INTERFACE_DESCRIPTION *)NULL;
}

