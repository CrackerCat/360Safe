#include "KiFastCallEntry.h"

//Dpc����ʹ��
TsFlt_DPC g_TsFltDpcInfo = { 0 };
//����Int2E KiSystemServiceHook��Ĵ���

//
//	�����ڰ�װKiFastCallEntryʱ������λ�ҹ�λ�õ�ָ��ֵ
//	���ҵ������е����ֵ�Ͱ�������ָ���滻Ϊһ�����ǵ�JMP����g_KiFastCallEntry_eb_jmp_address
//	2be1            sub    esp,ecx 
//	c1e902          shr    ecx,2   
//
CHAR g_KiFastCallEntry_Condition_Code[5] = { 0x2B, 0xE1, 0xC1, 0xE9, 0x02 };

//loc_1AEE8
CHAR g_FindKiSystemService_HookPoint_Code[] = {
	0x8B, 0xFC,										//mov     edi, esp
	0x3B, 0x35, 0x00, 0x00, 0x00, 0x00,				//cmp     esi, large ds:0				//��Ҫ�޸��ĵط�  �޸���- >cmp     esi,dword ptr [nt!MmUserProbeAddress (841a571c)]
	0x0F, 0x83, 0x00, 0x00, 0x8B, 0xFC,				//jnb     near ptr 0FC8CAEF6h			//
	0xF6, 0x45, 0x72, 0x02,							//test    byte ptr [ebp+72h]
	0x75, 0x06,										//jnz     short loc_1AF02
	0xF6, 0x45, 0x6C, 0x01,							//test    byte ptr [ebp+6Ch]
	0x74, 0x0C,										//jz      short loc_1AF0E
	0x3B, 0x35, 0x00, 0x00, 0x00, 0x00,				//cmp     esi, large ds:0				//��Ҫ�޸��ĵط�   �޸���- >cmp     esi, dword ptr[nt!MmUserProbeAddress(841a571c)]
	0x0F, 0x83, 0x00, 0x00, 0x32, 0x36,				//jnb     near ptr 3633AF0Eh			
	0x30, 0x30,										//xor     [eax], dh
	0x00, 0x36,										//add     [esi], dh
	0x30, 0x30,										//xor     [eax], dh
	0x30, 0x00,										//xor     [eax], al 
	0x36, 0x30, 0x30,								//xor     ss:[eax], dh
	0x31, 0x00,										//xor     [eax], eax
	0x36, 0x30, 0x30,								//xor     ss:[eax], dh
	0x32, 0x00,										//xor     al, [eax]
	0x37,											//aaa
	0x36, 0x30, 0x30,								// xor     ss:[eax], dh
	0x00, 0x37,										//add     [edi], dh  
	0x36, 0x30, 0x31,								//xor     ss:[ecx], dh 
	0x00, 0x39,										//add     [ecx], bh
	0x32, 0x30,										//xor     dh, [eax]
	0x30, 0x00,										//xor     [eax], al
	0x39, 0x36,										//cmp     [esi], esi
	0x30, 0x30										//xor     [eax], dh
};

//һ��shellcode
CHAR Hook_ShellCode_sub_15520[] = {							//sub_15520
	0XFF, 0X35, 0XFF, 0XFF, 0XFF, 0XFF,						// push    dword_1B0E4     -> ������ push  A
	0x81, 0x34, 0x24, 0x00, 0x36, 0x00, 0x36,				// xor     [esp+4+var_4], 36003600h
	0xC3,													// retn 
	0X90, 0X90, 0X90, 0X90,                                 // ���ĸ��ֽ�������·�ģ�������ʹ��(��ΪA)
	0X90, 0X90, 0X90, 0X90, 0X90, 0X90, 0X90, 0X90, 0X90    // ��Щ�ֽڶ������õģ�������
};

CHAR Hook_ShellCode_sub_15538[] = {							//sub_15538
	0x8B, 0xFF,												// mov    edi,edi 
	0x9C,													// pushf
	0XFF, 0X35, 0XFF, 0XFF, 0XFF, 0XFF,						// push    dword_1B0E4	   -> ������ push  A
	0x81, 0x34, 0x24, 0x00, 0x36, 0x00, 0x36,				// xor     [esp+8+var_8], 36003600h
	0xC3,													// retn 
	0X90, 0X90, 0X90, 0X90,                                 // ���ĸ��ֽ�������·�ģ�������ʹ��(��ΪA)
	0X90, 0X90, 0X90, 0X90, 0X90, 0X90, 0X90, 0X90, 0X90    // ��Щ�ֽڶ������õģ�������
};

//************************************     
// ��������: Hookport_Common_KiFastCallEntry_IDT     
// ����˵����VISTA֮ǰ��ϵͳ����IDT 4��ʽhook  
//           sub     esp, ecx    -> Int 4
//           shr     ecx, 2
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2019/12/31     
// �� �� ֵ:      
//************************************  
__declspec(naked) Hookport_Common_KiFastCallEntry_IDT()
{
	_asm
	{
			mov     edi, edi
			push    eax
			mov     eax, g_KiFastCallEntry_360HookPoint
			cmp     eax, [esp + 4]
			jz      loc_155A8
			pop     eax
			push[esp + 0x8]
			popfd
			jmp     Global_KiTrap04
		loc_155A8 :
			pop     eax
			add     esp, 8
			pushad
			push    edi									//ediָ��KiServiceTable��W32pServiceTable 
			push    ebx									//ebx��ԭʼ��KiFastCallEntry��SSDT��ȡ���ķ�������ַ 
			push    eax									//eax�Ƿ����
			call    HookPort_KiFastCallEntryFilterFunc  //KiFastCallEntryFilterFunc �������ַ���������ô���õģ�����ʱû�п������õĴ��룬�ѵ��Ǳ���֮���޸ĵģ�
			mov[esp + 0x10], eax					//��Ψһ����
			popad
			popfd
			sub     esp, ecx
			push    g_KiFastCallEntry_360HookPoint      //�����ǻ����ĵ�ַ��g_KiFastCallEntry_Fake_rtn_address��,push/ret��ʽ����ȥ�������ַ���������ô���õģ�����ʱû�п������õĴ��룬�ѵ��Ǳ���֮���޸ĵģ�
			retn
	}
}

//************************************     
// ��������: Hookport_High_KiFastCallEntry_IDT     
// ����˵����VISTA֮���ϵͳ����IDT 4��ʽhook 
//           sub     esp, ecx    -> Int 4
//           shr     ecx, 2
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2019/12/31     
// �� �� ֵ:      
//************************************ 
__declspec(naked) Hookport_High_KiFastCallEntry_IDT()
{
	_asm
	{
		mov     edi, edi
		push    eax
		mov     eax, g_KiFastCallEntry_360HookPoint
		cmp     eax, [esp + 4]
		jz      loc_155A8
		pop     eax
		push[esp + 0x8]
		popfd
		jmp     Global_KiTrap04
		loc_155A8 :
		pop     eax
		add     esp, 8
		pushad
		push    edi									//ediָ��KiServiceTable��W32pServiceTable 
		push    edx									//edx��ԭʼ��KiFastCallEntry��SSDT��ȡ���ķ�������ַ 
		push    eax									//eax�Ƿ����
		call    HookPort_KiFastCallEntryFilterFunc  //KiFastCallEntryFilterFunc �������ַ���������ô���õģ�����ʱû�п������õĴ��룬�ѵ��Ǳ���֮���޸ĵģ�
		mov[esp + 0x14], eax					//��Ψһ����
		popad
		popfd
		sub     esp, ecx
		push    g_KiFastCallEntry_360HookPoint  //�����ǻ����ĵ�ַ��g_KiFastCallEntry_Fake_rtn_address��,push/ret��ʽ����ȥ�������ַ���������ô���õģ�����ʱû�п������õĴ��룬�ѵ��Ǳ���֮���޸ĵģ�
		retn
	}
}


//************************************     
// ��������: Hookport_Common_KiFastCallEntry     
// ����˵����VISTA֮ǰ��ϵͳ���ô�ͳ��ʽhook   
//           sub     esp, ecx    -> JMP  Fake_KiFastCallEntry
//           shr     ecx, 2
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2019/12/31     
// �� �� ֵ:      
//************************************ 
__declspec(naked) Hookport_Common_KiFastCallEntry()
{
	_asm{
			mov    edi, edi
			pushad
			push    edi									//ediָ��KiServiceTable��W32pServiceTable 
			push    ebx									//ebx��ԭʼ��KiFastCallEntry��SSDT��ȡ���ķ�������ַ 
			push    eax									//eax�Ƿ����
			call    HookPort_KiFastCallEntryFilterFunc  //KiFastCallEntryFilterFunc �������ַ���������ô���õģ�����ʱû�п������õĴ��룬�ѵ��Ǳ���֮���޸ĵģ�
			mov[esp + 0x10], eax					//��Ψһ����
			popad
			popfd
			sub     esp, ecx
			shr     ecx, 2
			push    g_KiFastCallEntry_Fake_rtn_address  //�����ǻ����ĵ�ַ��g_KiFastCallEntry_Fake_rtn_address��,push/ret��ʽ����ȥ�������ַ���������ô���õģ�����ʱû�п������õĴ��룬�ѵ��Ǳ���֮���޸ĵģ�
			retn
	}
}

//************************************     
// ��������: Hookport_High_KiFastCallEntry     
// ����˵����VISTA֮���ϵͳ���ô�ͳ��ʽhook   
//           sub     esp, ecx    -> JMP  Fake_KiFastCallEntry
//           shr     ecx, 2
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2019/12/31     
// �� �� ֵ:      
//************************************ 
__declspec(naked) Hookport_High_KiFastCallEntry()
{
	_asm{
			mov    edi, edi
			pushad
			push    edi									//ediָ��KiServiceTable��W32pServiceTable 
			push    edx									//edx��ԭʼ��KiFastCallEntry��SSDT��ȡ���ķ�������ַ 
			push    eax									//eax�Ƿ����
			call    HookPort_KiFastCallEntryFilterFunc  //KiFastCallEntryFilterFunc 
			mov[esp + 0x14], eax						//��Ψһ����
			popad
			popfd
			sub     esp, ecx
			shr     ecx, 2
			push    g_KiFastCallEntry_Fake_rtn_address  //�����ǻ����ĵ�ַ��g_KiFastCallEntry_Fake_rtn_address��,push/ret��ʽ����ȥ
			retn
	}
}

//��������
VOID NTAPI HookPort_InlineHook5Byte_1521C(ULONG JmpAddress_a1, ULONG MdlAddress_a2, ULONG a3, ULONG a4)
{
	/*
	*(_DWORD *)MdlAddress_a2 = *(_DWORD *)JmpAddress_a1;
	*(_BYTE *)(MdlAddress_a2 + 4) = *(_BYTE *)(JmpAddress_a1 + 4);
	*/
	//�޸�ǰ->
	//	2be1            sub    esp,ecx 
	//	c1e902          shr    ecx,2   
	//�޸ĺ�->
	//  E9 XXXX         jmp     XXXXX
	//������ֱ��memcpy��û��ԭ�Ӳ���
	//.text:00015229 00C A5                                      movsd
	//.text:0001522A 00C A4                                      movsb
	RtlCopyMemory(MdlAddress_a2, JmpAddress_a1, 5);

}

//������5�ֽ�hook
VOID NTAPI HookPort_InterlockedCompareExchange64_15236(ULONG* a1, ULONG a2, ULONG a3, ULONG a4)
{
	ULONG v4 = 0; // ebx@1
	ULONG v5 = 0; // ecx@1
	bool v6 = 0; // zf@1

	v4 = *a1;
	v5 = *(_DWORD*)(a2 + 4);
	LOBYTE(v5) = *((_BYTE*)a1 + 4);
	v6 = _InterlockedCompareExchange64((volatile signed __int64*)a2, __PAIR__(v5, *a1), *(_QWORD*)a2) == __PAIR__(v5, v4);
}

//��ȫû��������������ڸ������������
ULONG sub_1567A(IN RTL_OSVERSIONINFOEXW osverinfo)
{
	PSYSTEM_PROCESS_INFORMATION pInfo = NULL;
	PSYSTEM_PROCESS_INFORMATION pNextpInfo = NULL; // eax@2
	ULONG ThreadCount; // eax@13
	BOOLEAN Result=TRUE;
	ULONG v8 = NULL; // ebx@13
	LARGE_INTEGER   UserTime = { 0 };
	UNICODE_STRING String1 = { 0 };
	ULONG BuildNumber = osverinfo.dwBuildNumber;
	ULONG MinorVersion = osverinfo.dwMinorVersion;
	ULONG MajorVersion = osverinfo.dwMajorVersion;
	RtlInitUnicodeString(&String1, L"Registry");
	pInfo = HookPort_QuerySystemInformation(SystemProcessInformation);
	if (!pInfo)
	{
		return Result;
	}
	if (pInfo->NextEntryDelta == 0)
	{
	LABEL_8:
		ExFreePool(pInfo);
		return Result;
	}
	pNextpInfo = pInfo = (PSYSTEM_PROCESS_INFORMATION)((PUCHAR)pInfo + pInfo->NextEntryDelta);
	if (pNextpInfo)
	{
		if (MajorVersion != 10 || MinorVersion || BuildNumber < 17134)
			goto LABEL_11;
		if (!RtlEqualUnicodeString(&String1, &pNextpInfo->ProcessName, TRUE))     //ProcessName
			goto LABEL_8;
	}
	ThreadCount = pNextpInfo->ThreadCount;	//ThreadCount
	if (ThreadCount)
	{
		UserTime = pNextpInfo->Threads[v8].UserTime;
		do
		{
			if (UserTime.LowPart == 0x20)
				break;
			if (UserTime.LowPart == 0x1F)
				break;
			if (UserTime.LowPart == 0x26)
				break;
			if (UserTime.LowPart == 0x1E)
				break;
			++v8;
		} while (v8 < ThreadCount);
	}
	if (v8 == ThreadCount)
	{
		Result = FALSE;
		goto LABEL_12;
	}
LABEL_11:
	Result = TRUE;
LABEL_12:
	ExFreePool(pInfo);
	return Result;
}


//���ģʽ��hook�޸�
VOID NTAPI DeferredRoutine1(
	IN struct _KDPC   *Dpc,
	IN PVOID   DeferredContext,
	IN PVOID   SystemArgument1,
	IN PVOID   SystemArgument2)
{
	KIRQL OldIrql; // bl@1
	PTsFlt_DPC pTsFltDpcInfo = (PTsFlt_DPC)DeferredContext;

	OldIrql = KfRaiseIrql(DISPATCH_LEVEL);
	InterlockedIncrement((LONG *)pTsFltDpcInfo->pFlag);
	KefAcquireSpinLockAtDpcLevel(pTsFltDpcInfo->pSpinLock);
	KefReleaseSpinLockFromDpcLevel(pTsFltDpcInfo->pSpinLock);
	KfLowerIrql(OldIrql);
}

//************************************     
// ��������: HookPort_Hook_153D0     
// ����˵����hook    
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2019/12/31     
// �� �� ֵ: NTSTATUS                               �ɹ�����0��ʧ�ܷ���258
// ��    ��: VOID     
// ��    ��: NTAPI * Hook                       ������ת��ַ�ĺ���   
// ��    ��: ULONG Jmp_Address                      Hook��ת�ĵ�ַ
// ��    ��: ULONG KiFastCallEntry_360HookPoint     Hook��
// ��    ��: ULONG a4                               ����
// ��    ��: ULONG a5                               ����
//************************************  
NTSTATUS HookPort_Hook_153D0(VOID(NTAPI *Hook)(ULONG, ULONG, ULONG, ULONG), ULONG Jmp_Address, ULONG KiFastCallEntry_360HookPoint, ULONG a4, ULONG a5)
{
	ULONG CpuNumber = 32;			//CPU�������Ӧ�ò�����32
	KAFFINITY ActiveProcessors_v5;
	ULONG NumberOfCpu_v6;
	KIRQL oldIrql_v8;
	PKDPC pDpc_v10, pDpc_v11;
	KIRQL NewIrql;
	ULONG nCurCpu_v18;
	ULONG nCount_v15;				//����CPU������
	ULONG nLoopTimes_v13;			//��ʱ����룬Ϊ�˸���ȫhook��ȫ������
	ULONG Numbera;
	ULONG Flag;
	nLoopTimes_v13 = 100000;
	nCurCpu_v18 = 0;
	nCount_v15 = 0;
	Numbera = 0;
	Flag = 0;
	NumberOfCpu_v6 = 0;
	if (MmIsAddressValid(Hook))
	{
		//ͳ��CPU����
		ActiveProcessors_v5 = KeQueryActiveProcessors();
		for (ULONG i_v7 = 0; i_v7 < CpuNumber; i_v7++)
		{
			if ((ActiveProcessors_v5 >> i_v7) & 1)
			{
				++NumberOfCpu_v6;
			}
		}
		//�����ǵ���
		if (NumberOfCpu_v6 == 1)
		{
			oldIrql_v8 = KfRaiseIrql(DISPATCH_LEVEL);
			_disable();
			Hook(Jmp_Address, KiFastCallEntry_360HookPoint, a4, a5);
			_enable();
			KfLowerIrql(oldIrql_v8);
			return 0;
		}
		//�����Ƕ�� ������ǰcpu�����cpu����������ס
		else
		{
			g_TsFltDpcInfo.pSpinLock = &g_SpinLock_WhiteList;
			g_TsFltDpcInfo.pFlag = &g_DpcFlag_dword_1B41C;
			KeInitializeSpinLock(&g_SpinLock_WhiteList);
			for (ULONG i = 0; i < CpuNumber; i++)
			{
				pDpc_v11 = &g_Dpc[i];
				//����KeInitializeDpc���̳�ʼ��һ��DPC���󣬲�ע��CustomDpc�ö������̡�
				KeInitializeDpc(pDpc_v11, DeferredRoutine1, &g_TsFltDpcInfo);
				//��KeSetTargetProcessorDpc����ָ���Ĵ�������һ��DPC���̽������С�
				KeSetTargetProcessorDpc(pDpc_v11, i);
				//��KeSetImportanceDpc����ָ����DPC����������������С�
				KeSetImportanceDpc(pDpc_v11, HighImportance);
			}
			g_DpcFlag_dword_1B41C = 0;	
			NewIrql = KfAcquireSpinLock(&g_SpinLock_WhiteList);
			for (ULONG i_v12 = 0; i_v12 < CpuNumber; i_v12++)
			{
				pDpc_v10 = &g_Dpc[i_v12];
				if ((1 << i_v12) & ActiveProcessors_v5)
				{
					++nCount_v15;
					nCurCpu_v18 = __readfsdword(0x51);
					if (i_v12 != nCurCpu_v18)//�ǵ�ǰ���ģ���Dpc��ʽ����
					{
						KeInsertQueueDpc(pDpc_v10, 0, 0);
					}
				}
			}
			//��ʱ�����
			KeStallExecutionProcessor(0xAu);
			while (TRUE)
			{
				if (g_DpcFlag_dword_1B41C == nCount_v15 - 1)
				{
					Hook(Jmp_Address, KiFastCallEntry_360HookPoint, a4, a5);
					goto LABEL_21;
				}
				//��ʱʧ�ܷ���
				if (++Numbera >= nLoopTimes_v13)
				{
					break;
				}
				KeStallExecutionProcessor(0xAu);
			}
			Flag = 1;
		LABEL_21:
			//�ָ��������
			KfReleaseSpinLock(&g_SpinLock_WhiteList, NewIrql);
			if (Flag != 1)
			{
				return 0;
			}
			return 258;
		}
	}
	return 258;
}


//************************************     
// ��������: Fake_ZwSetEvent     
// ����˵����Hook�����ַ�ʽ��
//           �޸ģ�Jmpxxxx            Global_IdtHook_Or_InlineHook == 0  
//           �޸ģ�Int 4              Global_IdtHook_Or_InlineHook == 1
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2019/12/31     
// �� �� ֵ: ULONG NTAPI     
// ��    ��: HANDLE EventHandle     
// ��    ��: PULONG PreviousState     
//************************************  
NTSTATUS NTAPI Fake_ZwSetEvent(HANDLE EventHandle, PULONG PreviousState)
{
	ULONG           Return;
	volatile LONG   *ZwSetEventAddress;
	volatile LONG   *KiFastCallEntry_360HookPoint;			//360Hook��
	PMDL			MemoryDescriptorList;
	CHAR			SystemInformation = 0;
	PCHAR  			fake_fuc, ebp_value, address, cr0_value;
	PCHAR  			p_address;
	PCHAR           pBuffer_v4, pBuffer_v5, pBuffer_v11, pBuffer_v17, pBuffer_v21;
	ULONG   		n, m, Local_KiTrap04;
	UCHAR		    Jmp_Address[8] = { 0 };
	PMDL		    v27;
	ULONG		    BuildNumber  = Global_osverinfo.dwBuildNumber;
	ULONG		    MinorVersion = Global_osverinfo.dwMinorVersion;
	ULONG		    MajorVersion = Global_osverinfo.dwMajorVersion;
	v27 = 0;
	Jmp_Address[0] = 0xE9u;                                   // ����ָ�jmp xxxxx
	Global_IdtHook_Or_InlineHook = 1;				          //Ĭ����1
	Return = STATUS_SUCCESS;
	//1�����������û�ȡ��ȡKiFastCallEntry�ĵ�ַ
	if (EventHandle == (PHANDLE)Global_Fake_ZwSetEvent_Handle && !ExGetPreviousMode())
	{
		Global_ZwSetEventHookFlag = 1;
		//sub_1567A����ʵ�ڿ������������׵��ϸ������
		if (!Global_Win32kFlag && !sub_1567A(Global_osverinfo))
		{
			Global_IdtHook_Or_InlineHook = 0;
		}
		//��ȡCPU��Ŀ��CPU>32����1
		if (HookPort_CheckCpuNumber(Global_osverinfo) == 1)
		{
			Global_IdtHook_Or_InlineHook = 0;
		}
		ZwSetEventAddress = HookPort_LockMemory((DWORD)((PCHAR)g_HookPort_Nt_Win32k_Data.SSDTTable_Data.SSDT_KeServiceTableBase + 4 * g_SSDT_Func_Index_Data.ZwSetEventIndex), sizeof(ULONG), &MemoryDescriptorList, Global_Version_Win10_Flag);
		if (!ZwSetEventAddress)
		{
			if (MemoryDescriptorList)
				HookPort_RemoveLockMemory(MemoryDescriptorList);
			HookPort_RtlWriteRegistryValue(10);
			return STATUS_NO_MEMORY;
		}
		InterlockedExchange(ZwSetEventAddress, g_SSDT_Func_Index_Data.pZwSetEvent);			// �ָ�SSDT���ӣ�ZwSetEvent��
		if (MemoryDescriptorList)
		{
			HookPort_RemoveLockMemory(MemoryDescriptorList);
			MemoryDescriptorList = 0;
		}
		//_asm
		//{
		//	MOV  ebp_value, EBP
		//}

		//// ջ���ݻ�ȡ���ص�ַ[EBP+4]������ָ������������ʱ���ص�KiFastCallEntry�еĵ�ַ��
		//g_call_ring0_rtn_address = (PVOID)*(ULONG *)((char *)ebp_value + 4);
		g_call_ring0_rtn_address = _ReturnAddress();
		//�жϷ���ֵ�ĺϷ���
		if (g_call_ring0_rtn_address < g_HookPort_Nt_Win32k_Data.NtData.NtImageBase || g_call_ring0_rtn_address >((ULONG)g_HookPort_Nt_Win32k_Data.NtData.NtImageBase + g_HookPort_Nt_Win32k_Data.NtData.NtImageSize))
		{
			HookPort_RtlWriteRegistryValue(11);
			return STATUS_NOT_FOUND;
		}
		address = g_call_ring0_rtn_address;
		p_address = (char *)g_call_ring0_rtn_address;
		// ��������ָ�����100�ֽڷ�Χ�����Ա����ǰ�װHOOK
		for (m = 0; m <= 100; m++)
		{
			for (n = 0; n < 5; n++)
			{
				if (*p_address != g_KiFastCallEntry_Condition_Code[n])
				{
					break;
				}
				p_address++;
			}
			//�ҵ��˷���������
			if (n == 5)
			{
				//
				// �ҵ�����ָ��
				//

				// ��������ָ��֮����Ǹ���ַ�������Ӵ���֮��ķ��ص�ַ
				//840541a4 2be1            sub     esp, ecx     ��ʱaddress                            = 840541a4
				//840541a6 c1e902          shr     ecx, 2       ��ʱg_KiFastCallEntry_360HookPoint     = 840541a6
				//840541a9 8bfc            mov     edi, esp     ��ʱg_KiFastCallEntry_Fake_rtn_address = 840541a9
				g_KiFastCallEntry_Fake_rtn_address = address + 5;
				g_KiFastCallEntry_360HookPoint = address + 2;
				break;
			}
			address--;
			p_address = (char *)address;
		}
		//�ж��Ƿ����ʧ��
		if (m == 100 || !g_KiFastCallEntry_Fake_rtn_address || !g_KiFastCallEntry_360HookPoint)
		{
			HookPort_RtlWriteRegistryValue(12);
			return STATUS_NOT_FOUND;
		}
		//new�ռ�洢�������תָ��
		pBuffer_v4 = ExAllocatePoolWithTag(NonPagedPool, p_jmpstub_codeLen, HOOKPORT_POOLTAG2);
		RtlZeroMemory(pBuffer_v4, p_jmpstub_codeLen);
		p_jmpstub_code = pBuffer_v4;
		if (!pBuffer_v4)
		{
			HookPort_RtlWriteRegistryValue(15);
			return STATUS_NO_MEMORY;
		}
		//Ĭ����1  
		//�����1  ����:IdtHook4���ж� 
		//�����0  ����:InlineHook
		if (Global_IdtHook_Or_InlineHook)
		{
			//Win7
			if (MajorVersion != 5 || MinorVersion && MinorVersion != 1)
			{
				if (ZwQuerySystemInformation(0xC4, &SystemInformation, 4u, 0) < 0)// �����ڸ��û��0XC4��ѡ���������������������
				{
					*&SystemInformation = 0;
				}
				pBuffer_v4 = p_jmpstub_code;
			}
			pBuffer_v5 = Hook_ShellCode_sub_15520;
			if (*(unsigned short*)pBuffer_v5 == 0xFF8Bu)
			{
				pBuffer_v5 = Hook_ShellCode_sub_15520 + 2;
			}
			RtlCopyMemory(pBuffer_v4, pBuffer_v5, 0x12);
			//push xxxx
			*(PVOID*)((PCHAR)p_jmpstub_code + 0X2) = (PVOID)((PCHAR)p_jmpstub_code + 0XE);
			//��ȡIDT��4���жϵ�ַ
			Global_KiTrap04 = HookPort_GetInterruptFuncAddress(4);
			Local_KiTrap04 = Global_KiTrap04;
			if (Global_KiTrap04 < g_HookPort_Nt_Win32k_Data.NtData.NtImageBase || Global_KiTrap04 >((DWORD)g_HookPort_Nt_Win32k_Data.NtData.NtImageBase + g_HookPort_Nt_Win32k_Data.NtData.NtImageSize))
			{
				ExFreePool(p_jmpstub_code);
				HookPort_RtlWriteRegistryValue(0xE);
				return STATUS_NOT_SUPPORTED;
			}
			if (SystemInformation & 1)
			{
				ULONG v9 = 0;
				ULONG v10 = 0;
				ULONG KiTrap04FunSize = 0x80;
				while (1)
				{
					if (*(unsigned short*)((PCHAR)Global_KiTrap04 + v9) == 0xC483u && *(unsigned char*)((PCHAR)Global_KiTrap04 + v9 + 3) == 0xE9u)
					{
						v10 = (Global_KiTrap04 + *(unsigned short*)((PCHAR)Global_KiTrap04 + v9 + 4)) + v9 + 8;
						if (v10 > g_HookPort_Nt_Win32k_Data.NtData.NtImageBase && v10 < ((PCHAR)g_HookPort_Nt_Win32k_Data.NtData.NtImageBase + g_HookPort_Nt_Win32k_Data.NtData.NtImageSize))
							break;
					}
					if (++v9 >= KiTrap04FunSize)
						goto LABEL_47;
				}
				v27 = (Global_KiTrap04 + v9 + 4);
				Local_KiTrap04 = v10;
				Global_KiTrap04 = v10;
			}
		LABEL_47:
			//���4��nopλ��
			*(PVOID*)((PCHAR)p_jmpstub_code + 0XE) = Global_KiTrap04;
			//���ݸߵͰ汾���ֲ�ͬ��Shellcode
			pBuffer_v11 = Hookport_High_KiFastCallEntry_IDT;// VISTA֮���ϵͳ
			if (BuildNumber < 6000)
			{
				pBuffer_v11 = Hookport_Common_KiFastCallEntry_IDT;// VISTA֮ǰ��ϵͳ
			}
			//xor     [esp+4+var_4], 36003600h->(��������滻��)
			*(PVOID*)((PCHAR)p_jmpstub_code + 0X9) = Global_KiTrap04 ^ (ULONG)pBuffer_v11;
			//Mdlӳ��HookPoint
			KiFastCallEntry_360HookPoint = HookPort_LockMemory(
				(PVOID)((PCHAR)g_KiFastCallEntry_360HookPoint - 2),
				2u,
				&MemoryDescriptorList,
				Global_Version_Win10_Flag
				);
			//�жϺϷ���
			if (!KiFastCallEntry_360HookPoint)
			{
				goto Exit1;
			}
			//���￴�����ڸ���
			if (SystemInformation & 1 && v27)
			{
				ULONG v14 = (PCHAR)p_jmpstub_code - v27 - 4;
				volatile LONG *v15 = HookPort_LockMemory(v27, sizeof(ULONG), &v27, Global_Version_Win10_Flag);
				if (!v15)
				{
					if (v27)
					{
						HookPort_RemoveLockMemory(v27);
					}
					goto Exit2;
				}
				InterlockedExchange(v15, v14);
				if (v27)
				{
					HookPort_RemoveLockMemory(v27);
				}
			}
			else
			{
				//IDT4���ж�Hook,��ȷ����Ӧ����0
				Return = HookPort_Hook_IDT_152DA(HookPort_SetKiTrapXAddress, p_jmpstub_code);
				if (Return)
				{
					if (Return == 258)
					{
						HookPort_RtlWriteRegistryValue(0x11);
					}
					ExFreePool(p_jmpstub_code);
					p_jmpstub_code = 0;
					if (MemoryDescriptorList)
						HookPort_RemoveLockMemory(MemoryDescriptorList);
					return Return;
				}
			}
			//д��int4
			InterlockedExchange(KiFastCallEntry_360HookPoint, 0xE9C104CD);
			if (MemoryDescriptorList)
				HookPort_RemoveLockMemory(MemoryDescriptorList);
		}
		//��ͳJmp xxxx��ʽ
		else
		{
			//��һ��Ҫxor���ܵ�
			pBuffer_v17 = Hook_ShellCode_sub_15538;
			if (*(unsigned short*)pBuffer_v17 == 0xFF8Bu)
			{
				pBuffer_v17 = Hook_ShellCode_sub_15538 + 2;
			}
			RtlCopyMemory(pBuffer_v4, pBuffer_v17, 0x13);
			//push xxxx
			*(PVOID*)((PCHAR)p_jmpstub_code + 0X3) = (PVOID)((PCHAR)p_jmpstub_code + 0XF);
			//
			*(PVOID*)((PCHAR)p_jmpstub_code + 0XF) = (PVOID)((PCHAR)g_KiFastCallEntry_Fake_rtn_address - 5);
			//���ݰ汾�ж�ѡ�ò�ͬ��shellcode����
			if (BuildNumber < 6000)
			{
				pBuffer_v21 = Hookport_Common_KiFastCallEntry;
			}
			else
			{
				pBuffer_v21 = Hookport_High_KiFastCallEntry;
			}
			*(PVOID*)((PCHAR)p_jmpstub_code + 0XA) = (ULONG)pBuffer_v21 ^ (ULONG)((PCHAR)g_KiFastCallEntry_Fake_rtn_address - 5);
			//Mdlӳ��HookPoint
			KiFastCallEntry_360HookPoint = HookPort_LockMemory((PVOID)((PCHAR)g_KiFastCallEntry_Fake_rtn_address - 5), 5u, &MemoryDescriptorList, Global_Version_Win10_Flag);
			//�жϺϷ���
			if (!KiFastCallEntry_360HookPoint)
			{
			Exit1:
				if (MemoryDescriptorList)
				{
					HookPort_RemoveLockMemory(MemoryDescriptorList);
				}
			Exit2:
				HookPort_RtlWriteRegistryValue(16);
				Return = STATUS_NO_MEMORY;
			Exit3:
				ExFreePool(p_jmpstub_code);
				p_jmpstub_code = 0;
				return Return;
			}
			//��ȡ���ƫ��
			ULONG Offset_v30 = (ULONG)(p_jmpstub_code)-(ULONG)(g_KiFastCallEntry_Fake_rtn_address);
			//�����jmp XXXXX   
			*(ULONG *)&Jmp_Address[1] = Offset_v30;
			if (ExIsProcessorFeaturePresent(			//����ExIsProcessorFeaturePresent�����ѯҪָ���Ĵ������������Ĵ��ڡ�
				PF_COMPARE_EXCHANGE_DOUBLE)				//����������8�ֽڵĴ洢�������ȽϺͽ�����CMPXCHG8B��ָ��
				)
			{
				//��������5���ֽ�hook
				Return = HookPort_Hook_153D0(
					HookPort_InlineHook5Byte_1521C,		//Hook������ַ
					(ULONG)Jmp_Address,					//Jmp���Լ������ĵ�ַ
					(ULONG)KiFastCallEntry_360HookPoint,//Ҫ�޸ĵĵ�ַ
					0,									//����
					0);                                 //����
			}
			else
			{
				//������5�ֽ�hook
				Return = HookPort_Hook_153D0(
					(VOID(NTAPI *)(ULONG, ULONG, ULONG, ULONG))HookPort_InterlockedCompareExchange64_15236,	//Hook������ַ
					(ULONG)Jmp_Address,					  //Jmp���Լ������ĵ�ַ
					(ULONG)KiFastCallEntry_360HookPoint,  //Ҫ�޸ĵĵ�ַ
					0,									  //����
					0									  //����
					);
			}
			if (MemoryDescriptorList)
			{
				HookPort_RemoveLockMemory(MemoryDescriptorList);
			}
			if (Return)
			{
				if (Return == 258)
				{
					HookPort_RtlWriteRegistryValue(0x11);
				}
				goto Exit3;
			}
		}
		return STATUS_SUCCESS;
	}
	//2����������
	// ����ԭ����
	return ((NTSTATUS(NTAPI*)(HANDLE, PLONG))g_SSDT_Func_Index_Data.pZwSetEvent)(EventHandle, PreviousState);
}

NTSTATUS NTAPI HookPort_InstallZwSetEventHook()
{
	char *SymbolAddr = 0; // eax@1
	volatile LONG *Mdlv2_MappedSystemVa = 0; // eax@3
	NTSTATUS Result = 0; // edi@7
	STRING DestinationString; // [sp+0h] [bp-Ch]@1
	PMDL MemoryDescriptorList = 0; // [sp+8h] [bp-4h]@1
	MemoryDescriptorList = 0;
	RtlInitAnsiString(&DestinationString, "ZwSetEvent");
	SymbolAddr = HookPort_GetSymbolAddress(&DestinationString, g_HookPort_Nt_Win32k_Data.NtData.NtImageBase);
	if (SymbolAddr)
	{
		g_SSDT_Func_Index_Data.ZwSetEventIndex = *(DWORD *)(SymbolAddr + 1);
		PVOID NtSetEventAddress = (DWORD)((PCHAR)g_HookPort_Nt_Win32k_Data.SSDTTable_Data.SSDT_KeServiceTableBase + 4 * g_SSDT_Func_Index_Data.ZwSetEventIndex);
		Mdlv2_MappedSystemVa = HookPort_LockMemory(
			NtSetEventAddress,
			sizeof(ULONG),
			&MemoryDescriptorList,
			Global_Version_Win10_Flag
			);
		if (Mdlv2_MappedSystemVa)
		{
			g_SSDT_Func_Index_Data.pZwSetEvent = InterlockedExchange(Mdlv2_MappedSystemVa, Fake_ZwSetEvent);// ��װZwSetEvent��SSDT����,������ԭʼZwSetEvent�ģ��������лָ�
		}
		if (MemoryDescriptorList)
		{
			HookPort_RemoveLockMemory(MemoryDescriptorList);
		}
		Global_Fake_ZwSetEvent_Handle = (HANDLE)0x711E8525;				//��ٵ�ZwSetEvent��������ţ�
		Result = ZwSetEvent(Global_Fake_ZwSetEvent_Handle, 0);          //��һ���ض���α�������ZwSetEvent����
		if (!Global_ZwSetEventHookFlag)								    //hook��־λ���ɹ�1�����ɹ�0
		{
			HookPort_RtlWriteRegistryValue(10);
		}
	}
	else
	{
		HookPort_RtlWriteRegistryValue(8);
		Result = STATUS_UNSUCCESSFUL;
	}
	return Result;
}


//��ȡKiSystemService��Hook��
//����Hook��
//1:
//nt!KiFastCallEntry+0xe9:
//840791a9 8bfc            mov     edi, esp
//840791ab 3b351c571a84    cmp     esi, dword ptr[nt!MmUserProbeAddress(841a571c)]
//2:
//807f295c 8bfc            mov     edi, esp
//807f295e f6457202        test    byte ptr[ebp + 72h], 2
//807f2962 7506            jne     807f296a
//807f2964 f6456c01        test    byte ptr[ebp + 6Ch], 1
//807f2968 740c            je      807f2976
//807f296a 3b351c571a84    cmp     esi, dword ptr[nt!MmUserProbeAddress(841a571c)]
ULONG HookPort_GetKiSystemService_HookPoint(IN ULONG MmUserProbeAddress, IN ULONG NtImageBase, IN ULONG NtImageSize, OUT ULONG *Index)
{
	ULONG KiSystemServiceAddress;
	ULONG PageSize = 1024;						//1ҳ=1024���ֽ�
	ULONG LoopNumber;							//����
	UCHAR HookShellBuff[0x50] = { 0 };			//����hook���
	//1��ͨ��IDT��λ��KiSystemService����
	KiSystemServiceAddress = HookPort_GetKiSystemService_IDT();
	if (KiSystemServiceAddress)
	{
		//2���ж�KiSystemService��ַ�Ϸ���
		if ((KiSystemServiceAddress < (ULONG)NtImageBase) || ((ULONG)NtImageBase >((ULONG)NtImageBase + NtImageSize)))
		{
			goto Error;
		}
		//3�����涼�ǲ���hook�����
		RtlCopyMemory(HookShellBuff, g_FindKiSystemService_HookPoint_Code, 10);
		//�޸�ǰ
		//mov     edi, esp
		//cmp     esi, large ds:0				//��һ��		
		//�޸���
		//mov     edi, esp
		//cmp     esi,dword ptr [nt!MmUserProbeAddress (841a571c)]   //ע�⿴��һ��
		*(ULONG*)(HookShellBuff + 4) = (ULONG)MmUserProbeAddress;
		////��1�ַ�������
		for (LoopNumber = 0; LoopNumber < PageSize; LoopNumber++)
		{
			if (RtlCompareMemory((CONST VOID*)(KiSystemServiceAddress + LoopNumber), HookShellBuff, 0xA) == 0xA)
			{
				//�ҵ�hook��
				//nt!KiFastCallEntry+0xe9:
				//840791a9 8bfc            mov     edi, esp
				//840791ab 3b351c571a84    cmp     esi, dword ptr[nt!MmUserProbeAddress(841a571c)]
				break;
			}
		}
		//��2�ַ�������
		if (LoopNumber == PageSize)
		{
			RtlZeroMemory(HookShellBuff, sizeof(HookShellBuff));
			RtlCopyMemory(HookShellBuff, (CONST VOID*)(g_FindKiSystemService_HookPoint_Code + 0xC), 0x16);
			//�޸�ǰ
			//cmp     esi, large ds:0
			//jnb     near ptr 3633AF0Eh			//��һ��
			//�޸���
			//807f2968 740c            je      807f2976
			//807f296a 3b351c571a84    cmp     esi, dword ptr[nt!MmUserProbeAddress(841a571c)]
			*(ULONG*)(HookShellBuff + 0x10) = (ULONG)MmUserProbeAddress;
			for (LoopNumber = 0; LoopNumber < PageSize; LoopNumber++)
			{
				if (RtlCompareMemory((CONST VOID*)(KiSystemServiceAddress + LoopNumber), HookShellBuff, 0x16) == 0x16)
				{
					//2:
					//807f295c 8bfc            mov     edi, esp
					//807f295e f6457202        test    byte ptr[ebp + 72h], 2
					//807f2962 7506            jne     807f296a
					//807f2964 f6456c01        test    byte ptr[ebp + 6Ch], 1
					//807f2968 740c            je      807f2976
					//807f296a 3b351c571a84    cmp     esi, dword ptr[nt!MmUserProbeAddress(841a571c)]
					break;
				}
			}
			//�����Ҳ���ֱ����������ط��˳�
			if (LoopNumber == PageSize)
			{
				goto Error;
			}
			*Index = 0x10;
		}
		else
		{
			*Index = 4;
		}
		//����Hook��
		//1:
		//nt!KiFastCallEntry+0xe9:
		//840791a9 8bfc            mov     edi, esp
		//840791ab 3b351c571a84    cmp     esi, dword ptr[nt!MmUserProbeAddress(841a571c)]
		//2:
		//807f295c 8bfc            mov     edi, esp
		//807f295e f6457202        test    byte ptr[ebp + 72h], 2
		//807f2962 7506            jne     807f296a
		//807f2964 f6456c01        test    byte ptr[ebp + 6Ch], 1
		//807f2968 740c            je      807f2976
		//807f296a 3b351c571a84    cmp     esi, dword ptr[nt!MmUserProbeAddress(841a571c)]
		return LoopNumber + KiSystemServiceAddress;
	}
Error:
	return 0;
}

//�Ͱ汾 < 2003 KiSystemService
//InlineHook hook��KiSystemService
//�������ܣ�
//1��ֻ�滻��ַ���Լ���Fake��ַ��������FakeKiSystemService����������
BOOLEAN HookPort_SetFakeKiSystemServiceAddress()
{
	UNICODE_STRING UMmUserProbeAddress;
	PMDL MemoryDescriptorList;
	ULONG MmUserProbeAddress;
	volatile LONG * Mdlv4_KiSystemServiceHookPoint = NULL;
	PVOID pKiSystemService_HookPoint;
	BOOLEAN Result = FALSE;
	ULONG Index;
	PVOID pBuff;
	MemoryDescriptorList = 0;
	RtlInitUnicodeString(&UMmUserProbeAddress, L"MmUserProbeAddress");
	MmUserProbeAddress = MmGetSystemRoutineAddress(&UMmUserProbeAddress);
	if (MmUserProbeAddress)
	{
		//1�����صõ�hook��
		pKiSystemService_HookPoint = HookPort_GetKiSystemService_HookPoint(MmUserProbeAddress, (ULONG)g_HookPort_Nt_Win32k_Data.NtData.NtImageBase, g_HookPort_Nt_Win32k_Data.NtData.NtImageSize, &Index);
		//2���жϵ�ַ�Ϸ���
		if (pKiSystemService_HookPoint)
		{
			pBuff = ExAllocatePoolWithTag(NonPagedPool, sizeof(ULONG), HOOKPORT_POOLTAG6);
			if (pBuff)
			{
				RtlZeroMemory(pBuff, sizeof(ULONG));
				//Ȼ��ʹ��MDL��ȫhook
				Mdlv4_KiSystemServiceHookPoint = HookPort_LockMemory((PVOID)((ULONG)pKiSystemService_HookPoint + Index), sizeof(ULONG), (ULONG)&MemoryDescriptorList,Global_Version_Win10_Flag);
				if (Mdlv4_KiSystemServiceHookPoint)
				{
					//IATHook
					//����Index=4
					//�޸�ǰ
					//8404b1a9 8bfc            mov     edi, esp
					//8404b1ab 3b351c771784    cmp     esi, dword ptr[nt!MmUserProbeAddress(8417771c)] ��һ���滻����
					//8404b1b1 0f832e020000    jae     nt!KiSystemCallExit2 + 0xa5 (8404b3e5)
					//�޸ĺ�
					//8404b1a6 c1e902          shr     ecx, 2
					//8404b1a9 8bfc            mov     edi, esp
					//8404b1ab 3b35087a9886    cmp     esi, dword ptr ds : [86987A08h]   ��һ���滻����pBuff
					//����Index=0x10(δ����)
					InterlockedExchange(Mdlv4_KiSystemServiceHookPoint, (LONG)pBuff);
					//����
					g_Fake_KiSystemServiceFuncAddress = (ULONG)pBuff;// �������ǹ����hook������ַ
				}
				//�ͷ�
				if (MemoryDescriptorList)
				{
					HookPort_RemoveLockMemory(MemoryDescriptorList);
				}
				Result = TRUE;
			}
		}
	}
	return Result;
}

//�п������
//��Fake_ZwSetEvent����һ��
ULONG HookPort_SetFakeKiSystemServiceData(ULONG ImageBase_a1, ULONG ImageSize_a2)
{
	return 0;
}