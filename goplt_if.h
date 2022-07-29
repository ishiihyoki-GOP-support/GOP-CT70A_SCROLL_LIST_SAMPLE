/*
 * goplt_if.h
 *
 *  Created on: 2019/05/21
 *      Author: k-chigotaki
 */

#ifndef _GOPLT_IF_H_
#define _GOPLT_IF_H_
#ifdef __RL78__
#define FAR __far
#else
#define FAR
#endif
typedef signed char         int8_t;
typedef unsigned char       uint8_t;
typedef signed short        int16_t;
typedef unsigned short      uint16_t;
typedef signed long         int32_t;
typedef unsigned long       uint32_t;
typedef unsigned short      MD_STATUS;

typedef int BOOL;

#define PUSH	1
#define OPEN	0
#define MD_OK	0

#define COMMERR (-1)
#define ERROR_TIMEOUT	0x01
#define ERROR_STX		0x02
#define ERROR_OVERFLOW	0x04
#define ERROR_SUM		0x08
#define ERROR_NAK		0x10

//テキストメモリーの長さ(80文字+終端1)
#define LT_TEXT_LENGTH 81

typedef struct _ltmes_callback {
	const char FAR* mes;
	void (FAR* func)(void);
} ltMes_Callback;

typedef struct _mem_ltmes_callback {
	const char FAR* mem;
	void (FAR* func)(void *);
} ltMem_Callback;

void LtSetMessageCallback(ltMes_Callback FAR* p);
void LtSetMemoryCallback(ltMem_Callback FAR* p);

uint8_t _get_error_code();

BOOL LtMemWrite(char FAR* memname, int32_t val);
BOOL LtTMemWrite(char FAR* memname, int8_t* val);
BOOL LtMemArrayWrite(char FAR* memname, int16_t num, int32_t FAR vals[]);
BOOL LtMemVaListWrite(char FAR* memname, int16_t num, ...);
BOOL LtMemRead(char FAR* memname, int32_t FAR* pval);
BOOL LtTMemRead(char FAR* memname, int8_t FAR* pval);
BOOL LtMemArrayRead(char FAR* memname, int16_t num, int32_t FAR vals[]);
BOOL LtGMemWrite(char FAR* memname, int16_t offset, int16_t num, uint8_t FAR vals[]);
BOOL LtGMemRead(char FAR* memname, int16_t offset, int16_t num, uint8_t FAR vals[]);

const char FAR* LtEnq(char FAR* rcv);


int RamUpload(char* srcname, int blockno, int packsize);




BOOL SPI_LtMemArrayWrite(char FAR* memname, int32_t num, int32_t FAR vals[]);
BOOL SPI_LtMemArrayRead(char FAR* memname, int16_t num, int32_t FAR vals[]);
BOOL SPI_LtTMemWrite(char FAR* memname, uint8_t FAR vals[]);
BOOL SPI_LtTMemRead(char FAR* memname, uint8_t FAR vals[]);
BOOL SPI_LtGMemWrite(char FAR* memname, int16_t offset, int16_t num, uint8_t FAR vals[]);
BOOL SPI_LtGMemRead(char FAR* memname, int16_t offset, int16_t num, uint8_t FAR vals[]);
const char FAR* SPI_LtEnq(char FAR* rcv);



BOOL SpiRamSend(char* src, int blockno, int size);
BOOL SpiFileSend(char* src, char* dest, int size);
BOOL CopyAllSPI();


#endif
