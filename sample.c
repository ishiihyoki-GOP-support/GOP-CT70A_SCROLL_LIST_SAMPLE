#include "stdio.h"
#include "stdlib.h"
#include "goplt_if.h"
#if defined(WIN32)
#include "time.h""
#endif

//エラーログ保存用構造体
struct errlog {
#if defined(WIN32)
    time_t tm;      //時刻
#endif
    BOOL is_check;  //チェック済みフラグ
    int no;         //号機番号
    int errcode;    //エラーコード
} logdata[100];


int cuidx = 0;      //エラー書き込み位置

//エラーコード　メッセージ　対応
char* errmes[] = {
    "扉開",
    "サーボ異常",
    "回転数不足",
    "ロック不良",
    "非常停止",
    "ワーク未検出",
    "クランプ不良",
};
//エラーの追加
void add_item(int errcode,int gouki_no)
{
    if (cuidx >= 100)return;        //ログが100件超えたら何もしない(デモなので)
#if defined(WIN32)
    logdata[cuidx].tm = time(NULL);     //現在時刻
#endif
    logdata[cuidx].errcode = errcode;   //エラーコード
    logdata[cuidx].no = gouki_no;       //号機
    cuidx++;                        //書き込み位置を進める
}

//転送用ログテキストファイルの作成
void makelog(char* fname) {
#if defined(WIN32)
    FILE* pf;
    pf = fopen(fname, "w");
    int i;
    for (i = 0; i < cuidx; i++) {
        struct tm* pt;
        pt = localtime(&logdata[i].tm);
        fprintf(pf, "\\{%d:%s\\[R%s\\[L25:%4d/% 2d/% 2d %02d:%02d:%02d\\} %d号機 %s\n"
            ,i+1                                //タッチブルエリアのidを行番号に
            , i % 2 ? "\\[b7fff" : "\\[b8b7f"   //偶数行、奇数行で背景色変更
            , logdata[i].is_check?"":"\\[Bi0:"      //チェックされてなければアイコンビットマップ表示
            , pt->tm_year + 1900, pt->tm_mon, pt->tm_mday, pt->tm_hour, pt->tm_min, pt->tm_sec  //時刻
            ,logdata[i].no                          //号機番号
            ,errmes[logdata[i].errcode]             //エラーコードに対応するメッセージ
        );
    }
    fprintf(pf, "\\[S");
    fclose(pf);
#endif
}

#define RRAND(a) ((rand() * (a)) / RAND_MAX)    //0～aの範囲の乱数生成


int block = 0;      //書込みramバッファーブロック管理用

//ログの追加＆表示更新
void fnADDLOG(void)
{
    add_item(RRAND(7), RRAND(9));       //デモのため乱数でエラー番号と号機番号生成
    makelog("list.txt");                //RAMバッファーに転送するテキストファイルの生成

    RamUpload("list.txt", block, 0);    //RAMバッファーに転送
//    SpiRamSend("list.txt", block, 0);   //SPI転送時はこちら

    LtMemWrite("LOGLIST.BlockNo", block); //ログリストの再描画トリガー
    block = (block) ? 0 : 1000;         //RAMバッファー書き込み位置をピンポン
}

//GOPからLOGLIST.TouchIDの値が送られてきたときの処理
void fnListTouch(void *param)
{
    int i = atoi((char*)param)-1;       //GOPから送られてきたidを行番号に
    logdata[i].is_check = 1;                //該当行の確認済みフラグをセット
    makelog("list.txt");                //転送するテキストファイルの作成

    RamUpload("list.txt", block, 0);    //RAMバッファーに転送
//    SpiRamSend("list.txt", block, 0);   //SPI転送時はこちら

    LtMemWrite("LOGLIST.BlockNo", block);//ログリストの再描画トリガー
    block = (block) ? 0 : 1000;         //RAMバッファー書き込み位置をピンポン
}
ltMem_Callback memtable[] = {
    {"LOGLIST.TouchID",fnListTouch},
    //テーブルの最終を示すため下行が必要
    {NULL,NULL}
};
//サンプルのメインルーチン
void sample_main(void)
{
    int btnst = 0;
    //メッセージハンドラテーブルの登録
    LtSetMemoryCallback(memtable);
    //GOP-LTをリセット
    ResetOut(0);
    ResetOut(1);
    //ボーレートセット
    PortSetRate(115200);
    //GOP_READY(通信準備可)がセットされるまで待つ
    while(!Is_Gop_Ready());
    //メインループ
    while (1U)
    {
        //本アプリダイアログのボタンが押されればログの追加(チャタリング防止のため押して離したときに追加)
        if (!btnst&&GetBtn()) {
            btnst = 1;
        }
        if (btnst && !GetBtn()) {
            fnADDLOG();       //ログの追加＆表示更新
            btnst = 0;
        }
        //GOP-LTからのメッセージ有無確認
        if(Is_SetData())
        {
            //メッセージの受信とハンドラの処理
            LtEnq(NULL);
        }
    }
}
