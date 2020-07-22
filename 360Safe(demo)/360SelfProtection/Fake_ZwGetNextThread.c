#include "Fake_ZwGetNextThread.h"

//�����߳�
//ԭ����ִ�к���
//�������������̵߳ľ����ֱ�ӰѾ�����㲢�ҷ��ش���ֵ
NTSTATUS NTAPI After_ZwGetNextThread_Func(IN ULONG FilterIndex, IN PVOID ArgArray, IN NTSTATUS InResult, IN PULONG RetFuncArgArray)
{
	NTSTATUS       result = STATUS_SUCCESS;
	NTSTATUS       Status = STATUS_SUCCESS;
	//0����ȡZwGetNextThreadԭʼ����
	PHANDLE NewThreadHandle = *(ULONG*)((ULONG)ArgArray + 0x14);			//���������
	//1��ԭ��������ִ�� and �����������߳�
	if (NT_SUCCESS(InResult) && Safe_QueryWintePID_ThreadHandle(*(HANDLE*)NewThreadHandle))
	{
		//��������ֱ�Ӿ�����㣬��ֹ����
		Safe_ZwNtClose(*(HANDLE*)NewThreadHandle, g_VersionFlag);
		*(HANDLE*)NewThreadHandle = 0;
		result = STATUS_ACCESS_DENIED;
	}
	else
	{
		result = STATUS_SUCCESS;
	}
	return result;
}

//�����߳�
NTSTATUS NTAPI Fake_ZwGetNextThread(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS       result = STATUS_SUCCESS;
	NTSTATUS       Status = STATUS_SUCCESS;
	ACCESS_MASK    DesiredAccess_Flag =														   //0x520D00B7
		(GENERIC_WRITE | GENERIC_ALL) |                                                        //0x50000000 = GENERIC_WRITE | GENERIC_ALL
		(MAXIMUM_ALLOWED) |                                                                    //0x02000000 = MAXIMUM_ALLOWED
		(WRITE_OWNER | WRITE_DAC | DELETE) |   	                                               //0x000D0000 = WRITE_OWNER | WRITE_DAC | DELETE
		(THREAD_SET_THREAD_TOKEN | THREAD_SET_INFORMATION | THREAD_SET_CONTEXT) |			   //0x000000B0 = THREAD_SET_THREAD_TOKEN | THREAD_SET_INFORMATION | THREAD_SET_CONTEXT															   //0x000000B0 = 
		(THREAD_SUSPEND_RESUME | THREAD_ALERT | THREAD_TERMINATE);							   //0x00000007 = THREAD_SUSPEND_RESUME | THREAD_ALERT | THREAD_TERMINATE
	//0����ȡZwGetNextThreadԭʼ����
	ACCESS_MASK	   In_DesiredAccess = *(ULONG*)((ULONG)ArgArray + 8);
	//1��������Ӧ�ò����
	if (ExGetPreviousMode())
	{
		//����Ȩ�޲�����
		if (In_DesiredAccess & DesiredAccess_Flag)
		{
			//�����߷Ǳ������̣���Ҫ�����ж�
			if (!Safe_QueryWhitePID(PsGetCurrentProcessId()))
			{
				*(ULONG*)ret_func = After_ZwGetNextThread_Func;
			}
		}
	}
	return result;
}