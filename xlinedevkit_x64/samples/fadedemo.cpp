/*****************************************************************
*
*     Project:      Firefly X10i Board
*
*     File:         fadedemo.cpp
*
*     Version:      1.1
*
*     Description:  IO Demonstration
*
*     Notes:        Console application to demonstrate IO
*
*					Application prints out detailed information
*					about its operation on startup.
*
*     Changes:
*
*	  1.1 RDP		Added support for X15 product 10/10/2007
*     1.0 MJB       First version. Based on issue 1.0 of
*                   X10 fadedemo.cpp, 28/7/2005
*
*****************************************************************/

#include <stdio.h>
#include <time.h>

#ifndef X10_LINUX_BUILD
/* WIN32 specific build */
	#include <conio.h>
	#define SleepInMS(time) Sleep(time)
	#include "authenticate_win.h"

/* Remove "scanf" and "getch" warnings when compiling using Visual V++ 2005 */
	#pragma warning( disable : 4996 )
#else
/* LINUX specific build */
	#include <ctype.h>
	#include "linuxkb.c"
	#include "authenticate_linux.h"
	#define SleepInMS(time) usleep( time * 1000 )		// Linux usleep is in microseconds
#endif

#include "fflyusb.h"
#include "unlockio.h"
#include "x10errhd.c"
#include "x15key.h"

// Boolean directions
#define BRIGHTER		TRUE
#define DIMMER			FALSE

/* #define BASE_TIME		0.009 */

#define BASIC_DELAY  10			/* units of 10mS */

/*
void delay( double delay_time )
{
	time_t start, finish;
	double elapsed_time;

	start = clock( );
	do
	{
		finish = clock( );

		elapsed_time = (double)(finish - start) / CLOCKS_PER_SEC;
	} while ( elapsed_time < delay_time );
}
*/

int main( int argc, char* argv[] )
{
	FireFlyUSB 		FireFly;
	Authenticate	X15Authenticate;
	usbOutput 		OnPeriods, OffPeriods;
	usbInput 		speedSetting;
	BYTE 			byBrightness;
	BYTE 			fittedBoard;
	BOOL 			bDirection;
	/*	double 			speed_mult; */
	int  speed_mult;
	int 			i;

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "This program demonstates IO lamp fading. Use switches to adjust speed.\n" );
	printf( "Press  any key  to exit program.\n\n" );
	printf( "Establishing link with FireFly USB device..." );

	#ifdef X10_LINUX_BUILD
		InitialiseConsole( );
		EnableGetchNewline( );
	#endif

	/* initialise the firefly device */
	/* Get a handle to the device */
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
	if ( !FireFly.ConfigureReels( 0, 0, 0 ) )
	{
		printf( "Failure %d configuring reels.\n", (int)FireFly.GetLastError( ) );
	}

	for ( i = 0; i < 4; i++ )
	{
		OffPeriods.byOut[i] = 0x00;
		OnPeriods.byOut[i] = 0x00;
	}

	OnPeriods.byOut[1] = 0x01;
	OnPeriods.byAux    = 0x00;
	OffPeriods.byAux   = 0x00;

	FireFly.SetOffPeriodOutputs( OffPeriods );
	FireFly.SetOnPeriodOutputs( OnPeriods );

	byBrightness = 0;
	bDirection = BRIGHTER;


    


    FireFly.SetOutputBrightness( byBrightness );
    SleepInMS(5000);









	while ( TRUE )
	{
        break;
		FireFly.SetOutputBrightness( byBrightness );

		FireFly.GetInputs( &speedSetting );
		speed_mult = 0;
		for ( i=0x01; i<=0x80; i*=2 )
		{
			if ( ( ~speedSetting.byIn[2] & 0xff ) & i )
				speed_mult++;
		}

		printf( "Speed = [" );

		for ( i=0; i<(int)speed_mult; i++ )
			printf( ">" );

		for ( i=(int)speed_mult; i<8; i++ )
			printf( " " );

		printf( "]\r" );

		/* Wait for suitable delay -- and greatly reduces CPU load */
		/* delay( BASE_TIME * (9 - speed_mult) );*/			/* leaves cpu in processor intensive spin -- use Sleep() instead */

		SleepInMS( BASIC_DELAY * (9 - speed_mult) );		/* multi-process/thread friendly delay */

		/* check for keystroke, any key exits program. */
		if ( _kbhit() )
		{
			if ( toupper( getch() ) )
				break;
		}

		if ( bDirection == BRIGHTER )
			byBrightness++;
		else
			byBrightness--;

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
						OnPeriods.byOut[i] = 0x01;

					else if ( OnPeriods.byOut[i] != 0xff )
					{
						FireFly.SetOffPeriodOutputs( OffPeriods );
						OnPeriods.byOut[i] = OnPeriods.byOut[i] | ( OnPeriods.byOut[i] * 2 );
					}
				}

				FireFly.SetOutputBrightness( byBrightness );
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
						OffPeriods.byOut[i] = 0xfe;

					else if ( OffPeriods.byOut[i] != 0xff )
					{
						FireFly.SetOnPeriodOutputs( OnPeriods );
						OffPeriods.byOut[i] = OffPeriods.byOut[i] << 1;
					}
				}
				FireFly.SetOutputBrightness( byBrightness );
			}

			FireFly.SetOffPeriodOutputs( OffPeriods );
			FireFly.SetOnPeriodOutputs( OnPeriods );
		}
	}

	printf( "\n\nClosing device..." );
	if ( !FireFly.close( ) )
		printf( "error - %d.\n\n", (int)FireFly.GetLastError( ) );
	else
		printf( "success.\n\n" );

	if ( (int)FireFly.GetLastError( ) != USB_MESSAGE_EXECUTION_SUCCESS )
		return( ExitFunction( "Leaving program.", &FireFly, &X15Authenticate, 1 ));

	return ( ExitFunction( "Leaving program.", &FireFly, &X15Authenticate, 0 ) );
}
