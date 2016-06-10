/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         eepromdemo.cpp
*
*     Version:      1.1
*
*     Description:  EEPROM Demonstration
*
*     Notes:        Console application to demonstrate EEPROM
*
*					Application reads from and writes to EEPROM
*
*
*     Changes:
*
*	  1.1 RDP		Added support for X15 product 10/10/2007
*     1.0 MJB       First version. Based on issue 1.1 of
*                   X10 eepromdemo.cpp, 28/7/2005
*
*****************************************************************/

#include <stdio.h>

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
#define MEMORY_CAPACITY		512				// EEPROM size in bytes
#define MEMORY_BEGIN		8				// First location in EEPROM that can be written to/read from

// Obtains a hex number from the user. Repeats until valid response is given
UINT obtain_hex_input( int max_bytes )
{
	char input[100];
	int length, i;
	UINT result = 0;
	BOOL input_error;
	#ifdef X10_LINUX_BUILD
		int  ignore __attribute__ ((unused));
	#else
		int  ignore;
	#endif

	do
	{
		input_error = FALSE;
		ignore = scanf( "%s", input );				// ignore used to quell g++ compiler warning
		length = (int)strlen( input );

		if ( ( length < 1 ) || ( length > max_bytes ) ) input_error = TRUE;
		else
		{
			for ( i = 0; i < length; i++ )
			{
				input[i] = toupper(input[i]);

				if (( input[i] >= '0' ) && ( input[i] <= '9' ))
					result += ( ( input[i] - '0' ) << ( (length - i - 1) * 4 ) );
				else if (( input[i] >= 'A' ) && ( input[i] <= 'F' ))
					result += ( ( input[i] - 'A' + 10 ) << ( (length - i - 1) * 4 ) );
				else input_error = TRUE;
			}
		}

		if ( input_error ) printf( "Error - Please try again: " );
	} while( input_error );

	return( result );
}

int main(int argc, char* argv[])
{
	FireFlyUSB		FireFly;
	Authenticate	X15Authenticate;
	UINT 			read_address=0, read_bytes=0, write_address=0, write_bytes=0, index;
	BYTE 			data[MEMORY_CAPACITY]={0};						// Create buffer for data
	BYTE 			byte_to_write;
	BOOL 			addressing_error;
	BYTE 			fittedBoard;


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

	#ifdef X10_LINUX_BUILD
	InitialiseConsole( );
	EnableEcho( );
	#endif

	while ( TRUE )
	{
		do
		{
			printf( "Please enter start read address (in hex): " );
			read_address = obtain_hex_input( 3 );
			printf( "Please enter number of bytes to read (in hex): " );
			read_bytes = obtain_hex_input( 3 );

			if ( ( read_address + read_bytes ) > MEMORY_CAPACITY )
			{
				printf( "\nAddress error: cannot read beyond address %Xh.\n", MEMORY_CAPACITY );
				addressing_error = TRUE;
			}
			else if ( read_address < MEMORY_BEGIN )
			{
				printf( "\nAddress error: cannot read below address %Xh.\n", MEMORY_BEGIN );
				addressing_error = TRUE;
			}
			else addressing_error = FALSE;

			if ( addressing_error ) printf( "Please try again.\n\n" );
		} while ( addressing_error );

		printf ( "\nReading %Xh bytes from address %Xhh ... ", read_bytes, read_address );

		// Read data
		if ( !FireFly.ReadEEPROM( read_address, (LPBYTE) &data, read_bytes ) )
			return( ExitFunction( "failed.", &FireFly, &X15Authenticate, 1 ) );
		printf( "success.\n" );

		for ( index = 0; index < read_bytes; index++ )
		{
			if ( !(( index + read_address ) % 16 ) || ( index == 0 ) )
				printf( "\n0x%04X:\t", index+read_address );
			printf( "%02X ", data[index] & 0x00FF );
		}

		do
		{
			printf( "\n\nPlease enter start write address (in hex): " );
			write_address = obtain_hex_input( 3 );
			printf( "Please enter number of bytes to write (in hex): " );
			write_bytes = obtain_hex_input( 3 );
			printf( "Please enter byte to write (in hex): " );
			byte_to_write = (BYTE)obtain_hex_input( 2 );

			if ( ( write_address + write_bytes ) > MEMORY_CAPACITY )
			{
				printf( "\nAddress error: cannot write beyond address %Xh.\n", MEMORY_CAPACITY );
				addressing_error = TRUE;
			}
			else if ( write_address < MEMORY_BEGIN )
			{
				printf( "\nAddress error: cannot write below address %Xh.\n", MEMORY_BEGIN );
				addressing_error = TRUE;
			}
			else addressing_error = FALSE;

			if ( addressing_error ) printf( "Please try again." );
		} while ( addressing_error );

		// Update data to write
		for ( index = 0; index < write_bytes; index++ ) data[index] = byte_to_write;

		// Write data
		printf( "\n\nWriting %Xh bytes of '%02Xh' to address %Xh ... \n", write_bytes, byte_to_write, write_address );
		if ( !FireFly.WriteEEPROM( write_address, (LPBYTE) &data, write_bytes ) )
			return( ExitFunction( "failed.", &FireFly, &X15Authenticate, 1 ) );
		printf( "success.\n" );

		// Generate array and print to the screen
		for ( index = 0; index < write_bytes; index++ )
		{
			if ( !(( index + write_address ) % 16 ) || ( index == 0 ) )
				printf( "\n0x%04X:\t", index + write_address );

			printf( "%02X ", data[index] & 0x00FF );
		}

		// Wait for keystroke.
		printf( "\nPress any key to continue or 'C' to close...\n\n" );
		char key = getch();
		if ( key == 'C' || key == 'c' ) break;
	}

	return( ExitFunction( "Leaving program.", &FireFly, &X15Authenticate, 0 ) );
}
