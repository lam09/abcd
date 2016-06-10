#include "com_ag_game_card.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
//#include <string>
#include <syslog.h>

#include <ctype.h>
#include "linuxkb.c"
#include "authenticate_linux.h"

#include "fflyusb.h"
#include "unlockio.h"
#include "x10errhd.c"
#include "x15key.h"

#include <queue>

#include <stdint.h> //bill
typedef unsigned char Byte; //bill


using std::queue;


BOOL                    initialized = FALSE;
// BOOL                 configured = FALSE;
BOOL                    inhibited = FALSE;
BOOL                    resetLogged = FALSE;
FireFlyUSB              FireFly;
const usbSerialPort     portNum = PORT_A;
const usbSerialPort     portNumBill = PORT_B;
const float             hostTimeout = 0;
const BYTE              deviceNum = 0;

int AddressAcceptorBill = -1;
int AddressAcceptorCoin = -1;

unsigned int            counter = 0;
queue<int>              coins;

#define                 MAX_CCTALK_RX_LEN   257

#define MEMORY_CAPACITY		512				// EEPROM size in bytes
#define MEMORY_BEGIN		8				// First location in EEPROM that can be written to/read from

int configure(const char *string);
int close();
int init(BOOL);
int logError(char, int);



int logError(char *file = "", int line = 0) {
    int                 error;

    error = FireFly.GetLastError();
    //if (error != 0) {
        if (strlen(file) > 0) {
            syslog(LOG_ERR, "Communication with FireFly, error number: %d, in %s:%d", error, file, line);
        } else {
            syslog(LOG_ERR, "Communication with FireFly, error number: %d", error);
        }
        FireFly.ClearErrors();
    //}
    return error;
}



int close() {
    int                 error;

    error = logError();

    FireFly.close();
    initialized = FALSE;
    return error;
}



/**
* EEPROM - saving bills and coins to memory
*
* 0x008 checksum - if not valid delet all
* 0x009 id of last bill
* 0x0010 coins
* 0x0020 value of last bill and id 
*/

bool WriteToMemory(UINT write_address, BYTE byte_to_write, UINT write_bytes = 1){

	BYTE data[MEMORY_CAPACITY];
	//data[0] = byte_to_write;
	int index;
	for (index = 0; index < write_bytes; index++) data[index] = byte_to_write;

	//printf("\nWriting %i to address %i bytes %i\n", byte_to_write, write_address, write_bytes);

	if (!FireFly.WriteEEPROM(write_address, (LPBYTE)&data, write_bytes)) return false;


	return true;


}

BYTE* ReadFromMemory(UINT read_address, UINT read_bytes){

	BYTE data[MEMORY_CAPACITY];							// Create buffer for data
	UINT index;

	//printf("\nReading");

	//printf("address:%02X\n", read_address);
	//printf("read_bytes:%02X\n", read_bytes);


	if (!FireFly.ReadEEPROM(read_address, (LPBYTE)&data, read_bytes)) return NULL;

	//printf("\nReadbytes%02X ", read_bytes);

/*	for (index = 0; index < read_bytes; index++)
	{
	if (!((index + read_address) % 16) || (index == 0))
	printf("\n0x%04X:\t", index + read_address);
	printf("%02X ", data[index] & 0x00FF);
	}*/
	return data;
}

int ComputeCheckSum(){

	//printf("WriteCheckSum function\n");

	BYTE* data;
	UINT index;

	data = ReadFromMemory(9, 39);

	//printf("%02X ", data[0] & 0x00FF);

	int checksum = 0;

	for (index = 0; index < 39; index++)
	{
		if (!((index + 9) % 16) || (index == 0))
		printf("\n0x%04X:\t", index + 9);
		printf("%02X ", data[index] & 0x00FF);

		//printf("\nsumming:%i", data[index]);
		checksum += data[index];
	}

	checksum = checksum % 256;

	printf("\nComputed CheckSum:%i\n", checksum);
	return checksum;
}

int GetCheckSum(){
	BYTE* data;
	data = ReadFromMemory(8, 1);
	printf("\nReading checksum: %i\n", data[0]);
	return data[0];
}

bool WriteCheckSum(){

	return WriteToMemory(8, ComputeCheckSum());

}

void ClearAll(){
	WriteToMemory(8,0,40);
}

bool isChecksumValid(){

	if (GetCheckSum() == ComputeCheckSum()){ //all good
		printf("\nChecksum is valid\n");
		return true; 
	} else { //data is not consistent
		printf("\nChecksum is not valid - clearing all\n");
		ClearAll();
		return false;
	}

}

/*
33 - ID bankovky
32 - hodnota bankovky
*/
int* GetSavedBill(){
	BYTE* data = ReadFromMemory(32, 2);
	return new int[2]{data[0], data[1]};
}

int SetSavedBill(int value){
	int id = (ReadFromMemory(9, 1)[0] + 1) % 256;

	if (!WriteToMemory(32, value)) return -1;
	if (!WriteToMemory(33, id)) return -1;


	if (!WriteToMemory(9, id)) return -1; //save new id

	if (!WriteCheckSum()) return -1;

	return id;
}

/**
* End of EEPROM
*/



void fillBuffer(BYTE *buffer, const char *string, BOOL calculateChecksum = FALSE) {
    int                 length = 0;
    unsigned int        b = 0;
    int                 i = 0, j = 0;
    unsigned int        checksum = 0;

    length = strlen(string);
    for (i=0, j=0; i<=length; i+=3, j++) {
        sscanf(string+i, "%x", &b);
        buffer[j] = (BYTE)b;
        checksum += b;
    }

    //printf("%d %d %x\n", i, j, 256 - (checksum % 256));

    if (calculateChecksum) {
        buffer[j] = 256 - (checksum % 256);
    }
}

void fillBufferFromString(BYTE *buffer, const char *string) {
    int                 length = 0;
    unsigned int        b = 0;
    int                 i = 0, j = 0;
    unsigned int        checksum = 0;

    length = strlen(string);
    for (i=0, j=0; i<=length; i+=4, j++) {
        sscanf(string+i, "%u", &b);
        buffer[j] = (BYTE)b;
        checksum += b;
    }

    //printf("%d %d %x\n", i, j, 256 - (checksum % 256));

    buffer[j] = 256 - (checksum % 256);
}

int configure(const char *string = "045 000 001 229") {
    CCTalkConfig        cctalk_config;						// CCTalk configuration structure
    
    //coin
    cctalk_config.device_number = deviceNum;
    //cctalk_config.method = Once;
    cctalk_config.method = Repeated;
    cctalk_config.next_trigger_device = NO_TRIGGER;
    cctalk_config.poll_retry_count = 5;
    //cctalk_config.polling_interval = 0;
    cctalk_config.polling_interval = 200;
    cctalk_config.max_response_time = 2000;
    cctalk_config.min_buffer_space = 20;

	//cctalk_config.poll_msg[0] = 45;					// destination address
	//cctalk_config.poll_msg[1] = 0;					// transmit data length (after Header, before Checksum)
	//cctalk_config.poll_msg[2] = 1;					// source address
	//cctalk_config.poll_msg[3] = 229;					// command: E5 = ReadBufferedCredit  FE=SimplePoll
	//cctalk_config.poll_msg[4] = 255;					// checksum

    fillBufferFromString(cctalk_config.poll_msg, string);
    fillBufferFromString(cctalk_config.inhibit_msg, string);

    //fillBuffer(cctalk_config.poll_msg, "02 00 01 e5", TRUE);



    //fillBuffer(cctalk_config.inhibit_msg, "02 00 01 e5", TRUE);


    if (!FireFly.ConfigureCCTalkPort(portNum, &cctalk_config)) {
        // printf("config 1\n");
        return close();
    }

    if (!FireFly.SetPolledHostTimeout(portNum, deviceNum, (double)hostTimeout)) {
        // printf("config 2\n");
        return close();
    }
    
    return USB_MESSAGE_EXECUTION_SUCCESS;
}

JNIEXPORT jboolean JNICALL Java_com_ag_game_card_Initialize
(JNIEnv *, jobject){

	if (!initialized) {
		openlog("rumservis-coinslot", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);
	}

	if (initialized) {
		return true;
	}

	// BYTE                 fittedBoard;
	DCB 			    config;								// Configure device
	//int                 error = 0;

	if (!FireFly.init()) {
		// printf("init 1\n");
		close();
		return false;
	}

	// if (!FireFly.GetFittedBoard(&fittedBoard)) {
	// // printf("init 2\n");
	// return close();
	// }

	// if (!UnlockX10(&FireFly)) {
	// return close();
	// }

	config.BaudRate = 9600;
	config.Parity = NOPARITY;
	config.fOutxCtsFlow = FALSE;
	config.fRtsControl = RTS_CONTROL_TOGGLE;
	if (!FireFly.SetConfig(portNum, &config, PORT_CCTALK_MODE1)) {
		// printf("init 3\n");
		close();
		return false;
	}
	if (!FireFly.SetConfig(portNumBill, &config, PORT_CCTALK_MODE1)) {
		// printf("init 3\n");
		close();
		return false;
	}

	if (!FireFly.SetPolledHostTimeout(portNumBill, deviceNum, (double)hostTimeout)) {
		close();
		return false;
	}

	isChecksumValid(); // clear memory if neccesary

	syslog(LOG_INFO, "Device initialized");
	initialized = TRUE;
	return true;

}


JNIEXPORT jboolean JNICALL Java_com_ag_game_card_close
  (JNIEnv *env, jobject obj) {
    jboolean            success = TRUE;

    //success = FireFly.DeletePolledMessage(portNum, 0);
    //success = success && FireFly.close();

    close();
    syslog(LOG_INFO, "Device closed");
    closelog();
    return success;
}

/**
 * Bill acceptor
 */

int BillinEscrow = 0;
int counterBill = 0;
int newCounterBill = 0;
BOOL initializedBill = FALSE;
BOOL resetLoggedBill = FALSE;
BOOL IsReadBillBuffer = FALSE;
CCTalkConfig cctalk_config; // CCTalk configuration structure for bill acceptor
BYTE rxMsg [MAX_CCTALK_RX_LEN + 1];      // Create buffer for message to receive
BYTE rxMsglast [MAX_CCTALK_RX_LEN + 1];      // Create buffer for message to receive
BOOL had_response;                       // Have we received CCTalk response?
long unsigned int bytesRxd;
BOOL is_inhibited;                       // Is device inhibited?


const unsigned short crc_tabccitt[] =
{
0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

uint16_t calculate_crc_lookup_CCITT_A( int l, Byte* n )
{
    uint16_t crc = 0x0000;
    for (int i = 0; i < l; ++i ){
        crc = ( crc << 8 ) ^ crc_tabccitt[ ( crc >> 8 ) ^ n[i] ];
    }
    return crc;
}

void printarray(BYTE* cctalk_config, int size){
    for (int i=0;i<size;i++) printf("%u ",cctalk_config[i]);
}

void Insertinto(const char *string,  int size){

    Byte *ibyteArray = new Byte[size];

    int length = 0;
    unsigned int b = 0;
    int i = 0, j = 0;

    length = strlen(string);
    for (i=0, j=0; i<=length; i+=3, j++) {
        sscanf(string+i, "%x", &b);
        cctalk_config.poll_msg[j] = (BYTE)b;
        cctalk_config.inhibit_msg[j] = (BYTE)b;

       // printf("\n%x",cctalk_config[j]);
    }


    int n=0;

    for(int i=0;i<size-1;i++){ // all but last and second
        if(i != 2) ibyteArray[n++] = cctalk_config.poll_msg[i];
    }

    uint16_t CRC16 = calculate_crc_lookup_CCITT_A(size-2,ibyteArray);

    uint8_t CRC[2];
    CRC[0]=CRC16 & 0xff;
    CRC[1]=(CRC16 >> 8);

    cctalk_config.poll_msg[2] = CRC[0];
    cctalk_config.poll_msg[size-1] = CRC[1];

    cctalk_config.inhibit_msg[2] = CRC[0];
    cctalk_config.inhibit_msg[size-1] = CRC[1];

}

int SendMessage(){

if ( !FireFly.ConfigureCCTalkPort( portNumBill, &cctalk_config ) )
        return close();

        had_response = FALSE;

            do
            {

                BOOL issame = true;

                // Receive data
                if ( !FireFly.ReceivePolledMessage( portNumBill, deviceNum, rxMsg, &bytesRxd, &is_inhibited ) )
                    return close();

                // Print received data
                if ( bytesRxd > 0 )
                {
                    had_response = TRUE;

                    for (unsigned int i=0; i<bytesRxd; i++ ) {
                        if(rxMsg[i] != rxMsglast[i]){ issame = false; break;}
                    }

                    if (!issame) { // print only if response is different
                        printf( "unique CCTalk Response = [ " );
						for (unsigned int i = 0; i<bytesRxd; i++) printf("%02X ", rxMsg[i]);
                    }

                    
					for (unsigned int i = 0; i<bytesRxd; i++) rxMsglast[i] = rxMsg[i];

                    // Delete message
                    if ( !FireFly.DeletePolledMessage( portNumBill, 0 ) )
                        return close();
                }
                else
                {
                    if ( !had_response ) {
                        if (!issame) printf( "No Response. " );
                    }

                    if (!issame) {
                        if ( is_inhibited ) printf( "Device IS inhibited.\n" );
                        else printf( "Device is NOT inhibited.\n" );
                    }
                }
            } while ( bytesRxd > 0 );
            //printf( "\n" );
            return 0;

}

void InitializeBill(){

    cctalk_config.device_number = deviceNum;
    cctalk_config.method = Repeated;
    cctalk_config.next_trigger_device = NO_TRIGGER;
    cctalk_config.poll_retry_count = 5;
    cctalk_config.polling_interval = 200;
    cctalk_config.max_response_time = 2000;
    cctalk_config.min_buffer_space = 20;

    cctalk_config.poll_msg[0] = 0x00;                   // destination address
    cctalk_config.poll_msg[1] = 0x00;                   // transmit data length (after Header, before Checksum)
    cctalk_config.poll_msg[2] = 0x00;                   // source address
    cctalk_config.poll_msg[3] = 0x00;                   // command: E5 = ReadBufferedCredit  FE=SimplePoll
    cctalk_config.poll_msg[4] = 0x00;                   // checksum

    cctalk_config.inhibit_msg[0] = 0x00;
    cctalk_config.inhibit_msg[1] = 0x00;
    cctalk_config.inhibit_msg[2] = 0x00;
    cctalk_config.inhibit_msg[3] = 0x00;
    cctalk_config.inhibit_msg[4] = 0x00;






  /*  Insertinto("28 01 01 99 03 00", 6); //Modify bill operating mode = ACK ( stacker enabled, escrow enabled )
    printarray(cctalk_config.poll_msg,6);
    SendMessage();*/

    usleep(200000);

    //usleep(200000);

    Insertinto("28 08 01 e7 ff ff ff ff ff ff ff ff 00", 13); //Modify inhibit status = ACK
    printarray(cctalk_config.poll_msg,13);
    SendMessage();



	usleep(200000);
   /* usleep(200000);

    Insertinto("28 01 01 e4 01 00", 6); //Modify master inhibit status = ACK
    printarray(cctalk_config.poll_msg,6);
    SendMessage();*/

    initializedBill = TRUE;
    IsReadBillBuffer = FALSE;
}

void AcceptBill(){
    Insertinto("28 01 01 9a 01 00", 6);
    IsReadBillBuffer = FALSE;
    SendMessage();
}

void SetReadBillBuffer(){
    Insertinto("28 00 01 9f 00", 5);
    IsReadBillBuffer = TRUE;
}

void SetGetAddressBuffer(){
	Insertinto("00 00 01 fe 00", 5);
	IsReadBillBuffer = FALSE;
}


/*
* Setters of acceptor state and getters of address - call after initialization
*/

/*
Get address of bill acceptor if any
NOT WORKING - get address from checksum
*/
JNIEXPORT jint JNICALL Java_com_ag_game_card_GetAcceptorAddressBill
(JNIEnv *, jobject)
{
	if (!initialized) {
		printf("\nReturning - Heber not initialized");
		return -1;
	}

	if (!initializedBill) {
		InitializeBill();
	}

	SetGetAddressBuffer();

	//printarray(cctalk_config.poll_msg,5);

	SendMessage();

	if (had_response){

		AddressAcceptorBill = rxMsg[2];
		return AddressAcceptorBill;

	}
	else {
		return -2;
	}

}

/*
Get address of coin acceptor if any
valid response is > 0 - adress
-1 when not initialzed
-2 if error while reading
-3 if no bytes read
*/
JNIEXPORT jint JNICALL Java_com_ag_game_card_GetAcceptorAddressCoin
(JNIEnv *, jobject){

	BYTE                msg[MAX_CCTALK_RX_LEN + 1];      // Create buffer for message to receive
	long unsigned int   bytesRead = 0;

	if (!initialized) {
		return -1;
	}

	configure("000 000 001 254");

	if (!FireFly.ReceivePolledMessage(portNum, deviceNum, msg, &bytesRead, &inhibited)) {
		//bytesRead = 0;
		logError(__FILE__, __LINE__);
		//configure();
		return -2;
	}

	if (inhibited) {
		printf("is inhibited\n");
	}

	if (bytesRead > 0)
	{
		AddressAcceptorCoin = msg[2];
		return AddressAcceptorCoin;
	}
	else return -3;

}

/*
Enable/Disable Bill acceptor
*/
JNIEXPORT jboolean JNICALL Java_com_ag_game_card_EnableAcceptorBill
(JNIEnv *, jobject){


	printf("Enabling Bill Acceptor\n");

	if (!initialized) {
		//init();
		printf("\nReturning - Heber not initialized");
		return false;
	}

	if (!initializedBill) {
		//InitializeBill();
		printf("\nReturning - Bill acceptor not initialized");
		return false;
	}
	usleep(200000);

	Insertinto("28 08 01 e7 ff ff ff ff ff ff ff ff 00", 13); //Modify inhibit status = ACK
	printarray(cctalk_config.poll_msg, 13);
	SendMessage();

	usleep(200000);

	IsReadBillBuffer = FALSE;

	printf("Bill Acceptor enabed\n");

	return true;
}

JNIEXPORT jboolean JNICALL Java_com_ag_game_card_DisableAcceptorBill
(JNIEnv *, jobject){


	printf("Disabling Bill Acceptor\n");

	if (!initialized) {
		//init();
		printf("\nReturning - Heber not initialized");
		return false;
	}

	if (!initializedBill) {
		//InitializeBill();
		printf("\nReturning - Bill acceptor not initialized");
		return false;
	}
	usleep(200000);

	Insertinto("28 08 01 e7 00 00 00 00 00 00 00 00 00", 13); //Modify inhibit status = ACK
	printarray(cctalk_config.poll_msg, 13);
	SendMessage();

	usleep(200000);

	IsReadBillBuffer = FALSE;

	printf("Bill Acceptor disabled\n");

	return true;
}


/*
Enable/Disable Coin acceptor
*/
JNIEXPORT jboolean JNICALL Java_com_ag_game_card_EnableAcceptorCoin
(JNIEnv *, jobject){
	return false;
}

JNIEXPORT jboolean JNICALL Java_com_ag_game_card_DisableAcceptorCoin
(JNIEnv *, jobject){
	return false;
}


/*
End of On/Off, adress
*/


JNIEXPORT jintArray JNICALL Java_com_ag_game_card_getLastBill
(JNIEnv *env, jobject obj){

	jintArray returnValue = env->NewIntArray(2);
	jint *narr = env->GetIntArrayElements(returnValue, 0);

     if (!initialized) {
        //init();
		 printf("\nReturning - Heber not initialized");
		 narr[0] = -1; //hodnota bankovky
		 narr[1] = -1; //id bankovky
		 env->ReleaseIntArrayElements(returnValue, narr, 0);
		 return returnValue;
    }

    if (!initializedBill) {
        InitializeBill();
		printf("\nReturning - Bill acceptor not initialized");
		narr[0] = -2; //hodnota bankovky
		narr[1] = -2; //id bankovky
		env->ReleaseIntArrayElements(returnValue, narr, 0);
		return returnValue;
    }

    if(!IsReadBillBuffer) SetReadBillBuffer();

    //printarray(cctalk_config.poll_msg,5);

    SendMessage();

    if(had_response){

        newCounterBill = rxMsg[4];

        //length = msg[1];
        if (newCounterBill == 0) {
            if (counterBill != 0) {
                //printf("");
                //syslog(LOG_ERR, "Unexpected reset of the device");
                counterBill = 0;
                resetLoggedBill = TRUE;
            }
        } else {
            if (counterBill != newCounterBill) { //ak sa zmenil event
                counterBill = newCounterBill;

                int ResultA = rxMsg[5]; //bankovka
                int ResultB = rxMsg[6]; //status

                if(ResultB == 1 && ResultA != 0 && BillinEscrow == 0){ //ak je bankovka zadrzana a id bankovky nieje nulove
                    BillinEscrow = 1;
                    AcceptBill();
                }
                else if(ResultB == 0 && ResultA != 0 && BillinEscrow == 1){ //ak je zadrzana bankovka akceptovana
                    printf("\nbankovka: %x\nSaving to memory\n",ResultA);
					
					int id = SetSavedBill(ResultA); //write to memory

                    BillinEscrow = 0;
					narr[0] = ResultA; //hodnota bankovky
					narr[1] = id; //id bankovky
					env->ReleaseIntArrayElements(returnValue, narr, 0);
					return returnValue;
				}
				else if (ResultA == 0 && BillinEscrow == 1) { BillinEscrow = 0; }
            } 
        }
		//printf("\nReturning last bill value");
		int* lastsavedbill = GetSavedBill();
		narr[0] = lastsavedbill[0];
		narr[1] = lastsavedbill[1];
		env->ReleaseIntArrayElements(returnValue, narr, 0);
		return returnValue;   //return last saved bill from memory
    }
    else { // device not responding
		 printf("\nBill acceptor not responding");
         initializedBill = FALSE; 
		 narr[0] = -3; //hodnota bankovky
		 narr[1] = -3; //id bankovky
		 env->ReleaseIntArrayElements(returnValue, narr, 0);
		 return returnValue;
    }  


  }

/**
 * End of Bill acceptor
 */

/**
* Pulse counters
*/

bool sendCounterPulse(int output_bit_id)
{
	BOOL bPulseComplete, bCurrentDetected;
	BYTE byTimeRemaining;
	int PULSE_DURATION_MS = 100;

		printf("Performing %dms pulse to output ", PULSE_DURATION_MS);
		printf("OP%d ... ", output_bit_id);
		

		if (!FireFly.PulseOutput(output_bit_id, PULSE_DURATION_MS))
		{
			printf("\nFailure %d pulsing output bit.\n", (int)FireFly.GetLastError());
			return false;
		}

		do
		{
			if (!FireFly.PulseOutputResult(&byTimeRemaining, &bPulseComplete, &bCurrentDetected))
			{
				printf("\nFailure %d obtaining pulse output result.\n", (int)FireFly.GetLastError());
				return false;
			}
		} while (!bPulseComplete);

		if (bCurrentDetected) printf("current detected.\n");
		else printf("no current detected.\n");

		return true;
	
}

JNIEXPORT jboolean JNICALL Java_com_ag_game_card_sendCounterOnePulse
(JNIEnv *, jobject){
	return sendCounterPulse(OUTPUT_BIT_OP0);
}

JNIEXPORT jboolean JNICALL Java_com_ag_game_card_sendCounterTwoPulse
(JNIEnv *, jobject){
	return sendCounterPulse(OUTPUT_BIT_OP1);
}

JNIEXPORT jboolean JNICALL Java_com_ag_game_card_sendCounterThreePulse
(JNIEnv *, jobject){
	return sendCounterPulse(OUTPUT_BIT_OP2);
}

/**
* Coins
*/

JNIEXPORT jint JNICALL Java_com_ag_game_card_getLastCoin
  (JNIEnv *env, jobject obj) {
    int                 coin = 0;

    if (!coins.empty()) {
        coin = coins.front();
        coins.pop();
        return coin;
    }

    BYTE                msg[MAX_CCTALK_RX_LEN + 1];      // Create buffer for message to receive
    long unsigned int   bytesRead = 0;
    unsigned int        i = 0;
    //BYTE                length;
    //BYTE                checksum;
    int                 checksum;
    unsigned int        newCounter = 0;
    unsigned int        events = 0;
    
    if (!initialized) {
        //init();
		return 0;
        
    }
    
	configure();

    if (!FireFly.ReceivePolledMessage(portNum, deviceNum, msg, &bytesRead, &inhibited)) {
        //bytesRead = 0;
        logError(__FILE__, __LINE__);
        //configure();
        return 0;
    }
    
    if (inhibited) {
        printf("is inhibited\n");
    }
    
    if (bytesRead > 0)
    {
      //  printf("read %d\n", (int)bytesRead);
        checksum = 0;
        for (i=0; i<bytesRead; i++)
        {
        //    printf("%02X ", msg[i]);
            checksum += msg[i];
        }
        //printf("\n");

        //printf("checksum %d, %d\n", checksum, checksum % 256);
        if (checksum % 256 == 0) {
            //FireFly.DeletePolledMessage(portNumBill, 0);
        }
        
        newCounter = msg[4];
        //length = msg[1];
        if (newCounter == 0) {
            if (counter != 0) {
                //printf("");
                syslog(LOG_ERR, "Unexpected reset of the device");
                counter = 0;
                resetLogged = TRUE;
            }
        } else {
            if (counter == 0) {
                if (newCounter > 0 && resetLogged) {
                    events = newCounter;
                    resetLogged = FALSE;
                }
            } else if (counter < newCounter) {
                events = newCounter - counter;
            } else if (counter > newCounter) {
                events = 256 - counter - 1 + newCounter;
            }
        }

        events = (events > 5) ? 5 : events;

        for (i=0; i<events; i+=2) {
            coin = msg[5+i];
            if (coin > 0) {
                coins.push(coin);
            } else {
                coins.push(-msg[6+i]);
            }
        }
        counter = newCounter;
    }

    if (!coins.empty()) {
        coin = coins.front();
        coins.pop();
        return coin;
    }

    return 0;
}



JNIEXPORT jintArray JNICALL Java_com_ag_game_card_GetButtons
  (JNIEnv *env, jobject obj) {

    jintArray       returnValue = env->NewIntArray(5);
    jint            *narr;
    //usbInput        changedInputs;
    BYTE            inputs[USB_IN_LENGTH] = {0};
    FireFlyUSB      *FireFlyPointer = &FireFly;
    int             i;

    if (!initialized) {
       // init();

		//TODO: return nothing
		narr = env->GetIntArrayElements(returnValue, 0);

		for (i = 0; i<4; i++) {
			narr[i] = 255;
		}

		env->ReleaseIntArrayElements(returnValue, narr, 0);
		return returnValue;

    }

   // FireFlyPointer->GetChangedInputs(&changedInputs);
    FireFlyPointer->GetInputs(inputs);

    narr = env->GetIntArrayElements(returnValue, 0);

    for (i=0; i<4; i++) {
        narr[i] = inputs[i];
    }

    env->ReleaseIntArrayElements(returnValue, narr, 0);
    return returnValue;
}



#define BINARY_PATTERN "%d%d%d%d%d%d%d%d"
#define BINARY(byte)  \
  (byte & 0x80 ? 1 : 0), \
  (byte & 0x40 ? 1 : 0), \
  (byte & 0x20 ? 1 : 0), \
  (byte & 0x10 ? 1 : 0), \
  (byte & 0x08 ? 1 : 0), \
  (byte & 0x04 ? 1 : 0), \
  (byte & 0x02 ? 1 : 0), \
  (byte & 0x01 ? 1 : 0) 



JNIEXPORT void JNICALL Java_com_ag_game_card_controlBrightness
  (JNIEnv *env, jobject obj, jint on, jint off, jint brightness) {

    usbOutput       onPeriods;
    usbOutput       offPeriods;
    int             i;

    //init();

    onPeriods.byAux = 0x00;
    offPeriods.byAux = 0x00;
    //printf("\t%x\t\t(%d)\t\t%x\t\t(%d)\t%d\n", on, on, off, off, brightness);

    //for (i=3; i>=0; i--) {
    for (i=0; i<4; i++) {
        onPeriods.byOut[i] = (BYTE)(on % 256);
        offPeriods.byOut[i] = (BYTE)(off % 256);
        on /= 256;
        off /= 256;
       /* printf("%d\t"BINARY_PATTERN"\t(%x\t%d)\t"BINARY_PATTERN"\t(%x\t%d)\n", i,
            BINARY(onPeriods.byOut[i]), onPeriods.byOut[i], onPeriods.byOut[i],
            BINARY(offPeriods.byOut[i]), offPeriods.byOut[i], offPeriods.byOut[i]);*/
    }

    //onPeriods.byOut[3] = 2<<13;

    if (brightness < 0) {
        brightness = 0;
    } else if (brightness > 10) {
        brightness = 10;
    }  

    if (!FireFly.SetOnPeriodOutputs(onPeriods)) {
        close();
    }
    if (!FireFly.SetOffPeriodOutputs(offPeriods)) {
        close();
    }
    if (!FireFly.SetOutputBrightness(brightness)) {
        close();
    }
}



JNIEXPORT void JNICALL Java_com_ag_game_card_pulseStart
  (JNIEnv *env, jobject obj, jint output, jint duration) {
    BYTE            remaining = 0;
    BOOL            complete = FALSE;
    BOOL            current = FALSE;

	if (!initialized){
		return;
	}

    if (!FireFly.PulseOutput(output, duration)) {
        close();
    }

    // 0, 1, 2 are mechanical counters, wait for them to finish
    if (output >= 0 && output <= 2) {
        do {
            FireFly.PulseOutputResult(&remaining, &complete, &current);
        } while (!complete);
    }
}



JNIEXPORT jint JNICALL Java_com_ag_game_card_pulseRemaining
  (JNIEnv *env, jobject obj) {
    jint            result = -1;
    BYTE            remaining = 0;
    BOOL            complete = FALSE;
    BOOL            current = FALSE;

	if (!initialized) {
		//init();
		return 0;
	}

    if (FireFly.PulseOutputResult(&remaining, &complete, &current)) {
        if (complete) {
            result = -1;
        } else {
            result = (int)remaining;
        }
    } else {
        close();
    }

    return result;
}




int main() {
    return 1;
}

