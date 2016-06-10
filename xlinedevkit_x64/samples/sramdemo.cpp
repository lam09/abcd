/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         sramdemo.cpp
*
*     Version:      1.1
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
*
*	  1.1 RDP		Added support for X15 product 10/10/2007
*     1.0 MJB       First version. Based on issue 1.1 of
*                   X10 sramdemo.cpp, 29/7/2005
*
*****************************************************************/

#include <stdio.h>
#include <time.h>

#ifndef X10_LINUX_BUILD
/* WIN32 specific build */
	#include <conio.h>
	#include "authenticate_win.h"

/* Remove "scanf" and "getch" warnings when compiling using Visual V++ 2005 */
	#pragma warning( disable : 4996 )
#else
/* LINUX specific build */
	#include <ctype.h>
	#include "linuxkb.c"
	#include "authenticate_linux.h"
#endif

#include "fflyusb.h"
#include "unlockio.h"
#include "x10errhd.c"
#include "x15key.h"

// Constants defining the operation of the application
#define NUM_BYTES			0x0100
#define READ_ADDR			0x0000
#define WRITE_ADDR			0x0000

// define global so ExitFunction() can access to release memory
LPBYTE 			SRAM_data=NULL;


int DisplaySoftwareVersions( FireFlyUSB *FireFly )
{
	BYTE VersionDll[20]={0}, Version8051[20]={0}, VersionPIC[20]={0};

	if ( !FireFly->GetDllVersion( VersionDll ) )
		printf("Failure obtaining DLL version - %d!\n", (int)FireFly->GetLastError( ) );

	if ( !FireFly->Get8051Version( Version8051 ) )
		printf("Failure obtaining 8051 version - %d!\n", (int)FireFly->GetLastError( ) );

	if ( !FireFly->GetPICVersion( VersionPIC ) )
		printf("Failure obtaining PIC version - %d!\n", (int)FireFly->GetLastError( ) );

	printf( "Product version information:\n" );
	printf( "    API library version   : %s\n",   VersionDll );
	printf( "    8051 software version : %s\n",   Version8051 );
	printf( "    PIC software version  : %s\n\n", VersionPIC );

	return( 1 );
}

int main(int argc, char* argv[])
{
	FireFlyUSB 		FireFly;
	Authenticate	X15Authenticate;
	BYTE 			fittedBoard=0;
	BYTE 			i = 0;
	UINT 			e;

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "This demonstration reads and writes to the serial EEPROM on the board.\n\n" );
	printf( "Establishing link with FireFly USB device..." );

/* initialise the firefly device */
	// Get a handle to the device
	if ( !FireFly.init( ) )
		return( ExitFunction( "initialisation failed.", &FireFly, &X15Authenticate, 1 ) );

/* Determine which board type is fitted, and unlock the IO security accordingly */
	FireFly.GetFittedBoard( &fittedBoard );
	switch ( fittedBoard )
	{
		case	X10_BOARD:
		case	X10I_BOARD:		UnlockX10( &FireFly );
								break;

		case 	X15_BOARD:		// start X15 authentication process
								X15Authenticate.BeginThread( &FireFly, EncryptedKey, 90 );
								break;

		default:				// unknown board fitted
								return( ExitFunction( "unknown board fitted.", &FireFly, &X15Authenticate, 1 ) );
								break;
	}
	printf( "success.\n\n" );

	UnlockX10( &FireFly );

	if ( !DisplaySoftwareVersions( &FireFly ) ) printf( "Error obtaining version info.\n\n" );

	printf( "Allocating memory..." );
	SRAM_data = (LPBYTE)malloc( NUM_BYTES + 1 );
	if ( SRAM_data == NULL )
	{
		printf( "Allocate memory failure.\n\n" );
		free(SRAM_data);
		return( ExitFunction( "memory allocation error.", &FireFly, &X15Authenticate, 1 ) );
	}
	else
		printf( "success.\n\n" );

	#ifdef X10_LINUX_BUILD
		InitialiseConsole( );
	#endif

	while ( SRAM_data != NULL )
	{
		// Wait for keystroke.
		printf( "\nPlease press:\n" );
		printf( "      'E' to erase\n" );
		printf( "      'R' to read\n" );
		printf( "      'W' to write '%02x's\n", i );
		printf( "   or 'C' to close...\n" );

		char key = getch();
		if ( key == 'E' || key == 'e' )
		{
			// Generate array and print to the screen
			for ( UINT e = 0; e < NUM_BYTES; e++ ) SRAM_data[e] = 0;

			if ( !FireFly.WriteSRAM( WRITE_ADDR, SRAM_data, NUM_BYTES ) ) {
				printf( "\nWrite failed - %d.\n", (int)FireFly.GetLastError( ) );
			}
			else printf( "\nSuccessfully erased %d bytes.\n", NUM_BYTES );
		}
		if ( key == 'R' || key == 'r' )
		{
			for ( e = 0; e < NUM_BYTES; e++ ) SRAM_data[e] = 0xff;
			// Read data
			if ( !FireFly.ReadSRAM( READ_ADDR, SRAM_data, NUM_BYTES ) ) {
				printf( "Read failed - %d.\n", (int)FireFly.GetLastError( ) );
			}
			else
			{
				for ( UINT c = 0; c < NUM_BYTES; c++ )
				{
					if ( !((c + READ_ADDR) % 16) ) printf( "\n%04X:\t", c+READ_ADDR );
					printf( "%02X ", SRAM_data[c] & 0x00FF );
				}

				printf( "\nSuccessfully read %d bytes:\n", NUM_BYTES );
			}
		}
		if ( key == 'W' || key == 'w' )
		{
			for ( UINT e = 0; e < NUM_BYTES; e++ ) SRAM_data[e] = i;

			if ( !FireFly.WriteSRAM( WRITE_ADDR, SRAM_data, NUM_BYTES ) ) {
				printf( "\nWrite failed - %d.\n", (int)FireFly.GetLastError( ) );
			}
			else
			{
				printf( "\nSuccessfully written %d '%02x's.\n", NUM_BYTES, i );
				i++;
			}
		}
		if ( key == 'C' || key == 'c' ) break;
	}

	if (SRAM_data)
		free(SRAM_data);
	return( ExitFunction( "Leaving program.", &FireFly, &X15Authenticate, 0 ) );
}
