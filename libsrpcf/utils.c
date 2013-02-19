/*
 * SRPCF - Simple Remote Procedire Command Framework
 * File: utils.c
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
#include <ctype.h>
#include <time.h>

#include "srpcf_types.h"
#include "srpcf.h"
#include "srpcf_err.h"
#include "libsrpcf.h"


s32 findBasename( const s8 *str ) {

	s8 *p;

	// Look for last '/' character
	p = rindex( str, '/' );
	if( !p )
		return 0;

	return (p - str + 1);
}


s32 countCharacter( const s8 *str, const s8 c ) {

	s32 i, count;

	count = 0;
	for( i = 0 ; str[ i ] ; i++ )
		if( str[ i ] == c )
			count++;

	return count;
}


s32 countLinklist( commonLinklist_t *head ) {

    s32 i;

    for( i = 0 ; head ; head = head->next, i++ );
    return i;
}


commonLinklist_t **tailOfLinklist( commonLinklist_t **head ) {

    commonLinklist_t **ppCommonLinklist;

	if( !*head )
		return head;

    for( ppCommonLinklist = head ;
			(*ppCommonLinklist)->next ; 
			ppCommonLinklist = &((*ppCommonLinklist)->next) );

	return ppCommonLinklist;
}


void appendLinklist( commonLinklist_t **head, commonLinklist_t *object ) {

	commonLinklist_t **ppCommonLinklist;

	ppCommonLinklist = tailOfLinklist( head );
	if( *ppCommonLinklist )
		(*ppCommonLinklist)->next = object;
	else
		*head = object;
}


commonLinklist_t *retriveFirstLinklist( commonLinklist_t **head ) {

	commonLinklist_t *pCommonLinklist = *head;

	if( !*head )
		return NULL;

	if( (*head)->next )
		*head = (*head)->next;
	else
		*head = NULL;

	return pCommonLinklist;
}


commonLinklist_t *removeLinklist( commonLinklist_t **head, commonLinklist_t *tgt ) {

    commonLinklist_t *curr = *head, *prev = NULL;

    if( !*head )
        return NULL;

    if( *head == tgt ) {

        if( (*head)->next )
            *head = (*head)->next;
        else
            *head = NULL;

        return curr;
    }

    for( prev = *head, curr = (*head)->next ;
        curr ; prev = curr, curr = curr->next ) {

        if( curr == tgt ) {

            prev->next = curr->next;
            return curr;
        }
    }

    return NULL;
}


void freeLinklist( commonLinklist_t *head ) {

	commonLinklist_t *prev;
	for( ; head ; ) {

		prev = head;
		head = head->next;
		free( prev );
	}
}


bool checkSrpcfCmdParam( const s8 *param, s8 **compare, s32 size ) {

	s32 i;
	for( i = 0 ; i < size ; i++ )
		if( !strcmp( param, compare[ i ] ) )
			return TRUE;

	return FALSE;
}


s8 *readFileToNewBuffer( const s8 *basePath, const s8 *restPath ) {

	s32 fd, len;
	s8 *p = NULL, path[ LIBSRPCF_MAX_PATH ];

	// Get full path
	snprintf( path, LIBSRPCF_MAX_PATH, "%s%s", basePath, restPath );

	// Open the file
	fd = open( path, O_RDONLY );
	if( fd < 0 )
		return NULL;

	// Read result
	len = read( fd, path, LIBSRPCF_MAX_PATH );
	if( len <= 0 )
		goto ErrExit;
	path[ len ] = 0;

	// Strip '\n'
	if( path[ len - 1 ] == '\n' ) {

		path[ len - 1 ] = 0;
		len--;
	}

	// Allocate a buffer
	p = malloc( len + 1 );
	if( !p )
		goto ErrExit;

	// Copy the string
	strncpy( p, path, len + 1 );

ErrExit:

	// Close
	close( fd );

	return p;
}


bool writeFileWithText( const s8 *basePath, const s8 *restPath, const s8 *text ) {

	s32 fd, len, wb;
	bool ret = TRUE;
	s8 path[ LIBSRPCF_MAX_PATH ];

	// Check string length
	len = strlen( text );
	if( len <= 0 )
		return FALSE;

	// Get full path
	snprintf( path, LIBSRPCF_MAX_PATH, "%s%s", basePath, restPath );

	// Open the file
	fd = open( path, O_WRONLY );
	if( fd < 0 )
		return FALSE;

	// Read result
	wb = write( fd, text, len );
	if( wb != len )
		ret = FALSE;

	// Close
	close( fd );
	return ret;
}


bool fetchLocation( const s8 *basePath, const s8 *restPath, s8 *buf, s32 len ) {

	s32 fd, rb;
	s8 path[ LIBSRPCF_MAX_PATH ];

	// Get full path
	snprintf( path, LIBSRPCF_MAX_PATH, "%s%s", basePath, restPath );

	// Open the file
	fd = open( path, O_RDONLY );
	if( fd < 0 )
		return FALSE;

	// Read path
	rb = read( fd, buf, len );
	if( rb <= 0 ) {

		close( fd );
		return FALSE;
	}
	buf[ rb ] = 0;

	// Close
	close( fd );
	return TRUE;
}


s8 *readRedirectFileToNewBuffer( const s8 *basePath, const s8 *restPath ) {
	s8 path[ LIBSRPCF_MAX_PATH ];

	// Redirect to somewhere
	if( fetchLocation( basePath, restPath, path, LIBSRPCF_MAX_PATH ) == FALSE )
		return NULL;

	return readFileToNewBuffer( path, "" );
}


bool writeRedirectFileWithText( const s8 *basePath, const s8 *restPath, const s8 *text ) {

	s32 len;
	s8 path[ LIBSRPCF_MAX_PATH ];

	// Check string length
	len = strlen( text );
	if( len <= 0 )
		return FALSE;

	// Redirect to somewhere
	if( fetchLocation( basePath, restPath, path, LIBSRPCF_MAX_PATH ) == FALSE )
		return FALSE;

	return writeFileWithText( path, "", text );
}


bool writeEitherWayFileWithInteger( const s8 *basePath, const s8 *restPath, const s32 value, const bool direct, const bool hex ) {

	s8 buf[ LIBSRPCF_MAX_PATH ];
	s8 *decf = "%d", *hexf = "%X";
	s8 *p = decf;

	if( hex == TRUE )
		p = hexf;

	// Prepare buffer
	snprintf( buf, LIBSRPCF_MAX_PATH, p, value );
	if( direct == TRUE )
        return writeFileWithText( basePath, restPath, buf );
	else
		return writeRedirectFileWithText( basePath, restPath, buf );
}


s8 *readFileToNewHugeBuffer( const s8 *basePath, const s8 *restPath, u32 size ) {

    s32 fd, len;
    s8 *p;

	// Allocate huge buffer first
	p = (s8 *)malloc( size );
	if( !p )
		return NULL;

    // Get full path
    snprintf( p, size, "%s%s", basePath, restPath );

    // Open the file
    fd = open( p, O_RDONLY );
    if( fd < 0 ) {

		free( p );
        return NULL;
	}

    // Read result
    len = read( fd, p, size );
    if( len <= 0 ) {

		free( p );
		close( fd );
	}
    p[ len ] = 0;

    // Strip '\n'
    if( p[ len - 1 ] == '\n' ) {

        p[ len - 1 ] = 0;
        len--;
    }

    // Close
    close( fd );
    return p;
}


s8 *readRedirectFileToNewHugeBuffer( const s8 *basePath, const s8 *restPath, u32 size ) {
    s8 path[ LIBSRPCF_MAX_PATH ];

    // Redirect to somewhere
    if( fetchLocation( basePath, restPath, path, LIBSRPCF_MAX_PATH ) == FALSE )
        return NULL;

    return readFileToNewHugeBuffer( path, "", size );
}


s8 *readFileToBuffer( const s8 *path, s32 seek, s8 *buf, u32 size ) {

	s32 fd, len;
	s8 *ret = NULL;

	// Open the file
	fd = open( path, O_RDONLY );
	if( fd < 0 )
		return NULL;

	// Seek position
	if( lseek( fd, seek, SEEK_SET ) != seek )
		goto ErrExit;

	// Read result
	len = read( fd, buf, size );
	if( len <= 0 )
		goto ErrExit;
	buf[ len ] = 0;

	// Strip '\n'
	if( buf[ len - 1 ] == '\n' )
		buf[ len - 1 ] = 0;

	// Return
	ret = buf;

ErrExit:

	// Close
	close( fd );

	return ret;
}


bool writeBufferToFile( const s8 *path, s32 seek, s8 *buf, u32 size ) {

	s32 fd, len;
	bool ret = FALSE;

	// Open the file
	fd = open( path, O_WRONLY );
	if( fd < 0 )
		return FALSE;

	// Seek position
	if( lseek( fd, seek, SEEK_SET ) != seek )
		goto ErrExit;

	// Read result
	len = write( fd, buf, size );
	if( len <= 0 || len != size )
		goto ErrExit;

	// Successful
	ret = TRUE;

ErrExit:

	// Close
	close( fd );
	return ret;
}


s8 *readStringToBuffer( const s8 *path, s32 seek, s8 *buf, u32 size, u32 rbs ) {

	s32 fd, len, i, j;
	s8 *p, *ret = NULL;
	bool found;

	// Check
	if( (rbs < 1) || (rbs > size ) )
		return NULL;

	// Open the file
	fd = open( path, O_RDONLY );
	if( fd < 0 )
		return NULL;

	// Seek position
	if( lseek( fd, seek, SEEK_SET ) != seek )
		goto ErrExit;

	// Read result
	for( found = FALSE, i = 0 ; i < size ; i += len ) {

		// Read data
		p = buf + i;
		len = read( fd, p, rbs );
		if( len <= 0 || len != rbs )
			break;

		// Detect NULL terminator
		for( j = 0 ; j < len ; j++ )
			if( *(p + j) == '\0' ) {

				found = TRUE;
				break;
			}

		if( found == TRUE )
			break;
	}

	// Return
	ret = buf;

ErrExit:

	// Close
	close( fd );
	return ret;
}


bool copyFileToFile( const s8 *path, const s8 *filename, s32 seek, u32 rbs, u32 which ) {

    s32 fd, fd1, len;
	s8 *p;
	bool ret = FALSE;

    // Sanity check
    if( rbs < 1 || !path || !filename || (which >= 3) )
        return FALSE;

    // Open the file
    fd = open( path, O_RDONLY );
    if( fd < 0 )
        return FALSE;

	// Create the file
	fd1 = open( filename, O_WRONLY | O_CREAT | O_TRUNC, 0644 );
	if( fd1 < 0 )
		goto ErrExit1;

	// Allocate a buffer
	p = malloc( rbs );
	if( !p )
		goto ErrExit1;
	memset( p, 0, rbs );

    // Seek position
	switch( which ) {

    case 0:
		if( lseek( fd, seek, SEEK_SET ) != seek )
        	goto ErrExit2;
		break;

	case 1:
		if( lseek( fd1, seek, SEEK_SET ) != seek )
			goto ErrExit2;
		break;

	case 2:
		if( lseek( fd, seek, SEEK_SET ) != seek )
			goto ErrExit2;
		if( lseek( fd1, seek, SEEK_SET ) != seek )
			goto ErrExit2;
		break;

	default:
		goto ErrExit2;
	}

    // Read buffer
	len = read( fd, p, rbs );
    if( len <= 0 )
		goto ErrExit2;

	// Write buffer
	write( fd1, p, len );

	// Indicate SUCCESS
	ret = TRUE;

ErrExit2:
	free( p );

ErrExit1:
	close( fd1 );
    close( fd );

    return ret;
}


u32 computeDumpMemorySize( u32 size ) {

	u32 line;

	line = size / LIBSRPCF_BPL;
	if( size % LIBSRPCF_BPL )
		line++;

	// 16:SPEC, 3:TITLE, 1:\n, 1:NULL
	return (((16 + 3 + 1) * line + 1) + (size * 2));
}


bool dumpMemory( s8 *pDest, u32 len, const s8 *pBuf, u32 size ) {

    u32 i;
	s8 *p;

	// Check NULL or 0
	if( !len || !size || !pDest || !pBuf )
		return FALSE;

	if( computeDumpMemorySize( size ) < len )
		return FALSE;

	// Clear buffer
	memset( pDest, 0, len );

	// Format output
	p = pDest;
	for( i = 0 ; i < size ; i++ ) {

		// Print the line index
		if( !i )
			p += sprintf( p, "%2.2x:", i );
		else if( !(i % LIBSRPCF_BPL) )
			p += sprintf( p, "\n%2.2x:", i );

		// Print value in hex
		p += sprintf( p, " %2.2X", (*(pBuf + i) & 0xFF) );
	}

	return TRUE;
}


bool isNumeric( const s8 *p ) {

	s8 c;

	if( *p ) {

          while ((c=*p++)) {
		  	
                if (!isdigit(c)) 
			return FALSE;
          }
          return TRUE;
      }
      return FALSE;
}


bool writeEntry( s8 *basePath, s8 *restPath, s8 *data ) {

	s32 fd, len;
	s8 path[ LIBSRPCF_MAX_PATH ];

	// Get full path
	snprintf( path, LIBSRPCF_MAX_PATH, "%s%s", basePath, restPath );
	DBGPRINT("Write path = %s\n",path);

	// Open the file
	fd = open( path, O_WRONLY | O_TRUNC);

	if( fd < 0 )
		return FALSE;

	//[TODO]Clear all data First

	if( data == NULL )
		len = write(fd, "01011",5);	//For Clear 01:one parameter, 01:length, 1 is clear
	else
		len = write(fd, data,strlen(data));

	if(!len){
		DBGPRINT("Write Entry emptry\n");
 		goto ErrExit;
	}
	
	return TRUE;
	
ErrExit:

	// Close
	close( fd );
		
	return FALSE;
}


s8 *trim( s8 *s ) {

    s8* back = s + strlen( s );

    while( isspace( *--back ) );
    *(back + 1) = '\0';

    return s;
}


bool isPCIFormat( const s8 *p ) {

    s8 *delim=":",*temp;
    s8 path[LIBSRPCF_MAX_PATH];

    snprintf( path, LIBSRPCF_MAX_PATH, "%s", p );
    temp=strtok(path,delim);
    if(temp==NULL){
        return FALSE;
    }

    temp=strtok(NULL,delim);
    if(temp==NULL){
        return FALSE;
    }

    temp=strtok(NULL,delim);
    if(temp==NULL){
        return FALSE;
    }

return TRUE;
}


static s8 *months[] = {
	"JAN",
	"FEB",
	"MAR",
	"APR",
	"MAY",
	"JUN",
	"JUL",
	"AUG",
	"SEP",
	"OCT",
	"NOV",
	"DEC"
};


u32 parseMonthString( const s8 *str ) {

	s32 i;

	if( strlen( str ) != 3 )
		return 0;

	for( i = 0 ; i < ARRAY_SIZE( months ) ; i++ )
		if( !strcmp( str, months[ i ] ) )
			return i + 1;

	return 0;
}


s8 *getMonthString( u32 index ) {

	if( !index || (index > ARRAY_SIZE( months )) )
		return NULL;

	return months[ index - 1 ];
}


bool isDateFormat( const s8 *str ) {

	u32 year, month, day;
	bool leap = FALSE;
	s8 input[ 12 ];

	// Expected "JAN-01-2011"
	if( strlen( str ) != 11 )
		return FALSE;
	strncpy( input, str, sizeof( input ) );

	if( input[ 3 ] != '-' || input[ 6 ] != '-' )
		return FALSE;

	// Terminate string
	input[ 3 ] = 0;
	input[ 6 ] = 0;

	// Parse month string
	month = parseMonthString( &input[ 0 ] );

	// Convert day & year string
	day = strtol( (const s8 *)&input[ 4 ], NULL, 10 );
	year = strtol( (const s8 *)&input[ 7 ], NULL, 10 );

	// Identify leap year
	if( !(year % 400) )
		leap = TRUE;
	else if( year % 100 )
		if( !(year % 4) )
			leap = TRUE;

	// Sanity check
	switch( month ) {
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		if( day < 1 || day > 31 )
			return FALSE;
		break;

	case 4:
	case 6:
	case 9:
	case 11:
		if( day < 1 || day > 30 )
			return FALSE;
		break;

	case 2:
		if( day < 1 )
			return FALSE;

		if( leap == TRUE ) {
			if( day > 29 )
				return FALSE;
		}
		else if( day > 28 )
			return FALSE;
		break;

	default:
		return FALSE;
	}

	return TRUE;
}


bool isTimeFormat( const s8 *str ) {

    s32 len;
    s8 *am_str = "AM", *pm_str = "PM";
    s8 buf[ 12 ];
    u32 h = 0, m = 0, s = 0;
    bool pm = FALSE;

    // Length check
    len = strlen( str );
    if( len != 11 && len != 8 && len != 7 && len != 5 && len != 4 )
        return FALSE;
    strncpy( buf, str, sizeof( buf ) );

    // Convert strings
    switch( len ) {

    case 11:
        // HH:MM:SS AM
        buf[ 2 ] = 0;
        buf[ 5 ] = 0;
        buf[ 8 ] = 0;

        h = strtol( (const s8 *)&buf[ 0 ], NULL, 10 );
        m = strtol( (const s8 *)&buf[ 3 ], NULL, 10 );
        s = strtol( (const s8 *)&buf[ 6 ], NULL, 10 );
        if( !strcmp( &buf[ 9 ], am_str ) )
            pm = FALSE;
        else if( !strcmp( &buf[ 9 ], pm_str ) )
            pm = TRUE;
        else
            return FALSE;
        break;

    case 8:

        // HH:MM AM
        // HH:MM:SS
        buf[ 2 ] = 0;
        buf[ 5 ] = 0;
        if( !strcmp( &buf[ 6 ], am_str ) || !strcmp( &buf[ 6 ], pm_str ) ) {

            // HH:MM AM
            if( buf[ 6 ] == 'A' )
                pm = FALSE;
            else
                pm = TRUE;
        }
        else {

            // HH:MM:SS
            s = strtol( (const s8 *)&buf[ 6 ], NULL, 10 );
            pm = FALSE;
        }
        h = strtol( (const s8 *)&buf[ 0 ], NULL, 10 );
        m = strtol( (const s8 *)&buf[ 3 ], NULL, 10 );
        break;

    case 7:
        // H:MM AM
        buf[ 1 ] = 0;
        buf[ 4 ] = 0;
        h = strtol( (const s8 *)&buf[ 0 ], NULL, 10 );
        m = strtol( (const s8 *)&buf[ 2 ], NULL, 10 );
        if( !strcmp( &buf[ 5 ], am_str ) )
            pm = FALSE;
        else if( !strcmp( &buf[ 5 ], pm_str ) )
            pm = TRUE;
        else
            return FALSE;
        break;

    case 5:
        // HH:MM
        buf[ 2 ] = 0;
        h = strtol( (const s8 *)&buf[ 0 ], NULL, 10 );
        m = strtol( (const s8 *)&buf[ 3 ], NULL, 10 );
        pm = FALSE;
        break;

    case 4:
        // H:MM
        buf[ 1 ] = 0;
        h = strtol( (const s8 *)&buf[ 0 ], NULL, 10 );
        m = strtol( (const s8 *)&buf[ 2 ], NULL, 10 );
        pm = FALSE;
        break;

    default:
        return FALSE;
    }

    // Sanity check
    if( h < 1 || h > 12 || m > 59 || s > 59 )
        return FALSE;

	return TRUE;
}


bool isMACFormat( const s8 *str ) {

	s32 i;
	bool valid = TRUE;

	if( strlen( str ) !=  LIBSRPCF_MAC_STR_LEN )
		return FALSE;

	for( i = 0 ; i < LIBSRPCF_MAC_STR_LEN ; i++ ) {

		if( str[ i ] >= '0' && str[ i ] <= '9' )
			continue;

		if( str[ i ] >= 'a' && str[ i ] <= 'f' )
			continue;

		if( str[ i ] >= 'A' && str[ i ] <= 'F' )
			continue;

		valid = FALSE;
	}

	return valid;
}


bool isIPv4Format( const s8 *str ) {

	s32 i, j, pos[ 3 ], len;
	s8 buf[ LIBSRPCF_IP_STR_BUF ];
	u32 addr[ 4 ];
	bool valid = TRUE;

	// Length check
	len = strlen( str );
	if( len > LIBSRPCF_IP_STR_LEN || len < LIBSRPCF_IP_STR_LEN_MIN )
		return FALSE;
	strncpy( buf, str, LIBSRPCF_IP_STR_BUF );

	// Look for positions of delimiters
	for( i = 0, j = 0 ; i < len ; i++ ) {

		if( buf[ i ] == '.' ) {

			// Exceed the limit
			if( j >= 3 )
				return FALSE;

			// Record & Terminate string
			pos[ j ] = i;
			buf[ i ] = 0;
			j++;
		}
	}

	// Must have 3 dots at least or at most
	if( j != 3 )
		return FALSE;

	// Convert strings
	addr[ 0 ] = strtol( (const s8 *)&buf[ 0 ], NULL, 10 );
	addr[ 1 ] = strtol( (const s8 *)&buf[ pos[ 0 ] + 1 ], NULL, 10 );
	addr[ 2 ] = strtol( (const s8 *)&buf[ pos[ 1 ] + 1 ], NULL, 10 );
	addr[ 3 ] = strtol( (const s8 *)&buf[ pos[ 2 ] + 1 ], NULL, 10 );

	// Check range
	for( i = 0 ; i < 4 ; i++ )
		if( addr[ i ] > 255 ) {

			valid = FALSE;
			break;
		}

	// Not ZERO
	snprintf( buf, LIBSRPCF_IP_STR_BUF, "%d.%d.%d.%d", addr[ 0 ], addr[ 1 ], addr[ 2 ], addr[ 3 ] );
	if( strcmp( buf, str ) )
		valid = FALSE;

	return valid;
}


u32 convertHexOrDecToBin( const s8 *str ) {

	if( str[ 0 ] == '0' && str[ 1 ] == 'x' )
		return strtol( str + 2, NULL, 16 );

	return strtol( str, NULL, 10 );
}


bool sanityCheckInputValue( const s8 *str ) {

	s32 len, i;

	len = strlen( str );
	if( str[ 0 ] == '0' && str[ 1 ] == 'x' ) {

		// Hex
		for( i = 2 ; i < len ; i++ ) {

			if( ((str[ i ] >= '0') && (str[ i ] <= '9')) 
				|| ((str[ i ] >= 'a') && (str[ i ] <= 'f'))
				|| ((str[ i ] >= 'A') && (str[ i ] <= 'F')) )
				continue;

			return FALSE;
		}
	}
	else {

		// Dec
		for( i = 0 ; i < len ; i++ ) {

			if( (str[ i ] >= '0') && (str[ i ] <= '9') )
				continue;

			return FALSE;
		}
	}

	return TRUE;
}


bool sanityCheckPrintable( const s8 *str, u32 len ) {

	u32 i;

	for( i = 0 ; i < len ; i++ )
		if( str[ i ] < ' ' || str[ i ] >= 0x7F )
			return FALSE;

	return TRUE;
}


s8 *mallocStringBuffer( const s8 *str ) {

	s32 len;
	s8 *p;

	if( !str )
		return NULL;

	len = strlen( str );
	if( len < 1 )
		return NULL;

	p = (s8 *)malloc( len + 1 );
	if( !p )
		return NULL;

	strncpy( p, str, len + 1 );
	return p;
}


u32 calculateChecksum( const s8 *str, u32 len ) {

	u32 chksum = 0, i;

	for( i = 0 ; i < len ; i++ )
		chksum += str[ i ];

	return ~chksum + 1;
}


