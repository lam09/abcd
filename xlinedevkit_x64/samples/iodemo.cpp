/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         iodemo.cpp
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
*     1.0 MJB       First version. Based on issue 1.2 of
*                   X10 iodemo.cpp, 28/7/2005
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


#define PULSE_DURATION_MS			150

void test_general_io( FireFlyUSB * FireFly );
void test_output_pulsing( FireFlyUSB * FireFly );
void test_input_change( FireFlyUSB * FireFly );
void test_input_change_via_callback( FireFlyUSB * FireFly );
void callback_routine (usbInput *changedInputs, usbInput *currentInputs );

long cbcount=0;

int main(int argc, char* argv[])
{
	FireFlyUSB 		FireFly;
	Authenticate	X15Authenticate;
	BYTE 			fittedBoard;
	char 			key;

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "This application demonstrates X10 IO.\n\n" );
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
								X15Authenticate.BeginThread( &FireFly, EncryptedKey, 30 );
								break;

		default:				// unknown board fitted
								return( ExitFunction( "unknown board fitted.", &FireFly, &X15Authenticate, 1 ) );
								break;
	}
	printf( "success.\n\n" );

	#ifdef X10_LINUX_BUILD
	InitialiseConsole( );
	#endif

	while ( TRUE )
	{
		printf( "X10 IO Menu\n" );
		printf( "===========\n\n" );

		printf( "Options:\n" );
		printf( "    'T' to test general X10 input reading and output writing.\n" );
		printf( "    'P' to test output pulsing.\n" );
		printf( "    'U' to test input change.\n" );
		printf( "    'I' to test input changes via Callback Interrupt.\n" );

		printf( "or, 'C' to exit.\n\n" );
		printf( "Please make a selection: " );

		key = toupper( getch() );
		printf( "\n\n" );
		if ( key == 'T' ) test_general_io( &FireFly );
		else if ( key == 'P' ) test_output_pulsing( &FireFly );
		else if ( key == 'U' ) test_input_change( &FireFly );
		else if ( key == 'I' ) test_input_change_via_callback( &FireFly );

		else if ( key == 'C' ) break;
	}

	return( ExitFunction( "Leaving program.", &FireFly, &X15Authenticate, 0 ) );
}

void test_general_io( FireFlyUSB * FireFly )
{
	BYTE inputs[USB_IN_LENGTH]={0}, outputs[USB_OUT_LENGTH]={0};
	char key;
	int i;

	for ( i = 1; i < USB_OUT_LENGTH; i++ ) outputs[i] = 0x00;
	outputs[0] = 0x01;

	while ( TRUE )
	{
		if ( !FireFly->GetInputs( inputs ) )
		{
			printf( "\nFailure %d getting inputs.\n", (int)FireFly->GetLastError( ) );
			break;
		}

		/* Print inputs to screen. */
		printf( "\nInputs   0    1    2    SW   CS\n         " );
		for ( i = 0; i < USB_IN_LENGTH; i++ ) printf( "%02X   ", inputs[i] & 0x000000FF );

		if ( !FireFly->SetOutputs( outputs ) )
		{
			printf( "Failure %d setting outputs.\n", (int)FireFly->GetLastError( ) );
			break;
		}
		printf( "\n\nOutputs  0    1    2    3    AUX\n         " );
		for ( i = 0; i < USB_OUT_LENGTH; i++ ) printf( "%02X   ", outputs[i] & 0x000000FF );

		/* Toggle outputs. */
		for ( i = 0; i < USB_OUT_LENGTH; i++ )
		{
			if ( ( outputs[i] == 0x80 ) || ( ( i == 4 ) && ( outputs[i] == 0x20 ) ) )	// AUX only has 6 outputs
			{
				outputs[i] = 0x00;
 				outputs[(i != USB_OUT_LENGTH - 1 ? i + 1 : 0)] = 0x01;
				break;
			}
			else outputs[i] = outputs[i] * 2;
		}

		printf( "\n\nPress any key to update display or 'C' to close.\n" );

		key = getch();
		if ( key == 'C' || key == 'c' ) break;
	}
}

void test_output_pulsing( FireFlyUSB * FireFly )
{
	int output_bit_id = OUTPUT_BIT_OP0;
	BOOL bit_state = TRUE, bPulseComplete=0, bCurrentDetected=0;
	BYTE byTimeRemaining=0;
	char key;

	while ( TRUE )
	{
		printf( "Performing %dms pulse to output ", PULSE_DURATION_MS );
		if ( output_bit_id < 32 ) printf( "OP%d ... ", output_bit_id );
		else printf( "AUX%d ... ", (output_bit_id-32) );

		if ( !FireFly->PulseOutput( output_bit_id, PULSE_DURATION_MS ) )
		{
			printf( "\nFailure %d pulsing output bit.\n", (int)FireFly->GetLastError( ) );
			break;
		}

		do
		{
			if ( !FireFly->PulseOutputResult( &byTimeRemaining, &bPulseComplete, &bCurrentDetected ) )
			{
				printf( "\nFailure %d obtaining pulse output result.\n", (int)FireFly->GetLastError( ) );
				break;
			} else {
                printf( "%d %d %d\n", byTimeRemaining, bPulseComplete, bCurrentDetected );
            }
		} while ( !bPulseComplete );

		if ( bCurrentDetected ) printf( "current detected.\n" );
		else printf( "no current detected.\n" );

		output_bit_id++;
		if ( output_bit_id > OUTPUT_BIT_AUX5 )
		{
			output_bit_id = OUTPUT_BIT_OP0;
			if ( bit_state == TRUE ) bit_state = FALSE;
			else bit_state = TRUE;
		}

		printf( "\nPress any key to update display or 'C' to close.\n" );

		key = getch();
		if ( key == 'C' || key == 'c' ) break;
	}
}

void test_input_change( FireFlyUSB * FireFly )
{
	BYTE inputs[USB_IN_LENGTH]={0};
	usbInput changedInputs;
	char key;
	int i;

	while ( TRUE )
	{
		printf( "Press: 'R' to read inputs, 'U' to read changed inputs, 'C' to quit.\n\n" );
		key = getch();
		if ( key == 'C' || key == 'c' ) break;
		else if ( key == 'R' || key == 'r' )
		{
			if ( !FireFly->GetInputs( inputs ) )
			{
				printf( "\nFailure %d getting inputs.\n", (int)FireFly->GetLastError( ) );
				break;
			}

			/* Print inputs to screen. */
			printf( "Inputs   0    1    2    SW   CS\n         " );
			for ( i = 0; i < USB_IN_LENGTH; i++ ) printf( "%02X   ", inputs[i] & 0x000000FF );
			printf( "\n\n" );
		}

		else if ( key == 'U' || key == 'u' )
		{
			if ( !FireFly->GetChangedInputs( &changedInputs ) )
			{
				printf( "\nFailure %d getting inputs.\n", (int)FireFly->GetLastError( ) );
				break;
			}

			// Print inputs to screen.
			printf( "Changed   0    1    2    SW   CS\n" );
			printf( "          %02X   %02X   %02X   ", changedInputs.byIn[0], changedInputs.byIn[1], changedInputs.byIn[2] );
			printf( "%02X   %02X\n\n", changedInputs.bySw, changedInputs.byCs );
		}

	}
}


void test_input_change_via_callback( FireFlyUSB * FireFly )
{
	char key;
	usbInput changedInputsMask;
	usbInput changedInputsLevel;

	memset( (BYTE *)&changedInputsMask, 0x00, sizeof( changedInputsMask) );
	memset( (BYTE *)&changedInputsLevel, 0x00, sizeof( changedInputsLevel) );

	changedInputsMask.byIn[0] = 0x0F;
	changedInputsLevel.byIn[0] = 0x03;

	changedInputsMask.bySw = 0x0F;
	changedInputsLevel.bySw = 0x03;

	cbcount = 0;

	if ( !FireFly->ConfigureChangedInputsCallback( &changedInputsMask, &changedInputsLevel, &callback_routine ) )
	{
		printf( "\nFailure %d setting callback to address %lX.\n", (int)FireFly->GetLastError( ), (DWORD)&callback_routine );
		return;
	}

	printf("Callback now activated, will automatically read and display changes on selected inputs lines as follows:\n");
	printf(".    Dev kit switch IP0/IP1 (and X10i DIP switch 0/1) switching to HIGH will activate callback\n");
	printf("     Dev kit switch IP2/IP3 (and DIP switch 2/3) switching to LOW also activate callback \n");
	printf("     Press 'C' to exit Callback test.\n\n" );
	while ( TRUE )
	{
		key = getch();									// blocks awaiting keyboard char
		if ( key == 'C' || key == 'c' )
			break;

	}
	// this step disables the callback mechanism once more
	if ( !FireFly->ConfigureChangedInputsCallback( &changedInputsMask, &changedInputsLevel, NULL ) )
	{
		printf( "\nFailure %d setting callback to NULL.\n", (int)FireFly->GetLastError( ) );
		return;
	}

	printf("\n\n");

}

void callback_routine(usbInput *changedInputs, usbInput *currentInputs )
{
	cbcount++;
	printf( "\nCallback %06ld  invoked:  Changes:  In0=0x%02X   In1=0x%02X   In2=0x%02X    SW=0x%02X    CS=0x%02X \n",
												cbcount,
												changedInputs->byIn[0],
												changedInputs->byIn[1],
												changedInputs->byIn[2],
	 											changedInputs->bySw,
												changedInputs->byCs );
	printf( "Current input values:                In0=0x%02X   In1=0x%02X   In2=0x%02X    SW=0x%02X    CS=0x%02X \n",
												currentInputs->byIn[0],
												currentInputs->byIn[1],
												currentInputs->byIn[2],
	 											currentInputs->bySw,
												currentInputs->byCs );
	printf( "Press 'C' to exit Callback test.\n\n" );

}


