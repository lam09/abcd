/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         randomdemo.c
*
*     Version:      1.0
*
*     Description:  Random Number Generator Demonstration
*
*     Notes:        Console application to demonstrate random number generator
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

int test_random_auto( void *xBoard, Authenticate *X15Authenticate, char key );
int test_random_manual( void *xBoard, Authenticate *X15Authenticate );

int main( int argc, char* argv[] )
{
	void            *xBoard;
	Authenticate	x15Authenticate;
	BYTE 			VersionProduct[20]={0}, VersionDll[20]={0}, Version8051[20]={0}, VersionPIC[20]={0};
	BYTE 			SerialNumberPIC[20]={0};
	BYTE 			fittedBoard=0;
	char 			key;

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "This application demonstrates the random number generator\n\n" );
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

	printf( "Product version information:\n" );
	printf( "    Product version       : %s\n",   VersionProduct );
	printf( "    API library version   : %s\n",   VersionDll );
	printf( "    8051 software version : %s\n",   Version8051 );
	printf( "    PIC software version  : %s\n",   VersionPIC );
	printf( "    PIC serial number     : %s\n\n", SerialNumberPIC );


	#ifdef X10_LINUX_BUILD
		InitialiseConsole( );
		EnableGetchNewline( );
	#endif

	while ( TRUE )
	{
		printf( "Options:\n" );
		printf( "    'M' to fetch random numbers manually.\n" );
		printf( "    'G' to fetch random numbers by polling with GetInputs().\n" );
		printf( "    'H' to fetch random numbers by polling with GetChangedInputs().\n" );
		printf( "    'R' to fetch random numbers by polling with GetRawInputs().\n" );
		printf( "or, 'C' to exit.\n\n" );

		printf( "Please make a selection: " );

		switch ( key = toupper( getch() ) )
		{
			case 'M': test_random_manual( xBoard, &x15Authenticate );
					  break;
			case 'G':
			case 'H':
			case 'R': test_random_auto( xBoard, &x15Authenticate, key);
					  break;
			case 'C': return( XlineExit( "Leaving program.", xBoard, &x15Authenticate, 0 ) );
					  break;
		}

	}

}

int test_random_auto( void *xBoard, Authenticate *x15Authenticate, char key )
{
	int 		i;
	usbInput	inputs;
	WORD 		random_number;
	time_t 		time_seed;

	#ifdef X10_LINUX_BUILD
		int			ignore __attribute__ ((unused));
	#else
		int 		ignore;
	#endif


	/* create 32 bit seed */
	time( &time_seed );

	printf( "\nThe two random number seeds are: %ld and %ld\n", (time_seed & 0x0000ffff), ( (time_seed >> 16) & 0x0000ffff) );


	printf( "\n\nEnter the number of random numbers to poll: ");
	ignore = scanf( "%d", &i );						// ignore used to quell gcc compiler warning

	if ( !XlineEnableRandomNumberGenerator( xBoard, (DWORD)time_seed ) )
	{
		return( XlineExit( "Get random number failed.", xBoard, x15Authenticate, 1 ) );
	}
	else
	{
		printf( "\nEnabled random number generator\n");
	}

	while(i)
	{
		switch( key )
		{
			case 'G': XlineGetInputs( xBoard, &inputs );
					  break;
			case 'H': XlineGetChangedInputs( xBoard, &inputs );
					  break;
			case 'R': XlineGetRawInputs( xBoard, &inputs );
					  break;
		}

		if ( inputs.byIn[2] & 0x80)
		{
			if ( !XlineGetRandomNumber( xBoard, &random_number ) )
			{
				return( XlineExit( "Get random number failed.", xBoard, x15Authenticate, 1 ) );
			}

			printf( "%u\n", random_number );
			i--;
		}
	}

	XlineDisableRandomNumberGenerator( xBoard );

	return( 0 );
}

int test_random_manual( void *xBoard, Authenticate *x15Authenticate )
{
	time_t time_seed;
	WORD random_number;
	char key;

	/* create 32 bit seed */
	time( &time_seed );

	printf("\nThe two random number seeds are: %ld and %ld\n", (time_seed & 0x0000ffff), ( (time_seed >> 16) & 0x0000ffff) );

	printf( "\nPress any key to view the current random number, or 'C' to return...\n");

	XlineEnableRandomNumberGenerator( xBoard, (DWORD)time_seed );

	printf( "Enabled random number generator\n");

	while ( TRUE )
	{
		if ( !XlineGetRandomNumber( xBoard, &random_number ) )
		{
			return( XlineExit( "Get random number failed.", xBoard, x15Authenticate, 1 ) );
		}

		printf( "%u\n", random_number);

		/* Wait for keystroke. */
		key = getch( );
		if ( key == 'C' || key == 'c' ) break;
	}

	XlineDisableRandomNumberGenerator( xBoard );

	return( 0 );
}
