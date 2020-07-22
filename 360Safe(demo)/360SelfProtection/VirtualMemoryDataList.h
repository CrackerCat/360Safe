#pragma once
#include <ntifs.h>
#include "Data.h"

#define PIDMMNEWNUMBER        0x62					//��������ڴ���Ϣ�ģ�ÿ�����̿��Դ�0x64��new����Ϣ,ʵ��ʹ��0x62��
#define PIDMMNEWNUMBERMAXIMUM 0x64
#define PIDMMNUMBER           0xC6					//��������ڴ���Ϣ�ģ�һ�����Դ�0xC8������,ʵ��ʹ��0xC6��
#define PIDMMNUMBERMAXIMUM    0xC8

//ÿ�����̿��Է���ܶ���ڴ����ݣ��������0x62��
//��С0x4B8
typedef struct _ALLOCATEVIRTUALMEMORYDATA {
	ULONG  ListNumber;								//+0x4						Ŀǰʹ���˼���������
	PVOID  BaseAddress[PIDMMNEWNUMBERMAXIMUM];		//+0X8						�����׵�ַ
	SIZE_T RegionSize[PIDMMNEWNUMBERMAXIMUM];		//+0X198					�����С
	HANDLE ProcessId[PIDMMNEWNUMBERMAXIMUM];		//+0X328					PsGetCurrentProcessId
	HANDLE UniqueProcessId;							//+0X428					pPsGetProcessId
}ALLOCATEVIRTUALMEMORYDATA, *PALLOCATEVIRTUALMEMORYDATA;

//��������ڴ���Ϣ�Ľṹ��     
typedef struct _ALLOCATEVIRTUALMEMORY_DIRECTORY {
	ULONG ListNumber;														// +0   һ���ж�����
	ALLOCATEVIRTUALMEMORYDATA VirtualMmBuff[PIDMMNUMBERMAXIMUM];			// +4   ÿһ���ж��ٴ�
	KSPIN_LOCK	SpinLock;													// ĩβ ������			g_SpinLock_39010
}ALLOCATEVIRTUALMEMORY_DIRECTORY, *PALLOCATEVIRTUALMEMORY_DIRECTORY;
PALLOCATEVIRTUALMEMORY_DIRECTORY g_VirtualMemoryData_List;

/*****************************���*****************************/
//����ڴ���Ϣ
//�ɹ�����1��ʧ�ܷ���0
BOOLEAN Safe_InsertVirtualMemoryDataList(IN PVOID In_BaseAddress, IN SIZE_T In_RegionSize, IN HANDLE In_UniqueProcessId, IN HANDLE In_ProcessId);
/*****************************���*****************************/

/*****************************ɾ��*****************************/
//ɾ���ڴ���Ϣ
PVOID NTAPI Safe_DeleteVirtualMemoryDataList(IN HANDLE In_ProcessId);

//ɾ���ڴ���Ϣ
//WINDOWS_VERSION_XP��Win2K��Ч
PVOID NTAPI Safe_DeleteVirtualMemoryDataList_XP_WIN2K(IN HANDLE In_UniqueProcessId, IN HANDLE In_ProcessId, IN ULONG In_Esp, IN ULONG In_ExpandableStackBottom, IN ULONG In_ExpandableStackSize);
/*****************************ɾ��*****************************/


/*****************************��ѯ*****************************/
//��ѯ�ڴ���Ϣ
//�ɹ�����1��ʧ�ܷ���0
BOOLEAN Safe_QueryVirtualMemoryDataList(IN PVOID In_BaseAddress, IN SIZE_T In_RegionSize, IN HANDLE In_UniqueProcessId, IN HANDLE In_ProcessId);
/*****************************��ѯ*****************************/
