/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         serialdemo.cpp
*
*     Version:      1.1
*
*     Description:  Serial IO Demonstration
*
*     Notes:        Console application to demonstrate serial IO
*
*					This application reads and writes data on
*					a specific port.
*
*     Changes:
*
*	  1.1 RDP		Added support for X15 product 10/10/2007
*     1.0 MJB       First version. Based on issue 1.2 of X10
*                   serialdemo.cpp, 29/7/2005
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

#define RX_BUFFER_SIZE  212
#define TX_BUFFER_SIZE	12

#define TIMEOUT_MSG_TIME			2.0f		// Timeout in seconds
#define TIMEOUT_MSG_TEXT			"Timeout message"
#define TIMEOUT_MSG_LENGTH			(BYTE)strlen(TIMEOUT_MSG_TEXT)

int portABaudRates[] = { 1200, 2400, 4800, 9600, 19200, 38400, 62500 };
int portBBaudRates[] = { 110, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 38400, 57600, 62500 };


int main( int argc, char* argv[] )
{
	FireFlyUSB 			FireFly;
	Authenticate		X15Authenticate;
	DCB 				config={0};							// Configure device
	BYTE 				msg[TX_BUFFER_SIZE]={0};				// Create buffer for string to transmit
	BYTE 				rxMsg [RX_BUFFER_SIZE + 1]={0};			// Create buffer for message to receive
	BYTE 				txChar = 'A', dst, interval;
	WORD 				parityErrors;
	BOOL 				bReceived;
	long unsigned int 	bytesRxd;
	int 				i, baudRate, numBaudRates, *baudRates;
	usbSerialPort 		portNum;
	char 				key=0, baudRateString[10]={0};
	BYTE 				fittedBoard=0;
	#ifdef X10_LINUX_BUILD
		int					ignore __attribute__ ((unused));
	#else
		int					ignore;
	#endif

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "Serial Port Demonstration\n\n" );

	#ifdef X10_LINUX_BUILD
	InitialiseConsole( );
	EnableGetchNewline( );
	#endif

	// Establish Port: A or B
	while( 1 != 2 )
	{
		printf( "Use PORT A or PORT B (A or B)? " );
		key = getch();
		if ( key == 'A' || key == 'a' )      { portNum = PORT_A; break; }
		else if ( key == 'B' || key == 'b' ) { portNum = PORT_B; break; }
		else printf( "%c - Invalid port.\n", toupper( key ) );
	}
	printf( "%c\n", toupper( key ) );

	if ( portNum == PORT_A ) { baudRates = portABaudRates; numBaudRates = sizeof( portABaudRates ); }
	else { baudRates = portBBaudRates; numBaudRates = sizeof( portBBaudRates ); }
	numBaudRates = numBaudRates / 4;

	#ifdef X10_LINUX_BUILD
	EnableEcho( );
	#endif

	// Establish Baud Rate
	key = 0;
	while( key == 0 )
	{
		printf( "Possible Baud Rates:\n%d", baudRates[0] );
		for ( i = 1; i<numBaudRates; i++ ) printf( ", %d", baudRates[i] );
		printf( "\n\nPlease enter Baud rate: " );
		ignore = scanf( "%s", baudRateString );			// ignore used to quell g++ compiler warning
		baudRate = atoi( baudRateString );

		for ( i = 0; i<numBaudRates; i++ )
		{
			if ( baudRates[i] == baudRate ) key = 1;
		}
		if ( key == 0 ) printf( "Unknown Baud Rate.\n\n" );
	}

	#ifdef X10_LINUX_BUILD
	DisableEcho( );
	#endif

	config.Parity = NOPARITY;
	// Establish Parity: None, Odd or Even
	while( 1 != 2 )
	{
		printf( "Parity? (N)one, (O)dd or (E)ven? " );
		key = getch();
		if ( key == 'N' || key == 'n' )      { config.Parity = NOPARITY; break; }
		else if ( key == 'O' || key == 'o' ) { config.Parity = ODDPARITY; break; }
		else if ( key == 'E' || key == 'e' ) { config.Parity = EVENPARITY; break; }
		else printf( "%c - Invalid parity option.\n", toupper( key ) );
	}
	printf( "%c\n", toupper( key ) );

	printf( "\nInitialising and unlocking X10 device..." );

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

	config.BaudRate = baudRate;
	config.fOutxCtsFlow = FALSE;
	config.fRtsControl = RTS_CONTROL_TOGGLE;
	if ( !FireFly.SetConfig( portNum, &config, PORT_RS232 ) )
		return( ExitFunction( "SetConfig failed.", &FireFly, &X15Authenticate, 1 ) );

	// Check to see if any old data is in serial port SRAM
	if ( !FireFly.Receive( portNum, rxMsg, &bytesRxd ) )
		return( ExitFunction( "Receive error.", &FireFly, &X15Authenticate, 1 ) );

	// Print received data
	if ( bytesRxd > 0 )
	{
		rxMsg[bytesRxd] = '\0';		// Terminate string
		printf( "\nReceived old serial port data: %d bytes, [%s]\n", (int)bytesRxd, rxMsg );
	}

	if ( !FireFly.SetTimeoutMessage( portNum, (LPBYTE)TIMEOUT_MSG_TEXT, TIMEOUT_MSG_LENGTH, TIMEOUT_MSG_TIME ) )
		return( ExitFunction( "SetTimeoutMessage error.", &FireFly, &X15Authenticate, 1 ) );

	printf( "\n\nTesting single byte send and single byte receive (with timestamp).\n" );
	printf( "Also testing watchdog timeout feature. This is set to timeout at 2\n" );
	printf( "seconds with the send message 'Timeout'.\n\n" );
	printf( "Press any key to send over serial port or 'C' to exit demo.\n\n" );

	while ( TRUE )
	{
		if ( !FireFly.ReceiveByteWithTimestamp( portNum, &dst, &interval, &bReceived ) )
			return( ExitFunction( "Receive byte with timestamp error.", &FireFly, &X15Authenticate, 1 ) );

		if ( bReceived )		// Byte received
		{
			printf( "Byte received: %c (%02X), timestamp = %dms.\n", dst, dst, interval );
		}

		// Obtain parity error count
		if ( !FireFly.GetParityErrors( portNum, &parityErrors ) )
			return( ExitFunction( "GetParityErrors error.", &FireFly, &X15Authenticate, 1 ) );

		if ( parityErrors ) printf( "Parity errors: %d\n", parityErrors );

		// Upon keystroke, test to see if it is 'C', in which case exit program. Otherwise
		// send key to serial port.
		if ( _kbhit() )
		{
			key = getch();
			if ( key == 'C' || key == 'c' ) break;
			else
			{
				// Transmit data
				if ( !FireFly.Send( portNum, (LPBYTE)&key, 1 ) )
					return( ExitFunction( "Serial Send error.", &FireFly, &X15Authenticate, 1 ) );
				else printf( "Sent byte: %c\n", key );
			}
		}
	}

	if ( !FireFly.SetTimeoutMessage( portNum, msg, 0, 0.0 ) )
		return( ExitFunction( "SetTimeoutMessage error.", &FireFly, &X15Authenticate, 1 ) );

	printf( "\n\nTesting multiple byte send and receive.\n\n" );
	printf( "Press any key between transactions, or 'C' to quit.\n\n" );

	msg[0] = '!';
	msg[TX_BUFFER_SIZE - 1] = '!';

	while ( TRUE )
	{
		// Receive data
		if ( !FireFly.Receive( portNum, rxMsg, &bytesRxd ) )
			return( ExitFunction( "Serial Receiveerror.", &FireFly, &X15Authenticate, 1 ) );

		// Print received data
		if ( bytesRxd > 0 )
		{
			rxMsg[bytesRxd] = '\0';		// Terminate string
			printf( "Received %d bytes:- [%s]\n", (int)bytesRxd, rxMsg );
		}
		else printf( "Received no bytes.\n" );

		// Obtain parity error count
		if ( !FireFly.GetParityErrors( portNum, &parityErrors ) )
			return( ExitFunction( "GetParityErrors error.", &FireFly, &X15Authenticate, 1 ) );

		printf( "Parity error count: %d\n", parityErrors );

		// Generate data to transmit
		for ( int c = 1; c < TX_BUFFER_SIZE - 1; c++ ) msg[c] = txChar;

		// Print data to transmit
		printf( "Sending '!' + '%c' * %d + '!'\n", txChar, TX_BUFFER_SIZE - 2 );

		// Transmit data
		if ( !FireFly.Send( portNum, msg, TX_BUFFER_SIZE ) )
			return( ExitFunction( "Serial Send error.", &FireFly, &X15Authenticate, 1 ) );

		if ( ++txChar > 'Z' ) txChar = 'A';

		// Wait for keystroke.
		key = getch();
		if ( key == 'C' || key == 'c' ) break;
	}

	return( ExitFunction( "Leaving program.", &FireFly, &X15Authenticate, 0 ) );
}

