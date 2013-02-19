/*
 * SRPCF - Simple Remote Procedire Command Framework
 * File: frame.c
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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>

#include "srpcf_types.h"
#include "srpcf.h"
#include "srpcf_err.h"
#include "libsrpcf.h"
#include "srpcfsvr.h"
#include "srpcfsh.h"
#include "netsock.h"


bool transferSrpcfFrame( s32 *pMxqFd, const void *pktBuf, const s32 length ) {

	s32 wByte;

	// Transfer data from client to server
	transferSocket( *pMxqFd, pktBuf, length, &wByte );
	if( wByte < 0 ) {
    
        DBGPRINT( "Cannot send out the packet\n" );
        return FALSE;
    }

    return TRUE;
}


void *receiveSrpcfFrame( s32 *pMxqFd ) {

	void *packet;
    s8 pBuf[ LIBSRPCF_MSG_SIZE ];
    srpcfSvrCommHdr_t *pSrpcfSvrCommHdr = (srpcfSvrCommHdr_t *)pBuf;
	s32 rByte;

	// Receive data from client
	receiveSocket( *pMxqFd, pBuf, LIBSRPCF_MSG_SIZE, &rByte );
	if( rByte <= 0 ) {

        DBGPRINT( "Cannot receive a packet\n" );
        return NULL;
    }

    if( pSrpcfSvrCommHdr->srpcfPktLen < sizeof( srpcfSvrCommHdr_t ) ) {

        DBGPRINT( "Invalid packet content\n" );
        return NULL;
    }

    // Allocate memory for receiving a packet
    packet = malloc( pSrpcfSvrCommHdr->srpcfPktLen );
    if( !packet ) {

        DBGPRINT( "Out of memory\n" );
        return NULL;
    }

    // Copy the header
    memcpy( packet, pBuf, pSrpcfSvrCommHdr->srpcfPktLen );

    // Return the pointer of a packet
    return packet;
}


