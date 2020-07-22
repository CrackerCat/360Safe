/*
ʱ�䣺����һ����ʮһ�¶�ʮ���� 16:01:42
���ܣ�ѧϰ����hookport�ں�
�ɻ�ĵط���ԭ��������û��ж�غ����ģ�����������
�ο����ϣ�
1��Hookport�������																		 
��ַ��https://bbs.pediy.com/thread-194022.htm
2��WinDBG��������ʱ�ж�DriverEntry�ķ���												 
��ַ��https://www.cnblogs.com/DeeLMind/p/7093321.html
3��---RootKit ���ļ����������� NT!_MDL ͻ�� KiServiceTable ��ֻ���������� PART II ----   
��ַ��https://bbs.pediy.com/thread-226043.htm
4��[����]SSDT/SSSDT��Щ��                                                                
��ַ��https://blog.csdn.net/zhuhuibeishadiao/article/details/71440177
*/

#include "Driver.h"


//��hash�㷨�ĺ���
__declspec(naked) HookPort_EncryptHash(UCHAR *pImageName)
{
	_asm
	{
			mov     edi, edi
			push    ebp
			mov     ebp, esp
			push    ecx
			push    ebx
			push    esi
			push    edi
			pushad
			mov     esi, [ebp + 8]
			mov     ebx, [ebp + 8]
			mov     edi, [ebp + 8]
			xor     al, al
		loc_16F35 :
			scasb
			jnz     short loc_16F35
			sub     edi, ebx
			cld
			xor     ecx, ecx
			dec     ecx
			mov     edx, ecx
		loc_16F40 :
			xor     eax, eax
			xor     ebx, ebx
			lodsb
			xor     al, cl
			mov     cl, ch
			mov     ch, dl
			mov     dl, dh
			mov     dh, 8
		loc_16F4F:
			shr     bx, 1
			rcr     ax, 1
			jnb     short loc_16F60
			xor     ax, 0C6B4h
			xor     bx, 0CE96h
		loc_16F60 :
			dec     dh
			jnz     short loc_16F4F
			xor     ecx, eax
			xor     edx, ebx
			dec     edi
			jnz     short loc_16F40
			not     edx
			not     ecx
			mov     eax, edx
			rol     eax, 10h
			mov     ax, cx
			mov[ebp - 4], eax
			popad
			mov     eax, [ebp - 4]
			pop     edi
			pop     esi
			pop     ebx
			Leave
			retn    4
	}
}


//************************************     
// ��������: HookPort_GetModuleBaseAddress_EncryptHash     
// ����˵�������ݺ�������ȡָ���ں˻�ַ    
// IDA��ַ ��sub_16E2C
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2019/12/05     
// �� �� ֵ: BOOLEAN NTAPI     
// ��    ��: ULONG   Hash				��ϣֵ 
// ��    ��: PVOID * pModuleBase        ģ���ַ
// ��    ��: ULONG * ModuleSize         ģ���С
// ��    ��: ULONG * LoadOrderIndex    
//************************************  
BOOLEAN  HookPort_GetModuleBaseAddress_EncryptHash(IN ULONG Hash, OUT PVOID *pModuleBase, OUT ULONG *ModuleSize, OUT ULONG *LoadOrderIndex)
{

	NTSTATUS status; // eax@5
	ULONG    Result;
	ULONG    uCount; // eax@8  
	PSYSTEM_MODULE_INFORMATION    pSysModule;

	ULONG ReturnLength; // [sp+Ch] [bp-14h]@5  
	PCHAR  pModuleInfo = NULL; // [sp+10h] [bp-10h]@8
	size_t	BufLen = 4096; // [sp+14h] [bp-Ch]@12
	ULONG HookPortTag = 0x494E4654;
	PCHAR   pName = NULL;
	ULONG   ui;
	PVOID   pBuff = NULL;
	Result = 0;
	do {

		if (pModuleInfo)
		{
			ExFreePool(pModuleInfo);
		}

		pModuleInfo = ExAllocatePoolWithTag(NonPagedPool, BufLen, HookPortTag);

		if (!pModuleInfo)
		{
			Result = 0;
			return Result;

		}

		status = ZwQuerySystemInformation(SystemModuleInformation, pModuleInfo, BufLen, &ReturnLength);
		if (!NT_SUCCESS(status) && status != STATUS_INFO_LENGTH_MISMATCH) {
			ExFreePool(pModuleInfo);
			return Result;
		}

		BufLen += 4096;

	} while (!NT_SUCCESS(status));

	uCount = (ULONG)*(ULONG *)pModuleInfo;
	pSysModule = (PSYSTEM_MODULE_INFORMATION)(pModuleInfo + sizeof(ULONG));
	for (ui = 0; ui < uCount; ui++)
	{
		pName = strrchr(pSysModule->ImageName, '\\');
		if (pName) {
			++pName;
		}
		else {
			pName = pSysModule->ImageName;
		}
		//�л���Сд�����ϣֵ

		pBuff = _strlwr(pName);
		ULONG LocalHash = HookPort_EncryptHash(pBuff);
		//KdPrint(("ImageName:%s Hash:%X\t\n", pBuff, LocalHash));
		if (LocalHash == Hash)
		{
			//�ҵ����˳�ѭ��
			Result = 1;
			break;
		}
		pSysModule++;

	}


	if (ui >= uCount)
	{
		ExFreePool(pModuleInfo);
		Result = 0;
		return Result;
	}
	if (pModuleBase)
	{
		*pModuleBase = pSysModule->Base;
	}
	if (ModuleSize)
	{
		*ModuleSize = pSysModule->Size;
	}
	if (LoadOrderIndex)
	{
		*LoadOrderIndex = pSysModule->LoadOrderIndex;
	}
	ExFreePool(pModuleInfo);
	return Result;
}

//LoadImageNotifyRoutine��Fake����
ULONG Fake_LoadImageNotifyRoutine(ULONG CallIndex, PVOID ArgArray, PULONG ret_func, PULONG ret_arg)
{
	//0���������
	PUNICODE_STRING FullImageName;
	HANDLE ProcessId;
	PIMAGE_INFO ImageInfo;
	PCHAR   pBuff, pName;
	ULONG   Result, HashNumber;
	STRING  DestinationString;
	//1�������������ȷֱ𱣴��������������ʹ��
	FullImageName = *(ULONG*)((ULONG)ArgArray);
	ProcessId = *(ULONG*)((ULONG)ArgArray + 4);
	ImageInfo = *(ULONG*)((ULONG)ArgArray + 8);
	pBuff = NULL;
	//2���жϲ�����ȡ�Ƿ�����
	if ((!(ImageInfo->SystemModeImage)) && (ImageInfo->ImageBase < MmHighestUserAddress) && !FullImageName)
	{
		return 0;
	}
	//3������llImageName�������hash
	pBuff = ExAllocatePoolWithTag(NonPagedPool, 0x100, HOOKPORT_POOLTAG7);
	if (!pBuff)
	{
		return 0;
	}
	DestinationString.Length = 0;
	DestinationString.MaximumLength = 0x100;
	DestinationString.Buffer = pBuff;
	if (RtlUnicodeStringToAnsiString(&DestinationString, FullImageName, 0) < 0)
	{
		ExFreePool(pBuff);
		return 0;
	}
	pName = strrchr(pBuff, '\\');
	if (pName)
	{
		++pName;
	}
	else
	{
		pName = pBuff;
	}
	//3��1 �����ϣ
	HashNumber = HookPort_EncryptHash(pName);
	//3��2 �ͷſռ�
	ExFreePool(pBuff);
	//4 δ֪
	if (HashNumber == Global_Hash_2)
	{
		if (dword_1B120 && !dword_1B128 && !dword_1B12C)
		{
			//hook KiSystemService 
			//����ֵ���ɹ�����1  ʧ�ܷ���0
			if (HookPort_SetFakeKiSystemServiceAddress() == (_DWORD *)1)
			{
				dword_1B128 = 1;
			}
			dword_1B120 = 0;
			return 0;
		}
	}
	//5 δ֪
	else if ((HashNumber == Global_Hash_3 || HashNumber == Global_Hash_4) && !dword_1B12C && !dword_1B130)
	{
		//���ÿպ������к������أ����������� δ֪
		////���ù�����˺����뿪��
		HookPort_SetFilterSwitchFunction(g_FilterFun_Rule_table_head_Temp, ZwDisplayString_FilterIndex, Fake_VacancyFunc);
		HookPort_SetFilterRule(g_FilterFun_Rule_table_head_Temp, ZwDisplayString_FilterIndex, 1);
		dword_1B130 = 1;
	}
	return 0;
}

//************************************     
// ��������: HookPort_AllocFilterTable     
// ����˵����׼������������SSDT��SSSDT��Ϣ
// IDA��ַ ��sub_15774
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2019/12/18     
// �� �� ֵ: BOOLEAN     
//************************************  
BOOLEAN  HookPort_AllocFilterTable()
{
	ULONG Hook_Tag = 0x53564354;
	g_SS_Filter_Table = (PSYSTEM_SERVICE_FILTER_TABLE)ExAllocatePoolWithTag(NonPagedPool, sizeof(SYSTEM_SERVICE_FILTER_TABLE), Hook_Tag);
	if (!g_SS_Filter_Table)
		return FALSE;
	RtlZeroMemory(g_SS_Filter_Table, sizeof(SYSTEM_SERVICE_FILTER_TABLE));
	return TRUE;
}

//************************************     
// ��������: HookPort_GetNativeFunAddress     
// ����˵������ȡָ��������ַ    
// IDA��ַ ��sub_121C6
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2019/12/05     
// �� �� ֵ: BOOLEAN      
//************************************ 
BOOLEAN  HookPort_GetNativeFunAddress(PVOID* NtImageBase)
{
	ULONG result; // eax@2
	STRING DestinationString; // [sp+4h] [bp-8h]@1

	RtlInitAnsiString(&DestinationString, "ZwAccessCheckAndAuditAlarm");
	g_SSDT_Func_Index_Data.pZwAccessCheckAndAuditAlarm = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase);
	if (g_SSDT_Func_Index_Data.pZwAccessCheckAndAuditAlarm
		&& (RtlInitAnsiString(&DestinationString, "ZwAdjustPrivilegesToken"),
		(g_SSDT_Func_Index_Data.pZwAdjustPrivilegesToken = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwAllocateVirtualMemory"),
		(g_SSDT_Func_Index_Data.pZwAllocateVirtualMemory = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwAlpcConnectPort"),
		g_SSDT_Func_Index_Data.pZwAlpcConnectPort = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase),
		RtlInitAnsiString(&DestinationString, "ZwAlpcConnectPortEx"),
		g_SSDT_Func_Index_Data.pZwAlpcConnectPortEx = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase),
		RtlInitAnsiString(&DestinationString, "ZwConnectPort"),
		(g_SSDT_Func_Index_Data.pZwConnectPort = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwCreateFile"),
		(g_SSDT_Func_Index_Data.pZwCreateFile = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwCreateKey"),
		(g_SSDT_Func_Index_Data.pZwCreateKey = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwCreateSection"),
		(g_SSDT_Func_Index_Data.pZwCreateSection = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwCreateSymbolicLinkObject"),
		(g_SSDT_Func_Index_Data.pZwCreateSymbolicLinkObject = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwDeleteFile"),
		(g_SSDT_Func_Index_Data.pZwDeleteFile = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwDeleteKey"),
		(g_SSDT_Func_Index_Data.pZwDeleteKey = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwDeleteValueKey"),
		(g_SSDT_Func_Index_Data.pZwDeleteValueKey = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwDeviceIoControlFile"),
		(g_SSDT_Func_Index_Data.pZwDeviceIoControlFile = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwDisplayString"),
		(g_SSDT_Func_Index_Data.pZwDisplayString = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwDuplicateObject"),
		(g_SSDT_Func_Index_Data.pZwDuplicateObject = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwEnumerateKey"),
		(g_SSDT_Func_Index_Data.pZwEnumerateKey = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwEnumerateValueKey"),
		(g_SSDT_Func_Index_Data.pZwEnumerateValueKey = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwFreeVirtualMemory"),
		(g_SSDT_Func_Index_Data.pZwFreeVirtualMemory = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwFsControlFile"),
		(g_SSDT_Func_Index_Data.pZwFsControlFile = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwLoadDriver"),
		(g_SSDT_Func_Index_Data.pZwLoadDriver = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwLoadKey"),
		(g_SSDT_Func_Index_Data.pZwLoadKey = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwMapViewOfSection"),
		(g_SSDT_Func_Index_Data.pZwMapViewOfSection = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwMakeTemporaryObject"),
		(g_SSDT_Func_Index_Data.pZwMakeTemporaryObject = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwOpenFile"),
		(g_SSDT_Func_Index_Data.pZwOpenFile = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwOpenKey"),
		(g_SSDT_Func_Index_Data.pZwOpenKey = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwOpenKeyEx"),
		g_SSDT_Func_Index_Data.pZwOpenKeyEx = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase),
		RtlInitAnsiString(&DestinationString, "ZwOpenProcess"),
		(g_SSDT_Func_Index_Data.pZwOpenProcess = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwOpenThread"),
		(g_SSDT_Func_Index_Data.pZwOpenThread = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwOpenSection"),
		(g_SSDT_Func_Index_Data.pZwOpenSection = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwOpenSymbolicLinkObject"),
		(g_SSDT_Func_Index_Data.pZwOpenSymbolicLinkObject = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwQueryKey"),
		(g_SSDT_Func_Index_Data.pZwQueryKey = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwQueryInformationProcess"),
		(g_SSDT_Func_Index_Data.pZwQueryInformationProcess = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwQueryInformationThread"),
		g_SSDT_Func_Index_Data.pZwQueryInformationThread = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase),
		RtlInitAnsiString(&DestinationString, "ZwQueryValueKey"),
		(g_SSDT_Func_Index_Data.pZwQueryValueKey = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwQuerySystemInformation"),
		(g_SSDT_Func_Index_Data.pZwQuerySystemInformation = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwReplaceKey"),
		(g_SSDT_Func_Index_Data.pZwReplaceKey = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwRequestWaitReplyPort"),
		(g_SSDT_Func_Index_Data.pZwRequestWaitReplyPort = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwRestoreKey"),
		(g_SSDT_Func_Index_Data.pZwRestoreKey = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwSecureConnectPort"),
		g_SSDT_Func_Index_Data.pZwSecureConnectPort = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase),
		RtlInitAnsiString(&DestinationString, "ZwSetInformationProcess"),
		(g_SSDT_Func_Index_Data.pZwSetInformationProcess = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwSetInformationFile"),
		(g_SSDT_Func_Index_Data.pZwSetInformationFile = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwSetInformationThread"),
		(g_SSDT_Func_Index_Data.pZwSetInformationThread = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwSetTimer"),
		(g_SSDT_Func_Index_Data.pZwSetTimer = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwSetSecurityObject"),
		(g_SSDT_Func_Index_Data.pZwSetSecurityObject = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwSetSystemInformation"),
		(g_SSDT_Func_Index_Data.pZwSetSystemInformation = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwSetSystemTime"),
		(g_SSDT_Func_Index_Data.pZwSetSystemTime = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwSetValueKey"),
		(g_SSDT_Func_Index_Data.pZwSetValueKey = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwTerminateProcess"),
		(g_SSDT_Func_Index_Data.pZwTerminateProcess = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwWriteFile"),
		(g_SSDT_Func_Index_Data.pZwWriteFile = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0)
		&& (RtlInitAnsiString(&DestinationString, "ZwUnloadDriver"),
		(g_SSDT_Func_Index_Data.pZwUnloadDriver = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase)) != 0))
	{
		RtlInitAnsiString(&DestinationString, "ZwUnmapViewOfSection");
		g_SSDT_Func_Index_Data.pZwUnmapViewOfSection = (ULONG)HookPort_GetSymbolAddress((ULONG)&DestinationString, NtImageBase);
		result = g_SSDT_Func_Index_Data.pZwUnmapViewOfSection != 0;
	}
	else
	{
		result = 0;
	}
	return result;

}

//************************************     
// ��������: HookPort_InitializeIndex     
// ����˵������ʼ��Nt�ں˺�������   
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2019/12/31     
// �� �� ֵ: BOOLEAN     
//************************************  
BOOLEAN HookPort_InitializeIndex()
{

	//SSDT����
	g_SSDT_Func_Index_Data.ZwCreateKeyIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwQueryValueKeyIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwDeleteKeyIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwDeleteValueKeyIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwRenameKeyIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwReplaceKeyIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwRestoreKeyIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwSetValueKeyIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwCreateFileIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwFsControlFileIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwSetInformationFileIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwWriteFileIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwCreateProcessIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwCreateProcessExIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwCreateUserProcessIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwCreateThreadIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwOpenThreadIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwDeleteFileIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwOpenFileIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwReadVirtualMemoryIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwWriteVirtualMemoryIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwTerminateProcessIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwQueueApcThreadIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwSetContextThreadIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwSetInformationThreadIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwProtectVirtualMemoryIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwAdjustGroupsTokenIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwAdjustPrivilegesTokenIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwRequestWaitReplyPortIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwCreateSectionIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwOpenSectionIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwCreateSymbolicLinkObjectIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwOpenSymbolicLinkObjectIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwLoadDriverIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwUnloadDriverIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwQuerySystemInformationIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwSetSystemInformationIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwSetSystemTimeIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwSystemDebugControlIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwOpenProcessIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwDeviceIoControlFileIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwOpenKeyIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwDuplicateObjectIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwWriteFileGatherIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwResumeThreadIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwQueryAttributesFileIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwCreateThreadExIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwAlpcSendWaitReceivePortIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwSetSecurityObjectIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwAllocateVirtualMemoryIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwOpenMutantIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwCreateMutantIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwVdmControlIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwGetNextThreadIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwGetNextProcessIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwRequestPortIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwFreeVirtualMemoryIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwEnumerateValueKeyIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwQueryKeyIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwEnumerateKeyIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwConnectPortIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwSecureConnectPortIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwAlpcConnectPortIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwSetTimerIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwSetInformationProcessIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwMapViewOfSectionIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwTerminateThreadIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwAssignProcessToJobObjectIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwTerminateJobObjectIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwDebugActiveProcessIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwSetInformationJobObjectIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwQueueApcThreadExIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwAlpcConnectPortExIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwMakeTemporaryObjectIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwDisplayStringIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwContinueIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwAccessCheckIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwAccessCheckAndAuditAlarmIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwQueryInformationThreadIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwQueryInformationProcessIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwQueryIntervalProfileIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwSetIntervalProfileIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwCreateProfileIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwSuspendThreadIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwSuspendProcessIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwApphelpCaCheControlIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwLoadKeyIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwLoadKey2Index = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwLoadKeyExIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwUnmapViewOfSectionIndex_Win8_Win10 = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.ZwOpenKeyExIndex = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.dword_1BAA0 = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.dword_1BB08 = g_SSDTServiceLimit;
	g_SSDT_Func_Index_Data.dword_1BA98 = g_SSDTServiceLimit;


	//ShadowSSDT����
	g_ShadowSSDT_Func_Index_Data.ZwUnmapViewOfSectionIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserSetWinEventHookIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserCallHwndParamLockIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserRegisterUserApiHookIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserSetParentIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserChildWindowFromPointExIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserDestroyWindowIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserInternalGetWindowTextIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserMoveWindowIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserRealChildWindowFromPointIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserSetInformationThreadIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserSetInternalWindowPosIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowLongIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowPlacementIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowPosIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowRgnIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserShowWindowIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserShowWindowAsyncIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserSendInputIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtUserCallOneParamIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtUserRegisterWindowMessageIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtUserCallNoParamIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtUserCallTwoParamIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtUserCallHwndLockIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtUserUnhookWindowsHookExIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtUserClipCursorIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtUserGetKeyStateIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtUserGetKeyboardStateIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtUserGetAsyncKeyStateIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtUserAttachThreadInputIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtUserRegisterHotKeyIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtUserRegisterRawInputDevicesIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtGdiBitBltIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtGdiStretchBltIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtGdiMaskBltIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtGdiPlgBltIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtGdiTransparentBltIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtGdiAlphaBlendIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtGdiGetPixelIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtUserGetRawInputDataIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtUserGetRawInputBufferIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtUserSetImeInfoExIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtGdiOpenDCWIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtGdiDeleteObjectAppIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtUserBlockInputIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtUserLoadKeyboardLayoutExIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtGdiAddFontResourceWIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtGdiAddFontMemResourceExIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.NtGdiAddRemoteFontToDCIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserBuildHwndListIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserQueryWindowIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserFindWindowExIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserWindowFromPointIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserMessageCallIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserPostMessageIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowsHookExIndex = g_SSDTServiceLimit;
	g_ShadowSSDT_Func_Index_Data.ZwUserPostThreadMessageIndex = g_SSDTServiceLimit;
	return TRUE;
}

//************************************     
// ��������: HookPort_GetAllNativeFunAddress     
// ����˵������ȡ������ַ��������š�    
// IDA��ַ ��sub_12ACC
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2019/12/05     
// �� �� ֵ: BOOLEAN NTAPI     
//************************************ 
BOOLEAN  NTAPI HookPort_GetAllNativeFunAddress(PVOID* NtImageBase, IN RTL_OSVERSIONINFOEXW osverinfo)
{
	ULONG BuildNumber = osverinfo.dwBuildNumber;
	ULONG MinorVersion = osverinfo.dwMinorVersion;
	ULONG MajorVersion = osverinfo.dwMajorVersion;
	//1����ȡ������ַ
	BOOLEAN	result = HookPort_GetNativeFunAddress(NtImageBase);
	if (!result)
	{
		return FALSE;
	}
	//2����ʼ��������ű���ΪĬ��ֵ;
	HookPort_InitializeIndex();
	//3����ȡ�������
	if (!g_SSDT_Func_Index_Data.pZwCreateKey || *(UCHAR *)g_SSDT_Func_Index_Data.pZwCreateKey != 0xB8 || (g_SSDT_Func_Index_Data.ZwCreateKeyIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwCreateKey + 1), g_SSDT_Func_Index_Data.ZwCreateKeyIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwCreateKeyIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwQueryValueKey || *(UCHAR *)g_SSDT_Func_Index_Data.pZwQueryValueKey != 0xB8 || (g_SSDT_Func_Index_Data.ZwQueryValueKeyIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwQueryValueKey + 1), g_SSDT_Func_Index_Data.ZwQueryValueKeyIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwQueryValueKeyIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwDeleteKey || *(UCHAR *)g_SSDT_Func_Index_Data.pZwDeleteKey != 0xB8 || (g_SSDT_Func_Index_Data.ZwDeleteKeyIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwDeleteKey + 1), g_SSDT_Func_Index_Data.ZwDeleteKeyIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwDeleteKeyIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwDeleteValueKey || *(UCHAR *)g_SSDT_Func_Index_Data.pZwDeleteValueKey != 0xB8 || (g_SSDT_Func_Index_Data.ZwDeleteValueKeyIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwDeleteValueKey + 1), g_SSDT_Func_Index_Data.ZwDeleteValueKeyIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwDeleteValueKeyIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwSetValueKey || *(UCHAR *)g_SSDT_Func_Index_Data.pZwSetValueKey != 0xB8 || (g_SSDT_Func_Index_Data.ZwSetValueKeyIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwSetValueKey + 1), g_SSDT_Func_Index_Data.ZwSetValueKeyIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwSetValueKeyIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwCreateFile || *(UCHAR *)g_SSDT_Func_Index_Data.pZwCreateFile != 0xB8 || (g_SSDT_Func_Index_Data.ZwCreateFileIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwCreateFile + 1), g_SSDT_Func_Index_Data.ZwCreateFileIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwCreateFileIndex = g_SSDTServiceLimit;

	if (!g_SSDT_Func_Index_Data.pZwSetInformationFile || *(UCHAR *)g_SSDT_Func_Index_Data.pZwSetInformationFile != 0xB8 || (g_SSDT_Func_Index_Data.ZwSetInformationFileIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwSetInformationFile + 1), g_SSDT_Func_Index_Data.ZwSetInformationFileIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwSetInformationFileIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwWriteFile || *(UCHAR *)g_SSDT_Func_Index_Data.pZwWriteFile != 0xB8 || (g_SSDT_Func_Index_Data.ZwWriteFileIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwWriteFile + 1), g_SSDT_Func_Index_Data.ZwWriteFileIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwWriteFileIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwOpenThread || *(UCHAR *)g_SSDT_Func_Index_Data.pZwOpenThread != 0xB8 || (g_SSDT_Func_Index_Data.ZwOpenThreadIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwOpenThread + 1), g_SSDT_Func_Index_Data.ZwOpenThreadIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwOpenThreadIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwDeleteFile || *(UCHAR *)g_SSDT_Func_Index_Data.pZwDeleteFile != 0xB8 || (g_SSDT_Func_Index_Data.ZwDeleteFileIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwDeleteFile + 1), g_SSDT_Func_Index_Data.ZwDeleteFileIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwDeleteFileIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwOpenFile || *(UCHAR *)g_SSDT_Func_Index_Data.pZwOpenFile != 0xB8 || (g_SSDT_Func_Index_Data.ZwOpenFileIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwOpenFile + 1), g_SSDT_Func_Index_Data.ZwOpenFileIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwOpenFileIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwTerminateProcess || *(UCHAR *)g_SSDT_Func_Index_Data.pZwTerminateProcess != 0xB8 || (g_SSDT_Func_Index_Data.ZwTerminateProcessIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwTerminateProcess + 1), g_SSDT_Func_Index_Data.ZwTerminateProcessIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwTerminateProcessIndex = g_SSDTServiceLimit;

	if (!g_SSDT_Func_Index_Data.pZwSetInformationThread || *(UCHAR *)g_SSDT_Func_Index_Data.pZwSetInformationThread != 0xB8 || (g_SSDT_Func_Index_Data.ZwSetInformationThreadIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwSetInformationThread + 1), g_SSDT_Func_Index_Data.ZwSetInformationThreadIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwSetInformationThreadIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwRequestWaitReplyPort || *(UCHAR *)g_SSDT_Func_Index_Data.pZwRequestWaitReplyPort != 0xB8 || (g_SSDT_Func_Index_Data.ZwRequestWaitReplyPortIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwRequestWaitReplyPort + 1), g_SSDT_Func_Index_Data.ZwRequestWaitReplyPortIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwRequestWaitReplyPortIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwCreateSection || *(UCHAR *)g_SSDT_Func_Index_Data.pZwCreateSection != 0xB8 || (g_SSDT_Func_Index_Data.ZwCreateSectionIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwCreateSection + 1), g_SSDT_Func_Index_Data.ZwCreateSectionIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwCreateSectionIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwOpenSection || *(UCHAR *)g_SSDT_Func_Index_Data.pZwOpenSection != 0xB8 || (g_SSDT_Func_Index_Data.ZwOpenSectionIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwOpenSection + 1), g_SSDT_Func_Index_Data.ZwOpenSectionIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwOpenSectionIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwCreateSymbolicLinkObject || *(UCHAR *)g_SSDT_Func_Index_Data.pZwCreateSymbolicLinkObject != 0xB8 || (g_SSDT_Func_Index_Data.ZwCreateSymbolicLinkObjectIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwCreateSymbolicLinkObject + 1), g_SSDT_Func_Index_Data.ZwCreateSymbolicLinkObjectIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwCreateSymbolicLinkObjectIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwOpenSymbolicLinkObject || *(UCHAR *)g_SSDT_Func_Index_Data.pZwOpenSymbolicLinkObject != 0xB8 || (g_SSDT_Func_Index_Data.ZwOpenSymbolicLinkObjectIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwOpenSymbolicLinkObject + 1), g_SSDT_Func_Index_Data.ZwOpenSymbolicLinkObjectIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwOpenSymbolicLinkObjectIndex = g_SSDTServiceLimit;


	if (!g_SSDT_Func_Index_Data.pZwLoadDriver || *(UCHAR *)g_SSDT_Func_Index_Data.pZwLoadDriver != 0xB8 || (g_SSDT_Func_Index_Data.ZwLoadDriverIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwLoadDriver + 1), g_SSDT_Func_Index_Data.ZwLoadDriverIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwLoadDriverIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwUnloadDriver || *(UCHAR *)g_SSDT_Func_Index_Data.pZwUnloadDriver != 0xB8 || (g_SSDT_Func_Index_Data.ZwUnloadDriverIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwUnloadDriver + 1), g_SSDT_Func_Index_Data.ZwUnloadDriverIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwUnloadDriverIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwQuerySystemInformation || *(UCHAR *)g_SSDT_Func_Index_Data.pZwQuerySystemInformation != 0xB8 || (g_SSDT_Func_Index_Data.ZwQuerySystemInformationIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwQuerySystemInformation + 1), g_SSDT_Func_Index_Data.ZwQuerySystemInformationIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwQuerySystemInformationIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwSetSystemInformation || *(UCHAR *)g_SSDT_Func_Index_Data.pZwSetSystemInformation != 0xB8 || (g_SSDT_Func_Index_Data.ZwSetSystemInformationIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwSetSystemInformation + 1), g_SSDT_Func_Index_Data.ZwSetSystemInformationIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwSetSystemInformationIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwSetSystemTime || *(UCHAR *)g_SSDT_Func_Index_Data.pZwSetSystemTime != 0xB8 || (g_SSDT_Func_Index_Data.ZwSetSystemTimeIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwSetSystemTime + 1), g_SSDT_Func_Index_Data.ZwSetSystemTimeIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwSetSystemTimeIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwOpenProcess || *(UCHAR *)g_SSDT_Func_Index_Data.pZwOpenProcess != 0xB8 || (g_SSDT_Func_Index_Data.ZwOpenProcessIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwOpenProcess + 1), g_SSDT_Func_Index_Data.ZwOpenProcessIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwOpenProcessIndex = g_SSDTServiceLimit;

	if (!g_SSDT_Func_Index_Data.pZwDeviceIoControlFile || *(UCHAR *)g_SSDT_Func_Index_Data.pZwDeviceIoControlFile != 0xB8 || (g_SSDT_Func_Index_Data.ZwDeviceIoControlFileIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwDeviceIoControlFile + 1), g_SSDT_Func_Index_Data.ZwDeviceIoControlFileIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwDeviceIoControlFileIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwOpenKey || *(UCHAR *)g_SSDT_Func_Index_Data.pZwOpenKey != 0xB8 || (g_SSDT_Func_Index_Data.ZwOpenKeyIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwOpenKey + 1), g_SSDT_Func_Index_Data.ZwOpenKeyIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwOpenKeyIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwOpenKeyEx || *(UCHAR *)g_SSDT_Func_Index_Data.pZwOpenKeyEx != 0xB8 || (g_SSDT_Func_Index_Data.ZwOpenKeyExIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwOpenKeyEx + 1), g_SSDT_Func_Index_Data.ZwOpenKeyExIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwOpenKeyExIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwDuplicateObject || *(UCHAR *)g_SSDT_Func_Index_Data.pZwDuplicateObject != 0xB8 || (g_SSDT_Func_Index_Data.ZwDuplicateObjectIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwDuplicateObject + 1), g_SSDT_Func_Index_Data.ZwDuplicateObjectIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwDuplicateObjectIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwFsControlFile || *(UCHAR *)g_SSDT_Func_Index_Data.pZwFsControlFile != 0xB8 || (g_SSDT_Func_Index_Data.ZwFsControlFileIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwFsControlFile + 1), g_SSDT_Func_Index_Data.ZwFsControlFileIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwFsControlFileIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwReplaceKey || *(UCHAR *)g_SSDT_Func_Index_Data.pZwReplaceKey != 0xB8 || (g_SSDT_Func_Index_Data.ZwReplaceKeyIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwReplaceKey + 1), g_SSDT_Func_Index_Data.ZwReplaceKeyIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwReplaceKeyIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwRestoreKey || *(UCHAR *)g_SSDT_Func_Index_Data.pZwRestoreKey != 0xB8 || (g_SSDT_Func_Index_Data.ZwRestoreKeyIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwRestoreKey + 1), g_SSDT_Func_Index_Data.ZwRestoreKeyIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwRestoreKeyIndex = g_SSDTServiceLimit;


	if (!g_SSDT_Func_Index_Data.pZwAdjustPrivilegesToken || *(UCHAR *)g_SSDT_Func_Index_Data.pZwAdjustPrivilegesToken != 0xB8 || (g_SSDT_Func_Index_Data.ZwAdjustPrivilegesTokenIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwAdjustPrivilegesToken + 1), g_SSDT_Func_Index_Data.ZwAdjustPrivilegesTokenIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwAdjustPrivilegesTokenIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwSetSecurityObject || *(UCHAR *)g_SSDT_Func_Index_Data.pZwSetSecurityObject != 0xB8 || (g_SSDT_Func_Index_Data.ZwSetSecurityObjectIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwSetSecurityObject + 1), g_SSDT_Func_Index_Data.ZwSetSecurityObjectIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwSetSecurityObjectIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwUnmapViewOfSection || *(UCHAR *)g_SSDT_Func_Index_Data.pZwUnmapViewOfSection != 0xB8 || (g_SSDT_Func_Index_Data.ZwUnmapViewOfSectionIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwUnmapViewOfSection + 1), g_SSDT_Func_Index_Data.ZwUnmapViewOfSectionIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwUnmapViewOfSectionIndex = g_SSDTServiceLimit;
	else
	{
		g_SSDT_Func_Index_Data.ZwUnmapViewOfSectionIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwUnmapViewOfSection + 1);
		if (MajorVersion == 6 && (MinorVersion == 2 || MinorVersion == 3) || MajorVersion == 10 && !MinorVersion)// Win8��Win10
			g_SSDT_Func_Index_Data.ZwUnmapViewOfSectionIndex_Win8_Win10 = g_SSDT_Func_Index_Data.ZwUnmapViewOfSectionIndex + 1;					//�����д���δ�˶ԣ�������������������

	}
	if (!g_SSDT_Func_Index_Data.pZwAllocateVirtualMemory || *(UCHAR *)g_SSDT_Func_Index_Data.pZwAllocateVirtualMemory != 0xB8 || (g_SSDT_Func_Index_Data.ZwAllocateVirtualMemoryIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwAllocateVirtualMemory + 1), g_SSDT_Func_Index_Data.ZwAllocateVirtualMemoryIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwAllocateVirtualMemoryIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwFreeVirtualMemory || *(UCHAR *)g_SSDT_Func_Index_Data.pZwFreeVirtualMemory != 0xB8 || (g_SSDT_Func_Index_Data.ZwFreeVirtualMemoryIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwFreeVirtualMemory + 1), g_SSDT_Func_Index_Data.ZwFreeVirtualMemoryIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwFreeVirtualMemoryIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwEnumerateValueKey || *(UCHAR *)g_SSDT_Func_Index_Data.pZwEnumerateValueKey != 0xB8 || (g_SSDT_Func_Index_Data.ZwEnumerateValueKeyIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwEnumerateValueKey + 1), g_SSDT_Func_Index_Data.ZwEnumerateValueKeyIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwEnumerateValueKeyIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwQueryKey || *(UCHAR *)g_SSDT_Func_Index_Data.pZwQueryKey != 0xB8 || (g_SSDT_Func_Index_Data.ZwQueryKeyIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwQueryKey + 1), g_SSDT_Func_Index_Data.ZwQueryKeyIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwQueryKeyIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwConnectPort || *(UCHAR *)g_SSDT_Func_Index_Data.pZwConnectPort != 0xB8 || (g_SSDT_Func_Index_Data.ZwConnectPortIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwConnectPort + 1), g_SSDT_Func_Index_Data.ZwConnectPortIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwConnectPortIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwSecureConnectPort)
	{
		if (MajorVersion == 5)                    // Win2K��WinXP��Win2003
		{
			if (!MinorVersion)                      // Win_2K
			{
				g_SSDT_Func_Index_Data.ZwSecureConnectPortIndex = 0xB8;
				goto LABEL_138;
			}
			if (MinorVersion == 1)                  // Win_XP
			{
				g_SSDT_Func_Index_Data.ZwSecureConnectPortIndex = 0xD2;
				goto LABEL_138;
			}
		}
	LABEL_137:
		g_SSDT_Func_Index_Data.ZwSecureConnectPortIndex = g_SSDTServiceLimit;
		goto LABEL_138;
	}
	if (*(UCHAR *)g_SSDT_Func_Index_Data.pZwSecureConnectPort != 0xB8u)
		goto LABEL_137;
	g_SSDT_Func_Index_Data.ZwSecureConnectPortIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwSecureConnectPort + 1);
	if ((ULONG)g_SSDT_Func_Index_Data.ZwSecureConnectPortIndex >= g_SSDTServiceLimit)
		goto LABEL_137;
LABEL_138:
	if (!g_SSDT_Func_Index_Data.pZwAlpcConnectPort || *(UCHAR *)g_SSDT_Func_Index_Data.pZwAlpcConnectPort != 0xB8 || (g_SSDT_Func_Index_Data.ZwAlpcConnectPortIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwAlpcConnectPort + 1), g_SSDT_Func_Index_Data.ZwAlpcConnectPortIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwAlpcConnectPortIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwAlpcConnectPortEx || *(UCHAR *)g_SSDT_Func_Index_Data.pZwAlpcConnectPortEx != 0xB8 || (g_SSDT_Func_Index_Data.ZwAlpcConnectPortExIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwAlpcConnectPortEx + 1), g_SSDT_Func_Index_Data.ZwAlpcConnectPortExIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwAlpcConnectPortExIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwSetTimer || *(UCHAR *)g_SSDT_Func_Index_Data.pZwSetTimer != 0xB8 || (g_SSDT_Func_Index_Data.ZwSetTimerIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwSetTimer + 1), g_SSDT_Func_Index_Data.ZwSetTimerIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwSetTimerIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwSetInformationProcess || *(UCHAR *)g_SSDT_Func_Index_Data.pZwSetInformationProcess != 0xB8 || (g_SSDT_Func_Index_Data.ZwSetInformationProcessIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwSetInformationProcess + 1), g_SSDT_Func_Index_Data.ZwSetInformationProcessIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwSetInformationProcessIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwMapViewOfSection || *(UCHAR *)g_SSDT_Func_Index_Data.pZwMapViewOfSection != 0xB8 || (g_SSDT_Func_Index_Data.ZwMapViewOfSectionIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwMapViewOfSection + 1), g_SSDT_Func_Index_Data.ZwMapViewOfSectionIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwMapViewOfSectionIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwMakeTemporaryObject || *(UCHAR *)g_SSDT_Func_Index_Data.pZwMakeTemporaryObject != 0xB8 || (g_SSDT_Func_Index_Data.ZwMakeTemporaryObjectIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwMakeTemporaryObject + 1), g_SSDT_Func_Index_Data.ZwMakeTemporaryObjectIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwMakeTemporaryObjectIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwDisplayString || *(UCHAR *)g_SSDT_Func_Index_Data.pZwDisplayString != 0xB8 || (g_SSDT_Func_Index_Data.ZwDisplayStringIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwDisplayString + 1), g_SSDT_Func_Index_Data.ZwDisplayStringIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwDisplayStringIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwAccessCheckAndAuditAlarm || *(UCHAR *)g_SSDT_Func_Index_Data.pZwAccessCheckAndAuditAlarm != 0xB8 || (g_SSDT_Func_Index_Data.ZwAccessCheckAndAuditAlarmIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwAccessCheckAndAuditAlarm + 1), g_SSDT_Func_Index_Data.ZwAccessCheckAndAuditAlarmIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwAccessCheckAndAuditAlarmIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwQueryInformationProcess || *(UCHAR *)g_SSDT_Func_Index_Data.pZwQueryInformationProcess != 0xB8 || (g_SSDT_Func_Index_Data.ZwQueryInformationProcessIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwQueryInformationProcess + 1), g_SSDT_Func_Index_Data.ZwQueryInformationProcessIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwQueryInformationProcessIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwQueryInformationThread || *(UCHAR *)g_SSDT_Func_Index_Data.pZwQueryInformationThread != 0xB8 || (g_SSDT_Func_Index_Data.ZwQueryInformationThreadIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwQueryInformationThread + 1), g_SSDT_Func_Index_Data.ZwQueryInformationThreadIndex >= g_SSDTServiceLimit))
		g_SSDT_Func_Index_Data.ZwQueryInformationThreadIndex = g_SSDTServiceLimit;
	if (!g_SSDT_Func_Index_Data.pZwLoadKey || *(UCHAR *)g_SSDT_Func_Index_Data.pZwLoadKey != 0xB8 || (g_SSDT_Func_Index_Data.ZwLoadKeyIndex = *(DWORD *)((PCHAR)g_SSDT_Func_Index_Data.pZwLoadKey + 1), g_SSDT_Func_Index_Data.ZwLoadKeyIndex >= g_SSDTServiceLimit))
	{
		g_SSDT_Func_Index_Data.ZwLoadKeyIndex = g_SSDTServiceLimit;
		g_SSDT_Func_Index_Data.ZwLoadKeyExIndex = g_SSDTServiceLimit;
		g_SSDT_Func_Index_Data.ZwLoadKey2Index = g_SSDTServiceLimit;
	}
	switch (MajorVersion)
	{
		//Win2K��WinXP��Win2003
	case 5:
	{
		//win2k
		if (MinorVersion == 0)
		{
			g_SSDT_Func_Index_Data.ZwLoadKey2Index = g_SSDT_Func_Index_Data.ZwLoadKeyIndex + 1;
			g_SSDT_Func_Index_Data.ZwRenameKeyIndex = g_SSDTServiceLimit;
			g_SSDT_Func_Index_Data.ZwCreateProcessIndex = 41;
			g_SSDT_Func_Index_Data.ZwCreateProcessExIndex = g_SSDTServiceLimit;
			g_SSDT_Func_Index_Data.ZwCreateUserProcessIndex = g_SSDTServiceLimit;
			g_SSDT_Func_Index_Data.ZwCreateThreadIndex = 46;
			g_SSDT_Func_Index_Data.ZwReadVirtualMemoryIndex = 164;
			g_SSDT_Func_Index_Data.ZwWriteVirtualMemoryIndex = 240;
			g_SSDT_Func_Index_Data.ZwQueueApcThreadIndex = 158;
			g_SSDT_Func_Index_Data.ZwSetContextThreadIndex = 186;
			g_SSDT_Func_Index_Data.ZwProtectVirtualMemoryIndex = 119;
			g_SSDT_Func_Index_Data.ZwAdjustGroupsTokenIndex = 9;
			g_SSDT_Func_Index_Data.ZwSystemDebugControlIndex = 222;
			g_ShadowSSDT_Func_Index_Data.ZwUserBuildHwndListIndex = 4398;
			g_ShadowSSDT_Func_Index_Data.ZwUserQueryWindowIndex = 4562;
			g_ShadowSSDT_Func_Index_Data.ZwUserFindWindowExIndex = 4464;
			g_ShadowSSDT_Func_Index_Data.ZwUserWindowFromPointIndex = 4664;
			g_ShadowSSDT_Func_Index_Data.ZwUserMessageCallIndex = 4540;
			g_ShadowSSDT_Func_Index_Data.ZwUserPostMessageIndex = 4555;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowsHookExIndex = 4626;
			g_ShadowSSDT_Func_Index_Data.ZwUserPostThreadMessageIndex = 4556;
			g_SSDT_Func_Index_Data.ZwWriteFileGatherIndex = 238;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetParentIndex = 4606;
			g_SSDT_Func_Index_Data.ZwResumeThreadIndex = 181;
			g_ShadowSSDT_Func_Index_Data.ZwUserChildWindowFromPointExIndex = 4415;
			g_ShadowSSDT_Func_Index_Data.ZwUserDestroyWindowIndex = 4441;
			g_ShadowSSDT_Func_Index_Data.ZwUserInternalGetWindowTextIndex = 4529;
			g_ShadowSSDT_Func_Index_Data.ZwUserMoveWindowIndex = 4545;
			g_ShadowSSDT_Func_Index_Data.ZwUserRealChildWindowFromPointIndex = 4563;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetInformationThreadIndex = 4597;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetInternalWindowPosIndex = 4598;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowLongIndex = 4621;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowPlacementIndex = 4622;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowPosIndex = 4623;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowRgnIndex = 4624;
			g_ShadowSSDT_Func_Index_Data.ZwUserShowWindowIndex = 4632;
			g_ShadowSSDT_Func_Index_Data.ZwUserShowWindowAsyncIndex = 4633;
			g_ShadowSSDT_Func_Index_Data.ZwUserSendInputIndex = 4577;
			g_SSDT_Func_Index_Data.ZwQueryAttributesFileIndex = 122;
			g_SSDT_Func_Index_Data.ZwCreateThreadExIndex = g_SSDTServiceLimit;
			g_SSDT_Func_Index_Data.ZwAlpcSendWaitReceivePortIndex = g_SSDTServiceLimit;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWinEventHookIndex = 4629;
			g_ShadowSSDT_Func_Index_Data.ZwUserCallHwndParamLockIndex = 4405;
			g_ShadowSSDT_Func_Index_Data.ZwUserRegisterUserApiHookIndex = g_SSDTServiceLimit;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_ClientLoadLibrary_Index = 64;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_ClientImmLoadLayout_Index = 80;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_fnHkOPTINLPEVENTMSG_Index = 48;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_fnHkINLPKBDLLHOOKSTRUCT_Index = 44;
			g_ShadowSSDT_Func_Index_Data.NtUserRegisterWindowMessageIndex = 4568;
			g_ShadowSSDT_Func_Index_Data.NtUserCallNoParamIndex = 4408;
			g_ShadowSSDT_Func_Index_Data.NtUserCallOneParamIndex = 4409;
			g_SSDT_Func_Index_Data.ZwCreateMutantIndex = 37;
			g_SSDT_Func_Index_Data.ZwOpenMutantIndex = 104;
			g_SSDT_Func_Index_Data.ZwVdmControlIndex = 232;
			g_SSDT_Func_Index_Data.ZwGetNextThreadIndex = g_SSDTServiceLimit;
			g_SSDT_Func_Index_Data.ZwGetNextProcessIndex = g_SSDTServiceLimit;
			g_SSDT_Func_Index_Data.ZwRequestPortIndex = 175;
			g_ShadowSSDT_Func_Index_Data.NtUserCallTwoParamIndex = 4410;
			g_ShadowSSDT_Func_Index_Data.NtUserCallHwndLockIndex = 4402;
			g_ShadowSSDT_Func_Index_Data.NtUserUnhookWindowsHookExIndex = 4645;
			g_ShadowSSDT_Func_Index_Data.NtUserClipCursorIndex = 4416;
			g_ShadowSSDT_Func_Index_Data.NtUserGetKeyboardStateIndex = 4499;
			g_ShadowSSDT_Func_Index_Data.NtUserGetKeyStateIndex = 4501;
			g_ShadowSSDT_Func_Index_Data.NtUserGetAsyncKeyStateIndex = 4469;
			g_ShadowSSDT_Func_Index_Data.NtUserAttachThreadInputIndex = 4393;
			g_ShadowSSDT_Func_Index_Data.NtUserRegisterHotKeyIndex = 4566;
			g_ShadowSSDT_Func_Index_Data.NtUserRegisterRawInputDevicesIndex = g_SSDTServiceLimit;
			g_ShadowSSDT_Func_Index_Data.NtGdiStretchBltIndex = 4378;
			g_ShadowSSDT_Func_Index_Data.NtGdiMaskBltIndex = 4315;
			g_ShadowSSDT_Func_Index_Data.NtGdiPlgBltIndex = 4325;
			g_ShadowSSDT_Func_Index_Data.NtGdiTransparentBltIndex = 4384;
			g_ShadowSSDT_Func_Index_Data.NtGdiGetPixelIndex = 4283;
			g_SSDT_Func_Index_Data.ZwTerminateThreadIndex = 225;
			g_SSDT_Func_Index_Data.ZwTerminateJobObjectIndex = 223;
			g_SSDT_Func_Index_Data.ZwDebugActiveProcessIndex = g_SSDTServiceLimit;
			g_SSDT_Func_Index_Data.ZwAssignProcessToJobObjectIndex = 18;
			g_SSDT_Func_Index_Data.ZwSetInformationJobObjectIndex = 195;
			g_ShadowSSDT_Func_Index_Data.NtUserGetRawInputDataIndex = g_SSDTServiceLimit;
			g_ShadowSSDT_Func_Index_Data.NtUserGetRawInputBufferIndex = g_SSDTServiceLimit;
			g_ShadowSSDT_Func_Index_Data.NtUserSetImeInfoExIndex = 4594;
			g_ShadowSSDT_Func_Index_Data.NtGdiOpenDCWIndex = 4321;
			g_ShadowSSDT_Func_Index_Data.NtGdiDeleteObjectAppIndex = 4213;
			g_ShadowSSDT_Func_Index_Data.NtUserBlockInputIndex = 4396;
			g_SSDT_Func_Index_Data.ZwContinueIndex = 28;
			g_SSDT_Func_Index_Data.ZwQueryIntervalProfileIndex = 138;
			g_SSDT_Func_Index_Data.ZwSetIntervalProfileIndex = 201;
			g_SSDT_Func_Index_Data.ZwCreateProfileIndex = 42;
			g_ShadowSSDT_Func_Index_Data.NtUserLoadKeyboardLayoutExIndex = 4534;
			g_SSDT_Func_Index_Data.ZwSuspendThreadIndex = 221;
			g_SSDT_Func_Index_Data.ZwSuspendProcessIndex = g_SSDTServiceLimit;
		}
		//winXP
		else if (MinorVersion == 1)
		{
			g_SSDT_Func_Index_Data.ZwLoadKey2Index = g_SSDT_Func_Index_Data.ZwLoadKeyIndex + 1;
			g_SSDT_Func_Index_Data.ZwRenameKeyIndex = 192;
			g_SSDT_Func_Index_Data.ZwCreateProcessIndex = 47;
			g_SSDT_Func_Index_Data.ZwCreateProcessExIndex = 48;
			g_SSDT_Func_Index_Data.ZwCreateUserProcessIndex = g_SSDTServiceLimit;
			g_SSDT_Func_Index_Data.ZwCreateThreadIndex = 53;
			g_SSDT_Func_Index_Data.ZwReadVirtualMemoryIndex = 186;
			g_SSDT_Func_Index_Data.ZwWriteVirtualMemoryIndex = 277;
			g_SSDT_Func_Index_Data.ZwQueueApcThreadIndex = 180;
			g_SSDT_Func_Index_Data.ZwSetContextThreadIndex = 213;
			g_SSDT_Func_Index_Data.ZwProtectVirtualMemoryIndex = 137;
			g_SSDT_Func_Index_Data.ZwAdjustGroupsTokenIndex = 10;
			g_SSDT_Func_Index_Data.ZwSystemDebugControlIndex = 255;
			g_ShadowSSDT_Func_Index_Data.ZwUserBuildHwndListIndex = 4408;
			g_ShadowSSDT_Func_Index_Data.ZwUserQueryWindowIndex = 4579;
			g_ShadowSSDT_Func_Index_Data.ZwUserFindWindowExIndex = 4474;
			g_ShadowSSDT_Func_Index_Data.ZwUserWindowFromPointIndex = 4688;
			g_ShadowSSDT_Func_Index_Data.ZwUserMessageCallIndex = 4556;
			g_ShadowSSDT_Func_Index_Data.ZwUserPostMessageIndex = 4571;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowsHookExIndex = 4645;
			g_ShadowSSDT_Func_Index_Data.ZwUserPostThreadMessageIndex = 4572;
			g_SSDT_Func_Index_Data.ZwWriteFileGatherIndex = 275;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetParentIndex = 4625;
			g_SSDT_Func_Index_Data.ZwResumeThreadIndex = 206;
			g_ShadowSSDT_Func_Index_Data.ZwUserChildWindowFromPointExIndex = 4425;
			g_ShadowSSDT_Func_Index_Data.ZwUserDestroyWindowIndex = 4451;
			g_ShadowSSDT_Func_Index_Data.ZwUserInternalGetWindowTextIndex = 4545;
			g_ShadowSSDT_Func_Index_Data.ZwUserMoveWindowIndex = 4561;
			g_ShadowSSDT_Func_Index_Data.ZwUserRealChildWindowFromPointIndex = 4580;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetInformationThreadIndex = 4616;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetInternalWindowPosIndex = 4617;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowLongIndex = 4640;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowPlacementIndex = 4641;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowPosIndex = 4642;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowRgnIndex = 4643;
			g_ShadowSSDT_Func_Index_Data.ZwUserShowWindowIndex = 4651;
			g_ShadowSSDT_Func_Index_Data.ZwUserShowWindowAsyncIndex = 4652;
			g_ShadowSSDT_Func_Index_Data.ZwUserSendInputIndex = 4598;
			g_SSDT_Func_Index_Data.ZwQueryAttributesFileIndex = 139;
			g_SSDT_Func_Index_Data.ZwCreateThreadExIndex = g_SSDTServiceLimit;
			g_SSDT_Func_Index_Data.ZwAlpcSendWaitReceivePortIndex = g_SSDTServiceLimit;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWinEventHookIndex = 4648;
			g_ShadowSSDT_Func_Index_Data.ZwUserCallHwndParamLockIndex = 4415;
			g_ShadowSSDT_Func_Index_Data.ZwUserRegisterUserApiHookIndex = 4585;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_ClientLoadLibrary_Index = 66;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_ClientImmLoadLayout_Index = 84;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_fnHkOPTINLPEVENTMSG_Index = 49;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_fnHkINLPKBDLLHOOKSTRUCT_Index = 45;
			g_ShadowSSDT_Func_Index_Data.NtUserRegisterWindowMessageIndex = 4589;
			g_ShadowSSDT_Func_Index_Data.NtUserCallNoParamIndex = 4418;
			g_ShadowSSDT_Func_Index_Data.NtUserCallOneParamIndex = 4419;
			g_SSDT_Func_Index_Data.ZwCreateMutantIndex = 43;
			g_SSDT_Func_Index_Data.ZwOpenMutantIndex = 120;
			g_SSDT_Func_Index_Data.ZwVdmControlIndex = 268;
			g_SSDT_Func_Index_Data.ZwGetNextThreadIndex = g_SSDTServiceLimit;
			g_SSDT_Func_Index_Data.ZwGetNextProcessIndex = g_SSDTServiceLimit;
			g_SSDT_Func_Index_Data.ZwRequestPortIndex = 199;
			g_ShadowSSDT_Func_Index_Data.NtUserCallTwoParamIndex = 4420;
			g_ShadowSSDT_Func_Index_Data.NtUserCallHwndLockIndex = 4412;
			g_ShadowSSDT_Func_Index_Data.NtUserUnhookWindowsHookExIndex = 4666;
			g_ShadowSSDT_Func_Index_Data.NtUserClipCursorIndex = 4426;
			g_ShadowSSDT_Func_Index_Data.NtUserGetKeyboardStateIndex = 4510;
			g_ShadowSSDT_Func_Index_Data.NtUserGetKeyStateIndex = 4512;
			g_ShadowSSDT_Func_Index_Data.NtUserGetAsyncKeyStateIndex = 4479;
			g_ShadowSSDT_Func_Index_Data.NtUserAttachThreadInputIndex = 4403;
			g_ShadowSSDT_Func_Index_Data.NtUserRegisterHotKeyIndex = 4586;
			g_ShadowSSDT_Func_Index_Data.NtUserRegisterRawInputDevicesIndex = 4587;
			g_ShadowSSDT_Func_Index_Data.NtGdiStretchBltIndex = 4388;
			g_ShadowSSDT_Func_Index_Data.NtGdiMaskBltIndex = 4323;
			g_ShadowSSDT_Func_Index_Data.NtGdiPlgBltIndex = 4333;
			g_ShadowSSDT_Func_Index_Data.NtGdiTransparentBltIndex = 4394;
			g_ShadowSSDT_Func_Index_Data.NtGdiGetPixelIndex = 4287;
			g_SSDT_Func_Index_Data.ZwTerminateThreadIndex = 258;
			g_SSDT_Func_Index_Data.ZwTerminateJobObjectIndex = 256;
			g_SSDT_Func_Index_Data.ZwDebugActiveProcessIndex = 57;
			g_SSDT_Func_Index_Data.ZwAssignProcessToJobObjectIndex = 19;
			g_SSDT_Func_Index_Data.ZwSetInformationJobObjectIndex = 225;
			g_ShadowSSDT_Func_Index_Data.NtUserGetRawInputBufferIndex = 4523;
			g_ShadowSSDT_Func_Index_Data.NtUserGetRawInputDataIndex = 4524;
			g_ShadowSSDT_Func_Index_Data.NtUserSetImeInfoExIndex = 4613;
			g_ShadowSSDT_Func_Index_Data.NtGdiOpenDCWIndex = 4329;
			g_ShadowSSDT_Func_Index_Data.NtGdiDeleteObjectAppIndex = 4218;
			g_ShadowSSDT_Func_Index_Data.NtUserBlockInputIndex = 4406;
			g_SSDT_Func_Index_Data.ZwContinueIndex = 32;
			g_SSDT_Func_Index_Data.ZwQueryIntervalProfileIndex = 158;
			g_SSDT_Func_Index_Data.ZwSetIntervalProfileIndex = 231;
			g_SSDT_Func_Index_Data.ZwCreateProfileIndex = 49;
			g_ShadowSSDT_Func_Index_Data.NtUserLoadKeyboardLayoutExIndex = 4550;
			g_SSDT_Func_Index_Data.ZwSuspendThreadIndex = 254;
			g_SSDT_Func_Index_Data.ZwSuspendProcessIndex = 253;
		}
		//win2003δʵ��
		else if (MinorVersion == 2)
		{
			KdPrint(("Win2003δ֧��\t\n"));
			return FALSE;
		}
		g_ShadowSSDT_Func_Index_Data.NtGdiBitBltIndex = 4109;
		g_ShadowSSDT_Func_Index_Data.NtGdiAlphaBlendIndex = 4103;
		g_ShadowSSDT_Func_Index_Data.NtGdiAddFontResourceWIndex = 4098;
		g_ShadowSSDT_Func_Index_Data.NtGdiAddFontMemResourceExIndex = 4100;
		g_ShadowSSDT_Func_Index_Data.NtGdiAddRemoteFontToDCIndex = 4099;
		g_SSDT_Func_Index_Data.ZwQueueApcThreadExIndex = g_SSDTServiceLimit;
		g_SSDT_Func_Index_Data.ZwApphelpCaCheControlIndex = g_SSDTServiceLimit;
		g_SSDT_Func_Index_Data.ZwAccessCheckIndex = g_SSDT_Func_Index_Data.ZwAccessCheckAndAuditAlarmIndex - 1;	//�������������
		return TRUE;
	}
	//VISTA��Win7��Win8
	case 6:
	{
		if (MinorVersion == 0 || MinorVersion == 1)					//�������������
		{
			g_SSDT_Func_Index_Data.ZwLoadKey2Index = g_SSDT_Func_Index_Data.ZwLoadKeyIndex + 1;
			g_SSDT_Func_Index_Data.ZwLoadKeyExIndex = g_SSDT_Func_Index_Data.ZwLoadKeyIndex + 2;
		}
		if (MinorVersion == 2 && MinorVersion == 3)					//�������������
		{
			g_SSDT_Func_Index_Data.ZwLoadKey2Index = g_SSDT_Func_Index_Data.ZwLoadKeyIndex - 1;
			g_SSDT_Func_Index_Data.ZwLoadKeyExIndex = g_SSDT_Func_Index_Data.ZwLoadKeyIndex - 2;
		}
		//VISTA
		if (MinorVersion == 0)
		{
			g_SSDT_Func_Index_Data.ZwRenameKeyIndex = 267;
			g_SSDT_Func_Index_Data.ZwCreateProcessIndex = 72;
			g_SSDT_Func_Index_Data.ZwCreateProcessExIndex = 73;
			if (BuildNumber == 6001 || BuildNumber == 6002 || BuildNumber == 6003)
			{
				g_SSDT_Func_Index_Data.ZwCreateUserProcessIndex = 383;
				g_SSDT_Func_Index_Data.ZwWriteVirtualMemoryIndex = 358;
				g_SSDT_Func_Index_Data.ZwSetContextThreadIndex = 289;
				g_SSDT_Func_Index_Data.ZwWriteFileGatherIndex = 356;
				g_SSDT_Func_Index_Data.ZwResumeThreadIndex = 282;
				g_SSDT_Func_Index_Data.ZwCreateThreadExIndex = 382;
				g_SSDT_Func_Index_Data.ZwSystemDebugControlIndex = 332;
				g_SSDT_Func_Index_Data.ZwVdmControlIndex = 349;
				g_SSDT_Func_Index_Data.ZwGetNextThreadIndex = 368;
				g_SSDT_Func_Index_Data.ZwGetNextProcessIndex = 367;
				g_SSDT_Func_Index_Data.ZwRequestPortIndex = 275;
				g_SSDT_Func_Index_Data.ZwTerminateThreadIndex = 335;
				g_SSDT_Func_Index_Data.ZwTerminateJobObjectIndex = 333;
				g_SSDT_Func_Index_Data.ZwSetInformationJobObjectIndex = 302;
				g_SSDT_Func_Index_Data.ZwSetIntervalProfileIndex = 308;
				g_SSDT_Func_Index_Data.ZwSuspendThreadIndex = 331;
				g_SSDT_Func_Index_Data.ZwSuspendProcessIndex = 330;
			}
			else
			{
				if (BuildNumber != 6000)
				{
					return FALSE;
				}
				g_SSDT_Func_Index_Data.ZwCreateUserProcessIndex = 389;
				g_SSDT_Func_Index_Data.ZwWriteVirtualMemoryIndex = 362;
				g_SSDT_Func_Index_Data.ZwSetContextThreadIndex = 293;
				g_SSDT_Func_Index_Data.ZwWriteFileGatherIndex = 370;
				g_SSDT_Func_Index_Data.ZwResumeThreadIndex = 281;
				g_SSDT_Func_Index_Data.ZwCreateThreadExIndex = 388;
				g_SSDT_Func_Index_Data.ZwSystemDebugControlIndex = 336;
				g_SSDT_Func_Index_Data.ZwVdmControlIndex = 353;
				g_SSDT_Func_Index_Data.ZwGetNextThreadIndex = 372;
				g_SSDT_Func_Index_Data.ZwGetNextProcessIndex = 371;
				g_SSDT_Func_Index_Data.ZwRequestPortIndex = 274;
				g_SSDT_Func_Index_Data.ZwTerminateThreadIndex = 339;
				g_SSDT_Func_Index_Data.ZwTerminateJobObjectIndex = 337;
				g_SSDT_Func_Index_Data.ZwSetInformationJobObjectIndex = 306;
				g_SSDT_Func_Index_Data.ZwSetIntervalProfileIndex = 312;
				g_SSDT_Func_Index_Data.ZwSuspendThreadIndex = 335;
				g_SSDT_Func_Index_Data.ZwSuspendProcessIndex = 334;
			}
			g_SSDT_Func_Index_Data.ZwApphelpCaCheControlIndex = g_SSDTServiceLimit;
			g_SSDT_Func_Index_Data.ZwCreateThreadIndex = 78;
			g_SSDT_Func_Index_Data.ZwCreateMutantIndex = 67;
			g_SSDT_Func_Index_Data.ZwOpenMutantIndex = 191;
			g_SSDT_Func_Index_Data.ZwReadVirtualMemoryIndex = 261;
			g_SSDT_Func_Index_Data.ZwQueueApcThreadIndex = 255;
			g_SSDT_Func_Index_Data.ZwProtectVirtualMemoryIndex = 210;
			g_SSDT_Func_Index_Data.ZwAdjustGroupsTokenIndex = 11;
			g_ShadowSSDT_Func_Index_Data.ZwUserBuildHwndListIndex = 4418;
			g_ShadowSSDT_Func_Index_Data.ZwUserQueryWindowIndex = 4600;
			g_ShadowSSDT_Func_Index_Data.ZwUserFindWindowExIndex = 4487;
			g_ShadowSSDT_Func_Index_Data.ZwUserWindowFromPointIndex = 4713;
			g_ShadowSSDT_Func_Index_Data.ZwUserMessageCallIndex = 4575;
			g_ShadowSSDT_Func_Index_Data.ZwUserPostMessageIndex = 4593;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowsHookExIndex = 4669;
			g_ShadowSSDT_Func_Index_Data.ZwUserPostThreadMessageIndex = 4594;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetParentIndex = 4646;
			g_ShadowSSDT_Func_Index_Data.ZwUserChildWindowFromPointExIndex = 4438;
			g_ShadowSSDT_Func_Index_Data.ZwUserDestroyWindowIndex = 4462;
			g_ShadowSSDT_Func_Index_Data.ZwUserInternalGetWindowTextIndex = 4562;
			g_ShadowSSDT_Func_Index_Data.ZwUserMoveWindowIndex = 4580;
			g_ShadowSSDT_Func_Index_Data.ZwUserRealChildWindowFromPointIndex = 4601;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetInformationThreadIndex = 4638;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetInternalWindowPosIndex = 4639;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowLongIndex = 4662;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowPlacementIndex = 4663;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowPosIndex = 4664;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowRgnIndex = 4665;
			g_ShadowSSDT_Func_Index_Data.ZwUserShowWindowIndex = 4675;
			g_ShadowSSDT_Func_Index_Data.ZwUserShowWindowAsyncIndex = 4676;
			g_ShadowSSDT_Func_Index_Data.ZwUserSendInputIndex = 4621;
			g_SSDT_Func_Index_Data.ZwQueryAttributesFileIndex = 212;
			g_SSDT_Func_Index_Data.ZwAlpcSendWaitReceivePortIndex = 38;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWinEventHookIndex = 4672;
			g_ShadowSSDT_Func_Index_Data.ZwUserCallHwndParamLockIndex = 4425;
			g_ShadowSSDT_Func_Index_Data.ZwUserRegisterUserApiHookIndex = 4607;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_ClientLoadLibrary_Index = 66;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_ClientImmLoadLayout_Index = 84;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_fnHkOPTINLPEVENTMSG_Index = 49;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_fnHkINLPKBDLLHOOKSTRUCT_Index = 45;
			g_ShadowSSDT_Func_Index_Data.NtUserRegisterWindowMessageIndex = 4611;
			g_ShadowSSDT_Func_Index_Data.NtUserCallNoParamIndex = 4428;
			g_ShadowSSDT_Func_Index_Data.NtUserCallOneParamIndex = 4429;
			g_ShadowSSDT_Func_Index_Data.NtUserCallTwoParamIndex = 4430;
			g_ShadowSSDT_Func_Index_Data.NtUserCallHwndLockIndex = 4422;
			g_ShadowSSDT_Func_Index_Data.NtUserUnhookWindowsHookExIndex = 4690;
			g_ShadowSSDT_Func_Index_Data.NtUserClipCursorIndex = 4439;
			g_ShadowSSDT_Func_Index_Data.NtUserGetKeyboardStateIndex = 4524;
			g_ShadowSSDT_Func_Index_Data.NtUserGetKeyStateIndex = 4526;
			g_ShadowSSDT_Func_Index_Data.NtUserGetAsyncKeyStateIndex = 4493;
			g_ShadowSSDT_Func_Index_Data.NtUserAttachThreadInputIndex = 4413;
			g_ShadowSSDT_Func_Index_Data.NtUserRegisterHotKeyIndex = 4608;
			g_ShadowSSDT_Func_Index_Data.NtUserRegisterRawInputDevicesIndex = 4609;
			g_ShadowSSDT_Func_Index_Data.NtGdiBitBltIndex = 4109;
			g_ShadowSSDT_Func_Index_Data.NtGdiStretchBltIndex = 4397;
			g_ShadowSSDT_Func_Index_Data.NtGdiMaskBltIndex = 4331;
			g_ShadowSSDT_Func_Index_Data.NtGdiPlgBltIndex = 4341;
			g_ShadowSSDT_Func_Index_Data.NtGdiTransparentBltIndex = 4403;
			g_ShadowSSDT_Func_Index_Data.NtGdiAlphaBlendIndex = 4103;
			g_ShadowSSDT_Func_Index_Data.NtGdiGetPixelIndex = 4294;
			g_SSDT_Func_Index_Data.ZwDebugActiveProcessIndex = 116;
			g_SSDT_Func_Index_Data.ZwAssignProcessToJobObjectIndex = 42;
			g_ShadowSSDT_Func_Index_Data.NtUserGetRawInputBufferIndex = 4537;
			g_ShadowSSDT_Func_Index_Data.NtUserGetRawInputDataIndex = 4538;
			g_ShadowSSDT_Func_Index_Data.NtUserSetImeInfoExIndex = 4635;
			g_SSDT_Func_Index_Data.ZwQueueApcThreadExIndex = g_SSDTServiceLimit;
			g_ShadowSSDT_Func_Index_Data.NtGdiOpenDCWIndex = 4337;
			g_ShadowSSDT_Func_Index_Data.NtGdiDeleteObjectAppIndex = 4220;
			g_ShadowSSDT_Func_Index_Data.NtUserBlockInputIndex = 4416;
			g_SSDT_Func_Index_Data.ZwContinueIndex = 55;
			g_SSDT_Func_Index_Data.ZwQueryIntervalProfileIndex = 232;
			g_SSDT_Func_Index_Data.ZwCreateProfileIndex = 74;
			g_ShadowSSDT_Func_Index_Data.NtUserLoadKeyboardLayoutExIndex = 4568;
			g_ShadowSSDT_Func_Index_Data.NtGdiAddFontResourceWIndex = 4098;
			g_ShadowSSDT_Func_Index_Data.NtGdiAddFontMemResourceExIndex = 4100;
			g_ShadowSSDT_Func_Index_Data.NtGdiAddRemoteFontToDCIndex = 4099;
			g_SSDT_Func_Index_Data.ZwAccessCheckIndex = g_SSDT_Func_Index_Data.ZwAccessCheckAndAuditAlarmIndex - 1;	//�������������
			return TRUE;
		}
		//Win7
		if (MinorVersion == 1)
		{
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_ClientLoadLibrary_Index = 65;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_ClientImmLoadLayout_Index = 82;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_fnHkOPTINLPEVENTMSG_Index = 49;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_fnHkINLPKBDLLHOOKSTRUCT_Index = 45;
			if (BuildNumber == 7600 || BuildNumber == 7601)// Win7
			{
				g_SSDT_Func_Index_Data.ZwRenameKeyIndex = 290;
				g_SSDT_Func_Index_Data.ZwCreateProcessIndex = 79;
				g_SSDT_Func_Index_Data.ZwCreateProcessExIndex = 80;
				g_SSDT_Func_Index_Data.ZwCreateUserProcessIndex = 93;
				g_SSDT_Func_Index_Data.ZwCreateThreadIndex = 87;
				g_SSDT_Func_Index_Data.ZwReadVirtualMemoryIndex = 277;
				g_SSDT_Func_Index_Data.ZwWriteVirtualMemoryIndex = 399;
				g_SSDT_Func_Index_Data.ZwQueueApcThreadIndex = 269;
				g_SSDT_Func_Index_Data.ZwSetContextThreadIndex = 316;
				g_SSDT_Func_Index_Data.ZwProtectVirtualMemoryIndex = 215;
				g_SSDT_Func_Index_Data.ZwAdjustGroupsTokenIndex = 11;
				g_SSDT_Func_Index_Data.ZwSystemDebugControlIndex = 368;
				g_ShadowSSDT_Func_Index_Data.ZwUserBuildHwndListIndex = 4419;
				g_ShadowSSDT_Func_Index_Data.ZwUserQueryWindowIndex = 4611;
				g_ShadowSSDT_Func_Index_Data.ZwUserFindWindowExIndex = 4492;
				g_ShadowSSDT_Func_Index_Data.ZwUserWindowFromPointIndex = 4725;
				g_ShadowSSDT_Func_Index_Data.ZwUserMessageCallIndex = 4586;
				g_ShadowSSDT_Func_Index_Data.ZwUserPostMessageIndex = 4604;
				g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowsHookExIndex = 4681;
				g_ShadowSSDT_Func_Index_Data.ZwUserPostThreadMessageIndex = 4605;
				g_SSDT_Func_Index_Data.ZwWriteFileGatherIndex = 397;
				g_ShadowSSDT_Func_Index_Data.ZwUserSetParentIndex = 4656;
				g_SSDT_Func_Index_Data.ZwResumeThreadIndex = 304;
				g_ShadowSSDT_Func_Index_Data.ZwUserChildWindowFromPointExIndex = 4443;
				g_ShadowSSDT_Func_Index_Data.ZwUserDestroyWindowIndex = 4467;
				g_ShadowSSDT_Func_Index_Data.ZwUserInternalGetWindowTextIndex = 4572;
				g_ShadowSSDT_Func_Index_Data.ZwUserMoveWindowIndex = 4591;
				g_ShadowSSDT_Func_Index_Data.ZwUserRealChildWindowFromPointIndex = 4612;
				g_ShadowSSDT_Func_Index_Data.ZwUserSetInformationThreadIndex = 4648;
				g_ShadowSSDT_Func_Index_Data.ZwUserSetInternalWindowPosIndex = 4649;
				g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowLongIndex = 4674;
				g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowPlacementIndex = 4675;
				g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowPosIndex = 4676;
				g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowRgnIndex = 4677;
				g_ShadowSSDT_Func_Index_Data.ZwUserShowWindowIndex = 4687;
				g_ShadowSSDT_Func_Index_Data.ZwUserShowWindowAsyncIndex = 4688;
				g_ShadowSSDT_Func_Index_Data.ZwUserSendInputIndex = 4632;
				g_SSDT_Func_Index_Data.ZwQueryAttributesFileIndex = 217;
				g_SSDT_Func_Index_Data.ZwCreateThreadExIndex = 88;
				g_SSDT_Func_Index_Data.ZwAlpcSendWaitReceivePortIndex = 39;
				g_ShadowSSDT_Func_Index_Data.ZwUserSetWinEventHookIndex = 4684;
				g_SSDT_Func_Index_Data.ZwTerminateProcessIndex = 370;
				g_ShadowSSDT_Func_Index_Data.ZwUserCallHwndParamLockIndex = 4426;
				g_ShadowSSDT_Func_Index_Data.ZwUserRegisterUserApiHookIndex = 4618;
				g_ShadowSSDT_Func_Index_Data.NtUserRegisterWindowMessageIndex = 4623;		//g_ShadowSSDT_Func_Index_Data.NtUserRegisterWindowMessageIndex
				g_ShadowSSDT_Func_Index_Data.NtUserCallNoParamIndex = 4429;					//g_ShadowSSDT_Func_Index_Data.NtUserCallNoParamIndex
				g_ShadowSSDT_Func_Index_Data.NtUserCallOneParamIndex = 4430;					//g_ShadowSSDT_Func_Index_Data.NtUserCallOneParamIndex
				g_ShadowSSDT_Func_Index_Data.NtUserCallTwoParamIndex = 4431;					//g_ShadowSSDT_Func_Index_Data.NtUserCallTwoParamIndex
				g_SSDT_Func_Index_Data.ZwCreateMutantIndex = 74;
				g_SSDT_Func_Index_Data.ZwOpenMutantIndex = 187;
				g_SSDT_Func_Index_Data.ZwVdmControlIndex = 386;
				g_SSDT_Func_Index_Data.ZwGetNextThreadIndex = 140;
				g_SSDT_Func_Index_Data.ZwGetNextProcessIndex = 139;
				g_SSDT_Func_Index_Data.ZwRequestPortIndex = 298;
				g_ShadowSSDT_Func_Index_Data.NtUserCallHwndLockIndex = 4423;					//g_ShadowSSDT_Func_Index_Data.NtUserCallHwndLockIndex
				g_ShadowSSDT_Func_Index_Data.NtUserUnhookWindowsHookExIndex = 4703;			//g_ShadowSSDT_Func_Index_Data.NtUserUnhookWindowsHookExIndex
				g_ShadowSSDT_Func_Index_Data.NtUserClipCursorIndex = 4444;					//g_ShadowSSDT_Func_Index_Data.NtUserClipCursorIndex
				g_ShadowSSDT_Func_Index_Data.NtUserGetKeyboardStateIndex = 4530;				//g_ShadowSSDT_Func_Index_Data.NtUserGetKeyboardStateIndex
				g_ShadowSSDT_Func_Index_Data.NtUserGetKeyStateIndex = 4532;
				g_ShadowSSDT_Func_Index_Data.NtUserGetAsyncKeyStateIndex = 4498;
				g_ShadowSSDT_Func_Index_Data.NtUserAttachThreadInputIndex = 4414;
				g_ShadowSSDT_Func_Index_Data.NtUserRegisterHotKeyIndex = 4619;
				g_ShadowSSDT_Func_Index_Data.NtUserRegisterRawInputDevicesIndex = 4620;
				g_ShadowSSDT_Func_Index_Data.NtGdiBitBltIndex = 4110;
				g_ShadowSSDT_Func_Index_Data.NtGdiStretchBltIndex = 4398;
				g_ShadowSSDT_Func_Index_Data.NtGdiMaskBltIndex = 4333;
				g_ShadowSSDT_Func_Index_Data.NtGdiPlgBltIndex = 4343;
				g_ShadowSSDT_Func_Index_Data.NtGdiTransparentBltIndex = 4404;
				g_ShadowSSDT_Func_Index_Data.NtGdiAlphaBlendIndex = 4103;
				g_ShadowSSDT_Func_Index_Data.NtGdiGetPixelIndex = 4296;
				g_SSDT_Func_Index_Data.ZwTerminateThreadIndex = 371;
				g_SSDT_Func_Index_Data.ZwTerminateJobObjectIndex = 369;
				g_SSDT_Func_Index_Data.ZwDebugActiveProcessIndex = 96;
				g_SSDT_Func_Index_Data.ZwAssignProcessToJobObjectIndex = 43;
				g_SSDT_Func_Index_Data.ZwSetInformationJobObjectIndex = 330;
				g_ShadowSSDT_Func_Index_Data.NtUserGetRawInputBufferIndex = 4543;
				g_ShadowSSDT_Func_Index_Data.NtUserGetRawInputDataIndex = 4544;
				g_ShadowSSDT_Func_Index_Data.NtUserSetImeInfoExIndex = 4646;
				g_SSDT_Func_Index_Data.ZwQueueApcThreadExIndex = 270;
				g_ShadowSSDT_Func_Index_Data.NtGdiOpenDCWIndex = 4339;
				g_ShadowSSDT_Func_Index_Data.NtGdiDeleteObjectAppIndex = 4221;
				g_ShadowSSDT_Func_Index_Data.NtUserBlockInputIndex = 4417;
				g_SSDT_Func_Index_Data.ZwContinueIndex = 60;
				g_SSDT_Func_Index_Data.ZwQueryIntervalProfileIndex = 242;
				g_SSDT_Func_Index_Data.ZwCreateProfileIndex = 81;
				g_SSDT_Func_Index_Data.ZwSetIntervalProfileIndex = 340;
				g_ShadowSSDT_Func_Index_Data.NtUserLoadKeyboardLayoutExIndex = 4579;
				g_ShadowSSDT_Func_Index_Data.NtGdiAddFontResourceWIndex = 4098;
				g_ShadowSSDT_Func_Index_Data.NtGdiAddFontMemResourceExIndex = 4100;
				g_ShadowSSDT_Func_Index_Data.NtGdiAddRemoteFontToDCIndex = 4099;				//g_ShadowSSDT_Func_Index_Data.NtGdiAddRemoteFontToDCIndex
				g_SSDT_Func_Index_Data.ZwSuspendThreadIndex = 367;
				g_SSDT_Func_Index_Data.ZwSuspendProcessIndex = 366;
				g_SSDT_Func_Index_Data.ZwApphelpCaCheControlIndex = 41;
				g_SSDT_Func_Index_Data.ZwAccessCheckIndex = g_SSDT_Func_Index_Data.ZwAccessCheckAndAuditAlarmIndex - 1;	//�������������
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
		//Win8
		if (MinorVersion == 2)
		{
			if (BuildNumber != 9200)
			{
				goto Win10__;
			}
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_ClientLoadLibrary_Index = 67;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_ClientImmLoadLayout_Index = 84;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_fnHkOPTINLPEVENTMSG_Index = 49;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_fnHkINLPKBDLLHOOKSTRUCT_Index = 45;
			g_SSDT_Func_Index_Data.ZwRenameKeyIndex = 118;
			g_SSDT_Func_Index_Data.ZwCreateProcessIndex = 341;
			g_SSDT_Func_Index_Data.ZwCreateProcessExIndex = 340;
			g_SSDT_Func_Index_Data.ZwCreateUserProcessIndex = 325;
			g_SSDT_Func_Index_Data.ZwCreateThreadIndex = 333;
			g_SSDT_Func_Index_Data.ZwReadVirtualMemoryIndex = 131;
			g_SSDT_Func_Index_Data.ZwWriteVirtualMemoryIndex = MinorVersion;
			g_SSDT_Func_Index_Data.ZwQueueApcThreadIndex = 139;
			g_SSDT_Func_Index_Data.ZwSetContextThreadIndex = 91;
			g_SSDT_Func_Index_Data.ZwProtectVirtualMemoryIndex = 195;
			g_SSDT_Func_Index_Data.ZwAdjustGroupsTokenIndex = 415;
			g_SSDT_Func_Index_Data.ZwSystemDebugControlIndex = 37;
			g_ShadowSSDT_Func_Index_Data.ZwUserBuildHwndListIndex = 4456;
			g_ShadowSSDT_Func_Index_Data.ZwUserQueryWindowIndex = 4578;
			g_ShadowSSDT_Func_Index_Data.ZwUserFindWindowExIndex = 4555;
			g_ShadowSSDT_Func_Index_Data.ZwUserWindowFromPointIndex = 4745;
			g_ShadowSSDT_Func_Index_Data.ZwUserMessageCallIndex = 4604;
			g_ShadowSSDT_Func_Index_Data.ZwUserPostMessageIndex = 4586;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowsHookExIndex = 4655;
			g_ShadowSSDT_Func_Index_Data.ZwUserPostThreadMessageIndex = 4585;
			g_SSDT_Func_Index_Data.ZwWriteFileGatherIndex = 4;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetParentIndex = 4686;
			g_SSDT_Func_Index_Data.ZwResumeThreadIndex = 104;
			g_ShadowSSDT_Func_Index_Data.ZwUserChildWindowFromPointExIndex = 4432;
			g_ShadowSSDT_Func_Index_Data.ZwUserDestroyWindowIndex = 4471;
			g_ShadowSSDT_Func_Index_Data.ZwUserInternalGetWindowTextIndex = 4619;
			g_ShadowSSDT_Func_Index_Data.ZwUserMoveWindowIndex = 4599;
			g_ShadowSSDT_Func_Index_Data.ZwUserRealChildWindowFromPointIndex = 4577;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetInformationThreadIndex = 4694;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetInternalWindowPosIndex = 4693;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowLongIndex = 4662;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowPlacementIndex = 4661;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowPosIndex = 4660;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowRgnIndex = 4659;
			g_ShadowSSDT_Func_Index_Data.ZwUserShowWindowIndex = 4649;
			g_ShadowSSDT_Func_Index_Data.ZwUserShowWindowAsyncIndex = 4648;
			g_ShadowSSDT_Func_Index_Data.ZwUserSendInputIndex = 4710;
			g_SSDT_Func_Index_Data.ZwQueryAttributesFileIndex = 193;
			g_SSDT_Func_Index_Data.ZwCreateThreadExIndex = 332;
			g_SSDT_Func_Index_Data.ZwAlpcSendWaitReceivePortIndex = 385;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWinEventHookIndex = 4652;
			g_ShadowSSDT_Func_Index_Data.ZwUserCallHwndParamLockIndex = 4449;
			g_ShadowSSDT_Func_Index_Data.ZwUserRegisterUserApiHookIndex = 4724;
			g_ShadowSSDT_Func_Index_Data.NtUserRegisterWindowMessageIndex = 4719;
			g_ShadowSSDT_Func_Index_Data.NtUserCallNoParamIndex = 4446;
			g_ShadowSSDT_Func_Index_Data.NtUserCallOneParamIndex = 4445;
			g_ShadowSSDT_Func_Index_Data.NtUserCallTwoParamIndex = 4444;
			g_SSDT_Func_Index_Data.ZwOpenMutantIndex = 224;
			g_SSDT_Func_Index_Data.ZwCreateMutantIndex = 346;
			g_SSDT_Func_Index_Data.ZwVdmControlIndex = 16;
			g_SSDT_Func_Index_Data.ZwGetNextThreadIndex = 270;
			g_SSDT_Func_Index_Data.ZwGetNextProcessIndex = 271;
			g_SSDT_Func_Index_Data.ZwRequestPortIndex = 110;
			g_ShadowSSDT_Func_Index_Data.NtUserCallHwndLockIndex = 4452;
			g_ShadowSSDT_Func_Index_Data.NtUserUnhookWindowsHookExIndex = 4767;
			g_ShadowSSDT_Func_Index_Data.NtUserClipCursorIndex = 4431;
			g_ShadowSSDT_Func_Index_Data.NtUserGetKeyboardStateIndex = 4513;
			g_ShadowSSDT_Func_Index_Data.NtUserGetKeyStateIndex = 4511;
			g_ShadowSSDT_Func_Index_Data.NtUserGetAsyncKeyStateIndex = 4549;
			g_ShadowSSDT_Func_Index_Data.NtUserAttachThreadInputIndex = 4461;
			g_ShadowSSDT_Func_Index_Data.NtUserRegisterHotKeyIndex = 4723;
			g_ShadowSSDT_Func_Index_Data.NtUserRegisterRawInputDevicesIndex = 4722;
			g_ShadowSSDT_Func_Index_Data.NtGdiBitBltIndex = 4398;
			g_ShadowSSDT_Func_Index_Data.NtGdiStretchBltIndex = 4108;
			g_ShadowSSDT_Func_Index_Data.NtGdiMaskBltIndex = 4173;
			g_ShadowSSDT_Func_Index_Data.NtGdiPlgBltIndex = 4163;
			g_ShadowSSDT_Func_Index_Data.NtGdiTransparentBltIndex = 4102;
			g_ShadowSSDT_Func_Index_Data.NtGdiAlphaBlendIndex = 4405;
			g_ShadowSSDT_Func_Index_Data.NtGdiGetPixelIndex = 4210;
			g_SSDT_Func_Index_Data.ZwTerminateThreadIndex = 34;
			g_SSDT_Func_Index_Data.ZwTerminateJobObjectIndex = 36;
			g_SSDT_Func_Index_Data.ZwDebugActiveProcessIndex = 320;
			g_SSDT_Func_Index_Data.ZwAssignProcessToJobObjectIndex = 381;
			g_SSDT_Func_Index_Data.ZwSetInformationJobObjectIndex = 77;
			g_ShadowSSDT_Func_Index_Data.NtUserGetRawInputBufferIndex = 4500;
			g_ShadowSSDT_Func_Index_Data.NtUserGetRawInputDataIndex = 4499;
			g_ShadowSSDT_Func_Index_Data.NtUserSetImeInfoExIndex = 4696;
			g_SSDT_Func_Index_Data.ZwQueueApcThreadExIndex = 138;
			g_ShadowSSDT_Func_Index_Data.NtGdiOpenDCWIndex = 4167;
			g_ShadowSSDT_Func_Index_Data.NtGdiDeleteObjectAppIndex = 4285;
			g_ShadowSSDT_Func_Index_Data.NtUserBlockInputIndex = 4458;
			g_SSDT_Func_Index_Data.ZwContinueIndex = 362;
			g_SSDT_Func_Index_Data.ZwQueryIntervalProfileIndex = 168;
			g_SSDT_Func_Index_Data.ZwCreateProfileIndex = 339;
			g_SSDT_Func_Index_Data.ZwSetIntervalProfileIndex = 67;
			g_ShadowSSDT_Func_Index_Data.NtUserLoadKeyboardLayoutExIndex = 4612;
			g_ShadowSSDT_Func_Index_Data.NtGdiAddFontResourceWIndex = 4410;
			g_ShadowSSDT_Func_Index_Data.NtGdiAddFontMemResourceExIndex = 4408;
			g_ShadowSSDT_Func_Index_Data.NtGdiAddRemoteFontToDCIndex = 4409;
			g_SSDT_Func_Index_Data.ZwApphelpCaCheControlIndex = 383;
			g_SSDT_Func_Index_Data.dword_1BB08 = 4803;
			return TRUE;
		}
		//Win8 9600
		else
		{
			if (MinorVersion != 3 || BuildNumber != 9600)
			{
				goto Win10__;
			}
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_ClientLoadLibrary_Index = 71;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_ClientImmLoadLayout_Index = 88;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_fnHkOPTINLPEVENTMSG_Index = 49;
			g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_fnHkINLPKBDLLHOOKSTRUCT_Index = 45;
			g_SSDT_Func_Index_Data.ZwRenameKeyIndex = 121;
			g_SSDT_Func_Index_Data.ZwCreateProcessIndex = 345;
			g_SSDT_Func_Index_Data.ZwCreateProcessExIndex = 344;
			g_SSDT_Func_Index_Data.ZwCreateUserProcessIndex = 329;
			g_SSDT_Func_Index_Data.ZwCreateThreadIndex = 337;
			g_SSDT_Func_Index_Data.ZwReadVirtualMemoryIndex = 134;
			g_SSDT_Func_Index_Data.ZwWriteVirtualMemoryIndex = MinorVersion;
			g_SSDT_Func_Index_Data.ZwQueueApcThreadIndex = 142;
			g_SSDT_Func_Index_Data.ZwSetContextThreadIndex = 94;
			g_SSDT_Func_Index_Data.ZwProtectVirtualMemoryIndex = 198;
			g_SSDT_Func_Index_Data.ZwAdjustGroupsTokenIndex = 420;
			g_SSDT_Func_Index_Data.ZwSystemDebugControlIndex = 37;
			g_SSDT_Func_Index_Data.ZwWriteFileGatherIndex = 5;
			g_SSDT_Func_Index_Data.ZwResumeThreadIndex = 107;
			g_SSDT_Func_Index_Data.ZwQueryAttributesFileIndex = 196;
			g_SSDT_Func_Index_Data.ZwCreateThreadExIndex = 336;
			g_SSDT_Func_Index_Data.ZwAlpcSendWaitReceivePortIndex = 390;
			g_SSDT_Func_Index_Data.ZwOpenMutantIndex = 227;
			g_SSDT_Func_Index_Data.ZwCreateMutantIndex = 350;
			g_SSDT_Func_Index_Data.ZwVdmControlIndex = 16;
			g_SSDT_Func_Index_Data.ZwGetNextThreadIndex = 273;
			g_SSDT_Func_Index_Data.ZwGetNextProcessIndex = 274;
			g_SSDT_Func_Index_Data.ZwRequestPortIndex = 113;
			g_SSDT_Func_Index_Data.ZwTerminateThreadIndex = 34;
			g_SSDT_Func_Index_Data.ZwTerminateJobObjectIndex = 36;
			g_SSDT_Func_Index_Data.ZwDebugActiveProcessIndex = 324;
			g_SSDT_Func_Index_Data.ZwAssignProcessToJobObjectIndex = 386;
			g_SSDT_Func_Index_Data.ZwSetInformationJobObjectIndex = 80;
			g_ShadowSSDT_Func_Index_Data.ZwUserBuildHwndListIndex = 4458;
			g_ShadowSSDT_Func_Index_Data.ZwUserQueryWindowIndex = 4579;
			g_ShadowSSDT_Func_Index_Data.ZwUserFindWindowExIndex = 4556;
			g_ShadowSSDT_Func_Index_Data.ZwUserWindowFromPointIndex = 4748;
			g_ShadowSSDT_Func_Index_Data.ZwUserMessageCallIndex = 4606;
			g_ShadowSSDT_Func_Index_Data.ZwUserPostMessageIndex = 4588;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowsHookExIndex = 4658;
			g_ShadowSSDT_Func_Index_Data.ZwUserPostThreadMessageIndex = 4587;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetParentIndex = 4689;
			g_ShadowSSDT_Func_Index_Data.ZwUserChildWindowFromPointExIndex = 4434;
			g_ShadowSSDT_Func_Index_Data.ZwUserDestroyWindowIndex = 4473;
			g_ShadowSSDT_Func_Index_Data.ZwUserInternalGetWindowTextIndex = 4622;
			g_ShadowSSDT_Func_Index_Data.ZwUserMoveWindowIndex = 4601;
			g_ShadowSSDT_Func_Index_Data.ZwUserRealChildWindowFromPointIndex = 4578;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetInformationThreadIndex = 4697;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetInternalWindowPosIndex = 4696;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowLongIndex = 4665;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowPlacementIndex = 4664;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowPosIndex = 4663;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowRgnIndex = 4662;
			g_ShadowSSDT_Func_Index_Data.ZwUserShowWindowIndex = 4652;
			g_ShadowSSDT_Func_Index_Data.ZwUserShowWindowAsyncIndex = 4651;
			g_ShadowSSDT_Func_Index_Data.ZwUserSendInputIndex = 4713;
			g_ShadowSSDT_Func_Index_Data.ZwUserSetWinEventHookIndex = 4655;
			g_ShadowSSDT_Func_Index_Data.ZwUserCallHwndParamLockIndex = 4451;
			g_ShadowSSDT_Func_Index_Data.ZwUserRegisterUserApiHookIndex = 4727;
			g_ShadowSSDT_Func_Index_Data.NtUserRegisterWindowMessageIndex = 4722;
			g_ShadowSSDT_Func_Index_Data.NtUserCallNoParamIndex = 4448;
			g_ShadowSSDT_Func_Index_Data.NtUserCallOneParamIndex = 4447;
			g_ShadowSSDT_Func_Index_Data.NtUserCallTwoParamIndex = 4446;
			g_ShadowSSDT_Func_Index_Data.NtUserCallHwndLockIndex = 4454;
			g_ShadowSSDT_Func_Index_Data.NtUserUnhookWindowsHookExIndex = 4770;
			g_ShadowSSDT_Func_Index_Data.NtUserClipCursorIndex = 4433;
			g_ShadowSSDT_Func_Index_Data.NtUserGetKeyboardStateIndex = 4515;
			g_ShadowSSDT_Func_Index_Data.NtUserGetKeyStateIndex = 4513;
			g_ShadowSSDT_Func_Index_Data.NtUserGetAsyncKeyStateIndex = 4550;
			g_ShadowSSDT_Func_Index_Data.NtUserAttachThreadInputIndex = 4463;
			g_ShadowSSDT_Func_Index_Data.NtUserRegisterHotKeyIndex = 4726;
			g_ShadowSSDT_Func_Index_Data.NtUserRegisterRawInputDevicesIndex = 4725;
			g_ShadowSSDT_Func_Index_Data.NtGdiBitBltIndex = 4400;
			g_ShadowSSDT_Func_Index_Data.NtGdiStretchBltIndex = 4109;
			g_ShadowSSDT_Func_Index_Data.NtGdiMaskBltIndex = 4174;
			g_ShadowSSDT_Func_Index_Data.NtGdiPlgBltIndex = 4164;
			g_ShadowSSDT_Func_Index_Data.NtGdiTransparentBltIndex = 4103;
			g_ShadowSSDT_Func_Index_Data.NtGdiAlphaBlendIndex = 4407;
			g_ShadowSSDT_Func_Index_Data.NtGdiGetPixelIndex = 4211;
			g_ShadowSSDT_Func_Index_Data.NtUserGetRawInputBufferIndex = 4502;
			g_ShadowSSDT_Func_Index_Data.NtUserGetRawInputDataIndex = 4501;
			g_ShadowSSDT_Func_Index_Data.NtUserSetImeInfoExIndex = 4699;
			g_SSDT_Func_Index_Data.ZwQueueApcThreadExIndex = 141;
			g_ShadowSSDT_Func_Index_Data.NtGdiOpenDCWIndex = 4168;
			g_ShadowSSDT_Func_Index_Data.NtGdiDeleteObjectAppIndex = 4287;
			g_ShadowSSDT_Func_Index_Data.NtUserBlockInputIndex = 4460;
			g_SSDT_Func_Index_Data.ZwContinueIndex = 367;
			g_SSDT_Func_Index_Data.ZwQueryIntervalProfileIndex = 171;
			g_SSDT_Func_Index_Data.ZwCreateProfileIndex = 343;
			g_SSDT_Func_Index_Data.ZwSetIntervalProfileIndex = 70;
			g_ShadowSSDT_Func_Index_Data.NtUserLoadKeyboardLayoutExIndex = 4615;
			g_ShadowSSDT_Func_Index_Data.NtGdiAddFontResourceWIndex = 4412;
			g_ShadowSSDT_Func_Index_Data.NtGdiAddFontMemResourceExIndex = 4410;
			g_ShadowSSDT_Func_Index_Data.NtGdiAddRemoteFontToDCIndex = 4411;
			g_SSDT_Func_Index_Data.ZwApphelpCaCheControlIndex = 388;
			g_SSDT_Func_Index_Data.dword_1BB08 = 4806;
		}
		g_SSDT_Func_Index_Data.ZwAccessCheckIndex = g_SSDT_Func_Index_Data.ZwAccessCheckAndAuditAlarmIndex + 1;	//�������������
		g_SSDT_Func_Index_Data.ZwSuspendThreadIndex = 38;
		g_SSDT_Func_Index_Data.ZwSuspendProcessIndex = 39;
		return TRUE;
	}
	case 0xA:
	{
	Win10__:
		if (!Global_Version_Win10_Flag || BuildNumber < 10240)
		{
			return 0;
		}
		if (BuildNumber >= 14352)
		{
			if (BuildNumber < 14942)
				goto LABEL_234;
			if (BuildNumber < 15002)
			{
				g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_ClientLoadLibrary_Index = 81;
				g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_ClientImmLoadLayout_Index = 98;
				goto LABEL_228;
			}
			if (BuildNumber < 17035 || BuildNumber >= 17063 && (BuildNumber < 18272 || BuildNumber >= 18362))
			{
			LABEL_234:
				g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_ClientLoadLibrary_Index = 75;
				g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_ClientImmLoadLayout_Index = 0x5C;
				goto LABEL_228;
			}
		}
		g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_ClientLoadLibrary_Index = 74;
		g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_ClientImmLoadLayout_Index = 0x5B;
	LABEL_228:
		g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_fnHkOPTINLPEVENTMSG_Index = 0x31;
		g_ShadowSSDT_Func_Index_Data.KeUserModeCallback_fnHkINLPKBDLLHOOKSTRUCT_Index = 0x2D;
		if (!Global_Win32kFlag)
		{
			return 1;
		}
		//return sub_12A50();
	}

	}
	return TRUE;
}


// ���ò���
VOID HookPort_SetFlag_On()
{
	dword_1B110 = 1;
}

//һ����
//layerfsd����Ľṹ�����£�
//typedef NTSTATUS (NTAPI *pPsReleaseProcessExitSynchronization)(HANDLE, HANDLE);
//���ǰٶ���������ṹӦ����
//typedef NTSTATUS(NTAPI *pPsReleaseProcessExitSynchronization)(__in PEPROCESS Process);
HANDLE	HookPort_GetApiPortProcessId(IN RTL_OSVERSIONINFOEXW osverinfo)
{
	PVOID P;

	BOOLEAN	Found_Port = FALSE;

	PEPROCESS	ProcObject;

	PVOID RefObject;

	ULONG ReturnLength, Item_Count, i;

	HANDLE ObjectHandle = NULL;

	HANDLE Handle = NULL;

	NTSTATUS	status;

	CHAR	ObjectInformation[0x1000] = { 0 };

	PPUBLIC_OBJECT_TYPE_INFORMATION	pPubObjTypeInfo;

	CLIENT_ID ClientId;
	OBJECT_ATTRIBUTES ObjectAttributes;

	PSYSTEM_HANDLE_INFORMATION	pHandleInfo;

	PVOID	pHandleBuf, psystemhandleinfo = NULL;

	UNICODE_STRING SystemRoutineName;

	ULONG BuildNumber = osverinfo.dwBuildNumber;
	ULONG MinorVersion = osverinfo.dwMinorVersion;
	ULONG MajorVersion = osverinfo.dwMajorVersion;
	//1������vista���ϵ�ϵͳ������ʹ��PsAcquireProcessExitSynchronization / PsReleaseProcessExitSynchronization�����������ͷ�process rundown��
	if (BuildNumber >= 6000)
	{
		RtlInitUnicodeString(&SystemRoutineName, PsAcquireProcessExitSynchronizationName);
		PsAcquireProcessExitSynchronization = (pPsAcquireProcessExitSynchronization)MmGetSystemRoutineAddress(&SystemRoutineName);

		RtlInitUnicodeString(&SystemRoutineName, PsReleaseProcessExitSynchronizationName);
		PsReleaseProcessExitSynchronization = (pPsReleaseProcessExitSynchronization)MmGetSystemRoutineAddress(&SystemRoutineName);
	}
	//2����ȡϵͳ���
	psystemhandleinfo = (PVOID)HookPort_QuerySystemInformation(SystemHandleInformation);
	if (!psystemhandleinfo)
	{
		return NULL;
	}
	//2��1 ���ݻ������׵�ַ
	pHandleBuf = psystemhandleinfo;
	//2��2 �жϾ������
	Item_Count = *(PULONG)psystemhandleinfo;
	if (Item_Count == 0)
	{
		ExFreePool(psystemhandleinfo);
		return NULL;
	}
	pHandleInfo = (PSYSTEM_HANDLE_INFORMATION)((PCHAR)psystemhandleinfo + 4);

	InitializeObjectAttributes(&ObjectAttributes, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
	for (i = 0; i < Item_Count; i++, pHandleInfo++)
	{
		ClientId.UniqueProcess = (HANDLE)pHandleInfo->ProcessID;
		ClientId.UniqueThread = NULL;
		if (!NT_SUCCESS(ZwOpenProcess(&Handle, 0x40u, &ObjectAttributes, &ClientId)))
		{
			continue;
		}
		if (ZwDuplicateObject(Handle, (HANDLE)(pHandleInfo->Handle), (HANDLE)-1, &ObjectHandle, 0, 0, 2) < 0)
		{

			if ((WORD)BuildNumber >= 6000
				&& PsAcquireProcessExitSynchronization
				&& PsReleaseProcessExitSynchronization)
			{

				status = PsLookupProcessByProcessId(ClientId.UniqueProcess, &ProcObject);
				if (NT_SUCCESS(status))
				{

					status = PsAcquireProcessExitSynchronization(ProcObject);
					if (NT_SUCCESS(status))
					{

						KeAttachProcess(ProcObject);
						status = ObReferenceObjectByHandle((HANDLE)(pHandleInfo->Handle), FILE_ANY_ACCESS, 0, KernelMode, &RefObject, 0);

						if (NT_SUCCESS(status))
						{

							if (RefObject == pHandleInfo->Object)
							{

								status = ZwQueryObject(
									(HANDLE)(pHandleInfo->Handle),
									ObjectTypeInformation,
									ObjectInformation,
									sizeof(ObjectInformation),
									&ReturnLength);

								if (NT_SUCCESS(status))
								{

									pPubObjTypeInfo = (PPUBLIC_OBJECT_TYPE_INFORMATION)ObjectInformation;
									if (pPubObjTypeInfo->TypeName.Buffer)
									{

										if (0 == _wcsnicmp(pPubObjTypeInfo->TypeName.Buffer, L"ALPC Port", 9))
										{

											status = ZwQueryObject(
												(HANDLE)(pHandleInfo->Handle),
												ObjectNameInformation,
												ObjectInformation,
												sizeof(ObjectInformation),
												&ReturnLength);
											if (NT_SUCCESS(status))
											{

												pPubObjTypeInfo = (PPUBLIC_OBJECT_TYPE_INFORMATION)ObjectInformation;
												if (pPubObjTypeInfo->TypeName.Buffer && pPubObjTypeInfo->TypeName.Length)
												{

													if (0 == _wcsnicmp(pPubObjTypeInfo->TypeName.Buffer, L"\\Windows\\ApiPort", 0x10))
													{

														Found_Port = TRUE;

													}

												}

											}

										}

									}

								}

							}

						}


						if (RefObject)
						{
							ObDereferenceObject(RefObject);
						}
						PsReleaseProcessExitSynchronization(ProcObject);
						KeDetachProcess();
						ObDereferenceObject(ProcObject);
						ZwClose(Handle);
						if (Found_Port)
						{
							ExFreePool(pHandleBuf);
							return ClientId.UniqueProcess;
						}

					}
					else
					{
						ObfDereferenceObject(ProcObject);
					}


				}

			}

			ZwClose(Handle);
			continue;

		}

		pPubObjTypeInfo = (PPUBLIC_OBJECT_TYPE_INFORMATION)ObjectInformation;
		if (NT_SUCCESS(ZwQueryObject(ObjectHandle, ObjectTypeInformation, ObjectInformation, sizeof(ObjectInformation), &ReturnLength))
			&& (pPubObjTypeInfo->TypeName.Buffer)
			&& (!_wcsnicmp(pPubObjTypeInfo->TypeName.Buffer, L"Port", 4) || !_wcsnicmp(pPubObjTypeInfo->TypeName.Buffer, L"ALPC Port", 9))
			&& NT_SUCCESS(ZwQueryObject(ObjectHandle, ObjectNameInformation, ObjectInformation, sizeof(ObjectInformation), &ReturnLength))
			&& (pPubObjTypeInfo->TypeName.Buffer && pPubObjTypeInfo->TypeName.Length)
			&& !_wcsnicmp(L"\\Windows\\ApiPort", pPubObjTypeInfo->TypeName.Buffer, 0x10))
		{
			break;
		}

		ZwClose(ObjectHandle);
		ZwClose(Handle);
		continue;
	}

	ZwClose(ObjectHandle);
	ZwClose(Handle);
	ExFreePool(pHandleBuf);
	return ClientId.UniqueProcess;
}

//IAT hook KeUserModeCallback
VOID HookPort_HookKeUserModeCallback(IN ULONG Version_Win10_Flag)
{
	PVOID 			pModuleBase = NULL;
	ULONG 			ModuleSize = NULL;
	UCHAR           Win32k[0x50] = { 0 };
	PULONG 			pOriginalProc = NULL;
	PVOID 			pOriginalProcAddr = NULL;
	volatile LONG * Mdlv1_KeUserModeCallback = NULL;
	PMDL			MemoryDescriptorList = NULL;
	//1�����ݰ汾�жϲ���win32k.sys����win32kfull.sys
	if (Version_Win10_Flag)
	{
		//Win10ʹ��win32kfull.sys
		memcpy(Win32k, WIN32KFULLSYS, strlen(WIN32KFULLSYS));
	}
	else
	{
		//��Win10ʹ��win32k.sys
		memcpy(Win32k, WIN32KSYS, strlen(WIN32KFULLSYS));
	}
	if (HookPort_GetModuleBaseAddress(Win32k, &pModuleBase, &ModuleSize, 0))
	{
		//�ӵ�����л�ȡԭʼ��ַ
		pOriginalProc = HookPort_HookImportedFunction(pModuleBase, ModuleSize, "KeUserModeCallback", NTOSKERNL, &pOriginalProcAddr);
		//�ж��Ƿ��Hook��
		if ((pOriginalProc != Filter_KeUserModeCallbackDispatcher) && (pOriginalProc))
		{
			//����ԭʼ����
			pOriginalKeUserModeCallbackAddr = pOriginalProcAddr;
			OriginalKeUserModeCallback = (PVOID)pOriginalProc;			//����һ��KeUserModeCallback����ָ��
			//Ȼ��ʹ��MDL��ȫhook
			Mdlv1_KeUserModeCallback = HookPort_LockMemory(pOriginalProcAddr, sizeof(ULONG), (ULONG)&MemoryDescriptorList, Version_Win10_Flag);
			if (Mdlv1_KeUserModeCallback)
			{
				//IATHook
				InterlockedExchange(Mdlv1_KeUserModeCallback, (LONG)Filter_KeUserModeCallbackDispatcher);
			}
			if (MemoryDescriptorList)
			{
				HookPort_RemoveLockMemory(MemoryDescriptorList);
			}
		}
	}
}

//************************************     
// ��������: HookPort_InitSDT     
// ����˵��������hook���   
// IDA��ַ ��sub_16B34
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�    
// �� �� ֵ: NTSTATUS     
//************************************  
NTSTATUS NTAPI HookPort_InitSDT()
{
	NTSTATUS		Status = STATUS_SUCCESS;
	HANDLE			ApiPortProcessId = NULL;
	BOOLEAN			flag = 0;
	PVOID 			pModuleBase = NULL;
	ULONG 			ModuleSize = NULL;
	PEPROCESS 	    Process = NULL;
	KAPC_STATE 	    ApcState = { 0 };
	UNICODE_STRING  SystemRoutineName = { 0 };
	NTSTATUS(NTAPI *pPsSetCreateProcessNotifyRoutineEx)(_In_ PCREATE_PROCESS_NOTIFY_ROUTINE_EX NotifyRoutine, _In_ BOOLEAN Remove);
	ULONG BuildNumber  = Global_osverinfo.dwBuildNumber;
	ULONG MinorVersion = Global_osverinfo.dwMinorVersion;
	ULONG MajorVersion = Global_osverinfo.dwMajorVersion;
	//1����ȡԭʼNT�ں˻���ַ
	if (!HookPort_GetModuleBaseAddress(0, &g_HookPort_Nt_Win32k_Data.NtData.NtImageBase, &g_HookPort_Nt_Win32k_Data.NtData.NtImageSize, 0))
	{
		KdPrint(("��ȡNT�ں˻�ַʧ��\t\n"));
		HookPort_RtlWriteRegistryValue(2);
		return STATUS_UNSUCCESSFUL;
	}
	KdPrint(("NT�ں˻���ַ�ǣ�%X\t\n", g_HookPort_Nt_Win32k_Data.NtData.NtImageBase));
	//2����ȡSSSDT��ַ
	if (HookPort_GetModuleBaseAddress(WIN32KSYS, &pModuleBase, &ModuleSize, 0))
	{
		KdPrint(("win32k�ں˻���ַ�ǣ�%X\t\n", pModuleBase));
		Global_Win32kFlag = 1;
		if (!HookPort_GetShadowTableAddress(
			&g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiServiceTableBase, //[Out]ShadowSSDT_GuiServiceTableBase
			&g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiNumberOfServices, //[Out]ShadowSSDT_GuiNumberOfServices
			&g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiParamTableBase,   //[Out]ShadowSSDT_GuiParamTableBase
			g_HookPort_Nt_Win32k_Data.NtData.NtImageBase,									//[In]Nt�ں˵Ļ���ַ
			Global_Version_Win10_Flag,														//[In]Win10��־
			Global_osverinfo																//[In]�汾��Ϣ
			))
		{
			HookPort_RtlWriteRegistryValue(1);
			return STATUS_UNSUCCESSFUL;
		}
	}
	//3����ȡSSDT��ַ
	Status = HookPort_GetSSDTTableAddress(
		&g_HookPort_Nt_Win32k_Data.SSDTTable_Data.SSDT_KeServiceTableBase,			//[Out]SSDT_KeServiceTableBase
		&g_HookPort_Nt_Win32k_Data.SSDTTable_Data.SSDT_KeNumberOfServices,			//[Out]SSDT_KeNumberOfServices
		&g_HookPort_Nt_Win32k_Data.SSDTTable_Data.SSDT_KeParamTableBase,			//[Out]SSDT_KeParamTableBase
		g_HookPort_Nt_Win32k_Data.NtData.NtImageBase								//[In]Nt�ں˵Ļ���ַ
		);
	if (!NT_SUCCESS(Status))
	{

		KdPrint(("HookPort: HookPort_GetSSDTTableAddress failed,err=%08x\n", Status));
		return Status;
	}
	KdPrint(("��ȡSSDT��ɹ�\t\n"));
	//4����ȡ���غ�����ַ
	if (!HookPort_GetAllNativeFunAddress(g_HookPort_Nt_Win32k_Data.NtData.NtImageBase, Global_osverinfo))
	{
		KdPrint(("HookPort_GetAllNativeFunAddress failed\t\n"));
		HookPort_RtlWriteRegistryValue(4);
		return STATUS_UNSUCCESSFUL;
	}
	//4��1 ��ʼ����������
	HookPort_InitFilterTable();
	//5��׼�����������HOOK��Ҫ�õ�����
	if (!HookPort_AllocFilterTable())
	{
		HookPort_RtlWriteRegistryValue(0x12);
		return STATUS_NO_MEMORY;
	}
	//6���жϷ�win10�������win32k.sys
	if (!Global_Version_Win10_Flag || Global_Win32kFlag)
	{
		if (!HookPort_InitProxyAddress(0))
		{
			ExFreePool(g_SS_Filter_Table);
			HookPort_RtlWriteRegistryValue(7);
			return STATUS_NO_MEMORY;
		}
	}
	else
	{
		HookPort_SetFlag_On();
	}
	//7��ʹ��ZwSetEvent��ȡKiFastCallEntry��ַ
	if (HookPort_InstallZwSetEventHook() != STATUS_SUCCESS)// ʹ��ZwSetEvent��ȡKiFastCallEntry��ַ
	{
		if (filter_function_table_Size_temp)
		{
			ExFreePool(filter_function_table_Size_temp);
		}
		ExFreePool(g_SS_Filter_Table);
		return STATUS_UNSUCCESSFUL;
	}
	//8���������̡��̡߳�ģ��ص�
	PsSetCreateProcessNotifyRoutine(Filter_CreateProcessNotifyRoutine, FALSE);
	PsSetLoadImageNotifyRoutine(Filter_LoadImageNotifyRoutine);
	PsSetCreateThreadNotifyRoutine(Filter_CreateThreadNotifyRoutine);
	if (BuildNumber >= 6001)
	{
		RtlInitUnicodeString(&SystemRoutineName, L"PsSetCreateProcessNotifyRoutineEx");
		pPsSetCreateProcessNotifyRoutineEx = MmGetSystemRoutineAddress(&SystemRoutineName);
		if (MmIsAddressValid(pPsSetCreateProcessNotifyRoutineEx))
		{
			pPsSetCreateProcessNotifyRoutineEx(Filter_CreateProcessNotifyRoutineEx, FALSE);
		}
	}
	//9��1 �ж��Ƿ����win32k
	if (Global_Win32kFlag)
	{
		//9��2 win10��������п�������
		if (Global_Version_Win10_Flag)
		{
			//����ZwSetSystemInformation������hook����
			g_SS_Filter_Table->SwitchTableForSSDT[g_SSDT_Func_Index_Data.ZwSetSystemInformationIndex] = 1;
			//��ȡcsrss�Ľ���id
			ApiPortProcessId = HookPort_GetApiPortProcessId(Global_osverinfo);
			if (ApiPortProcessId)
			{
				if (NT_SUCCESS(PsLookupProcessByProcessId(ApiPortProcessId, &Process)))
				{
					//����GUI�̺߳������㴦��ShadowSSDT
					KeStackAttachProcess(Process, &ApcState);
					//��ȡwin10_Win7��Shadow��ֵ
					//���и�����δ���򣬸о�����sub_14F82
					if (HookPort_GetShadowTableAddress_Win10(
						pModuleBase,																				//[In]Win32����ַ
						&g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiServiceTableBase,				//[Out]ShadowSSDT_GuiServiceTableBase
						&g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiNumberOfServices,				//[Out]ShadowSSDT_GuiNumberOfServices
						&g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiParamTableBase,				//[Out]ShadowSSDT_GuiParamTableBase
						&g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiServiceTableBase_Win10_14316, //[Out]ShadowSSDT_GuiServiceTableBase_Win10_14316
						&g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiNumberOfServices_Win10_14316, //[Out]ShadowSSDT_GuiNumberOfServices_Win10_14316
						&g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiParamTableBase_Win10_14316,	//[Out]ShadowSSDT_GuiParamTableBase_Win10_14316
						Global_osverinfo																			//[In]�汾��Ϣ
						))
					{
						//��ʼ����������
						HookPort_InitFilterTable();
						//����ShadowSSDThook����
						HookPort_InitProxyAddress(1);
					}
					else
					{
						g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiServiceTableBase = 0;
					}
					//�������
					KeUnstackDetachProcess(&ApcState);
					ObfDereferenceObject(Process);
				}
			}
			else
			{
				//ʧ�ܾͲ�����ShadowSSDT
				g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiServiceTableBase = 0;
			}
			//HookPort_GetApiPortProcessId_Win10();
		}
		// ��win10����
		else
		{
			//��ȡcsrss�Ľ���id
			ApiPortProcessId = HookPort_GetApiPortProcessId(Global_osverinfo);
			if (ApiPortProcessId)
			{
				if (NT_SUCCESS(PsLookupProcessByProcessId(ApiPortProcessId, &Process)))
				{
					//����GUI�̺߳������㴦��ShadowSSDT
					KeStackAttachProcess(Process, &ApcState);
					//�˺����޸������װKeUserModeCallback����
					HookPort_HookKeUserModeCallback(Global_Version_Win10_Flag);
					//����ShadowSSDThook����
					HookPort_InitProxyAddress(1);
					//�������
					KeUnstackDetachProcess(&ApcState);
					ObfDereferenceObject(Process);
				}
			}
			else
			{
				//ʧ�ܾͲ�����ShadowSSDT
				g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiServiceTableBase = 0;
			}
		}
	}
	//10�� ��win10
	else if (!Global_Version_Win10_Flag)
	{
		//����ZwSetSystemInformation������hook����
		g_SS_Filter_Table->SwitchTableForSSDT[g_SSDT_Func_Index_Data.ZwSetSystemInformationIndex] = 1;
	}
	return STATUS_SUCCESS;
}

//************************************     
// ��������: HookPort_AllocFilterRuleTable     
// ����˵������ʼ�������½������ӵ���������    
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2020/01/04     
// �� �� ֵ: PVOID NTAPI     
// ��    ��: IN ULONG NumberOfBytes    > 2������0���رգ���< 2������1�������� 
//************************************ 
PVOID NTAPI HookPort_AllocFilterRuleTable(IN ULONG NumberOfBytes)
{
	ULONG	Count, AllocSize;
	PVOID result;
	KIRQL old_irql;
	PFILTERFUN_RULE_TABLE	prule_table, ptemp;
	ULONG	flag = (NumberOfBytes < 2) ? 1 : 0;
	//1��new���������Ŀռ�
	prule_table = (PFILTERFUN_RULE_TABLE)ExAllocatePoolWithTag(NonPagedPool, sizeof(FILTERFUN_RULE_TABLE), HOOKPORT_POOLTAG3);
	if (!prule_table)
	{
		return NULL;
	}
	//1��1 ����ýṹ�Ĵ�С
	RtlZeroMemory(prule_table, sizeof(FILTERFUN_RULE_TABLE));
	prule_table->Size = sizeof(FILTERFUN_RULE_TABLE);

	//1��2 new��SSDT�ṹ���������С�Ŀռ�
	prule_table->SSDTRuleTableBase = ExAllocatePoolWithTag(NonPagedPool, g_HookPort_Nt_Win32k_Data.SSDTTable_Data.SSDT_KeNumberOfServices * sizeof(ULONG), HOOKPORT_POOLTAG4);
	if (!prule_table->SSDTRuleTableBase)
	{
		ExFreePool(prule_table);
		return NULL;
	}
	//1��3 ѭ�����SSDTRuleTableBase�ṹ��1����0
	for (ULONG i_v4 = 0; i_v4 < g_HookPort_Nt_Win32k_Data.SSDTTable_Data.SSDT_KeNumberOfServices; i_v4++)
	{
		prule_table->SSDTRuleTableBase[i_v4] = flag;
	}

	//1��4 ����ShadowSSDT_GuiNumberOfServices����
	if (g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiNumberOfServices)
	{
		AllocSize = g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiNumberOfServices * sizeof(ULONG);
	}
	else
	{
		AllocSize = 8000;							//�Ҳ������ø����ֵ
	}

	//1��5 new��ShadowSSDT�ṹ���������С�Ŀռ�
	prule_table->ShadowSSDTRuleTableBase = ExAllocatePoolWithTag(NonPagedPool, AllocSize, HOOKPORT_POOLTAG5);
	if (!prule_table->ShadowSSDTRuleTableBase) {
		ExFreePool(prule_table->SSDTRuleTableBase);
		ExFreePool(prule_table);
		return NULL;
	}
	//1��6 ѭ�����ShadowSSDTRuleTableBase�ṹ��1����0
	for (Count = 0; Count < AllocSize / sizeof(ULONG); Count++) {
		prule_table->ShadowSSDTRuleTableBase[Count] = flag;
	}
	KeAcquireSpinLock(&g_Filter_Rule_SpinLock, &old_irql);

	if (!g_FilterFun_Rule_table_head)
	{
		g_FilterFun_Rule_table_head = prule_table;
	}
	else
	{
		ptemp = g_FilterFun_Rule_table_head;
		while (ptemp->Next)
		{
			ptemp = ptemp->Next;
		}
		ptemp->Next = prule_table;
	}
	KeReleaseSpinLock(&g_Filter_Rule_SpinLock, old_irql);
	return prule_table;
}

//���ù��򿪹�
VOID NTAPI HookPort_SetFilterRule(IN PFILTERFUN_RULE_TABLE	filter_rule,IN ULONG  index,IN ULONG  rule)
{
	ULONG	service_index;
	if ((index > FILTERFUNCNT - 1) || (index == 12))
	{
		return;
	}
	service_index = filter_function_table[index];
	if (service_index == g_SSDTServiceLimit) 
	{
		filter_rule->FakeServiceRuleFlag[index] = rule;
		return;
	}

	//ShadowSSDT����
	if (_CHECK_IS_SHADOW_CALL(service_index)) {
		filter_rule->ShadowSSDTRuleTableBase[service_index & 0xFFF] = rule;
		return;

	}

	if (index == ZwLoad_Un_Driver_FilterIndex)
	{
		if (g_SSDT_Func_Index_Data.ZwUnloadDriverIndex == g_SSDTServiceLimit)
		{
			filter_rule->SSDTRuleTableBase[service_index] = rule;
		}
		else
		{
			filter_rule->SSDTRuleTableBase[g_SSDT_Func_Index_Data.ZwUnloadDriverIndex] = rule;
			filter_rule->SSDTRuleTableBase[service_index] = rule;
		}
		return;
	}
	if (index == ZwWriteFile_FilterIndex)
	{
		if (g_SSDT_Func_Index_Data.ZwWriteFileGatherIndex == g_SSDTServiceLimit)
		{
			filter_rule->SSDTRuleTableBase[service_index] = rule;
		}
		else
		{
			filter_rule->SSDTRuleTableBase[g_SSDT_Func_Index_Data.ZwWriteFileGatherIndex] = rule;
			filter_rule->SSDTRuleTableBase[service_index] = rule;
		}
		return;
	}
	if (index == ZwSetInformationFile_FilterIndex)
	{
		if (g_SSDT_Func_Index_Data.ZwCreateThreadExIndex == g_SSDTServiceLimit)
		{
			filter_rule->SSDTRuleTableBase[service_index] = rule;
		}
		else
		{
			filter_rule->SSDTRuleTableBase[g_SSDT_Func_Index_Data.ZwCreateThreadExIndex] = rule;
			filter_rule->SSDTRuleTableBase[service_index] = rule;
		}
		return;
	}
	if (index != ZwSetValueKey_FilterIndex || g_ShadowSSDT_Func_Index_Data.NtUserSetImeInfoExIndex == g_SSDTServiceLimit)
	{
		filter_rule->SSDTRuleTableBase[service_index] = rule;
	}
	else
	{
		filter_rule->ShadowSSDTRuleTableBase[g_ShadowSSDT_Func_Index_Data.NtUserSetImeInfoExIndex & 0xFFF] = rule;
		filter_rule->SSDTRuleTableBase[service_index] = rule;
	}
	return;
}

//���ù�������
ULONG NTAPI HookPort_SetFilterRuleName(IN PFILTERFUN_RULE_TABLE FilterFun_Rule_table_head, IN CHAR* FilterRuleName)
{
	ULONG BuffLenMax = 0xF;			//����������󳤶�
	ULONG BuffLen = 0;
	RtlZeroMemory(FilterFun_Rule_table_head->FilterRuleName, sizeof(FilterFun_Rule_table_head->FilterRuleName));
	BuffLen = strlen(FilterRuleName);
	if (BuffLen > BuffLenMax)
	{
		BuffLen = 0xF;
	}
	RtlCopyMemory(FilterFun_Rule_table_head->FilterRuleName, FilterRuleName, BuffLen);
	return BuffLen;
}

//���ù�����˺���
BOOLEAN NTAPI HookPort_SetFilterSwitchFunction(IN PFILTERFUN_RULE_TABLE filter_rule, IN ULONG index_a2, OUT PVOID func_addr)
{
	ULONG	service_index;
	ULONG   Local_Index;
	if ((index_a2 <= FILTERFUNCNT - 1) && (index_a2 != 12))
	{
		//1���������ģ����Ϣ��ģ���С��������
		if (func_addr && KeGetCurrentIrql() < DISPATCH_LEVEL)
		{
			if (filter_rule->FilterRuleName[0])
			{
				if (!filter_rule->pModuleBase)
				{
					HookPort_FindModuleBaseAddress(func_addr, &filter_rule->pModuleBase, &filter_rule->ModuleSize, 0, 0);
				}
			}
			else
			{
				HookPort_FindModuleBaseAddress(func_addr, &filter_rule->pModuleBase, &filter_rule->ModuleSize, &filter_rule->FilterRuleName, 0X10);
			}
		}

		service_index = filter_function_table[index_a2];

		//2�����˵���Ч����,2000����Ĭ��ֵ����Чֵ��
		if (service_index == g_SSDTServiceLimit) {
			filter_rule->FakeServiceRoutine[index_a2] = (PULONG)func_addr;
			return TRUE;
		}

		//3����ʼ��ShadowSSDT���֣�0x1000��
		if (_CHECK_IS_SHADOW_CALL(service_index))
		{
			g_SS_Filter_Table->SwitchTableForShadowSSDT[(service_index & 0xFFF)] = 1;
			filter_rule->FakeServiceRoutine[index_a2] = (PULONG)func_addr;
			return TRUE;
		}

		if (index_a2 == ZwLoad_Un_Driver_FilterIndex)
		{
			Local_Index = g_SSDT_Func_Index_Data.ZwUnloadDriverIndex;
		}
		else
		{
			if (index_a2 != ZwWriteFile_FilterIndex)
			{
				if (index_a2 == ZwCreateThread_FilterIndex)
				{
					if (g_SSDT_Func_Index_Data.ZwCreateThreadExIndex == g_SSDTServiceLimit)
					{
						g_SS_Filter_Table->SwitchTableForSSDT[service_index] = 1;
						filter_rule->FakeServiceRoutine[index_a2] = (PULONG)func_addr;
						return TRUE;
					}
					g_SS_Filter_Table->SwitchTableForSSDT[g_SSDT_Func_Index_Data.ZwCreateThreadExIndex] = 1;
				}
				if (index_a2 == ZwSetValueKey_FilterIndex && g_ShadowSSDT_Func_Index_Data.NtUserSetImeInfoExIndex != g_SSDTServiceLimit)
				{
					g_SS_Filter_Table->SwitchTableForShadowSSDT[(g_ShadowSSDT_Func_Index_Data.NtUserSetImeInfoExIndex & 0xFFF)] = 1;
				}
				g_SS_Filter_Table->SwitchTableForSSDT[service_index] = 1;
				filter_rule->FakeServiceRoutine[index_a2] = (PULONG)func_addr;
				return TRUE;
			}
			Local_Index = g_SSDT_Func_Index_Data.ZwWriteFileGatherIndex;
		}
		if (Local_Index != g_SSDTServiceLimit)
		{
			g_SS_Filter_Table->SwitchTableForSSDT[Local_Index] = 1;
			g_SS_Filter_Table->SwitchTableForSSDT[service_index] = 1;
			filter_rule->FakeServiceRoutine[index_a2] = (PULONG)func_addr;
			return TRUE;
		}
	}
	return 0;
}

//��ʼ�������ӿ�
ULONG NTAPI HookPort_InitDeviceExtInterface(IN PDEVICE_OBJECT DeviceObject)
{
	PHOOKPORT_EXTENSION pHookPortExt;
	pHookPortExt = DeviceObject->DeviceExtension;
	pHookPortExt->State = (PULONG)3;
	pHookPortExt->HookPort_FilterRule_Init = HookPort_AllocFilterRuleTable;					//��ʼ������
	pHookPortExt->HookPort_SetFilterSwitchFunction = HookPort_SetFilterSwitchFunction;		//���ù�����˺���
	pHookPortExt->HookPort_SetFilterRule = HookPort_SetFilterRule;							//���ù��򿪹�
	pHookPortExt->HookPort_SetFilterRuleName = HookPort_SetFilterRuleName;					//���ù�������
	pHookPortExt->Value3F1 = HOOKPORT_VERSION;												//�汾
	return pHookPortExt;
}

//��ȡ��������������˳�� 
ULONG HookPort_1858E(OUT ULONG *Flag_1, OUT ULONG *Flag_2, OUT PVOID *ValueDataBuff)
{
	BOOLEAN result; // eax@2
	ULONG LoadOrderIndex_Oneself; //����������LoadOrderIndex������˳��
	ULONG LoadOrderIndex_Assign; //ָ��������LoadOrderIndex������˳��
	if (HookPort_GetModuleBaseAddress_EncryptHash(Global_Hash_2, 0, 0, &LoadOrderIndex_Assign))
	{
		//��ȡ���������ļ���˳��
		//PCHunter������ģ��->����˳��
		result = HookPort_GetModuleLoadOrderIndex(Global_DriverObject->DriverStart, &LoadOrderIndex_Oneself);
		if (result)
		{
			result = 1;
			*Flag_1 = 1;
			*Flag_2 = LoadOrderIndex_Oneself < LoadOrderIndex_Assign;
			*ValueDataBuff = LoadOrderIndex_Assign;
		}
	}
	else
	{
		*Flag_1 = 0;
		result = 1;
		*Flag_2 = 1;
	}
	return result;
}

//�������ܣ�
//1�����������ж��Ƿ�����FakeKiSystemService��hook
//2����ʼ����չ�ṹ������������һ��sysʹ��
PVOID HookPort_19230()
{
	CHAR			Flag; // di@2 ���أ���ʾ����Ĳ�ͬ���ͣ�
	ULONG           Flag_1, Flag_2;
	PVOID           ValueDataBuff = NULL;
	PVOID			result;
	PVOID 			pModuleBase = NULL;
	ULONG 			ModuleSize = NULL;
	USHORT          LoadOrderIndex = NULL;
	//��ʼ����ϣֵ
	Global_Hash_1 = 0x42503C81;
	Global_Hash_2 = 0x4D71E020;
	Global_Hash_3 = 0xB8178767;
	Global_Hash_4 = 0x7848DA1;

	if (HookPort_GetModuleBaseAddress_EncryptHash(Global_Hash_1, &pModuleBase, &ModuleSize, 0))
	{
		Flag = 1;
		//δ�����Win7�����޷����룬���������HookPort_SetFakeKiSystemServiceData()������Fake_ZwSetEvent����ͬ������hook KiSystemService or KiFastCallEntry
		if (HookPort_CheckSysVersion(Global_osverinfo, g_HookPort_Nt_Win32k_Data.NtData.NtImageBase) == 1 && !HookPort_CheckCpuNumber(Global_osverinfo) && HookPort_SetFakeKiSystemServiceData(pModuleBase, ModuleSize))
		{
			HookPort_GetModuleBaseAddress_EncryptHash(Global_Hash_2, &dword_1B13C_ModuleBase, &dword_1B140_ModuleSize, 0);
			HookPort_GetModuleBaseAddress_EncryptHash(Global_Hash_3, &dword_1B14C_ModuleBase, &dword_1B150_ModuleSize, 0);
			return (PVOID)HookPort_GetModuleBaseAddress_EncryptHash(Global_Hash_4, &dword_1B144_ModuleBase, &dword_1B148_ModuleSize, 0);
		}
	}
	else
	{
		Flag = 1;
		dword_1B124 = 1;
	}
	//��ȡ��������������˳��
	result = (PVOID)HookPort_1858E(&Flag_1, &Flag_2, &ValueDataBuff);
	if (result)
	{
		if (Flag_1)
		{
			if (Flag_2)
			{
				//Int 2E KiSystemService Win2000���²���
				HookPort_SetFakeKiSystemServiceAddress();		//hook KiSystemService jmp ���Լ�������ֻ�滻��ַ��
			}
			if (!g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiServiceTableBase && !Flag_2)
			{
				//дע�����Ϣ
				//HookPort_RtlWriteRegistryValue_1(Flag, ValueDataBuff, 0);
			}
		}
		else
		{
			dword_1B120 = 1;
		}
		//��ʼ�������½������ӵ���������
		result = HookPort_AllocFilterRuleTable(3u);
		g_FilterFun_Rule_table_head_Temp = result;
		if (g_FilterFun_Rule_table_head_Temp)
		{
			//��ʼ����������
			HookPort_SetFilterRuleName(g_FilterFun_Rule_table_head_Temp, "HPATBK");
			//���ù�����˺���
			HookPort_SetFilterSwitchFunction(g_FilterFun_Rule_table_head_Temp, LoadImageNotifyRoutine_FilterIndex, Fake_LoadImageNotifyRoutine);
			//���ù�ؿ���
			HookPort_SetFilterRule(g_FilterFun_Rule_table_head_Temp, LoadImageNotifyRoutine_FilterIndex, 1);
			//׼������
			g_FilterFun_Rule_table_head_Temp->IsFilterFunFilledReady = 1;
		}
	}
	return result;
}

//************************************     
// ��������: HookPort_Close     
// ����˵����    
// IDA��ַ ��sub_1060C
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�     
// �� �� ֵ: NTSTATUS     
// ��    ��: IN PDEVICE_OBJECT DeviceObject     
// ��    ��: IN PIRP Irp     
//************************************  
NTSTATUS	HookPort_Close(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	)
{
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IofCompleteRequest(Irp, IO_NO_INCREMENT);
	return 0;
}

//������Ȥ��ͨ�ô���
NTSTATUS HookPort_CommonProc(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	)
{
	//ֱ����ɣ����سɹ�
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS	HookPort_Create(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	)
{
	NTSTATUS			     Status = STATUS_SUCCESS;
	PIO_STACK_LOCATION	     IrpStack;
	SECURITY_SUBJECT_CONTEXT Create_SubjectSecurityContext = { 0 };
	PIO_SECURITY_CONTEXT     Create_SecurityContext = NULL;
	PACCESS_STATE			 Create_AccessState = NULL;
	BOOLEAN                  SeCaptureSubjectContext_Flag = FALSE;
	Irp->IoStatus.Information = 0;
	IrpStack = IoGetCurrentIrpStackLocation(Irp);
    Create_SecurityContext = IrpStack->Parameters.Create.SecurityContext;
	Create_AccessState = Create_SecurityContext->AccessState;
	//1��������Ӧ�ò����
	if (Irp->RequestorMode)
	{
		//2������ľ��ǻ�ȡȨ�ޣ�Ȼ���ж�Ȩ���Ƿ���administrators����
		if (Create_AccessState)
		{
			//��ȡSubjectSecurityContext
			RtlCopyMemory(&Create_SubjectSecurityContext, &Create_AccessState->SubjectSecurityContext, sizeof(SECURITY_SUBJECT_CONTEXT));
		}
		else
		{
			//�����ھͺ�����ȡ�ˣ������Ҫ�ͷŵ�
			SeCaptureSubjectContext(&Create_SubjectSecurityContext);
			SeCaptureSubjectContext_Flag = TRUE;
		}
		//3����Ҫ�ж�ClientToken or PrimaryToken�Ƿ���administrators����
		if (!Create_SubjectSecurityContext.ClientToken && Create_SubjectSecurityContext.PrimaryToken)
		{
			//���ư�������administrators�鷵���棬�����
			if (SeTokenIsAdmin(Create_SubjectSecurityContext.ClientToken))
			{
				//�Ϸ�����
				Status = STATUS_SUCCESS;
			}
			else
			{
				//Ȩ�޲���
				Status = STATUS_PRIVILEGE_NOT_HELD;
			}
		}
		//�ж����Ƶȼ���ֵԽ������Խ��
		else if (Create_SubjectSecurityContext.ClientToken && Create_SubjectSecurityContext.ImpersonationLevel > SecurityIdentification)
		{
			//���ư�������administrators�鷵���棬�����
			if (SeTokenIsAdmin(Create_SubjectSecurityContext.ClientToken))
			{
				//�Ϸ�����
				Status = STATUS_SUCCESS;
			}
			else
			{
				//Ȩ�޲���
				Status = STATUS_PRIVILEGE_NOT_HELD;
			}
		}
		else
		{
			//��������ֱ�Ӵ��󷵻�
			Status = STATUS_PRIVILEGE_NOT_HELD;
		}
		if (SeCaptureSubjectContext_Flag)
		{
			//ȡ����0
			SeCaptureSubjectContext_Flag = FALSE;
			//�ͷ�
			SeReleaseSubjectContext(&Create_SubjectSecurityContext);
		}
	}
	Irp->IoStatus.Status = Status;			//��ʾIRP���״̬
	IofCompleteRequest(Irp, IO_NO_INCREMENT);
	return Status;
}

NTSTATUS	HookPort_DeviceControl(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	)
{
	NTSTATUS			Status = STATUS_SUCCESS;
	PIO_STACK_LOCATION	IrpStack;
	PLARGE_INTEGER		ioBuf;
	ULONG				inBufLength, outBufLength;
	ULONG				ioControlCode;
	Irp->IoStatus.Information = 0;
	IrpStack = IoGetCurrentIrpStackLocation(Irp);
	ioBuf = Irp->AssociatedIrp.SystemBuffer;									
	inBufLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;		
	outBufLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;		
	ioControlCode = IrpStack->Parameters.DeviceIoControl.IoControlCode;			
	//����Щ�Ƿ������Է�ʽ�����ӡ������Ϣ�ģ�������д��
	switch (ioControlCode)
	{
		case HOOKPORT_GETVER:
		{
			//��R3����һ��HookPort�İ汾�ţ��ǳ���Ҫ��
			if (outBufLength < sizeof(ULONG))
			{
				//outBufLength< 4�Ͳ�������
				Status = STATUS_INVALID_PARAMETER;
			}
			else
			{
				//���ذ汾��
				ioBuf->LowPart = HOOKPORT_VERSION;
				//С��8�ֽ���4�ֽڣ�������8�ֽ�
				if (outBufLength < sizeof(ULONGLONG))
				{
					Irp->IoStatus.Information = sizeof(ULONG);
				}
				else
				{
					//��һ��������ڣ�������������
					ioBuf->HighPart = dword_1B114;
					Irp->IoStatus.Information = sizeof(ULONGLONG);
				}
			}
			break;
		}
		case HOOKPORT_DEBUGMEASSAGE1:
		{
			//�������������Ϣ�ģ����ӣ�
			if (inBufLength >= sizeof(ULONG))
			{
				DbgPrintf_dword_1B174 = ioBuf->LowPart;
			}
			break;
		}
		case HOOKPORT_DEBUGMEASSAGE2:
		{
			//�������������Ϣ�ģ����ӣ�
			if (inBufLength >= sizeof(ULONG))
			{
				DbgPrintf_dword_1AFA0 = ioBuf->LowPart;
			}
			break;
		}
		case HOOKPORT_DEBUGMEASSAGE3:
		{
			//�������������Ϣ�ģ����ӣ�
			if (inBufLength >= sizeof(ULONG))
			{
				DbgPrintf_dword_1B178 = ioBuf->LowPart;
			}
			break;
		}
		case HOOKPORT_DEBUGMEASSAGE4:
		{
			//R3����һ��ʱ�䣬A�㵽B��ִ�д���ʱ�����С�ڸ�ʱ��Ŵ�ӡ������Ϣ���ѵ���ֹ������
			//һ������ж�ʱ���Ƿǳ��̵�
			if (inBufLength >= sizeof(ULONG))
			{
				DbgPrintf_qdword_1AFB0 = ioBuf->QuadPart;
			}
			break;
		}
		default:
		{
			//���󷵻أ�����������Ч��
			KdPrint(("Unknown ioControlCode:%X\t\n", ioControlCode));
			Status = STATUS_INVALID_DEVICE_REQUEST;
			break;
		}
	}
	Irp->IoStatus.Status = Status;							//��ʾIRP���״̬
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Status;
}

//************************************     
// ��������: DriverUnload     
// ����˵����ж�غ������Լ��������д�ģ�ԭ��������û��ж�غ����ģ�    
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2019/12/05     
// �� �� ֵ: VOID     
// ��    ��: IN PDRIVER_OBJECT DriverObject     
//************************************  
VOID DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
	KdPrint(("ж�سɹ�\t\n"));
	return;
}

//************************************     
// ��������: DriverEntry     
// ����˵���������������     
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2019/11/29     
// �� �� ֵ: NTSTATUS     
// ��    ��: IN PDRIVER_OBJECT DriverObj     
// ��    ��: IN PUNICODE_STRING RegPath     
//************************************  
NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT  DriverObject,		//������������������
	IN PUNICODE_STRING RegPath				//������·������ע�����
	)
{
	NTSTATUS Status = STATUS_INVALID_DEVICE_REQUEST;
	UNICODE_STRING	SymbolicLinkName;
	UNICODE_STRING	DestinationString;
	PDEVICE_OBJECT	DeviceObject = NULL;
	UNREFERENCED_PARAMETER(RegPath);
	Global_DriverObject = (ULONG)DriverObject;
	//1����ȡ�汾��Ϣ
	Status = HookPort_PsGetVersion();
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("HookPort: HookPort_PsGetVersion failed,err=%08x\t\n", Status));
		return Status;
	}
	//2����ȫģʽ�²�����
	if (*(ULONG*)InitSafeBootMode)
	{
		if (*(ULONG*)InitSafeBootMode == 1)
		{
			Status = RtlCheckRegistryKey(RTL_REGISTRY_CONTROL, HookPort_Minimal);
		}
		else
		{
			if (*(ULONG*)InitSafeBootMode <= 1u || *(ULONG*)InitSafeBootMode > 3u)
			{
				return STATUS_NOT_SAFE_MODE_DRIVER;
			}
			Status = RtlCheckRegistryKey(RTL_REGISTRY_CONTROL, HookPort_Network);
		}
		if (Status < 0)
			return STATUS_NOT_SAFE_MODE_DRIVER;
	}
	//2�������豸
	RtlInitUnicodeString(&DestinationString, HookPort_DeviceName);
	RtlInitUnicodeString(&SymbolicLinkName, HookPort_LinkName);
	Status = IoCreateDevice(
		DriverObject,
		sizeof(HOOKPORT_EXTENSION),		  //��չ18u                                  
		&DestinationString,
		FILE_DEVICE_UNKNOWN,			 //#define FILE_DEVICE_UNKNOWN             0x00000022
		FILE_DEVICE_SECURE_OPEN,		 // DeviceCharacteristics   ,#define FILE_DEVICE_SECURE_OPEN                 0x00000100                        
		FALSE,
		&DeviceObject);
	if (!NT_SUCCESS(Status))
	{

		KdPrint(("HookPort: DriverEntry IoCreateDevice failed,err=%08x\t\n", Status));
		return Status;
	}

	//3�����豸����һ����������
	Status = IoCreateSymbolicLink(&SymbolicLinkName, &DestinationString);
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("HookPort: DriverEntry IoCreateSymbolicLink failed,err=%08x\t\n", Status));
		IoDeleteDevice(DeviceObject);
		return Status;
	}

	//4�� ������Ȥ��ͨ�ô���
	for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		DriverObject->MajorFunction[i] = HookPort_CommonProc;
	}
	//4��1 DeviceControl����Щ����������Ϣ��ص�ֱ������
	DriverObject->MajorFunction[IRP_MJ_CREATE] = HookPort_Create;					 //�ж�Ȩ������֮���
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = HookPort_Close;						 //��
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HookPort_DeviceControl;     //��ȡHookPort�汾�����ÿ���������Ϣ��־λ

	//5����ʼ�����ָ���hook���������̡��̻߳ص��ȵ�
	Status = HookPort_InitSDT();
	if (!NT_SUCCESS(Status))
	{
		IoDeleteSymbolicLink(&SymbolicLinkName);
		IoDeleteDevice(DeviceObject);
		KdPrint(("HookPort: HookPort_InitSDT failed,err=%08x\t\n", Status));
		return STATUS_UNSUCCESSFUL;
	}
	//6����ʼ�������ӿں���
	HookPort_InitDeviceExtInterface(DeviceObject);
	//7��
	//1�����������ж��Ƿ�����FakeKiSystemService��hook
	//2����ʼ����չ�ṹ������������һ��sysʹ��
	HookPort_19230();
	KdPrint(("HookPort: 360HookPort�������سɹ�\t\n"));
	DriverObject->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}