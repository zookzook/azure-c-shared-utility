// Copyright(c) Microsoft.All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// This file is made an integral part of a tlsio adapter implementation with a #include. It
// is broken out for readability. 

#include "unit_test_api.h"

// tlsio_verify_internal_state compares the supplied expected_state with the internal state
// of the tlsio adapter, and the expected_message_queue_length with the actual
// message queue length, and uses xlogging to log any discrepancies. It returns 0 if there
// are no discrepancies, and non-zero otherwise. This function is not guaranteed to be 
// accurate during tlsio callbacks.
// This function exists only for unit testing builds and must never be
// called in production code.
int tlsio_verify_internal_state(const CONCRETE_IO_HANDLE tlsio_in,
	TLSIO_STATE_EXT expected_state, size_t expected_message_queue_length)
{
	TLS_IO_INSTANCE* tlsio = (TLS_IO_INSTANCE*)tlsio_in;

	bool state_ok;
	int result;

	// Find the actual message queue size
	size_t message_queue_length = 0;
	if (tlsio->pending_transmission_list != NULL)
	{
		LIST_ITEM_HANDLE list = singlylinkedlist_get_head_item(tlsio->pending_transmission_list);
		while (list != NULL)
		{
			message_queue_length++;
			list = singlylinkedlist_get_next_item(list);
		}
	}

	// Make sure callbacks haven't gotten munged
	bool callbacks_are_set =
		tlsio->on_bytes_received != NULL &&
		tlsio->on_open_complete != NULL &&
		tlsio->on_io_error != NULL;

	bool callbacks_are_null =
		tlsio->on_bytes_received == NULL &&
		tlsio->on_open_complete == NULL &&
		tlsio->on_io_error == NULL &&
		tlsio->on_bytes_received_context == NULL &&
		tlsio->on_io_error_context == NULL &&
		tlsio->on_open_complete_context == NULL;


	switch (tlsio->tlsio_state)
	{
	case TLSIO_STATE_CLOSED: state_ok = expected_state == TLSIO_STATE_EXT_CLOSED; break;
	case TLSIO_STATE_OPENING_WAITING_DNS: state_ok = expected_state == TLSIO_STATE_EXT_OPENING; break;
	case TLSIO_STATE_OPENING_WAITING_SOCKET: state_ok = expected_state == TLSIO_STATE_EXT_OPENING; break;
	case TLSIO_STATE_OPENING_WAITING_SSL: state_ok = expected_state == TLSIO_STATE_EXT_OPENING; break;
	case TLSIO_STATE_OPEN: state_ok = expected_state == TLSIO_STATE_EXT_OPEN; break;
	case TLSIO_STATE_ERROR: state_ok = expected_state == TLSIO_STATE_EXT_ERROR; break;
	default: state_ok = false;
	}

	if (!state_ok)
	{
		LogError("Unexptected internal tlsio_state %d does not map to external state %d", tlsio->tlsio_state, expected_state);
		result = __FAILURE__;
	}
	else if (expected_message_queue_length != message_queue_length)
	{
		LogError("Expected message queue size %u does not match actual %u", expected_message_queue_length, message_queue_length);
		result = __FAILURE__;
	}
	else if (tlsio->tlsio_state == TLSIO_STATE_CLOSED && !callbacks_are_null)
	{
		LogError("Unexpected callback values while TLSIO_STATE_CLOSED");
		result = __FAILURE__;
	}
	else if (tlsio->tlsio_state != TLSIO_STATE_CLOSED && !callbacks_are_set)
	{
		LogError("Unexpected callback values while not in TLSIO_STATE_CLOSED");
		result = __FAILURE__;
	}
	else if (tlsio->hostname == NULL)
	{
		LogError("Hostname is NULL");
		result = __FAILURE__;
	}
	else
	{
		result = 0;
	}

	return result;
}
