/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         inpmuxdemo.c
*
*     Version:      1.0
*
*     Description:  Input multiplexer Demonstration
*
*     Notes:        Console application to demonstrate input
*                   mutliplexing
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

#ifndef X10_LINUX_BUILD
	#define InitialiseConsole()
#endif

static unsigned char qMuxEmabled = 0;

static void displayMultiplexedInputs( void *xBoard );
static void togleMultiplexedInputs( void *xBoard );


int main( int argc, char* argv[])
{
	void	 		*xBoard;
	Authenticate	x15Authenticate;
	char 			key;
	BYTE 			fittedBoard;

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "This application demonstrations X10 Input multiplexer.\n\n" );
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

    /* This is only used on the Linux version but #ifdefs look ugly in the code*/
	InitialiseConsole();

	while( TRUE )
	{
		printf( "X-line Multiplexed Inputs Menu\n" );
		printf( "====-=========================\n\n" );

		printf( "Options:\n" );
        if ( 0 == qMuxEmabled )
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
            togleMultiplexedInputs( xBoard );
        }
		else if ( key == 'P' )
        {
            displayMultiplexedInputs( xBoard );
        }
		else if ( key == 'C' )
        {
            break;
        }
	}

	return( XlineExit( "Leaving program.", xBoard, &x15Authenticate, 0 ));
}


static void displayMultiplexedInputs( void *xBoard )
{
    usbMultiplexedInput MuxInputs;
    int key;
    int i;
    int c;

	while( TRUE )
	{
		if ( !XlineGetMultiplexedInputs( xBoard, &MuxInputs ))
		{
			printf( "\nFailure %d getting multiplexed inputs.\n", (int)XlineGetLastError( xBoard ) );
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


static void togleMultiplexedInputs( void *xBoard )
{
	if ( 0 == qMuxEmabled )
	{
		if ( 0 == XlineInputMultiplexing( xBoard, InputMultiplexEnabled ) )
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
		if ( 0 == XlineInputMultiplexing( xBoard, InputMultiplexDisabled ) )
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
