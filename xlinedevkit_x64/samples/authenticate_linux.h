/*****************************************************************
*
*     Project:      X10i/X15 Board
*
*     File:         authenticate_linux.h
*
*     Version:      1.1
*
*     Description:  X15 authentication thread class
*
*     Changes:
*
*	  1.1 RDP		Modified from X20 for use with X15 10/10/2007
*     1.0 MJB       First version (for X20), 27/2/2007
*
*****************************************************************/

#ifndef AUTHENTICATELINUX_H
#define AUTHENTICATELINUX_H

/* include project files */
#include "fflyusb.h"

/* project difinitions */
#define DES_KEY_LENGTH				24		// bytes

/* -------------------------------------------------------------------------------------------------------
   Class declaration
   ------------------------------------------------------------------------------------------------------*/
class Authenticate
{
public:
/* class construction */
					Authenticate				( void );
	virtual 		~Authenticate				( void );

/* public methods */
	void 			BeginThread					( FireFlyUSB* ffly, unsigned char *keys, unsigned int freqSeconds );
	void 			KillThread					( void );
	unsigned int 	AuthenticationSuccessCount	( void );
	bool 			ErrorStatus					( void );

/* public properties */
	FireFlyUSB*		X15;
	unsigned char 	Key[DES_KEY_LENGTH];
	unsigned int 	FreqSeconds;
	unsigned int 	SessionID;
	unsigned int 	SuccessfulAuthentications;
	unsigned int 	TotalRuntime;
	bool 			ContinueAuthentication;
	bool 			ErrorOccurred;
	pthread_t 		AuthenticateThread;
};

#endif
