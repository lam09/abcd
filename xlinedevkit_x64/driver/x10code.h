/****************************************************************************
*
*     X-line Linux Kernel driver
*     File:    X10Code.h
*     Version: 1.0
*     Copyright 2007 Heber Limited (http://www.heber.co.uk)
*
*     This program is free software; you can redistribute it and/or modify it
*     under the terms of the GNU General Public License as published by the
*     Free Software Foundation; either version 2, or (at your option) any
*     later version.
*
*     This program is distributed in the hope that it will be useful, but
*     WITHOUT ANY WARRANTY; without even the implied warranty of
*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*     General Public License for more details.
*
*     You should have received a copy of the GNU General Public License along
*     with this program; if not, write to the Free Software Foundation, Inc.,
*     675 Mass Ave, Cambridge, MA 02139, USA.
*
*     Changes:
*     1.0 MJB       First version, 30/9/2003
*
****************************************************************************/

#ifndef X10CODE_H
#define X10CODE_H

#define MAX_FILE_NAME                 128
#define MAX_FILE_SIZE                 (64 * 1024)
#define MAX_INTEL_HEX_RECORD_LENGTH   16
#define INVALID_HANDLE_VALUE          -1

// Define the pipes to be no larger than the physical pipe available. A smaller value will
// save Cypress SRAM space (xdata) but reduce throughput per USB frame:
#define USB_2_0_PIPE_SIZE				256
#define USB_1_1_PIPE_SIZE				64

typedef struct _INTEL_HEX_RECORD
{
   unsigned char  Length;
   unsigned short Address;
   unsigned char  Type;
   unsigned char  Data[MAX_INTEL_HEX_RECORD_LENGTH];
} INTEL_HEX_RECORD, *PINTEL_HEX_RECORD;


typedef struct _X10_COMMAND
{
	int  CommandpipeNum;
	int  AnswerpipeNum;
	int  CommandLength;
	int  AnswerLength;
	char Buffer[512];						// In order to prevent ioctl error, must set USB
											// buffer length to 512 bytes. However, in USB 2
											// high speed, the maximum acceptible packet size
											// for XLine boards is defined in USB_2_0_PIPE_SIZE
}X10_COMMAND, *PX10_COMMAND;

struct X10I_COMMAND
{
    int  *   error_code             ;
    int virtual_pipe_number         ;
    unsigned char * user_tx_buffer  ;
    unsigned long user_tx_buffer_size      ;
    unsigned char * user_rx_buffer  ;
    unsigned long *  user_rx_buffer_size     ;
    unsigned char command           ;
} ;

#define X10_IOC_MAGIC                 0x81
#define X10_IOC_READ_MAGIC            0x82
#define X10_IOC_WRITE_MAGIC           0x83
#define X10I_COMMAND_MAGIC            0x84

#define IOCTL_X10_COMMAND             _IOWR(X10_IOC_MAGIC,0,X10_COMMAND)
#define IOCTL_X10_READ                _IOWR(X10_IOC_READ_MAGIC,0,X10_COMMAND)
#define IOCTL_X10_WRITE               _IOWR(X10_IOC_WRITE_MAGIC,0,X10_COMMAND)
#define IOCTL_X10I_COMMAND            _IO(X10I_COMMAND_MAGIC,0)

#endif

