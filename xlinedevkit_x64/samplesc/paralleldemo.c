/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         paralleldemo.c
*
*     Version:      1.0
*
*     Description:  Parallel IO Demonstration
*
*     Notes:        Console application to demonstrate parallel IO
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

void TestInputPulseReading( void *xBoard );
void TestHopperCoinRelease( void *xBoard );

int main(int argc, char* argv[])
{
	void			*xBoard;
	Authenticate	x15Authenticate;
	BYTE 			fittedBoard;
	char 			key;

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "This application demonstrations X10 Parallel IO.\n\n" );
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

	#ifdef X10_LINUX_BUILD
		InitialiseConsole( );
		EnableGetchNewline( );
	#endif

	while ( TRUE )
	{
		printf( "X-line Parallel I/O Menu\n" );
		printf( "========================\n\n" );

		printf( "Options:\n" );
		printf( "    'P' to test input pulse reading.\n" );
		printf( "    'H' to test parallel hopper coin release.\n" );
		printf( "or, 'C' to exit.\n\n" );

		printf( "Please make a selection: " );

		key = toupper( getch() );
		printf( "\n\n" );
		if ( key == 'P' ) TestInputPulseReading( xBoard );
		else if ( key == 'H' ) TestHopperCoinRelease( xBoard );
		else if ( key == 'C' ) break;
	}

	return( XlineExit( "Leaving program.", xBoard, &x15Authenticate, 0 ) );
}

int GetNumber( const char displayString[], int lowValue, int highValue )
{
	char tempString[10];
	BOOL valueOK = FALSE;
	int value;

	#ifdef X10_LINUX_BUILD
		int			ignore __attribute__ ((unused));
		EnableEcho( );
	#else
		int 		ignore;
	#endif

	while( !valueOK )
	{
		printf( "%s (%d-%d):", displayString, lowValue, highValue );
		ignore = scanf( "%s", tempString );				// ignore used to quell gcc compiler warning
		value = atoi( tempString );

		if ( ( value >= lowValue ) && ( value <= highValue ) ) valueOK = TRUE;
		else printf( "Incorrect value.\n\n" );
	}

	#ifdef X10_LINUX_BUILD
		DisableEcho( );
	#endif

	return( value );
}

void TestInputPulseReading( void *xBoard )
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

	if ( !XlineConfigurePulsedInputEx( xBoard, inputNum, lowPulseTime, highPulseTime, activeState ) )
	{
		printf( "ConfigurePulsedInput error - %d.\n\n", (int)XlineGetLastError( xBoard ) );
		return;
	}

	if ( !XlineBeginPulsedInputCheck( xBoard, inputNum ) )
	{
		printf( "BeginPulsedInputCheck error - %d.\n\n", (int)XlineGetLastError( xBoard ) );
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
		if ( ( key == 'E' ) || ( key == 'e' ) )
		{
			printf( "Resetting input counter.\n" );
			if ( !XlineResetPulsedInputCounter( xBoard, inputNum ) )
			{
				printf( "ResetPulsedInputCounter error - %d.\n\n", (int)XlineGetLastError( xBoard ) );
				return;
			}
		}
		else if ( ( key == 'D' ) || ( key == 'd' ) )
		{
			printf( "Decrementing input counter.\n" );
			if ( !XlineDecrementPulsedInputCounter( xBoard, inputNum ) )
			{
				printf( "DecrementPulsedInputCounter error - %d.\n\n", (int)XlineGetLastError( xBoard ) );
				return;
			}
		}
		else if ( ( key == 'R' ) || ( key == 'r' ) )
		{
			printf( "Reading input counter: " );
			if ( !XlineReadPulsedInputCounter( xBoard, inputNum, &inputCounterValue ) )
			{
				printf( "ReadPulsedInputCounter error - %d.\n\n", (int)XlineGetLastError( xBoard ) );
				return;
			}
			printf( "%d\n", inputCounterValue );
		}
		else if ( ( key == 'C' ) || ( key == 'c' ) )
		{
			break;
		}
	}
}

void TestHopperCoinRelease( void *xBoard )
{
	usbInputBitId inputNum;
	usbOutputBitId outputNum;
	BYTE lowPulseTime, highPulseTime, coinsToRelease, coinsReleased;
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
	if ( !XlineResetPulsedInputCounter( xBoard, inputNum ) )
	{
		printf( "ResetPulsedInputCounter error - %d.\n\n", (int)XlineGetLastError( xBoard ) );
		return;
	}

	while ( TRUE )
	{
		printf( "Press: \n" );
		printf( "  'R' to release parallel hopper coins.\n" );
		printf( "  'S' to get parallel hopper status.\n" );
		printf( "  'C' to leave submenu.\n\n" );

		key = getch();
		if ( ( key == 'R' ) || ( key == 'r' ) )
		{
			printf( "Releasing parallel hopper coins...\n" );
			if ( !XlineReleaseParallelHopperCoinsEx( xBoard, inputNum, lowPulseTime, highPulseTime, activeState,
				                                     outputNum, coinTimeout, coinsToRelease ) )
			{
				if ( XlineGetLastError( xBoard ) == USB_MESSAGE_PARALLEL_INPUT_CURRENTLY_ACTIVE )
				{
					printf( "The specified input is currently active. This must be set inactive\n" );
					printf( "prior to releasing coins.\n\n" );
				}
				else
				{
					printf( "ReleaseParallelHopperCoins error - %d.\n\n", (int)XlineGetLastError( xBoard ) );
					return;
				}
			}
		}
		else if ( ( key == 'S' ) || ( key == 's' ) )
		{
			printf( "Getting parallel hopper status..." );
			if ( !XlineGetParallelHopperStatus( xBoard, inputNum, &status, &coinsReleased ) )
			{
				printf( "GetParallelHopperStatus error - %d.\n\n", (int)XlineGetLastError( xBoard ) );
				/*	return; */
			}

			if ( status == Running ) printf( "Running, %d released.\n", coinsReleased );
			else if ( status == Success ) printf( "Success, %d released.\n", coinsReleased );
			else if ( status == Failure ) printf( "Failure, %d released.\n", coinsReleased );
			else if ( status == InitJam ) printf( "InitJam, %d released.\n", coinsReleased );
			else if ( status == Jam ) printf( "Jam, %d released.\n", coinsReleased );
		}
		else if ( ( key == 'C' ) || ( key == 'c' ) )
		{
			break;
		}
	}
}
