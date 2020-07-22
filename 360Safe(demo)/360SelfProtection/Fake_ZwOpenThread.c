#include "Fake_ZwOpenThread.h"

//���߳�
//ԭ����ִ�к���
//���򿪵������̵߳ľ����ֱ�ӰѾ�����㲢�ҷ��ش���ֵ
NTSTATUS NTAPI After_ZwOpenThread_Func(IN ULONG FilterIndex, IN PVOID ArgArray, IN NTSTATUS InResult, IN PULONG RetFuncArgArray)
{
	NTSTATUS       result = STATUS_SUCCESS;
	NTSTATUS       Status = STATUS_SUCCESS;
	//0����ȡZwOpenThreadԭʼ����
	PHANDLE ThreadHandle = *(ULONG*)((ULONG)ArgArray);			//���������
	//1���ж��ϴε���ԭʼ��������ֵ
	if (!NT_SUCCESS(InResult))
	{
		return InResult;
	}
	//����ַ�Ϸ���
	if (myProbeRead(ThreadHandle, sizeof(HANDLE), sizeof(CHAR)))
	{
		KdPrint(("ProbeRead(After_ZwOpenThread_Func��ThreadHandle) error \r\n"));
		return result;
	}
	//��ֹ���ܱ����߳�
	if (Safe_QueryWintePID_ThreadHandle(*(HANDLE*)ThreadHandle))
	{
		//�����߳�ֱ�Ӿ�����㣬��ֹ����
		Safe_ZwNtClose(*(HANDLE*)ThreadHandle, g_VersionFlag);
		*(HANDLE*)ThreadHandle = 0;
		result = STATUS_ACCESS_DENIED;
	}
	return result;
}

//���߳�
NTSTATUS NTAPI Fake_ZwOpenThread(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS       result = STATUS_SUCCESS;
	NTSTATUS       Status = STATUS_SUCCESS;
	ACCESS_MASK    DesiredAccess_Flag =														   //0x520D00B7
		(GENERIC_WRITE | GENERIC_ALL) |                                                        //0x50000000 = GENERIC_WRITE | GENERIC_ALL
		(MAXIMUM_ALLOWED) |                                                                    //0x02000000 = MAXIMUM_ALLOWED
		(WRITE_OWNER | WRITE_DAC | DELETE) |   	                                               //0x000D0000 = WRITE_OWNER | WRITE_DAC | DELETE
		(THREAD_SET_THREAD_TOKEN | THREAD_SET_INFORMATION | THREAD_SET_CONTEXT) |			   //0x000000B0 = THREAD_SET_THREAD_TOKEN | THREAD_SET_INFORMATION | THREAD_SET_CONTEXT															   //0x000000B0 = 
		(THREAD_SUSPEND_RESUME | THREAD_ALERT | THREAD_TERMINATE);							   //0x00000007 = THREAD_SUSPEND_RESUME | THREAD_ALERT | THREAD_TERMINATE

	//0����ȡZwOpenThreadԭʼ����
	ACCESS_MASK	   In_DesiredAccess = *(ULONG*)((ULONG)ArgArray + 4);
	//1��������Ӧ�ò����
	if (ExGetPreviousMode())
	{
		//����Ȩ�޲�����
		if (In_DesiredAccess & DesiredAccess_Flag)
		{
			//�����߷Ǳ������̣���Ҫ�����ж�
			if (!Safe_QueryWhitePID(PsGetCurrentProcessId()))
			{
				if (!g_Win2K_XP_2003_Flag || (!Safe_InsertSystemInformationList(IoGetCurrentProcess(), SYSTEMROOT_SYSTEM32_CSRSS_EXE, g_VersionFlag)))
				{
					*(ULONG*)ret_func = After_ZwOpenThread_Func;
				}
			}
		}
	}
	return result;
}