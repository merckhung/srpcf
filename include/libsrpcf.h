/*
 * SRPCF - Simple Remote Procedire Command Framework
 * File: libsrpcf.h
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

//
// Definitions
//
#define LIBSRPCF_MSG_MAX			8
#define LIBSRPCF_MSG_SIZE			4096
#define LIBSRPCF_MSG_MAXSIZE		(LIBSRPCF_MSG_MAX * LIBSRPCF_MSG_SIZE)
#define LIBSRPCF_MSG_PRIO			8

#define LIBSRPCF_MSG_RETRY_TIME		50000
#define LIBSRPCF_MSG_RETRY_MSEC		500
#define LIBSRPCF_MSG_DELAY			500
#define LIBSRPCF_MSG_RETRY_CLEAN		10

#define LIBSRPCF_FILE_PMODE			0640
#define LIBSRPCF_FILE_CMODE			(O_RDWR | O_CREAT)
#define LIBSRPCF_FILE_OMODE			(O_RDWR)

#define LIBSRPCF_REVISION			SRPCF_CODE_REVISION
#define LIBSRPCF_MAX_PATH			256
#define LIBSRPCF_MAX_WRITE_PACKAGE	256
#define LIBSRPCF_PLUGIN_PATH		"plugins"
#define LIBSRPCF_PLUGIN_SUFFIX		".srpcf"
#define LIBSRPCF_BPL				16

#define LIBSRPCF_MAC_STR_LEN		12
#define LIBSRPCF_MAC_STR_BUF		(LIBSRPCF_MAC_STR_LEN + 1)

#define LIBSRPCF_WWID_STR_LEN		16
#define LIBSRPCF_WWID_STR_BUF		(LIBSRPCF_WWID_STR_LEN + 1)

#define LIBSRPCF_IP_STR_LEN_MIN		7
#define LIBSRPCF_IP_STR_LEN			15
#define LIBSRPCF_IP_STR_BUF			(LIBSRPCF_IP_STR_LEN + 1)

#define LIBSRPCF_DATE_STR_LEN		11
#define LIBSRPCF_DATE_STR_BUF		(LIBSRPCF_DATE_STR_LEN + 1)
#define LIBSRPCF_TIME_STR_LEN		11
#define LIBSRPCF_TIME_STR_BUF		(LIBSRPCF_TIME_STR_LEN + 1)

#define LIBSRPCF_SPDSIGN_STR_OFF	0xB0
#define LIBSRPCF_SPDSIGN_STR_LEN	16
#define LIBSRPCF_SPDSIGN_STR_BUF	(LIBSRPCF_SPDSIGN_STR_LEN + 1)

#define LIBSRPCF_SPDSIGN_DATE_OFF	0xC0
#define LIBSRPCF_SPDSIGN_DATE_LEN	16
#define LIBSRPCF_SPDSIGN_DATE_BUF	(LIBSRPCF_SPDSIGN_DATE_LEN + 1)

#define LIBSRPCF_TESTSVR_PORT		7500
#define LIBSRPCF_TESTBUF_SZ			1024
#define LIBSRPCF_TESTEND_STR		"TESTCOMPLETED"

#define LIBSRPCF_SEEPROM_BUF		256
#define LIBSRPCF_SEEPROM_BYTES_ONCE	16


//
// Macros
//
#define LIBSRPCF_HELPER_TEXT( DESC )			static s8 *__tmp_help_text = DESC;


#define LIBSRPCF_ERROR_FUNC( NAME ) \
    void srpcfError_##NAME( s32 errorCode )
#define LIBSRPCF_ERROR_SWITCH					switch( errorCode )


#define LIBSRPCF_SHELL_IMPLEMENT( NAME ) \
    s8 *srpcfHelper_##NAME( void ) { return __tmp_help_text; } \
    bool srpcfParser_##NAME( cmdOpt_t *pCmdOpt, u32 numOpts )

#define LIBSRPCF_SERVER_IMPLEMENT( NAME ) \
    s8 *srpcfExecutor_##NAME( cmdOpt_t *pCmdOpt, u32 numOpts, u32 *errorCode )

#define LIBSRPCF_SRPCF_FOREACH \
    cmdOpt_t *ppCmdOpt; \
    	ForeachLinkList( pCmdOpt, ppCmdOpt )


//
// Enumernations
//
typedef enum _srpcfReqOpCode {

    SRPCF_REQ_QUERY_SUPPORT = 1,
	SRPCF_REQ_EXECUTE,
	SRPCF_REQ_EXECUTE_PLUGIN,

} srpcfReqOpCode_t;


typedef enum _srpcfRspOpCode {

    SRPCF_RSP_QUERY_SUPPORT = 1,
	SRPCF_RSP_EXECUTE,
	SRPCF_RSP_EXECUTE_PLUGIN,

} srpcfRspOpCode_t;


//
// Structures
//
typedef struct _srpcfSupported {

    u32			srpcfCmdNo;
    bool		enabled;
    s8			*srpcfFuncName;

} srpcfSupported_t;


typedef struct PACKED _srpcfSupportedNum {

    u32			srpcfCmdNo;

} srpcfSupportedNum_t;


typedef struct PACKED _cmdOpt {

    union {

        struct _cmdOpt  *next;
        u64				dataLength;
    };

    union {

        s8            	*value;
        s8            	*dataPtr;
    };

} cmdOpt_t;


typedef struct PACKED _srpcfSvrCommHdr {

    srpcfReqOpCode_t	srpcfOpCode;
    u32               	srpcfPktLen;

} srpcfSvrCommHdr_t;


typedef struct PACKED _srpcfSvrReqPkt {

	srpcfSvrCommHdr_t	srpcfSvrCommHdr;

} srpcfSvrReqPkt_t;


typedef struct PACKED _srpcfSvrRspPkt {

	srpcfSvrCommHdr_t	srpcfSvrCommHdr;

} srpcfSvrRspPkt_t;


typedef struct PACKED _srpcfSvrSupportedSrpcf {

    srpcfSvrCommHdr_t	srpcfSvrCommHdr;
    u32               	numOfSupportedSrpcfs;
    srpcfSupportedNum_t	*listOfSupportedSrpcfs;

} srpcfSvrSupportedSrpcf_t;


typedef struct PACKED _srpcfSvrReqExecute {

	srpcfSvrCommHdr_t	srpcfSvrCommHdr;
	u32					srpcfCmdNo;
	u32					numOfCmdOptList;
	cmdOpt_t			*listOfCmdOpt;

} srpcfSvrReqExecute_t;


typedef struct PACKED _srpcfSvrRspExecute {

	srpcfSvrCommHdr_t	srpcfSvrCommHdr;
	u32					srpcfErrorCode;
	u32					dataLength;
	s8					*dataPtr;

} srpcfSvrRspExecute_t;


typedef struct PACKED _srpcfSvrReqExecutePlugin {

	srpcfSvrCommHdr_t	srpcfSvrCommHdr;
	u32					srpcfCmdNo;
	s8					srpcfName[ SRPCF_FUNC_MAXLEN ];
	u32					numOfCmdOptList;
	cmdOpt_t			*listOfCmdOpt;

} srpcfSvrReqExecutePlugin_t;


typedef struct PACKED _srpcfSvrCommPkt {

    union {

		srpcfSvrCommHdr_t			srpcfSvrCommHdr;
        srpcfSvrReqPkt_t     		srpcfSvrReqPkt;
        srpcfSvrRspPkt_t     		srpcfSvrRspPkt;
		srpcfSvrReqExecute_t		srpcfSvrReqExecute;
		srpcfSvrReqExecutePlugin_t	srpcfSvrReqExecutePlugin;
		srpcfSvrRspExecute_t		srpcfSvrRspExecute;
    };

} srpcfSvrCommPkt_t;


//
// Prototypes
//
bool transferSrpcfFrame( s32 *pMxqFd, const void *pktBuf, const s32 length );
void *receiveSrpcfFrame( s32 *pMxqFd );

s32 findBasename( const s8 *str );
s32 countCharacter( const s8 *str, const s8 c );
s32 countLinklist( commonLinklist_t *head );
commonLinklist_t **tailOfLinklist( commonLinklist_t **head );
void appendLinklist( commonLinklist_t **head, commonLinklist_t *object );
commonLinklist_t *retriveFirstLinklist( commonLinklist_t **head );
commonLinklist_t *removeLinklist( commonLinklist_t **head, commonLinklist_t *tgt );
void freeLinklist( commonLinklist_t *head );

bool sendSrpcfPacket( s32 *pMxqFd, srpcfSvrCommPkt_t *pSrpcfSvrCommPkt );
srpcfSvrCommPkt_t *recvSrpcfPacket( s32 *pMxqFd );
srpcfSvrRspPkt_t *requestSrpcfSupport( s32 *pMsqFd, s32 *pMcqFd );
bool responseSrpcfSupport( s32 *pMxqFd, srpcfSupported_t *pSrpcfSupported );
u32 serializeCmdOptObject( cmdOpt_t *pCmdOpt, cmdOpt_t *pCmdOptPkt );
bool deserializeCmdOptObject( cmdOpt_t *pCmdOptPkt, u32 numOfCmdOpt );
srpcfSvrRspExecute_t *requestSrpcfExecute( s32 *pMsqFd, s32 *pMcqFd, u32 srpcfCmdNo, cmdOpt_t *pCmdOpt );
srpcfSvrRspExecute_t *requestSrpcfExecutePlugin( s32 *pMsqFd, s32 *pMcqFd, u32 srpcfCmdNo, cmdOpt_t *pCmdOpt, s8 *srpcfName );
bool responseSrpcfExecute( s32 *pMxqFd, u32 srpcfCmdNo, u32 errorCode, s8 *dataRst );

u32 countSupportedSRPCFs( const srpcfSupported_t *pSrpcfSupported );
u32 checkSrpcfCmdEnabled( const s8 *srpcfStr, const srpcfSupported_t *pSrpcfSupported_t );
bool checkSrpcfCmdParam( const s8 *param, s8 **compare, s32 size );
s8 *readFileToNewBuffer( const s8 *basePath, const s8 *restPath );
bool writeFileWithText( const s8 *basePath, const s8 *restPath, const s8 *text );
bool fetchLocation( const s8 *basePath, const s8 *restPath, s8 *buf, s32 len );
s8 *readRedirectFileToNewBuffer( const s8 *basePath, const s8 *restPath );
bool writeRedirectFileWithText( const s8 *basePath, const s8 *restPath, const s8 *text );
bool writeEitherWayFileWithInteger( const s8 *basePath, const s8 *restPath, const s32 value, const bool direct, const bool hex );
s8 *readFileToNewHugeBuffer( const s8 *basePath, const s8 *restPath, u32 size );
s8 *readRedirectFileToNewHugeBuffer( const s8 *basePath, const s8 *restPath, u32 size );
s8 *readFileToBuffer( const s8 *path, s32 seek, s8 *buf, u32 size );
bool writeBufferToFile( const s8 *path, s32 seek, s8 *buf, u32 size );
s8 *readStringToBuffer( const s8 *path, s32 seek, s8 *buf, u32 size, u32 rbs );
bool copyFileToFile( const s8 *path, const s8 *filename, s32 seek, u32 rbs, u32 which );
u32 computeDumpMemorySize( u32 size );
bool dumpMemory( s8 *pDest, u32 len, const s8 *pBuf, u32 size );

bool isNumeric(const s8 *p);
bool writeEntry( s8 *basePath, s8 *restPath, s8 *data);
s8 *trim(s8 *s);
bool isPCIFormat(const s8 *p);

u32 parseMonthString( const s8 *str );
bool isDateFormat( const s8 *str );
bool isTimeFormat( const s8 *str );
bool isMACFormat( const s8 *str );
bool isIPv4Format( const s8 *str );
u32 convertHexOrDecToBin( const s8 *str );
bool sanityCheckInputValue( const s8 *str );
bool sanityCheckPrintable( const s8 *str, u32 len );
s8 *mallocStringBuffer( const s8 *str );
u32 calculateChecksum( const s8 *str, u32 len );


