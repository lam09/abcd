/**********************************************************************
*
*     Project:      Firefly X10i Board
*
*     File:         UnlockIO.h
*
*     Version:      1.0
*
*     Description:  Unlocks the X10i.
*
*     Changes:
*     1.0 MJB       First version, 8/9/2005
*
**********************************************************************/

#ifndef UNLOCKIO_H
#define UNLOCKIO_H

BOOL UnlockX10( FireFlyUSB * FireFly );
BOOL UnlockX10Recheck( FireFlyUSB * FireFly );
BOOL VerifyX10UnlockLibrary( FireFlyUSB * FireFly );

#endif



