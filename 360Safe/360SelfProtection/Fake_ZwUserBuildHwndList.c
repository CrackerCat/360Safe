#include "Fake_ZwUserBuildHwndList.h"

NTSTATUS NTAPI After_ZwUserBuildHwndList_Func(IN ULONG FilterIndex, IN PVOID ArgArray, IN NTSTATUS InResult, IN PULONG RetFuncArgArray)
{
	NTSTATUS   Result;
	Result = STATUS_SUCCESS;
	//0����ȡZwUserBuildHwndListԭʼ����

	ULONG In_cHwndMax = *(ULONG*)((ULONG)ArgArray + 0x10);
	PULONG In_phwndFirst = *(ULONG*)((ULONG)ArgArray + 0x14);
	ULONG In_pcHwndNeeded = *(ULONG*)((ULONG)ArgArray + 0x18);
	//1���ж��ϴε���ԭʼ��������ֵ
	if (!NT_SUCCESS(InResult))
	{
		return InResult;
	}
	//2���жϵ������ǲ��Ǳ�������
	//�ǣ�����
	//���ǣ����
	if (!Safe_QueryWhitePID(PsGetCurrentProcessId()))
	{
		if (In_cHwndMax)
		{
			if (myProbeRead(In_pcHwndNeeded, sizeof(PVOID), sizeof(ULONG)) && myProbeWrite(In_phwndFirst, sizeof(ULONG) * In_cHwndMax, sizeof(ULONG)))
			{
				KdPrint(("ProbeRead(After_ZwUserBuildHwndList_Func��In_pcHwndNeeded) error \r\n"));
				KdPrint(("myProbeWrite(After_ZwUserBuildHwndList_Func��In_phwndFirst) error \r\n"));
				return Result;
			}
			ULONG i = 0;
			ULONG j;

			while (i < In_pcHwndNeeded)
			{
				//ProcessID = orgNtUserQueryWindow((HWND)phwndFirst[i], 0);
				//Padd = querylist(PmainList, ProcessID, PsGetCurrentProcess());
			
					for (j = i; j < (In_pcHwndNeeded) - 1; j++)
					{
						In_phwndFirst[j] = In_phwndFirst[j + 1];
					}
					//ɾ�������е�
					In_phwndFirst[In_pcHwndNeeded - 1] = 0;

					//�ܴ�С-1
					*(ULONG*)In_pcHwndNeeded--;
					continue;
				
				i++;
			}
		}
	}
	return Result;
}

NTSTATUS NTAPI Fake_ZwUserBuildHwndList(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS   Result;
	Result = STATUS_SUCCESS;
	//1��������Ӧ�ò����
	if (ExGetPreviousMode()
		&& !Safe_CmpImageFileName("csrss.exe")
		&& !Safe_CmpImageFileName("matlab.exe")
		&& !Safe_CmpImageFileName("mupad.exe")
		&& !Safe_CmpImageFileName("explorer.exe"))
	{
		*ret_func = After_ZwUserBuildHwndList_Func;
	}
	return Result;
}