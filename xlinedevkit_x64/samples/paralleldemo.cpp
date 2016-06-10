/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         paralleldemo.cpp
*
*     Version:      1.1
*
*     Description:  Parallel IO Demonstration
*
*     Notes:        Console application to demonstrate parallel IO
*
*     Changes:
*
*	  1.1 RDP		Added support for X15 product 10/10/2007
*     1.0 MJB       First version. Based on issue 1.1 of X10
*                   paralleldemo.cpp, 28/7/2005
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


void TestInputPulseReading( FireFlyUSB * FireFly );
void TestHopperCoinRelease( FireFlyUSB * FireFly );

int main(int argc, char* argv[])
{
	FireFlyUSB 		FireFly;
	Authenticate	X15Authenticate;
	BYTE 			fittedBoard;
	char 			key;

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "This application demonstrates X10 Parallel IO.\n\n" );
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

	#ifdef X10_LINUX_BUILD
	InitialiseConsole( );
	EnableGetchNewline( );
	#endif

	while ( TRUE )
	{
		printf( "X10 Parallel I/O Menu\n" );
		printf( "=====================\n\n" );

		printf( "Options:\n" );
		printf( "    'P' to test input pulse reading.\n" );
		printf( "    'H' to test parallel hopper coin release.\n" );
		printf( "or, 'C' to exit.\n\n" );

		printf( "Please make a selection: " );

		key = toupper( getch() );
		printf( "\n\n" );
		if ( key == 'P' ) TestInputPulseReading( &FireFly );
		else if ( key == 'H' ) TestHopperCoinRelease( &FireFly );
		else if ( key == 'C' ) break;
	}

	return( ExitFunction( "Leaving program.", &FireFly, &X15Authenticate, 0 ) );
}

int GetNumber( const char displayString[], int lowValue, int highValue )
{
	char tempString[10];
	BOOL valueOK = FALSE;
	int value;
	#ifdef X10_LINUX_BUILD
		int ignore __attribute__ ((unused));
	#else
		int ignore;
	#endif

	#ifdef X10_LINUX_BUILD
	EnableEcho( );
	#endif

	while( !valueOK )
	{
		printf( "%s (%d-%d):", displayString, lowValue, highValue );
		ignore = scanf( "%s", tempString );				// ignore used to quell g++ compiler warning
		value = atoi( tempString );

		if ( ( value >= lowValue ) && ( value <= highValue ) ) valueOK = TRUE;
		else printf( "Incorrect value.\n\n" );
	}

	#ifdef X10_LINUX_BUILD
	DisableEcho( );
	#endif

	return( value );
}

void TestInputPulseReading( FireFlyUSB * FireFly )
{
	usbInputBitId inputNum;
	BYTE lowPulseTime, highPulseTime, inputCounterValue;
	ActiveState activeState = High;
	BOOL valueOK = FALSE;
	char key;

	inputNum = (usbInputBitId)GetNumber( "Please enter input number to read", 0, 23 );
	lowPulseTime = (BYTE)GetNumber( "Please enter input pulse lower time in ms", 0, 254 );
	highPulseTime = (BYTE)GetNumber( "Please enter input pulse upper time in ms", 0, 254 );
	while( !valueOK )
	{
		printf( "Is input active High, Low, Rising Edge, Falling Edge (H/L/R/F): " );
		key = getch();
		if ( key == 'H' || key == 'h' ) { activeState = High; valueOK = TRUE; }
		else if ( key == 'L' || key == 'l' ) { activeState = Low; valueOK = TRUE; }
		else if ( key == 'R' || key == 'r' ) { activeState = RisingEdge; valueOK = TRUE; }
		else if ( key == 'F' || key == 'f' ) { activeState = FallingEdge; valueOK = TRUE; }
		else printf( "Incorrect response.\n\n" );
	}

	if ( !FireFly->ConfigurePulsedInputEx( inputNum, lowPulseTime, highPulseTime, activeState ) )
	{
		printf( "ConfigurePulsedInput error - %d.\n\n", (int)FireFly->GetLastError( ) );
		return;
	}

	if ( !FireFly->BeginPulsedInputCheck( inputNum ) )
	{
		printf( "BeginPulsedInputCheck error - %d.\n\n", (int)FireFly->GetLastError( ) );
		return;
	}

	while ( TRUE )
	{
		printf( "\nPress: \n" );
		printf( "  'E' to reset input counter.\n" );
		printf( "  'D' to decrement input counter.\n" );
		printf( "  'R' to read input counter.\n" );
		printf( "  'C' to leave submenu.\n\n" );

		key = getch();
		if ( key == 'E' || key == 'e' )
		{
			printf( "Resetting input counter.\n" );
			if ( !FireFly->ResetPulsedInputCounter( inputNum ) )
			{
				printf( "ResetPulsedInputCounter error - %d.\n\n", (int)FireFly->GetLastError( ) );
				return;
			}
		}
		else if ( key == 'D' || key == 'd' )
		{
			printf( "Decrementing input counter.\n" );
			if ( !FireFly->DecrementPulsedInputCounter( inputNum ) )
			{
				printf( "DecrementPulsedInputCounter error - %d.\n\n", (int)FireFly->GetLastError( ) );
				return;
			}
		}
		else if ( key == 'R' || key == 'r' )
		{
			printf( "Reading input counter: " );
			if ( !FireFly->ReadPulsedInputCounter( inputNum, &inputCounterValue ) )
			{
				printf( "ReadPulsedInputCounter error - %d.\n\n", (int)FireFly->GetLastError( ) );
				return;
			}
			printf( "%d\n", inputCounterValue );
		}
		else if ( key == 'C' || key == 'c' ) break;
	}
}

void TestHopperCoinRelease( FireFlyUSB * FireFly )
{
	usbInputBitId inputNum;
	usbOutputBitId outputNum;
	BYTE lowPulseTime, highPulseTime, coinsToRelease, coinsReleased=0;
	WORD coinTimeout;
	ActiveState activeState = High;
	BOOL valueOK = FALSE;
	ReleaseHopperCoinsStatus status;
	char key;

	inputNum = (usbInputBitId)GetNumber( "Please enter input number to read", 0, 23 );
	lowPulseTime = (BYTE)GetNumber( "Please enter input pulse lower time in ms", 0, 254 );
	highPulseTime = (BYTE)GetNumber( "Please enter input pulse upper time in ms", 0, 254 );
	while( !valueOK )
	{
		printf( "Is input active High, Low, Rising Edge, Falling Edge (H/L/R/F): " );
		key = getch();
		if ( key == 'H' || key == 'h' ) { activeState = High; valueOK = TRUE; }
		else if ( key == 'L' || key == 'l' ) { activeState = Low; valueOK = TRUE; }
		else if ( key == 'R' || key == 'r' ) { activeState = RisingEdge; valueOK = TRUE; }
		else if ( key == 'F' || key == 'f' ) { activeState = FallingEdge; valueOK = TRUE; }
		else printf( "Incorrect response.\n\n" );
	}
	printf( "\n" );

	outputNum = (usbOutputBitId)GetNumber( "Please enter output number", 0, 31 );
	coinTimeout = (WORD)GetNumber( "Coin Timeout in ms", 1, 8000 );
	coinsToRelease = (BYTE)GetNumber( "Number of coins to release", 0, 255 );

	printf( "Clearing previous input pulse counter...\n" );
	if ( !FireFly->ResetPulsedInputCounter( inputNum ) )
	{
		printf( "ResetPulsedInputCounter error - %d.\n\n", (int)FireFly->GetLastError( ) );
		return;
	}

	while ( TRUE )
	{
		printf( "Press: \n" );
		printf( "  'R' to release parallel hopper coins.\n" );
		printf( "  'S' to get parallel hopper status.\n" );
		printf( "  'C' to leave submenu.\n\n" );

		key = getch();
		if ( key == 'R' || key == 'r' )
		{
			printf( "Releasing parallel hopper coins...\n" );
			if ( !FireFly->ReleaseParallelHopperCoinsEx( inputNum, lowPulseTime, highPulseTime, activeState,
				                                         outputNum, coinTimeout, coinsToRelease ) )
			{
				if ( FireFly->GetLastError( ) == USB_MESSAGE_PARALLEL_INPUT_CURRENTLY_ACTIVE )
				{
					printf( "The specified input is currently active. This must be set inactive\n" );
					printf( "prior to releasing coins.\n\n" );
				}
				else
				{
					printf( "ReleaseParallelHopperCoins error - %d.\n\n", (int)FireFly->GetLastError( ) );
					return;
				}
			}
		}
		else if ( key == 'S' || key == 's' )
		{
			printf( "Getting parallel hopper status..." );
			if ( !FireFly->GetParallelHopperStatus( inputNum, &status, &coinsReleased ) )
			{
				printf( "GetParallelHopperStatus error - %d.\n\n", (int)FireFly->GetLastError( ) );
	//			return;
			}

			if ( status == Running ) printf( "Running, %d released.\n", coinsReleased );
			else if ( status == Success ) printf( "Success, %d released.\n", coinsReleased );
			else if ( status == Failure ) printf( "Failure, %d released.\n", coinsReleased );
			else if ( status == InitJam ) printf( "InitJam, %d released.\n", coinsReleased );
			else if ( status == Jam ) printf( "Jam, %d released.\n", coinsReleased );
		}
		if ( key == 'C' || key == 'c' ) break;
	}
}
