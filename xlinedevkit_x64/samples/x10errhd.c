/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         x10errhd.c
*
*     Version:      1.0
*
*     Description:  Exit function code which is shared between
*                   all X10 console demos.
*
*     Changes:
*     1.0 MJB       First version. Based on issue 1.0 of
*                   X10 x10errhd.c, 29/7/2005
*
*****************************************************************/

#include "fflyusb.h"
#ifndef X10_LINUX_BUILD
	#include "authenticate_win.h"
#else
	#include "authenticate_linux.h"
#endif


//int ExitFunction( char *message, FireFlyUSB *FireFly, Authenticate* X15Authenticate, int retCode )
int ExitFunction( const char message[], FireFlyUSB *FireFly, Authenticate* X15Authenticate, int retCode )
{
	printf( "%s\n", message );
	if ( FireFly->GetLastError( ) != USB_MESSAGE_EXECUTION_SUCCESS )
	{
		printf( "X10 Error Code: %d\n", (int)FireFly->GetLastError( ) );
	}

	// Clean up the authentication thread
	if ( X15Authenticate )
	{
		X15Authenticate->KillThread( );
	}

	// Close device.
	if ( !FireFly->close( ) )
	{
		printf( "\nError closing device - %d.\n\n", (int)FireFly->GetLastError( ) );
	}

	#ifdef X10_LINUX_BUILD
	CloseConsole( );
	#endif

//	return( retCode );
	exit( retCode );
}

