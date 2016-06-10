/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         spidemo.c
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
*     1.0 JSH       First properly released version using true C code and C-API, 11/10/2011.
*
*****************************************************************/

#include <stdio.h>
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

int main( int argc, char* argv[] )
{
	void            *xBoard;
	Authenticate	x15Authenticate;
	BYTE 			fittedBoard;
	BYTE 			SPI_message[50], receive_message[50], counter, i, index;
	char 			key;

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "This program demonstrates the SPI standard.\n\n" );

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

	if ( !XlineEnableSPI( xBoard ) ) return( XlineExit( "Failure enabling SPI.", xBoard, &x15Authenticate, 1 ) );

	#ifdef X10_LINUX_BUILD
		InitialiseConsole( );
	#endif

	counter = '0';
	i = 0;
	while ( TRUE )
	{
		sprintf( (char *)SPI_message, "HEBER %c", counter );

		printf( "Message to display on SEC device: '%s'\n", SPI_message );

		if ( !XlineSendSEC( xBoard, 0x40, i, 7, SPI_message, 15, 0, receive_message ) )
		{
			return( XlineExit( "Failure sending SPI message.", xBoard, &x15Authenticate, 1 ) );
		}

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

	return( XlineExit( "Leaving program.", xBoard, &x15Authenticate, 0 ) );
}
