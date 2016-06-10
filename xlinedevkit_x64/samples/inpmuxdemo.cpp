/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         inpmuxdemo.cpp
*
*     Version:      1.1
*
*     Description:  Input multiplexer Demonstration
*
*     Notes:        Console application to demonstrate input
*                   mutliplexing
*
*     Changes:
*
*	  1.1 RDP		Added support for X15 product 10/10/2007
*     1.0 MJB       First version. Based on issue 1.0 of
*                   X10 inpmuxdemo.cpp, 28/07/2005
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

#ifndef X10_LINUX_BUILD
#define InitialiseConsole()
#endif

static unsigned char qMuxEmabled = 0;

static void displayMultiplexedInputs( FireFlyUSB * FireFly );
static void togleMultiplexedInputs( FireFlyUSB * FireFly );


int main( int argc, char* argv[])
{
	FireFlyUSB 		FireFly;
	Authenticate	X15Authenticate;
	char 			key;
	BYTE 			fittedBoard;

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "This application demonstrates X10 Input multiplexer.\n\n" );
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

    /* This is only used on the Linux version but #ifdefs look ugly in the
       code*/
	InitialiseConsole();

	while( TRUE )
	{
		printf( "X10 Multiplexed Inputs Menu\n" );
		printf( "===========\n\n" );

		printf( "Options:\n" );
        if( 0 == qMuxEmabled )
        {
            printf( "    'T' to Enable the input multiplexer.\n" );
        }
        else
        {
            printf( "    'T' to disable the input multiplexer.\n" );
        }

		printf( "    'P' to Display multiplexed inputs.\n" );
		printf( "or, 'C' to exit.\n\n" );

		printf( "Please make a selection: " );

		key = toupper( getch());
		printf( "\n\n" );
		if ( key == 'T' )
        {
            togleMultiplexedInputs( &FireFly );
        }
		else if ( key == 'P' )
        {
            displayMultiplexedInputs( &FireFly );
        }
		else if ( key == 'C' )
        {
            break;
        }
	}

	return( ExitFunction( "Leaving program.", &FireFly, &X15Authenticate, 0 ));
}


static void displayMultiplexedInputs( FireFlyUSB * FireFly )
{
    usbMultiplexedInput MuxInputs;
    int key;
    int i;
    int c;

	while( TRUE )
	{
		if( !FireFly->GetMultiplexedInputs( &MuxInputs ))
		{
			printf( "\nFailure %d getting multiplexed inputs.\n", (int)FireFly->GetLastError( ) );
			break;
		}

		/* Print inputs to screen. */
        printf( "Multiplexed inputs %s\n", MuxInputs.byMuxStatus != 0 ? "enabled" : "disabled" );
		printf( "\nInputs   2    1    0" );
		for( i = 0; i < USB_MUXIN_CHANNELS; i++ )
        {
            printf( "\nchannel%d ", i + 1 );
            for( c = USB_MUXIN_BYTES; c > 0 ; c-- )
            {
                printf( "%02X   ", MuxInputs.byMuxInp[ i ][ c - 1 ]);
            }
        }

		printf( "\n\nPress any key to update display or 'C' to close.\n" );

		key = toupper( getch());
		if ( key == 'C' )
        {
            break;
        }
	}
}


static void togleMultiplexedInputs( FireFlyUSB *FireFly )
{

    if( 0 == qMuxEmabled )
    {
        if( 0 == FireFly->InputMultiplexing( InputMultiplexEnabled ))
        {
            printf( "\nFailed in enabling input multiplexer\n" );
        }
        else
        {
            printf( "\nInput Multiplexer enabled\n" );
            qMuxEmabled = 1;
        }
    }
    else
    {
        if( 0 == FireFly->InputMultiplexing( InputMultiplexDisabled ))
        {
            printf( "\nFailed in disabling input multiplexer\n" );
        }
        else
        {
            printf( "\nInput Multiplexer disabled\n" );
            qMuxEmabled = 0;
        }
    }
}
