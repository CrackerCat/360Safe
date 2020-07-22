#pragma once
#include	<ntifs.h>
#include	"defs.h"
#include	"Data.h"
#include    "System.h"
#include    "WinKernel.h"
#include    "DebugPrint.h"
#include    "Win32k.h"
#include	"SSDT.h"
#include    "FilterHook.h"
#include    "KiFastCallEntry.h"
#include    "Filter_ZwDisplayString.h"

extern PULONG InitSafeBootMode;

#define ObjectNameInformation		1	

//ͨѶ����
#define HOOKPORT_GETVER            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0801, METHOD_BUFFERED, FILE_ANY_ACCESS)	    //����HookPort�汾��
#define HOOKPORT_DEBUGMEASSAGE1    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0802, METHOD_BUFFERED, FILE_ANY_ACCESS)		//���Debug��Ϣ���أ����ã�DbgPrintf_dword_1B174
#define HOOKPORT_DEBUGMEASSAGE2    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0803, METHOD_BUFFERED, FILE_ANY_ACCESS)		//���Debug��Ϣ���أ����ã�DbgPrintf_dword_1AFA0
#define HOOKPORT_DEBUGMEASSAGE3    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0804, METHOD_BUFFERED, FILE_ANY_ACCESS)		//���Debug��Ϣ���أ����ã�DbgPrintf_dword_1B178
#define HOOKPORT_DEBUGMEASSAGE4    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0805, METHOD_BUFFERED, FILE_ANY_ACCESS)		//R3����һ��ʱ�䣬A�㵽B��ִ�д���ʱ�����С�ڸ�ʱ��Ŵ�ӡ������Ϣ���ѵ���ֹ�����ԣ����ã�

ULONG     DbgPrintf_dword_1B174 = NULL;		//����
ULONG     DbgPrintf_dword_1AFA0 = NULL;     //����
ULONG     DbgPrintf_dword_1B178 = NULL;     //����
ULONGLONG DbgPrintf_qdword_1AFB0 = NULL;	//������ִ��ʱ��

//������İ汾��0x3F1
#define HOOKPORT_VERSION                    0x3F1	

//�������Ĳ���
KSPIN_LOCK	g_Filter_Rule_SpinLock;

typedef NTSTATUS(NTAPI *pPsAcquireProcessExitSynchronization)(__in PEPROCESS Process);
//dword_1A710
pPsAcquireProcessExitSynchronization	PsAcquireProcessExitSynchronization;
#define		PsAcquireProcessExitSynchronizationName  L"PsAcquireProcessExitSynchronization"


typedef NTSTATUS(NTAPI *pPsReleaseProcessExitSynchronization)(__in PEPROCESS Process);
//dword_1A714PsReleaseProcessExitSynchronization
pPsReleaseProcessExitSynchronization	PsReleaseProcessExitSynchronization;
#define		PsReleaseProcessExitSynchronizationName  L"PsReleaseProcessExitSynchronization"

////��ȡcsrss�Ľ���id
HANDLE	HookPort_GetApiPortProcessId(IN RTL_OSVERSIONINFOEXW osverinfo);

//IAT hook KeUserModeCallback
VOID HookPort_HookKeUserModeCallback(IN ULONG Version_Win10_Flag);

//��ʼ��SSDT��ShadowSSDT�Ȳ���
NTSTATUS NTAPI HookPort_InitSDT();

//��ȡSSDT��ShadowSSDTԭʼ��ַ������
BOOLEAN  NTAPI HookPort_GetAllNativeFunAddress(PVOID* NtImageBase, IN RTL_OSVERSIONINFOEXW osverinfo);

//��ȡָ��������ַ
BOOLEAN NTAPI HookPort_GetNativeFunAddress(PVOID* NtImageBase);

//��ʼ��Nt�ں˺������� 
BOOLEAN HookPort_InitializeIndex();

//��ʼ����������
ULONG HookPort_InitFilterTable();

//���ܹ�ϣ
BOOLEAN  HookPort_GetModuleBaseAddress_EncryptHash(IN ULONG Hash, OUT PVOID *pModuleBase, OUT ULONG *ModuleSize, OUT ULONG *LoadOrderIndex);

//Filter_LoadImageNotifyRoutine��Ӧ��Fake����
ULONG Fake_LoadImageNotifyRoutine(ULONG CallIndex, PVOID ArgArray, PULONG ret_func, PULONG ret_arg);

//��֪��������;
ULONG Fake_VacancyFunc(ULONG a1, ULONG a2, ULONG a3, ULONG a4);

//�������ܣ�
//1�����������ж��Ƿ�����FakeKiSystemService��hook
//2����ʼ����չ�ṹ������������һ��sysʹ��
PVOID HookPort_19230();

//��ȡ��������������˳�� 
ULONG HookPort_1858E(OUT ULONG *Flag_1, OUT ULONG *Flag_2, OUT PVOID *ValueDataBuff);

//׼�����������HOOK��Ҫ�õ�����
BOOLEAN  HookPort_AllocFilterTable();

//��ʼ�������ӿ�
ULONG NTAPI HookPort_InitDeviceExtInterface(IN PDEVICE_OBJECT DeviceObject);

//��ʼ�������½������ӵ���������
PVOID NTAPI HookPort_AllocFilterRuleTable(IN ULONG NumberOfBytes);

//���ù�������
ULONG NTAPI HookPort_SetFilterRuleName(IN PFILTERFUN_RULE_TABLE FilterFun_Rule_table_head, IN CHAR *FilterRuleName);

//���ù��򿪹�
VOID NTAPI HookPort_SetFilterRule(IN PFILTERFUN_RULE_TABLE	filter_rule, IN ULONG index, IN ULONG rule);

//���ù�����˺���
BOOLEAN NTAPI HookPort_SetFilterSwitchFunction(IN PFILTERFUN_RULE_TABLE filter_rule, IN ULONG index_a2, OUT PVOID func_addr);
