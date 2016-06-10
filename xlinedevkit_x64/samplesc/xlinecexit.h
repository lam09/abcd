/*****************************************************************
*
*     Project:      Firefly X10i/X15 Board
*
*     File:         xlinecexit.c
*
*     Version:      1.0
*
*     Description:  Exit function code which is shared between
*                   all C X-line console demos.
*
*     Changes:
*     1.0 MJB       First version, 15/1/2008
*
*****************************************************************/

#ifndef XLINECEXIT__H
#define XLINECEXIT__H

#include "xlineauthenticate.h"

int XlineExit( const char message[], void *xBoard, Authenticate *x15Authenticate, int retCode );

#endif
