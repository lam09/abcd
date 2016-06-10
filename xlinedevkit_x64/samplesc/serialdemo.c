/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         serialdemo.c
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
*     1.0 JSH       First properly released version using true C code and C-API, 11/10/2011.
*
*****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define RX_BUFFER_SIZE  212
#define TX_BUFFER_SIZE	12

#define TIMEOUT_MSG_TIME			2.0f		/* Timeout in seconds */
#define TIMEOUT_MSG_TEXT			"Timeout message"
#define TIMEOUT_MSG_LENGTH			(BYTE)strlen(TIMEOUT_MSG_TEXT)

int portABaudRates[] = { 1200, 2400, 4800, 9600, 19200, 38400, 62500 };
int portBBaudRates[] = { 110, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 38400, 57600, 62500 };


int main( int argc, char* argv[] )
{
	void				*xBoard;
	Authenticate		x15Authenticate;
	DCB 				config;								/* Configure device */
	BYTE 				msg[TX_BUFFER_SIZE];				/* Create buffer for string to transmit */
	BYTE 				rxMsg [RX_BUFFER_SIZE + 1];			/* Create buffer for message to receive */
	BYTE 				txChar = 'A', dst, interval;
	WORD 				parityErrors;
	BOOL 				bReceived;
	long unsigned int 	bytesRxd;
	int 				i, c, baudRate, numBaudRates, *baudRates;
	usbSerialPort 		portNum;
	char 				key, baudRateString[10];
	BYTE 				fittedBoard;

	#ifdef X10_LINUX_BUILD
		int			ignore __attribute__ ((unused));
	#else
		int 		ignore;
	#endif


	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "Serial Port Demonstration\n\n" );

	#ifdef X10_LINUX_BUILD
		InitialiseConsole( );
		EnableGetchNewline( );
	#endif

	/* Establish Port: A or B */
	while( 1 != 2 )
	{
		printf( "Use PORT A or PORT B (A or B)? " );
		key = getch();
		if ( ( key == 'A' ) || ( key == 'a' ) )
		{
			portNum = PORT_A;
			break;
		}
		else if ( ( key == 'B' ) || ( key == 'b' ) )
		{
			portNum = PORT_B;
			break;
		}
		else
		{
			printf( "%c - Invalid port.\n", toupper( key ) );
		}
	}
	printf( "%c\n", toupper( key ) );

	if ( portNum == PORT_A )
	{
		baudRates = portABaudRates;
		numBaudRates = sizeof( portABaudRates );
	}
	else
	{
		baudRates = portBBaudRates;
		numBaudRates = sizeof( portBBaudRates );
	}
	numBaudRates = numBaudRates / 4;

	#ifdef X10_LINUX_BUILD
		EnableEcho( );
	#endif

	/* Establish Baud Rate */
	key = 0;
	while( key == 0 )
	{
		printf( "Possible Baud Rates:\n%d", baudRates[0] );
		for ( i = 1; i<numBaudRates; i++ )
		{
			printf( ", %d", baudRates[i] );
		}
		printf( "\n\nPlease enter Baud rate: " );
		ignore = scanf( "%s", baudRateString );				// ignore used to quell gcc warning
		baudRate = atoi( baudRateString );

		for ( i = 0; i<numBaudRates; i++ )
		{
			if ( baudRates[i] == baudRate )
			{
				key = 1;
			}
		}
		if ( key == 0 ) printf( "Unknown Baud Rate.\n\n" );
	}

	#ifdef X10_LINUX_BUILD
		DisableEcho( );
	#endif

	/* Establish Parity: None, Odd or Even */
	while( 1 != 2 )
	{
		printf( "Parity? (N)one, (O)dd or (E)ven? " );
		key = getch();
		if ( ( key == 'N' ) || ( key == 'n' ) )
		{
			config.Parity = NOPARITY;
			break;
		}
		else if ( ( key == 'O' ) || ( key == 'o' ) )
		{
			config.Parity = ODDPARITY;
			break;
		}
		else if ( ( key == 'E' ) || ( key == 'e' ) )
		{
			config.Parity = EVENPARITY;
			break;
		}
		else
		{
			printf( "%c - Invalid parity option.\n", toupper( key ) );
		}
	}
	printf( "%c\n", toupper( key ) );

	printf( "\nInitialising and unlocking X10 device..." );

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

	config.BaudRate = baudRate;
	config.fOutxCtsFlow = FALSE;
	config.fRtsControl = RTS_CONTROL_TOGGLE;
	if ( !XlineSetConfig( xBoard, portNum, &config, PORT_RS232 ) )
	{
		return( XlineExit( "SetConfig failed.", xBoard, &x15Authenticate, 1 ) );
	}

	/* Check to see if any old data is in serial port SRAM */
	if ( !XlineReceive( xBoard, portNum, rxMsg, (LPUINT)&bytesRxd ) )
	{
		return( XlineExit( "Receive error.", xBoard, &x15Authenticate, 1 ) );
	}

	/* Print received data */
	if ( bytesRxd > 0 )
	{
		rxMsg[bytesRxd] = '\0';		/* Terminate string */
		printf( "\nReceived old serial port data: %d bytes, [%s]\n", (int)bytesRxd, rxMsg );
	}

	if ( !XlineSetTimeoutMessage( xBoard, portNum, (LPBYTE)TIMEOUT_MSG_TEXT, TIMEOUT_MSG_LENGTH, TIMEOUT_MSG_TIME ) )
	{
		return( XlineExit( "SetTimeoutMessage error.", xBoard, &x15Authenticate, 1 ) );
	}

	printf( "\n\nTesting single byte send and single byte receive (with timestamp).\n" );
	printf( "Also testing watchdog timeout feature. This is set to timeout at 2\n" );
	printf( "seconds with the send message 'Timeout'.\n\n" );
	printf( "Press any key to send over serial port or 'C' to exit demo.\n\n" );

	while ( TRUE )
	{
		if ( !XlineReceiveByteWithTimestamp( xBoard, portNum, &dst, &interval, &bReceived ) )
		{
			return( XlineExit( "Receive byte with timestamp error.", xBoard, &x15Authenticate, 1 ) );
		}

		if ( bReceived )		/* Byte received */
		{
			printf( "Byte received: %c (%02X), timestamp = %dms.\n", dst, dst, interval );
		}

		/* Obtain parity error count */
		if ( !XlineGetParityErrors( xBoard, portNum, &parityErrors ) )
		{
			return( XlineExit( "GetParityErrors error.", xBoard, &x15Authenticate, 1 ) );
		}

		if ( parityErrors ) printf( "Parity errors: %d\n", parityErrors );

		/* Upon keystroke, test to see if it is 'C', in which case exit program. Otherwise
		   send key to serial port. */
		if ( _kbhit() )
		{
			key = getch();
			if ( ( key == 'C' ) || ( key == 'c' ) )
			{
				break;
			}
			else
			{
				/* Transmit data */
				if ( !XlineSend( xBoard, portNum, (LPBYTE)&key, 1 ) )
				{
					return( XlineExit( "Serial Send error.", xBoard, &x15Authenticate, 1 ) );
				}
				else printf( "Sent byte: %c\n", key );
			}
		}
	}

	if ( !XlineSetTimeoutMessage( xBoard, portNum, msg, 0, 0.0 ) )
	{
		return( XlineExit( "SetTimeoutMessage error.", xBoard, &x15Authenticate, 1 ) );
	}

	printf( "\n\nTesting multiple byte send and receive.\n\n" );
	printf( "Press any key between transactions, or 'C' to quit.\n\n" );

	msg[0] = '!';
	msg[TX_BUFFER_SIZE - 1] = '!';

	while ( TRUE )
	{
		/* Receive data */
		if ( !XlineReceive( xBoard, portNum, rxMsg, (LPUINT)&bytesRxd ) )
		{
			return( XlineExit( "Serial Receiveerror.", xBoard, &x15Authenticate, 1 ) );
		}

		/* Print received data */
		if ( bytesRxd > 0 )
		{
			rxMsg[bytesRxd] = '\0';		/* Terminate string */
			printf( "Received %d bytes:- [%s]\n", (int)bytesRxd, rxMsg );
		}
		else
		{
			printf( "Received no bytes.\n" );
		}

		/* Obtain parity error count */
		if ( !XlineGetParityErrors( xBoard, portNum, &parityErrors ) )
		{
			return( XlineExit( "GetParityErrors error.", xBoard, &x15Authenticate, 1 ) );
		}

		printf( "Parity error count: %d\n", parityErrors );

		/* Generate data to transmit */
		for ( c = 1; c < TX_BUFFER_SIZE - 1; c++ ) msg[c] = txChar;

		/* Print data to transmit */
		printf( "Sending '!' + '%c' * %d + '!'\n", txChar, TX_BUFFER_SIZE - 2 );

		/* Transmit data */
		if ( !XlineSend( xBoard, portNum, msg, TX_BUFFER_SIZE ) )
		{
			return( XlineExit( "Serial Send error.", xBoard, &x15Authenticate, 1 ) );
		}

		if ( ++txChar > 'Z' )
		{
			txChar = 'A';
		}

		/* Wait for keystroke. */
		key = getch();
		if ( ( key == 'C' ) || ( key == 'c' ) )
		{
			break;
		}
	}

	return( XlineExit( "Leaving program.", xBoard, &x15Authenticate, 0 ) );
}

