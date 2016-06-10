/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         eepromdemo.c
*
*     Version:      1.0
*
*     Description:  EEPROM Demonstration
*
*     Notes:        Console application to demonstrate EEPROM
*
*					Application reads from and writes to EEPROM
*
*
*     Changes:
*     1.0 MJB       First version using C API, 11/1/2008.
*     1.0 JSH       First properly released version using true C code and C-API, 11/10/2011.
*
*****************************************************************/

#include <stdio.h>
#include <string.h>

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
#define MEMORY_CAPACITY		512				/* EEPROM size in bytes */
#define MEMORY_BEGIN		8				/* First location in EEPROM that can be written to/read from */

/* Obtains a hex number from the user. Repeats until valid response is given */
UINT obtain_hex_input( int max_bytes )
{
	char input[100];
	int length, i;
	UINT result = 0;
	BOOL input_error;

	#ifdef X10_LINUX_BUILD
		int			ignore __attribute__ ((unused));
	#else
		int 		ignore;
	#endif

	do
	{
		input_error = FALSE;
		ignore = scanf( "%s", input );				// ignore used to quell gcc compiler warning
		length = (int)strlen( input );

		if ( ( length < 1 ) || ( length > max_bytes ) ) input_error = TRUE;
		else
		{
			for ( i = 0; i < length; i++ )
			{
				input[i] = toupper(input[i]);

				if ( ( input[i] >= '0' ) && ( input[i] <= '9' ) )
				{
					result += ( ( input[i] - '0' ) << ( (length - i - 1) * 4 ) );
				}
				else if ( ( input[i] >= 'A' ) && ( input[i] <= 'F' ) )
				{
					result += ( ( input[i] - 'A' + 10 ) << ( (length - i - 1) * 4 ) );
				}
				else
				{
					input_error = TRUE;
				}
			}
		}

		if ( input_error )
		{
			printf( "Error - Please try again: " );
		}
	} while( input_error );

	return( result );
}

int main( int argc, char* argv[] )
{
	void            *xBoard;
	Authenticate	x15Authenticate;
	UINT 			read_address, read_bytes, write_address, write_bytes, index;
	BYTE 			data[MEMORY_CAPACITY]={0};						/* Create buffer for data */
	BYTE 			byte_to_write;
	BOOL 			addressing_error;
	BYTE 			fittedBoard;
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
			else
			{
				addressing_error = FALSE;
			}

			if ( addressing_error )
			{
				printf( "Please try again.\n\n" );
			}
		} while ( addressing_error );

		printf ( "\nReading %Xh bytes from address %Xhh ... ", read_bytes, read_address );

		/* Read data */
		if ( !XlineReadEEPROM( xBoard, read_address, (LPBYTE) &data, read_bytes ) )
		{
			return( XlineExit( "failed.", xBoard, &x15Authenticate, 1 ) );
		}
		printf( "success.\n" );

		for ( index = 0; index < read_bytes; index++ )
		{
			if ( !(( index + read_address ) % 16 ) || ( index == 0 ) )
			{
				printf( "\n0x%04X:\t", index+read_address );
			}
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

			if ( addressing_error )
			{
				printf( "Please try again." );
			}
		} while ( addressing_error );

		/* Update data to write */
		for ( index = 0; index < write_bytes; index++ )
		{
			data[index] = byte_to_write;
		}

		/* Write data */
		printf( "\n\nWriting %Xh bytes of '%02Xh' to address %Xh ... \n", write_bytes, byte_to_write, write_address );
		if ( !XlineWriteEEPROM( xBoard, write_address, (LPBYTE) &data, write_bytes ) )
		{
			return( XlineExit( "failed.", xBoard, &x15Authenticate, 1 ) );
		}
		printf( "success.\n" );

		/* Generate array and print to the screen */
		for ( index = 0; index < write_bytes; index++ )
		{
			if ( !(( index + write_address ) % 16 ) || ( index == 0 ) )
			{
				printf( "\n0x%04X:\t", index + write_address );
			}

			printf( "%02X ", data[index] & 0x00FF );
		}

		/* Wait for keystroke.*/
		printf( "\nPress any key to continue or 'C' to close...\n\n" );
		key = getch();
		if ( ( key == 'C' ) || ( key == 'c' ) ) break;
	}

	return( XlineExit( "Leaving program.", xBoard, &x15Authenticate, 0 ) );
}
