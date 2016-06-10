/*****************************************************************
*
*     Project:      X15 Board
*
*     File:         xlineauthenticate.h
*
*     Version:      1.0
*
*     Description:  X15 authentication thread
*
*     Changes:
*     1.0 MJB       First version (for X20), 18/1/2008
*
*****************************************************************/

#ifndef XLINEAUTHENTICATE__H
#define XLINEAUTHENTICATE__H

#if defined(X10_LINUX_BUILD)
	#include "pthread.h"

	#define THREAD_TYPE					pthread_t
#else
	#include <windows.h>

	#define THREAD_TYPE					HANDLE
#endif

/* project difinitions */
#define DES_KEY_LENGTH				24		/* bytes */

typedef struct
{
	void			*X15;
	unsigned char 	Key[DES_KEY_LENGTH];
	unsigned int 	FreqSeconds;
	unsigned int 	SessionID;
	unsigned int 	SuccessfulAuthentications;
	BOOL 			ContinueAuthentication;
	BOOL 			ErrorOccurred;
	THREAD_TYPE		AuthenticateThread;
} Authenticate;

Authenticate AuthenticateBeginThread(  void *x15Board, unsigned char *keys, unsigned int freqSeconds );
void         AuthenticateKillThread(   Authenticate *authenticate );
unsigned int AuthenticateSuccessCount( Authenticate *authenticate );
BOOL         AuthenticateErrorStatus(  Authenticate *authenticate );

#endif
