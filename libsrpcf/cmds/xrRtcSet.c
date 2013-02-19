/*
 * SRPCF - Simple Remote Procedire Command Framework
 * File: xrRtcSet.c
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
\txrRtcSet\n\
USAGE:\n\
\txrRtcSet\n\
" );


// SRPCF Shell Error Handle
LIBSRPCF_ERROR_FUNC( xrRtcSet ) {}


// SRPCF Shell Implementation
LIBSRPCF_SHELL_IMPLEMENT( xrRtcSet ) {

	LIBSRPCF_SRPCF_FOREACH {

		// Do something at client side
		//ppCmdOpt->value;
	}

	return FALSE;
}


//
// SRPCF Server Implementation
//
LIBSRPCF_SERVER_IMPLEMENT( xrRtcSet ) {

	// Do something at server side
	*errorCode = SRPCF_SUCCESSFUL;
	return NULL;
}


