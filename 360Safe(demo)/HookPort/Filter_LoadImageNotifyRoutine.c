/*
�����򲿷֣�win10�汾����������
���LoadImageNotifyRoutine��Fake��������HookPort���������ˣ�������360SelfProtection����
*/

#include "Filter_LoadImageNotifyRoutine.h"


//Filter_LoadImageNotifyRoutine�ص���������Ŀ��أ���ֹ�ظ�����
ULONG Global_LoadImageNotifyRoutine_Flag = NULL;		//���أ�ִ�о���1��δִ����0����ֹ�ظ�����

// ���ò���
VOID HookPort_SetFlag_Off()
{
	dword_1B110 = 0;
}

BOOL HookPort_CmpNtdll(PUNICODE_STRING FullImageName)
{
	UNICODE_STRING String_Ntdll = { 0 };
	UNICODE_STRING String_System32Ntdll = { 0 };
	NTSTATUS result = FALSE;
	RtlInitUnicodeString(&String_Ntdll, L"\\SystemRoot\\System32\\ntdll.dll");
	if ((RtlEqualUnicodeString(FullImageName, &String_Ntdll, TRUE) == 1) || (FullImageName->Length > 0x28))
	{
		if (_wcsnicmp((PWSTR)((CHAR *)FullImageName->Buffer + FullImageName->Length - 0x26), L"\\System32\\ntdll.dll", 0x13u) == 0)
		{
			result = TRUE;
		}
	}

	return result;

}

//���LoadImageNotifyRoutine��Fake��������HookPort���������ˣ�������360SelfProtection����
VOID Filter_LoadImageNotifyRoutine(IN PUNICODE_STRING FullImageName, IN HANDLE ProcessId, IN PIMAGE_INFO ImageInfo)
{
	NTSTATUS result;
	PULONG FuncTable[16];
	PULONG ArgTable[16];

	ULONG		RetCount;
	PVOID		pArgArray = &FullImageName;//�������飬ָ��ջ�����ڱ����������в���
	if (Global_Version_Win10_Flag)
	{
		//δ�����Win10δ����
		if (!Global_Win32kFlag)
		{
			if (FullImageName)
			{
				if (BYTE1(ImageInfo->Properties) & 1)
				{
					if (HookPort_CmpNtdll(FullImageName))
					{
						//���ø����ط�ֹ�ظ�����
						if (!InterlockedCompareExchange(Global_LoadImageNotifyRoutine_Flag, 1, 0))
						{
							////����������
							//if (HookPort_12864(ImageInfo->ImageBase, 1))
							//{
							//	HookPort_InitFilterTable();
							//	if (HookPort_InitProxyAddress(0))
							//	{
							//		//����ZwSetSystemInformation������hook����
							//		g_SS_Filter_Table->SwitchTableForSSDT[g_SSDT_Func_Index_Data.ZwSetSystemInformationIndex] = 1;
							//		//sub_18500();
							//		HookPort_SetFlag_Off();
							//	}
							//}
						}
					}
				}
			}
		}
	}
	HookPort_DoFilter(LoadImageNotifyRoutine_FilterIndex, pArgArray, 0, 0, 0, 0);
}

