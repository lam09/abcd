/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         iodemo.c
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
	#include <string.h>				/* specifically for memset() */
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

#define PULSE_DURATION_MS			150
#define NUM_OUT_BYTES				4
#define NUM_IN_BYTES				3

void test_general_io( void *xBoard );
void test_output_pulsing( void *xBoard );
void test_input_change( void *xBoard );
void test_input_change_via_callback( void *xBoard );
void callback_routine (usbInput *changedInputs, usbInput *currentInputs );

long cbcount;

int main( int argc, char* argv[] )
{
	void			*xBoard;
	Authenticate	x15Authenticate;
	BYTE 			fittedBoard;
	char 			key;

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "This application demonstrations X10 IO.\n\n" );
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
	#endif

	while ( TRUE )
	{
		printf( "X-line IO Menu\n" );
		printf( "==============\n\n" );

		printf( "Options:\n" );
		printf( "    'T' to test general X10 input reading and output writing.\n" );
		printf( "    'P' to test output pulsing.\n" );
		printf( "    'U' to test input change.\n" );
		printf( "    'I' to test input changes via Callback Interrupt.\n" );

		printf( "or, 'C' to exit.\n\n" );
		printf( "Please make a selection: " );

		key = toupper( getch() );
		printf( "\n\n" );
		if ( key == 'T' ) test_general_io( xBoard );
		else if ( key == 'P' ) test_output_pulsing( xBoard );
		else if ( key == 'U' ) test_input_change( xBoard );
		else if ( key == 'I' ) test_input_change_via_callback( xBoard );
		else if ( key == 'C' ) break;
	}

	return( XlineExit( "Leaving program.", xBoard, &x15Authenticate, 0 ) );
}

void test_general_io( void *xBoard )
{
	char key;
	int i;
	usbInput inputs;
	usbOutput outputs;

	for ( i = 1; i < NUM_OUT_BYTES; i++ )
	{
		outputs.byOut[i] = 0x00;
	}
	outputs.byAux = 0x00;
	outputs.byOut[0] = 0x01;

	while ( TRUE )
	{

		if ( !XlineGetInputs( xBoard, &inputs ) )
		{
			printf( "\nFailure %d getting inputs.\n", (int)XlineGetLastError( xBoard ) );
			break;
		}

		/* Print inputs to screen. */
		printf( "\nInputs   0    1    2    SW   CS\n         " );
		for ( i = 0; i < NUM_IN_BYTES;  i++ )
		{
			printf( "%02X   ", inputs.byIn[i] & 0xFF );
		}
		printf( "%02X   ", inputs.bySw & 0xFF );
		printf( "%02X   ", inputs.byCs & 0xFF );

		// setting on/off period outputs the same either turns an output on or off continually, independent of brightness setting
		if ( !XlineSetOnPeriodOutputs( xBoard, outputs ) )
		{
			printf( "Failure %d setting outputs.\n", (int)XlineGetLastError( xBoard ) );
			break;
		}
		if ( !XlineSetOffPeriodOutputs( xBoard, outputs ) )
		{
			printf( "Failure %d setting outputs.\n", (int)XlineGetLastError( xBoard ) );
			break;
		}

		printf( "\n\nOutputs  0    1    2    3    AUX\n         " );
		for ( i = 0; i < NUM_OUT_BYTES; i++ )
		{
			printf( "%02X   ", outputs.byOut[i] & 0xFF );
		}

		/* Toggle outputs. */
		for ( i = 0; i < NUM_OUT_BYTES; i++ )
		{
			//if ( ( outputs[i] == 0x80 ) || ( ( i == 4 ) && ( outputs[i] == 0x20 ) ) )	/* AUX only has 6 outputs */
			if ( outputs.byOut[i] == 0x80 )
			{
				outputs.byOut[i] = 0x00;
 				outputs.byOut[(i != NUM_OUT_BYTES - 1 ? i + 1 : 0)] = 0x01;
				break;
			}
			else
			{
				outputs.byOut[i] = outputs.byOut[i] * 2;
			}
		}

		printf( "\n\nPress any key to update display or 'C' to exit this test.\n" );

		key = getch();
		if ( ( key == 'C' ) || ( key == 'c' ) )
		{
			break;
		}
	}
}

void test_output_pulsing( void *xBoard )
{
	int output_bit_id = OUTPUT_BIT_OP0;
	BOOL bit_state = TRUE, bPulseComplete, bCurrentDetected;
	BYTE byTimeRemaining;
	char key;

	while ( TRUE )
	{
		printf( "Performing %dms pulse to output ", PULSE_DURATION_MS );
		if ( output_bit_id < 32 )
		{
			printf( "OP%d ... ", output_bit_id );
		}
		else
		{
			printf( "AUX%d ... ", (output_bit_id-32) );
		}

		if ( !XlinePulseOutput( xBoard, output_bit_id, PULSE_DURATION_MS ) )
		{
			printf( "\nFailure %d pulsing output bit.\n", (int)XlineGetLastError( xBoard ) );
			break;
		}

		do
		{
			if ( !XlinePulseOutputResult( xBoard, &byTimeRemaining, &bPulseComplete, &bCurrentDetected ) )
			{
				printf( "\nFailure %d obtaining pulse output result.\n", (int)XlineGetLastError( xBoard ) );
				break;
			}
		} while ( !bPulseComplete );

		if ( bCurrentDetected )
		{
			printf( "current detected.\n" );
		}
		else
		{
			printf( "no current detected.\n" );
		}

		output_bit_id++;
		if ( output_bit_id > OUTPUT_BIT_AUX5 )
		{
			output_bit_id = OUTPUT_BIT_OP0;
			if ( bit_state == TRUE ) bit_state = FALSE;
			else bit_state = TRUE;
		}

		printf( "\nPress any key to update display or 'C' to exit this test.\n" );

		key = getch();
		if ( ( key == 'C' ) || ( key == 'c' ) )
		{
			break;
		}
	}
}

void test_input_change( void *xBoard )
{
	usbInput inputs;
	usbInput changedInputs;
	char key;
	int i;

	while ( TRUE )
	{
		printf( "Press: 'R' to read inputs, 'U' to read changed inputs, 'C' to quit.\n\n" );
		key = getch();
		if ( ( key == 'C' ) || ( key == 'c' ) )
		{
			break;
		}
		else if ( ( key == 'R' ) || ( key == 'r' ) )
		{

			if ( !XlineGetInputs( xBoard, &inputs ) )
			{
				printf( "\nFailure %d getting inputs.\n", (int)XlineGetLastError( xBoard ) );
				break;
			}

			/* Print inputs to screen. */
			printf( "Inputs   0    1    2    SW   CS\n         " );
			for ( i = 0; i < NUM_IN_BYTES; i++ )
			{
				printf( "%02X   ", inputs.byIn[i] & 0xFF );
			}
			printf( "%02X   ", inputs.bySw);
			printf( "%02X   ", inputs.byCs);
			printf( "\n\n" );
		}
		else if ( key == 'U' || key == 'u' )
		{
			if ( !XlineGetChangedInputs( xBoard, &changedInputs ) )
			{
				printf( "\nFailure %d getting inputs.\n", (int)XlineGetLastError( xBoard ) );
				break;
			}

			/* Print inputs to screen. */
			printf( "Changed   0    1    2    SW   CS\n" );
			printf( "          %02X   %02X   %02X   ", changedInputs.byIn[0], changedInputs.byIn[1], changedInputs.byIn[2] );
			printf( "%02X   %02X\n\n", changedInputs.bySw, changedInputs.byCs );
		}
	}
}


void test_input_change_via_callback( void *xBoard )
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

	if ( !XlineConfigureChangedInputsCallback( xBoard, &changedInputsMask, &changedInputsLevel, &callback_routine ) )
	{
		printf( "\nFailure %d setting callback to address %lX.\n", (int)XlineGetLastError( xBoard ), (DWORD)&callback_routine );
		return;
	}

	printf("Callback now activated, will automatically read and display changes on selected inputs lines as follows:\n");
	printf(".    Dev kit switch IP0/IP1 (and X10i DIP switch 0/1) switching to HIGH will activate callback\n");
	printf("     Dev kit switch IP2/IP3 (and DIP switch 2/3) switching to LOW also activate callback \n");
	printf("     Press 'C' to exit Callback test.\n\n" );
	while ( TRUE )
	{
		key = getch();									/* blocks awaiting keyboard char */
		if ( key == 'C' || key == 'c' )
			break;

	}
	/* this step disables the callback mechanism once more */
	if ( !XlineConfigureChangedInputsCallback( xBoard, &changedInputsMask, &changedInputsLevel, NULL ) )
	{
		printf( "\nFailure %d setting callback to NULL.\n", (int)XlineGetLastError( xBoard ) );
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

