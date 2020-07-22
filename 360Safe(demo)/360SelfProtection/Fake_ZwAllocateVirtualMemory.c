#include "Fake_ZwAllocateVirtualMemory.h"

//************************************     
// ��������: After_ZwAllocateVirtualMemory_Func     
// ����˵����ԭʼ����ִ�к���
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
NTSTATUS NTAPI After_ZwAllocateVirtualMemory_Func(IN ULONG FilterIndex, IN PVOID ArgArray, IN NTSTATUS InResult, IN PULONG RetFuncArgArray)
{
	NTSTATUS       Status, result;
	PROCESS_BASIC_INFORMATION PBI = { 0 };
	ULONG          ReturnLength = NULL;
	result = STATUS_SUCCESS;
	//0����ȡZwAllocateVirtualMemoryԭʼ����
	HANDLE  In_ProcessHandle = *(ULONG*)((ULONG)ArgArray);
	PVOID   In_pBaseAddress = *(ULONG*)((ULONG)ArgArray + 4); //ָ��
	PSIZE_T RegionSize = *(ULONG*)((ULONG)ArgArray + 0xC);    //ָ��
	//1���ж��ϴε���ԭʼ��������ֵ
	if (!NT_SUCCESS(InResult))
	{
		return InResult;
	}
	//2��GetProcessPid
	Status = Safe_ZwQueryInformationProcess(In_ProcessHandle, ProcessBasicInformation, &PBI, sizeof(PROCESS_BASIC_INFORMATION), &ReturnLength);
	if (NT_SUCCESS(Status))
	{
		//�жϲ����Ϸ���
		if (myProbeRead(In_pBaseAddress, sizeof(PVOID), sizeof(CHAR)) && myProbeRead(RegionSize, sizeof(ULONG), sizeof(CHAR)))
		{
			KdPrint(("ProbeRead(After_ZwAllocateVirtualMemory_Func��In_pBaseAddress��RegionSize) error \r\n"));
			return result;
		}
		//������̷�����ڴ���Ϣ
		if (!Safe_InsertVirtualMemoryDataList(*(PVOID*)In_pBaseAddress, *(ULONG*)RegionSize, PBI.UniqueProcessId, PsGetCurrentProcessId()))
		{
			//���ʧ�ܣ����������㣬�����ش���ֵ
			if (g_HighgVersionFlag)
			{
				ZwFreeVirtualMemory(In_ProcessHandle, &In_pBaseAddress, &RegionSize, MEM_RELEASE);
			}
			else
			{
				NtFreeVirtualMemory(In_ProcessHandle, &In_pBaseAddress, &RegionSize, MEM_RELEASE);
			}
			*(PVOID*)In_pBaseAddress = 0;
			*(ULONG*)RegionSize = 0;
			result = STATUS_ACCESS_DENIED;
		}
	}

	return result;
}


//�����ڴ�
NTSTATUS NTAPI Fake_ZwAllocateVirtualMemory(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS       result = STATUS_SUCCESS;
	NTSTATUS       Status = STATUS_SUCCESS;
	BOOLEAN        Flag = FALSE;
	//0����ȡZwAllocateVirtualMemoryԭʼ����
	HANDLE In_ProcessHandle = *(ULONG*)((ULONG)ArgArray);
	//1��������Ӧ�ò����
	if (ExGetPreviousMode())                 
	{
		//�����ֵ!=��ǰ���̵ľ����-1�������⴦��
		if (In_ProcessHandle != NtCurrentProcess())
		{
			//����ǰ���������
			if (!Safe_QueryWhitePID(PsGetCurrentProcessId()))
			{
				//�ж�Ҫ�򿪵��ǲ��Ǳ�������,�Ǽ����жϣ�����ֱ�ӳɹ��˳�
				if (Safe_QueryWintePID_ProcessHandle(In_ProcessHandle))	
				{
					if (!Safe_QuerySystemInformationList(IoGetCurrentProcess(), SYSTEMROOT_SYSTEM32_CSRSS_EXE))
					{
						if (!Safe_QuerySystemInformationList(IoGetCurrentProcess(), SYSTEMROOT_SYSTEM32_DLLHOST_EXE))
						{
							if (!Safe_InsertSystemInformationList(IoGetCurrentProcess(), SYSTEMROOT_SYSTEM32_CSRSS_EXE, g_VersionFlag))
							{
								if (!Safe_InsertSystemInformationList(IoGetCurrentProcess(), SYSTEMROOT_SYSTEM32_DLLHOST_EXE, g_VersionFlag))
								{
									if (g_Win2K_XP_2003_Flag)
									{
										Flag = TRUE;
									}
									//�����̸߳�������1ʱ�򣨸մ���ʱ������㣩
									//ע�뱣�����̵���������Ѿ�GG�ˣ���Ϊ�������Ľ��̵��̸߳��������ܵ���1
									result = Safe_FindEprocessThreadCount(In_ProcessHandle, Flag);
									if (!result)
									{
										result = STATUS_ACCESS_DENIED;
										return result;
									}
									*(ULONG*)ret_func = After_ZwAllocateVirtualMemory_Func;
									*(ULONG*)ret_arg = 0;
								}
							}
						}
					}
				}
			}
		}
	}
	return result;
}