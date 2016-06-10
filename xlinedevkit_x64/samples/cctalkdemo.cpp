/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         cctalkdemo.cpp
*
*     Version:      1.2
*
*     Description:  CCTalk Mode 1 Demonstration
*
*     Notes:        Console application to demonstrate CCTalk
*                   Mode 1 operation
*
*     Changes:
*
*	  1.2 RDP		Added support for X15 product 10/10/2007
*     1.1 MJB       Added host timeout inhibit function, 1/2/2006
*     1.0 MJB       First version. Based on issue 1.4 of
*                   X10 cctalkdemo.cpp, 28/7/2005
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


#define MAX_CCTALK_RX_LEN		257						// Maximum CCTalk response length


/* -------------------------------------------------------------------------------------------------------
   Main entry function
   ------------------------------------------------------------------------------------------------------*/
int main( int argc, char* argv[] )
{
	FireFlyUSB 		FireFly;
	Authenticate	X15Authenticate;
	BYTE 			fittedBoard;
	DCB 			config;								// Configure device
	usbSerialPort 	portNum;							// Port: PORT_A or PORT_B
	CCTalkConfig 	cctalk_config;						// CCTalk configuration structure
	BYTE 			rxMsg [MAX_CCTALK_RX_LEN + 1];		// Create buffer for message to receive
	BOOL 			is_inhibited;						// Is device inhibited?
	BOOL 			had_response;						// Have we received CCTalk response?
	BYTE 			deviceNum;
	float 			hostTimeout;
	char 			key;
	long unsigned int bytesRxd, i;
	#ifdef X10_LINUX_BUILD
		int				ignore __attribute__ ((unused));
	#else
		int				ignore;
	#endif

/* display demo header information */
	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "CCTalk Mode 1 Demonstration\n\n" );
	printf( "\nInitialising and unlocking device..." );

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
	ignore = scanf( "%f", &hostTimeout );				// ignore used to quell g++ compiler warning

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
	if ( !FireFly.SetConfig( portNum, &config, PORT_CCTALK_MODE1 ) )
		return( ExitFunction( "SetConfig failed.", &FireFly, &X15Authenticate, 1 ) );

	deviceNum = 0;

/* Configure the CCTalk Port */
	cctalk_config.device_number = deviceNum;
	cctalk_config.method = Repeated;
	cctalk_config.next_trigger_device = NO_TRIGGER;
	cctalk_config.poll_retry_count = 5;
	cctalk_config.polling_interval = 200;
	cctalk_config.max_response_time = 2000;
	cctalk_config.min_buffer_space = 20;
	cctalk_config.poll_msg[0] = 0x02;					// destination address
	cctalk_config.poll_msg[1] = 0x00;					// transmit data length (after Header, before Checksum)
	cctalk_config.poll_msg[2] = 0x01;					// source address
	cctalk_config.poll_msg[3] = 0xe5;					// command: E5 = ReadBufferedCredit  FE=SimplePoll
	cctalk_config.poll_msg[4] = 0x18;					// checksum
	cctalk_config.inhibit_msg[0] = 0x02;
	cctalk_config.inhibit_msg[1] = 0x00;
	cctalk_config.inhibit_msg[2] = 0x01;
	cctalk_config.inhibit_msg[3] = 0xe5;
	cctalk_config.inhibit_msg[4] = 0x18;

	if ( !FireFly.ConfigureCCTalkPort( portNum, &cctalk_config ) )
		return( ExitFunction( "ConfigureCCTalkPort failed.", &FireFly, &X15Authenticate, 1 ) );

	if ( !FireFly.SetPolledHostTimeout( portNum, deviceNum, (double)hostTimeout ) )
		return( ExitFunction( "SetPolledHostTimeout failed.", &FireFly, &X15Authenticate, 1 ) );

	printf( "\nTesting CCTalk operation ('I' to reinitialise, 'R' to read, 'E' to erase buffer and 'C' to exit)\n\n" );

	while ( TRUE )
	{
		key = getch();

		if ( ( key == 'i' ) || ( key == 'I' ) )
		{
			printf( "Configure CCTalk Port...\n" );
			if ( !FireFly.ConfigureCCTalkPort( portNum, &cctalk_config ) )
				return( ExitFunction( "ConfigureCCTalkPort failed.", &FireFly, &X15Authenticate, 1 ) );

			if ( !FireFly.SetPolledHostTimeout( portNum, deviceNum, (double)hostTimeout ) )
				return( ExitFunction( "SetPolledHostTimeout failed.", &FireFly, &X15Authenticate, 1 ) );
		}
		else if ( ( key == 'r' ) || ( key == 'R' ) )
		{
			had_response = FALSE;

			do
			{
				// Receive data
				if ( !FireFly.ReceivePolledMessage( portNum, deviceNum, rxMsg, &bytesRxd, &is_inhibited ) )
					return( ExitFunction( "ReceivePolledMessage failed.", &FireFly, &X15Authenticate, 1 ) );

				// Print received data
				if ( bytesRxd > 0 )
				{
					had_response = TRUE;

					printf( "CCTalk Response = [ " );
					for ( i=0; i<bytesRxd; i++ ) printf( "%02X ", rxMsg[i] );
					printf( "]\n" );

					// Delete message
					if ( !FireFly.DeletePolledMessage( portNum, 0 ) )
						return( ExitFunction( "DeletePolledMessage failed.", &FireFly, &X15Authenticate, 1 ) );
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
			if ( !FireFly.EmptyPolledBuffer( portNum, 0 ) )
				return( ExitFunction( "EmptyPolledBuffer failed.", &FireFly, &X15Authenticate, 1 ) );
			else printf( "success.\n" );
		}

		else if ( ( key == 'c' ) || ( key == 'C' ) ) break;
	}

	return( ExitFunction( "Leaving program.", &FireFly, &X15Authenticate, 0 ) );
}

