/*
 * SRPCF - Simple Remote Procedire Command Framework
 * File: netsock.h
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
#define SRPCF_DEF_PORT				8989


//
// Prototypes
//
s32 initializeSocket( s32 *fd, s8 *addr, s32 port );
s32 connectSocket( s32 *fd, s8 *addr, s32 port );
void deinitializeSocket( s32 fd );
s32 acceptSocket( s32 fd, s32 *apsd );
s32 transferSocket( s32 fd, const void *pktBuf, const u32 length, s32 *wByte );
s32 receiveSocket( s32 fd, void *pktBuf, const u32 length, s32 *rByte );


