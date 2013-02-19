/*
 * SRPCF - Simple Remote Procedire Command Framework
 * File: srpcfsvr.c
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
#include <signal.h>
#include <pthread.h>
#include <dlfcn.h>

#include "srpcf_types.h"
#include "srpcf.h"
#include "srpcf_err.h"
#include "libsrpcf.h"
#include "srpcf_support.h"
#include "srpcfsvr.h"
#include "netsock.h"


//
// Global variables
//
static srpcfSvrThd_t *srpcfSvrThdHead = NULL;
static pthread_mutex_t threadLock = PTHREAD_MUTEX_INITIALIZER;
static volatile s8 terminate = 0;


static void usage( void ) {

    fprintf( stderr, "\n""\n" );
    fprintf( stderr, "Simple Remote Procedure Command Framework Server\n\n" );
    fprintf( stderr, "Usage: srpcfsvr [-h]\n" );
    fprintf( stderr, "\t-h\tprint this message.\n");
    fprintf( stderr, "\n");
}


static bool executeSrpcfPluginFunction( s32 *pMxqFd, srpcfSvrReqExecutePlugin_t *pSrpcfSvrReqExecutePlugin ) {

	s32 ret;
	u32 errorCode;
    void *handle;
	s8 *rstData;
    s8 execute[ SRPCF_FUNC_MAXLEN ];
	s8 *(*pSrpcfFuncExecutor)(cmdOpt_t*, u32, u32*);
	s8 path[ LIBSRPCF_MAX_PATH ];
	struct stat srpcfStat;

    // Get fullpath
    snprintf( path, 
		LIBSRPCF_MAX_PATH, 
		LIBSRPCF_PLUGIN_PATH "/%s" LIBSRPCF_PLUGIN_SUFFIX,
		pSrpcfSvrReqExecutePlugin->srpcfName );

    // Check for exist
    ret = stat( path, &srpcfStat );
    if( ret < 0 )
        return XR_END_SRPCF;

    // Open instance itself
    handle = dlopen( path, RTLD_LAZY );
    if( !handle ) {

		fprintf( stderr, "Internal error: cannot open executing instance\n" );
        goto ErrExit;
    }

    // Lookup Symbols
	snprintf( execute, SRPCF_FUNC_MAXLEN, SRPCF_EXECUTOR_PREFIX "%s", pSrpcfSvrReqExecutePlugin->srpcfName );
    pSrpcfFuncExecutor = dlsym( handle, execute );
    if( !pSrpcfFuncExecutor ) {

		fprintf( stderr, "Internal error: cannot find the symbol of SRPCF executor\n" );
		goto ErrExit;
    }

	// Deserialize CmdOpt object
	if( pSrpcfSvrReqExecutePlugin->numOfCmdOptList ) {

		ret = deserializeCmdOptObject(
				(cmdOpt_t *)&pSrpcfSvrReqExecutePlugin->listOfCmdOpt, 
				pSrpcfSvrReqExecutePlugin->numOfCmdOptList );
		if( ret == FALSE ) {

			fprintf( stderr, "Internal error: cannot convert serialize object to linklist\n" );
			goto ErrExit1;
		}
	}

	// Execute SRPCF function
	rstData = pSrpcfFuncExecutor( 
			(cmdOpt_t *)&pSrpcfSvrReqExecutePlugin->listOfCmdOpt, 
			pSrpcfSvrReqExecutePlugin->numOfCmdOptList, 
			&errorCode );

	// Response for this SRPCF command
    ret = responseSrpcfExecute( pMxqFd, pSrpcfSvrReqExecutePlugin->srpcfCmdNo, errorCode, rstData );

	// Free resource
	if( rstData )
		free( rstData );

ErrExit1:

    // Release resources
    dlclose( handle );
	return ret;

ErrExit:

    return FALSE;
}


static bool executeSrpcfFunction( s32 *pMxqFd, srpcfSvrReqExecute_t *pSrpcfSvrReqExecute ) {

	bool found = FALSE, ret;
    s32 i;
	u32 errorCode;
    void *handle;
	s8 *rstData;
    s8 execute[ SRPCF_FUNC_MAXLEN ];
	s8 *(*pSrpcfFuncExecutor)(cmdOpt_t*, u32, u32*);

	for( i = 0 ; srpcfSupportedTbl[ i ].srpcfCmdNo != XR_END_SRPCF ; i++ ) {

		if( srpcfSupportedTbl[ i ].srpcfCmdNo == pSrpcfSvrReqExecute->srpcfCmdNo ) {

			// Obtain function name
			snprintf( execute, SRPCF_FUNC_MAXLEN, SRPCF_EXECUTOR_PREFIX "%s", srpcfSupportedTbl[ i ].srpcfFuncName );
			found = TRUE;
			break;
		}
	}
	if( found == FALSE ) {

		fprintf( stderr, "Internal error: cannot find corresponding SRPCF function\n" );
		return FALSE;
	}

    // Open instance itself
    handle = dlopen( NULL, RTLD_LAZY );
    if( !handle ) {

		fprintf( stderr, "Internal error: cannot open executing instance\n" );
        goto ErrExit;
    }

    // Lookup Symbols
    pSrpcfFuncExecutor = dlsym( handle, execute );
    if( !pSrpcfFuncExecutor ) {

		fprintf( stderr, "Internal error: cannot find the symbol of SRPCF executor\n" );
		goto ErrExit;
    }

	// Deserialize CmdOpt object
	if( pSrpcfSvrReqExecute->numOfCmdOptList ) {

		ret = deserializeCmdOptObject(
				(cmdOpt_t *)&pSrpcfSvrReqExecute->listOfCmdOpt, 
				pSrpcfSvrReqExecute->numOfCmdOptList );
		if( ret == FALSE ) {

			fprintf( stderr, "Internal error: cannot convert serialize object to linklist\n" );
			goto ErrExit1;
		}
	}

	// Execute SRPCF function
	rstData = pSrpcfFuncExecutor( (cmdOpt_t *)&pSrpcfSvrReqExecute->listOfCmdOpt, pSrpcfSvrReqExecute->numOfCmdOptList, &errorCode );

	// Response for this SRPCF command
    ret = responseSrpcfExecute( pMxqFd, pSrpcfSvrReqExecute->srpcfCmdNo, errorCode, rstData );

	// Free resource
	if( rstData )
		free( rstData );

ErrExit1:

    // Release resources
    dlclose( handle );
	return ret;

ErrExit:

    return FALSE;
}


static void *handleIncomingConnection( void *arg ) {

    srpcfSvrThd_t *pSrpcfSvrThd = (srpcfSvrThd_t *)arg;
	srpcfSvrTask_t *pSrpcfSvrTask;
	s8 term = 0;

    // Sanity check
    if( !pSrpcfSvrThd )
        pthread_exit( 0 );
 
	// Allocate a empty task
    pSrpcfSvrTask = (srpcfSvrTask_t *)malloc( sizeof( srpcfSvrTask_t ) );
    if( !pSrpcfSvrTask )
		pthread_exit( 0 );

    // Main thread loop
    while( !terminate || !term ) {

		// Receive a packet
        pSrpcfSvrTask->pktData = receiveSrpcfFrame( &pSrpcfSvrThd->cfd );
        if( !pSrpcfSvrTask->pktData )
			break;

		// Handle request
		switch( pSrpcfSvrTask->pktData->srpcfSvrReqPkt.srpcfSvrCommHdr.srpcfOpCode ) {

        // SRPCF Support Query
        case SRPCF_REQ_QUERY_SUPPORT:
			responseSrpcfSupport( &pSrpcfSvrThd->cfd, srpcfSupportedTbl );
			break;

        // SRPCF Execute
        case SRPCF_REQ_EXECUTE:
			executeSrpcfFunction( &pSrpcfSvrThd->cfd, &pSrpcfSvrTask->pktData->srpcfSvrReqExecute );
			term = 1;
			break;

        // SRPCF Execute Plugin
        case SRPCF_REQ_EXECUTE_PLUGIN:
            executeSrpcfPluginFunction( &pSrpcfSvrThd->cfd, &pSrpcfSvrTask->pktData->srpcfSvrReqExecutePlugin );
			term = 1;
			break;

        // Unknown
        default:
            DBGPRINT( "Unknown Operation Code %d\n",
                pSrpcfSvrTask->pktData->srpcfSvrReqPkt.srpcfSvrCommHdr.srpcfOpCode );
			term = 1;
            break;
    	}

		// Free packet buffer
		free( pSrpcfSvrTask->pktData );

		// Delay for a while
        usleep( SRPCFSVR_SLEEP_MS );
	}

    // Close this connection
    deinitializeSocket( pSrpcfSvrThd->cfd );

	// Free resource
	free( pSrpcfSvrTask );

    // Detach my context
    pthread_mutex_lock( &threadLock );
    removeLinklist( (commonLinklist_t **)&srpcfSvrThdHead, (commonLinklist_t *)pSrpcfSvrThd );
    pthread_mutex_unlock( &threadLock );

    // Free memory
    free( pSrpcfSvrThd );

    // Return
    pthread_exit( 0 );
}


s32 main( s32 argc, s8 **argv ) {

    s8 c;
    pid_t pid, sid;
	s32 daemon = 1;
	s32 sfd, cfd, ret;
	srpcfSvrThd_t *pSrpcfSvrThd;

	// Parse options
    while( (c = getopt( argc, argv, "ch" )) != EOF ) {

        switch( c ) {

			case 'c' :
				daemon = 0;
				break;

            case 'h' :
            default:
                usage();
                return 1;
        }
    }

	if( !daemon ) {

		goto NoDaemon;
	}

    // Initialize daemon things
	// Fork a child process
    pid = fork();
    if( pid < 0 ) {
    
		// Error here
        fprintf( stderr, "srpcfsvr: cannot fork a child process.\n" );
        exit( 1 );
    }

    if( pid > 0 ) {
    
		// Exit parent process
        exit( 0 );
    }

	// Adjust UMASK
    umask( 0 );

	// check for SID
    sid = setsid();
    if( sid < 0 ) {

        exit( 1 );
    }

	// Change directory to /
    if( chdir( "/" ) < 0 ) {
    
        exit( 1 );
    }

	// Close standard in/out/error
    close( 0 );
    close( 1 );
    close( 2 );

NoDaemon:

	// Open a socket
    if( initializeSocket( &sfd, NULL, SRPCF_DEF_PORT ) ) {

        DBGPRINT( "Cannot initialize socket\n" );
		exit( -1 );
    }

	// Handle incoming connections
	while( !terminate ) {

		// Accept new connection
		if( acceptSocket( sfd, &cfd ) != TRUE )
			continue;

		// Allocate a new thread context
		pSrpcfSvrThd =(srpcfSvrThd_t *)malloc( sizeof( srpcfSvrThd_t ) );
        if( !pSrpcfSvrThd ) {

            DBGPRINT( "Out of memory\n" );
            deinitializeSocket( cfd );
            usleep( SRPCFSVR_SLEEP_MS );
            continue;
        }

		// Attach the thread context
		pthread_mutex_lock( &threadLock );
		appendLinklist( (commonLinklist_t **)&srpcfSvrThdHead, (commonLinklist_t *)pSrpcfSvrThd );
        pthread_mutex_unlock( &threadLock );

		// Fill in the data
        pSrpcfSvrThd->cfd = cfd;

        // Create a thread
        ret = pthread_create(
            &pSrpcfSvrThd->pth,
            NULL,
            handleIncomingConnection,
            (void *)pSrpcfSvrThd );
        if( ret ) {

            fprintf( stderr, "Failed to create a thread\n" );
            break;
        }

		// Delay for awhile
		usleep( SRPCFSVR_SLEEP_MS );
	}

	// Cancel all running threads, but don't free, the OS will do
	for( pSrpcfSvrThd = srpcfSvrThdHead ; pSrpcfSvrThd ; pSrpcfSvrThd = pSrpcfSvrThd->next )
		pthread_cancel( pSrpcfSvrThd->pth );

	// Close the socket
	deinitializeSocket( sfd );

	// Return
    return 0;
}


