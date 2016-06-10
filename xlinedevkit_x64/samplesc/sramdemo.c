/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         sramdemo.c
*
*     Version:      1.0
*
*     Description:  SRAM Demonstration
*
*     Notes:        Console application to demonstrate SRAM
*
*					Application reads NUM_BYTES bytes from memory
*					starting at READ_ADDR.  Its prints the values of the
*					memory at these location to the screen.  The application
*					then writes NUM_BYTES bytes of ascending data
*					starting at WRITE_ADDR as well as printing the data to
*					screen.
*
*					The application pauses between operations.
*
*
*     Changes:
*     1.0 MJB       First version using C API, 11/1/2008.
*     1.0 JSH       First properly released version using true C code and C-API, 11/10/2011.
*
*****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if defined(X10_LINUX_BUILD)
	/* LINUX specific build */
	#include <ctype.h>
	#include "linuxkb.h"
	#define TRUE 1						/* needed for 'C' */
	#define FALSE 0
#else
	/* WIN32 specific build */
	#include <conio.h>

	/* Remove "scanf" and "getch" warnings when compiling using Visual V++ 2005 */
	#pragma warning( disable : 4996 )
#endif

#include "xlinecapi.h"
#include "xlineauthenticate.h"
#include "unlockioc.h"
#include "xlinecexit.h"
#include "x15key.h"

/* Constants defining the operation of the application */
#define NUM_BYTES			0x0100
#define READ_ADDR			0x0000
#define WRITE_ADDR			0x0000

// define global so ExitFunction() can access to release memory
LPBYTE 			SRAM_data=NULL;

int DisplaySoftwareVersions( void *xBoard )
{
	BYTE VersionDll[20]={0}, Version8051[20]={0}, VersionPIC[20]={0};

	if ( !XlineGetDllVersion( xBoard, VersionDll ) )
		printf("Failure obtaining DLL version - %d!\n", (int)XlineGetLastError( xBoard ) );

	if ( !XlineGet8051Version( xBoard, Version8051 ) )
		printf("Failure obtaining 8051 version - %d!\n", (int)XlineGetLastError( xBoard ) );

	if ( !XlineGetPICVersion( xBoard, VersionPIC ) )
		printf("Failure obtaining PIC version - %d!\n", (int)XlineGetLastError( xBoard ) );

	printf( "Product version information:\n" );
	printf( "    API library version   : %s\n",   VersionDll );
	printf( "    8051 software version : %s\n",   Version8051 );
	printf( "    PIC software version  : %s\n\n", VersionPIC );

	return( 1 );
}

int main(int argc, char* argv[])
{
	void 			*xBoard;
	Authenticate	x15Authenticate;
	BYTE 			fittedBoard;
	BYTE 			i = 0;
	UINT 			e, c;
	char 			key;

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "This demonstration reads and writes to the serial EEPROM on the board.\n\n" );
	printf( "Establishing link with FireFly USB device..." );

	/* initialise the firefly device */
	xBoard = XlineInitBoard( );
	if ( xBoard == NULL )
	{
		return( XlineExit( "initialisation failed.", xBoard, &x15Authenticate, 1 ) );
	}

	/* Determine which board type is fitted, and unlock the IO security accordingly */
	XlineGetFittedBoard( xBoard, &fittedBoard );
	switch ( fittedBoard )
	{
		case	X10_BOARD:
		case	X10I_BOARD:		UnlockX10c( xBoard );
								break;

		case 	X15_BOARD:		/* start X15 authentication process */
								x15Authenticate = AuthenticateBeginThread( xBoard, EncryptedKey, 90 );
								break;

		default:				/* unknown board fitted */
								return( XlineExit( "unknown board fitted.", xBoard, &x15Authenticate, 1 ) );
								break;
	}
	printf( "success.\n\n" );

	if ( !DisplaySoftwareVersions( xBoard ) ) printf( "Error obtaining version info.\n\n" );

	printf( "Allocating memory..." );
	SRAM_data = (LPBYTE)malloc( NUM_BYTES + 1 );

	if ( SRAM_data == NULL )
		printf( "failure.\n\n" );
	else
		printf( "success.\n\n" );

	#ifdef X10_LINUX_BUILD
		InitialiseConsole( );
	#endif

	while ( SRAM_data != NULL )
	{
		/* Wait for keystroke.*/
		printf( "\nPlease press:\n" );
		printf( "      'E' to erase\n" );
		printf( "      'R' to read\n" );
		printf( "      'W' to write '%02x's\n", i );
		printf( "   or 'C' to close...\n" );

		key = getch();
		if ( key == 'E' || key == 'e' )
		{
			/* Generate array and print to the screen */
			for ( e = 0; e < NUM_BYTES; e++ ) SRAM_data[e] = 0;

			if ( !XlineWriteSRAM( xBoard, WRITE_ADDR, SRAM_data, NUM_BYTES ) )
			{
				printf( "\nWrite failed - %d.\n", (int)XlineGetLastError( xBoard ) );
			}
			else
			{
				printf( "\nSuccessfully erased %d bytes.\n", NUM_BYTES );
			}
		}
		else if ( ( key == 'R' ) || ( key == 'r' ) )
		{
			for ( e = 0; e < NUM_BYTES; e++ )
				SRAM_data[e] = 0xff;
			/* Read data */
			if ( !XlineReadSRAM( xBoard, READ_ADDR, SRAM_data, NUM_BYTES ) )
			{
				printf( "Read failed - %d.\n", (int)XlineGetLastError( xBoard ) );
			}
			else
			{
				for ( c = 0; c < NUM_BYTES; c++ )
				{
					if ( !((c + READ_ADDR) % 16) ) printf( "\n%04X:\t", c+READ_ADDR );
					printf( "%02X ", SRAM_data[c] & 0x00FF );
				}

				printf( "\nSuccessfully read %d bytes:\n", NUM_BYTES );
			}
		}
		else if ( ( key == 'W' ) || ( key == 'w' ) )
		{
			for ( e = 0; e < NUM_BYTES; e++ ) SRAM_data[e] = i;

			if ( !XlineWriteSRAM( xBoard, WRITE_ADDR, SRAM_data, NUM_BYTES ) )
			{
				printf( "\nWrite failed - %d.\n", (int)XlineGetLastError( xBoard ) );
			}
			else
			{
				printf( "\nSuccessfully written %d '%02x's.\n", NUM_BYTES, i );
				i++;
			}
		}
		else if ( ( key == 'C' ) || ( key == 'c' ) ) break;
	}

	if ( SRAM_data )
		free( SRAM_data );

	return( XlineExit( "Leaving program.", xBoard, &x15Authenticate, 0 ) );
}
