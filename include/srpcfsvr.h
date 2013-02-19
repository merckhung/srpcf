/*
 * SRPCF - Simple Remote Procedire Command Framework
 * File: srpcfsvr.h
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
#define SRPCFSVR_REVISION		SRPCF_CODE_REVISION
#define SRPCFSVR_SLEEP_MS		100


//
// Structures
//
typedef struct _srpcfSvrThd {

    struct _srpcfSvrThd	*next;

    pthread_t           pth;
    s32                 cfd;
    s8                  packet[ LIBSRPCF_MSG_SIZE ];
    s32                 rwByte;

} srpcfSvrThd_t;


typedef struct _srpcfSvrTask {

    struct _srpcfSvrTask 	*next;
    srpcfSvrCommPkt_t		*pktData;

} srpcfSvrTask_t;


