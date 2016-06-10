/***************************************************************************
*
*     Project:      X-line Board Range API
*
*     File:         x10err.h
*
*     Version:      1.4
*
*     Description:  Error code definitions
*
*     Changes:
*	  1.4 RDP		Added RecordingStatus enumuration
*					  23-10-2007
*     1.3 RDP		Added USB_MESSAGE_FUNCTION_NOT_SUPPORTED_ON_X15,
*						  USB_MESSAGE_SMARTCARD_SW1SW2_ERROR,
*						  10/07/2007
*	  1.2 RDP		Added USB_MESSAGE_ILLEGAL_POINTER to usbErrorCode,
*					Added InitJam, Jam to ReleaseHopperCoinsStatus,
*					see requrements doc 80-19636-2, 10-05-2007
*     1.1 MJB       Added 'USB_MESSAGE_SAS_ADDRESS_INTERRUPT' return
*                   code, 27/3/06
*     1.0 MJB       First version.
*                   Based on X10 x10err.h v1.0, 19/7/2005
*
***************************************************************************/

/********    Header Sentry    ********/
#ifndef X10IDEFS_H
#define X10IDEFS_H

/**************************************************************************/
/* Generic Type Definitions                                               */
/**************************************************************************/

/********    Types    ********/
/* typedef unsigned int             BOOL;		// is in sdk windef.h */
/* typedef unsigned char            BYTE;	*/
/* typedef unsigned short           WORD;	*/

#ifdef X10_LINUX_BUILD
	typedef unsigned int             BOOL;
	typedef unsigned int *           LPBOOL;
	typedef unsigned char            UCHAR;
	typedef unsigned char            BYTE;
	typedef unsigned short           USHORT;
	typedef unsigned short           WORD;
	typedef unsigned long            ULONG;
	typedef unsigned char *          LPBYTE;
	typedef unsigned short *         LPWORD;
	typedef unsigned long            DWORD;
	typedef unsigned long *          LPDWORD;
/*	typedef ULONG_PTR		         LPDWORD; */
	typedef unsigned int             UINT;
	typedef long unsigned int *      LPUINT;
	typedef int                      HANDLE;

	typedef struct _DCB
	{
	    DWORD BaudRate;
	    __extension__ DWORD fParity: 1;					/* type bit-field is GCC extension to "C"; __extension__ this suppresses warning msg*/
	    __extension__ DWORD fOutxCtsFlow: 1;
	    __extension__ DWORD fRtsControl: 2;
	    BYTE Parity;
	} DCB, *LPDCB;
#endif


#ifndef TRUE
	#define TRUE                  	1	/* make '1' and use C rather than C++ comments for correct C90 conformance */
#endif
#ifndef FALSE
	#define FALSE                	0	/* make '0' and use C rather than C++ comments for correct C90 conformance */
#endif

#ifndef min
	#define min(a, b)                 ((a) < (b) ? (a) : (b))
#endif
#ifndef max
	#define max(a, b)                 ((a) > (b) ? (a) : (b))
#endif

#ifndef bmBIT0
	#define bmBIT0 					0x0001
#endif
#ifndef bmBIT1
	#define bmBIT1					0x0002
#endif
#ifndef bmBIT2
	#define bmBIT2	 				0x0004
#endif
#ifndef bmBIT3
	#define bmBIT3					0x0008
#endif
#ifndef bmBIT4
	#define bmBIT4					0x0010
#endif
#ifndef bmBIT5
	#define bmBIT5					0x0020
#endif
#ifndef bmBIT6
	#define bmBIT6					0x0040
#endif
#ifndef bmBIT7
	#define bmBIT7					0x0080
#endif
#ifndef bmBIT8
	#define bmBIT8 					0x0100
#endif
#ifndef bmBIT9
	#define bmBIT9 					0x0200
#endif
#ifndef bmBIT10
	#define bmBIT10 				0x0400
#endif
#ifndef bmBIT11
	#define bmBIT11					0x0800
#endif
#ifndef bmBIT12
	#define bmBIT12					0x1000
#endif
#ifndef bmBIT13
	#define bmBIT13					0x2000
#endif
#ifndef bmBIT14
	#define bmBIT14					0x4000
#endif
#ifndef bmBIT15
	#define bmBIT15					0x8000
#endif

#define UNIDENTIFIED_BOARD		0				/* The board has so far been unidentified     */
#define X10_BOARD				1				/* An X10 board is connected                  */
#define X10I_BOARD				2				/* An X10i board is connected                 */
#define	X15_BOARD				3				/* An X15 board is connected                  */

#define UNKNOWN_SPEED			0				/* Speed is unknown                           */
#define USB_1_1_FULL_SPEED		1				/* Board is working in full speed             */
#define USB_2_0_HIGH_SPEED		2				/* Board is working in high speed             */

#define UNIDENTIFIED_BOARD		0				/* The board has so far been unidentified     */
#define X10_BOARD				1				/* An X10 board is connected                  */
#define X10I_BOARD				2				/* An X10i board is connected                 */
#define X15_BOARD				3				/* An X15 board is connected                  */

/**************************************************************************/
/* Error / Return Codes                                                   */
/**************************************************************************/

typedef enum _usbErrorCode
{
    USB_MESSAGE_EXECUTION_SUCCESS                = 0,
    USB_MESSAGE_NOT_RECOGNISED                   = 1,
    USB_MESSAGE_LENGTH_ERROR                     = 2,
    USB_MESSAGE_EXECUTION_FAILURE                = 3,
    USB_DEVICE_WRITE_ERROR                       = 4,
    USB_DEVICE_READ_ERROR                        = 5,
    USB_DEVICE_RESPONSE_ERROR                    = 6,
    USB_DEVICE_BYTE_COUNT_ERROR                  = 7,
    USB_MESSAGE_PARAMETER_OUT_OF_RANGE           = 8,
    USB_MESSAGE_UNKNOWN_HANDSHAKE_MODE           = 9,
    USB_MESSAGE_ILLEGAL_BAUD_RATE                = 10,
    USB_MESSAGE_UNKNOWN_SERIAL_PROTOCOL          = 11,
    USB_MESSAGE_I2C_NACK                         = 12,
    USB_MESSAGE_I2C_ERROR                        = 13,
    USB_MESSAGE_EXECUTION_TIMEOUT                = 14,
    USB_MESSAGE_SRAM_ADDRESSING_ERROR            = 15,
    USB_MESSAGE_REEL_NUMBER_ERROR                = 16,
    USB_MESSAGE_REEL_BUSY                        = 17,
    USB_MESSAGE_SPIN_TABLE_LENGTH_ERROR          = 18,
    USB_MESSAGE_TOO_FEW_REEL_STEPS               = 19,
    USB_MESSAGE_REEL_STATUS_LENGTH_ERROR         = 20,
    USB_MESSAGE_SYNC_REQUEST_LENGTH_ERROR        = 21,
    USB_MESSAGE_PIPE_UNAVAILABLE                 = 22,
	USB_MESSAGE_RANDOM_GENERATOR_DISABLED        = 23,
	USB_MESSAGE_PIPE_CURRENTLY_USED              = 24,
	USB_MESSAGE_FUNCTION_NOT_SUPPORTED_ON_X10    = 25,
	USB_MESSAGE_FUNCTION_NOT_SUPPORTED_ON_X10I   = 26,
	USB_MESSAGE_SAS_ADDRESS_INTERRUPT            = 27,
	USB_MESSAGE_TRY_AGAIN						 = 28,
	USB_MESSAGE_PARALLEL_INPUT_CURRENTLY_ACTIVE  = 29,
	USB_MESSAGE_ILLEGAL_POINTER					 = 30,
	USB_MESSAGE_FUNCTION_NOT_SUPPORTED_ON_X15    = 31,
	USB_MESSAGE_SMARTCARD_SW1SW2_ERROR			 = 32,
	USB_MESSGAE_PIC_BUSY						 = 33,
	USB_MESSAGE_PIC_BUFFER_FULL					 = 34,
	USB_MESSAGE_RECORDING_IN_PROGRESS			 = 35,
	USB_MESSAGE_RECORDING_NOT_CONFIGURED		 = 36,
	USB_MESSAGE_THREAD_FAIL						 = 37,
	USB_MESSAGE_FUNCTION_NOT_CONFIGURED			 = 38,
	USB_MESSAGE_DEBUG_MSG						 = 50

} usbErrorCode;

/**************************************************************************/
/* Inputs                                                                 */
/**************************************************************************/

#define ON 1
#define OFF 0

#define USB_IN_0		0
#define USB_IN_1		1
#define USB_IN_2		2
#define USB_SW			3
#define USB_CS			4
#define USB_IN_LENGTH   5

#define USB_MUXIN_CHANNELS (4)
#define USB_MUXIN_BYTES (3)

typedef enum _usbInputBitId
{
    INPUT_BIT_IP0, INPUT_BIT_IP1, INPUT_BIT_IP2, INPUT_BIT_IP3,
    INPUT_BIT_IP4, INPUT_BIT_IP5, INPUT_BIT_IP6, INPUT_BIT_IP7,
    INPUT_BIT_IP8, INPUT_BIT_IP9, INPUT_BIT_IP10, INPUT_BIT_IP11,
    INPUT_BIT_IP12, INPUT_BIT_IP13, INPUT_BIT_IP14, INPUT_BIT_IP15,
    INPUT_BIT_IP16, INPUT_BIT_IP17, INPUT_BIT_IP18, INPUT_BIT_IP19,
    INPUT_BIT_IP20, INPUT_BIT_IP21, INPUT_BIT_IP22, INPUT_BIT_IP23,
    INPUT_BIT_SW0, INPUT_BIT_SW1, INPUT_BIT_SW2, INPUT_BIT_SW3,
    INPUT_BIT_SW4, INPUT_BIT_SW5, INPUT_BIT_SW6, INPUT_BIT_SW7,
    INPUT_BIT_CURRENT_SENSE
} usbInputBitId;

typedef struct
{
    BYTE	byIn[3];		/* Inputs [0..2] */
    BYTE	bySw;			/* Sw input      */
    BYTE	byCs;			/* Cs input      */
} usbInput;


typedef struct
{
    BYTE byMuxStatus;
    BYTE byMuxInp[ USB_MUXIN_CHANNELS ][ USB_MUXIN_BYTES ];
} usbMultiplexedInput;

typedef enum
{
    InputMultiplexDisabled,
    InputMultiplexEnabled
} usbInputMultiplexing;

#define 	INPUTS_THREAD_POLL_TIME	5						/* mS sleep between thread invoked GetChangedInputs() */

/**************************************************************************/
/* Outputs                                                                */
/**************************************************************************/

#define USB_OUT_0		0
#define USB_OUT_1		1
#define USB_OUT_2		2
#define USB_OUT_3		3
#define USB_AUX			4
#define USB_OUT_LENGTH  	5

typedef enum _usbOutputBitId
{
    OUTPUT_BIT_OP0, OUTPUT_BIT_OP1, OUTPUT_BIT_OP2, OUTPUT_BIT_OP3,
    OUTPUT_BIT_OP4, OUTPUT_BIT_OP5, OUTPUT_BIT_OP6, OUTPUT_BIT_OP7,
    OUTPUT_BIT_OP8, OUTPUT_BIT_OP9, OUTPUT_BIT_OP10, OUTPUT_BIT_OP11,
    OUTPUT_BIT_OP12, OUTPUT_BIT_OP13, OUTPUT_BIT_OP14, OUTPUT_BIT_OP15,
    OUTPUT_BIT_OP16, OUTPUT_BIT_OP17, OUTPUT_BIT_OP18, OUTPUT_BIT_OP19,
    OUTPUT_BIT_OP20, OUTPUT_BIT_OP21, OUTPUT_BIT_OP22, OUTPUT_BIT_OP23,
    OUTPUT_BIT_OP24, OUTPUT_BIT_OP25, OUTPUT_BIT_OP26, OUTPUT_BIT_OP27,
    OUTPUT_BIT_OP28, OUTPUT_BIT_OP29, OUTPUT_BIT_OP30, OUTPUT_BIT_OP31,
    OUTPUT_BIT_AUX0, OUTPUT_BIT_AUX1, OUTPUT_BIT_AUX2, OUTPUT_BIT_AUX3,
    OUTPUT_BIT_AUX4, OUTPUT_BIT_AUX5
} usbOutputBitId;

typedef enum _usbOutputId
{
    USB_OP_0,
    USB_OP_1,
    USB_OP_2,
    USB_OP_3,
    USB_OP_AUX
} usbOutputId;

typedef struct
{
    BYTE	byOut[4];		/* Outputs [0..3] */
    BYTE	byAux;			/* Aux output     */
} usbOutput;


/**************************************************************************/
/* Reels                                                                  */
/**************************************************************************/

#define MAX_RAMP_TABLE_LENGTH		60
#define MAX_NUMBER_OF_REELS			 3

/* Obsolete reel status structure used by GetReelStatus() */
typedef struct
{
    BYTE	byPosition[3];
    BYTE	byError[3];
    BYTE	byBusy[3];
    BYTE	bySynchronised[3];
} usbReelStatus;

/* New reel status structure used by GetReelStatusEx(). Use this if possible. */
typedef struct
{
    WORD	position[3];
    WORD	error[3];
    BYTE	busy[3];
    BYTE	synchronised[3];
} ReelStatusEx;

typedef struct
{
    BYTE	num_entries;						/* Number of entries */
    BYTE	ramp_code[MAX_RAMP_TABLE_LENGTH];	/* Ramp table codes  */
} usbRampTable;


/**************************************************************************/
/* Parallel I/O                                                           */
/**************************************************************************/

typedef enum { Running, Success, Failure, InitJam, Jam } ReleaseHopperCoinsStatus;
typedef enum { Low = 0, High, FallingEdge, RisingEdge } ActiveState;


/**************************************************************************/
/* I/O Recording                                                          */
/**************************************************************************/

typedef enum { RecordingIdle, RecordingConfigured, RecordingInProgress, RecordingComplete, RecordingStopped } RecordingStatus;


/**************************************************************************/
/* Serial I/O                                                             */
/**************************************************************************/

#define CBR_110                 110
#define CBR_300                 300
#define CBR_600                 600
#define CBR_1200                1200
#define CBR_2400                2400
#define CBR_4800                4800
#define CBR_9600                9600
#define CBR_14400               14400
#define CBR_19200               19200
#define CBR_38400               38400
#define CBR_57600               57600
#define CBR_62500				62500

/* RTS Control Flow Values */
#ifndef RTS_CONTROL_DISABLE
#define RTS_CONTROL_DISABLE		0x00
#endif
#ifndef RTS_CONTROL_ENABLE
#define RTS_CONTROL_ENABLE		0x01
#endif
#ifndef RTS_CONTROL_HANDSHAKE
#define RTS_CONTROL_HANDSHAKE	0x02
#endif
#ifndef RTS_CONTROL_TOGGLE
#define RTS_CONTROL_TOGGLE		0x03
#endif

#ifndef NOPARITY
#define NOPARITY				0
#endif
#ifndef ODDPARITY
#define ODDPARITY				1
#endif
#ifndef EVENPARITY
#define EVENPARITY				2
#endif
#ifndef MARKPARITY
#define MARKPARITY				3
#endif
#ifndef SPACEPARITY
#define SPACEPARITY				4
#endif
#ifndef DATA_BIT_9PARITY
#define DATA_BIT_9PARITY		5
#endif

/* Types */
typedef enum _usbSerialPort { PORT_A, PORT_B, NUM_SERIAL_PORTS } usbSerialPort;
typedef enum _usbPortType { PORT_RS232, PORT_RS232_POLLED, PORT_CCTALK, PORT_CCTALK_MODE1, PORT_MDB_SLAVE } usbPortType;

/* CCTalk polling method */
typedef enum _PollMethod { Disabled, Once, Repeated, Triggered } PollMethod;

/* CCTalk configuration */
#define MAX_POLL_MSG_LENGTH	25	/* Max. poll and inhibit message length for polled mode */

#define NO_TRIGGER		4					/* Use this in parameter 'next_trigger_device'                 */
											/* in structure 'RS232PollConfig' if you do                    */
											/* not need to trigger another device                          */

typedef struct
{
	BYTE device_number;						/* Device number (0-3)                                         */
	PollMethod method;						/* Poll method: Disabled, Once or Repeated                     */
	BYTE next_trigger_device;				/* Next device to trigger. Set to NO_TRIGGER if not needed.    */
	BYTE poll_retry_count;					/* Number of retries before sending an inhibit message         */
	int polling_interval;					/* Polling interval (0-2550ms)                                 */
	WORD max_response_time;					/* Max time to wait for response (0-65535ms)                   */
	BYTE min_buffer_space;					/* Min receive buffer space allowed                            */
	BYTE poll_msg[MAX_POLL_MSG_LENGTH];		/* Poll message                                                */
	BYTE inhibit_msg[MAX_POLL_MSG_LENGTH];	/* Device inhibit message                                      */
} CCTalkConfig;

typedef struct
{
	BYTE device_number;						/* Device number (0-3)                                         */
	PollMethod method;						/* Poll method: Disabled, Once or Repeated                     */
	BYTE next_trigger_device;				/* Next device to trigger. Set to NO_TRIGGER if not needed.    */
	BYTE poll_retry_count;					/* Number of retries before sending an inhibit message         */
	int polling_interval;					/* Polling interval (0-2550ms)                                 */
	BYTE remove_local_echo;					/* Remove local echo? TRUE or FALSE                            */
	BYTE length_byte_offset;				/* Offset to length byte in message                            */
	BYTE add_to_length_byte;				/* Offset to received length byte                              */
	WORD max_response_time;					/* Max time to wait for response (0-65535ms)                   */
	BYTE min_buffer_space;					/* Min receive buffer space allowed                            */
	BYTE poll_msg[MAX_POLL_MSG_LENGTH];		/* Poll message                                                */
	BYTE inhibit_msg[MAX_POLL_MSG_LENGTH];	/* Device inhibit message                                      */
} RS232PollConfig;

/* define an MDB message struct */
#define MAX_MDB_DATA_BYTES 36

typedef struct
{
	BYTE address;
	BYTE datalength;
	BYTE databyte[MAX_MDB_DATA_BYTES];
	BYTE chk;
} MDBMessageDef;

/**************************************************************************/
/* EEPROM                                                                 */
/**************************************************************************/

typedef struct
{
	BYTE pageSize;							/* EEPROM page size                                            */
	BYTE numAddressBytes;					/* Number of EEPROM address bytes (either 1 or 2)              */
	WORD maxAddress;						/* Maximum EEPROM address (e.g. EEPROM size in bytes)          */
	BYTE i2cAddress;						/* EEPROM I2C Address                                          */
} X10EEPROM;


#define FEAT_OFFSET		9

#endif
