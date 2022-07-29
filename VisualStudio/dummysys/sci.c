#include <winsock2.h>
//#include <sys/time.h>
#include <windows.h>
#include <winbase.h>
#include "sci.h"
#include "dummyhard.h"
#include <process.h>
#include <setupapi.h>

#define BUFF_MAX 5000


static int issock=0;
static HANDLE hPort;
static unsigned char buf[100];
static unsigned char linebuf[100];
static int linecount;
static unsigned char recvBuff[BUFF_MAX],sendBuff[BUFF_MAX];
static volatile int inCount,outCount,CRCount,SendinCount,SendoutCount,SendCRCount;
static int volatile alive=0,rt=0,st=0;
CRITICAL_SECTION sciPort;

void CloseSocket();
int OpenSocket();
void rcvTsk(void *id);
void sendTsk(void *id);

extern HWND m_hDlg;


#define COMERR(title)	{\
							LPVOID lpMsgBuf;\
							FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),(LPTSTR)&lpMsgBuf,0,NULL);\
							MessageBox( NULL, lpMsgBuf,title, MB_OK | MB_ICONINFORMATION );\
							LocalFree( lpMsgBuf );\
						}

int GetCTSSta()
{
	if(USEPORT!=SIM){
		unsigned long val;
		GetCommModemStatus(hPort,&val);
		return val&MS_CTS_ON?1:0;
	}else{
		return 1;
	}
}
void SetRTSSta(int sta)
{
	DCB dcb;
	memset(&dcb, '\0', sizeof dcb);
	EscapeCommFunction(hPort,sta?SETRTS:CLRRTS);  // handle to communications device
}

// CDataTransferDlg メッセージ ハンドラ
char* GetCommPort(char *buf)
{
	// 色々初期化
	DWORD i;
	BOOL ret = FALSE;
	BYTE Buffer[256]; //領域確保（てきとう）
	DWORD Length = 0;
	SP_DEVINFO_DATA DeviceInfoData = { sizeof(SP_DEVINFO_DATA) };  /// １件デバイス情報
	HDEVINFO hDevInfo = 0;  // 列挙デバイス情報
	int detect_flag = 0;
	//	AllocConsole();
	//	freopen("CONOUT$", "w", stdout);
	// COMポートのデバイス情報を取得
	hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_COMPORT, NULL, m_hDlg, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));
	// 列挙の終わりまでループ
	for (i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++) // １件取得
	{
		// COMポート番号を取得
		detect_flag = 0;
		SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_HARDWAREID, NULL, Buffer, sizeof(Buffer), &Length);
		//		printf("%s\n",hwid);
		if (strstr(Buffer,"PID_6011")) {	//FDIT 4232H のPID
			char *s = (char *)Buffer;
			ret = TRUE;
			s = s + strlen(s) + 1;
			//			printf("%s\n",hwid);
			if (strstr(s,"MI_02")) {	//CH3のPID
				HKEY tmp_key = SetupDiOpenDevRegKey(hDevInfo, &DeviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
				if (tmp_key) {
					DWORD tmp_type = 0;
					DWORD tmp_size = sizeof(Buffer);
					RegQueryValueEx(tmp_key, "PortName", NULL, &tmp_type, Buffer, &tmp_size);
					strcpy(buf, Buffer);
					return buf;
				}
			}
		}
	}
	return NULL;
}



int PortInit(int portno){
	char *portname,buf[32],buf2[32];
	portname = GetCommPort(buf);
	if (!portname)return;
	strcpy(buf2, "\\\\.\\");
	strcat(buf2, portname);
	portname = buf2;
	if((hPort=(HANDLE)CreateFile(portname,GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL))==0){
		return 0;
	}
	{
		//通信設定
		DCB stDcb;
		GetCommState(hPort,&stDcb);
//		stDcb.fBinary=TRUE;
//		stDcb.BaudRate=CBR_4800;
//		stDcb.BaudRate=CBR_9600;
		stDcb.BaudRate=CBR_38400;
//		stDcb.BaudRate=CBR_115200;
		stDcb.ByteSize=8;
//		stDcb.Parity=EVENPARITY;//NOPARITY;
		stDcb.Parity=NOPARITY;
		stDcb.StopBits=ONESTOPBIT;
		SetCommState(hPort,&stDcb);
	}
	SetupComm(hPort,2000,2000);
	{
		//タイムアウト設定
		COMMTIMEOUTS stTimeOut;
		stTimeOut.ReadIntervalTimeout = MAXDWORD;
		stTimeOut.ReadTotalTimeoutMultiplier = 0;
	    stTimeOut.ReadTotalTimeoutConstant = 0;
		stTimeOut.WriteTotalTimeoutMultiplier = 0;
		stTimeOut.WriteTotalTimeoutConstant = 0;
		SetCommTimeouts(hPort,&stTimeOut);
	}
	linecount=0;
	inCount=0;
	outCount=0;
	CRCount=0;
	SendinCount=0;
	SendoutCount=0;
	SendCRCount=0;
	alive=1;
	InitializeCriticalSection(&sciPort);
	return -1;
}

void _PortClose(){
	DeleteCriticalSection(&sciPort);
	CloseHandle(hPort);
}
void PortClose(){
	alive=0;
	while(rt!=0||st!=0)Sleep(1);
	if(issock==0)_PortClose();else CloseSocket();
}

void PortSetRate(int rate){
	DCB stDcb;
	GetCommState(hPort,&stDcb);
	stDcb.BaudRate=rate;
	SetCommState(hPort,&stDcb);
}


void SetRTS(int mode){
	DCB dcb;
	memset(&dcb, '\0', sizeof dcb);
	EscapeCommFunction(hPort,mode?SETRTS:CLRRTS);  // handle to communications device
}
int _commGetc(){
	static char rbuf[64],*rtop;
	static DWORD readlen=0;
	int ret;
	BOOL r;
	if (!readlen) {
		memset(rbuf, 0, 64);
		r = ReadFile(hPort, (LPVOID)rbuf, (DWORD)64, &readlen, NULL);
		if (r == 0) {
			COMERR("commGetc")
				exit(0);
		}
		if (readlen == 0)return -1;
		rtop = rbuf;
	}
	ret = *rtop++;
	readlen--;
	return ret;

}

void _commPutc(char c){
	char *buf_start;
	DWORD writelen;
	BOOL r;
	buf_start=buf;
	*buf_start++=c;
	*buf_start=0;
	r=WriteFile(hPort,(LPVOID)buf,(DWORD)1,&writelen,NULL);
	if(r==0){
		COMERR("commPutc")
		exit(0);
	}
}
#if USEPORT!=SIM
int commGetc(){
	int c;
	c=_commGetc();
#ifdef DEBUG_LOG_RCV
	if(c>=0x20)	putchar(c);
	if (c == '\r')	putchar('\n');
#endif
	return c;
}

void commPutc(unsigned char c){
	_commPutc(c);
#ifdef DEBUG_LOG_SEND
	putchar(c);
	if(c=='\r')	putchar('\n');

#endif
}
#else
int commGetc(){
	int c;
	if(outCount==inCount)return -1;
	else{
		c=recvBuff[outCount];
		outCount=(outCount+1)%BUFF_MAX;
		return c;
	}
}
void commPutc(char c){
	sendBuff[SendinCount] = c;
	SendinCount = (SendinCount + 1) % BUFF_MAX;
	
}
#endif
void commPuts(char *s){
	while(*s!=0){
		commPutc(*s++);
	}
	commPutc('\r');
	commPutc('\n');
}


char *commGets(){
	if(CRCount==0) return NULL;
	linecount=0;
	while(1){
		if(recvBuff[outCount]==0x0d){
			outCount= (outCount+1)%BUFF_MAX;
			linebuf[linecount]='\0';
			CRCount--;
			return linebuf;
		}
		linebuf[linecount++]=recvBuff[outCount];
		outCount = (outCount + 1) % BUFF_MAX;


	}


}







int PortEnable(){
	return alive;
}

void CommConnect(int portNo){
	if(portNo!=100){
		issock=0;
		PortInit(portNo);
		PortSetRate(38400);
	}else{
		issock=1;
		OpenSocket();
//		_beginthread(rcvTsk,0,NULL);
//		_beginthread(sendTsk,0,NULL);

	}
}

SOCKET m_soc;

void CloseSocket(){
	/* 送受信を無効にする */
	shutdown((unsigned int)m_soc, 2);
	/* ソケットを破棄する */
	closesocket((unsigned int)m_soc);
	m_soc=NULL;
}

int OpenSocket(){
	struct  sockaddr_in	 serversockaddr;		/* サーバーのアドレス */
	unsigned long serveraddr;		/* サーバーのIPアドレス */
	WSADATA data;
	struct timeval sttimeout;
	WSAStartup(MAKEWORD(2,0), &data);
	m_soc	 = socket(PF_INET, SOCK_STREAM, 0);
	if(m_soc == (void *)INVALID_SOCKET){
		return 0;
	}
	/* svNameにドットで区切った10進数のIPアドレスが入っている場合、serveraddrに32bit整数のIPアドレスが返ります */
//	serveraddr = inet_addr("172.16.49.71");
	serveraddr = inet_addr("127.0.0.1");
	/* サーバーのアドレスの構造体にサーバーのIPアドレスとポート番号を設定します */
	serversockaddr.sin_family	 = AF_INET;				/* インターネットの場合 */
	serversockaddr.sin_addr.s_addr  = serveraddr;				/* サーバーのIPアドレス */
	serversockaddr.sin_port	 = htons((unsigned short)50000);		/* ポート番号 */
	memset(serversockaddr.sin_zero,(int)0,sizeof(serversockaddr.sin_zero));
			/* 指定のソケットでサーバーへコネクトします */
	if(connect((unsigned int)m_soc,(struct sockaddr *)&serversockaddr,sizeof(serversockaddr)) == SOCKET_ERROR){
		return 0;
	}
	sttimeout.tv_sec=0;
	sttimeout.tv_usec=1;
	setsockopt((unsigned int)m_soc,SOL_SOCKET ,SO_RCVTIMEO,(char *)&sttimeout,sizeof(sttimeout));
	alive=1;
	_beginthread(rcvTsk,0,NULL);
	_beginthread(sendTsk,0,NULL);
}


void rcvTsk(void *id){
	char rbuf[100];
	int rcv,i;
	rt=1;
	while(alive){
	    struct timeval timeout = {0, 0};
		fd_set rfds, wfds, efds;
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		FD_ZERO(&efds);
		FD_SET(m_soc, &rfds);
		FD_SET(m_soc, &wfds);
		FD_SET(m_soc, &efds);
		alive=1;
//		printf("*");
		if(select(m_soc +1, &rfds, &wfds, &efds, &timeout)){
			if(FD_ISSET(m_soc, &rfds)){
				rcv=recv((unsigned int)m_soc,rbuf,100,0);
				for(i=0;i<rcv;i++){
					recvBuff[inCount]=rbuf[i];
					inCount = (inCount + 1) % BUFF_MAX;
				}
			}
		}
//		printf("-");
		Sleep(5);
	}
	rt=0;
	alive=0;
}
void sendTsk(void *id){
	int len=0;
	char sbuf[100];
	st=1;
	printf("sendtsk\n");
	while(alive){
		len=0;
		while(SendinCount!=SendoutCount){
			sbuf[len]=sendBuff[SendoutCount];
			SendinCount = (SendinCount + 1) % BUFF_MAX;
			len++;
			if(len>=100)break;
		}
		if(len!=0){
			send(m_soc,sbuf,len,0);
		}
		Sleep(5);
	}
	st=0;
}
