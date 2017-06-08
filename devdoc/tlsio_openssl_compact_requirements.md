# tlsio_openssl_compact


## Overview

The tlsio_openssl_compact adapter implements a tlsio adapter for compact OpenSSL platforms such as Espressif's ESP32.  


## References

[tlsio base specification](https://github.com/Azure/azure-c-shared-utility/blob/master/devdoc/tlsio.md)

[OpenSSL](https://www.openssl.org/)

## Base Specification

The tlsio_openssl_compact adapter conforms to the
[tlsio base specification](https://github.com/Azure/azure-c-shared-utility/blob/master/devdoc/tlsio.md) and 
additionally implements the [Unit Testing API](#Unit-Testing-API) below.


## API Calls


###   tlsio_setoption

**SRS_TLSIO_OPENSSL_COMPACT_30_520 [** The `tlsio_setoption` shall do nothing and return 0. **]**


###   tlsio_retrieveoptions

**SRS_TLSIO_OPENSSL_COMPACT_30_560: [** The `tlsio_retrieveoptions` shall do nothing and return NULL. **]**




## Unit Testing API

The unit testing API allows verification of internal tlsio adapter states without violating the principle of encapsulation.

Because the `tlsio_verify_internal_state` function operates on internal implementation, there is no need
to capture the details of its internal behavior in formal documentation. Code commenting is the correct 
level of documentation.

The Unit Testing API is only compiled when TLSIO_STATE_VERIFICATION_ENABLE is defined. The implementation 
of the Unit Testing API may go directly within a #define block in the tlsio adapter's implementation file,
or the implementation file may conditionally include a code file that lives with the unit tests.

**SRS_TLSIO_OPENSSL_COMPACT_30_300: [** If TLSIO_STATE_VERIFICATION_ENABLE is #defined in the compiler, the tlsio adapter shall 
implement `tlsio_verify_internal_state`.
```c
#ifdef TLSIO_STATE_VERIFICATION_ENABLE

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
int tlsio_verify_internal_state(const CONCRETE_IO_HANDLE tlsio,
	TLSIO_STATE_EX expected_state, uint32_t expected_message_queue_length);
    
#define TLSIO_ASSERT_INTERNAL_STATE(a, b, c) ASSERT_ARE_EQUAL(int, 0, tlsio_verify_internal_state(a,b,c))

#endif // TLSIO_STATE_VERIFICATION_ENABLE
```
**]**

