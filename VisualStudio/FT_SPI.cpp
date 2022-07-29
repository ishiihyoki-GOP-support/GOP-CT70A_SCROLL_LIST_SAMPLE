
/* Standard C libraries */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
/* OS specific libraries */
#include<windows.h>

/* Include D2XX header*/
#include "ftd2xx.h"
/* Include libMPSSE header */
#include "libMPSSE_spi.h"
#include "FT_SPI.h"

#include "win_rapper.h"


#define SPICMD_DATA_START				0xa0
#define SPICMD_DATA_ERASE				0xa1
#define SPICMD_FILE_HEAD				0xa2
#define SPICMD_FILE_BLOCK				0xa3
#define SPICMD_DATA_END					0xa4
#define SPICMD_READ_LASTBLOCK			0xa8

#define SPICMD_MEM_READ_REQ				0xc0
#define SPICMD_MEM_READ_READ			0xc1
#define SPICMD_MEM_WRITE				0xc2
#define SPICMD_ENQ_REQ					0xc3
#define SPICMD_ENQ_READ					0xc4
#define SPICMD_MEM_BI_READ_REQ			0xc5
#define SPICMD_MEM_BIOffset_READ_REQ	0xcd
#define SPICMD_MEM_BI_READ_READ			0xc6
#define SPICMD_MEM_BI_WRITE				0xc7
#define SPICMD_MEM_BIOffset_WRITE		0xcf
#define SPICMD_SPI_RESET				0xff

/******************************************************************************/
/*								Macro and type defines							   */
/******************************************************************************/
/* Helper macros */
#define GET_FUN_POINTER	GetProcAddress
#define CHECK_STATUS(exp) {if(exp!=FT_OK){return FALSE;}};
#define APCHECK_STATUS(exp) {if(exp!=FT_OK){return SPI_ERR;}};
#define APCHECK_STATUS_B(exp) {if(exp!=FT_OK){ret=SPI_ERR;goto end;}};
#define CHECK_NULL(exp) {if(exp==NULL){return FALSE;}};

/* Application specific macro definations */
#define SPI_DEVICE_BUFFER_SIZE		256
#define SPI_WRITE_COMPLETION_RETRY		10
#define START_ADDRESS_EEPROM 	0x00 /*read/write start address inside the EEPROM*/
#define END_ADDRESS_EEPROM		0x10
#define RETRY_COUNT_EEPROM		10	/* number of retries if read/write fails */
#define CHANNEL_TO_OPEN			0	/*0 for first available channel, 1 for next... */
#define SPI_SLAVE_0				0
#define SPI_SLAVE_1				1
#define SPI_SLAVE_2				2
#define DATA_OFFSET				4
#define USE_WRITEREAD			0


CRITICAL_SECTION ftspi;

/******************************************************************************/
/*								Global variables							  	    */
/******************************************************************************/
static FT_HANDLE ftHandleA=NULL;
static FT_HANDLE ftHandleB=NULL;
static uint8 buffer[SPI_DEVICE_BUFFER_SIZE] = {0};

int GetSPIChannels()
{
	FT_STATUS status = FT_OK;
	uint32 channels = 0;

	status = SPI_GetNumChannels(&channels);
	APCHECK_STATUS(status);
	return (int)channels;
}


extern "C"
int OpenSPI(int rate)
{
	ChannelConfig channelConf = {0};
	uint8 address = 0;
	uint32 channels = 0;
	uint16 data = 0;
	uint8 i = 0;
	uint8 latency = 10;	
	FT_STATUS status = FT_OK;
	if (!GetSPIChannels())return FALSE;
	channelConf.ClockRate = rate;
	channelConf.LatencyTimer = latency;
	//FT4232Hの仕様上の不具合のためモード1,3ではそのままでは使用できない。
	//※初回の奇数エッジの前に無理やりクロックをアサート→ネゲートとするため、アイドル→アサート→ネゲートの波形となりごみが出る
	//これを対策するためモードを0で動作し、クロックのごみを防止し、モード1のタイミングにあわすためDOにD-FFを追加。
	//そのため立ち上がりでのラッチとなるためモード１以外で使用できません。
	//下記でconfigOptionsでモード1を指定してますが、SPI_InitChannelを修正しモード1はモード0と同じ動作となっています。
	channelConf.configOptions = SPI_CONFIG_OPTION_MODE1 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
	channelConf.Pin = 0x80808080;/*FinalVal-FinalDir-InitVal-InitDir (for dir 0=in, 1=out)*/

	status = SPI_OpenChannel(0/*channel A*/,&ftHandleA);
	CHECK_STATUS(status);
	status = SPI_InitChannel(ftHandleA,&channelConf);
	CHECK_STATUS(status);
	status = SPI_OpenChannel(1/*channel B*/,&ftHandleB);
	CHECK_STATUS(status);
	channelConf.configOptions = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
	channelConf.Pin = 0x00000000;/*FinalVal-FinalDir-InitVal-InitDir (for dir 0=in, 1=out)*/
	status = SPI_InitChannel(ftHandleB,&channelConf);
	CHECK_STATUS(status);
	InitializeCriticalSection(&ftspi);
//	reset_out(FALSE);
	return TRUE;

}

void CloseSPI()
{
	FT_STATUS status = FT_OK;
	status = SPI_CloseChannel(ftHandleB);
	ftHandleB=NULL;
	status = SPI_CloseChannel(ftHandleA);
	ftHandleA=NULL;
	DeleteCriticalSection(&ftspi);
}

int ReOpenSPI(int rate)
{
	ChannelConfig channelConf = {0};
	uint8 address = 0;
	uint32 channels = 0;
	uint16 data = 0;
	uint8 i = 0;
	uint8 latency = 10;	
	FT_STATUS status = FT_OK;

	EnterCriticalSection(&ftspi);
	status = SPI_CloseChannel(ftHandleA);
	channelConf.ClockRate = rate;
	channelConf.LatencyTimer = latency;
	//FT4232Hの仕様上モード1,3ではそのままでは使用できないためDOにD-FFを追加。
	//そのため立ち上がりでのラッチとなるためモード１以外で使用できません。
	channelConf.configOptions = SPI_CONFIG_OPTION_MODE1 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
	channelConf.Pin = 0x80808080;/*FinalVal-FinalDir-InitVal-InitDir (for dir 0=in, 1=out)*/

	status = SPI_OpenChannel(0/*channel A*/,&ftHandleA);
	CHECK_STATUS(status);
	status = SPI_InitChannel(ftHandleA,&channelConf);
	CHECK_STATUS(status);
	LeaveCriticalSection(&ftspi);

}
int IsSPIOpend()
{
	return (int)ftHandleA;
}
void printbuf(char* mes, uint8_t* buf, int num)
{
	printf(mes);
	for (int i = 0; i < num; i++)
	{
		printf(" %02x", buf[i]);
	}
	printf("\n");
}

MD_STATUS xfer_spi(uint8_t *_buf, uint16_t tx_num)
{
	uint32	wrlen = 0;
	MD_STATUS r = MD_OK;
	uint8_t *readbuf = (uint8_t *)malloc(tx_num);
	memset(readbuf, 0, tx_num);
	EnterCriticalSection(&ftspi);
	r=SPI_ReadWrite(ftHandleA, readbuf,_buf, tx_num, &wrlen,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES
	);
	LeaveCriticalSection(&ftspi);
	memcpy(_buf, readbuf, tx_num);
	free(readbuf);
	return r;
}

MD_STATUS xfer_spi_R(uint8_t* _buf, uint16_t tx_num)
{
	uint32	wrlen = 0;
	MD_STATUS r = MD_OK;
	uint8_t* readbuf = (uint8_t*)malloc(tx_num);
	memset(readbuf, 0, tx_num);
	EnterCriticalSection(&ftspi);
	r = SPI_Read(ftHandleA, readbuf, tx_num, &wrlen,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES
	);
	LeaveCriticalSection(&ftspi);
	printbuf("R_buf", readbuf, tx_num);
	memcpy(_buf, readbuf, tx_num);
	free(readbuf);
	return r;
}
MD_STATUS xfer_spi_W(uint8_t* _buf, uint16_t tx_num)
{
	uint32	wrlen = 0;
	MD_STATUS r = MD_OK;
	uint8_t* readbuf = (uint8_t*)malloc(tx_num);
	memset(readbuf, 0, tx_num);
	printbuf("W_buf", _buf, tx_num);
	EnterCriticalSection(&ftspi);
	r = SPI_Write(ftHandleA, _buf, tx_num, &wrlen,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES
	);
	LeaveCriticalSection(&ftspi);
	memcpy(_buf, readbuf, tx_num);
	free(readbuf);
	return r;
}


void spi_cs(BOOL sta)
{
	EnterCriticalSection(&ftspi);
	SPI_ToggleCS(ftHandleA, sta?true:false);
	LeaveCriticalSection(&ftspi);
}


uint8_t getio()
{
	uint8 sta,ret=0;
	FT_STATUS status = FT_OK;
	if(!ftHandleA)return-1;
	EnterCriticalSection(&ftspi);
	status =FT_ReadGPIO(ftHandleA,&sta);
	LeaveCriticalSection(&ftspi);
	if(status!=FT_OK)return -1;
	ret |= (sta&FTIO_IN_GOPREADY) ? 0x01 : 0x00;
	ret |= (sta&FTIO_IN_SETSENDDATA) ? 0x02 : 0x00;
	return ret;
}

void reset_out(BOOL sta)
{
	FT_STATUS status = FT_OK;
	if(!ftHandleA)return;
	EnterCriticalSection(&ftspi);
	status =FT_WriteGPIO(ftHandleA,FTIO_OUT_RESET,sta?FTIO_OUT_RESET:0);
	LeaveCriticalSection(&ftspi);
}
