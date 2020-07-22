#include "Fake_ZwCreateThread.h"

//�������ĺ���
//WINDOWS_VERSION_XP��Win2K��Ч
BOOLEAN NTAPI Safe_19AFC(IN HANDLE In_ProcessHandle, IN ULONG In_Eip, IN ULONG In_Eax, IN ULONG In_Esp, IN ULONG In_ExpandableStackBottom, IN ULONG In_ExpandableStackSize)
{

	BOOLEAN		   result = TRUE;
	NTSTATUS	   Status = STATUS_SUCCESS;
	PEPROCESS      pPeprocess = NULL;
	HANDLE		   SectionHandle = NULL;
	HANDLE		   Out_SectionHandle = NULL;
	KAPC_STATE 	   ApcState;
	ULONG          ResultLength = NULL;
	ULONG          UniqueProcessId = NULL;
	PVOID          SectionObject = NULL;
	ULONG          WinXP_EPROCESS_SectionObjectIndex = 0x138;    //+0x138 SectionObject
	ULONG          WinXP_EPROCESS_UniqueProcessIdIndex = 0x84;   //+0x84  void *UniqueProcessId
	ULONG          Win2k_EPROCESS_SectionHandleIndex = 0x1AC;    //+1xac  void *SectionHandle
	ULONG          Win2k_EPROCESS_UniqueProcessIdIndex = 0x9C;   //+0x9c  void *UniqueProcessId
	SECTION_IMAGE_INFORMATION ImageInformation = { 0 };
	Status = ObReferenceObjectByHandle(In_ProcessHandle, NULL, PsProcessType, UserMode, &pPeprocess, 0);
	if (!NT_SUCCESS(Status))
	{
		return result;
	}
	//1�����ݰ汾��ȡ:Win2K
	if (g_VersionFlag == WINDOWS_VERSION_2K)
	{
		//��ȡSection���
		SectionHandle = *(HANDLE*)((ULONG)pPeprocess + Win2k_EPROCESS_SectionHandleIndex);
		//��ȡPID
		UniqueProcessId = *(HANDLE*)((ULONG)pPeprocess + Win2k_EPROCESS_UniqueProcessIdIndex);
		//����
		KeStackAttachProcess(In_ProcessHandle, &ApcState);
		Status = ZwQuerySection(SectionHandle, SectionImageInformation, &ImageInformation, sizeof(SECTION_IMAGE_INFORMATION), &ResultLength);
		//�������
		KeUnstackDetachProcess(&ApcState);
		if (!NT_SUCCESS(Status))
		{
			goto _FunctionRet;
		}
	}
	//1��1 ���ݰ汾��ȡ:WINDOWS_VERSION_XP
	else if (g_VersionFlag == WINDOWS_VERSION_XP)
	{
		SectionObject = *(PVOID*)((ULONG)pPeprocess + WinXP_EPROCESS_SectionObjectIndex);
		//����
		KeStackAttachProcess(In_ProcessHandle, &ApcState);
		Status = ObOpenObjectByPointer(SectionObject, NULL, NULL, 1, MmSectionObjectType, KernelMode, &Out_SectionHandle);
		//�жϺ�������ֵ
		if (!NT_SUCCESS(Status))
		{
			//�������
			KeUnstackDetachProcess(&ApcState);
			goto _FunctionRet;
		}
		Status = ZwQuerySection(Out_SectionHandle, SectionImageInformation, &ImageInformation, sizeof(SECTION_IMAGE_INFORMATION), &ResultLength);
		//�������
		KeUnstackDetachProcess(&ApcState);
		//�жϺ�������ֵ
		if (!NT_SUCCESS(Status))
		{
			goto _FunctionRet;
		}
		//��ȡPID
		UniqueProcessId = *(HANDLE*)((ULONG)pPeprocess + WinXP_EPROCESS_UniqueProcessIdIndex);
	}
	//�����汾ֱ���˳�����
	else
	{
		goto _FunctionRet;
	}
	//2���жϲ���
	if (In_Eax == ImageInformation.TransferAddress && In_Eip == g_Thread_Information.ThreadContext_Eip)
	{
		if (In_ExpandableStackSize >= 0x100000)
		{
			In_ExpandableStackSize = 0x100000;
		}
		//ɾ��ָ���ڴ�����
		Safe_DeleteVirtualMemoryDataList_XP_WIN2K(UniqueProcessId, PsGetCurrentProcessId(), In_Esp, In_ExpandableStackBottom, In_ExpandableStackSize);
	}
_FunctionRet:
	if (Out_SectionHandle)
	{
		ZwClose(Out_SectionHandle);
		Out_SectionHandle = NULL;
	}
	if (pPeprocess)
	{
		ObfDereferenceObject(pPeprocess);
		pPeprocess = NULL;
	}
	return result;
}
//�����߳�
NTSTATUS NTAPI Fake_ZwCreateThread(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS       Status = STATUS_SUCCESS;
	NTSTATUS       result = STATUS_SUCCESS;
	PEPROCESS      pPeprocess = NULL;
	ULONG          ReturnLength = NULL;
	ULONG          ExpandableStackBottom = NULL;
	ULONG          ExpandableStackSize = NULL;		 // ExpandableStackBase - ExpandableStackBottom	
	PROCESS_BASIC_INFORMATION ProcessInformation = { 0 };
	//0����ȡZwCreateThreadԭʼ����
	PHANDLE     In_ThreadHandle = *(ULONG*)((ULONG)ArgArray);
	ACCESS_MASK In_DesiredAccess = *(ULONG*)((ULONG)ArgArray + 4);
	POBJECT_ATTRIBUTES In_ObjectAttributes = *(ULONG*)((ULONG)ArgArray + 8);
	HANDLE       In_ProcessHandle = *(ULONG*)((ULONG)ArgArray + 0xC);
	PCLIENT_ID   In_PCLIENT_ID = *(ULONG*)((ULONG)ArgArray + 0x10);
	PCONTEXT     In_ThreadContext = *(ULONG*)((ULONG)ArgArray + 0x14);
	PUSER_STACK  In_UserStack = *(ULONG*)((ULONG)ArgArray + 0x18);
	//1�������ֵ!=��ǰ���̵ľ����-1�������⴦��
	if (In_ProcessHandle == NtCurrentProcess())
	{
		return result;
	}
	//2����������
	if (Safe_QueryWhitePID(PsGetCurrentProcessId())						//�������̵���ֱ���˳�
		|| !ExGetPreviousMode()											//�û�ģʽ����ֱ���˳�
		|| !Safe_QueryObjectType(In_ProcessHandle, L"Process")			//�����Process�����˳�
		|| ObReferenceObjectByHandle(In_ProcessHandle, 2u, PsProcessType, UserMode, &pPeprocess, 0) < 0)
	{
		return result;
	}
	if (pPeprocess != IoGetCurrentProcess())
	{
		ObfDereferenceObject(pPeprocess);
		return result;
	}
	//���ü���-1
	ObfDereferenceObject(pPeprocess);
	if (!Safe_CheckSysProcess_Csrss_Lsass(In_ProcessHandle) || Safe_FindEprocessThreadCount(In_ProcessHandle, 0))
	{
		//�ж�ҪĿ������ǲ��Ǳ�������
		if (Safe_QueryWintePID_ProcessHandle(In_ProcessHandle))
		{
			//�Ͱ汾
			if (!g_Win2K_XP_2003_Flag || Safe_FindEprocessThreadCount(In_ProcessHandle, 0))
			{
				//�жϲ����Ϸ���
				if (myProbeRead(In_ThreadContext, sizeof(In_ThreadContext), 1) && myProbeRead(In_UserStack, sizeof(USER_STACK), 4))
				{
					KdPrint(("ProbeRead(Fake_ZwCreateThread��In_ThreadContext or In_UserStack) error \r\n"));
					return result;
				}
				ExpandableStackBottom = In_UserStack->ExpandableStackBottom;
				ExpandableStackSize = (ULONG)In_UserStack->ExpandableStackBase - ExpandableStackBottom;
				//��ȡEIP
				if (!g_Thread_Information.ThreadContext_Eip)
				{
					g_Thread_Information.ThreadContext_Eip = In_ThreadContext->Eip;
					return result;
				}
				//WINDOWS_VERSION_XP��Win2K��Ч
				if (Safe_19AFC(In_ProcessHandle, In_ThreadContext->Eip, In_ThreadContext->Eax, In_ThreadContext->Esp, ExpandableStackBottom, ExpandableStackSize))
				{
					return result;
				}
			}
			//֪ͨ�û���R3 ���ػ��Ƿ���
			Safe_18A72_SendR3(PsGetCurrentProcessId(), PsGetCurrentThreadId(), 2);
			result = STATUS_ACCESS_DENIED;
		}
		return result;
	}
	//coherence.exe��ʲô����̣��������� ƽʱû��ע�������׵��ϸ������
	if (Safe_CheckSysProcess_Coherence())
	{
		//��ȡ����PID
		Status = Safe_ZwQueryInformationProcess(In_ProcessHandle, ProcessBasicInformation, &ProcessInformation, sizeof(PROCESS_BASIC_INFORMATION), &ReturnLength);
		if (NT_SUCCESS(Status))
		{
			g_Thread_Information.UniqueProcessId = ProcessInformation.UniqueProcessId;
		}
		return result;
	}
	result = STATUS_ACCESS_DENIED;
	return result;
}