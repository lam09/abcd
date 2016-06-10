/*****************************************************************
*
*     Project:      X15 Board
*
*     File:         xlineauthenticate.c
*
*     Version:      1.0
*
*     Description:  X15 authentication thread
*
*     Changes:
*     1.0 MJB       First version (for X20), 18/1/2008
*     1.0 JSH       First properly released version using true C code and C-API, 11/10/2011.
*
*****************************************************************/

#include <stdio.h>
#ifdef X10_LINUX_BUILD
	#include <unistd.h>
	#define TRUE 1						/* needed for 'C' */
	#define FALSE 0
#endif
#include "xlinecapi.h"
#include "xlineauthenticate.h"

#define SC_RESET_DELAY_MS				300
#define SETSESSIONKEY_DELAY_MS			300

/*-------------------------------------------------------------------
Name......... AuthenticateFunction()
Author....... Mike Ballinger
Date......... 27/2/2007
Description.. The actual authentication thread.
Parameters... ptr - A pointer to the Authentication structure.
Return....... None
History...... - Initial release (MJB 27/2/07)
-------------------------------------------------------------------*/

#if defined(X10_LINUX_BUILD)
	void *AuthenticateFunction( void *ptr )
#else
	DWORD WINAPI AuthenticateFunction( void *ptr )
#endif
{
	Authenticate *authenticate;

	authenticate = (Authenticate *)ptr;

	while( authenticate->ContinueAuthentication )
	{
		printf( "Authenticating ... delay = %d\n", authenticate->FreqSeconds );

		if ( !XlineAuthenticate( authenticate->X15, authenticate->Key, authenticate->SessionID ) )
		{
			authenticate->ContinueAuthentication = FALSE;

			/* We have encountered an error. Set the authentication error flag.*/
			authenticate->ErrorOccurred = TRUE;
			printf( "Authentication failed\n" );
		}
		else
		{
			authenticate->SuccessfulAuthentications ++;

			#if defined(X10_LINUX_BUILD)
				sleep( authenticate->FreqSeconds );
			#else
				Sleep( authenticate->FreqSeconds * 1000 );
			#endif
		}
	}

	#if defined(X10_LINUX_BUILD)
		pthread_exit( 0 );
	#else
		return( 1 );
	#endif
}

/*-------------------------------------------------------------------
Name......... AuthenticateBeginThread()
Author....... Mike Ballinger
Date......... 27/2/2007
Description.. Starts the authentication thread running in the
............. background.
Parameters... x15Board - A pointer to the Xline board structure
............. keys - The encrypted 192-bit key to use.
............. freqSeconds - Number of seconds between authentication
............. cycles.
Return....... Authentication structure
History...... - Initial release (MJB 27/2/07)
-------------------------------------------------------------------*/
Authenticate AuthenticateBeginThread( void *x15Board, unsigned char *keys, unsigned int freqSeconds )
{
	int i;
	unsigned char smartcardMessage[50];
	unsigned int length, sw1sw2;
	Authenticate authenticate;

	for ( i = 0; i < DES_KEY_LENGTH; i ++ )
	{
		authenticate.Key[i] = keys[i];
	}

	authenticate.X15 = x15Board;
	authenticate.ErrorOccurred = FALSE;
	authenticate.FreqSeconds = freqSeconds;
	authenticate.SessionID = 0x123456;
	authenticate.SuccessfulAuthentications = 0;
	authenticate.ContinueAuthentication = TRUE;

	if ( !XlineSmartcardReset( authenticate.X15, smartcardMessage, &length, SC_RESET_DELAY_MS ) )
	{
		authenticate.ErrorOccurred = TRUE;
	}
	else
	{
		if ( !XlineSmartcardSetSessionID( authenticate.X15, authenticate.SessionID, &sw1sw2, SETSESSIONKEY_DELAY_MS ) )
		{
			authenticate.ErrorOccurred = TRUE;
		}
		else
		{
			#if defined(X10_LINUX_BUILD)
				pthread_create( &authenticate.AuthenticateThread, NULL, AuthenticateFunction, (void *)&authenticate );
			#else
				DWORD threadId;
				authenticate.AuthenticateThread = CreateThread(	NULL,
																0,
																AuthenticateFunction,
																(void *)&authenticate,
																0,
																&threadId );
			#endif
		}
	}

	return( authenticate );
}

/*-------------------------------------------------------------------
Name......... AuthenticateKillThread()
Author....... Mike Ballinger
Date......... 27/2/2007
Description.. Kills the authentication thread.
Parameters... Authentication structure
Return....... None
History...... - Initial release (MJB 27/2/07)
-------------------------------------------------------------------*/
void AuthenticateKillThread( Authenticate *authenticate )
{
	authenticate->ContinueAuthentication = FALSE;

	#ifndef X10_LINUX_BUILD
		/* Wait for thread to terminate */
		CloseHandle( authenticate->AuthenticateThread );
	#endif

	authenticate->X15 = NULL;
}

/*-------------------------------------------------------------------
Name......... AuthenticateSuccessCount()
Author....... Mike Ballinger
Date......... 27/2/2007
Description.. Returns the total number of successful authentication
............. cycles.
Parameters... Authentication structure
Return....... Number of authentication cycles.
History...... - Initial release (MJB 27/2/07)
-------------------------------------------------------------------*/
unsigned int AuthenticateSuccessCount( Authenticate *authenticate )
{
	return( authenticate->SuccessfulAuthentications );
}

/*-------------------------------------------------------------------
Name......... AuthenticateErrorStatus()
Author....... Mike Ballinger
Date......... 27/2/2007
Description.. Obtains error status.
Parameters... Authentication structure
Return....... 'true' if an error has occurred (therefore the
............. authentication thread will have stopped) or 'false'
............. if no error.
History...... - Initial release (MJB 27/2/07)
-------------------------------------------------------------------*/
BOOL AuthenticateErrorStatus( Authenticate *authenticate )
{
	return( authenticate->ErrorOccurred );
}
