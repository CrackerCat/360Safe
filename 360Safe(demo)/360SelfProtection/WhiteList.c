#include "WhiteList.h"

//�ж��ǲ��ǰ���������
//1������ǣ���������������Ϣ��������Ĩ��
//2��������ǣ�ֱ���˳�
BOOLEAN Safe_DeleteWhiteList_PID(_In_ HANDLE ProcessId)
{
	KIRQL NewIrql;
	ULONG Index = 0;						//�±�����
	NewIrql = KfAcquireSpinLock(&g_White_List.SpinLock);
	//�жϰ���������
	if (g_White_List.WhiteListNumber)
	{
		for (ULONG Index = 0; Index < g_White_List.WhiteListNumber; Index++)
		{
			//�жϾ���Ϸ��ԣ������4�ı�����
			//0x00,0x04,0x08,0x10,0x14�ȵȵĶ����Ƽ�Ȼ��2λ��ԶΪ0����ô΢�������������λ��һ����־λ������ָʾ��ǰ���ֵ��������ں˶����Ǹ������������ҵ���
			if ((((ULONG)ProcessId | 3) ^ 3) == ((g_White_List.WhiteListPID[Index] | 3) ^ 3))
			{
				//����˳����̵���Ϣ(��һ����ǰŲ)
				for (ULONG i = Index; i <= g_White_List.WhiteListNumber; i++)
				{
					g_White_List.WhiteListPID[i] = g_White_List.WhiteListPID[i + 1];			//����PID
					g_White_List.SafeModIndex[i] = g_White_List.SafeModIndex[i + 1];			//δ֪
				}
				//�������̸���-1
				--g_White_List.WhiteListNumber;
				break;
			}
		}
	}
	KfReleaseSpinLock(&g_White_List.SpinLock, NewIrql);
	return TRUE;
}

//����ProcessId��SessionIdɾ��
//1������ǣ���������������Ϣ��������Ĩ��
//2��������ǣ�ֱ���˳�
BOOLEAN Safe_DeleteWhiteList_PID_SessionId(_In_ HANDLE ProcessId)
{
	KIRQL NewIrql;
	ULONG Index = 0;						//�±�����
	NewIrql = KfAcquireSpinLock(&g_White_List.SpinLock);
	//�жϰ���������
	if (g_White_List.WhiteListNumber)
	{
		for (ULONG Index = 0; Index < g_White_List.WhiteListNumber; Index++)
		{
			//�жϾ���Ϸ��ԣ������4�ı�����
			//0x00,0x04,0x08,0x10,0x14�ȵȵĶ����Ƽ�Ȼ��2λ��ԶΪ0����ô΢�������������λ��һ����־λ������ָʾ��ǰ���ֵ��������ں˶����Ǹ������������ҵ���
			if (((((ULONG)ProcessId | 3) ^ 3) == ((g_White_List.WhiteListPID[Index] | 3) ^ 3)) && g_White_List.SafeModIndex[Index])
			{
				//����˳����̵���Ϣ(��һ����ǰŲ)
				for (ULONG i = Index; i <= g_White_List.WhiteListNumber; i++)
				{
					g_White_List.WhiteListPID[i] = g_White_List.WhiteListPID[i + 1];			//����PID
					g_White_List.SafeModIndex[i] = g_White_List.SafeModIndex[i + 1];	//δ֪
				}
				//�������̸���-1
				--g_White_List.WhiteListNumber;
				break;
			}
		}
	}
	KfReleaseSpinLock(&g_White_List.SpinLock, NewIrql);
	return TRUE;
}


//����SessionIdɾ��
//1������ǣ���������������Ϣ��������Ĩ��
//2��������ǣ�ֱ���˳�
BOOLEAN Safe_DeleteWhiteList_SessionId(_In_ HANDLE SessionId)
{
	KIRQL NewIrql;
	ULONG Index = 0;						//�±�����
	NewIrql = KfAcquireSpinLock(&g_White_List.SpinLock);
	//�жϰ���������
	if (g_White_List.WhiteListNumber)
	{
		for (ULONG Index = 0; Index < g_White_List.WhiteListNumber; Index++)
		{
			
			if ((ULONG)SessionId == g_White_List.SafeModIndex[Index])
			{
				//����˳����̵���Ϣ(��һ����ǰŲ)
				for (ULONG i = Index; i <= g_White_List.WhiteListNumber; i++)
				{
					g_White_List.WhiteListPID[i] = g_White_List.WhiteListPID[i + 1];			//����PID
					g_White_List.SafeModIndex[i] = g_White_List.SafeModIndex[i + 1];		    //δ֪
				}
				//�������̸���-1
				--g_White_List.WhiteListNumber;
				break;
			}
		}
	}
	KfReleaseSpinLock(&g_White_List.SpinLock, NewIrql);
	return TRUE;
}

//Win2K
// ��Ӱ�����������Ϣ
// �ɹ�����1��ʧ�ܷ���0
BOOLEAN  Safe_InsertWhiteList_PID_Win2003(_In_ HANDLE ProcessId, _In_ ULONG SafeModIndex)
{
	PEPROCESS	ProcObject;
	KIRQL NewIrql;
	ULONG Index = 0;						//�±�����
	UCHAR ImageFileNameBuff[0x256] = { 0 };
	NTSTATUS	status,result;
	result = TRUE;							//Ĭ�Ϸ���ֵ
	status = PsLookupProcessByProcessId(ProcessId, &ProcObject);
	if (NT_SUCCESS(status))
	{
		Safe_PsGetProcessImageFileName(ProcObject, &ImageFileNameBuff, sizeof(ImageFileNameBuff));
		if (!_stricmp(&ImageFileNameBuff, "360Safeup.exe") || !_stricmp(&ImageFileNameBuff, "Safeboxup.exe"))
		{
			SafeModIndex = 0;
		}
		ObfDereferenceObject(ProcObject);
	}
	NewIrql = KfAcquireSpinLock(&g_White_List.SpinLock);
	//�жϰ���������
	if (g_White_List.WhiteListNumber)
	{
		//1����������  ����������+1���ɹ�����TRUE������<=0xFE����ʧ��FALSE������>0xFE��
		//2���Ѵ���    ���ӣ�Ĭ�Ϸ���TRUE���ɹ���
		while ((((ULONG)ProcessId | 3) ^ 3) != ((g_White_List.WhiteListPID[Index] | 3) ^ 3))
		{
			//�������µİ�������Ϣ�Ͳ���
			if (++Index >= g_White_List.WhiteListNumber)
			{
				//���������̸���<=0xFE
				if (g_White_List.WhiteListNumber <= WHITELISTNUMBER)
				{
					g_White_List.WhiteListPID[g_White_List.WhiteListNumber] = ProcessId;
					g_White_List.SafeModIndex[g_White_List.WhiteListNumber] = SafeModIndex;		//����SafeMon,���Ҹ�dos·�����б�ڼ��ret_arg = ���������±�
					//��������������1
					g_White_List.WhiteListNumber++;
					//�ɹ�����
					result = TRUE;
					break;
				}
				else
				{
					//ʧ�ܷ���
					result = FALSE;
					break;
				}
			}
		}
	}
	KfReleaseSpinLock(&g_White_List.SpinLock, NewIrql);
	return result;
}


// ��Ӱ�����������Ϣ
// �ɹ�����1��ʧ�ܷ���0
BOOLEAN  Safe_InsertWhiteList_PID(_In_ HANDLE ProcessId, _In_ ULONG SafeModIndex)
{
	KIRQL NewIrql;
	ULONG Index = 0;						//�±�����
	UCHAR ImageFileNameBuff[0x256] = { 0 };
	NTSTATUS	status, result;
	ULONG GotoFalg;							//����ͬgoto���õ�Falg
	GotoFalg = 1;
	NewIrql = KfAcquireSpinLock(&g_White_List.SpinLock);
	//�жϰ���������
	if (g_White_List.WhiteListNumber)
	{
		//1����������  ����������+1���ɹ�����TRUE������<=0xFE����ʧ��FALSE������>0xFE��
		while ((((ULONG)ProcessId | 3) ^ 3) != ((g_White_List.WhiteListPID[Index] | 3) ^ 3))
		{
			//�������µİ�������Ϣ�Ͳ���
			if (++Index >= g_White_List.WhiteListNumber)
			{
				//ȡ������2
				GotoFalg = 0;
				//���������̸���<=0xFE
				if (g_White_List.WhiteListNumber <= WHITELISTNUMBER)
				{
					g_White_List.WhiteListPID[g_White_List.WhiteListNumber] = ProcessId;
					g_White_List.SafeModIndex[g_White_List.WhiteListNumber] = SafeModIndex;
					//��������������1
					g_White_List.WhiteListNumber++;
					//�ɹ�����
					result = TRUE;
					break;
				}
				else
				{
					//ʧ�ܷ���
					result = FALSE;
					break;
				}
			}
		}
		//2���Ѵ���    ֻ���SafeModIndex����
		if (GotoFalg)
		{
			g_White_List.SafeModIndex[Index] = SafeModIndex;
			result = TRUE;
		}
	}
	KfReleaseSpinLock(&g_White_List.SpinLock, NewIrql);
	return result;
}

//�ж��ǲ��ǰ�����_EPROCESS
//����ֵ����1������0
BOOLEAN Safe_QueryWhiteEProcess(_In_ PEPROCESS Process)
{
	ULONG result;
	PEPROCESS	ProcObject;
	NTSTATUS	status;
	result = FALSE;
	//�жϰ���������
	if (g_White_List.WhiteListNumber)
	{
		for (ULONG Index = 0; Index < g_White_List.WhiteListNumber; Index++)
		{
			status = PsLookupProcessByProcessId(g_White_List.WhiteListPID[Index], &ProcObject);
			if (NT_SUCCESS(status))
			{
				ObfDereferenceObject(ProcObject);
				//�ж�Process�Ƿ������������ͬ
				if (Process == ProcObject)
				{
					result = TRUE;
					break;
				}

			}
		}
	}
	else
	{
		result = FALSE;
	}
	return result;
}


//�ж��ǲ��ǰ�������PID
//����ֵ����1������0
BOOLEAN Safe_QueryWhitePID(_In_ HANDLE ProcessId)
{
	ULONG result;
	PEPROCESS	ProcObject;
	result = FALSE;
	//�жϰ���������
	if (g_White_List.WhiteListNumber)
	{
		for (ULONG Index = 0; Index < g_White_List.WhiteListNumber; Index++)
		{
			//�ж��ǲ��ǰ���������
			if ((((ULONG)ProcessId | 3) ^ 3) == ((g_White_List.WhiteListPID[Index] | 3) ^ 3))
			{
				//����Ƿ���TRUE
				result = TRUE;
				break;
			}
		}
	}
	else
	{
		result = FALSE;
	}
	return result;
}


//�������ܣ�
//�ж��������������SessionId�Ƿ���ڵ�ǰ���̵�SessionId
//����ֵ��
//����ֵ����1������0
BOOLEAN Safe_QuerySpecialWhiteSessionId()
{
	ULONG SelfSessionId, SpecialWhiteSessionId;
	PVOID pSessionIDAddress = 0xFFDF02D8;	//Win10_14393���°汾һ���̶���ַ���Ի�ȡ��SessionId
	KIRQL NewIrql;
	ULONG result;
	ULONG Index = 0;						//�±�����
	//WINDOWS_VERSION_2K
	if (g_VersionFlag == WINDOWS_VERSION_2K)
	{
		result = TRUE;
		return result;
	}
	//Win10_14393���ϻ�ȡ��ʽ
	if (g_dynData->pRtlGetActiveConsoleId_Win10_14393)     
	{
		SelfSessionId = g_dynData->pRtlGetActiveConsoleId_Win10_14393();		//��ȡ����SessionId
	}
	else
	{
		if (!MmIsAddressValid(pSessionIDAddress))
		{
			result = TRUE;
			return result;
		}
		SelfSessionId = *(ULONG*)pSessionIDAddress;								//��ȡ����SessionId
	}
	//��������
	NewIrql = KfAcquireSpinLock(&g_SpecialWhite_List.SpinLock);
	//�ж��������������
	if (g_SpecialWhite_List.SpecialWhiteListNumber)
	{
		for (ULONG Index = 0; Index < g_SpecialWhite_List.SpecialWhiteListNumber; Index++)
		{
			SpecialWhiteSessionId = g_SpecialWhite_List.SpecialWhiteListSessionId;
			if (SelfSessionId == SpecialWhiteSessionId || SpecialWhiteSessionId == SPECIALSIGN)
			{
				result = TRUE;
				break;
			}
		}
	}
	else
	{
		result = FALSE;
	}
	//��������
	KfReleaseSpinLock(&g_SpecialWhite_List.SpinLock, NewIrql);
	return result;
}

//Eprocess_UniqueProcessId
//�ж��ǲ��ǰ�������PID
//����ֵ����1������0
BOOLEAN Safe_QueryWhitePID_PsGetProcessId(IN PEPROCESS pPeprocess)
{
	BOOLEAN        Result = FALSE;
	HANDLE         ProcessId = NULL;
	ProcessId = Safe_pPsGetProcessId(pPeprocess);
	if (ProcessId)
	{
		Result = Safe_QueryWhitePID(ProcessId);
	}
	return Result;
}

//����ProcessHandleת����Eprocess��Ȼ�����Safe_QueryWhitePID_PsGetProcessId
//�ж��ǲ��ǰ�������PID
//����ֵ����1������0
BOOLEAN Safe_QueryWintePID_ProcessHandle(IN HANDLE ProcessHandle)
{
	NTSTATUS       Status;
	BOOLEAN        Result = FALSE;
	Status = STATUS_SUCCESS;
	PEPROCESS pPeprocess = NULL;
	if (ProcessHandle && (Status = ObReferenceObjectByHandle(ProcessHandle, NULL, PsProcessType, UserMode, &pPeprocess, NULL), NT_SUCCESS(Status)))
	{
		Result = Safe_QueryWhitePID_PsGetProcessId(pPeprocess);
		ObfDereferenceObject((PVOID)pPeprocess);
	}
	else
	{
		Result = FALSE;
	}
	return Result;
}

//����ThreadHandle��ȡ��ǰ����PID
BOOLEAN  Safe_QueryWhitePID_PsGetThreadProcessId(PVOID VirtualAddress)
{
	BOOLEAN        Result = FALSE;
	HANDLE         ProcessId = NULL;
	if (!MmIsAddressValid(VirtualAddress))
	{
		return Result;
	}
	//ֻ��win2K�Ż��У�Ӧ����ETHREADĳ��ƫ��
	if (g_dynData->Eprocess_Offset.dword_34DF4)
	{
		if (!MmIsAddressValid((CHAR *)VirtualAddress + g_dynData->Eprocess_Offset.dword_34DF4))
		{
			return Result;
		}
		Result = Safe_QueryWhitePID_PsGetProcessId(*(PVOID *)((CHAR *)VirtualAddress + g_dynData->Eprocess_Offset.dword_34DF4));
	}
	else
	{
		ProcessId = g_dynData->pPsGetThreadProcessId(VirtualAddress);
		Result = Safe_QueryWhitePID(ProcessId);
	}
	return Result;
}

//�����߳̾����ȡPID��Ȼ���ж�PID�Ƿ��Ǳ�������
//�ж��ǲ��ǰ�������PID
//����ֵ����1������0
BOOLEAN Safe_QueryWintePID_ThreadHandle(IN HANDLE ThreadHandle)
{
	NTSTATUS       Status;
	BOOLEAN        Result = FALSE;
	Status = STATUS_SUCCESS;
	PETHREAD ThreadObject = NULL;
	if (ThreadHandle && (Status = ObReferenceObjectByHandle(ThreadHandle, NULL, PsThreadType, UserMode, &ThreadObject, NULL), NT_SUCCESS(Status)))
	{
		Result = Safe_QueryWhitePID_PsGetThreadProcessId(ThreadObject);
		ObfDereferenceObject((PVOID)ThreadObject);
	}
	else
	{
		Result = FALSE;
	}
	return Result;
}