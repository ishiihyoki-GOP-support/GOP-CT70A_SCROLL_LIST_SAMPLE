#include <windows.h>
#include <winbase.h>
#include <commctrl.h>
#include "resource.h"
#include <winuser.h >
#include <stdio.h>
#include <stdlib.h>
#include "sci.h"
#include "dummyhard.h"
#include <shlobj.h>

VOID alive(int a);
HWND m_hDlg,hBT[4],hLP[4],hSLIDER,hCT[2],hLEVEL,hINFO;

extern void CommConnect();


int SYSBEEP()
{
	MessageBeep(MB_OK);

}
int TimeCount(){
	return (int)timeGetTime();
//	return (int)GetTickCount();
}

int GetSlider(){
	return SendMessage(hSLIDER,TBM_GETPOS,(WPARAM)0,(LPARAM)0);
}

void SetLevel(int i){
	SendMessage(hLEVEL,PBM_SETPOS,(WPARAM)i,(LPARAM)0);
}

static oldctsta[2]={0,0};
void SetCT(int val,int no){
	char temp[20];
	if(oldctsta[no]!=val){
		sprintf(temp,"%d",val);
		SendMessage(hCT[no],WM_SETTEXT,(WPARAM)0,(LPARAM)temp);
	}
	oldctsta[no]=val;
}

void SetInfo(unsigned char *msg){
	SendMessage(hINFO,WM_SETTEXT,(WPARAM)0,(LPARAM)msg);
}
char *GetInfo(char *buf,int length){
	SendMessage(hINFO,WM_SETTEXT,(WPARAM)length,(LPARAM)buf);
	return buf;
}


static int nowState=0;
int GetLamp(){
	return nowState;
}

void SetLamp(int state){
	int i,RBstate;
	for(i=0;i<4;i++){
		if(((state>>i)&0x00000001)==0)RBstate=BST_UNCHECKED;else RBstate=BST_CHECKED;
		SendMessage(hLP[i],BM_SETCHECK,(WPARAM)RBstate, 0L);
	}
	nowState=state;
}


static int _btsta[4]={0,0,0,0};
int GetBtn(){
	int i,sta=0;
	for(i=3;i>=0;i--){
		if(_btsta[i]==1)sta=sta*2+1;else sta*=2;
	}
	return sta;
}

static long pBTProc[4];
 
LRESULT CALLBACK BTProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ){
	int i;
	for(i=0;i<4;i++)if(hDlg==hBT[i])break;

	switch(message){
	case WM_LBUTTONDOWN:
		_btsta[i]=1;break;
	case WM_LBUTTONUP:
		_btsta[i]=0;break;
	}
	return CallWindowProc((WNDPROC)pBTProc[i],hDlg,message,wParam,lParam);
}

LRESULT CALLBACK Dialog_Main( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ){
	int i;
	switch( message ){
		case WM_INITDIALOG:
				m_hDlg=hDlg;
				//ｽﾗｲﾀﾞｰの初期化
				hSLIDER=GetDlgItem(m_hDlg,IDC_SLIDER0);
				SendMessage(hSLIDER,TBM_SETRANGE,(WPARAM)TRUE,(LPARAM)MAKELONG(0, 255));

				//ﾚﾍﾞﾙﾒｰﾀｰの初期化
				hLEVEL=GetDlgItem(m_hDlg,IDC_LEVEL0);
				SendMessage(hLEVEL,PBM_SETRANGE,(WPARAM)0,(LPARAM)MAKELPARAM(0, 255));
				SendMessage(hLEVEL,PBM_SETPOS,(WPARAM)0,(LPARAM)0);
				
				//ｶｳﾝﾀの初期化
				hCT[0] = GetDlgItem(hDlg,IDC_CT0);
				hCT[1] = GetDlgItem(hDlg,IDC_CT1);
				for(i=0;i<2;i++){
					SendMessage(hCT[i],WM_SETTEXT,(WPARAM)0,(LPARAM)"0");
				}

				//ﾎﾞﾀﾝの初期化
				hBT[0]=GetDlgItem(hDlg,IDC_BT0);
				hBT[1]=GetDlgItem(hDlg,IDC_BT1);
				hBT[2]=GetDlgItem(hDlg,IDC_BT2);
				hBT[3]=GetDlgItem(hDlg,IDC_BT3);
				for(i=0;i<4;i++){
					pBTProc[i]=GetWindowLong(hBT[i],GWL_WNDPROC);
					SetWindowLong(hBT[i],GWL_WNDPROC,(LONG)BTProc);
				}

				//ﾗﾝﾌﾟの初期化
				hLP[0] =GetDlgItem(hDlg,IDC_LP0);
				hLP[1] =GetDlgItem(hDlg,IDC_LP1);
				hLP[2] =GetDlgItem(hDlg,IDC_LP2);
				hLP[3] =GetDlgItem(hDlg,IDC_LP3);

				//情報ﾎﾞｯｸｽの初期化
				hINFO=GetDlgItem(hDlg,IDC_INFO);
				alive(1);
				CommConnect(USEPORT);
				return FALSE;

		case WM_COMMAND:
			if( LOWORD(wParam) == IDCLOSE  ) {
				DestroyWindow(hDlg);
				alive(0);
				return TRUE;
			}
			break;

	}
    return FALSE;
}
static void (*_timer)();
void HardInit(){
	HWND hDlg;
	_timer=NULL;
	hDlg=CreateDialog(NULL, (LPCTSTR)IDD_DIALOG, 0, (DLGPROC)Dialog_Main);
	ShowWindow(hDlg,SW_SHOW);
	UpdateWindow(hDlg);
}


void Wait(int i){
	Sleep((DWORD)i);
}


void SetTimerFunc(void (*t)()){
	_timer=t;
}

void TIMER1ms(){
	if(_timer!=NULL){
		(*_timer)();
	}
}



LPCTSTR ShowFolderDlg()
{
	LPITEMIDLIST pIdl;			// 戻り値
	BROWSEINFO brDirInfo;       // ディレクトリ情報
	char strBuff[MAX_PATH];		// ディレクトリ位置情報バッファー
	char strBuff2[MAX_PATH];	// ディレクトリ名取得用バッファー
	static char retPath[MAX_PATH];
	//ディレクトリ位置バッファー確保
	memset(strBuff, '\0', sizeof(strBuff));
	memset(strBuff2, '\0', sizeof(strBuff));

	brDirInfo.hwndOwner = m_hDlg;
	brDirInfo.ulFlags = BIF_RETURNONLYFSDIRS;	//実在するディレクトリのみ
//	brDirInfo.lpszTitle = "保存先";				//ダイアログボックスタイトル
	brDirInfo.lpszTitle = "書き込みデータのあるフォルダを選択";	//ダイアログボックスタイトル
	brDirInfo.pszDisplayName = strBuff;			//フォルダー名
	brDirInfo.pidlRoot = CSIDL_DESKTOP;			//ルート位置
	brDirInfo.lpfn = 0;							//コールバック関数名
	brDirInfo.lParam = 0;						//コールバック関数の引数

	//フォルダー参照表示
	pIdl = SHBrowseForFolder(&brDirInfo);

	//フォルダー判定
	if (pIdl) {
		//フォルダー指定成功の場合
		SHGetPathFromIDList(pIdl, strBuff2);
		CoTaskMemFree(pIdl);
	}
	else {
		return NULL;
	}

	// 指定フォルダー
	sprintf(retPath, "%s\\", strBuff2);
	return retPath;
}