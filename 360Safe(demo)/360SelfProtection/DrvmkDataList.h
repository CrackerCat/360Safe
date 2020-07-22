#pragma once
#include <ntifs.h>
#include "Data.h"
#include "WinKernel.h"


//�ڰ���������
#define DRVMKNUMBER                         0x270E				
#define DRVMKNUMBERMAXIMUM				    0x2710

//����PE��ϣֵ���ļ���С������or���ر�־λ
//��С0x18���ֽ�
typedef struct _PE_HASH_DATA
{
	ULONG  Hash[4];									//��ϣֵ
	ULONG  PESize;									//�ļ���С
	ULONG  LoadDriver_Flag;							//���� ����or���б�ʶ   1���� 0����
}PE_HASH_DATA, *PPE_HASH_DATA;


//360�İ���������������ı�							����drvmk.dat�ļ�����
typedef struct _SYS_BLACK_WHITE_DATA
{
	ULONG		 ListNumber;						//��������ʹ�ø���
	PE_HASH_DATA Pe_Hash_Data[DRVMKNUMBERMAXIMUM];	//����ý��̵���Ϣ��������ϣֵ���ļ���С������or���б�ʶ
	KSPIN_LOCK   SpinLock;						    //������
}SYS_BLACK_WHITE_DATA, *PSYS_BLACK_WHITE_DATA;

PSYS_BLACK_WHITE_DATA g_Drvmk_List;

/******************************ɾ��******************************/
//ɾ���ڰ�����
PVOID NTAPI Safe_DeleteDrvmkDataList(IN ULONG In_Hash, IN SIZE_T In_FileSize, IN ULONG Pass_Flag);
/******************************ɾ��******************************/

/******************************���******************************/
//��Ӻڰ�����
PVOID NTAPI Safe_InsertDrvmkDataList(IN ULONG In_Hash, IN SIZE_T In_FileSize, IN ULONG Pass_Flag);
/******************************���******************************/

/******************************��ѯ******************************/
//�ж��Ƿ����
ULONG NTAPI Safe_QueryDrvmkDataList(IN ULONG In_Hash,IN SIZE_T In_FileSize);
/******************************��ѯ******************************/

/*****************************��ʼ��*****************************/
//��ȡTextOutCache��ֵ������ݣ���������һ��·��ָ��xxx\\xxx\\xxx\\drvmk.dat
NTSTATUS NTAPI Safe_InitializeTextOutCacheList(IN PCWSTR In_Data, IN ULONG Type, IN ULONG DataLength,IN ULONG Flag);

//��ʼ�������������غͷ��н�����Ϣ�ģ�R3��R0������
VOID NTAPI Safe_Initialize_List();
/*****************************��ʼ��*****************************/