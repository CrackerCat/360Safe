#include "FilterHook.h"


//
// 
// ����������ݵ��úŵ��ù��˺���������һ��״ֵ̬���������жϽ��
// ����:
//	CallIndex			[INT]ϵͳ������ú�
//	ArgArray			[INT]ԭ�����Ĳ������飬���а�����ջ�б���ĸ÷��������еĲ���			
//	RetFuncArray		[OUT]�������غ���ָ�������,���Ϊ16������ָ��
//	RetFuncArgArray		[OUT]�뷵�صĺ���ָ���Ӧ��һ������,�ڵ���RetFuncArray�е�һ������ʱ��Ҫ�����ڱ������ж�Ӧ�Ĳ���
//  PULONG RetNumber    [OUT]����ֵ����!gFilterFun_Rule_table_head����0
//	Result				[OUT]����ֵ
// ����ֵ:
//	ʹ�� NT_SUCCESS ����в���
//
//sub_10DAC
NTSTATUS NTAPI HookPort_DoFilter(ULONG CallIndex, PHANDLE ArgArray, PULONG *RetFuncArray, PULONG *RetFuncArgArray, PULONG RetNumber, PULONG Result)
{

	ULONG		Index = 0;
	PULONG		ret_func;
	PULONG		ret_arg;
	ULONG       ServiceIndex = 0;
	NTSTATUS	status;
	ULONG       ebp_value, Rtn_Address;
	PFILTERFUN_RULE_TABLE	ptemp_rule;

	NTSTATUS(NTAPI *FilterFunc)(ULONG, PHANDLE, PULONG, PULONG);

	//���úų����������˺�������
	if (CallIndex >= FILTERFUNCNT)
	{
		return 1;
	}

	////ͨ��ebp��ȡretn����ֵ
	//_asm
	//{
	//	MOV  ebp_value, EBP
	//}
	//Rtn_Address = (PVOID)*(ULONG *)((CHAR *)ebp_value + 4);
	Rtn_Address = _ReturnAddress();
	//���˹�����Ϣ������
	if (!g_FilterFun_Rule_table_head)
	{
	LABEL_17:
		if (filter_function_table[CallIndex] != g_SSDTServiceLimit
			&& CallIndex != ZwSetValueKey_FilterIndex			//ZwSetValueKeyIndex
			&& CallIndex != ZwContinue_FilterIndex				//ZwContinueIndex
			&& Rtn_Address != g_call_ring0_rtn_address)
		{
			dword_1B114 = 1;
		}
		if (RetNumber)
		{
			*RetNumber = Index;
		}
		return 1;
	}

	ptemp_rule = g_FilterFun_Rule_table_head;
	//ULONG Number = RetFuncArray - RetFuncArgArray;
	//ִ���Լ�������鹹API������ֱ���ɹ�(һ����0x10�λ���)
	while (1)
	{
		// ���Ҷ�Ӧ�Ĺ��˺�����������֮
		if (ptemp_rule->IsFilterFunFilledReady
			&& ptemp_rule->FakeServiceRoutine[CallIndex])
		{

			ret_func = ret_arg = NULL;

			FilterFunc = (NTSTATUS(NTAPI *)(ULONG, PHANDLE, PULONG, PULONG))ptemp_rule->FakeServiceRoutine[CallIndex];

			status = FilterFunc(CallIndex, ArgArray, (PULONG)&ret_func, (PULONG)&ret_arg);

			if (ret_func && RetFuncArray && Index < 0x10)
			{
				++Index;
				*RetFuncArray++ = ret_func;
				*RetFuncArgArray++ = ret_arg;
			}
			//�жϹ����hook�����Ƿ�ִ�гɹ�
			if (status)
			{
				//ʧ�ܷ��أ�error��
				break;
			}
		}
		ptemp_rule = ptemp_rule->Next;
		//�����ǿ����˳����ǿռ�����һ��0x10�λ��ᣩ
		if (!(ULONG)ptemp_rule)
		{
			//�˳�������������⣩
			goto LABEL_17;
		}
	}
	//��Բ��ֲ�����Ȥ�ĺ����������ⷵ��
	ServiceIndex = filter_function_table[CallIndex];
	if (status == STATUS_HOOKPORT_FILTER_RULE_ERROR)
	{
		if (Result)
		{
			if (ServiceIndex == g_SSDTServiceLimit)
			{
				*Result = STATUS_HOOKPORT_FILTER_RULE_ERROR;
			}
			else
			{
				*Result = _CHECK_IS_SHADOW_CALL(ServiceIndex) && CallIndex != ZwUserBuildHwndList_FilterIndex && CallIndex != ZwUserSetInformationThread_FilterIndex;
			}
		}
	}
	else
	{
		if (ServiceIndex != g_SSDTServiceLimit && _CHECK_IS_SHADOW_CALL(ServiceIndex))
		{
			if (CallIndex != ZwUserBuildHwndList_FilterIndex && CallIndex != ZwUserSetInformationThread_FilterIndex)
			{
				if (Result)
				{
					*Result = 0;
				}
			}
			else if (Result)
			{
				*Result = status;
			}
		}
		else if (Result)
		{
			*Result = status;
		}
	}
	return STATUS_SUCCESS;

}

// ����FilterFunRuleTable���е�Rule((����PreviousMode�н�һ���ж�))���ж��Ƿ���ҪHook
BOOLEAN	NTAPI HookPort_HookOrNot(ULONG ServiceIndex, BOOLEAN GuiServiceCall)
{

	KPROCESSOR_MODE kpm;
	PFILTERFUN_RULE_TABLE	prule_table;
	ULONG	rule;

	//���ò���
	if (dword_1B110 == 1)
	{
		return FALSE;
	}

	if (ServiceIndex == g_SSDT_Func_Index_Data.ZwSetSystemInformationIndex)
	{
		return TRUE;
	}

	if (!g_FilterFun_Rule_table_head)
		return FALSE;

	kpm = ExGetPreviousMode();

	// GUIϵͳ����
	if (GuiServiceCall)
	{
		prule_table = g_FilterFun_Rule_table_head;
		do {
			rule = prule_table->ShadowSSDTRuleTableBase[ServiceIndex];
			if (RULE_MUST_HOOK == rule)
				return TRUE;
			if ((RULE_KERNEL_HOOK == rule && KernelMode == kpm)
				|| (RULE_GUI_HOOK == rule && UserMode == kpm)
				) {
				return TRUE;
			}
			prule_table = prule_table->Next;
		} while (prule_table);
		return 	FALSE;
	}

	// Kiϵͳ����	
	prule_table = g_FilterFun_Rule_table_head;
	do {
		rule = prule_table->SSDTRuleTableBase[ServiceIndex];
		if (RULE_MUST_HOOK == rule)
			return TRUE;
		if ((RULE_KERNEL_HOOK == rule && KernelMode == kpm)
			|| (RULE_GUI_HOOK == rule && UserMode == kpm)
			) {
			return TRUE;
		}
		prule_table = prule_table->Next;
	} while (prule_table);
	return FALSE;

}

//��ȡԭʼ��SSDT��ShadowSSDT��ַ
ULONG NTAPI HookPort_GetOriginalServiceRoutine(IN ULONG ServiceIndex)
{
	ULONG Index;
	ULONG ServiceTableBase = 0;
	ULONG Result =0;
	ULONG BuildNumber = Global_osverinfo.dwBuildNumber;
	ULONG MinorVersion = Global_osverinfo.dwMinorVersion;
	ULONG MajorVersion = Global_osverinfo.dwMajorVersion;
	Index = ServiceIndex;
	//SSSDT
	if (ServiceIndex & 0x1000)
	{
		if (BuildNumber < 14316)
		{
			ServiceTableBase = g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiServiceTableBase;
			Index = ServiceIndex & 0xFFF;
		}
		else
		{
			ServiceTableBase = *(ULONG *)(*((ULONG *)KeGetCurrentThread() + 0xF) + 0x10);
			Index = ServiceIndex & 0xFFF;
		}
		Result = *(ULONG*)(ServiceTableBase + 4 * Index);
	}
	//SSDT
	else
	{
		ServiceTableBase = g_HookPort_Nt_Win32k_Data.SSDTTable_Data.SSDT_KeServiceTableBase;
		Result = *(ULONG*)(ServiceTableBase + 4 * Index);
	}
	return Result;
}

//
// 
// 
// ����:
//	CallIndex			[INT]ϵͳ������ú�
//	ArgArray			[INT]ԭ�����Ĳ������飬���а�����ջ�б���ĸ÷��������еĲ���		
//	InResult			[INT]����ԭʼ�����ķ���ֵ
//	RetFuncArray		[INT]�������غ���ָ�������,���Ϊ16������ָ��
//	RetFuncArgArray		[INT]�뷵�صĺ���ָ���Ӧ��һ������,�ڵ���RetFuncArray�е�һ������ʱ��Ҫ�����ڱ������ж�Ӧ�Ĳ���
//	RetCount			[INT]�ظ������˼��Σ�һ�㶼��1�Σ�Ĭ��pFilterFun_Rule_table_head->Next�ǿյ�
// ����ֵ:
//	ʹ�� NT_SUCCESS ����в���
//
ULONG NTAPI HookPort_ForRunFuncTable(IN ULONG CallIndex, IN PHANDLE ArgArray, IN NTSTATUS InResult, IN PULONG *RetFuncArray, IN PULONG *RetFuncArgArray, IN ULONG  RetCount)
{
	NTSTATUS Status;
	NTSTATUS(NTAPI *pPostProcessPtr)(ULONG,		// ��    ��: IN ULONG FilterIndex        [In]Filter_ZwOpenFileIndex���
		PHANDLE,								// ��    ��: IN PVOID ArgArray           [In]ZwOpenFile�������׵�ַ
		NTSTATUS,								// ��    ��: IN NTSTATUS Result          [In]����ԭʼZwOpenFile����ֵ
		ULONG									// ��    ��: IN PULONG RetFuncArgArray   [In]]�뷵�صĺ���ָ���Ӧ��һ������,�ڵ���RetFuncArray�е�һ������ʱ��Ҫ�����ڱ������ж�Ӧ�Ĳ���
		);
	Status = InResult;
	for (ULONG i = 0; i < RetCount; i++)
	{
		pPostProcessPtr = RetFuncArray[i];
		if (pPostProcessPtr && MmIsAddressValid(pPostProcessPtr)) 
		{
			Status = pPostProcessPtr(CallIndex, ArgArray, InResult, RetFuncArgArray[i]);
			if (!NT_SUCCESS(Status))
			{
				break;
			}
		}
	}
	return Status;
}

//************************************     
// ��������: HookPort_KiFastCallEntryFilterFunc     
// ����˵�����κ�����JMPSTUB�б����ã����ݹ����ж��Ƿ���˴˴ε���    
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2019/12/31     
// �� �� ֵ: PULONG NTAPI     
// ��    ��: ULONG ServiceIndex     
// ��    ��: PULONG OriginalServiceRoutine     
// ��    ��: PULONG ServiceTable     
//************************************  
PULONG NTAPI HookPort_KiFastCallEntryFilterFunc(ULONG ServiceIndex, PULONG OriginalServiceRoutine, PULONG ServiceTable)
{
	ULONG BuildNumber = Global_osverinfo.dwBuildNumber;
	ULONG MinorVersion = Global_osverinfo.dwMinorVersion;
	ULONG MajorVersion = Global_osverinfo.dwMajorVersion;
	//�ж��Ƿ���SSDT�еĵ��� 
	if (ServiceTable == g_HookPort_Nt_Win32k_Data.SSDTTable_Data.SSDT_KeServiceTableBase && ServiceIndex <= g_HookPort_Nt_Win32k_Data.SSDTTable_Data.SSDT_KeNumberOfServices)
	{
		if (g_SS_Filter_Table->SwitchTableForSSDT[ServiceIndex] && HookPort_HookOrNot(ServiceIndex, FALSE))
		{
			g_SS_Filter_Table->SavedSSDTServiceAddress[ServiceIndex] = OriginalServiceRoutine;		//��������ԭʼ�����ĵ�ַ 
			return g_SS_Filter_Table->ProxySSDTServiceAddress[ServiceIndex];						//�������Ǵ������ĵ�ַ 
		}
		return OriginalServiceRoutine;
	}
	//�ж��Ƿ���ShadowSSDT�еĵ���,����ͬ��(�ж�win10_14316֮ǰ��汾����) 
	if ((ServiceTable == g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiServiceTableBase && ServiceIndex <= g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiNumberOfServices)
		|| (BuildNumber >= 14316
		&& ServiceTable == g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiServiceTableBase_Win10_14316
		&& ServiceIndex < g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiNumberOfServices_Win10_14316)
		)
	{
		if (g_SS_Filter_Table->SwitchTableForShadowSSDT[ServiceIndex] && HookPort_HookOrNot(ServiceIndex, TRUE))
		{
			g_SS_Filter_Table->SavedShadowSSDTServiceAddress[ServiceIndex] = OriginalServiceRoutine;		//��������ԭʼ�����ĵ�ַ 
			return g_SS_Filter_Table->ProxyShadowSSDTServiceAddress[ServiceIndex];
		}
	}
	return OriginalServiceRoutine; // ��������,��ֱ�ӷ���ԭʼ���� 
}

//************************************     
// ��������: HookPort_FilterHook     
// ����˵����������Ȥ��hook�����������ͨ�ô�������¾�ֱ������
//           ��4���ط���Ҫ��̬�޸�
//           sub     esp, 0BBBBBBBBh    ��Ҫ���ٶ��ռ�               
//			 push    0AAAAAAAAh         ����HookPort_DoFilter����
//           mov     ecx, 0CCCCCCCCh    ����memcpy������Ҫnew���ռ�
//           push    0DDDDDDDDh         ����sub_10A38����
//           push    0AAAAAAAAh         ����DbgPrint_17DA4����
//           retn    0EEEEh             ��Ҫ�ͷſռ�
// IDA��ַ ��sub_18082
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2019/12/31     
// �� �� ֵ:      
//************************************ 
__declspec(naked) __cdecl HookPort_FilterHook()
{

	/*
	��̬�޸���ָ�����:
	1��sub		esp, 0xbbbbbbbb��retn 0xeeee��һ��
	�޸���ʽ��  ����SSDT��SSSDT�ĵ��ĸ�ParamTableBase�����¼��ֵ�Ƕ�Ӧ�����Ĳ�������
	2��push     0xAAAAAAA
	�޸���ʽ�����push Index
	3��mov		ecx, 0xCCCCCCCC
	�޸���ʽ��  ����SSDT��SSSDT�ĵ��ĸ�ParamTableBase�����¼��ֵ�Ƕ�Ӧ�����Ĳ�������
	*/
	_asm{
			mov     edi, edi
			push    ebp
			mov		ebp, esp
			sub		esp, 0xBBBBBBBB							//��̬�޸�
			mov		[ebp - 0x4], esi
			mov		[ebp - 0x8], edi
			mov		[ebp - 0xC], ecx
			mov		[ebp - 0x10], edx
			lea		eax, [ebp - 0x14]
			push	eax
			lea		eax, [ebp - 0x18]
			push	eax
			lea		eax, [ebp - 0x58]
			push	eax
			lea		eax, [ebp - 0x98]
			push	eax
			lea		eax, [ebp + 0x8]
			push	eax
			push	0xAAAAAAAA								//��̬�޸�
			call    HookPort_DoFilter						//����������ݵ��úŵ��ù��˺���������һ��״ֵ̬���������жϽ��
			test	eax, eax
			mov		eax, [ebp - 0x14]
			jz      short Quit
			mov		edi, esp
			lea		esi, [ebp + 0x8]
			mov		ecx, 0xCCCCCCCC							//��̬�޸�
			rep		movsd
			push	0xDDDDDDDD								//��̬�޸�
			call    HookPort_GetOriginalServiceRoutine
			mov		esi, [ebp + 0x4]
			cmp     esi, g_call_ring0_rtn_address
			jnz     short loc_1810D
			mov     esi, dword_1B130
			test    esi, esi
			jz      short loc_1810D
			mov     esi, offset HookPort_FilterHook
			mov	    [ebp + 0x4], esi
			mov		esi, [ebp - 0x4]
			mov		edi, [ebp - 0x8]
			mov		ecx, [ebp - 0xC]
			mov		edx, [ebp - 0x10]
			call	eax
			mov     esi, g_call_ring0_rtn_address
			mov		[ebp + 0x4], esi
			jmp     short loc_1811B
		loc_1810D :
			mov		esi, [ebp - 0x4]
			mov		edi, [ebp - 0x8]
			mov		ecx, [ebp - 0xC]
			mov		edx, [ebp - 0x10]
			call	eax
		loc_1811B :
			mov     [ebp - 0x14], eax
			push    [ebp - 0x18]
			lea		eax, [ebp - 0x58]
			push	eax
			lea		eax, [ebp - 0x98]
			push	eax
			push    [ebp - 0x14]
			lea		eax, [ebp + 0x8]
			push	eax
			push	0xAAAAAAAA								//��̬�޸�
			call	HookPort_ForRunFuncTable
		Quit :
			mov		esi, [ebp - 0x4]
			mov		edi, [ebp - 0x8]
			mov		ecx, [ebp - 0xC]
			mov		edx, [ebp - 0x10]
			Leave											//Leave�������൱==mov esp,ebp��pop ebp
			retn	0xEEEE									//��̬�޸�
			nop												//��β��ʶ��
			nop
			nop
			nop
			push	0x4536251
			nop
			nop
			nop
			nop
	}
}

//�Լ�д�ĺ�����ȡHookPort_FilterHook�����ܴ�С
ULONG HookPort_PredictBlockEnd(ULONG uAddress, ULONG uSearchLength, UCHAR *Signature, ULONG SignatureLen)
{
	ULONG	Index;
	UCHAR	*p;
	ULONG	uRetAddress;

	if (uAddress == 0)
	{
		return 0;
	}
	p = (UCHAR*)uAddress;
	for (Index = 0; Index < uSearchLength; Index++)
	{
		if (memcmp(p, Signature, SignatureLen) == 0)
		{
			return p;
		}
		p++;
	}
	return 0;
}

//************************************     
// ��������: HookPort_InitFilterTable     
// ����˵������ʼ����������    
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2019/12/18     
// �� �� ֵ: ULONG     
//************************************  
ULONG HookPort_InitFilterTable()
{
	ULONG result; // eax@1

	filter_function_table[0] = g_SSDT_Func_Index_Data.ZwCreateKeyIndex;
	filter_function_table[1] = g_SSDT_Func_Index_Data.ZwQueryValueKeyIndex;
	filter_function_table[2] = g_SSDT_Func_Index_Data.ZwDeleteKeyIndex;
	filter_function_table[3] = g_SSDT_Func_Index_Data.ZwDeleteValueKeyIndex;
	filter_function_table[4] = g_SSDT_Func_Index_Data.ZwRenameKeyIndex;
	filter_function_table[5] = g_SSDT_Func_Index_Data.ZwReplaceKeyIndex;
	filter_function_table[6] = g_SSDT_Func_Index_Data.ZwRestoreKeyIndex;
	filter_function_table[7] = g_SSDT_Func_Index_Data.ZwSetValueKeyIndex;
	filter_function_table[8] = g_SSDT_Func_Index_Data.ZwCreateFileIndex;
	filter_function_table[9] = g_SSDT_Func_Index_Data.ZwFsControlFileIndex;
	filter_function_table[10] = g_SSDT_Func_Index_Data.ZwSetInformationFileIndex;
	filter_function_table[11] = g_SSDT_Func_Index_Data.ZwWriteFileIndex;
	filter_function_table[13] = g_SSDT_Func_Index_Data.ZwCreateProcessIndex;
	filter_function_table[14] = g_SSDT_Func_Index_Data.ZwCreateProcessExIndex;
	filter_function_table[15] = g_SSDT_Func_Index_Data.ZwCreateUserProcessIndex;
	filter_function_table[16] = g_SSDT_Func_Index_Data.ZwCreateThreadIndex;
	filter_function_table[17] = g_SSDT_Func_Index_Data.ZwOpenThreadIndex;
	filter_function_table[18] = g_SSDT_Func_Index_Data.ZwDeleteFileIndex;
	filter_function_table[19] = g_SSDT_Func_Index_Data.ZwOpenFileIndex;
	filter_function_table[20] = g_SSDT_Func_Index_Data.ZwReadVirtualMemoryIndex;
	filter_function_table[21] = g_SSDT_Func_Index_Data.ZwTerminateProcessIndex;
	filter_function_table[22] = g_SSDT_Func_Index_Data.ZwQueueApcThreadIndex;
	filter_function_table[23] = g_SSDT_Func_Index_Data.ZwSetContextThreadIndex;
	filter_function_table[24] = g_SSDT_Func_Index_Data.ZwSetInformationThreadIndex;
	filter_function_table[25] = g_SSDT_Func_Index_Data.ZwProtectVirtualMemoryIndex;
	filter_function_table[26] = g_SSDT_Func_Index_Data.ZwWriteVirtualMemoryIndex;
	filter_function_table[27] = g_SSDT_Func_Index_Data.ZwAdjustGroupsTokenIndex;
	filter_function_table[28] = g_SSDT_Func_Index_Data.ZwAdjustPrivilegesTokenIndex;
	filter_function_table[29] = g_SSDT_Func_Index_Data.ZwRequestWaitReplyPortIndex;
	filter_function_table[30] = g_SSDT_Func_Index_Data.ZwCreateSectionIndex;
	filter_function_table[31] = g_SSDT_Func_Index_Data.ZwOpenSectionIndex;
	filter_function_table[32] = g_SSDT_Func_Index_Data.ZwCreateSymbolicLinkObjectIndex;
	filter_function_table[33] = g_SSDT_Func_Index_Data.ZwOpenSymbolicLinkObjectIndex;
	filter_function_table[34] = g_SSDT_Func_Index_Data.ZwLoadDriverIndex;
	filter_function_table[35] = g_SSDT_Func_Index_Data.ZwQuerySystemInformationIndex;
	filter_function_table[36] = g_SSDT_Func_Index_Data.ZwSetSystemInformationIndex;
	filter_function_table[37] = g_SSDT_Func_Index_Data.ZwSetSystemTimeIndex;
	filter_function_table[38] = g_SSDT_Func_Index_Data.ZwSystemDebugControlIndex;
	filter_function_table[39] = g_ShadowSSDT_Func_Index_Data.ZwUserBuildHwndListIndex;
	filter_function_table[40] = g_ShadowSSDT_Func_Index_Data.ZwUserQueryWindowIndex;
	filter_function_table[41] = g_ShadowSSDT_Func_Index_Data.ZwUserFindWindowExIndex;
	filter_function_table[42] = g_ShadowSSDT_Func_Index_Data.ZwUserWindowFromPointIndex;
	filter_function_table[43] = g_ShadowSSDT_Func_Index_Data.ZwUserMessageCallIndex;
	filter_function_table[44] = g_ShadowSSDT_Func_Index_Data.ZwUserPostMessageIndex;
	filter_function_table[45] = g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowsHookExIndex;
	filter_function_table[46] = g_ShadowSSDT_Func_Index_Data.ZwUserPostThreadMessageIndex;
	filter_function_table[47] = g_SSDT_Func_Index_Data.ZwOpenProcessIndex;
	filter_function_table[48] = g_SSDT_Func_Index_Data.ZwDeviceIoControlFileIndex;
	filter_function_table[49] = g_ShadowSSDT_Func_Index_Data.ZwUserSetParentIndex;
	filter_function_table[51] = g_SSDT_Func_Index_Data.ZwDuplicateObjectIndex;
	filter_function_table[50] = g_SSDT_Func_Index_Data.ZwOpenKeyIndex;
	filter_function_table[52] = g_SSDT_Func_Index_Data.ZwResumeThreadIndex;
	filter_function_table[53] = g_ShadowSSDT_Func_Index_Data.ZwUserChildWindowFromPointExIndex;
	filter_function_table[54] = g_ShadowSSDT_Func_Index_Data.ZwUserDestroyWindowIndex;
	filter_function_table[55] = g_ShadowSSDT_Func_Index_Data.ZwUserInternalGetWindowTextIndex;
	filter_function_table[56] = g_ShadowSSDT_Func_Index_Data.ZwUserMoveWindowIndex;
	filter_function_table[57] = g_ShadowSSDT_Func_Index_Data.ZwUserRealChildWindowFromPointIndex;
	filter_function_table[58] = g_ShadowSSDT_Func_Index_Data.ZwUserSetInformationThreadIndex;
	filter_function_table[70] = g_SSDT_Func_Index_Data.ZwUnmapViewOfSectionIndex;
	filter_function_table[59] = g_ShadowSSDT_Func_Index_Data.ZwUserSetInternalWindowPosIndex;
	filter_function_table[71] = g_ShadowSSDT_Func_Index_Data.ZwUserSetWinEventHookIndex;
	filter_function_table[60] = g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowLongIndex;
	filter_function_table[72] = g_SSDT_Func_Index_Data.ZwSetSecurityObjectIndex;
	filter_function_table[61] = g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowPlacementIndex;
	filter_function_table[73] = g_ShadowSSDT_Func_Index_Data.ZwUserCallHwndParamLockIndex;
	filter_function_table[62] = g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowPosIndex;
	filter_function_table[74] = g_ShadowSSDT_Func_Index_Data.ZwUserRegisterUserApiHookIndex;
	filter_function_table[63] = g_ShadowSSDT_Func_Index_Data.ZwUserSetWindowRgnIndex;
	filter_function_table[76] = g_ShadowSSDT_Func_Index_Data.NtUserRegisterWindowMessageIndex;
	filter_function_table[64] = g_ShadowSSDT_Func_Index_Data.ZwUserShowWindowIndex;
	filter_function_table[77] = g_ShadowSSDT_Func_Index_Data.NtUserCallNoParamIndex;
	filter_function_table[65] = g_ShadowSSDT_Func_Index_Data.ZwUserShowWindowAsyncIndex;
	filter_function_table[78] = g_SSDT_Func_Index_Data.ZwAllocateVirtualMemoryIndex;
	filter_function_table[66] = g_SSDT_Func_Index_Data.ZwQueryAttributesFileIndex;
	filter_function_table[79] = g_ShadowSSDT_Func_Index_Data.NtUserCallOneParamIndex;
	filter_function_table[67] = g_ShadowSSDT_Func_Index_Data.ZwUserSendInputIndex;
	filter_function_table[80] = g_SSDT_Func_Index_Data.ZwCreateMutantIndex;
	filter_function_table[68] = g_SSDT_Func_Index_Data.ZwAlpcSendWaitReceivePortIndex;
	filter_function_table[81] = g_SSDT_Func_Index_Data.ZwOpenMutantIndex;
	filter_function_table[69] = g_SSDTServiceLimit;
	filter_function_table[75] = g_SSDTServiceLimit;
	filter_function_table[82] = g_SSDT_Func_Index_Data.ZwVdmControlIndex;
	filter_function_table[84] = g_SSDT_Func_Index_Data.ZwGetNextProcessIndex;
	filter_function_table[83] = g_SSDT_Func_Index_Data.ZwGetNextThreadIndex;
	filter_function_table[85] = g_SSDT_Func_Index_Data.ZwRequestPortIndex;
	filter_function_table[86] = g_SSDT_Func_Index_Data.ZwFreeVirtualMemoryIndex;
	filter_function_table[87] = g_ShadowSSDT_Func_Index_Data.NtUserCallTwoParamIndex;
	filter_function_table[88] = g_ShadowSSDT_Func_Index_Data.NtUserCallHwndLockIndex;
	filter_function_table[89] = g_SSDT_Func_Index_Data.ZwEnumerateValueKeyIndex;
	filter_function_table[90] = g_SSDT_Func_Index_Data.ZwQueryKeyIndex;
	filter_function_table[91] = g_SSDT_Func_Index_Data.ZwEnumerateKeyIndex;
	filter_function_table[92] = g_SSDT_Func_Index_Data.ZwConnectPortIndex;
	filter_function_table[93] = g_SSDT_Func_Index_Data.ZwSecureConnectPortIndex;
	filter_function_table[94] = g_SSDT_Func_Index_Data.ZwAlpcConnectPortIndex;
	filter_function_table[95] = g_ShadowSSDT_Func_Index_Data.NtUserUnhookWindowsHookExIndex;
	filter_function_table[98] = g_SSDT_Func_Index_Data.ZwSetTimerIndex;
	filter_function_table[99] = g_ShadowSSDT_Func_Index_Data.NtUserClipCursorIndex;
	filter_function_table[100] = g_SSDT_Func_Index_Data.ZwSetInformationProcessIndex;
	filter_function_table[101] = g_ShadowSSDT_Func_Index_Data.NtUserGetKeyStateIndex;
	filter_function_table[102] = g_ShadowSSDT_Func_Index_Data.NtUserGetKeyboardStateIndex;
	filter_function_table[103] = g_ShadowSSDT_Func_Index_Data.NtUserGetAsyncKeyStateIndex;
	filter_function_table[104] = g_ShadowSSDT_Func_Index_Data.NtUserAttachThreadInputIndex;
	filter_function_table[105] = g_ShadowSSDT_Func_Index_Data.NtUserRegisterHotKeyIndex;
	filter_function_table[106] = g_ShadowSSDT_Func_Index_Data.NtUserRegisterRawInputDevicesIndex;
	filter_function_table[107] = g_ShadowSSDT_Func_Index_Data.NtGdiBitBltIndex;
	filter_function_table[108] = g_ShadowSSDT_Func_Index_Data.NtGdiStretchBltIndex;
	filter_function_table[109] = g_ShadowSSDT_Func_Index_Data.NtGdiMaskBltIndex;
	filter_function_table[110] = g_ShadowSSDT_Func_Index_Data.NtGdiPlgBltIndex;
	filter_function_table[111] = g_ShadowSSDT_Func_Index_Data.NtGdiTransparentBltIndex;
	filter_function_table[112] = g_ShadowSSDT_Func_Index_Data.NtGdiAlphaBlendIndex;
	filter_function_table[113] = g_ShadowSSDT_Func_Index_Data.NtGdiGetPixelIndex;
	filter_function_table[114] = g_SSDT_Func_Index_Data.ZwMapViewOfSectionIndex;
	filter_function_table[115] = g_SSDT_Func_Index_Data.ZwTerminateThreadIndex;
	filter_function_table[117] = g_SSDT_Func_Index_Data.ZwTerminateJobObjectIndex;
	filter_function_table[116] = g_SSDT_Func_Index_Data.ZwAssignProcessToJobObjectIndex;
	filter_function_table[118] = g_SSDT_Func_Index_Data.ZwDebugActiveProcessIndex;
	filter_function_table[119] = g_SSDT_Func_Index_Data.ZwSetInformationJobObjectIndex;
	filter_function_table[121] = g_ShadowSSDT_Func_Index_Data.NtUserGetRawInputDataIndex;
	filter_function_table[120] = g_ShadowSSDT_Func_Index_Data.NtUserGetRawInputBufferIndex;
	filter_function_table[96] = g_SSDTServiceLimit;
	filter_function_table[97] = g_SSDTServiceLimit;
	filter_function_table[122] = g_SSDTServiceLimit;
	filter_function_table[123] = g_SSDTServiceLimit;
	filter_function_table[124] = g_ShadowSSDT_Func_Index_Data.NtUserSetImeInfoExIndex;
	filter_function_table[129] = g_SSDTServiceLimit;
	filter_function_table[130] = g_SSDTServiceLimit;
	filter_function_table[134] = g_SSDTServiceLimit;
	filter_function_table[135] = g_SSDT_Func_Index_Data.ZwContinueIndex;
	filter_function_table[136] = g_SSDT_Func_Index_Data.ZwAccessCheckIndex;
	filter_function_table[137] = g_SSDT_Func_Index_Data.ZwAccessCheckAndAuditAlarmIndex;
	filter_function_table[139] = g_SSDT_Func_Index_Data.ZwQueryInformationProcessIndex;
	filter_function_table[138] = g_SSDT_Func_Index_Data.ZwQueryInformationThreadIndex;
	filter_function_table[140] = g_SSDT_Func_Index_Data.ZwQueryIntervalProfileIndex;
	filter_function_table[141] = g_SSDT_Func_Index_Data.ZwSetIntervalProfileIndex;
	filter_function_table[142] = g_SSDT_Func_Index_Data.ZwCreateProfileIndex;
	filter_function_table[143] = g_ShadowSSDT_Func_Index_Data.NtUserLoadKeyboardLayoutExIndex;
	filter_function_table[145] = g_ShadowSSDT_Func_Index_Data.NtGdiAddFontMemResourceExIndex;
	filter_function_table[146] = g_ShadowSSDT_Func_Index_Data.NtGdiAddRemoteFontToDCIndex;
	filter_function_table[144] = g_ShadowSSDT_Func_Index_Data.NtGdiAddFontResourceWIndex;
	filter_function_table[148] = g_SSDT_Func_Index_Data.ZwSuspendProcessIndex;
	filter_function_table[147] = g_SSDT_Func_Index_Data.ZwSuspendThreadIndex;
	filter_function_table[149] = g_SSDT_Func_Index_Data.ZwApphelpCaCheControlIndex;
	filter_function_table[151] = g_SSDT_Func_Index_Data.ZwLoadKeyIndex;
	filter_function_table[126] = g_SSDT_Func_Index_Data.ZwAlpcConnectPortExIndex;
	filter_function_table[153] = g_SSDT_Func_Index_Data.ZwLoadKeyExIndex;
	filter_function_table[125] = g_SSDT_Func_Index_Data.ZwQueueApcThreadExIndex;
	filter_function_table[152] = g_SSDT_Func_Index_Data.ZwLoadKey2Index;
	filter_function_table[127] = g_SSDT_Func_Index_Data.ZwMakeTemporaryObjectIndex;
	filter_function_table[150] = g_SSDT_Func_Index_Data.ZwUnmapViewOfSectionIndex_Win8_Win10;
	filter_function_table[128] = g_SSDT_Func_Index_Data.ZwDisplayStringIndex;
	filter_function_table[154] = g_SSDT_Func_Index_Data.ZwOpenKeyExIndex;
	filter_function_table[131] = g_ShadowSSDT_Func_Index_Data.NtGdiOpenDCWIndex;
	filter_function_table[155] = g_SSDT_Func_Index_Data.dword_1BAA0;
	filter_function_table[132] = g_ShadowSSDT_Func_Index_Data.NtGdiDeleteObjectAppIndex;
	filter_function_table[156] = g_SSDT_Func_Index_Data.dword_1BB08;
	result = g_SSDT_Func_Index_Data.dword_1BA98;
	filter_function_table[133] = g_ShadowSSDT_Func_Index_Data.NtUserBlockInputIndex;
	filter_function_table[157] = g_SSDT_Func_Index_Data.dword_1BA98;
	return result;
}


//************************************     
// ��������: HookPort_InitProxyAddress     
// ����˵�������g_SS_Filter_Table->SSDT��SSSDT������
//			������˺������ַ�Ϊ���ַ�ʽ��
//			1������Ȥ�ĵ�������������Ե�Fake_XXXX������
//			2��������Ȥ��ͨ�ô����޸�HookPort_FilterHook������
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2019/12/18     
// �� �� ֵ: ULONG NTAPI     
// ��    ��: ULONG Flag          0 == SSDT��1 == shadowSSDT���л���GUI�̣߳�
//************************************  
ULONG NTAPI HookPort_InitProxyAddress(ULONG Flag)
{
	ULONG FunSize = 0; // ebx@1
	ULONG Number = 0; // esi@4
	ULONG Index = 0; // edx@32
	ULONG v13 = 0;
	PVOID pBuff_v5;
	//������
	UCHAR	cDbgObjSign[] = { 0x90, 0x90, 0x90, 0x90, 0x68, 0x51, 0x62, 0x53, 0x04, 0x90, 0x90, 0x90, 0x90, 0x00 };
	//��ȡHookPort_FilterHook������С
	FunSize = HookPort_PredictBlockEnd((ULONG)HookPort_FilterHook, 0x100, cDbgObjSign, strlen(cDbgObjSign));
	//����д�������ֽڣ� + strlen(cDbgObjSign)��
	FunSize = (FunSize - (ULONG)HookPort_FilterHook) + strlen(cDbgObjSign);
	//2��new����Ӧ�Ŀռ��ţ�filter_function_table_Size_temp�����������ͨ�ò�����Ȥ��SSDT��SSSDT���ô��붼�����ű���
	if (!filter_function_table_Size_temp)
	{
		filter_function_table_Size_temp = ExAllocatePoolWithTag(NonPagedPool, FILTERFUNCNT * FunSize, HOOKPORT_POOLTAG1);
		RtlZeroMemory(filter_function_table_Size_temp, FILTERFUNCNT * FunSize);
		if (!filter_function_table_Size_temp)
			return 0;
	}
	Number = 0;
	v13 = 0;
	//3������SSDT��SSSDTFake_Hook������Ϊ���֣�
	//1:����Ȥ�ģ�����д��Fake_xxxx��������
	//2:������Ȥ�ģ�ʹ��ͨ��Hook����HookPort_FilterHook��������Բ�ͬ��NT�����޸�HookPort_FilterHook��
	do
	{
		if (Number == 0xC)							//��һ���ǿյ�				ZwSetEvnet
		{
			goto Next;
		}
		if (Number == ZwWriteFile_FilterIndex)		//filter_function_table[11] = ZwWriteFileIndex;
		{
			g_SS_Filter_Table->ProxySSDTServiceAddress[g_SSDT_Func_Index_Data.ZwWriteFileIndex] = Filter_ZwWriteFile;
			g_SS_Filter_Table->ProxySSDTServiceAddress[g_SSDT_Func_Index_Data.ZwWriteFileGatherIndex] = Filter_ZwWriteFileGather;
			goto Next;
		}
		if (Number == ZwCreateThread_FilterIndex)		//filter_function_table[16] = ZwCreateThreadIndex;
		{
			g_SS_Filter_Table->ProxySSDTServiceAddress[g_SSDT_Func_Index_Data.ZwCreateThreadIndex] = Filter_ZwCreateThread;
			g_SS_Filter_Table->ProxySSDTServiceAddress[g_SSDT_Func_Index_Data.ZwCreateThreadExIndex] = Filter_ZwCreateThreadEx;
			goto Next;
		}
		if (Number == ZwLoad_Un_Driver_FilterIndex)         // filter_function_table[34] = ZwLoadDriverIndex;
		{
			g_SS_Filter_Table->ProxySSDTServiceAddress[g_SSDT_Func_Index_Data.ZwLoadDriverIndex] = Filter_ZwLoadDriver;
			g_SS_Filter_Table->ProxySSDTServiceAddress[g_SSDT_Func_Index_Data.ZwUnloadDriverIndex] = Filter_ZwUnloadDriver;
			goto Next;
		}
		if (Number == ZwOpenFile_FilterIndex)           // filter_function_table[19] = ZwOpenFileIndex;
		{
			g_SS_Filter_Table->ProxySSDTServiceAddress[g_SSDT_Func_Index_Data.ZwOpenFileIndex] = Filter_ZwOpenFile;
			goto Next;
		}
		if (Number == ZwCreateFile_FilterIndex)         // filter_function_table[8] = ZwCreateFileIndex;
		{
			g_SS_Filter_Table->ProxySSDTServiceAddress[g_SSDT_Func_Index_Data.ZwCreateFileIndex] = Filter_ZwCreateFile;
			goto Next;
		}
		if (Number == ZwSetSystemInformation_FilterIndex) // filter_function_table[36] = ZwSetSystemInformationIndex;
		{
			g_SS_Filter_Table->ProxySSDTServiceAddress[g_SSDT_Func_Index_Data.ZwSetSystemInformationIndex] = Filter_ZwSetSystemInformation;
			goto Next;
		}
		if (Number == NtUserSetImeInfoEx_FilterIndex)                       // filter_function_table[124] = NtUserSetImeInfoExIndex;
		{
			if (Flag)
			{
				//g_SS_Filter_Table->ProxyShadowSSDTServiceAddress[g_ShadowSSDT_Func_Index_Data.NtUserSetImeInfoExIndex & 0xFFF] = &off_114B8;
				goto Next;
			}
		}
		else
		{
			if (Number	  == CreateProcessNotifyRoutine_FilterIndex    //�ⲿ����HookPort�����Я����Fake����������Fake��������ͨ��360SafeProtection��ֵ��
				|| Number == ClientLoadLibrary_FilterIndex
				|| Number == fnHkOPTINLPEVENTMSG_XX2_FilterIndex
				|| Number == ClientImmLoadLayout_XX1_FilterIndex
				|| Number == fnHkOPTINLPEVENTMSG_XX1_FilterIndex
				|| Number == fnHkINLPKBDLLHOOKSTRUCT_FilterIndex
				|| Number == LoadImageNotifyRoutine_FilterIndex
				|| Number == CreateProcessNotifyRoutineEx_FilterIndex
				|| Number == CreateThreadNotifyRoutine_FilterIndex)
			{
				goto Next;
			}
			if (Number == ZwContinue_FilterIndex)         // filter_function_table[135] = ZwContinueIndex;
			{
				g_SS_Filter_Table->ProxySSDTServiceAddress[g_SSDT_Func_Index_Data.ZwContinueIndex] = Filter_ZwContinue;
			}
		}
		Index = filter_function_table[Number];
		if (Index == g_SSDTServiceLimit)               // �ж��Ƿ���Чֵ
		{
			goto Next;
		}
		if (Index & 0x1000)                          // �ж��ǲ���SSSDT
		{
			if (!Flag
				|| g_SS_Filter_Table->ProxyShadowSSDTServiceAddress[(Index & 0xFFF)]
				|| !g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiParamTableBase)
			{
				goto Next;
			}
		}
		else if (Flag == 1 || g_SS_Filter_Table->ProxySSDTServiceAddress[Index])
		{
			goto Next;
		}
		//������Ȥ��ͨ�ô�����
		//�޸���HookPort_FilterHook�ṹȻ�����g_SS_Filter_Table->SSDT��SSSDT[Index]������
		//ÿһ�����˺�����Ӧһ��HookPort_FilterHook��һ����FILTERFUNCNT��
		pBuff_v5 = (UCHAR *)filter_function_table_Size_temp + FunSize * Number;
		RtlCopyMemory(pBuff_v5, HookPort_FilterHook, FunSize);
		//�޸�HookPort_FilterHook����
		for (ULONG i_v9 = 0; i_v9 < FunSize; i_v9++)
		{
			PVOID v10 = (PVOID)((PCHAR)pBuff_v5 + i_v9);
			//1:�޸�HookPort_DoFilter�����Ĳ���1
			//push 0xAAAAA  ->  push Index
			if (*(ULONG *)v10 == 0xAAAAAAAA)
			{
				*(ULONG *)v10 = Number;
				//�ж��ǲ���call
				if (*(UCHAR *)((PCHAR)v10 + 4) == 0xE8u)
				{
					//�޸���call xxxx���ض�λ��new�����ռ��
					*(ULONG *)((PCHAR)v10 + 5) += (ULONG)HookPort_FilterHook - (ULONG)pBuff_v5;
				}
			}
			//2:�ж�Ҫʹ�ö��ռ䣬Ȼ���޸�sub esp, 0BBBBBBBBh->sub esp,XXXh
			//��ȡSSDT��SSSDT��ParamTableBase�Ϳ���ȷ�ϲ�������
			if (*(ULONG *)v10 == 0xBBBBBBBB)
			{
				//�ж�SSDT����SSSDT
				if (Index & 0x1000)
				{
					*(ULONG *)v10 = *(UCHAR*)((Index & 0xFFF) + (PCHAR)g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiParamTableBase) + 0x98;
				}
				else
				{

					*(ULONG *)v10 = *(UCHAR*)((PCHAR)g_HookPort_Nt_Win32k_Data.SSDTTable_Data.SSDT_KeParamTableBase + Index) + 0x98;
				}
			}
			//3:�ж�Ҫmemcpy���ռ䣬Ȼ���޸�qmemcpy(&savedregs, &a1, 0x33333330u)->qmemcpy(&savedregs, &a1, 0xXXXu);
			if (*(ULONG *)v10 == 0xCCCCCCCC)
			{
				//�ж�SSDT����SSSDT
				if (Index & 0x1000)
				{
					*(ULONG *)v10 = *(UCHAR*)((Index & 0xFFF) + (PCHAR)g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiParamTableBase) >> 2;
				}
				else
				{
					*(ULONG *)v10 = *(UCHAR*)((PCHAR)g_HookPort_Nt_Win32k_Data.SSDTTable_Data.SSDT_KeParamTableBase + Index) >> 2;
				}
			}
			//4:�޸�sub_10A38���������Ĳ���1			����ԭʼ����
			//push 0xAAAAA  ->  push Index
			if (*(ULONG *)v10 == 0xDDDDDDDD)
			{
				*(ULONG *)v10 = Index;
				//�ж��ǲ���call
				if (*(UCHAR *)((PCHAR)v10 + 4) == 0xE8u)
				{
					//�޸���call xxxx���ض�λ��new�����ռ��
					*(ULONG *)((PCHAR)v10 + 5) += (ULONG)HookPort_FilterHook - (ULONG)pBuff_v5;
				}
			}
			//5:�޸�retn
			if (*(ULONG *)v10 == 0xEEEEC2C9)
			{
				//�ж�SSDT����SSSDT
				if (Index & 0x1000)
				{
					*(USHORT *)((PCHAR)v10 + 2) = *(UCHAR*)((Index & 0xFFF) + (PCHAR)g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiParamTableBase);
				}
				else
				{
					*(USHORT *)((PCHAR)v10 + 2) = *(UCHAR*)((PCHAR)g_HookPort_Nt_Win32k_Data.SSDTTable_Data.SSDT_KeParamTableBase + Index);
				}
			}
		}
		//�޸���Ͻ��׵�ַ��ֵ�����ǵ�HOOK����
		//�ж�SSDT����SSSDT
		if (Index & 0x1000)
		{
			g_SS_Filter_Table->ProxyShadowSSDTServiceAddress[Index] = pBuff_v5;
		}
		else
		{
			g_SS_Filter_Table->ProxySSDTServiceAddress[Index] = pBuff_v5;
		}
	Next:
		v13 = ++Number;
	} while (Number < FILTERFUNCNT);

	return 1;
}