/*
 * SRPCF - Simple Remote Procedire Command Framework
 * File: xrPciList.c
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

#include "srpcf_plugin.h"
#include <dirent.h>


#define SYSFS_PCI_LIST_PATH	"/sys/bus/pci/devices"
#define PCI_LIST_TITLE		"PCI DEVICE\tDEVICE ID\tVENDOR ID\tREVISION ID\tFUNCTION #\n"
#define PCI_LIST_FMT		"%-16s %4.4X\t\t%4.4X\t\t0x%2.2X\t\t0x%2.2X\n"
#define SMALL_BUF			20
#define BIG_BUF				2048


typedef struct _pciClassName {

	u32	baseClass;
	u32	subClass;
	s8	*devName;

} pciClassName_t;


static pciClassName_t pciClassName[] = {

	{ 0x00,	0xFF, "OLD DEVICE" },
	{ 0x01, 0xFF, "HOST" },
	{ 0x02, 0xFF, "NETWORK" },
	{ 0x03, 0xFF, "DISPLAY" },
	{ 0x04, 0xFF, "MULTIMEDIA" },
	{ 0x05, 0xFF, "MEMORY" },
	{ 0x06, 0xFF, "BRIDGE" },
	{ 0x07, 0xFF, "SIMPLECOMM" },
	{ 0x08, 0xFF, "PERIPHERAL" },
	{ 0x09, 0xFF, "INPUT" },
	{ 0x0A, 0xFF, "DOCKING" },
	{ 0x0B, 0xFF, "PROCESSOR" },
	{ 0x0C, 0xFF, "SERIALBUS" },
	{ 0x0D, 0xFF, "WIRELESS" },
	{ 0x0E, 0xFF, "INTELLIGENT" },
	{ 0x0F, 0xFF, "SATELLITE" },
	{ 0x10, 0xFF, "ENCRYPTION" },
	{ 0x11, 0xFF, "SIGNAL" },
};


// SRPCF Shell Help
LIBSRPCF_HELPER_TEXT( "\
SYNTAX:\n\
\txrPciList\n\
USAGE:\n\
\tThis function will display the PCI device ID, vendor ID, revision ID, and a function number\n\
\tof all PCI devices on the controller. Some devices show up multiple times if they are used\n\
\tas separate devices by the firmware.\n\
" );


// SRPCF Shell Error
LIBSRPCF_ERROR_FUNC( xrPciList ) {}


// SRPCF Shell Implementation
LIBSRPCF_SHELL_IMPLEMENT( xrPciList ) {

	return TRUE;
}


// SRPCF Server Implementation
LIBSRPCF_SERVER_IMPLEMENT( xrPciList ) {

	s8 *p = NULL, *orig;
	DIR *top;
	struct dirent *dir;

	s32 i;
	s8 path[ LIBSRPCF_MAX_PATH ];
	s8 buf[ SMALL_BUF ];
	u32 func, venid, devid, rev, class;
	s8 *name;


	// Allocate buffer
	orig = p = malloc( BIG_BUF );
	if( !p )
		goto ErrExit;
	memset( p, 0, BIG_BUF );


	// Open the top directory
	top = opendir( SYSFS_PCI_LIST_PATH );
	if( !top )
		goto ErrExit;


	// Print title
	p += snprintf( p, BIG_BUF - (p - orig), PCI_LIST_TITLE );


	// Walk through all subdir
	for( dir = readdir( top ) ; dir ; dir = readdir( top ) ) {

		// Skip "." & ".."
		if( !strcmp( dir->d_name, "." )
			|| !strcmp( dir->d_name, ".." ) )
			continue;


		// Convert to BUS, DEV, and FUNC
		strncpy( buf, dir->d_name, sizeof( buf ) );
		buf[ 7 ] = 0;
		buf[ 10 ] = 0;
		func = strtol( (const s8 *)&buf[ 11 ], NULL, 16 );


		// Device ID
		snprintf( path, 
			LIBSRPCF_MAX_PATH, 
			SYSFS_PCI_LIST_PATH "/%s/%s", 
			dir->d_name, 
			"device" );

		if( !readFileToBuffer( path, 2, buf, sizeof( buf ) ) )
			continue;

		devid = strtol( (const s8 *)&buf, NULL, 16 );


		// Vendor ID
		snprintf( path, 
			LIBSRPCF_MAX_PATH, 
			SYSFS_PCI_LIST_PATH "/%s/%s", 
			dir->d_name, 
			"vendor" );

		if( !readFileToBuffer( path, 2, buf, sizeof( buf ) ) )
			continue;

		venid = strtol( (const s8 *)&buf, NULL, 16 );


		// Class
		snprintf( path,
			LIBSRPCF_MAX_PATH,
			SYSFS_PCI_LIST_PATH "/%s/%s",
			dir->d_name,
			"class" );

		if( !readFileToBuffer( path, 2, buf, sizeof( buf ) ) )
			continue;

		class = strtol( (const s8 *)&buf, NULL, 16 );


		// Revision
		snprintf( path,
			LIBSRPCF_MAX_PATH,
			SYSFS_PCI_LIST_PATH "/%s/%s",
			dir->d_name,
			"config" );

		rev = 0;
		if( !readFileToBuffer( path, 0x08, (s8 *)&rev, 1 ) )
			continue;


		// Look for PCI name
		for( name = NULL, i = 0 ; i < ARRAY_SIZE( pciClassName ) ; i++ )
			if( pciClassName[ i ].baseClass == ((class & 0x00FF0000) >> 16) )
				name = pciClassName[ i ].devName;
			

		// Print PCI device
		p += snprintf( p,
			BIG_BUF - (p - orig), 
			PCI_LIST_FMT,
			name, 
			devid, 
			venid, 
			rev, 
			func );
	}


	// Close the top directory
	closedir( top );

	// Return
    *errorCode = SRPCF_SUCCESSFUL;
	return orig;

ErrExit:

	if( orig )
		free( orig );

	*errorCode = SRPCF_FAILED_NODEV;
	return NULL;
}


