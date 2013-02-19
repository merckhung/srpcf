/*
 * SRPCF - Simple Remote Procedire Command Framework
 * File: xrHelp.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>

#include "srpcf_types.h"
#include "srpcf.h"
#include "srpcf_err.h"
#include "libsrpcf.h"
#include "srpcfsvr.h"
#include "srpcfsh.h"


// SRPCF Shell Help
LIBSRPCF_HELPER_TEXT( "\
SYNTAX:\n\
\txrHelp <SrpcfCmd>\n\
\t<SrpcfCmd> SRPCF Command Name.\n\
USAGE:\n\
\tThe function will display the help message for the specified\n\
\tSRPCF command or a list of all SRPCF command if no SrpcfCmd is supplied.\n\
" );


// SRPCF Shell Error Handle
LIBSRPCF_ERROR_FUNC( xrHelp ) {}


// SRPCF Shell Implementation
LIBSRPCF_SHELL_IMPLEMENT( xrHelp ) {

	s8 *(*srpcfFuncHelper)(void);
	srpcfSupported_t *(*funcSrpcfSupported)(void);
	void *handle;
	s8 helper[ SRPCF_FUNC_MAXLEN ];

	// Only allow i parameter
	if( numOpts != 1 )
		return FALSE;

	LIBSRPCF_SRPCF_FOREACH {

	    // Open instance itself
   		handle = dlopen( NULL, RTLD_LAZY );
    	if( !handle ) {

			dlclose( handle );
			return FALSE;
    	}

		// Find supportedSrpcf structure
		funcSrpcfSupported = dlsym( handle, "retriveSrpcfSupported" );
		if( !funcSrpcfSupported ) {

			dlclose( handle );
			return FALSE;
		}

        // Check for this SRPCF command
        if( checkSrpcfCmdEnabled( ppCmdOpt->value, funcSrpcfSupported() ) == XR_END_SRPCF )
            return FALSE;

		// Lookup for symbol
		snprintf( helper, SRPCF_FUNC_MAXLEN, SRPCF_HELPER_PREFIX "%s", ppCmdOpt->value );
		srpcfFuncHelper = dlsym( handle, helper );

		// Close resource
		dlclose( handle );

		// Cannot find symbol
		if( !srpcfFuncHelper )
			return FALSE;

		// Print out help text
		printf( "%s", srpcfFuncHelper() );
		break;
	}

	return TRUE;
}


// SRPCF Server Implementation
LIBSRPCF_SERVER_IMPLEMENT( xrHelp ) {

	// Nothing to do at server side
	*errorCode = SRPCF_SUCCESSFUL;
	return NULL;
}


