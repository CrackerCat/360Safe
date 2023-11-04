#pragma once
#include <ntddk.h>
#include "Data.h"
#include "Filter_CreateProcessNotifyRoutine.h"
#include "Filter_LoadImageNotifyRoutine.h"
#include "Filter_CreateThreadNotifyRoutine.h"
#include "Filter_ZwOpenFile.h"
#include "Filter_CreateProcessNotifyRoutineEx.h"
#include "Filter_ZwContinue.h"
#include "Filter_KeUserModeCallbackDispatcher.h"
#include "Filter_ZwCreateThread.h"
#include "Filter_ZwWriteFile.h"
#include "Filter_ZwCreateFile.h"
#include "Filter_ZwLoadDriver.h"
#include "Filter_ZwUnloadDriver.h"
#include "Filter_ZwSetSystemInformation.h"

/**************************���˺�������ǣ�***************************/
//˵����
//R3 == File_XXXX               �������������߰����
//R0 == FakeHook_XXXX           ����������
//������Ӧ�ò����ں˲������ 

#define ZwLoad_Un_Driver_FilterIndex	0x22 

#define ZwSetSystemInformation_FilterIndex	0x24 

#define NtUserSetImeInfoEx_FilterIndex	0x7C 

#define ZwSetValueKey_FilterIndex 0x7

#define ZwSetInformationFile_FilterIndex 0xA

#define ZwUserBuildHwndList_FilterIndex 0x27

#define ZwUserSetInformationThread_FilterIndex 0x3A
/**************************���˺�������ǣ�***************************/

//�Լ�д�ĺ�����ȡHookPort_FilterHook�����ܴ�С
ULONG HookPort_PredictBlockEnd(ULONG uAddress, ULONG uSearchLength, UCHAR *Signature, ULONG SignatureLen);

//���g_SS_Filter_Table->SSDT��SSSDT������
ULONG NTAPI HookPort_InitProxyAddress(ULONG Flag);

// ����FilterFunRuleTable���е�Rule((����PreviousMode�н�һ���ж�))���ж��Ƿ���ҪHook
BOOLEAN	NTAPI HookPort_HookOrNot(ULONG ServiceIndex, BOOLEAN GuiServiceCall);

//�κ�����JMPSTUB�б����ã����ݹ����ж��Ƿ���˴˴ε���
PULONG NTAPI HookPort_KiFastCallEntryFilterFunc(ULONG ServiceIndex, PULONG OriginalServiceRoutine, PULONG ServiceTable);

//����������ݵ��úŵ��ù��˺���������һ��״ֵ̬���������жϽ��
NTSTATUS NTAPI HookPort_DoFilter(ULONG CallIndex, PHANDLE ArgArray, PULONG *RetFuncArray, PULONG *RetFuncArgArray, PULONG RetNumber, PULONG Result);

//��ʼ����������
ULONG HookPort_InitFilterTable();

//��ȡԭʼ��SSDT��ShadowSSDT��ַ
ULONG NTAPI HookPort_GetOriginalServiceRoutine(IN ULONG ServiceIndex);

//���Ĳ��ִ�����
ULONG NTAPI HookPort_ForRunFuncTable(IN ULONG CallIndex, IN PHANDLE ArgArray, IN NTSTATUS InResult, IN PULONG *RetFuncArray, IN PULONG *RetFuncArgArray, IN ULONG  RetCount);





#define	_CHECK_IS_SHADOW_CALL( index )	( (index) & (0x1000) )
#define	_HOOKPORT_GET_SERVICE_PTR(service_index) \
	_CHECK_IS_SHADOW_CALL( (service_index) ) ? 	\
		( MmIsAddressValid( g_SS_Filter_Table->SavedShadowSSDTServiceAddress[(service_index) & (0xFFF)])?	\
			(g_SS_Filter_Table->SavedShadowSSDTServiceAddress[(service_index) & (0xFFF)]) \
			:(*(PVOID*)((PULONG)g_HookPort_Nt_Win32k_Data.ShadowSSDTTable_Data.ShadowSSDT_GuiServiceTableBase + ( (service_index) & (0xFFF) ))) ) \
		:( MmIsAddressValid( g_SS_Filter_Table->SavedSSDTServiceAddress[(service_index)])? \
			(g_SS_Filter_Table->SavedSSDTServiceAddress[(service_index)]) \
			:(*(PVOID*)((PULONG)g_HookPort_Nt_Win32k_Data.SSDTTable_Data.SSDT_KeServiceTableBase + (service_index) )))	