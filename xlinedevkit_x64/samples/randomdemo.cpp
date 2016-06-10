/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         randomdemo.cpp
*
*     Version:      1.1
*
*     Description:  Random Number Generator Demonstration
*
*     Notes:        Console application to demonstrate random number generator
*
*
*     Changes:
*
*	  1.2 RDP		Removed X15 restriction 10/01/2008
*	  1.1 RDP		Added support for X15 product 10/10/2007
*     1.0 MJB       First version. Based on issue 1.0 of
*                   X10 randomdemo.cpp, 29/7/2005
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

int test_random_auto( FireFlyUSB * FireFly, Authenticate* X15Authenticate, char key );
int test_random_manual( FireFlyUSB * FireFly, Authenticate* X15Authenticate );

int main( int argc, char* argv[] )
{
	FireFlyUSB 		FireFly;
	Authenticate	X15Authenticate;
	BYTE 			VersionProduct[20], VersionDll[20], Version8051[20], VersionPIC[20];
	BYTE 			SerialNumberPIC[20];
	BYTE 			fittedBoard;

	printf( "Firefly X10i/X15 Board\n" );
	printf( "======================\n\n" );
	printf( "This application demonstrates the random number generator\n" );
	printf( "on the X10 / X15\n\n" );
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
								X15Authenticate.BeginThread( &FireFly, EncryptedKey, 30 );
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

	char key;

	while ( TRUE )
	{
		printf( "X10 Random Menu\n" );
		printf( "===========\n\n" );

		printf( "Options:\n" );
		printf( "    'M' to fetch random numbers manually.\n" );
		printf( "    'G' to fetch random numbers by polling with GetInputs().\n" );
		printf( "    'H' to fetch random numbers by polling with GetChangedInputs().\n" );
		printf( "    'R' to fetch random numbers by polling with GetRawInputs().\n" );
		printf( "or, 'C' to exit.\n\n" );

		printf( "Please make a selection: " );

		switch ( key = toupper( getch() ) )
		{
			case 'M': test_random_manual( &FireFly, &X15Authenticate );
					  break;
			case 'G':
			case 'H':
			case 'R': test_random_auto( &FireFly, &X15Authenticate, key);
					  break;
			case 'C': return( ExitFunction( "Leaving program.", &FireFly, &X15Authenticate, 0 ) );
					  break;
		}

	}

}

int test_random_auto( FireFlyUSB * FireFly, Authenticate* X15Authenticate, char key )
{
	int 		i;
	usbInput	inputs;
	WORD 		random_number;
	time_t 		time_seed;
	#ifdef X10_LINUX_BUILD
		int 		ignore __attribute__ ((unused));
	#else
		int 		ignore;
	#endif

	// create 32 bit seed
	//printf("\nsize of time_t: %d bytes", sizeof(time_t) );
	time( &time_seed );

	printf("\nThe two random number seeds are: %ld and %ld\n", (time_seed & 0x0000ffff), ( (time_seed >> 16) & 0x0000ffff) );


	printf( "\n\nEnter the number of random numbers to poll: ");
	ignore = scanf("%d", &i);					// ignore used to quell g++ compiler warning

	if ( !FireFly->EnableRandomNumberGenerator( (DWORD)time_seed ) )
	{
		return( ExitFunction( "Get random number failed.", FireFly, X15Authenticate, 1 ) );
	}
	else
	{
		printf( "\nEnabled random number generator\n");
	}

	while(i)
	{
		switch(key)
		{
			case 'G': FireFly->GetInputs( &inputs );
					  break;
			case 'H': FireFly->GetChangedInputs( &inputs );
					  break;
			case 'R': FireFly->GetRawInputs( &inputs );
					  break;
		}

		if ( inputs.byIn[2] & 0x80)
		{
			if ( !FireFly->GetRandomNumber( &random_number ) )
				return( ExitFunction( "Get random number failed.", FireFly, X15Authenticate, 1 ) );

			printf( "%u\n", random_number);
			i--;
		}
	}

	FireFly->DisableRandomNumberGenerator();

	return 0;
}

int test_random_manual( FireFlyUSB * FireFly, Authenticate* X15Authenticate )
{
	time_t time_seed;
	WORD random_number;

	// create 32 bit seed
	//printf("\nsize of time_t: %d bytes", sizeof(time_t) );
	time( &time_seed );

	printf("\nThe two random number seeds are: %ld and %ld\n", (time_seed & 0x0000ffff), ( (time_seed >> 16) & 0x0000ffff) );

	printf( "\nPress any key to view the current random number, or 'C' to return...\n");

	FireFly->EnableRandomNumberGenerator( (DWORD)time_seed );

	printf( "Enabled random number generator\n");

	while ( TRUE )
	{
		if ( !FireFly->GetRandomNumber( &random_number ) )
			return( ExitFunction( "Get random number failed.", FireFly, X15Authenticate, 1 ) );

		printf( "%u\n", random_number);

		/* Wait for keystroke. */
		char key = getch( );
		if ( key == 'C' || key == 'c' ) break;
	}

	FireFly->DisableRandomNumberGenerator();

	return 0;
}
