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

/* LINUX specific build */
	#include <ctype.h>
	#include "linuxkb.c"
	#include "authenticate_linux.h"

#include "fflyusb.h"
#include "unlockio.h"
#include "x10errhd.c"
#include "x15key.h"

int main(int argc, char* argv[])
{
	FireFlyUSB 		FireFly;
	Authenticate	X15Authenticate;

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "This application demonstrates X10 IO.\n\n" );
	printf( "Establishing link with FireFly USB device..." );

/* initialise the firefly device */
	// Get a handle to the device
	if ( !FireFly.init( ) )
		return( ExitFunction( "initialisation failed.", &FireFly, &X15Authenticate, 1 ) );

	UnlockX10( &FireFly );

	printf( "success.\n\n" );


	//return( ExitFunction( "Leaving program.", &FireFly, &X15Authenticate, 0 ) );
}



