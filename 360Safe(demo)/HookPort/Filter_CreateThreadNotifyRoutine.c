#include "Filter_CreateThreadNotifyRoutine.h"

//NotifyRoutine
NTSTATUS NTAPI Filter_CreateThreadNotifyRoutine(IN HANDLE  ParentId, IN HANDLE  ThreadId, IN BOOLEAN  Create)
{
	NTSTATUS result;

	PULONG FuncTable[16] = { 0 };
	PULONG ArgTable[16] = { 0 };

	ULONG		RetCount;
	PVOID		pArgArray = &ParentId;//�������飬ָ��ջ�����ڱ����������в���

	result = HookPort_DoFilter(CreateThreadNotifyRoutine_FilterIndex, pArgArray, 0, 0, 0, 0);
	return result;
}