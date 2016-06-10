/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         cctalkdemo.c
*
*     Version:      1.2
*
*     Description:  CCTalk Mode 1 Demonstration
*
*     Notes:        Console application to demonstrate CCTalk
*                   Mode 1 operation
*
*     Changes:
*     1.0 MJB       First version using C API, 14/1/2008.
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


#define MAX_CCTALK_RX_LEN		257						/* Maximum CCTalk response length */


/* -------------------------------------------------------------------------------------------------------
   Main entry function
   ------------------------------------------------------------------------------------------------------*/
int main( int argc, char* argv[] )
{
	void 			*xBoard;
	Authenticate	x15Authenticate;
	BYTE 			fittedBoard;
	DCB 			config={0};							/* Configure device						*/
	usbSerialPort 	portNum=PORT_A;						/* Port: PORT_A or PORT_B				*/
	CCTalkConfig 	cctalk_config={0};					/* CCTalk configuration structure		*/
	BYTE 			rxMsg [MAX_CCTALK_RX_LEN + 1];		/* Create buffer for message to receive	*/
	BOOL 			is_inhibited;						/* Is device inhibited?					*/
	BOOL 			had_response;						/* Have we received CCTalk response?	*/
	BYTE 			deviceNum;
	float 			hostTimeout;
	char 			key;
	long unsigned int bytesRxd=0, i;

	#ifdef X10_LINUX_BUILD
		int			ignore __attribute__ ((unused));
	#else
		int 		ignore;
	#endif

/* display demo header information */
	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "CCTalk Mode 1 Demonstration\n\n" );
	printf( "\nInitialising and unlocking device..." );

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

	/* user port selection */
	while ( 1 != 2 )
	{
		printf( "Use PORT A or PORT B (A or B)?\n" );
		key = getch();
		if ( key == 'A' || key == 'a' )      { portNum = PORT_A; break; }
		else if ( key == 'B' || key == 'b' ) { portNum = PORT_B; break; }
		else printf( "%c - Invalid port.\n", toupper( key ) );
	}
	printf( "%c\n", toupper( key ) );

	/* user timeout selection */
	printf( "Enter host timeout in seconds (0 - 25.5) (0 disables): " );
	ignore = scanf( "%f", &hostTimeout );					// ignore used to quell gcc compiler return value warning

	/* LINUX platform specific initialisation */
	#ifdef X10_LINUX_BUILD
		InitialiseConsole( );
		EnableGetchNewline( );
	#endif

	/* configure the user selected serial port */
	config.BaudRate = 9600;
	config.Parity = NOPARITY;
	config.fOutxCtsFlow = FALSE;
	config.fRtsControl = RTS_CONTROL_TOGGLE;
	if ( !XlineSetConfig( xBoard, portNum, &config, PORT_CCTALK_MODE1 ) )
	{
		return( XlineExit( "SetConfig failed.", xBoard, &x15Authenticate, 1 ) );
	}

	deviceNum = 0;

	/* Configure the CCTalk Port */
	cctalk_config.device_number = deviceNum;
	cctalk_config.method = Repeated;
	cctalk_config.next_trigger_device = NO_TRIGGER;
	cctalk_config.poll_retry_count = 5;
	cctalk_config.polling_interval = 200;
	cctalk_config.max_response_time = 2000;
	cctalk_config.min_buffer_space = 20;
	cctalk_config.poll_msg[0] = 0x02;
	cctalk_config.poll_msg[1] = 0x00;
	cctalk_config.poll_msg[2] = 0x01;
	cctalk_config.poll_msg[3] = 0xe5;
	cctalk_config.poll_msg[4] = 0x18;
	cctalk_config.inhibit_msg[0] = 0x02;
	cctalk_config.inhibit_msg[1] = 0x00;
	cctalk_config.inhibit_msg[2] = 0x01;
	cctalk_config.inhibit_msg[3] = 0xe5;
	cctalk_config.inhibit_msg[4] = 0x18;

	if ( !XlineConfigureCCTalkPort( xBoard, portNum, &cctalk_config ) )
	{
		return( XlineExit( "ConfigureCCTalkPort failed.", xBoard, &x15Authenticate, 1 ) );
	}

	if ( !XlineSetPolledHostTimeout( xBoard, portNum, deviceNum, (double)hostTimeout ) )
	{
		return( XlineExit( "SetPolledHostTimeout failed.", xBoard, &x15Authenticate, 1 ) );
	}

	printf( "\nTesting CCTalk operation ('I' to reinitialise, 'R' to read, 'E' to erase buffer and 'C' to exit)\n\n" );

	while ( TRUE )
	{
		key = getch();

		if ( ( key == 'i' ) || ( key == 'I' ) )
		{
			printf( "Configure CCTalk Port...\n" );
			if ( !XlineConfigureCCTalkPort( xBoard, portNum, &cctalk_config ) )
			{
				return( XlineExit( "ConfigureCCTalkPort failed.", xBoard, &x15Authenticate, 1 ) );
			}

			if ( !XlineSetPolledHostTimeout( xBoard, portNum, deviceNum, (double)hostTimeout ) )
			{
				return( XlineExit( "SetPolledHostTimeout failed.", xBoard, &x15Authenticate, 1 ) );
			}
		}
		else if ( ( key == 'r' ) || ( key == 'R' ) )
		{
			had_response = FALSE;

			do
			{
				/* Receive data */
				if ( !XlineReceivePolledMessage( xBoard, portNum, deviceNum, rxMsg, (LPUINT)&bytesRxd, &is_inhibited ) )
				{
					return( XlineExit( "ReceivePolledMessage failed.", xBoard, &x15Authenticate, 1 ) );
				}

				/* Print received data */
				if ( bytesRxd > 0 )
				{
					had_response = TRUE;

					printf( "CCTalk Response = [ " );
					for ( i=0; i<bytesRxd; i++ ) printf( "%02X ", rxMsg[i] );
					printf( "]\n" );

					/* Delete message */
					if ( !XlineDeletePolledMessage( xBoard, portNum, 0 ) )
					{
						return( XlineExit( "DeletePolledMessage failed.", xBoard, &x15Authenticate, 1 ) );
					}

					bytesRxd=0;
				}
				else
				{
					if ( !had_response ) printf( "No Response. " );

					if ( is_inhibited ) printf( "Device IS inhibited.\n" );
					else printf( "Device is NOT inhibited.\n" );
				}
			} while ( bytesRxd > 0 );
			printf( "\n" );
		}
		else if ( ( key == 'e' ) || ( key == 'E' ) )
		{
			printf( "Erase CCTalk Receive Buffer ... " );
			if ( !XlineEmptyPolledBuffer( xBoard, portNum, 0 ) )
			{
				return( XlineExit( "EmptyPolledBuffer failed.", xBoard, &x15Authenticate, 1 ) );
			}
			else printf( "success.\n" );
		}

		else if ( ( key == 'c' ) || ( key == 'C' ) ) break;
	}

	return( XlineExit( "Leaving program.", xBoard, &x15Authenticate, 0 ) );
}

