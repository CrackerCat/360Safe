#include "Context.h"

//************************************     
// ��������: KeGetTrapFrame     
// ����˵������ȡ_KTRAP_FRAME�ṹ    
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2020/04/18     
// �� �� ֵ: PKTRAP_FRAME NTAPI     
// ��    ��: IN ULONG TrapFrameIndex    Win7 +0x128 TrapFrame        : Ptr32 _KTRAP_FRAME  
//************************************ 
PKTRAP_FRAME NTAPI Safe_KeGetTrapFrame(IN ULONG TrapFrameIndex)
{
	//KeGetCurrentThread����_KTHREAD�ṹ
	//_KTHREAD + TrapFrameIndex  = +0x128 TrapFrame        : Ptr32 _KTRAP_FRAME 
	return *((ULONG *)KeGetCurrentThread() + TrapFrameIndex);
}

ULONG NTAPI Safe_CheckCreateProcessCreationFlags()
{
	ULONG	     Result = NULL;
	ULONG		 Flag = NULL;
	PKTRAP_FRAME TrapFrame = NULL;          //_KTRAP_FRAME�ṹ
	//�ж��ǲ��Ǳ������̣��Ƿ��أ�1  ���Ƿ���0
	Result = Safe_QueryWhitePID((ULONG)PsGetCurrentProcessId());
	if (Result)
	{
		return Result;
	}
	//���ݰ汾��֧��ȡ_KTRAP_FRAME
	if (g_VersionFlag == WINDOWS_VERSION_XP)
	{
		//��ͬ����1����ͬ���ط�0
		if (!Safe_CmpImageFileName("explorer.exe"))
		{
			Result = Safe_CmpImageFileName("services.exe");
			if (!Result)
			{
				return Result;
			}
		}
		//��ȡ_KTRAP_FRAME�ṹ
		TrapFrame = Safe_KeGetTrapFrame(0x4D);
	}
	else
	{
		//��win7ֱ���˳�
		if (g_VersionFlag != WINDOWS_VERSION_7)
		{
			return Result;
		}
		//��ͬ����1����ͬ���ط�0
		if (!Safe_CmpImageFileName("explorer.exe") && !Safe_CmpImageFileName("services.exe"))
		{
			Result = Safe_CmpImageFileName("svchost.exe");
			if (!Result)
			{
				return Result;
			}
			Flag = 1;
		}
		//��ȡ_KTRAP_FRAME�ṹ
		TrapFrame = Safe_KeGetTrapFrame(0x4A);
	}
	//����LDR�����ȡ��Ӧ��API������ַ
	if (Flag)
	{
		//Win7
		Result = g_dynData->pCreateProcessAsUserW;
		if (!Result)
		{
			Result = Safe_PeLdrFindExportedRoutineByName("CreateProcessAsUserW",1);
			g_dynData->pCreateProcessAsUserW = Result;
			if (!Result)
			{
				return Result;
			}
		}
	}
	else
	{
		//Win2K_XP_2003
		Result = g_dynData->pCreateProcessW;
		if (!Result)
		{
			Result = Safe_PeLdrFindExportedRoutineByName("CreateProcessW", g_Win2K_XP_2003_Flag != 0);
			g_dynData->pCreateProcessW = Result;
			if (!Result)
			{
				return Result;
			}
		}
	}
	//�����ȡ���ĺ�����ַ pCreateProcessW or pCreateProcessAsUserW
	ULONG pCreateProcessAddress = Result;
	//Ȼ���ȡEbp
	if (TrapFrame)
	{
		ULONG Ebp_v5 = TrapFrame->Ebp;
		ULONG OldEbp_v5 = TrapFrame->Ebp;				//ԭʼEbp
		ULONG Number = 0;								//������
		ULONG NumberMaxSize = 0x64;						//���������ֵ
		ULONG TempEbp_v8 = 0;
		//ջ�����ҷ��ص�ַ��Ȼ��У��
		do
		{
			if (!Ebp_v5)
			{
				break;
			}
			if (Ebp_v5 > MmUserProbeAddress)
			{
				break;
			}
			//��ȡebp+4 ���ص�ַ
			ULONG Ret_Address = *(ULONG*)(Ebp_v5 + 4);
			if (Ret_Address > MmUserProbeAddress)
			{
				break;
			}
			//CreateProcessW
			//.text : 77DE204D                 mov     edi, edi
			//.text : 77DE204F                 push    ebp
			//.text : 77DE2050                 mov     ebp, esp
			//.text : 77DE2052                 push    0
			//.text : 77DE2054                 push    [ebp + lpProcessInformation]
			//.text : 77DE2057                 push    [ebp + lpStartupInfo]
			//.text : 77DE205A                 push    [ebp + lpCurrentDirectory]
			//.text : 77DE205D                 push    [ebp + lpEnvironment]
			//.text : 77DE2060                 push    [ebp + dwCreationFlags]     ->Ebp + 0x20��������
			//.text : 77DE2063                 push    [ebp + bInheritHandles]
			//.text : 77DE2066                 push    [ebp + lpThreadAttributes]
			//.text : 77DE2069                 push    [ebp + lpProcessAttributes]
			//.text : 77DE206C                 push    [ebp + lpCommandLine]
			//.text : 77DE206F                 push    [ebp + lpApplicationName]
			//.text : 77DE2072                 push    0
			//.text : 77DE2074                 call    _CreateProcessInternalW@48; CreateProcessInternalW(x, x, x, x, x, x, x, x, x, x, x, x)
			//.text : 77DE2079                 pop     ebp                         ->Ebp + 4��������
			//.text : 77DE207A                 retn    28h
			//�жϷ��ص�ַ�ĺϷ���
			if (Ret_Address > pCreateProcessAddress && Ret_Address - pCreateProcessAddress < 40)
			{
				ULONG dwCreationFlags = *(ULONG*)(OldEbp_v5 + 0x20) & 0xFFFFF7FF;// ��ĳһλ����;		//CREATE_SEPARATE_WOW_VDM???????? 16λ����
				Result = g_Win2K_XP_2003_Flag;
				//�жϱ�־λ��������Ȩ��
				if (!g_Win2K_XP_2003_Flag && dwCreationFlags == (CREATE_UNICODE_ENVIRONMENT | CREATE_SUSPENDED | DETACHED_PROCESS)						// Win7
					|| g_Win2K_XP_2003_Flag == 1 && dwCreationFlags == (BELOW_NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT | CREATE_SUSPENDED)	//��Win7
					|| !g_Win2K_XP_2003_Flag && dwCreationFlags == (CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT | CREATE_DEFAULT_ERROR_MODE)		//Win7
					|| g_Win2K_XP_2003_Flag == 1 && dwCreationFlags == (CREATE_DEFAULT_ERROR_MODE | EXTENDED_STARTUPINFO_PRESENT | CREATE_UNICODE_ENVIRONMENT | CREATE_SUSPENDED | CREATE_NEW_CONSOLE)																	//��Win7
					)
				{
					//����Ȩ��
					*(ULONG *)(OldEbp_v5 + 0x20) = dwCreationFlags | CREATE_PRESERVE_CODE_AUTHZ_LEVEL;
				}
				return Result;
			}
			TempEbp_v8 = Ebp_v5;
			Ebp_v5 = *(ULONG*)Ebp_v5;
			Number++;									//����
		} while (TempEbp_v8 != Ebp_v5 && Number <= NumberMaxSize);
	}
	return Result;
}