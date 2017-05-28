// Copyright(c) Microsoft.All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// This file is made an integral part of a tlsio adapter implementation with a #include. It
// is broken out for readability. 
#ifndef TLSIO_UNIT_TEST_API_H
#define TLSIO_UNIT_TEST_API_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// The tlsio external state values are the states of the tlsio adapter
// as seen by the caller on the basis of calls made and callbacks received.
typedef enum
{
	TLSIO_STATE_EXT_CLOSED,
	TLSIO_STATE_EXT_OPENING,
	TLSIO_STATE_EXT_OPEN,
	TLSIO_STATE_EXT_CLOSING,
	TLSIO_STATE_EXT_ERROR
} TLSIO_STATE_EXT;

// tlsio_verify_internal_state compares the supplied expected_state with the internal state
// of the tlsio adapter, and the expected_message_queue_length with the actual
// message queue length, and uses xlogging to log any discrepancies. It returns 0 if there
// are no discrepancies, and non-zero otherwise. This function is not guaranteed to be 
// accurate during tlsio callbacks.
// This function exists only for unit testing builds and must never be
// called in production code.
int tlsio_verify_internal_state(const CONCRETE_IO_HANDLE tlsio_in,
	TLSIO_STATE_EXT expected_state, size_t expected_message_queue_length);

#define TLSIO_ASSERT_INTERNAL_STATE(a, b, c) ASSERT_ARE_EQUAL(int, 0, tlsio_verify_internal_state(a,b,c))
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // TLSIO_UNIT_TEST_API_H