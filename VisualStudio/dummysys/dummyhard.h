#ifndef __dummyhard
#define __dummyhard
#define DEBUG_LOG_SEND
#define DEBUG_LOG_RCV

#define COM 0
#define SIM 100
#define TCP 100
//#define USEPORT COM1
#define USEPORT COM
/*�ꕶ����M*/
int commGetc();
/*�ꕶ�����M*/
void commPutc(char c);
/*���ю��Ԃ̎擾*/
int TimeCount();
/*��۸ޓ��͒l���擾0�`255*/
int GetSlider();
/*��۸ޏo��(Ұ�)�ɒl���0�`255*/
void SetLevel(int i);
/*��۸ޏo��(���l)�ɒl��ā@val:��Ēl no:��Ă��鶳��*/
void SetCT(int val,int no);
/*���\�����e��ݒ�@msg:�\�����镶����*/
void SetInfo(unsigned char *msg);
/*���\�����e���擾 buf:�i�[�̈�,length:�ő�SIZE*/
char *GetInfo(char *buf,int length);
/*�޼��ُo�͂̐ݒ�*/
void SetLamp(int state);
/*�޼��ُo�͏�Ԃ̎擾*/
int GetLamp();
/*�޼��ٓ��͏�Ԃ̎擾*/
int GetBtn();
/*�w�莞�ԑҋ@*/
void Wait(int i);
/*��ϊ��荞������ׂ̓o�^*/
void SetTimerFunc(void (*t)());
int SYSBEEP();
#endif