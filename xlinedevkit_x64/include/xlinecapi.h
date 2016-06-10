/***************************************************************************
*
*     Project:      X-line Board C API
*
*     File:         xlinecapi.h
*
*     Version:      1.1
*
*     Description:  Supports X10, X10i and X15 (not X20)
*
*     Changes:
*     1.1 MJB       Added X10 unlocking functions. These can be used in
*                   for example a non-compiled language where it is not
*                   possible to link in "unlockioc", 16/2/2009
*     1.0 MJB       First version, 9/1/2008.
*
***************************************************************************/

/********    Header Sentry    ********/
#ifndef XLINECAPI__H
#define XLINECAPI__H

#if defined(X10_LINUX_BUILD)
	#define DECLDIR
#else
	#include <windows.h>
	#if defined DLL_EXPORT
		#define DECLDIR __declspec(dllexport)
	#else
		#define DECLDIR __declspec(dllimport)
	#endif
#endif

#define NUMBER_OF_KEYS		4				// should match definition in eval_key.h for each custom PIC design
#define SIZE_OF_KEY			4

#include "x10idefs.h"

/* Specify "C" linkage to get rid of C++ name mangeling */
#ifdef __cplusplus
extern "C" {
#endif

	/** General functions *************************************************************************/
	/**********************************************************************************************/
	DECLDIR void *XlineInitBoardNumber( BYTE boardNumber );
	DECLDIR void *XlineInitBoard( void );
	DECLDIR BOOL XlineFreeBoard( void *xlineBoard );

	DECLDIR usbErrorCode XlineGetLastError( void *xlineBoard );
	DECLDIR BOOL XlineClearErrors( void *xlineBoard );

	/** Software version information **************************************************************/
	/**********************************************************************************************/
	DECLDIR BOOL XlineGetFittedBoard( void *xlineBoard, LPBYTE fittedBoard );				/* Unidentified, X10 or X10i connected */
	DECLDIR BOOL XlineGetBoardSpeed( void *xlineBoard, LPBYTE boardSpeed );					/* UNKNOWN_SPEED, USB_1_1_FULL_SPEED or USB_2_0_HIGH_SPEED */
	DECLDIR BOOL XlineGetProductVersion( void *xlineBoard, LPBYTE versionProduct );
	DECLDIR BOOL XlineGetDllVersion( void *xlineBoard, LPBYTE versionDll );
	DECLDIR BOOL XlineGet8051Version( void *xlineBoard, LPBYTE version8051 ); 									/* Memory pipe   */
	DECLDIR BOOL XlineGetPICVersion( void *xlineBoard, LPBYTE versionPIC );										/* Security pipe */
	DECLDIR BOOL XlineGetPICSerialNumber( void *xlineBoard, LPBYTE serialNumberPIC );							/* Security pipe */
	DECLDIR BOOL XlineSetPICSerialNumber( void *xlineBoard, LPBYTE serialNumberPIC );							/* Security pipe */
	DECLDIR BOOL XlineGetDallasSerialNumber( void *xlineBoard, LPBYTE serialNumberDallas, LPBYTE crcValid ); 	/* Security pipe */

	/** IO pipe functions *************************************************************************/
	/**********************************************************************************************/
	DECLDIR BOOL XlineGetInputs( void *xlineBoard, usbInput *inputs );
	DECLDIR BOOL XlineGetMultiplexedInputs( void *xlineBoard, usbMultiplexedInput *inputs );
	DECLDIR BOOL XlineInputMultiplexing( void *xlineBoard, usbInputMultiplexing input  );
	DECLDIR BOOL XlineGetChangedInputs( void *xlineBoard, usbInput *changedInputs );
	DECLDIR BOOL XlineConfigureChangedInputsCallback ( void *xlineBoard, usbInput *changedInputsMask, usbInput *changedInputsLevel,
										  void (*InputsCallbackFunction)( usbInput *changedInputs, usbInput *currentInputs  ) );
	DECLDIR BOOL XlineGetRawInputs( void *xlineBoard, usbInput *rawInputs );
	DECLDIR BOOL XlineModifyOutputs( void *xlineBoard, usbOutput *offOutputs, usbOutput *onOutputs );
	DECLDIR BOOL XlineSetOnPeriodOutputs( void *xlineBoard, usbOutput onPeriods );
	DECLDIR BOOL XlineSetOffPeriodOutputs( void *xlineBoard, usbOutput offPeriods );
	DECLDIR BOOL XlineSetOnPeriodOutputBit( void *xlineBoard, usbOutputId outputID, int bitNumber, BOOL bitState );
	DECLDIR BOOL XlineSetOffPeriodOutputBit( void *xlineBoard, usbOutputId outputID, int bitNumber, BOOL bitState );
	DECLDIR BOOL XlineSetOutputBrightness( void *xlineBoard, BYTE brightness );
	DECLDIR BOOL XlinePulseOutput( void *xlineBoard, BYTE outputNumber, BYTE pulseDurationMs );
	DECLDIR BOOL XlinePulseOffOutput( void *xlineBoard, BYTE outputNumber, BYTE pulseDurationMs );
	DECLDIR BOOL XlinePulseOutputResult( void *xlineBoard, LPBYTE timeRemaining, BOOL *pulseComplete, BOOL *currentDetected );

	/* Parallel Interface */
	DECLDIR BOOL XlineConfigurePulsedInput( void *xlineBoard, usbInputBitId inputBitID, BYTE pulseLowerTime, ActiveState inputActiveState );
	DECLDIR BOOL XlineConfigurePulsedInputEx( void *xlineBoard, usbInputBitId inputBitID, BYTE pulseLowerTime, BYTE pulseUpperTime, ActiveState inputActiveState );
	DECLDIR BOOL XlineBeginPulsedInputCheck( void *xlineBoard, usbInputBitId inputBitID );
	DECLDIR BOOL XlineEndPulsedInputCheck( void *xlineBoard, usbInputBitId inputBitID );
	DECLDIR BOOL XlineResetPulsedInputCounter( void *xlineBoard, usbInputBitId inputBitID );
	DECLDIR BOOL XlineDecrementPulsedInputCounter( void *xlineBoard, usbInputBitId inputBitID );
	DECLDIR BOOL XlineReadPulsedInputCounter( void *xlineBoard, usbInputBitId inputBitID, BYTE *inputCounterValue );
	DECLDIR BOOL XlineGetPulsedInputStatus( void *xlineBoard, usbInputBitId inputBitID, BOOL *jamStatus, BYTE *inputCounterValue );
	DECLDIR BOOL XlineReleaseParallelHopperCoins( void *xlineBoard, usbInputBitId inputBit, BYTE pulseLowerTime, ActiveState inputActiveState,
									 	     usbOutputBitId outputBit, WORD coinTimeout, BYTE coinsToRelease );
	DECLDIR BOOL XlineReleaseParallelHopperCoinsEx( void *xlineBoard, usbInputBitId inputBit,
										       BYTE pulseLowerTime, BYTE pulseUpperTime, ActiveState inputActiveState,
										       usbOutputBitId outputBit, WORD coinTimeout, BYTE coinsToRelease );
	DECLDIR BOOL XlineGetParallelHopperStatus( void *xlineBoard, usbInputBitId inputBitID, ReleaseHopperCoinsStatus *status, BYTE *coinsReleased );
	DECLDIR BOOL XlineStopHopperCoinRelease( void *xlineBoard, usbInputBitId inputBitID );
	DECLDIR BOOL XlineStopHopperCoinReleaseSafe( void *xlineBoard, usbInputBitId inputBitID );

	/* Reel Interface */
	DECLDIR BOOL XlineConfigureReels( void *xlineBoard, BYTE numberOfReels, BYTE halfStepsPerTurn, BYTE stepsPerSymbol );
	DECLDIR BOOL XlineConfigureReelsEx( void *xlineBoard, BYTE numberOfReels, WORD halfStepsPerTurn, BYTE stepsPerSymbol );
	DECLDIR BOOL XlineSpinReels( void *xlineBoard, BYTE reelNumber, BYTE directionAndStepSize, WORD steps );
	DECLDIR BOOL XlineSpinRampUp( void *xlineBoard, BYTE reelNumber, usbRampTable rampUpTable );
	DECLDIR BOOL XlineSpinRampDown( void *xlineBoard, BYTE reelNumber, usbRampTable rampDownTable );
	DECLDIR BOOL XlineSetDutyCycle( void *xlineBoard, BYTE reelNumber, BYTE offPeriod, BYTE onPeriod );
	DECLDIR BOOL XlineGetReelStatus( void *xlineBoard, usbReelStatus *status );
	DECLDIR BOOL XlineGetReelStatusEx( void *xlineBoard, ReelStatusEx * status );
	DECLDIR BOOL XlineReelSynchroniseEnable( void *xlineBoard, BYTE reelNumber );

	/** Serial pipe functions *********************************************************************/
	/**********************************************************************************************/
	/* Depending on the port used, the following functions can use either the SERIAL_A or SERIAL_B pipe */
	DECLDIR BOOL XlineSetConfig( void *xlineBoard, usbSerialPort port, LPDCB config, usbPortType type );
	DECLDIR BOOL XlineSend( void *xlineBoard, usbSerialPort port, LPBYTE data, UINT length );
	DECLDIR BOOL XlineReceive( void *xlineBoard, usbSerialPort port, LPBYTE data, LPUINT length );
	DECLDIR BOOL XlineReceiveByteWithTimestamp( void *xlineBoard, usbSerialPort port, LPBYTE rxByte, LPBYTE interval, BOOL *received );
	DECLDIR BOOL XlineSend9BitData( void *xlineBoard, usbSerialPort port, LPWORD data, UINT length );
	DECLDIR BOOL XlineReceive9BitData( void *xlineBoard, usbSerialPort port, LPWORD data, LPUINT length );
	DECLDIR BOOL XlineReceiveByteWithTimestamp9BitData( void *xlineBoard, usbSerialPort port, LPWORD rxWord, LPBYTE interval, BOOL *received );
	DECLDIR BOOL XlineSetTimeoutMessage( void *xlineBoard, usbSerialPort port, LPBYTE message, BYTE numBytes, float timeout );
	DECLDIR BOOL XlineGetParityErrors( void *xlineBoard, usbSerialPort port, LPWORD parityErrors );

	/* SAS (Slot Accounting System) functions */
	DECLDIR BOOL XlineSetSASMachineAddress( void *xlineBoard, usbSerialPort port, BYTE address );
	DECLDIR BOOL XlineSetSASAutoReply( void *xlineBoard, usbSerialPort port, BOOL AutoReplyEnabled, BYTE AutoReplyDelayMs );
	DECLDIR BOOL XlineSetSASBusy( void *xlineBoard, usbSerialPort port, BOOL busyState );
	DECLDIR BOOL XlineGetSASMessageStatus( void *xlineBoard, usbSerialPort port, BOOL *messageSuccess );

	/* CCTalk Mode 1 and RS232 Polled Mode */
	DECLDIR BOOL XlineConfigureCCTalkPort( void *xlineBoard, usbSerialPort port, CCTalkConfig *cctalkConfig );
	DECLDIR BOOL XlineConfigureRS232Poll( void *xlineBoard, usbSerialPort port, RS232PollConfig *pollConfig );
	DECLDIR BOOL XlineSetPolledHostTimeout( void *xlineBoard, usbSerialPort port, BYTE deviceNumber, double timeout );
	DECLDIR BOOL XlineEmptyPolledBuffer( void *xlineBoard, usbSerialPort port, BYTE deviceNumber );
	DECLDIR BOOL XlineReceivePolledMessage( void *xlineBoard, usbSerialPort port, BYTE deviceNumber, LPBYTE data, LPUINT length, BOOL *inhibited );
	DECLDIR BOOL XlineDeletePolledMessage( void *xlineBoard, usbSerialPort port, BYTE deviceNumber );

	/** SPI pipe functions ************************************************************************/
	/**********************************************************************************************/
	DECLDIR BOOL XlineEnableSPI( void *xlineBoard );
	DECLDIR BOOL XlineDisableSPI( void *xlineBoard );
	DECLDIR BOOL XlineSendSPI( void *xlineBoard, BYTE numberOfTxBits, LPBYTE txMessage, BYTE waitTimeMs, BYTE numberOfRxBits, LPBYTE rxMessage );
	DECLDIR BOOL XlineSendSEC( void *xlineBoard, BYTE command, BYTE id, BYTE numberOfTxBytes, LPBYTE txMessage,
							   BYTE waitTimeMs, BYTE numberOfRxBytes, LPBYTE rxMessage );

	/** Memory pipe functions *********************************************************************/
	/**********************************************************************************************/
	DECLDIR BOOL XlineConfigureEEPROM( void *xlineBoard, X10EEPROM *eepromConfig );
	DECLDIR BOOL XlineCheckEEPROM( void *xlineBoard, X10EEPROM *eepromConfig );
	DECLDIR BOOL XlineReadEEPROM( void *xlineBoard, WORD address, LPBYTE data, UINT totalLength );
	DECLDIR BOOL XlineWriteEEPROM( void *xlineBoard, WORD address, LPBYTE data, UINT totalLength );

	/** SRAM pipe functions ***********************************************************************/
	/**********************************************************************************************/
	DECLDIR BOOL XlineReadSRAM( void *xlineBoard, WORD address, LPBYTE data, UINT totalLength );
	DECLDIR BOOL XlineWriteSRAM( void *xlineBoard, WORD address, LPBYTE data, UINT totalLength );
	DECLDIR BOOL XlineReadLargeSRAM( void *xlineBoard, DWORD address, LPBYTE data, DWORD totalLength );
	DECLDIR BOOL XlineWriteLargeSRAM( void *xlineBoard, DWORD address, LPBYTE data, DWORD totalLength );

	/** Security pipe functions *******************************************************************/
	/**********************************************************************************************/
	/* Clock */
	DECLDIR BOOL XlineSetClock( void *xlineBoard, DWORD time );
	DECLDIR BOOL XlineGetClock( void *xlineBoard, LPDWORD time );

	DECLDIR BOOL XlineRelockHardware( void *xlineBoard );


	/* Power-off switches */
	DECLDIR BOOL XlineNextSecuritySwitchRead( void *xlineBoard, LPDWORD time, LPBYTE switches );
	DECLDIR BOOL XlineStartSecuritySwitchRead( void *xlineBoard );
	DECLDIR BOOL XlineClearSecuritySwitches( void *xlineBoard );
	DECLDIR BOOL XlineReadAndResetSecuritySwitchFlags( void *xlineBoard, LPBYTE closedSwitches, LPBYTE openSwitches );
	DECLDIR BOOL XlineCachedReadAndResetSecuritySwitchFlags( void *xlineBoard, LPBYTE closedSwitches, LPBYTE openSwitches );

	/* Battery */
	DECLDIR BOOL XlineReadAndResetBatteryFailFlag( void *xlineBoard, LPBYTE batteryFlag );
	DECLDIR BOOL XlineReadAndResetRTCFailure( void *xlineBoard, unsigned char *rtcFlag );
	DECLDIR BOOL XlineReadClockAtBatteryFailure( void *xlineBoard, LPDWORD time );

	/* Random Number Generator */
	DECLDIR BOOL XlineGetRandomNumber( void *xlineBoard, LPWORD randnumber );
	DECLDIR BOOL XlineEnableRandomNumberGenerator( void *xlineBoard, DWORD seed );
	DECLDIR BOOL XlineDisableRandomNumberGenerator( void *xlineBoard );

	/* Smartcard */
	DECLDIR BOOL XlineSmartcardReset( void *xlineBoard, LPBYTE rxMessage, unsigned int *rxLength, UINT msDelay );
	DECLDIR BOOL XlineSmartcardSetSessionID( void *xlineBoard, UINT sessionID, unsigned int *sw1sw2, UINT msDelay );
	DECLDIR BOOL XlineSmartcardGetApplicationID( void *xlineBoard, unsigned int *sw1sw2, unsigned char *idString, unsigned int msDelay );
	DECLDIR BOOL XlineAuthenticate( void *xlineBoard, unsigned char *encryptedKey, unsigned int sessionID );

	DECLDIR BOOL XlineSecurityIO( void *xlineBoard, BYTE command, LPBYTE txBuffer, DWORD txLength,
								  LPBYTE rxBuffer, DWORD *rxLength );


	/* These three functions are unique to the C-API and are not reflected from FFLYUSB class.
	   Are only used in the c-api timedemo.cpp prog. */
	/* Hardware locking */
	DECLDIR BOOL XlineVerifyX10UnlockLibrary( void *x10Board, unsigned char keys[NUMBER_OF_KEYS][SIZE_OF_KEY] );
	DECLDIR BOOL XlineUnlockX10( void *x10Board, unsigned char keys[NUMBER_OF_KEYS][SIZE_OF_KEY] );
	DECLDIR BOOL XlineUnlockX10Recheck( void *x10Board, unsigned char keys[NUMBER_OF_KEYS][SIZE_OF_KEY] );

#ifdef __cplusplus
}
#endif
#endif 			/* close #ifndef XLINECAPI__H */

