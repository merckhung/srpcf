/*
 * SRPCF - Simple Remote Procedire Command Framework
 * File: srpcf_types.h
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
// Types
//
typedef     unsigned char           u8;
typedef     unsigned short int      u16;
typedef     unsigned int            u32;
typedef     unsigned long long int  u64;

typedef     char                    s8;
typedef     short int               s16;
typedef     int                     s32;
typedef     long long int           s64;


#ifndef bool
typedef     unsigned char           bool;
#endif


//
// Definitions
//
#define 	TRUE					1
#define		FALSE					0


//
// Macros
//
#define		PACKED					__attribute__((packed))
#define		AS_STR(x)				#x


#define LinkListMalloc( HEAD, POINTER, COUNTER, DATATYPE ) { \
    if( COUNTER ) { \
        POINTER = &((*POINTER)->next); \
    } \
    else { \
        POINTER = HEAD; \
    } \
    *POINTER = (DATATYPE *)malloc( sizeof( DATATYPE ) ); \
    memset( *POINTER, 0, sizeof( DATATYPE ) ); \
}


#define ForeachLinkList( HEAD, POINTER ) \
	s32 idx; \
	for( idx = 0, POINTER = HEAD ; POINTER ; POINTER = POINTER->next, idx++ )


#define ARRAY_SIZE(x)     (sizeof(x) / sizeof((x)[0]))


//
// Structures
//
typedef struct _commonLinklist {

    struct _commonLinklist  *next;

} commonLinklist_t;


