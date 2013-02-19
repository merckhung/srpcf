/*
 * SRPCF - Simple Remote Procedire Command Framework
 * File: srpcf.c
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


u32 countSupportedSRPCFs( const srpcfSupported_t *pSrpcfSupported ) {

	u32 sum;
	for( sum = 0 ; (pSrpcfSupported + sum)->srpcfCmdNo != XR_END_SRPCF ; sum++ );
	return sum;
}


u32 checkSrpcfCmdEnabled( const s8 *srpcfStr, const srpcfSupported_t *pSrpcfSupported_t ) {

    s32 i;
    for( i = 0 ; (pSrpcfSupported_t + i)->srpcfCmdNo != XR_END_SRPCF ; i++ )
        if( ((pSrpcfSupported_t + i)->enabled == TRUE)
            && !strcmp( (pSrpcfSupported_t + i)->srpcfFuncName, srpcfStr ) )
            return (pSrpcfSupported_t + i)->srpcfCmdNo;

    return XR_END_SRPCF;
}


