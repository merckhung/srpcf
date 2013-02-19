/*
 * SRPCF - Simple Remote Procedire Command Framework
 * File: srpcf_err.h
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
#if defined(LIBSRPCF_DEBUG) || defined(SRPCFSVR_DEBUG) || defined(SRPCFSH_DEBUG)
#define DBGPRINT( msg, args... )    printf( "%s: " msg, __func__, ##args );
#else
#define DBGPRINT( msg, args... )
#endif


//
// Enumernation
//
enum {

    SRPCF_SUCCESSFUL = 0,
    SRPCF_FAILED_PERM,
    SRPCF_FAILED_AGAIN,
    SRPCF_FAILED_NOMEM,
    SRPCF_FAILED_ACCESS,
    SRPCF_FAILED_BUSY,
    SRPCF_FAILED_NODEV,
    SRPCF_FAILED_INVALID,
    SRPCF_FAILED_UNKNOWN,
};


