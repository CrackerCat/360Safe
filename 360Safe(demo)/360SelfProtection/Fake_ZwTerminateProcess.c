#include "Fake_ZwTerminateProcess.h"

//��������
NTSTATUS NTAPI Fake_ZwTerminateProcess(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS       result = STATUS_SUCCESS;
	NTSTATUS       Status = STATUS_SUCCESS;
	ACCESS_MASK    Out_GrantedAccess = NULL;
	ULONG          ReturnLength = NULL;
	PEPROCESS      pPeprocess = NULL;
	BOOLEAN        ObfDereferenceObjectFlag = FALSE;		//�����ObfDereferenceObject  ������Ҫ
	PROCESS_BASIC_INFORMATION ProcessInfo = { 0 };
	//0����ȡZwTerminateProcessԭʼ����
	HANDLE   In_ProcessHandle = *(ULONG*)((ULONG)ArgArray);
	NTSTATUS In_ExitStatus = *(ULONG*)((ULONG)ArgArray + 4);
	//��һ��R3����ZwTerminateProcess�����������̵ķ�����ʩ
	//1��������Ӧ�ò����
	if (ExGetPreviousMode())
	{
		//2����ȡҪ�������̵ľ��Ȩ��
		Status = Safe_GetGrantedAccess(In_ProcessHandle, &Out_GrantedAccess);
		//3�����������̲�����PROCESS_TERMINATEȨ��
		if (NT_SUCCESS(Status) && !(Out_GrantedAccess & PROCESS_TERMINATE))
		{
			//3��1 ����Ŀ�����ǣ���������     ��Ҫ��һ���жϣ��Ǳ�����������
			if (Safe_QueryWintePID_ProcessHandle(In_ProcessHandle))
			{
				//3��2 �жϵ������ǲ��Ǳ������̣�
				//�Ǳ������̵�������
				//�������̵��÷���
				if (!Safe_QueryWhitePID(PsGetCurrentProcessId()))
				{
					ULONG uGetCurrentProcessId = PsGetCurrentProcessId();
					ULONG uUniqueProcessId = Safe_GetUniqueProcessId(In_ProcessHandle);
					//3��3 �ж��Ƿ���Dos����������̣�����ǻ�ȡ������PID
					//taskkill /f /im PID ���������ɱ���������
					if (Safe_CmpImageFileName("taskkill.exe"))
					{
						//3��4 ��ȡ������Ϣ
						Status = ZwQueryInformationProcess(NtCurrentProcess(), ProcessBasicInformation, (PVOID)&ProcessInfo, sizeof(PROCESS_BASIC_INFORMATION), &ReturnLength);
						if (NT_SUCCESS(Status))
						{
							//��ȡ�����̵�PID
							uGetCurrentProcessId = ProcessInfo.InheritedFromUniqueProcessId;
						}
					}
					//�������ػ��Ƿ���
					//sub_188E8(uGetCurrentProcessId, PsGetCurrentThreadId(),uUniqueProcessId);
					result = STATUS_ACCESS_DENIED;
					return result;
				}
			}
		}
	}
	//�ڶ�����csrss.exe���̽����������̷�����ʩ���߰汾
	if (!Safe_QueryWhitePID(PsGetCurrentProcessId()) &&	   //�����߷Ǳ�������
		Safe_CmpImageFileName("csrss.exe") &&			   //��������csrss.exe����
		g_Win2K_XP_2003_Flag			   &&			   //�߰汾(��Win2K��Xp��2003)
		!In_ExitStatus					   &&			   //����������壿����������
		Safe_QueryWintePID_ProcessHandle(In_ProcessHandle) //������Ŀ���Ǳ�������
		)
	{
		result = STATUS_ACCESS_DENIED;
		return result;
	}
	//�������������̵�
	if ((In_ProcessHandle != NtCurrentProcess()) ||			//������
		(KeGetCurrentIrql() == APC_LEVEL) ||				//IRQL�жϵȼ�
		(!Safe_QueryWhitePID_PsGetThreadProcessId(KeGetCurrentThread()))	//�����߷Ǳ�������
		)
	{
		//4���ж�In_ProcessHandle ==0 and In_ProcessHandle != ��ǰ����
		if (In_ProcessHandle && In_ProcessHandle != NtCurrentProcess())
		{
			//4��1 �����Process�����˳�
			if (!Safe_QueryObjectType(In_ProcessHandle, L"Process"))
			{
				result = STATUS_SUCCESS;
				return result;
			}
			//4��2 ��ȡEprocess�ṹ
			//ע��ObReferenceObjectByHandle��ȡ��Eprocess�ṹ��Ҫ������
			Status = ObReferenceObjectByHandle(In_ProcessHandle, NULL, PsProcessType, ExGetPreviousMode(), &pPeprocess, 0);
			if (!NT_SUCCESS(Status))
			{
				//��ȡʧ��ֱ���˳�
				result = STATUS_SUCCESS;
				return result;
			}
			else
			{
				//��ʾObReferenceObjectByHandle�������óɹ���������Ҫ�ͷ�
				ObfDereferenceObjectFlag = TRUE;
			}
		}
		else
		{
			//In_ProcessHandle == 0��ʾ�����̳���ǰ�߳�����̶߳���ɱ��
			//ע��IoGetCurrentProcess��ʽ��ȡEprocess�ṹ����Ҫ������
			pPeprocess = IoGetCurrentProcess();
			ObfDereferenceObjectFlag = FALSE;
		}
		//5��ǰ���һ�������Ѿ���������������ڵ������е��ɻ�ϣ���д����ܽ�����
		if (Safe_QueryWhitePID_PsGetProcessId(pPeprocess) &&			//��������
			(KeGetCurrentIrql() == APC_LEVEL ||							//IRQL�жϵȼ�
			!ExGetPreviousMode())										//R0����
			)
		{
			//ʧ�ܷ���
			result = STATUS_ACCESS_DENIED;
		}
		else
		{
			//�ɹ�����
			result = STATUS_SUCCESS;
		}
		//6��ObfDereferenceObjectFlagΪ�����Ҫ�ͷţ�����ObReferenceObjectByHandle or IoGetCurrentProcess��ʽ��ȡ��Eprocess�ṹ
		if (ObfDereferenceObjectFlag)
		{
			ObfDereferenceObject(pPeprocess);
		}
	}
	return result;
}