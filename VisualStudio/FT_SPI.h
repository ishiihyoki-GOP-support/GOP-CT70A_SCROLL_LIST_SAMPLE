#include<windows.h>
#define SPI_STATE_BUSY		0x01	//コマンドによる動作中
#define SPI_STATE_ERROR		0x02	//コマンドを実行できない(ファイルにデータを書き込めない、メモリー名がおかしいなど)
#define SPI_STATE_RETRY		0x04	//受信コマンドがおかしい(SUM不一致、連番はずれなど)
#define SPI_STATE_STALL		0x08	//コマンドタイムアウト(コマンド未完成状態で一定時間無通信)
#define SPI_STATE_ERRCMD	0x10	//不正コマンド
#define SPI_STATE_READREADY	0x20	//返信データセット
#define SPI_STATE_ERRINT	0x40	//割り込みエラー
#define SPI_STATE_BLOCK		0x80	//ブロック不連続

#define FTIO_IN_RESET  0x40
#define FTIO_OUT_RESET  0x80

#define FTIO_IN_GOPREADY  0x20
#define FTIO_IN_SETSENDDATA  0x10


#define FILEBLOCKSIZE 1024
#define SPI_ERR (-1)
#ifdef __cplusplus
extern "C" {
#endif
	int GetSPIChannels();
	int OpenSPI(int rate);
	void CloseSPI();
	int ReOpenSPI(int rate);

	int IsSPIOpend();
#ifdef __cplusplus
}
#endif


int SpiCmd_ReadStatus(UCHAR *sta);
int SpiCmd_SPI_RESET();
int SpiCmd_ENQ_REQ();
int SpiCmd_ENQ_READ(char * rcvstr);
int SpiCmd_MEM_READ_REQ(char * memname,int num);
int SpiCmd_MEM_READ_READ(int num,int *read_buf);
int SpiCmd_MEM_WRITE(char * memname,int num,int *write_buf);
int SpiCmd_MEM_BI_READ_REQ(char * memname,int num);
int SpiCmd_MEM_BIOffset_READ_REQ(char * memname,int offset,int num);
int SpiCmd_MEM_BI_READ_READ(int num,unsigned char *read_buf);
int SpiCmd_MEM_BI_WRITE(char * memname,int num,unsigned char *write_buf);
int SpiCmd_MEM_BIOffset_WRITE(char * memname,int offset,int num,unsigned char *write_buf);
int SpiCmd_DATA_START();
int SpiCmd_DATA_ERASE();
int SpiCmd_DATA_END();
int SpiCmd_FILE_HEAD(char * fname,unsigned int filesize);
int SpiCmd_FILE_BLOCK(unsigned short no,unsigned char *buf);
int SpiCmd_ReadLastBlock(USHORT *no);
int ReadIO();
int ReadReset();
void SetReset(int sta);
