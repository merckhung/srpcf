/*
 * SRPCF - Simple Remote Procedire Command Framework
 * File: xrHelloWorld.c
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "srpcf_plugin.h"


// SRPCF Shell Help
LIBSRPCF_HELPER_TEXT( "\
SYNTAX:\n\
\txrHelloWorld\n\
USAGE:\n\
\tHello World Help.\n\
" );


// SRPCF Shell Error Handle
LIBSRPCF_ERROR_FUNC( xrHelloWorld ) {}


// SRPCF Shell Implementation
LIBSRPCF_SHELL_IMPLEMENT( xrHelloWorld ) {

	LIBSRPCF_SRPCF_FOREACH {
	}

	if( idx )
		return TRUE;

	return FALSE;
}


// SRPCF Server Implementation
LIBSRPCF_SERVER_IMPLEMENT( xrHelloWorld ) {

	s8 *str = "Hello World\n";
    s8 *p;

	// Allocate string buffer for returning
	// This buffer will be freed later by the SRPCF server,
	// after finish the socket tranmission
	p = malloc( strlen( str ) + 1 );
	if( !p ) {

		*errorCode = SRPCF_FAILED_NODEV;
		return NULL;
	}

	// Copy the string to the new buffer
	strncpy( p, str, strlen( str ) + 1 );

	// Indicate that it's executed successfully at the server side
    *errorCode = SRPCF_SUCCESSFUL;

	// Return the address of the string buffer
    return p;
}


