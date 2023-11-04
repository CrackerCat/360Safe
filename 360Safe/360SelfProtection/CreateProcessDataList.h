#pragma once
#include <ntifs.h>
#include "Data.h"

//���̸���
#define CREATEPROCESSNUMBER					0x4FE
#define CREATEPROCESSNUMBERMAXIMUM			0x500

typedef struct _CREATEPROCESSDATALIST
{
	ULONG      CreateProcessListNumber;							//����>= 0x4FEΪ��Ч
	PVOID	   Eprocess[CREATEPROCESSNUMBERMAXIMUM];			//Eprocess�ṹ
	PVOID	   ArrayIndex[CREATEPROCESSNUMBERMAXIMUM];			//SafeMon���Ҹ�dos·�����б�ڼ��ret_arg = ���������±�	
	KSPIN_LOCK SpinLock;										//������
}CREATEPROCESSDATALIST, *PCREATEPROCESSDATALIST;				//Fake_CreateProcess������������
CREATEPROCESSDATALIST g_CreateProcessData_List;

/*****************************���*****************************/
//��������  ��������+1��
//����ֵ���ɹ�����TRUE������<=0x4FE����ʧ��FALSE������>=0x4FE��
BOOLEAN Safe_InsertCreateProcessDataList(IN PEPROCESS Process, IN ULONG SafeModArrayIndex);
/*****************************���*****************************/

/*****************************ɾ��*****************************/
//�ж��ǲ�����������Eprocess
//1������ǣ�������������Ϣ��������Ĩ��
//2��������ǣ�ֱ���˳�
ULONG Safe_DeleteCreateProcessDataList(_In_ PEPROCESS ProcessId);
/*****************************ɾ��*****************************/