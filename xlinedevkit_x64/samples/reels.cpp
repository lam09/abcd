/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         reels.cpp
*
*     Version:      1.1
*
*     Description:  Reel Demonstration
*
*     Notes:        Console application to demonstrate reels
*
*					Application prints out detailed information
*					about its operation on startup.
*
*     Changes:
*
*	  1.1 RDP		Added support for X15 product 10/10/2007
*     1.0 MJB       First version. Based on issue 1.0 of
*                   X10 reels.cpp, 29/7/2005
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


// Some example ramp tables
usbRampTable RampUp20rpmHS   = { 4, { 100, 60, 37, 28 } };
usbRampTable RampDown20rpmHS = { 3, { 23,  60, 200 } };
usbRampTable RampUp30rpmHS   = { 5, { 100, 57, 30, 25, 21 } };
usbRampTable RampDown30rpmHS = { 4, { 23,  23, 58, 200 } };
usbRampTable RampUp40rpm     = { 4, { 100, 55, 29, 31 } };
usbRampTable RampDown40rpm   = { 3, { 22,  55, 200 } };
usbRampTable RampUp50rpm     = { 4, { 100, 50, 26, 25 } };
usbRampTable RampDown50rpm   = { 3, { 20,  53, 200 } };
usbRampTable RampUp60rpm     = { 4, { 100, 44, 25, 21 } };
usbRampTable RampDown60rpm   = { 4, { 24,  18, 48, 200 } };
usbRampTable RampUp70rpm     = { 5, { 100, 41, 24, 20, 18 } };
usbRampTable RampDown70rpm   = { 4, { 23,  16, 48, 200 } };
usbRampTable RampUp80rpm     = { 6, { 100, 39, 23, 19, 17, 16 } };
usbRampTable RampDown80rpm   = { 4, { 22,  16, 46, 200 } };
usbRampTable RampUp90rpm     = { 7, { 100, 39, 22, 18, 16, 15, 14 } };
usbRampTable RampDown90rpm   = { 4, { 22,  13, 45, 200 } };
usbRampTable RampUp96rpm     = { 7, { 100, 37, 24, 13, 18, 15, 13 } };
usbRampTable RampDown96rpm   = { 4, { 22,  13, 41, 200 } };
usbRampTable RampUp104rpm    = { 7, { 100, 36, 23, 13, 17, 15, 12 } };
usbRampTable RampDown104rpm  = { 4, { 22,  14, 39, 200 } };
usbRampTable RampUp114rpm    = { 7, { 100, 35, 19, 14, 16, 16, 11 } };
usbRampTable RampDown114rpm  = { 4, { 22,  14, 35, 200 } };
usbRampTable RampUpNudge     = { 3, { 100, 35, 19 } };
usbRampTable RampDownNudge   = { 3, { 22,  14, 35 } };

void DisplayReelStatus( ReelStatusEx * status )
{
	printf( "\n\nReel status report:\n" );
	printf( "   Reel num.:      1     2     3\n" );
	printf( "   Position:     %3d   %3d   %3d\n",
		status->position[0], status->position[1], status->position[2] );
	printf( "   Error:        %3d   %3d   %3d\n",
		status->error[0], status->error[1], status->error[2] );
	printf( "   Busy:         %3d   %3d   %3d\n",
		status->busy[0], status->busy[1], status->busy[2] );
	printf( "   Synchronised: %3d   %3d   %3d\n\n",
		status->synchronised[0], status->synchronised[1], status->synchronised[2] );
}

int main(int argc, char* argv[])
{
	FireFlyUSB 		FireFly;
	Authenticate	X15Authenticate;
	BYTE 			byDir = 2;
	char 			key;
	ReelStatusEx 	status;
	BYTE 			fittedBoard;

	printf( "Firefly X10i/X15 Board\n" );
	printf( "============++++======\n\n" );
	printf( "This demonstrates reel support.\n\n" );
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

	printf( "Configuring reels..." );
	if ( !FireFly.ConfigureReels( 3, 96, 0 ) ) {
		printf( "Failure %d configuring reels.\n", (int)FireFly.GetLastError( ) );
	}
	printf( "success.\n" );

	printf( "Configuring ramp up..." );
	if ( !FireFly.SpinRampUp( 0, RampUp114rpm ) )
		return( ExitFunction( "Failure configuring ramp up.", &FireFly, &X15Authenticate, 1 ) );
	if ( !FireFly.SpinRampUp( 1, RampUp114rpm ) )
		return( ExitFunction( "Failure configuring ramp up.", &FireFly, &X15Authenticate, 1 ) );
	if ( !FireFly.SpinRampUp( 2, RampUp114rpm ) )
		return( ExitFunction( "Failure configuring ramp up.", &FireFly, &X15Authenticate, 1 ) );
	printf( "success.\n" );

	printf( "Configuring ramp down..." );
	if ( !FireFly.SpinRampDown( 0, RampDown114rpm ) )
		return( ExitFunction( "Failure configuring ramp down.", &FireFly, &X15Authenticate, 1 ) );
	if ( !FireFly.SpinRampDown( 1, RampDown114rpm ) )
		return( ExitFunction( "Failure configuring ramp down.", &FireFly, &X15Authenticate, 1 ) );
	if ( !FireFly.SpinRampDown( 2, RampDown114rpm ) )
		return( ExitFunction( "Failure configuring ramp down.", &FireFly, &X15Authenticate, 1 ) );
	printf( "success.\n" );

	printf( "Synchronising reel..." );
	if ( !FireFly.ReelSynchroniseEnable( 0 ) )
		return( ExitFunction( "Failure configuring ramp down.", &FireFly, &X15Authenticate, 1 ) );
	if ( !FireFly.ReelSynchroniseEnable( 1 ) )
		return( ExitFunction( "Failure configuring ramp down.", &FireFly, &X15Authenticate, 1 ) );
	if ( !FireFly.ReelSynchroniseEnable( 2 ) )
		return( ExitFunction( "Failure configuring ramp down.", &FireFly, &X15Authenticate, 1 ) );
	printf( "success.\n\n" );

	printf( "Press any key to spin reels, 'D' to display reel diagnostics or 'C' to exit.\n" );

	#ifdef X10_LINUX_BUILD
	InitialiseConsole( );
	#endif

	while ( TRUE )
	{
		if ( !FireFly.SpinReels( 0, byDir, 48 ) )
			return( ExitFunction( "Failure spinging reels.", &FireFly, &X15Authenticate, 1 ) );
		if ( !FireFly.SpinReels( 1, -byDir, 48 ) )
			return( ExitFunction( "Failure spinging reels.", &FireFly, &X15Authenticate, 1 ) );
		if ( !FireFly.SpinReels( 2, byDir, 48 ) )
			return( ExitFunction( "Failure spinging reels.", &FireFly, &X15Authenticate, 1 ) );

		byDir = 0 - byDir;

		do
		{
			key = getch();
			if ( key == 'C' || key == 'c' ) break;

			if ( !FireFly.GetReelStatusEx( &status ) )
				return( ExitFunction( "Failure obtaining reel status.", &FireFly, &X15Authenticate, 1 ) );

			if ( key == 'D' || key == 'd' ) DisplayReelStatus( &status );
		} while ( ( status.busy[0] != 0 ) || ( status.busy[1] != 0 ) ||
			      ( status.busy[2] != 0 ) );

		if ( key == 'C' || key == 'c' ) break;
	}

	return( ExitFunction( "Leaving program.", &FireFly, &X15Authenticate, 0 ) );
}
