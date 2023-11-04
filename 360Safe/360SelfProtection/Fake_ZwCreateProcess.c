#include "Fake_ZwCreateProcess.h"

//************************************     
// ��������: After_ZwCreateProcess_Func     
// ����˵����ԭʼ����ִ�к��飬��������·�����ֹ�û��򿪣���������㣬������ô�򿪣�  
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2020/03/31     
// �� �� ֵ: NTSTATUS NTAPI     
// ��    ��: IN ULONG FilterIndex      [In]After_ZwOpenFileIndex���
// ��    ��: IN PVOID ArgArray         [In]ZwOpenFile�������׵�ַ
// ��    ��: IN NTSTATUS Result        [In]����ԭʼZwOpenFile����ֵ
// ��    ��: IN PULONG RetFuncArgArray [In]���ر�ɾ����SafeMon�б������±�
//************************************  
NTSTATUS NTAPI After_ZwCreateProcess_Func(IN ULONG FilterIndex, IN PVOID ArgArray, IN NTSTATUS InResult, IN PULONG RetFuncArgArray)
{

	NTSTATUS       Status, result;
	PEPROCESS pPeprocess = NULL;
	result = STATUS_SUCCESS;
	//0����ȡZwCreateProcessԭʼ����
	PHANDLE  ProcessHandle = *(ULONG*)((ULONG)ArgArray);

	//1���ж��ϴε���ԭʼ��������ֵ
	if (!NT_SUCCESS(InResult))
	{
		return InResult;
	}
	if (myProbeRead(ProcessHandle, sizeof(HANDLE), sizeof(CHAR)))
	{
		KdPrint(("ProbeRead(After_ZwCreateProcess_Func��ProcessHandle) error \r\n"));
		return result;
	}
	if (ObReferenceObjectByHandle(*(ULONG*)ProcessHandle, NULL, PsProcessType, KernelMode, &pPeprocess, 0) >= 0)
	{
		Safe_InsertCreateProcessDataList(pPeprocess, *(ULONG*)RetFuncArgArray);
		ObfDereferenceObject((PVOID)pPeprocess);
		pPeprocess = 0;
	}
	return result;
}

//************************************     
// ��������: After_ZwCreateProcessEx_Func     
// ����˵����ԭʼ����ִ�к��飬��������·�����ֹ�û��򿪣���������㣬������ô�򿪣�  
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2020/03/31     
// �� �� ֵ: NTSTATUS NTAPI     
// ��    ��: IN ULONG FilterIndex      [In]After_ZwOpenFileIndex���
// ��    ��: IN PVOID ArgArray         [In]ZwOpenFile�������׵�ַ
// ��    ��: IN NTSTATUS Result        [In]����ԭʼZwOpenFile����ֵ
// ��    ��: IN PULONG RetFuncArgArray [In]�뷵�صĺ���ָ���Ӧ��һ������,�ڵ���RetFuncArray�е�һ������ʱ��Ҫ�����ڱ������ж�Ӧ�Ĳ���
//************************************  
NTSTATUS NTAPI After_ZwCreateProcessEx_Func(IN ULONG FilterIndex, IN PVOID ArgArray, IN NTSTATUS InResult, IN PULONG RetFuncArgArray)
{
	NTSTATUS  Status, result;
	PROCESS_BASIC_INFORMATION PBI = { 0 };
	ULONG ReturnLength = NULL;
	PEPROCESS pPeprocess = NULL;
	result = STATUS_SUCCESS;
	//0����ȡZwCreateProcessԭʼ����
	PHANDLE  ProcessHandle = *(ULONG*)((ULONG)ArgArray);

	//1���ж��ϴε���ԭʼ��������ֵ
	if (!NT_SUCCESS(InResult))
	{
		return InResult;
	}
	if (myProbeRead(ProcessHandle, sizeof(HANDLE), sizeof(CHAR)))
	{
		KdPrint(("ProbeRead(Address) error \r\n"));
		return result;
	}
	//GetProcessPid
	Status = Safe_ZwQueryInformationProcess(*(ULONG*)ProcessHandle, ProcessBasicInformation, &PBI, sizeof(PROCESS_BASIC_INFORMATION), &ReturnLength);
	if (NT_SUCCESS(Status))
	{
		if (PBI.UniqueProcessId)
		{
			Safe_InsertWhiteList_PID_Win2003(PBI.UniqueProcessId, *(ULONG*)RetFuncArgArray);
		}
	}
	return result;
}

//��������Ex
NTSTATUS NTAPI Fake_ZwCreateProcessEx(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS       Status, Result;
	Result = STATUS_SUCCESS;
	//0����ȡZwCreateProcessԭʼ����
	HANDLE  SectionHandle = *(ULONG*)((ULONG)ArgArray + 0x14);
	//1��������Ӧ�ò����
	if (ExGetPreviousMode())
	{
		ULONG SafeModIndex = Safe_DeleteSafeMonDataList(*(HANDLE*)SectionHandle);
		if (SafeModIndex)
		{
			*(ULONG*)ret_func = After_ZwCreateProcessEx_Func;
			*(ULONG*)ret_arg = SafeModIndex;
		}
	}
	return Result;
}

//��������
NTSTATUS NTAPI Fake_ZwCreateProcess(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS       Status, result;
	result = STATUS_SUCCESS;
	//0����ȡZwCreateProcessԭʼ����
	HANDLE  SectionHandle = *(ULONG*)((ULONG)ArgArray + 0x14);
	//1��������Ӧ�ò����
	if (ExGetPreviousMode())
	{
		ULONG SafeModIndex = Safe_DeleteSafeMonDataList(*(HANDLE*)SectionHandle);
		if (SafeModIndex)
		{
			*(ULONG*)ret_func = After_ZwCreateProcess_Func;
			*(ULONG*)ret_arg = SafeModIndex;
		}
	}
	return result;
}