/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         reels.c
*
*     Version:      1.0
*
*     Description:  Reel Demonstration
*
*     Notes:        Console application to demonstrate reels
*
*					Application prints out detailed information
*					about its operation on startup.
*
*     Changes:
*     1.0 MJB       First version using C API, 11/1/2008.
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

/* Some example ramp tables */
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

void DisplayReelStatus( usbReelStatus * status )
{
	printf( "\n\nReel status report:\n" );
	printf( "   Reel num.:      1     2     3\n" );
	printf( "   Position:     %3d   %3d   %3d\n",
		status->byPosition[0], status->byPosition[1], status->byPosition[2] );
	printf( "   Error:        %3d   %3d   %3d\n",
		status->byError[0], status->byError[1], status->byError[2] );
	printf( "   Busy:         %3d   %3d   %3d\n",
		status->byBusy[0], status->byBusy[1], status->byBusy[2] );
	printf( "   Synchronised: %3d   %3d   %3d\n\n",
		status->bySynchronised[0], status->bySynchronised[1], status->bySynchronised[2] );
}

int main( int argc, char* argv[] )
{
	void			*xBoard;
	Authenticate	x15Authenticate;
	BYTE 			byDir = 2;
	char 			key;
	usbReelStatus 	status;
	BYTE 			fittedBoard;

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "This demonstrates reel support.\n\n" );
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

	printf( "Configuring reels..." );
	if ( !XlineConfigureReels( xBoard, 3, 96, 0 ) )
	{
		printf( "Failure %d configuring reels.\n", (int)XlineGetLastError( xBoard ) );
	}
	printf( "success.\n" );

	printf( "Configuring ramp up..." );
	if ( !XlineSpinRampUp( xBoard, 0, RampUp114rpm ) )
	{
		return( XlineExit( "Failure configuring ramp up.", xBoard, &x15Authenticate, 1 ) );
	}
	if ( !XlineSpinRampUp( xBoard, 1, RampUp114rpm ) )
	{
		return( XlineExit( "Failure configuring ramp up.", xBoard, &x15Authenticate, 1 ) );
	}
	if ( !XlineSpinRampUp( xBoard, 2, RampUp114rpm ) )
	{
		return( XlineExit( "Failure configuring ramp up.", xBoard, &x15Authenticate, 1 ) );
	}
	printf( "success.\n" );

	printf( "Configuring ramp down..." );
	if ( !XlineSpinRampDown( xBoard, 0, RampDown114rpm ) )
	{
		return( XlineExit( "Failure configuring ramp down.", xBoard, &x15Authenticate, 1 ) );
	}
	if ( !XlineSpinRampDown( xBoard, 1, RampDown114rpm ) )
	{
		return( XlineExit( "Failure configuring ramp down.", xBoard, &x15Authenticate, 1 ) );
	}
	if ( !XlineSpinRampDown( xBoard, 2, RampDown114rpm ) )
	{
		return( XlineExit( "Failure configuring ramp down.", xBoard, &x15Authenticate, 1 ) );
	}
	printf( "success.\n" );

	printf( "Synchronising reel..." );
	if ( !XlineReelSynchroniseEnable( xBoard, 0 ) )
	{
		return( XlineExit( "Failure configuring ramp down.", xBoard, &x15Authenticate, 1 ) );
	}
	if ( !XlineReelSynchroniseEnable( xBoard, 1 ) )
	{
		return( XlineExit( "Failure configuring ramp down.", xBoard, &x15Authenticate, 1 ) );
	}
	if ( !XlineReelSynchroniseEnable( xBoard, 2 ) )
	{
		return( XlineExit( "Failure configuring ramp down.", xBoard, &x15Authenticate, 1 ) );
	}
	printf( "success.\n\n" );

	printf( "Press any key to spin reels, 'D' to display reel diagnostics or 'C' to exit.\n" );

	#ifdef X10_LINUX_BUILD
		InitialiseConsole( );
	#endif

	while ( TRUE )
	{
		if ( !XlineSpinReels( xBoard, 0, byDir, 48 ) )
		{
			return( XlineExit( "Failure spinging reels.", xBoard, &x15Authenticate, 1 ) );
		}
		if ( !XlineSpinReels( xBoard, 1, -byDir, 48 ) )
		{
			return( XlineExit( "Failure spinging reels.", xBoard, &x15Authenticate, 1 ) );
		}
		if ( !XlineSpinReels( xBoard, 2, byDir, 48 ) )
		{
			return( XlineExit( "Failure spinging reels.", xBoard, &x15Authenticate, 1 ) );
		}

		byDir = 0 - byDir;

		do
		{
			if ( !XlineGetReelStatus( xBoard, &status ) )
			{
				return( XlineExit( "Failure obtaining reel status.", xBoard, &x15Authenticate, 1 ) );
			}

			key = getch();
			if ( ( key == 'C' ) || ( key == 'c' ) )
			{
				break;
			}
			else if ( ( key == 'D' ) || ( key == 'd' ) )
			{
				DisplayReelStatus( &status );
			}
		} while ( ( status.byBusy[0] != 0 ) || ( status.byBusy[1] != 0 ) || ( status.byBusy[2] != 0 ) );

		if ( key == 'C' || key == 'c' ) break;
	}

	return( XlineExit( "Leaving program.", xBoard, &x15Authenticate, 0 ) );
}
