/*
 * SRPCF - Simple Remote Procedire Command Framework
 * File: packet.c
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

#include "srpcf_types.h"
#include "srpcf.h"
#include "srpcf_err.h"
#include "libsrpcf.h"
#include "srpcfsvr.h"
#include "srpcfsh.h"


bool sendSrpcfPacket( s32 *pMxqFd, srpcfSvrCommPkt_t *pSrpcfSvrCommPkt ) {

    return transferSrpcfFrame( pMxqFd, pSrpcfSvrCommPkt, pSrpcfSvrCommPkt->srpcfSvrCommHdr.srpcfPktLen );
}


srpcfSvrCommPkt_t *recvSrpcfPacket( s32 *pMxqFd ) {

    return (srpcfSvrCommPkt_t *)receiveSrpcfFrame( pMxqFd );
}


static void delayForMoment( void ) {

	usleep( LIBSRPCF_MSG_DELAY );
}


srpcfSvrRspPkt_t *requestSrpcfSupport( s32 *pMsqFd, s32 *pMcqFd ) {

    srpcfSvrReqPkt_t srpcfSvrReqPkt;
    srpcfSvrRspPkt_t *pSrpcfSvrRspPkt;

    // Assemble packets
    srpcfSvrReqPkt.srpcfSvrCommHdr.srpcfOpCode = SRPCF_REQ_QUERY_SUPPORT;
    srpcfSvrReqPkt.srpcfSvrCommHdr.srpcfPktLen = sizeof( srpcfSvrRspPkt_t );

    // Send the request
    if( sendSrpcfPacket( pMsqFd, (srpcfSvrCommPkt_t *)&srpcfSvrReqPkt ) == FALSE ) {

        goto ErrExit;
    }

    // Delay for a moment
	delayForMoment();

    // Receive the response
    pSrpcfSvrRspPkt = (srpcfSvrRspPkt_t *)recvSrpcfPacket( pMcqFd );
    if( !pSrpcfSvrRspPkt
        || pSrpcfSvrRspPkt->srpcfSvrCommHdr.srpcfPktLen < sizeof( srpcfSvrRspPkt_t ) ) {

        goto ErrExit;
    }

    return pSrpcfSvrRspPkt;

ErrExit:

    return NULL;
}


bool responseSrpcfSupport( s32 *pMxqFd, srpcfSupported_t *pSrpcfSupported ) {

    bool ret = TRUE;
    s32 pktSize, srpcfSize, numSrpcfs, sum;
	s8 *pBuf[ LIBSRPCF_MSG_SIZE ];
    srpcfSvrSupportedSrpcf_t *pSrpcfSvrSupportedSrpcf
		= (srpcfSvrSupportedSrpcf_t *)pBuf;

	srpcfSupportedNum_t *pSrpcfSupportedNum
		= (srpcfSupportedNum_t *)&pSrpcfSvrSupportedSrpcf->listOfSupportedSrpcfs;

    // Collect information
    numSrpcfs = countSupportedSRPCFs( pSrpcfSupported );
    srpcfSize = sizeof( srpcfSupportedNum_t ) * numSrpcfs;
    pktSize = sizeof( srpcfSvrSupportedSrpcf_t ) + srpcfSize - sizeof( srpcfSupportedNum_t * );

    // Allocate a packet
	memset( pBuf, 0, LIBSRPCF_MSG_SIZE );

    // Fill in data
    pSrpcfSvrSupportedSrpcf->srpcfSvrCommHdr.srpcfOpCode = SRPCF_RSP_QUERY_SUPPORT;
    pSrpcfSvrSupportedSrpcf->srpcfSvrCommHdr.srpcfPktLen = pktSize;
    pSrpcfSvrSupportedSrpcf->numOfSupportedSrpcfs = numSrpcfs;
	for( sum = 0 ; (pSrpcfSupported + sum)->srpcfCmdNo != XR_END_SRPCF ; sum++ ) {

		(pSrpcfSupportedNum + sum)->srpcfCmdNo = (pSrpcfSupported + sum )->srpcfCmdNo;
	}

    // Send out the packet
    if( !sendSrpcfPacket( pMxqFd, (srpcfSvrCommPkt_t *)pSrpcfSvrSupportedSrpcf ) ) {

        ret = FALSE;
    }

    return ret;
}


u32 serializeCmdOptObject( cmdOpt_t *pCmdOpt, cmdOpt_t *pCmdOptPkt ) {

	u32 sz = 0;
	s32 len;

	for( ; pCmdOpt ; pCmdOpt = pCmdOpt->next ) {

		// Add (string length) + (dataLength field)
		pCmdOptPkt->dataLength = strlen( pCmdOpt->value ) + 1;
		sz += len = pCmdOptPkt->dataLength + sizeof( pCmdOptPkt->dataLength );

		// Copy data
		memcpy( &(pCmdOptPkt->dataPtr), pCmdOpt->value, pCmdOptPkt->dataLength );

		// Move to next offset
		pCmdOptPkt = (cmdOpt_t *)(((s8 *)pCmdOptPkt) + len);
	}

	return sz;
}


bool deserializeCmdOptObject( cmdOpt_t *pCmdOptPkt, u32 numOfCmdOpt ) {

	s32 i;

	for( i = 0 ; i < numOfCmdOpt ; i++ ) {

		if( (i + 1) < numOfCmdOpt )
			pCmdOptPkt->next = 
				(cmdOpt_t *)(((s8 *)pCmdOptPkt) 
				+ pCmdOptPkt->dataLength 
				+ sizeof( pCmdOptPkt->dataLength ));
		else
			pCmdOptPkt->next = NULL;

		pCmdOptPkt = pCmdOptPkt->next;
	}

	return TRUE;
}


srpcfSvrRspExecute_t *requestSrpcfExecute( s32 *pMsqFd, s32 *pMcqFd, u32 srpcfCmdNo, cmdOpt_t *pCmdOpt ) {

	s8 *pBuf[ LIBSRPCF_MSG_SIZE ];
	srpcfSvrReqExecute_t *pSrpcfSvrReqExecute = (srpcfSvrReqExecute_t *)pBuf;
	srpcfSvrRspExecute_t *pSrpcfSvrRspExecute;
	cmdOpt_t *pCmdOptPkt = (cmdOpt_t *)&pSrpcfSvrReqExecute->listOfCmdOpt;

	// Collect information
	memset( pBuf, 0, LIBSRPCF_MSG_SIZE );

    // Assemble packets
    pSrpcfSvrReqExecute->srpcfSvrCommHdr.srpcfOpCode = SRPCF_REQ_EXECUTE;
    pSrpcfSvrReqExecute->srpcfSvrCommHdr.srpcfPktLen =
		sizeof( srpcfSvrReqExecute_t ) + serializeCmdOptObject( pCmdOpt, pCmdOptPkt ) - sizeof( cmdOpt_t * );
	pSrpcfSvrReqExecute->srpcfCmdNo = srpcfCmdNo;
	pSrpcfSvrReqExecute->numOfCmdOptList = countLinklist( (commonLinklist_t *)pCmdOpt );
	if( !pSrpcfSvrReqExecute->numOfCmdOptList ) {

		pSrpcfSvrReqExecute->srpcfSvrCommHdr.srpcfPktLen += sizeof( cmdOpt_t * );
	}

	// Free the CmdOpt linklist here, there has been a serialized copy.
	freeLinklist( (commonLinklist_t *)pCmdOpt );

    // Send the request
    if( sendSrpcfPacket( pMsqFd, (srpcfSvrCommPkt_t *)pSrpcfSvrReqExecute ) == FALSE ) {

        goto ErrExit;
    }

    // Delay for a moment
    delayForMoment();

    // Receive the response
    pSrpcfSvrRspExecute = (srpcfSvrRspExecute_t *)recvSrpcfPacket( pMcqFd );
    if( !pSrpcfSvrRspExecute
        || pSrpcfSvrRspExecute->srpcfSvrCommHdr.srpcfPktLen < (sizeof( srpcfSvrCommHdr_t ) + sizeof( u32 ) * 2) ) {

        goto ErrExit;
    }

    return pSrpcfSvrRspExecute;

ErrExit:

    return NULL;
}


srpcfSvrRspExecute_t *requestSrpcfExecutePlugin( s32 *pMsqFd, s32 *pMcqFd, u32 srpcfCmdNo, cmdOpt_t *pCmdOpt, s8 *srpcfName ) {

	s8 *pBuf[ LIBSRPCF_MSG_SIZE ];
	srpcfSvrReqExecutePlugin_t *pSrpcfSvrReqExecutePlugin = (srpcfSvrReqExecutePlugin_t *)pBuf;
	srpcfSvrRspExecute_t *pSrpcfSvrRspExecute;
	cmdOpt_t *pCmdOptPkt = (cmdOpt_t *)&pSrpcfSvrReqExecutePlugin->listOfCmdOpt;

	// Collect information
	memset( pBuf, 0, LIBSRPCF_MSG_SIZE );

    // Assemble packets
    pSrpcfSvrReqExecutePlugin->srpcfSvrCommHdr.srpcfOpCode = SRPCF_REQ_EXECUTE_PLUGIN;
    pSrpcfSvrReqExecutePlugin->srpcfSvrCommHdr.srpcfPktLen =
		sizeof( srpcfSvrReqExecutePlugin_t ) + serializeCmdOptObject( pCmdOpt, pCmdOptPkt ) - sizeof( cmdOpt_t * );
	pSrpcfSvrReqExecutePlugin->srpcfCmdNo = srpcfCmdNo;
	pSrpcfSvrReqExecutePlugin->numOfCmdOptList = countLinklist( (commonLinklist_t *)pCmdOpt );

	strncpy( pSrpcfSvrReqExecutePlugin->srpcfName, srpcfName, SRPCF_FUNC_MAXLEN );

	if( !pSrpcfSvrReqExecutePlugin->numOfCmdOptList ) {

		pSrpcfSvrReqExecutePlugin->srpcfSvrCommHdr.srpcfPktLen += sizeof( cmdOpt_t * );
	}

	// Free the CmdOpt linklist here, there has been a serialized copy.
	freeLinklist( (commonLinklist_t *)pCmdOpt );

    // Send the request
    if( sendSrpcfPacket( pMsqFd, (srpcfSvrCommPkt_t *)pSrpcfSvrReqExecutePlugin ) == FALSE ) {

        goto ErrExit;
    }

    // Delay for a moment
    delayForMoment();

    // Receive the response
    pSrpcfSvrRspExecute = (srpcfSvrRspExecute_t *)recvSrpcfPacket( pMcqFd );
    if( !pSrpcfSvrRspExecute
        || pSrpcfSvrRspExecute->srpcfSvrCommHdr.srpcfPktLen < (sizeof( srpcfSvrCommHdr_t ) + sizeof( u32 ) * 2) ) {

        goto ErrExit;
    }

    return pSrpcfSvrRspExecute;

ErrExit:

    return NULL;
}


bool responseSrpcfExecute( s32 *pMxqFd, u32 srpcfCmdNo, u32 errorCode, s8 *dataRst ) {

    bool ret = TRUE;
    s32 pktSize, strLen = 0;
    s8 *pBuf[ LIBSRPCF_MSG_SIZE ];
	srpcfSvrRspExecute_t *pSrpcfSvrRspExecute = (srpcfSvrRspExecute_t *)pBuf;

    // Collect information
	if( dataRst )
		strLen = strlen( dataRst ) + 1;

    pktSize = sizeof( srpcfSvrRspExecute_t ) 
				- sizeof( pSrpcfSvrRspExecute->dataPtr ) + strLen;
	if( pktSize > LIBSRPCF_MSG_SIZE )
		return FALSE;

    // Allocate a packet
    memset( pBuf, 0, LIBSRPCF_MSG_SIZE );

    // Fill in data
    pSrpcfSvrRspExecute->srpcfSvrCommHdr.srpcfOpCode = SRPCF_RSP_EXECUTE;
    pSrpcfSvrRspExecute->srpcfSvrCommHdr.srpcfPktLen = pktSize;
    pSrpcfSvrRspExecute->srpcfErrorCode = errorCode;
	pSrpcfSvrRspExecute->dataLength = strLen;

	if( dataRst )
		memcpy( &pSrpcfSvrRspExecute->dataPtr, dataRst, strLen );

    // Send out the packet
    if( !sendSrpcfPacket( pMxqFd, (srpcfSvrCommPkt_t *)pSrpcfSvrRspExecute ) )
        ret = FALSE;

    return ret;
}


