/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         fadedemo.c
*
*     Version:      1.0
*
*     Description:  IO Demonstration
*
*     Notes:        Console application to demonstrate IO
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
	#include <unistd.h>									/* for usleep() */
	#include "linuxkb.h"
	#define SleepInMS(time) usleep( time * 1000 )		/* Linux usleep is in microseconds */
	#define TRUE 1						/* needed for 'C' */
	#define FALSE 0
#else
	/* WIN32 specific build */
	#include <conio.h>
	#define SleepInMS(time) Sleep(time)

	/* Remove "scanf" and "getch" warnings when compiling using Visual V++ 2005 */
	#pragma warning( disable : 4996 )
#endif

#include "xlinecapi.h"
#include "xlineauthenticate.h"
#include "unlockioc.h"
#include "xlinecexit.h"
#include "x15key.h"

/* Boolean directions */
#define BRIGHTER		TRUE
#define DIMMER			FALSE

#define BASIC_DELAY  10			/* units of 10mS */

int main( int argc, char* argv[] )
{
	void 			*xBoard;
	Authenticate	x15Authenticate;
	usbOutput 		OnPeriods, OffPeriods;
	usbInput 		speedSetting;
	BYTE 			byBrightness;
	BYTE 			fittedBoard;
	BOOL 			bDirection;
	int  			speed_mult;
	int 			i;

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "This program demonstates IO lamp fading. Use switches IP16-IP23 to adjust speed.\n" );
	printf( "Press  any key  to exit program.\n\n" );
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
	if ( !XlineConfigureReels( xBoard, 0, 0, 0 ) )
	{
		printf( "Failure %d configuring reels.\n", (int)XlineGetLastError( xBoard ) );
	}

	for ( i = 0; i < 4; i++ )
	{
		OffPeriods.byOut[i] = 0x00;
		OnPeriods.byOut[i] = 0x00;
	}
	OnPeriods.byOut[1] = 0x01;
	OnPeriods.byAux    = 0x00;
	OffPeriods.byAux   = 0x00;

	XlineSetOffPeriodOutputs( xBoard, OffPeriods );
	XlineSetOnPeriodOutputs( xBoard, OnPeriods );

	byBrightness = 0;
	bDirection = BRIGHTER;

	#ifdef X10_LINUX_BUILD
		InitialiseConsole( );
		EnableGetchNewline( );
	#endif

	while ( TRUE )
	{
		XlineSetOutputBrightness( xBoard, byBrightness );

		XlineGetInputs( xBoard, &speedSetting );
		speed_mult = 0;
		for ( i = 0x01; i <= 0x80; i *= 2 )
		{
			if ( ( ~speedSetting.byIn[2] & 0xff ) & i )
			{
				speed_mult++;
			}
		}

		printf( "Speed = [" );
		for ( i = 0; i < speed_mult; i++ )
		{
			printf( ">" );
		}
		for ( i = speed_mult; i<8; i++ )
		{
			printf( " " );
		}
		printf( "]\r" );

		#ifdef X10_LINUX_BUILD
			fflush( stdout );								/* normally '\n' used to flush buffer */
		#endif

		/* Wait for suitable delay */
		SleepInMS( BASIC_DELAY * (9 - speed_mult) );		/* multi-process/thread friendly delay */

		/* Upon keystroke, any key exits program. */
		if ( _kbhit() ) if ( toupper( getch() ) ) break;

		if ( bDirection == BRIGHTER ) byBrightness++;
		else byBrightness--;

		if ( ( ( byBrightness == 11  ) && ( bDirection == BRIGHTER ) ) ||
			 ( ( byBrightness == 255 ) && ( bDirection == DIMMER ) ) )
		{
			if ( bDirection == BRIGHTER )
			{
				byBrightness = 0;
				if ( OffPeriods.byOut[3] == 0xff )
				{
					OffPeriods.byOut[1] = 0xfe;
					byBrightness = 10;
					bDirection = DIMMER;
				}
				else for ( i = 1; i < 4; i++ )
				{
					OffPeriods.byOut[i] = OnPeriods.byOut[i];

					if ( ( i > 1 ) && ( OffPeriods.byOut[i-1] == 0xff ) && ( OnPeriods.byOut[i] == 0x00 ) )
					{
						OnPeriods.byOut[i] = 0x01;
					}
					else if ( OnPeriods.byOut[i] != 0xff )
					{
						XlineSetOffPeriodOutputs( xBoard, OffPeriods );
						OnPeriods.byOut[i] = OnPeriods.byOut[i] | ( OnPeriods.byOut[i] * 2 );
					}
				}

				XlineSetOutputBrightness( xBoard, byBrightness );
			}
			else if ( bDirection == DIMMER )
			{
				byBrightness = 10;
				if ( OnPeriods.byOut[3] == 0x00 )
				{
					OnPeriods.byOut[1] = 0x01;
					byBrightness = 0;
					bDirection = BRIGHTER;
				}
				else for ( i = 1; i < 4; i++ )
				{
					OnPeriods.byOut[i] = OffPeriods.byOut[i];

					if ( ( i > 1 ) && ( OffPeriods.byOut[i-1] == 0x00 ) && ( OnPeriods.byOut[i-1] == 0x00 ) && ( OffPeriods.byOut[i] == 0xff ) )
					{
						OffPeriods.byOut[i] = 0xfe;
					}

					else if ( OffPeriods.byOut[i] != 0xff )
					{
						XlineSetOnPeriodOutputs( xBoard, OnPeriods );
						OffPeriods.byOut[i] = OffPeriods.byOut[i] << 1;
					}
				}
				XlineSetOutputBrightness( xBoard, byBrightness );
			}

			XlineSetOffPeriodOutputs( xBoard, OffPeriods );
			XlineSetOnPeriodOutputs( xBoard, OnPeriods );
		}
	}

	return ( XlineExit( "Leaving program.", xBoard, &x15Authenticate, 0 ) );
}
