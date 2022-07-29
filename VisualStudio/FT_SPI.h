#include<windows.h>
#define SPI_STATE_BUSY		0x01	//�R�}���h�ɂ�铮�쒆
#define SPI_STATE_ERROR		0x02	//�R�}���h�����s�ł��Ȃ�(�t�@�C���Ƀf�[�^���������߂Ȃ��A�������[�������������Ȃ�)
#define SPI_STATE_RETRY		0x04	//��M�R�}���h����������(SUM�s��v�A�A�Ԃ͂���Ȃ�)
#define SPI_STATE_STALL		0x08	//�R�}���h�^�C���A�E�g(�R�}���h��������Ԃň�莞�Ԗ��ʐM)
#define SPI_STATE_ERRCMD	0x10	//�s���R�}���h
#define SPI_STATE_READREADY	0x20	//�ԐM�f�[�^�Z�b�g
#define SPI_STATE_ERRINT	0x40	//���荞�݃G���[
#define SPI_STATE_BLOCK		0x80	//�u���b�N�s�A��

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
