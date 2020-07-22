#pragma once
#include <ntddk.h>
#include "defs.h"
//ȫ��DriverObject
PDRIVER_OBJECT Global_DriverObject;

//���ò���
ULONG dword_1B110;					//��־λ������
ULONG dword_1B130;					//���ò�����HookPort_FilterHook
ULONG dword_1B114;					//���ظ�3��FakeXXX_����ʧ��

//���ּ��ܺ�Ĺ�ϣֵ(δ֪)
ULONG Global_Hash_1;
ULONG Global_Hash_2;
ULONG Global_Hash_3;
ULONG Global_Hash_4;

ULONG dword_1B134_ModuleBase;		//Global_Hash_1�Ļ�ַ
ULONG dword_1B138_ModuleSize;		//Global_Hash_1�Ļ�ַ��С

ULONG dword_1B13C_ModuleBase;		//Global_Hash_2�Ļ�ַ
ULONG dword_1B140_ModuleSize;		//Global_Hash_2�Ļ�ַ��С

ULONG dword_1B14C_ModuleBase;      //Global_Hash_3�Ļ�ַ
ULONG dword_1B150_ModuleSize;      //Global_Hash_3�Ļ�ַ��С

ULONG dword_1B144_ModuleBase;      //Global_Hash_4�Ļ�ַ
ULONG dword_1B148_ModuleSize;      //Global_Hash_4�Ļ�ַ��С

ULONG dword_1B120;					//δ֪ ����
ULONG dword_1B124;					//����Global_Hash_2~Global_Hash_4�Ŀ���Flag
ULONG dword_1B128;					//δ֪ ����
ULONG dword_1B12C;					//δ֪ ����

////Win32k��ǣ���ȡWin32k�ں˳ɹ���1
ULONG				 Global_Win32kFlag;
ULONG                Global_Version_Win10_Flag;		//Win10��ʶ
RTL_OSVERSIONINFOEXW Global_osverinfo;				//������Ҫʹ��ֱ�Ӷ����ȫ�ֵ�

//Fake_ZwSetEvent����ʹ�õı���
PVOID g_call_ring0_rtn_address;												//��ջ�����л�õ�KiFastCallEntry���ص�ַ

/**************************�궨��***************************/
#define	HookPort_DeviceName		L"\\Device\\360HookPort"
#define	HookPort_LinkName		L"\\DosDevices\\360HookPort"
#define HookPort_Minimal        L"\\SafeBoot\\Minimal\\HookPort"
#define HookPort_Network        L"\\SafeBoot\\Network\\HookPort"


#define	WIN32KSYS				"win32k.sys"
#define	WIN32KFULLSYS			"win32kfull.sys"
#define	NTOSKERNL				"ntoskrnl.exe"


#define	HOOKPORT_POOLTAG1		'HPPX'
#define	HOOKPORT_POOLTAG2	    'JMPP'
#define	HOOKPORT_POOLTAG3	    'HPIT'
#define	HOOKPORT_POOLTAG4	    'SMAP'
#define	HOOKPORT_POOLTAG5	    'SSMA'
#define HOOKPORT_POOLTAG6       'NMU '
#define HOOKPORT_POOLTAG7       'HPOR'


#define STATUS_HOOKPORT_FILTER_RULE_ERROR  ((NTSTATUS)0xC0000503L)


/**************************�궨��***************************/


#define	g_SSDTServiceLimit 2000

typedef struct _SYSTEM_SERVICE_FILTER_TABLE{
	PULONG ProxySSDTServiceAddress[g_SSDTServiceLimit + 1];			 //��ʼƫ��0000*4,���汻Hook��SSDT������Ӧ�Ĵ������ĵ�ַ 
	PULONG ProxyShadowSSDTServiceAddress[g_SSDTServiceLimit + 1];    //��ʼƫ��2001*4,���汻Hook��ShadowSSDT������Ӧ�Ĵ������ĵ�ַ 
	ULONG SwitchTableForSSDT[g_SSDTServiceLimit + 1];                //��ʼƫ��4002*4,����SSDT Hook����,�����ú����Ƿ�ᱻHook 
	ULONG SwitchTableForShadowSSDT[g_SSDTServiceLimit + 1];          //��ʼƫ��6003*4,����ShadowSSDT Hook����,�����ú����Ƿ�ᱻHook 
	PULONG SavedSSDTServiceAddress[g_SSDTServiceLimit + 1];			 //��ʼƫ��8004*4,���汻Hook��ԭʼSSDT�����ĵ�ַ            ����
	PULONG SavedShadowSSDTServiceAddress[g_SSDTServiceLimit + 1];    //��ʼƫ��A005*4,���汻Hook��ԭʼShadowSSDT�����ĵ�ַ      ����
}SYSTEM_SERVICE_FILTER_TABLE, *PSYSTEM_SERVICE_FILTER_TABLE;

PSYSTEM_SERVICE_FILTER_TABLE	g_SS_Filter_Table;								//Hook��ܵĽṹ��
// 
// ����ĳ�˵�������Ϊ����ṹ�ߴ�Ϊ0x51C�����˺����ĸ���0x9E
// 

#define FILTERFUNCNT 0x9E //���˺����ĸ��� 

typedef struct _FILTERFUN_RULE_TABLE{
	ULONG 	Size; 									//���ṹ�Ĵ�С,Ϊ0x51C	 
	struct _FILTERFUN_RULE_TABLE 	*Next; 			//ƫ��Ϊ0x4,ָ����һ���ڵ� 
	ULONG 	IsFilterFunFilledReady;             	//ƫ��Ϊ0x8,��־,�������˺������Ƿ�׼���� 
	PULONG 	SSDTRuleTableBase;                  	//ƫ��Ϊ0xC,��SSDT�����Ĺ��˹����,��Ĵ�СΪSSDTCnt*4 
	PULONG 	ShadowSSDTRuleTableBase;         		//ƫ��Ϊ0x10,��ShadowSSDT�����Ĺ��˹����,��Ĵ�СΪShadowSSDTCnt*4
	UCHAR	FilterRuleName[16];						//ƫ��Ϊ0x14~0x20���������
	PVOID   pModuleBase;							//ƫ��Ϊ0x24,����ַ
	ULONG   ModuleSize;								//ƫ��Ϊ0x28,����ַ��С
	PULONG 	FakeServiceRoutine[FILTERFUNCNT];    	//ƫ��Ϊ0x2C,���˺�������,���й��˺���0x9E��  (����)
	PULONG 	FakeServiceRuleFlag[FILTERFUNCNT];    	//ƫ��Ϊ0x2A4,���˺�������,���й��˺���0x9E�� (����)
}FILTERFUN_RULE_TABLE, *PFILTERFUN_RULE_TABLE;

PFILTERFUN_RULE_TABLE	        g_FilterFun_Rule_table_head;
PFILTERFUN_RULE_TABLE	        g_FilterFun_Rule_table_head_Temp;			    //����

//dword_1A940
//���ﱣ������˺�����ַ 
ULONG	filter_function_table[FILTERFUNCNT];
PVOID	filter_function_table_Size_temp;    //��ʱnew�����ı�������С�ǣ�filter_function_table�����С * ĳ��������С

#define RULE_MUST_HOOK				1
#define RULE_KERNEL_HOOK			2
#define RULE_GUI_HOOK				3

/*
// sizeof(HOOKPORT_EXTENSION) = 0x18
�豸��չ��������ӹ���Ľӿ�
1������������Ҫ���ӹ���ʱֻ��Ҫ��ȡHookport��������չ���������HookPort_FilterRule_Init��ʼ��һ������
2��HookPort_SetFilterSwitchFunction ���ù�����˺���
3��HookPort_SetFilterRuleFlag ���ÿ��ر�ʾ���� or �ر�
State							 ������ʶ
HookPort_FilterRule_Init		 ��ʼ�������½������ӵ���������
HookPort_SetFilterSwitchFunction ���ù�����˺���
HookPort_SetFilterRuleFlag       ���ù��򿪹�
HookPort_SetFilterRuleName       ���ù�������
Value3F1						 �������汾
*/
typedef struct _HOOKPORT_EXTENSION
{
	_DWORD State;
	_DWORD HookPort_FilterRule_Init;
	_DWORD HookPort_SetFilterSwitchFunction;
	_DWORD HookPort_SetFilterRule;
	_DWORD HookPort_SetFilterRuleName;
	_DWORD Value3F1;
}HOOKPORT_EXTENSION, *PHOOKPORT_EXTENSION;



//nt�ں���win32k����ַ
typedef struct _HOOKPORT_NT_WIN32K_DATA
{
	//NT�ں˻���ַ���С
	struct
	{
		PVOID NtImageBase;
		ULONG NtImageSize;
	}NtData;
	//ShadowSSDT����Ϣ
	struct
	{
	//win10_14316�汾֮ǰ
	PVOID ShadowSSDT_GuiServiceTableBase;
	ULONG ShadowSSDT_GuiNumberOfServices;
	PVOID ShadowSSDT_GuiParamTableBase;
	//win10_14316�汾֮��
	PVOID ShadowSSDT_GuiServiceTableBase_Win10_14316;
	ULONG ShadowSSDT_GuiNumberOfServices_Win10_14316;
	PVOID ShadowSSDT_GuiParamTableBase_Win10_14316;
	}ShadowSSDTTable_Data;
	//SSDT����Ϣ
	struct
	{
		PVOID SSDT_KeServiceTableBase;
		ULONG SSDT_KeNumberOfServices;
		PVOID SSDT_KeParamTableBase;
	}SSDTTable_Data;
}HOOKPORT_NT_WIN32K_DATA, *PHOOKPORT_NT_WIN32K_DATA;

//SSDT������ַ������
typedef struct _SSDT_FUNC_INDEX_DATA
{
	PVOID pZwSetEvent;									//HOOK KiFastCallEntry
	ULONG ZwSetEventIndex;

	PVOID pZwAccessCheckAndAuditAlarm  ;
	ULONG ZwAccessCheckAndAuditAlarmIndex  ;

	PVOID pZwAdjustPrivilegesToken  ;
	ULONG ZwAdjustPrivilegesTokenIndex  ;

	PVOID pZwAllocateVirtualMemory  ;
	ULONG ZwAllocateVirtualMemoryIndex  ;

	PVOID pZwAlpcConnectPort  ;
	ULONG ZwAlpcConnectPortIndex  ;

	PVOID pZwAlpcConnectPortEx  ;
	ULONG ZwAlpcConnectPortExIndex  ;

	PVOID pZwConnectPort  ;
	ULONG ZwConnectPortIndex  ;

	PVOID pZwCreateFile  ;
	ULONG ZwCreateFileIndex  ;

	PVOID pZwCreateKey  ;
	ULONG ZwCreateKeyIndex  ;

	PVOID pZwCreateSection  ;
	ULONG ZwCreateSectionIndex  ;

	PVOID pZwCreateSymbolicLinkObject  ;
	ULONG ZwCreateSymbolicLinkObjectIndex  ;

	PVOID pZwDeleteFile  ;
	ULONG ZwDeleteFileIndex  ;

	PVOID pZwDeleteKey  ;
	ULONG ZwDeleteKeyIndex  ;

	PVOID pZwDeleteValueKey  ;
	ULONG ZwDeleteValueKeyIndex  ;

	PVOID pZwDeviceIoControlFile  ;
	ULONG ZwDeviceIoControlFileIndex  ;

	PVOID pZwDisplayString  ;
	ULONG ZwDisplayStringIndex  ;

	PVOID pZwDuplicateObject  ;
	ULONG ZwDuplicateObjectIndex  ;

	PVOID pZwEnumerateKey  ;
	ULONG ZwEnumerateKeyIndex  ;

	PVOID pZwEnumerateValueKey  ;
	ULONG ZwEnumerateValueKeyIndex  ;

	PVOID pZwFreeVirtualMemory  ;
	ULONG ZwFreeVirtualMemoryIndex  ;

	PVOID pZwFsControlFile  ;
	ULONG ZwFsControlFileIndex  ;

	PVOID pZwLoadDriver  ;
	ULONG ZwLoadDriverIndex  ;

	PVOID pZwLoadKey;
	ULONG ZwLoadKeyIndex;

	PVOID pZwMapViewOfSection  ;
	ULONG ZwMapViewOfSectionIndex  ;

	PVOID pZwMakeTemporaryObject  ;
	ULONG ZwMakeTemporaryObjectIndex  ;
 
	PVOID pZwOpenFile  ;
	ULONG ZwOpenFileIndex  ;

	PVOID pZwOpenKey  ;
	ULONG ZwOpenKeyIndex  ;

	PVOID pZwOpenKeyEx  ;
	ULONG ZwOpenKeyExIndex  ;

	PVOID pZwOpenProcess  ;
	ULONG ZwOpenProcessIndex  ;

	PVOID pZwOpenThread  ;
	ULONG ZwOpenThreadIndex  ;

	PVOID pZwOpenSection  ;
	ULONG ZwOpenSectionIndex  ;

	PVOID pZwOpenSymbolicLinkObject  ;
	ULONG ZwOpenSymbolicLinkObjectIndex  ;

	PVOID pZwQueryKey  ;
	ULONG ZwQueryKeyIndex  ;

	PVOID pZwQueryInformationProcess  ;
	ULONG ZwQueryInformationProcessIndex  ;

	PVOID pZwQueryInformationThread  ;
	ULONG ZwQueryInformationThreadIndex  ;

	PVOID pZwQueryValueKey  ;
	ULONG ZwQueryValueKeyIndex  ;

	PVOID pZwQuerySystemInformation  ;
	ULONG ZwQuerySystemInformationIndex  ;
 
	PVOID pZwReplaceKey  ;
	ULONG ZwReplaceKeyIndex  ;

	PVOID pZwRequestWaitReplyPort  ;
	ULONG ZwRequestWaitReplyPortIndex  ;

	PVOID pZwRestoreKey  ;
	ULONG ZwRestoreKeyIndex  ;
 
	PVOID pZwSecureConnectPort  ;
	ULONG ZwSecureConnectPortIndex  ;

	PVOID pZwSetInformationProcess  ;
	ULONG ZwSetInformationProcessIndex  ;

	PVOID pZwSetInformationFile  ;
	ULONG ZwSetInformationFileIndex  ;

	PVOID pZwSetInformationThread  ;
	ULONG ZwSetInformationThreadIndex  ;

	PVOID pZwSetTimer  ;
	ULONG ZwSetTimerIndex  ;

	PVOID pZwSetSecurityObject  ;
	ULONG ZwSetSecurityObjectIndex  ;

	PVOID pZwSetSystemInformation  ;
	ULONG ZwSetSystemInformationIndex  ;

	PVOID pZwSetSystemTime  ;
	ULONG ZwSetSystemTimeIndex  ;

	PVOID pZwSetValueKey  ;
	ULONG ZwSetValueKeyIndex  ;

	PVOID pZwTerminateProcess  ;
	ULONG ZwTerminateProcessIndex  ;

	PVOID pZwWriteFile  ;
	ULONG ZwWriteFileIndex  ;

	PVOID pZwUnloadDriver  ;
	ULONG ZwUnloadDriverIndex  ;

	PVOID pZwUnmapViewOfSection;
	ULONG ZwUnmapViewOfSectionIndex;
	ULONG ZwUnmapViewOfSectionIndex_Win8_Win10;		//Win7�汾�����������֪����ʲô��

	ULONG ZwRenameKey;

	ULONG ZwRenameKeyIndex;

	ULONG ZwCreateProcessIndex;

	ULONG ZwCreateProcessExIndex;

	ULONG ZwCreateUserProcessIndex;

	ULONG ZwCreateThreadIndex;

	ULONG ZwRequestPortIndex;

	ULONG ZwGetNextProcessIndex;

	ULONG ZwGetNextThreadIndex;

	ULONG ZwVdmControlIndex;

	ULONG ZwCreateMutantIndex;

	ULONG ZwOpenMutantIndex;

	ULONG ZwSystemDebugControlIndex;

	ULONG ZwReadVirtualMemoryIndex;

	ULONG ZwWriteVirtualMemoryIndex;

	ULONG ZwQueueApcThreadIndex;

	ULONG ZwSetContextThreadIndex;

	ULONG ZwProtectVirtualMemoryIndex;

	ULONG ZwAdjustGroupsTokenIndex;

	ULONG ZwWriteFileGatherIndex;

	ULONG ZwResumeThreadIndex;

	ULONG ZwAlpcSendWaitReceivePortIndex;

	ULONG ZwCreateThreadExIndex;

	ULONG ZwQueryAttributesFileIndex;

	ULONG ZwTerminateThreadIndex;

	ULONG ZwAssignProcessToJobObjectIndex;

	ULONG ZwTerminateJobObjectIndex;

	ULONG ZwDebugActiveProcessIndex;

	ULONG ZwSetInformationJobObjectIndex;

	ULONG ZwQueueApcThreadExIndex;

	ULONG ZwContinueIndex;

	ULONG ZwAccessCheckIndex;

	ULONG ZwQueryIntervalProfileIndex;

	ULONG ZwSetIntervalProfileIndex;

	ULONG ZwCreateProfileIndex;

	ULONG ZwSuspendThreadIndex;

	ULONG ZwSuspendProcessIndex;

	ULONG ZwApphelpCaCheControlIndex;

	ULONG ZwLoadKey2Index;

	ULONG ZwLoadKeyExIndex;

	//����δ֪��
	ULONG dword_1BAA0;
	ULONG dword_1BB08;
	ULONG dword_1BA98;
}SSDT_FUNC_INDEX_DATA, *PSSDT_FUNC_INDEX_DATA;

//ShadowSSDT������ַ������
typedef struct _SHADOWSSDT_FUNC_INDEX_DATA
{
	ULONG ZwUnmapViewOfSectionIndex;
	ULONG ZwUserSetWinEventHookIndex;
	ULONG ZwUserCallHwndParamLockIndex;
	ULONG ZwUserRegisterUserApiHookIndex;
	ULONG ZwUserSetParentIndex;
	ULONG ZwUserChildWindowFromPointExIndex;
	ULONG ZwUserDestroyWindowIndex;
	ULONG ZwUserInternalGetWindowTextIndex;
	ULONG ZwUserMoveWindowIndex;
	ULONG ZwUserRealChildWindowFromPointIndex;
	ULONG ZwUserSetInformationThreadIndex;
	ULONG ZwUserSetInternalWindowPosIndex;
	ULONG ZwUserSetWindowLongIndex;
	ULONG ZwUserSetWindowPlacementIndex;
	ULONG ZwUserSetWindowPosIndex;
	ULONG ZwUserSetWindowRgnIndex;
	ULONG ZwUserShowWindowIndex;
	ULONG ZwUserShowWindowAsyncIndex;
	ULONG ZwUserSendInputIndex;
	ULONG NtUserCallOneParamIndex;
	ULONG NtUserRegisterWindowMessageIndex;
	ULONG NtUserCallNoParamIndex;
	ULONG NtUserCallTwoParamIndex;
	ULONG NtUserCallHwndLockIndex;
	ULONG NtUserUnhookWindowsHookExIndex;
	ULONG NtUserClipCursorIndex;
	ULONG NtUserGetKeyStateIndex;
	ULONG NtUserGetKeyboardStateIndex;
	ULONG NtUserGetAsyncKeyStateIndex;
	ULONG NtUserAttachThreadInputIndex;
	ULONG NtUserRegisterHotKeyIndex;
	ULONG NtUserRegisterRawInputDevicesIndex;
	ULONG NtGdiBitBltIndex;
	ULONG NtGdiStretchBltIndex;
	ULONG NtGdiMaskBltIndex;
	ULONG NtGdiPlgBltIndex;
	ULONG NtGdiTransparentBltIndex;
	ULONG NtGdiAlphaBlendIndex;
	ULONG NtGdiGetPixelIndex;
	ULONG NtUserGetRawInputDataIndex;
	ULONG NtUserGetRawInputBufferIndex;
	ULONG NtUserSetImeInfoExIndex;
	ULONG NtGdiOpenDCWIndex;
	ULONG NtGdiDeleteObjectAppIndex;
	ULONG NtUserBlockInputIndex;
	ULONG NtUserLoadKeyboardLayoutExIndex;
	ULONG NtGdiAddFontResourceWIndex;
	ULONG NtGdiAddFontMemResourceExIndex;
	ULONG NtGdiAddRemoteFontToDCIndex;
	ULONG ZwUserBuildHwndListIndex;
	ULONG ZwUserQueryWindowIndex;
	ULONG ZwUserFindWindowExIndex;
	ULONG ZwUserWindowFromPointIndex;
	ULONG ZwUserMessageCallIndex;
	ULONG ZwUserPostMessageIndex;
	ULONG ZwUserSetWindowsHookExIndex;
	ULONG ZwUserPostThreadMessageIndex;
	ULONG KeUserModeCallback_ClientLoadLibrary_Index;
	ULONG KeUserModeCallback_ClientImmLoadLayout_Index;
	ULONG KeUserModeCallback_fnHkOPTINLPEVENTMSG_Index;
	ULONG KeUserModeCallback_fnHkINLPKBDLLHOOKSTRUCT_Index;
}SHADOWSSDT_FUNC_INDEX_DATA, *PSHADOWSSDT_FUNC_INDEX_DATA;

HOOKPORT_NT_WIN32K_DATA         g_HookPort_Nt_Win32k_Data;				//����SSDT��ShadowSSDT��Nt�ں˻�����Ϣ����ַ�ʹ�С֮��ģ�
SHADOWSSDT_FUNC_INDEX_DATA      g_ShadowSSDT_Func_Index_Data;			//����shadowSSDT�����к������±�������Ϣ
SSDT_FUNC_INDEX_DATA            g_SSDT_Func_Index_Data;			        //����SSDT�����к������±�������Ϣ 