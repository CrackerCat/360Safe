#include "Fake_CreateProcessNotifyRoutine.h"

NTSTATUS NTAPI Fake_CreateProcessNotifyRoutine(ULONG CallIndex, PVOID ArgArray, PULONG ret_func, PULONG ret_arg)
{
	PEPROCESS	Process;
	NTSTATUS	status;
	HANDLE      ProcessHandle;
	CLIENT_ID   ClientId;
	ULONG       SafeModIndex = NULL;
	OBJECT_ATTRIBUTES ObjectAttributes;
	//PROCESS_SESSION_INFORMATION SessionInfo;
	UCHAR ImageFileNameBuff[0x256] = { 0 };
	ProcessHandle = NULL;
	IN HANDLE In_ParentId = *(ULONG*)((ULONG)ArgArray);
	IN HANDLE In_ProcessId = *(ULONG*)((ULONG)ArgArray + 4);
	IN BOOLEAN In_Create = *(ULONG*)((ULONG)ArgArray + 8);
	//��CreateΪTrueʱ���������´����Ľ��̣�ProcessId���ָ�����ĳ�ʼ���̱߳������󱻵��á�
	if (In_Create)
	{
		status = PsLookupProcessByProcessId(In_ProcessId, &Process);
		if (NT_SUCCESS(status))
		{
			//Win2K
			if (g_VersionFlag == WINDOWS_VERSION_2K)
			{
				SafeModIndex = Safe_DeleteCreateProcessDataList(Process);
				if (SafeModIndex)
				{
					Safe_InsertWhiteList_PID_Win2003(In_ProcessId, SafeModIndex);
				}
			}
			Safe_PsGetProcessImageFileName(Process, &ImageFileNameBuff, sizeof(ImageFileNameBuff));
			ObfDereferenceObject(Process);
			if (!_stricmp(&ImageFileNameBuff, "userinit.exe"))
			{
				//���ÿ���
				g_dynData->SystemInformation.Userinit_Flag = 1;
			}
			if (!g_dynData->SystemInformation.Explorer_Flag && !_stricmp(&ImageFileNameBuff, "explorer.exe"))
			{
				//���ÿ����뱣��explorer.exePID
				g_dynData->SystemInformation.Explorer_Flag = 1;
				g_dynData->SystemInformation.Explorer_ProcessId = In_ProcessId;
				ClientId.UniqueProcess = In_ProcessId;
				ClientId.UniqueThread = NULL;
				InitializeObjectAttributes(&ObjectAttributes, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
				if (!NT_SUCCESS(ZwOpenProcess(&ProcessHandle, 0x400u, &ObjectAttributes, &ClientId)))
				{
					ZwQueryInformationProcess(ProcessHandle, ProcessSessionInformation, &g_dynData->SystemInformation.Explorer_SessionId, sizeof(PROCESS_SESSION_INFORMATION), 0);
					ZwClose(ProcessHandle);
				}
			}
			if (!g_dynData->SystemInformation.Winlogon_ProcessId && !_stricmp(&ImageFileNameBuff, "winlogon.exe"))
			{
				g_dynData->SystemInformation.Winlogon_ProcessId = In_ProcessId;
			}
			if (g_Win2K_XP_2003_Flag && !g_dynData->SystemInformation.Wininit_ProcessId && !_stricmp(&ImageFileNameBuff, "wininit.exe"))
			{
				g_dynData->SystemInformation.Wininit_ProcessId = In_ProcessId;
			}
		}
	}
	//��CreateΪFalseʱ�������ڽ��̵����һ���̱߳��رգ����̵ĵ�ַ�ռ佫���ͷ�ʱ���á� 
	else
	{
		if (g_dynData->SystemInformation.Explorer_ProcessId && g_dynData->SystemInformation.Explorer_ProcessId == In_ProcessId)
		{
			g_dynData->SystemInformation.Explorer_ProcessId = 0;
		}
		//�ж��ǲ��ǰ���������
		//1������ǣ���������������Ϣ��������Ĩ��
		//2��������ǣ�ֱ���˳�
		Safe_DeleteWhiteList_PID(In_ProcessId);
		//ɾ���ڴ���Ϣ
		Safe_DeleteVirtualMemoryDataList(In_ProcessId);
		//���������̣�δ֪��
		if (In_ProcessId == g_dynData->dword_34EA0[9])
		{
			g_dynData->dword_34EA0[9] = 0;
		}
		if (In_ProcessId == g_dynData->dword_34EA0[10])
		{
			g_dynData->dword_34EA0[10] = 0;
		}
		if (In_ProcessId == g_dynData->dword_34DAC[11])
		{
			g_dynData->dword_34DAC[11] = 0;
		}
		if (In_ProcessId == g_dynData->dword_34DAC[12])
		{
			g_dynData->dword_34DAC[12] = 0;
		}
		if (In_ProcessId == g_dynData->dword_34DAC[0])
		{
			g_dynData->dword_34DAC[0] = 0;
		}
		if (In_ProcessId == g_dynData->dword_34DAC[1])
		{
			g_dynData->dword_34DAC[1] = 0;
		}
		if (In_ProcessId == g_dynData->dword_34DAC[4])
		{
			g_dynData->dword_34DAC[4] = 0;
		}
		if (In_ProcessId == g_dynData->dword_34DAC[5])
		{
			g_dynData->dword_34DAC[5] = 0;
		}
		if (In_ProcessId == g_dynData->dword_34DAC[7])
		{
			g_dynData->dword_34DAC[7] = 0;
		}
		if (In_ProcessId == g_dynData->dword_34DAC[6])
		{
			g_dynData->dword_34DAC[6] = 0;
		}
		if (g_dynData->dword_34D64 == In_ProcessId)
		{
			g_dynData->dword_34D64 = 0;
		}
		//����б�
		for (ULONG i = 0; i < SYSTEMNUMBER; i++)
		{
			//�����ϵͳ���̾�����б�
			if (g_dynData->SystemInformationList.SystemListPID[i] == In_ProcessId)
			{
				g_dynData->SystemInformationList.SystemListEprocess[i] = 0;
				g_dynData->SystemInformationList.SystemListPID[i] = 0;
			}
		}
	}
	return STATUS_SUCCESS;
}