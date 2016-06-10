/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         sasdemo.c
*
*     Version:      1.0
*
*     Description:  SAS Demonstration
*
*     Notes:        Console application to demonstrate SAS
*
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

int main( int argc, char* argv[] )
{
	void			*xBoard;
	Authenticate	x15Authenticate;
	BYTE 			VersionProduct[20]={0}, VersionDll[20]={0}, Version8051[20]={0}, VersionPIC[20]={0};
	BYTE 			SerialNumberPIC[20]={0}, SerialNumberDallas[20]={0};
	BYTE 			crcValid;
	BOOL 			messageSuccess;
	BYTE 			fittedBoard;
	DCB 			config;								/* Configure device */

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "This application demonstrates the SAS functionality\n" );
	printf( "microcontroller..\n\n" );
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

	if ( !XlineGetProductVersion( xBoard, VersionProduct ) )
	{
		return( XlineExit( "Failure obtaining product version.", xBoard, &x15Authenticate, 1 ) );
	}

	if ( !XlineGetDllVersion( xBoard, VersionDll ) )
	{
		return( XlineExit( "Failure obtaining DLL version.", xBoard, &x15Authenticate, 1 ) );
	}

	if ( !XlineGet8051Version( xBoard, Version8051 ) )
	{
		return( XlineExit( "Failure obtaining 8051 version.", xBoard, &x15Authenticate, 1 ) );
	}

	if ( !XlineGetPICVersion( xBoard, VersionPIC ) )
	{
		return( XlineExit( "Failure obtaining PIC version.", xBoard, &x15Authenticate, 1 ) );
	}

	if ( !XlineGetPICSerialNumber( xBoard, SerialNumberPIC ) )
	{
		return( XlineExit( "Failure obtaining PIC serial number.", xBoard, &x15Authenticate, 1 ) );
	}

	if ( !XlineGetDallasSerialNumber( xBoard, SerialNumberDallas, (LPBYTE)&crcValid ) )
	{
		return( XlineExit( "Failure obtaining PIC serial number.", xBoard, &x15Authenticate, 1 ) );
	}

	printf( "Product version information:\n" );
	printf( "    Product version       : %s\n",   VersionProduct );
	printf( "    API library version   : %s\n",   VersionDll );
	printf( "    8051 software version : %s\n",   Version8051 );
	printf( "    PIC software version  : %s\n",   VersionPIC );
	printf( "    PIC serial number     : %s\n", SerialNumberPIC );
	printf( "    Dallas serial number  : %s, crcValid = %d\n\n", SerialNumberDallas, crcValid );

	// Configure PORT_A, 19200,N
	printf( "Configuring serial PORT_A for 19200 baud and no parity to test SAS functions\n" );
	config.BaudRate = 19200;
	config.Parity = NOPARITY;
	config.fRtsControl = RTS_CONTROL_TOGGLE;
	config.fOutxCtsFlow = FALSE;
	if ( !XlineSetConfig( xBoard, PORT_A, &config, PORT_RS232 ) )
	{
		return( XlineExit( "Serial port SetConfig failed.", xBoard, &x15Authenticate, 1 ) );
	}

	printf( "SetSASMachineAddress\n" );
	if ( !XlineSetSASMachineAddress( xBoard, PORT_A, 112 ) )
	{
		return( XlineExit( "SetSASMachineAddress failed.", xBoard, &x15Authenticate, 1 ) );
	}


	printf( "SetSASAutoReply\n" );
	if ( !XlineSetSASAutoReply( xBoard, PORT_A, TRUE, 112 ) )
	{
		return( XlineExit( "SetSASAutoReply failed.", xBoard, &x15Authenticate, 1 ) );
	}

	printf( "SetSASBusy\n" );
	if ( !XlineSetSASBusy( xBoard, PORT_A, TRUE ) )
	{
		return( XlineExit( "SetSASBusy failed.", xBoard, &x15Authenticate, 1 ) );
	}


	printf( "GetSASMessageStatus\n" );
	if ( !XlineGetSASMessageStatus( xBoard, PORT_A, &messageSuccess ) )
	{
		return( XlineExit( "GetSASMessageStatus failed.", xBoard, &x15Authenticate, 1 ) );
	}

	return( XlineExit( "Leaving program.", xBoard, &x15Authenticate, 0 ) );
}
