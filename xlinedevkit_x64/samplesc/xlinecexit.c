/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         xlinecexit.c
*
*     Version:      1.0
*
*     Description:  Exit function code which is shared between
*                   all X10 console demos.
*
*     Changes:
*     1.0 MJB       First version, 15/1/2008
*     1.0 JSH       First properly released version using true C code and C-API, 11/10/2011.
*
*****************************************************************/

#include <stdio.h>
#include "xlinecapi.h"
#include "xlinecexit.h"

#if defined(X10_LINUX_BUILD)
	/* LINUX specific build */
	#include <ctype.h>
	#include "linuxkb.h"
/*	#define TRUE 1	*/					/* needed for 'C' */
/*	#define FALSE 0 */
#else
	/* WIN32 specific build */
	#include <conio.h>

	/* Remove "scanf" and "getch" warnings when compiling using Visual V++ 2005 */
	#pragma warning( disable : 4996 )
#endif


int XlineExit( const char message[], void *xBoard, Authenticate *x15Authenticate, int retCode )
{
	printf( "%s\n", message );
	if ( XlineGetLastError( xBoard ) != USB_MESSAGE_EXECUTION_SUCCESS )
	{
		printf( "X-line Error Code: %d\n", (int)XlineGetLastError( xBoard ) );
	}

	/* Clean up the authentication thread */
	if ( x15Authenticate )
	{
		AuthenticateKillThread( x15Authenticate );
	}

	/* Close device. */
	if ( !XlineFreeBoard( xBoard ) )
	{
		printf( "\nError closing device - %d.\n\n", (int)XlineGetLastError( xBoard ) );
	}

	#ifdef X10_LINUX_BUILD
		CloseConsole( );
	#endif

	return( retCode );
}
