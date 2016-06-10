/*****************************************************************
*
*     Project:      X10i/X15 Board
*
*     File:         authenticate_linux.cpp
*
*     Version:      1.1
*
*     Description:  X15 authentication thread class
*
*     Changes:
*
*	  1.1 RDP		Modified from X20 for use with X15, 10/10/2007
*     1.0 MJB       First version, 27/2/2007
*
*****************************************************************/

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "authenticate_linux.h"

#define SC_RESET_DELAY_MS				300
#define SETSESSIONKEY_DELAY_MS			300

/*-------------------------------------------------------------------
Name......... AuthenticateFunction()
Author....... Mike Ballinger
Date......... 27/2/2007
Description.. The actual authentication thread.
Parameters... ptr - A pointer to the Authentication class.
Return....... None
History...... - Initial release (MJB 27/2/07)
-------------------------------------------------------------------*/
void *AuthenticateFunction( void *ptr )
{
	Authenticate *authenticate;

	authenticate = (Authenticate *)ptr;

	while( authenticate->ContinueAuthentication )
	{
		printf( "Authenticating ...\n" );

		if ( !authenticate->X15->Authenticate( authenticate->Key, authenticate->SessionID ) )
		{
			authenticate->ContinueAuthentication = false;

			printf( "Error has occurred in authentication thread: %d\n", authenticate->X15->GetLastError( ) );

			// We have encountered an error. Set the authentication error flag.
			authenticate->ErrorOccurred = true;
		}
		else
		{
			authenticate->SuccessfulAuthentications ++;
			sleep( authenticate->FreqSeconds );

			authenticate->TotalRuntime += authenticate->FreqSeconds;
		}
	}

	pthread_exit( 0 );
}

/*-------------------------------------------------------------------
Name......... Authenticate()
Author....... Mike Ballinger
Date......... 27/2/2007
Description.. Class constructor.
Parameters... None
Return....... None
History...... - Initial release (MJB 27/2/07)
-------------------------------------------------------------------*/
Authenticate::Authenticate( )
{
	X15 = NULL;
	ErrorOccurred = false;
}

/*-------------------------------------------------------------------
Name......... ~Authenticate()
Author....... Mike Ballinger
Date......... 27/2/2007
Description.. Class destructor.
Parameters... None
Return....... None
History...... - Initial release (MJB 27/2/07)
-------------------------------------------------------------------*/
Authenticate::~Authenticate( )
{
	if ( X15 )
	{
		KillThread( );
	}
}

/*-------------------------------------------------------------------
Name......... BeginThread()
Author....... Mike Ballinger
Date......... 27/2/2007
Description.. Starts the authentication thread running in the
............. background.
Parameters... ffly - A pointer to the FireFlyUSB class.
............. keys - The encrypted 192-bit key to use.
............. freqSeconds - Number of seconds between authentication
............. cycles.
Return....... None
History...... - Initial release (MJB 27/2/07)
-------------------------------------------------------------------*/
void Authenticate::BeginThread( FireFlyUSB* ffly, unsigned char *keys, unsigned int freqSeconds )
{
	int i;
	unsigned char smartcardMessage[50];
	unsigned int length, sw1sw2;

	X15 = ffly;

	for ( i = 0; i < DES_KEY_LENGTH; i ++ )
	{
		Key[i] = keys[i];
	}

	FreqSeconds = freqSeconds;
	SessionID = 0x123456;
	SuccessfulAuthentications = 0;
	TotalRuntime = 0;
	ContinueAuthentication = true;

	if ( !X15->SmartcardReset( smartcardMessage, &length, SC_RESET_DELAY_MS ) )
	{
		ErrorOccurred = true;
	}
	else
	{
		if ( !X15->SmartcardSetSessionID( SessionID, &sw1sw2, SETSESSIONKEY_DELAY_MS ) )
		{
			ErrorOccurred = true;
		}
		else
		{
			pthread_create( &AuthenticateThread, NULL, AuthenticateFunction, this );
		}
	}
}

/*-------------------------------------------------------------------
Name......... KillThread()
Author....... Mike Ballinger
Date......... 27/2/2007
Description.. Kills the authentication thread.
Parameters... None
Return....... None
History...... - Initial release (MJB 27/2/07)
-------------------------------------------------------------------*/
void Authenticate::KillThread( void )
{
	ContinueAuthentication = false;

	// Wait for thread to terminate
//	pthread_join( AuthenticateThread, NULL );

	X15 = NULL;
}

/*-------------------------------------------------------------------
Name......... AuthenticationSuccessCount()
Author....... Mike Ballinger
Date......... 27/2/2007
Description.. Returns the total number of successful authentication
............. cycles.
Parameters... None
Return....... Number of authentication cycles.
History...... - Initial release (MJB 27/2/07)
-------------------------------------------------------------------*/
unsigned int Authenticate::AuthenticationSuccessCount( void )
{
	return( SuccessfulAuthentications );
}

/*-------------------------------------------------------------------
Name......... ErrorStatus()
Author....... Mike Ballinger
Date......... 27/2/2007
Description.. Obtains error status.
Parameters... None
Return....... 'true' if an error has occurred (therefore the
............. authentication thread will have stopped) or 'false'
............. if no error.
History...... - Initial release (MJB 27/2/07)
-------------------------------------------------------------------*/
bool Authenticate::ErrorStatus( void )
{
	return( ErrorOccurred );
}
