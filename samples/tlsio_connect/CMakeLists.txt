#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

compileAsC99()

set(tlsio_connect_c_files
    main.c
)

if(${use_mbedtls})
	add_subdirectory(mbed)
	set(tlsio_connect_c_files ${tlsio_connect_c_files} "./mbed/googlecerts.c")
endif()

IF(WIN32)
    #windows needs this define
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DGB_MEASURE_MEMORY_FOR_THIS -DGB_DEBUG_ALLOC)
ENDIF(WIN32)

add_executable(tlsio_connect ${tlsio_connect_c_files})

if(${use_mbedtls})
	find_library(MBEDTLS_LIBRARY NAMES mbedtls HINTS /usr/local/lib)
	find_library(MBEDCRYPTO_LIBRARY NAMES mbedcrypto HINTS /usr/local/lib)
	find_library(MBEDX509_LIBRARY NAMES mbedx509 HINTS /usr/local/lib)
	message(STATUS "in use mbed ${MBEDTLS_LIBRARY}	")
	message(STATUS "in use mbed ${MBEDCRYPTO_LIBRARY}	")
	message(STATUS "in use mbed ${MBEDX509_LIBRARY}	")
    target_link_libraries(tlsio_connect aziotsharedutil ${MBEDX509_LIBRARY} ${MBEDTLS_LIBRARY} ${MBEDCRYPTO_LIBRARY})
else()
	target_link_libraries(tlsio_connect 
		aziotsharedutil)
endif()

if(${use_openssl} AND WIN32)
	file(COPY $ENV{OpenSSLDir}/bin/libeay32.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	file(COPY $ENV{OpenSSLDir}/bin/ssleay32.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

set_target_properties(tlsio_connect
			   PROPERTIES
			   FOLDER "azure_c_shared_utility_samples")
