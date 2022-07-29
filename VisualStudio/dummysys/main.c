#include <windows.h>
#include <winbase.h>
#include "resource.h"
#include <winuser.h >
#include <stdio.h>
#include <mmsystem.h>
#include <process.h>
#include "sci.h"
HINSTANCE hInst;
int Timer1;
void main(void *);
void TIMER1ms();
void HardInit();
VOID CALLBACK _TIMER1ms( UINT uTimerID, UINT uMsg,DWORD dwUser, DWORD dwParam1, DWORD dwParam2 );
int f;
int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd){

	MSG msg;
	TIMECAPS p;
	AllocConsole();
	freopen("CONOUT$", "w", stdout); 
	/*À²Ï°‰Šú‰»*/
	timeGetDevCaps(&p,sizeof(TIMECAPS));
	timeBeginPeriod( p.wPeriodMin );
	HardInit();

	if(p.wPeriodMin>10){
		MessageBox( NULL, "‚±‚ÌPC‚Å‚Í“®ì‚Å‚«‚Ü‚¹‚ñ","À²Ï²İÀ°ÊŞÙ´×°", MB_OK );
		exit(0);
	}

	Timer1=timeSetEvent( 1, 0, (LPTIMECALLBACK)_TIMER1ms, 0, TIME_PERIODIC );
	_beginthread(main,0,NULL);

	while(GetMessage(&msg,NULL,(UINT)NULL,(UINT)NULL)){


		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if(f==0) break;
	}
	/*I—¹ˆ—@À²ÏØ¿°½‚ÌŠJ•ú*/
	timeEndPeriod( p.wPeriodMin );
	timeKillEvent( Timer1 );

//	KillTimer(0,TimerID);
	return 0;
}


VOID CALLBACK _TIMER1ms( UINT uTimerID, UINT uMsg,
					   DWORD dwUser, DWORD dwParam1, DWORD dwParam2 ){
	TIMER1ms();
}



VOID alive(int a){
	f=a;
}

