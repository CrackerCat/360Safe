#pragma once
#include <ntifs.h>
#include "Data.h"
#include "WinKernel.h"

//���������̸���
#define WHITELISTNUMBER						0xFE
#define WHITELISTNUMBERMAXIMUM				0x100

//�������������,Ŀǰֻ����360Tray.exe����+1
#define SPECIALWHITELISTSIZE                0xE			
#define SPECIALWHITELISTSIZEMAXIMUM	        0x10


//R3���������Ľ���
typedef struct _SPECIALWHITELIST
{
	ULONG SpecialWhiteListNumber;								 //������������̸���
	ULONG SpecialWhiteListPID[SPECIALWHITELISTSIZEMAXIMUM];		 //�����������PID
	ULONG SpecialWhiteListSessionId[SPECIALWHITELISTSIZEMAXIMUM];//����������Ľ��̵��ն�ID(SessionId): pbi.InheritedFromUniqueProcessId �Ľ����� == "services.exe"�͵���3600FFFF���������SessionId
	KSPIN_LOCK SpinLock;										 //����g_SpecialWhite_List�����������SpinLock��
}SPECIALWHITELIST, *PSPECIALWHITELIST;							 //���������Ŀǰֻ����360Tray.exe����+1,IRPͨѶ���ֺ�������
SPECIALWHITELIST g_SpecialWhite_List;

typedef struct _WHITELIST
{
	ULONG WhiteListNumber;										//���������̸���				    >= 0xFFΪ��Ч
	ULONG WhiteListPID[WHITELISTNUMBERMAXIMUM];					//��������PID						0x100
	ULONG SafeModIndex[WHITELISTNUMBERMAXIMUM];					//����SafeMon,���Ҹ�dos·�����б�ڼ��ret_arg = ���������±�
	KSPIN_LOCK SpinLock;										//������
}WHITELIST,*PWHITELIST;											//�������������PID�����磺zhudongfangyu.exe������xxxx.exe����,IRPͨѶ���ֺ�������
WHITELIST g_White_List;


/*****************************ɾ��*****************************/
//�ж��ǲ��ǰ���������
//1������ǣ���������������Ϣ��������Ĩ��
//2��������ǣ�ֱ���˳�
BOOLEAN Safe_DeleteWhiteList_PID(_In_ HANDLE ProcessId);

//����SessionIdɾ��
BOOLEAN Safe_DeleteWhiteList_SessionId(_In_ HANDLE SessionId);

//����PID��SessionIdɾ��
BOOLEAN Safe_DeleteWhiteList_PID_SessionId(_In_ HANDLE ProcessId);
/*****************************ɾ��*****************************/

/*****************************���*****************************/
//Win2K
// ��Ӱ�����������Ϣ
BOOLEAN  Safe_InsertWhiteList_PID_Win2003(_In_ HANDLE ProcessId, _In_ ULONG SessionId);

// ��Ӱ�����������Ϣ
BOOLEAN  Safe_InsertWhiteList_PID(_In_ HANDLE ProcessId, _In_ ULONG SessionId);
/*****************************���*****************************/

/*****************************��ѯ*****************************/
//�ж��ǲ��ǰ�����_EPROCESS
//����ֵ����1������0
BOOLEAN Safe_QueryWhiteEProcess(_In_ PEPROCESS Process);

//�ж��ǲ��ǰ�������PID
//����ֵ����1������0
BOOLEAN Safe_QueryWhitePID(_In_ HANDLE ProcessId);

//�������ܣ�
//�ж��������������SessionId�Ƿ���ڵ�ǰ���̵�SessionId
//����ֵ��
//����ֵ����1������0
BOOLEAN Safe_QuerySpecialWhiteSessionId();

//����ProcessHandleת����Eprocess��Ȼ�����Safe_QueryWhitePID_PsGetProcessId
//�ж��ǲ��ǰ�������PID
//����ֵ����1������0
BOOLEAN Safe_QueryWintePID_ProcessHandle(IN HANDLE ProcessHandle);

//�����߳̾����ȡPID��Ȼ���ж�PID�Ƿ��Ǳ�������
//�ж��ǲ��ǰ�������PID
//����ֵ����1������0
BOOLEAN Safe_QueryWintePID_ThreadHandle(IN HANDLE ThreadHandle);

//Eprocess_UniqueProcessId
//�ж��ǲ��ǰ�������PID
//����ֵ����1������0
BOOLEAN Safe_QueryWhitePID_PsGetProcessId(IN PEPROCESS pPeprocess);

//����ThreadHandle��ȡ��ǰ����PID
BOOLEAN  Safe_QueryWhitePID_PsGetThreadProcessId(PVOID VirtualAddress);
/*****************************��ѯ*****************************/
