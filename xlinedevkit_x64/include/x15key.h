/*****************************************************************
*
*     Project:      Axis X15 Board
*
*     File:         x15key.h
*
*     Version:      1.1
*
*     Description:  Contains the encrypted key required for
*                   authentication.
*
*     Changes:
*
*	  1.1 RDP		Modified from X20 for use with X15, 10/10/2007
*     1.0 MJB       First version, 15/3/07
*
*****************************************************************/

#ifndef X15KEY__H
#define X15KEY__H

/* X15 Generic Encrypted Key */
unsigned char EncryptedKey[ 24 ] = {
	0xC6, 0x30, 0x9E, 0xBD, 0x37, 0x94, 0x05, 0xA5,
	0x2E, 0x82, 0x51, 0x58, 0xAE, 0x97, 0x9B, 0x0E,
	0xF8, 0x8E, 0x7A, 0xC6, 0xA2, 0xCC, 0xD7, 0xF5
};

#endif
