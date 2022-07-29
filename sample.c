#include "stdio.h"
#include "stdlib.h"
#include "goplt_if.h"
#if defined(WIN32)
#include "time.h""
#endif

//�G���[���O�ۑ��p�\����
struct errlog {
#if defined(WIN32)
    time_t tm;      //����
#endif
    BOOL is_check;  //�`�F�b�N�ς݃t���O
    int no;         //���@�ԍ�
    int errcode;    //�G���[�R�[�h
} logdata[100];


int cuidx = 0;      //�G���[�������݈ʒu

//�G���[�R�[�h�@���b�Z�[�W�@�Ή�
char* errmes[] = {
    "���J",
    "�T�[�{�ُ�",
    "��]���s��",
    "���b�N�s��",
    "����~",
    "���[�N�����o",
    "�N�����v�s��",
};
//�G���[�̒ǉ�
void add_item(int errcode,int gouki_no)
{
    if (cuidx >= 100)return;        //���O��100���������牽�����Ȃ�(�f���Ȃ̂�)
#if defined(WIN32)
    logdata[cuidx].tm = time(NULL);     //���ݎ���
#endif
    logdata[cuidx].errcode = errcode;   //�G���[�R�[�h
    logdata[cuidx].no = gouki_no;       //���@
    cuidx++;                        //�������݈ʒu��i�߂�
}

//�]���p���O�e�L�X�g�t�@�C���̍쐬
void makelog(char* fname) {
#if defined(WIN32)
    FILE* pf;
    pf = fopen(fname, "w");
    int i;
    for (i = 0; i < cuidx; i++) {
        struct tm* pt;
        pt = localtime(&logdata[i].tm);
        fprintf(pf, "\\{%d:%s\\[R%s\\[L25:%4d/% 2d/% 2d %02d:%02d:%02d\\} %d���@ %s\n"
            ,i+1                                //�^�b�`�u���G���A��id���s�ԍ���
            , i % 2 ? "\\[b7fff" : "\\[b8b7f"   //�����s�A��s�Ŕw�i�F�ύX
            , logdata[i].is_check?"":"\\[Bi0:"      //�`�F�b�N����ĂȂ���΃A�C�R���r�b�g�}�b�v�\��
            , pt->tm_year + 1900, pt->tm_mon, pt->tm_mday, pt->tm_hour, pt->tm_min, pt->tm_sec  //����
            ,logdata[i].no                          //���@�ԍ�
            ,errmes[logdata[i].errcode]             //�G���[�R�[�h�ɑΉ����郁�b�Z�[�W
        );
    }
    fprintf(pf, "\\[S");
    fclose(pf);
#endif
}

#define RRAND(a) ((rand() * (a)) / RAND_MAX)    //0�`a�͈̗̔͂�������


int block = 0;      //������ram�o�b�t�@�[�u���b�N�Ǘ��p

//���O�̒ǉ����\���X�V
void fnADDLOG(void)
{
    add_item(RRAND(7), RRAND(9));       //�f���̂��ߗ����ŃG���[�ԍ��ƍ��@�ԍ�����
    makelog("list.txt");                //RAM�o�b�t�@�[�ɓ]������e�L�X�g�t�@�C���̐���

    RamUpload("list.txt", block, 0);    //RAM�o�b�t�@�[�ɓ]��
//    SpiRamSend("list.txt", block, 0);   //SPI�]�����͂�����

    LtMemWrite("LOGLIST.BlockNo", block); //���O���X�g�̍ĕ`��g���K�[
    block = (block) ? 0 : 1000;         //RAM�o�b�t�@�[�������݈ʒu���s���|��
}

//GOP����LOGLIST.TouchID�̒l�������Ă����Ƃ��̏���
void fnListTouch(void *param)
{
    int i = atoi((char*)param)-1;       //GOP���瑗���Ă���id���s�ԍ���
    logdata[i].is_check = 1;                //�Y���s�̊m�F�ς݃t���O���Z�b�g
    makelog("list.txt");                //�]������e�L�X�g�t�@�C���̍쐬

    RamUpload("list.txt", block, 0);    //RAM�o�b�t�@�[�ɓ]��
//    SpiRamSend("list.txt", block, 0);   //SPI�]�����͂�����

    LtMemWrite("LOGLIST.BlockNo", block);//���O���X�g�̍ĕ`��g���K�[
    block = (block) ? 0 : 1000;         //RAM�o�b�t�@�[�������݈ʒu���s���|��
}
ltMem_Callback memtable[] = {
    {"LOGLIST.TouchID",fnListTouch},
    //�e�[�u���̍ŏI���������߉��s���K�v
    {NULL,NULL}
};
//�T���v���̃��C�����[�`��
void sample_main(void)
{
    int btnst = 0;
    //���b�Z�[�W�n���h���e�[�u���̓o�^
    LtSetMemoryCallback(memtable);
    //GOP-LT�����Z�b�g
    ResetOut(0);
    ResetOut(1);
    //�{�[���[�g�Z�b�g
    PortSetRate(115200);
    //GOP_READY(�ʐM������)���Z�b�g�����܂ő҂�
    while(!Is_Gop_Ready());
    //���C�����[�v
    while (1U)
    {
        //�{�A�v���_�C�A���O�̃{�^�����������΃��O�̒ǉ�(�`���^�����O�h�~�̂��߉����ė������Ƃ��ɒǉ�)
        if (!btnst&&GetBtn()) {
            btnst = 1;
        }
        if (btnst && !GetBtn()) {
            fnADDLOG();       //���O�̒ǉ����\���X�V
            btnst = 0;
        }
        //GOP-LT����̃��b�Z�[�W�L���m�F
        if(Is_SetData())
        {
            //���b�Z�[�W�̎�M�ƃn���h���̏���
            LtEnq(NULL);
        }
    }
}
