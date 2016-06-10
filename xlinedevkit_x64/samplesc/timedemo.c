/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         timedemo.c
*
*     Version:      1.0
*
*     Description:  Real-time Clock Demonstration
*
*     Notes:        Console application to demonstrate real time clock
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

int main( int argc, char* argv[] )
{
	void            *xBoard;
	Authenticate	x15Authenticate;
	BYTE 			fittedBoard;
	BYTE 			VersionProduct[20]={0}, VersionDll[20]={0}, Version8051[20]={0}, VersionPIC[20]={0};
	BYTE 			SerialNumberPIC[20]={0};
	BYTE 			open_switches=0, closed_switches=0;
	DWORD			time;
	char 			key;

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "This application demonstrates the real time clock on the security\n" );
	printf( "microcontroller..\n\n" );
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

	if ( !XlineGetProductVersion( xBoard, VersionProduct ) )
	{
		return( XlineExit( "Failure obtaining product version.", xBoard, &x15Authenticate, 1 ) );
	}

	if ( !XlineGetDllVersion( xBoard, VersionDll ) )
	{
		return( XlineExit( "Failure obtaining DLL version.", xBoard, &x15Authenticate, 1 ) );
	}

	if ( !XlineGet8051Version( xBoard, Version8051 ) )
	{
		return( XlineExit( "Failure obtaining 8051 version.", xBoard, &x15Authenticate, 1 ) );
	}

	if ( !XlineGetPICVersion( xBoard, VersionPIC ) )
	{
		return( XlineExit( "Failure obtaining PIC version.", xBoard, &x15Authenticate, 1 ) );
	}

	if ( !XlineGetPICSerialNumber( xBoard, SerialNumberPIC ) )
	{
		return( XlineExit( "Failure obtaining PIC serial number.", xBoard, &x15Authenticate, 1 ) );
	}

	printf( "Product version information:\n" );
	printf( "    Product version       : %s\n",   VersionProduct );
	printf( "    API library version   : %s\n",   VersionDll );
	printf( "    8051 software version : %s\n",   Version8051 );
	printf( "    PIC software version  : %s\n",   VersionPIC );
	printf( "    PIC serial number     : %s\n\n", SerialNumberPIC );

	if ( !XlineReadAndResetSecuritySwitchFlags( xBoard, &closed_switches, &open_switches ) )
	{
		return( XlineExit( "ReadAndResetSecuritySwitchFlags failed.", xBoard, &x15Authenticate, 1 ) );
	}

	printf( "Switch flags: Closed = %02X, Open = %02X\n", closed_switches, open_switches );

	/* Set the time only once */
	time = 0x01020304;

	printf( "Setting time to 0x%04X%04X...\n", (int)((time >> 16) & 0x0000FFFF), (int)(time & 0x0000FFFF) );
	if ( !XlineSetClock( xBoard, time ) )
	{
		return( XlineExit( "Set time failed.", xBoard, &x15Authenticate, 1 ) );
	}

	printf( "\nPress any key to read the time or 'C' to close...\n" );

	#ifdef X10_LINUX_BUILD
		InitialiseConsole( );
		EnableGetchNewline( );
	#endif

	while ( TRUE )
	{
		if ( !XlineGetClock( xBoard, &time ) )
		{
			return( XlineExit( "Get time failed.", xBoard, &x15Authenticate, 1 ) );
		}

		printf( "\rTime: 0x%04X%04X", (int)((time >> 16) & 0x0000FFFF), (int)(time & 0x0000FFFF) );

		/* Wait for keystroke. */
		key = getch( );
		if ( ( key == 'C' ) || ( key == 'c' ) )
		{
			break;
		}

	}

	return( XlineExit( "Leaving program.", xBoard, &x15Authenticate, 0 ) );
}
