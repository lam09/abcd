/*****************************************************************
*
*     Project:      Axis X15 Board
*
*     File:         authenticatedemo.c
*
*     Version:      1.1
*
*     Description:  Runs the X15 authentication thread
*
*     Changes:
*
*	  1.1 RDP		Changed for X15 product from X20 version 10/10/2007
*     1.0 MJB       First version, 1/10/2007
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
	char 			key;

	printf( "Axis X15 Board\n" );
	printf( "==============\n\n" );
	printf( "This application runs the X15 authentication thread.\n\n" );

	// Get a handle to the device
	printf( "Establishing link with the X15..." );
	if ( !FireFly.init( ) )
		return( ExitFunction( "Initialisation error.", &FireFly, &X15Authenticate, 1 ) );
	printf( "success.\n" );

/* Determine which board type is fitted, and unlock the IO security accordingly */
	FireFly.GetFittedBoard( &fittedBoard );
	if ( fittedBoard != X15_BOARD )
	{
		return( ExitFunction( "This demo can only be used with an X15.", &FireFly, &X15Authenticate, 1 ) );
	}

	// Start authentication thread - run every 90 seconds
	X15Authenticate.BeginThread( &FireFly, EncryptedKey, 90 );

	#ifdef X10_LINUX_BUILD
		InitialiseConsole( );
		EnableGetchNewline( );
	#endif

	while ( true )
	{
		printf( "Press 'C' to quit\n" );

		key = toupper( getch() );
		printf( "\n" );
		if ( key == 'C' ) break;
		printf( "Count = %d   \r", X15Authenticate.AuthenticationSuccessCount());
	}

	return( ExitFunction( "Leaving program.", &FireFly, &X15Authenticate, 0 ) );
}

