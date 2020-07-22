#pragma once
#include <ntifs.h>
#include "WinKernel.h"
#include "Data.h"


/************************��ʼ��ϵͳ�����б�*********************/
//Safe_Initialize_Data���������
//��ʼ��ϵͳ���̺���
NTSTATUS Safe_InitializeSystemInformationFile();
/************************��ʼ��ϵͳ�����б�*********************/

/*****************************���*****************************/
//���ܣ�
//����򿪵���ָ����ϵͳ���̣������ļ���ϢУ����ȷ�������ö�Ӧ��PID��Eprocess
//g_dynData->SystemInformationList.xxxx       ��ָ��ϵͳ������Ϣ
BOOLEAN NTAPI Safe_InsertSystemInformationList(IN PEPROCESS Process, IN ULONG Index, IN ULONG Version_Flag);


/*****************************���*****************************/

/*****************************��ѯ*****************************/
//�ж��Ƿ����
BOOLEAN NTAPI Safe_QuerySystemInformationList(IN PEPROCESS Process, IN ULONG Index);
/*****************************��ѯ*****************************/

/*****************************���*****************************/
//�˶�csrss.exe��svchost.exe��dllhost.exe�Ϸ���
BOOLEAN NTAPI Safe_CheckSysProcess();

//���˵�csrss.exe��lsass.exe
BOOLEAN NTAPI Safe_CheckSysProcess_Csrss_Lsass(IN HANDLE In_Handle);


//coherence.exe
BOOLEAN NTAPI Safe_CheckSysProcess_Coherence();
/*****************************���*****************************/