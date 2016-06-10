/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         timedemo.cpp
*
*     Version:      1.1
*
*     Description:  Real-time Clock Demonstration
*
*     Notes:        Console application to demonstrate real time clock
*
*
*     Changes:
*
*	  1.1 RDP		Added support for X15 product 10/10/2007
*     1.0 MJB       First version. Based on issue 1.2
*                   of X10 timedemo.cpp, 29/7/2005
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

int main( int argc, char* argv[] )
{
	FireFlyUSB 		FireFly;
	Authenticate	X15Authenticate;
	BYTE 			fittedBoard;
	BYTE 			VersionProduct[20]={0}, VersionDll[20]={0}, Version8051[20]={0}, VersionPIC[20]={0};
	BYTE 			SerialNumberPIC[20]={0};
	BYTE 			open_switches=0, closed_switches=0;

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "This application demonstrates the real time clock on the security\n" );
	printf( "microcontroller..\n\n" );
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

	if ( !FireFly.GetProductVersion( VersionProduct ) )
		return( ExitFunction( "Failure obtaining product version.", &FireFly, &X15Authenticate, 1 ) );

	if ( !FireFly.GetDllVersion( VersionDll ) )
		return( ExitFunction( "Failure obtaining DLL version.", &FireFly, &X15Authenticate, 1 ) );


	if ( !FireFly.Get8051Version( Version8051 ) )
		return( ExitFunction( "Failure obtaining 8051 version.", &FireFly, &X15Authenticate, 1 ) );

	if ( !FireFly.GetPICVersion( VersionPIC ) )
		return( ExitFunction( "Failure obtaining PIC version.", &FireFly, &X15Authenticate, 1 ) );

	if ( !FireFly.GetPICSerialNumber( SerialNumberPIC ) )
		return( ExitFunction( "Failure obtaining PIC serial number.", &FireFly, &X15Authenticate, 1 ) );

	printf( "Product version information:\n" );
	printf( "    Product version       : %s\n",   VersionProduct );
	printf( "    API library version   : %s\n",   VersionDll );
	printf( "    8051 software version : %s\n",   Version8051 );
	printf( "    PIC software version  : %s\n",   VersionPIC );
	printf( "    PIC serial number     : %s\n\n", SerialNumberPIC );

	if ( !FireFly.ReadAndResetSecuritySwitchFlags( &closed_switches, &open_switches ) )
		return( ExitFunction( "ReadAndResetSecuritySwitchFlags failed.", &FireFly, &X15Authenticate, 1 ) );

	printf( "Switch flags: Closed = %02X, Open = %02X\n", closed_switches, open_switches );

	// Set the time only once
	DWORD time = 0x01020304;

	printf( "Setting time to 0x%04X%04X...\n", (int)((time >> 16) & 0x0000FFFF), (int)(time & 0x0000FFFF) );
	if ( !FireFly.SetClock( time ) )
		return( ExitFunction( "Set time failed.", &FireFly, &X15Authenticate, 1 ) );

	printf( "\nPress any key to get the time or 'C' to close...\n" );

	#ifdef X10_LINUX_BUILD
		InitialiseConsole( );
		EnableGetchNewline( );
	#endif

	while ( TRUE )
	{
		if ( !FireFly.GetClock( &time ) )
			return( ExitFunction( "Get time failed.", &FireFly, &X15Authenticate, 1 ) );

		printf( "\rTime: 0x%04X%04X", (int)((time >> 16) & 0x0000FFFF), (int)(time & 0x0000FFFF) );

		/* Wait for keystroke. */
		char key = getch( );

		if ( key == 'C' || key == 'c' ) break;

	}

	return( ExitFunction( "Leaving program.", &FireFly, &X15Authenticate, 0 ) );
}
