/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         spidemo.cpp
*
*     Version:      1.1
*
*     Description:  SPI Demonstration
*
*     Notes:        Console application to demonstrate SPI standard
*
*     Changes:
*
*	  1.1 RDP		Added support for X15 product 10/10/2007
*     1.0 MJB       First version. Based on issue 1.0 of X10
*                   spidemo.cpp, 29/7/2005
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

int main(int argc, char* argv[])
{
	FireFlyUSB 		FireFly;
	Authenticate	X15Authenticate;
	BYTE 			fittedBoard;

	BYTE 			SPI_message[50], receive_message[50], counter, i, index;
	char 			key;

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "This program demonstrates the SPI standard.\n\n" );

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

	if ( !FireFly.EnableSPI( ) ) return( ExitFunction( "Failure enabling SPI.", &FireFly, &X15Authenticate, 1 ) );

	#ifdef X10_LINUX_BUILD
	InitialiseConsole( );
	#endif

	counter = '0';
	i = 0;
	while ( TRUE )
	{
		sprintf( (char *)SPI_message, "HEBER %c", counter );

		printf( "Message to display on SEC device: '%s'\n", SPI_message );

		if ( !FireFly.SendSEC( 0x40, i, 7, SPI_message, 15, 0, receive_message ) )
			return( ExitFunction( "Failure sending SPI message.", &FireFly, &X15Authenticate, 1 ) );

		printf( "SPI Received message = [ " );
		for ( index = 0; index < 4; index++ ) printf( "%02X ", receive_message[index] );
		printf( "]\n" );

		counter++;
		i++;
		if ( counter > '9' ) counter = '0';

		printf( "\n\nPress any key to update display or 'C' to close.\n" );

		key = getch();
		if ( key == 'C' || key == 'c' ) break;
	}

	return( ExitFunction( "Leaving program.", &FireFly, &X15Authenticate, 0 ) );
}
