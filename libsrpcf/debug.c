/*
 * SRPCF - Simple Remote Procedire Command Framework
 * File: debug.c
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


void debugDumpMemory( s8 *pBuf, u32 size, u32 base ) {
#define BYTE_PER_LINE 16

    u32 i, j;
    u8 buf[ BYTE_PER_LINE ];
    s8 unalign = 0;

    if( size % BYTE_PER_LINE ) {

        unalign = 1;
    }

    printf( " Address | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F|   ASCII DATA   \n" );
    printf( "---------------------------------------------------------------------------\n" );

    for( i = 0 ; i <= size ; i++ ) {

        if( !(i % BYTE_PER_LINE) ) {

            if( i ) {
                for( j = 0 ; j < BYTE_PER_LINE ; j++ ) {

                    if( buf[ j ] >= '!' && buf[ j ] <= '~' )
                        printf( "%c", buf[ j ] );
                    else
                        printf( "." );
                }
                printf( "\n" );
            }

            if( i == size )
                break;

            printf( "%8.8X : ", i + base );
            memset( buf, ' ', sizeof( buf ) );
        }

        buf[ i % BYTE_PER_LINE ] = (BYTE)(*(pBuf + i));
        printf( "%2.2X ", buf[ i % BYTE_PER_LINE ] & 0xFF );
    }

    if( unalign ) {

        for( j = BYTE_PER_LINE - (size % BYTE_PER_LINE) - 1 ; j-- ; )
            printf( "   " );

        for( j = 0 ; j < (size % BYTE_PER_LINE) ; j++ )
            if( buf[ j ] >= '!' && buf[ j ] <= '~' )
                printf( "%c", buf[ j ] );
            else
                printf( "." );

        printf( "\n" );
    }

    printf( "---------------------------------------------------------------------------\n" );
}


