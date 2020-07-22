#include "Filter_CreateProcessNotifyRoutineEx.h"

//NotifyRoutine
NTSTATUS NTAPI Filter_CreateProcessNotifyRoutineEx(PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo)
{
	NTSTATUS result;

	PULONG   FuncTable[16] = { 0 };
	PULONG   ArgTable[16] = { 0 };

	ULONG		RetCount;
	PVOID		pArgArray = &Process;//�������飬ָ��ջ�����ڱ����������в���

	result = HookPort_DoFilter(CreateProcessNotifyRoutineEx_FilterIndex, pArgArray, 0, 0, 0, 0);
	return result;
}