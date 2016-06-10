/***************************************************************************
*
*     Project:      Firefly X10i Board
*
*     File:         fflyusb.h
*
*     Version:      1.3
*
*     Description:  FireFly X10i API
*                   Supports both X10 and X10i boards
*
*     Notes:
*
*     Changes:
*	  1.3 RDP		Added new functions:
*					- ConfigureRecordingInputs()
*					- StartRecordingInputs()
*					- GetRecordingStatus()
*					See 80-19901 23-10-07
*	  1.2 RDP		Added X15 functionality,
*					renamed X10Implementation to XProduct 08/10/2007
*
*     1.1 MJB       Added SAS functions SetSASMachineAddress() and
*                   SetSASBusy(), 30/3/2006
*     1.0 MJB       First version, based on the Linux X10 fflyusb.h
*                   issue 1.19, 10/8/2005
*
***************************************************************************/

/********    Header Sentry    ********/
#ifndef FIREFLYUSB_H
#define FIREFLYUSB_H

//#ifdef X10_LINUX_BUILD
	typedef unsigned char            BYTE;
	typedef unsigned short           WORD;
//#endif

#include "x10idefs.h"

typedef unsigned int             BOOL;
typedef unsigned char *          LPBYTE;
typedef unsigned int             BOOL;
typedef unsigned int *           LPBOOL;
typedef unsigned char            UCHAR;
typedef unsigned short           USHORT;
typedef unsigned long            ULONG;
typedef unsigned short *         LPWORD;
typedef unsigned long            DWORD;
typedef unsigned long *          LPDWORD;
typedef unsigned int             UINT;
typedef long unsigned int *      LPUINT;
typedef int                      HANDLE;


////////////////////////////////////////////////////////////////////////////////////////////////////
//     FIREFLY API                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////
class X10IOLock ;
class XBaseImplementation;
class FireFlyUSB
{
public:
	//** Required class functions *************************************************
	FireFlyUSB( void );										// Constructor
	~FireFlyUSB( void );										// Destructor
	FireFlyUSB(FireFlyUSB& );							// Constructor
	FireFlyUSB& operator=(FireFlyUSB& );				// Constructor

	//** General functions ********************************************************
	BOOL InitUSBBoard( BYTE boardNumber );
	BOOL init( int PortNumber=0);						// Initialises USB
	BOOL close( void );										// Closes handle to USB

	usbErrorCode GetLastError( void );						// Returns last error code
	BOOL ClearErrors( void );								// Clears last error code


	//** Software version information *********************************************
	BOOL GetFittedBoard( LPBYTE fittedBoard );			// Unidentified, X10 or X10i connected
	BOOL GetBoardSpeed( LPBYTE boardSpeed );			// UNKNOWN_SPEED, USB_1_1_FULL_SPEED or USB_2_0_HIGH_SPEED
	BOOL GetProductVersion( LPBYTE versionProduct );
	BOOL GetDllVersion( LPBYTE versionDll );
	BOOL Get8051Version( LPBYTE version8051 ); 			// Memory pipe
	BOOL GetPICVersion( LPBYTE versionPIC );			// Security pipe
	BOOL GetPICSerialNumber( LPBYTE serialNumberPIC );	// Security pipe
	BOOL SetPICSerialNumber( LPBYTE serialNumberPIC );	// Security pipe
	BOOL GetDallasSerialNumber( LPBYTE serialNumberDallas, LPBYTE crcValid ); // Security pipe


	//** IO ***********************************************************************
	// All of the following functions use the IO pipe
	// Obsolete functions
	BOOL GetInputBit( usbInputBitId input_bit_id, LPBOOL result );
	BOOL GetInputs( LPBYTE inputs );
	BOOL SetOutputBit( usbOutputBitId output_bit_id, BOOL bit_state );
	BOOL SetOutputs( LPBYTE outputs );

	// New functions - use these
	BOOL GetInputs( usbInput *inputs );
	BOOL GetMultiplexedInputs( usbMultiplexedInput *inputs );
	BOOL InputMultiplexing( usbInputMultiplexing input  );
	BOOL GetChangedInputs( usbInput *changedInputs );
	BOOL ConfigureChangedInputsCallback ( usbInput *changedInputsMask, usbInput *changedInputsLevel,
										  void (*InputsCallbackFunction)( usbInput *changedInputs, usbInput *currentInputs  ) );
	BOOL GetRawInputs( usbInput *rawInputs );
	BOOL ModifyOutputs( usbOutput *offOutputs, usbOutput *onOutputs );
	BOOL SetOnPeriodOutputs( usbOutput onPeriods );
	BOOL SetOffPeriodOutputs( usbOutput offPeriods );
	BOOL SetOnPeriodOutputBit( usbOutputId outputID, int bitNumber, BOOL bitState );
	BOOL SetOffPeriodOutputBit( usbOutputId outputID, int bitNumber, BOOL bitState );
	BOOL SetOutputBrightness( BYTE brightness );
	BOOL PulseOutput( BYTE outputNumber, BYTE pulseDurationMs );
	BOOL PulseOffOutput( BYTE outputNumber, BYTE pulseDurationMs );
	BOOL PulseOutputResult( LPBYTE timeRemaining, BOOL *pulseComplete, BOOL *currentDetected );

	// Parallel Interface
	BOOL ConfigurePulsedInput( usbInputBitId inputBitID, BYTE pulseLowerTime, ActiveState inputActiveState );
	BOOL ConfigurePulsedInputEx( usbInputBitId inputBitID, BYTE pulseLowerTime, BYTE pulseUpperTime, ActiveState inputActiveState );
	BOOL BeginPulsedInputCheck( usbInputBitId inputBitID );
	BOOL EndPulsedInputCheck( usbInputBitId inputBitID );
	BOOL ResetPulsedInputCounter( usbInputBitId inputBitID );
	BOOL DecrementPulsedInputCounter( usbInputBitId inputBitID );
	BOOL ReadPulsedInputCounter( usbInputBitId inputBitID, BYTE *inputCounterValue );
	BOOL GetPulsedInputStatus( usbInputBitId inputBitID, BOOL *jamStatus, BYTE *inputCounterValue );
	BOOL ReleaseParallelHopperCoins( usbInputBitId inputBit, BYTE pulseLowerTime, ActiveState inputActiveState,
									 usbOutputBitId outputBit, WORD coinTimeout, BYTE coinsToRelease );
	BOOL ReleaseParallelHopperCoinsEx(	usbInputBitId inputBit,
										BYTE pulseLowerTime, BYTE pulseUpperTime, ActiveState inputActiveState,
										usbOutputBitId outputBit, WORD coinTimeout, BYTE coinsToRelease );
	BOOL GetParallelHopperStatus( usbInputBitId inputBitID, ReleaseHopperCoinsStatus *status, BYTE *coinsReleased );
	BOOL StopHopperCoinRelease( usbInputBitId inputBitID );
	BOOL StopHopperCoinReleaseSafe( usbInputBitId inputBitID );

	// Reel Interface
	BOOL ConfigureReels( BYTE numberOfReels, BYTE halfStepsPerTurn, BYTE stepsPerSymbol );
	BOOL ConfigureReelsEx( BYTE numberOfReels, WORD halfStepsPerTurn, BYTE stepsPerSymbol );
	BOOL SpinReels( BYTE reelNumber, BYTE directionAndStepSize, WORD steps );
	BOOL SpinRampUp( BYTE reelNumber, usbRampTable rampUpTable );
	BOOL SpinRampDown( BYTE reelNumber, usbRampTable rampDownTable );
	BOOL SetDutyCycle( BYTE reelNumber, BYTE offPeriod, BYTE onPeriod );
	BOOL GetReelStatus( usbReelStatus *status );
	BOOL GetReelStatusEx( ReelStatusEx *status );
	BOOL ReelSynchroniseEnable( BYTE reelNumber );
	BOOL ConfigureRecordingInputs		( usbInputBitId* inputArray, DWORD startAddress, WORD maxMemorySize );
	BOOL StartRecordingInputs			( WORD msDuration );
	BOOL GetRecordingStatus				( RecordingStatus* status, WORD* remainingDuration, WORD* memoryUsed );
	BOOL ReadDiagnosticData				( BYTE* dataBlock, DWORD length, BYTE* retData, DWORD* retLength );
	BOOL WriteDiagnosticData			( BYTE* dataBlock, DWORD length );

	//** Serial *******************************************************************
	// Depending on the port used, the following functions can use either the
	// SERIAL_A or SERIAL_B pipe
	BOOL SetConfig( usbSerialPort port, LPDCB config, usbPortType type );
	BOOL Send( usbSerialPort port, LPBYTE data, UINT length );
	BOOL Receive( usbSerialPort port, LPBYTE data, LPUINT length );
	BOOL ReceiveByteWithTimestamp( usbSerialPort port, LPBYTE rxByte, LPBYTE interval, BOOL *received );
	BOOL Send9BitData( usbSerialPort port, LPWORD data, UINT length );
	BOOL Receive9BitData( usbSerialPort port, LPWORD data, LPUINT length );
	BOOL ReceiveByteWithTimestamp9BitData( usbSerialPort port, LPWORD rxWord, LPBYTE interval, BOOL *received );
	BOOL SetTimeoutMessage( usbSerialPort port, LPBYTE message, BYTE numBytes, float timeout );
	BOOL GetParityErrors( usbSerialPort port, LPWORD parityErrors );

	// SAS (Slot Accounting System) functions
	BOOL SetSASMachineAddress( usbSerialPort port, BYTE address );
	BOOL SetSASAutoReply( usbSerialPort port, BOOL AutoReplyEnabled, BYTE AutoReplyDelayMs );
	BOOL SetSASBusy( usbSerialPort port, BOOL busyState );
	BOOL GetSASMessageStatus( usbSerialPort port, BOOL *messageSuccess );

	// CCTalk Mode 1 and RS232 Polled Mode
	BOOL ConfigureCCTalkPort( usbSerialPort port, CCTalkConfig *cctalkConfig );
	BOOL ConfigureRS232Poll( usbSerialPort port, RS232PollConfig *pollConfig );
	BOOL SetPolledHostTimeout( usbSerialPort port, BYTE deviceNumber, double timeout );
	BOOL EmptyPolledBuffer( usbSerialPort port, BYTE deviceNumber );
	BOOL ReceivePolledMessage( usbSerialPort port, BYTE deviceNumber, LPBYTE data, LPUINT length, BOOL *inhibited );
	BOOL DeletePolledMessage( usbSerialPort port, BYTE deviceNumber );

	//** SPI (Serial Peripheral Inferface) ****************************************
	// All of the following functions use the SPI pipe
	BOOL EnableSPI( void );
	BOOL DisableSPI( void );
	BOOL SendSPI( BYTE numberOfTxBits, LPBYTE txMessage, BYTE waitTimeMs, BYTE numberOfRxBits, LPBYTE rxMessage );
	BOOL SendSEC( BYTE command, BYTE id, BYTE numberOfTxBytes, LPBYTE txMessage,
                  BYTE waitTimeMs, BYTE numberOfRxBytes, LPBYTE rxMessage );


	//** EEPROM *******************************************************************
	// All of the following functions use the MEMORY pipe
	BOOL ConfigureEEPROM( X10EEPROM *eepromConfig );
	BOOL CheckEEPROM( X10EEPROM *eepromConfig );
	BOOL ReadEEPROM( WORD address, LPBYTE data, UINT totalLength );
	BOOL WriteEEPROM( WORD address, LPBYTE data, UINT totalLength );


	//** SRAM *********************************************************************
	// All of the following functions use the SRAM pipe
	BOOL ReadSRAM( WORD address, LPBYTE data, UINT totalLength );
	BOOL WriteSRAM( WORD address, LPBYTE data, UINT totalLength );
	BOOL ReadLargeSRAM( DWORD address, LPBYTE data, DWORD totalLength );
	BOOL WriteLargeSRAM( DWORD address, LPBYTE data, DWORD totalLength );


	//** Security: clock **********************************************************
	// All of the following functions use the SECURITY pipe
	BOOL SetClock( DWORD time );
	BOOL GetClock( LPDWORD time );
	BOOL RelockHardware( void );

	//** Security: switches *******************************************************
	// All of the following functions use the SECURITY pipe
	BOOL NextSecuritySwitchRead( LPDWORD time, LPBYTE switches );
	BOOL StartSecuritySwitchRead( void );
	BOOL ClearSecuritySwitches( void );
	BOOL ReadAndResetSecuritySwitchFlags( LPBYTE closedSwitches, LPBYTE openSwitches );
	BOOL CachedReadAndResetSecuritySwitchFlags( LPBYTE closedSwitches, LPBYTE openSwitches );

	//** Security: battery *******************************************************
	// All of the following functions use the SECURITY pipe
	BOOL ReadAndResetBatteryFailFlag	( LPBYTE batteryFlag );
	BOOL ReadAndResetRTCFailure			( unsigned char *rtcFlag );
	BOOL ReadClockAtBatteryFailure		( LPDWORD time );

	//** Random Number Generator **************************************************
	// All of the following functions use the SECURITY pipe
	BOOL GetRandomNumber( LPWORD randnumber );
	BOOL EnableRandomNumberGenerator( DWORD seed );
	BOOL DisableRandomNumberGenerator( void );

	//** Security: Smartcard *******************************************************
	BOOL SmartcardReset( LPBYTE rxMessage, unsigned int *rxLength, UINT msDelay );
	BOOL SmartcardSetSessionID( UINT sessionID, unsigned int *sw1sw2, UINT msDelay );
	BOOL SmartcardGetApplicationID( unsigned int *sw1sw2, unsigned char *idString, unsigned int msDelay );
	BOOL Authenticate( unsigned char *encryptedKey, unsigned int sessionID );

	BOOL SecurityIO( 	BYTE command,
						LPBYTE txBuffer,
						DWORD txLength,
						LPBYTE rxBuffer,
						DWORD *rxLength );

private:
	BOOL PerformUSBIO(	BYTE command,			// Command, i.e. USB_MSG_GET_8051_VERSION
						int pipeNum,			// Pipe number, i.e. USB_PIPE_MEMORY
						LPBYTE txBuffer,		// Out buffer
						DWORD txLength,			// Out buffer length
						LPBYTE rxBuffer,		// In buffer
						DWORD *rxLength );		// In buffer length

	BOOL InitPort( int portNumber );
	BOOL DownloadHiCode( void );
	usbErrorCode PipeScheduler( void *pipeInfo );

	XBaseImplementation * XProduct;
	friend class X10IOLock;
	friend class X10UnitTest;
};

#endif

