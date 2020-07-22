#include "Fake_ZwWriteVirtualMemory.h"

//�޸ı������̵�ַ or �޸�PEB���� ��������������������������
BOOLEAN NTAPI Safe_CheckWriteMemory_PEB(IN HANDLE In_Handle, IN ULONG In_BaseAddress, SIZE_T In_BufferLength)
{
	BOOLEAN        result = FALSE;
	NTSTATUS       Status = STATUS_SUCCESS;
	ULONG          ReturnLength = NULL;
	ULONG          Peb_Offset = NULL;
	ULONG          Peb_ProcessParameters_Offset = 0x10;			//Peb->ProcessParameters(��������������Ҫ��Ϣ���������ؽ���֮���)
	ULONG          Peb_pShimData_Offset = 0x1E8;				//Peb->pShimData(����Shim Engine��Dll�ٳ�)
	ULONG          Peb_pContextData_Offset = 0x238;				//Peb->ProcessParameters(֪ʶä���������֪���Ĵ��и�����)
	PROCESS_BASIC_INFORMATION ProcessInformation = { 0 };
	//1����ȡPEB��Ϣ
	Status = Safe_ZwQueryInformationProcess(In_Handle, ProcessBasicInformation, &ProcessInformation, sizeof(PROCESS_BASIC_INFORMATION), &ReturnLength);
	if (NT_SUCCESS(Status))
	{
		if (In_BaseAddress + In_BufferLength >= In_BaseAddress)
		{
			if (Safe_QueryVirtualMemoryDataList(In_BaseAddress, In_BufferLength, ProcessInformation.UniqueProcessId, PsGetCurrentProcessId())// �ж�д�Ŀռ��Ƿ����б�ռ�֮��
				|| In_BaseAddress > ProcessInformation.PebBaseAddress								//����PebBase����Peb��ַ���е㲻�Ͻ���������Ӱ�죩
				&& ((Peb_Offset = In_BaseAddress - (ULONG)(ProcessInformation.PebBaseAddress),		
				   Peb_Offset == Peb_ProcessParameters_Offset)										// Peb->ProcessParameters
				|| Peb_Offset == Peb_pShimData_Offset												// Peb->pShimData
				|| Peb_Offset == Peb_pContextData_Offset)											// Peb->pContextData 
				&& In_BufferLength == sizeof(ULONG))												// 4���ֽ�
			{
				result = TRUE;
			}
		}
	}
	else
	{
		result = TRUE;
	}
	return result;
}

NTSTATUS NTAPI Fake_ZwWriteVirtualMemory(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS       result = STATUS_SUCCESS;
	NTSTATUS       Status = STATUS_SUCCESS;
	//0����ȡZwWriteVirtualMemoryԭʼ����
	HANDLE  In_ProcessHandle = *(ULONG*)((ULONG)ArgArray);
	PVOID   In_BaseAddress =*(ULONG*)((ULONG)ArgArray+4);
	PVOID   In_Buffer =*(ULONG*)((ULONG)ArgArray+8);
	ULONG   In_BufferLength = *(ULONG*)((ULONG)ArgArray+0xC);
	//1��������Ӧ�ò����
	if (ExGetPreviousMode())
	{
		//�����ֵ!=��ǰ���̵ľ����-1�������⴦��
		if (In_ProcessHandle != NtCurrentProcess())
		{
			//����ǰ���������
			if (!Safe_QueryWhitePID(PsGetCurrentProcessId()))
			{
				// ����ָ����Object����
				if (Safe_QueryObjectType(In_ProcessHandle, L"Process"))
				{
					if (!Safe_QueryWintePID_ProcessHandle(In_ProcessHandle)
						|| Safe_QuerySystemInformationList(IoGetCurrentProcess(), SYSTEMROOT_SYSTEM32_CSRSS_EXE,g_VersionFlag)
						|| Safe_QuerySystemInformationList(IoGetCurrentProcess(), SYSTEMROOT_SYSTEM32_DLLHOST_EXE, g_VersionFlag)
						|| Safe_InsertSystemInformationList(IoGetCurrentProcess(), SYSTEMROOT_SYSTEM32_CSRSS_EXE, g_VersionFlag)
						|| Safe_InsertSystemInformationList(IoGetCurrentProcess(), SYSTEMROOT_SYSTEM32_DLLHOST_EXE, g_VersionFlag))
					{
						//�Ǳ�������������
						if (Safe_CheckSysProcess_Csrss_Lsass(In_ProcessHandle) &&		//���˵�������Ϊ��csrss.exe��lsass.exe
							!Safe_FindEprocessThreadCount(In_ProcessHandle, 0))			//�������̸߳���
						{
							//������Ϊcoherence.exe����1������0
							return Safe_CheckSysProcess_Coherence() != FALSE ? STATUS_SUCCESS : STATUS_ACCESS_DENIED;
						}
					}
					//ִ�е����ﶼ���ܱ�������������
					//�޸ı��������ڴ��ַ or �޸�PEB���� ��������������������������
					else if (!Safe_CheckWriteMemory_PEB(In_ProcessHandle, In_BaseAddress, In_BufferLength))
					{
						//�������ػ��Ƿ���
						Safe_18A72_SendR3(PsGetCurrentProcessId(), PsGetCurrentProcessId(), 2);
						return STATUS_CALLBACK_BYPASS;
					}
				}
			}
		}
	}

	return result;
}