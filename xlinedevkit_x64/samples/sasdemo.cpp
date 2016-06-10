/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         sasdemo.cpp
*
*     Version:      1.1
*
*     Description:  SAS Demonstration
*
*     Notes:        Console application to demonstrate SAS
*
*
*     Changes:
*
*	  1.1 RDP		Added support for X15 product 10/10/2007
*     1.0 MJB       First version. Based on issue 1.2
*                   of X10 timedemo.cpp, 29/7/2005
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

int main( int argc, char* argv[] )
{
	FireFlyUSB 		FireFly;
	Authenticate	X15Authenticate;
	BYTE 			VersionProduct[20]={0}, VersionDll[20]={0}, Version8051[20]={0}, VersionPIC[20]={0};
	BYTE 			SerialNumberPIC[20]={0}, SerialNumberDallas[20]={0};
	BYTE 			crcValid=0;
	BOOL 			messageSuccess=FALSE;
	BYTE 			fittedBoard=0;
	DCB 			config={0};							// Configure device

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "This application demonstrates the SAS functionality\n" );
	printf( "microcontroller..\n\n" );
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

	if ( !FireFly.GetProductVersion( VersionProduct ) )
		return( ExitFunction( "Failure obtaining product version.", &FireFly, &X15Authenticate, 1 ) );

	if ( !FireFly.GetDllVersion( VersionDll ) )
		return( ExitFunction( "Failure obtaining DLL version.", &FireFly, &X15Authenticate, 1 ) );

	if ( !FireFly.Get8051Version( Version8051 ) )
		return( ExitFunction( "Failure obtaining 8051 version.", &FireFly, &X15Authenticate, 1 ) );

	if ( !FireFly.GetPICVersion( VersionPIC ) )
		return( ExitFunction( "Failure obtaining PIC version.", &FireFly, &X15Authenticate, 1 ) );

	if ( !FireFly.GetPICSerialNumber( SerialNumberPIC ) )
		return( ExitFunction( "Failure obtaining PIC serial number.", &FireFly, &X15Authenticate, 1 ) );

	if ( !FireFly.GetDallasSerialNumber( SerialNumberDallas, (LPBYTE)&crcValid ) )
		return( ExitFunction( "Failure obtaining PIC serial number.", &FireFly, &X15Authenticate, 1 ) );

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
	if ( !FireFly.SetConfig( PORT_A, &config, PORT_RS232 ) )
		return( ExitFunction( "Serial port SetConfig failed.", &FireFly, &X15Authenticate, 1 ) );

	printf( "SetSASMachineAddress\n" );

//	if ( !FireFly.SetSASMachineAddress( PORT_A, 112 ) )
	if ( !FireFly.SetSASMachineAddress( (usbSerialPort)0, 112 ) )
	{
printf("calling ExitFunction() \n");
		return( ExitFunction( "SetSASMachineAddress failed.", &FireFly, &X15Authenticate, 1 ) );
	}

	printf( "SetSASAutoReply\n" );
	if ( !FireFly.SetSASAutoReply( PORT_A, TRUE, 112 ) )
		return( ExitFunction( "SetSASAutoReply failed.", &FireFly, &X15Authenticate, 1 ) );

	printf( "SetSASBusy\n" );
	if ( !FireFly.SetSASBusy( PORT_A, TRUE ) )
		return( ExitFunction( "SetSASBusy failed.", &FireFly, &X15Authenticate, 1 ) );


	printf( "GetSASMessageStatus\n" );
	if ( !FireFly.GetSASMessageStatus( PORT_A, &messageSuccess ) )
		return( ExitFunction( "GetSASMessageStatus failed.", &FireFly, &X15Authenticate, 1 ) );

	return( ExitFunction( "Leaving program.", &FireFly, &X15Authenticate, 0 ) );
}
