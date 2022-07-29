#ifndef __dummyhard
#define __dummyhard
#define DEBUG_LOG_SEND
#define DEBUG_LOG_RCV

#define COM 0
#define SIM 100
#define TCP 100
//#define USEPORT COM1
#define USEPORT COM
/*一文字受信*/
int commGetc();
/*一文字送信*/
void commPutc(char c);
/*ｼｽﾃﾑ時間の取得*/
int TimeCount();
/*ｱﾅﾛｸﾞ入力値を取得0～255*/
int GetSlider();
/*ｱﾅﾛｸﾞ出力(ﾒｰﾀ)に値をｾｯﾄ0～255*/
void SetLevel(int i);
/*ｱﾅﾛｸﾞ出力(数値)に値をｾｯﾄ　val:ｾｯﾄ値 no:ｾｯﾄするｶｳﾝﾀ*/
void SetCT(int val,int no);
/*情報表示内容を設定　msg:表示する文字列*/
void SetInfo(unsigned char *msg);
/*情報表示内容を取得 buf:格納領域,length:最大SIZE*/
char *GetInfo(char *buf,int length);
/*ﾃﾞｼﾞﾀﾙ出力の設定*/
void SetLamp(int state);
/*ﾃﾞｼﾞﾀﾙ出力状態の取得*/
int GetLamp();
/*ﾃﾞｼﾞﾀﾙ入力状態の取得*/
int GetBtn();
/*指定時間待機*/
void Wait(int i);
/*ﾀｲﾏ割り込みﾊﾝﾄﾞﾗの登録*/
void SetTimerFunc(void (*t)());
int SYSBEEP();
#endif