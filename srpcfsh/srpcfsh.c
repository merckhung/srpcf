/*
 * SRPCF - Simple Remote Procedire Command Framework
 * File: srpcfsh.c
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
#include <signal.h>
#include <termios.h>

#include "srpcf_types.h"
#include "srpcf.h"
#include "srpcf_err.h"
#include "libsrpcf.h"
#include "srpcf_support.h"
#include "srpcfsvr.h"
#include "srpcfsh.h"
#include "netsock.h"


//
// Global variables
//
static cmdOpt_t *cmdOptHead = NULL;
static u32 numOfSrpcfParams = 0;

#ifdef SRPCF_COMMAND_LINE
static struct termios origTermSet, srpcfTermSet;
#endif


static void showSrpcfCmds( void ) {

	s32 i;

	printf( "Supported SRPCF Commands:\n" );
	for( i = 0 ; srpcfSupportedTbl[ i ].srpcfCmdNo != XR_END_SRPCF ; i++ ) {

		if( srpcfSupportedTbl[ i ].enabled == TRUE ) {

			printf( "\t%s\n", srpcfSupportedTbl[ i ].srpcfFuncName );
		}
	};
	printf( "\n" );
}


static void copyright( void ) {

    printf( "\nSimple Remote Procedure Command Framework Shell, Version "SRPCFSH_REVISION"\n" );
    printf( "Copyright (c) 2013 all rights reserved.\n\n" );
}


srpcfSupported_t *retriveSrpcfSupported( void ) {

	return srpcfSupportedTbl;
}


static u32 handleParameters( s32 argc, s8 **argv ) {

    s32 i, idx = 0;
    s8 *p, *q;
    cmdOpt_t **ppCmdOpt = NULL;

	for( i = 1 ; i < argc ; i++ ) {

		// Manipulate each cell of argv
		p = argv[ i ];
		if( countCharacter( p, ',') == strlen( p ) ) {

			// Skip "," cells
			continue;
		}

		// Convert multiple paramaters into linklist
		for( ; ; ) {

			// Check for string length
			if( !strlen( p ) ) {

				break;
			}

			// Look for terminating character
			q = index( p, ',' );
			if( q ) {

				*q = '\0';
				if( q == p ) {

					p++;
					continue;
				}
			}

			// Allocate memory for a srpcfTermSet linklist
			LinkListMalloc( &cmdOptHead, ppCmdOpt, idx, cmdOpt_t );

			// Assign value
			(*ppCmdOpt)->value = p;

			// Move to next parameter
			idx++;
			if( !q ) {

				break;
			}
			p = q + 1;
		}
	}

	return idx;
}


static u32 handleSrpcfFunction( s8 *srpcfCmdStr, srpcfFuncs_t *pSrpcfShell ) {

	u32 cmdNo;
	s32 baseOff;
	void *handle;
	s8 parser[ SRPCF_FUNC_MAXLEN ], helper[ SRPCF_FUNC_MAXLEN ], error[ SRPCF_FUNC_MAXLEN ];

	// Look for basename
	baseOff = findBasename( srpcfCmdStr );

	// Check for support & enable
	cmdNo = checkSrpcfCmdEnabled( srpcfCmdStr + baseOff, srpcfSupportedTbl );
	if( cmdNo == XR_END_SRPCF ) {

		goto ErrExit;
	}

	// Open instance itself
	handle = dlopen( NULL, RTLD_LAZY );
	if( !handle ) {

		goto ErrExit;
	}

	// Obtain function names
	snprintf( parser, SRPCF_FUNC_MAXLEN, SRPCF_PARSER_PREFIX "%s", srpcfCmdStr + baseOff );
	snprintf( helper, SRPCF_FUNC_MAXLEN, SRPCF_HELPER_PREFIX "%s", srpcfCmdStr + baseOff );
	snprintf( error, SRPCF_FUNC_MAXLEN, SRPCF_ERROR_PREFIX "%s", srpcfCmdStr + baseOff );

	// Lookup Symbols
	pSrpcfShell->srpcfFuncHelper = dlsym( handle, helper );
	pSrpcfShell->srpcfFuncParser = dlsym( handle, parser );
	pSrpcfShell->srpcfFuncError = dlsym( handle, error );
	if( !(pSrpcfShell->srpcfFuncHelper && pSrpcfShell->srpcfFuncParser && pSrpcfShell->srpcfFuncError) ) {

		cmdNo = XR_END_SRPCF;
	}

	// Release resources
	dlclose( handle );
	return cmdNo;

ErrExit:

	return XR_END_SRPCF;
}


static u32 srpcfPluginSearch( const s8 *reqSrpcfName, srpcfFuncs_t *pSrpcfFuncs, void **handle ) {

    s8 path[ LIBSRPCF_MAX_PATH ];
    s8 parser[ SRPCF_FUNC_MAXLEN ], helper[ SRPCF_FUNC_MAXLEN ], error[ SRPCF_FUNC_MAXLEN ];
    s32 ret, baseOff;
    struct stat srpcfStat;

    // Look for basename
    baseOff = findBasename( reqSrpcfName );

    // Get fullpath
    snprintf( path, LIBSRPCF_MAX_PATH, LIBSRPCF_PLUGIN_PATH "/%s" LIBSRPCF_PLUGIN_SUFFIX, reqSrpcfName + baseOff );

    // Check for exist
    ret = stat( path, &srpcfStat );
    if( ret < 0 )
        return XR_END_SRPCF;

    // Open a plugin instance
    *handle = dlopen( path, RTLD_LAZY );
    if( !*handle )
        return XR_END_SRPCF;

    // Obtain function names
    snprintf( parser, SRPCF_FUNC_MAXLEN, SRPCF_PARSER_PREFIX "%s", reqSrpcfName + baseOff );
    snprintf( helper, SRPCF_FUNC_MAXLEN, SRPCF_HELPER_PREFIX "%s", reqSrpcfName + baseOff );
    snprintf( error, SRPCF_FUNC_MAXLEN, SRPCF_ERROR_PREFIX "%s", reqSrpcfName + baseOff );

    // Lookup Symbols
    pSrpcfFuncs->srpcfFuncHelper = dlsym( *handle, helper );
    pSrpcfFuncs->srpcfFuncParser = dlsym( *handle, parser );
    pSrpcfFuncs->srpcfFuncError = dlsym( *handle, error );
    if( !(pSrpcfFuncs->srpcfFuncHelper && pSrpcfFuncs->srpcfFuncParser && pSrpcfFuncs->srpcfFuncError) ) {

        ret = XR_END_SRPCF;
        goto ErrExit;
    }

    ret = XR_START_SRPCF;

ErrExit:
    return ret;
}


static void installSupportedSrpcfs( srpcfSvrSupportedSrpcf_t *pSrpcfSvrSupportedSrpcf ) {

	srpcfSupportedNum_t *pSrpcfSupportedNum;
	s32 i, j;
	bool found;

	//DBGPRINT( "Supported %d SRPCF Commands\n", pSrpcfSvrSupportedSrpcf->numOfSupportedSrpcfs );
	pSrpcfSupportedNum = (srpcfSupportedNum_t *)&(pSrpcfSvrSupportedSrpcf->listOfSupportedSrpcfs);
	for( i = 0 ; 
			i < pSrpcfSvrSupportedSrpcf->numOfSupportedSrpcfs ; 
			i++, pSrpcfSupportedNum++ ) {

		// Search SRPCF table
		for( j = 0, found = FALSE ; srpcfSupportedTbl[ j ].srpcfCmdNo != XR_END_SRPCF ; j++ ) {

			if( srpcfSupportedTbl[ j ].srpcfCmdNo == pSrpcfSupportedNum->srpcfCmdNo ) {

				// Enable this SRPCF commands
				srpcfSupportedTbl[ j ].enabled = TRUE;
				found = TRUE;
				break;
			}
		}

		if( found == FALSE ) {

			DBGPRINT( "Cannot install SRPCF #%d\n", (s32)pSrpcfSupportedNum->srpcfCmdNo );
		}
	}

	// Free memory
	free( pSrpcfSvrSupportedSrpcf );
}


#ifdef SRPCF_COMMAND_LINE
static void autoCompleteCommandLine( s8 *cmdBuf, s32 *index ) {

	s32 i, sum = 0;
	srpcfSupported_t *pSrpcfSupported;
	bool autoComp = FALSE;
	bool nextSame;
	s8 c;

	if( *index < 1 ) {

		return;
	}

	// Print out all similar commands
	write( 1, "\r\n", 2 );

CheckAgain:
	for( i = 0, c = '\0', nextSame = TRUE ; srpcfSupportedTbl[ i ].srpcfCmdNo != XR_END_SRPCF ; i++ ) {

		if( (srpcfSupportedTbl[ i ].enabled == TRUE)
			&& (!strncmp( cmdBuf, srpcfSupportedTbl[ i ].srpcfFuncName, *index )) ) {

			pSrpcfSupported = &srpcfSupportedTbl[ i ];
			sum++;

			if( *(srpcfSupportedTbl[ i ].srpcfFuncName + *index) == '\0' ) {

				nextSame = FALSE;
			}
			else if( (c != '\0') && (c != *(srpcfSupportedTbl[ i ].srpcfFuncName + *index)) ) {
				
				nextSame = FALSE;
			}
			else if( c == '\0' ) {

				c = *(srpcfSupportedTbl[ i ].srpcfFuncName + *index);
				nextSame = TRUE;
			}
			if( autoComp == FALSE ) {

				printf( "%s   ", srpcfSupportedTbl[ i ].srpcfFuncName );
			}
		}
	}

	// Auto complete more one character
	if( ((sum == 1) && (nextSame == TRUE)) && strcmp( pSrpcfSupported->srpcfFuncName, cmdBuf ) ) {

		cmdBuf[ *index ] = *(pSrpcfSupported->srpcfFuncName + (*index));
		cmdBuf[ ++(*index) ] = '\0';
		autoComp = TRUE;
		goto CheckAgain;
	}

	printf( "\r\n%s%s", SRPCFSH_PROMPT, cmdBuf );
}


static void srpcfTerminalSetting( void ) {

	// Change terminal setting
    if( tcgetattr( 0, &origTermSet ) ) {

        return;
    }
    memcpy( &srpcfTermSet, &origTermSet, sizeof( struct termios ) );
	srpcfTermSet.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	srpcfTermSet.c_oflag &= ~OPOST;
	srpcfTermSet.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	srpcfTermSet.c_cflag &= ~(CSIZE | PARENB);
	srpcfTermSet.c_cflag |= CS8;
    if( tcsetattr( 0, TCSAFLUSH, &srpcfTermSet ) ) {

        return;
    }
	setvbuf( stdout, NULL, _IONBF, BUFSIZ );
}


static void srpcfTerminalRecover( void ) {

    // Restore setting
    tcsetattr( 0, TCSAFLUSH, &origTermSet );
}


static void executeCommandLine( const s8 *cmdBuf ) {

	if( !strcmp( "exit", cmdBuf )
		|| !strcmp( "quit", cmdBuf ) ) {

		// Exit SRPCF shell
		srpcfTerminalRecover();
		write( 1, "\n", 1 );
		exit( 0 );
	}
	else if( !strcmp( "help", cmdBuf ) ) {

		// Show SRPCF help
		copyright();
		showSrpcfCmds();
	}
}


static void srpcfCommandLine( void ) {

	s32 i, rbyte;
	s8 cmdBuf[ SRPCFSH_CMDBUF_LEN ], iBuf[ 8 ];
	bool tab;

	// Setup terminal
	srpcfTerminalSetting();

	// Command line
	for( i = 0, tab = FALSE ; ; ) {

		// Print out SRPCF shell prompt
		if( !i && tab == FALSE ) {

			write( 1, SRPCFSH_PROMPT, strlen( SRPCFSH_PROMPT ) );
		}
		if( tab == TRUE )
			tab = FALSE;

		// Read a byte
		memset( iBuf, 0, sizeof( iBuf ) );
		rbyte = read( 0, iBuf, sizeof( iBuf ) );
		if( !rbyte ) {

			fprintf( stderr, "Internal error: cannot read stdin\n" );
			continue;
		}

		// Manipulate this input
		switch( rbyte ) {

		// Simple character
		case 1:
			switch( (CHAR)*iBuf ) {

			case '\t':

				// Do auto completion
				cmdBuf[ i ] = 0;
				autoCompleteCommandLine( cmdBuf, &i );
				tab = TRUE;
				break;

			case '\r':
			case '\n':

				// Execute this command
				cmdBuf[ i ] = 0;
				executeCommandLine( cmdBuf );

				// Clear command buffer
				i = 0;
				write( 1, "\r\n", 2 );
				break;

			default:

				// Prevent from overloading
				if( i < SRPCFSH_CMDBUF_LEN ) {

					cmdBuf[ i++ ] = (CHAR)*iBuf;
					write( 1, &cmdBuf[ i - 1 ], 1 );
				}
				break;
			}
			break;

			case 2:
				break;

			case 3:
				break;

			case 4:
				break;

			case 5:
				break;

			default:
				break;
		}
	}

	// Restore setting
	srpcfTerminalRecover();
}
#endif


s32 main( s32 argc, s8 **argv ) {

	srpcfFuncs_t srpcfFuncs;
	srpcfSvrSupportedSrpcf_t *pSrpcfSvrSupportedSrpcf;
	srpcfSvrRspExecute_t *pSrpcfSvrRspExecute;
	s8 *pHelpStr;
	s32 ret = 0;
	u32 srpcfCmdNo;
	void *handle = NULL;
	s32 cfd;
	s8 ipAddr[] = "127.0.0.1";

	// Check for argc
	if( argc < 1 ) {

		fprintf( stderr, "Internal Error: cannot read input arguments\n" );
		return 1;
	}

	// Open a socket
	if( connectSocket( &cfd, ipAddr, SRPCF_DEF_PORT ) ) {

		fprintf( stderr, "Internal Error: cannot connect to SRPCF server\n" );
		return 1;
	}

	// Query support SRPCF commands
	pSrpcfSvrSupportedSrpcf = (srpcfSvrSupportedSrpcf_t *)requestSrpcfSupport( &cfd, &cfd );

	if( !pSrpcfSvrSupportedSrpcf ) {

		ret = 1;
		fprintf( stderr, "Internal Error: cannot query for supported SRPCFs\n" );
		goto ErrExit1;
	}

	// Install SRPCF commands
	installSupportedSrpcfs( pSrpcfSvrSupportedSrpcf );

	// Look for SRPCF function
	srpcfCmdNo = handleSrpcfFunction( argv[ 0 ], &srpcfFuncs );
	if( srpcfCmdNo == XR_END_SRPCF ) {

		// SRPCF plugin support
		srpcfCmdNo = srpcfPluginSearch( argv[ 0 ], &srpcfFuncs, &handle );
		if( srpcfCmdNo == XR_END_SRPCF ) {

#ifdef SRPCF_COMMAND_LINE
			srpcfCommandLine();
#else
			copyright();
			showSrpcfCmds();
#endif
			goto Exit;
		}
	}

	// Parse command line Input
	numOfSrpcfParams = handleParameters( argc, argv );

	// Execute SRPCF command
	if( srpcfFuncs.srpcfFuncParser( cmdOptHead, numOfSrpcfParams ) ) {

		// Run this SRPCF command on server
		if( srpcfCmdNo == XR_START_SRPCF )
			pSrpcfSvrRspExecute = requestSrpcfExecutePlugin( &cfd, &cfd, srpcfCmdNo, cmdOptHead, argv[ 0 ] + findBasename( argv[ 0 ] ) );
		else
			pSrpcfSvrRspExecute = requestSrpcfExecute( &cfd, &cfd, srpcfCmdNo, cmdOptHead );

		if( !pSrpcfSvrRspExecute ) {

			fprintf( stderr, "Internal Error: cannot execute SRPCF\n" );
			goto ErrExit1;
		}

		// Print out result
		if( (pSrpcfSvrRspExecute->srpcfErrorCode == SRPCF_SUCCESSFUL) 
			&& (pSrpcfSvrRspExecute->dataLength > 0)
			&& pSrpcfSvrRspExecute->dataPtr ) {

			printf( "%s\n", (s8 *)(&pSrpcfSvrRspExecute->dataPtr) );
		}
		else if( pSrpcfSvrRspExecute->srpcfErrorCode == SRPCF_SUCCESSFUL )
			printf( "SUCCESSFUL\n" );
		else
			printf( "ERROR: %d\n", pSrpcfSvrRspExecute->srpcfErrorCode );
	}
	else {

		pHelpStr = srpcfFuncs.srpcfFuncHelper();
		printf( "%s\n", pHelpStr );
	}

ErrExit1:

	// Close the socket
	deinitializeSocket( cfd );

	// Close plugin instance if any
	if( handle )
		dlclose( handle );
	
Exit:
    return ret;
}


