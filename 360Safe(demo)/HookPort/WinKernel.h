#pragma once
#include <ntddk.h>
#include <ntimage.h>
#include "defs.h"
#define	SELFPROTECTION_POOLTAG		'INFT'

ULONG dword_1B170;					//sub_193F2��������������ǣ�ȡֵ0~2֮��

typedef struct tagSYSTEM_MODULE_INFORMATION {
	ULONG Reserved[2];
	PVOID Base;
	ULONG Size;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT ModuleNameOffset;
	CHAR ImageName[256];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

//
// System Information Classes.
//
typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation,
	SystemProcessorInformation,              // obsolete...delete
	SystemPerformanceInformation,
	SystemTimeOfDayInformation,
	SystemPathInformation,
	SystemProcessInformation,                //ϵͳ������Ϣ
	SystemCallCountInformation,
	SystemDeviceInformation,
	SystemProcessorPerformanceInformation,
	SystemFlagsInformation,
	SystemCallTimeInformation,
	SystemModuleInformation,				//ϵͳģ��
	SystemLocksInformation,
	SystemStackTraceInformation,
	SystemPagedPoolInformation,
	SystemNonPagedPoolInformation,
	SystemHandleInformation,
	SystemObjectInformation,
	SystemPageFileInformation,
	SystemVdmInstemulInformation,
	SystemVdmBopInformation,
	SystemFileCacheInformation,
	SystemPoolTagInformation,
	SystemInterruptInformation,
	SystemDpcBehaviorInformation,
	SystemFullMemoryInformation,
	SystemLoadImage,					   //26 ��������
	SystemUnloadGdiDriverInformation,
	SystemTimeAdjustmentInformation,
	SystemSummaryMemoryInformation,
	SystemMirrorMemoryInformation,
	SystemPerformanceTraceInformation,
	SystemObsolete0,
	SystemExceptionInformation,
	SystemCrashDumpStateInformation,
	SystemKernelDebuggerInformation,
	SystemContextSwitchInformation,
	SystemRegistryQuotaInformation,
	SystemLoadAndCallImage,					//38 ��������
	SystemPrioritySeperation,
	SystemVerifierAddDriverInformation,
	SystemVerifierRemoveDriverInformation,
	SystemProcessorIdleInformation,
	SystemLegacyDriverInformation,
	SystemCurrentTimeZoneInformation,
	SystemLookasideInformation,
	SystemTimeSlipNotification,
	SystemSessionCreate,
	SystemSessionDetach,
	SystemSessionInformation,
	SystemRangeStartInformation,
	SystemVerifierInformation,
	SystemVerifierThunkExtend,
	SystemSessionProcessInformation,
	SystemLoadGdiDriverInSystemSpace,
	SystemNumaProcessorMap,
	SystemPrefetcherInformation,
	SystemExtendedProcessInformation,
	SystemRecommendedSharedDataAlignment,
	SystemComPlusPackage,
	SystemNumaAvailableMemory,
	SystemProcessorPowerInformation,
	SystemEmulationBasicInformation,
	SystemEmulationProcessorInformation,
	SystemExtendedHandleInformation,
	SystemLostDelayedWriteInformation,
	SystemBigPoolInformation,
	SystemSessionPoolTagInformation,
	SystemSessionMappedViewInformation,
	SystemHotpatchInformation,
	SystemObjectSecurityMode,
	SystemWatchdogTimerHandler,
	SystemWatchdogTimerInformation,
	SystemLogicalProcessorInformation,
	SystemWow64SharedInformation,
	SystemRegisterFirmwareTableInformationHandler,
	SystemFirmwareTableInformation,
	SystemModuleInformationEx,
	SystemVerifierTriageInformation,
	SystemSuperfetchInformation,
	SystemMemoryListInformation,
	SystemFileCacheInformationEx,
	MaxSystemInfoClass   // MaxSystemInfoClass should always be the last enum
} SYSTEM_INFORMATION_CLASS;

extern
NTSTATUS
ZwQuerySystemInformation(
IN ULONG SystemInformationClass,
IN PVOID SystemInformation,
IN ULONG SystemInformationLength,
OUT PULONG ReturnLength);

NTSTATUS
NTAPI
ZwQueryInformationProcess(
IN HANDLE ProcessHandle,
IN PROCESSINFOCLASS ProcessInformationClass,
OUT PVOID ProcessInformation,
IN ULONG ProcessInformationLength,
OUT PULONG ReturnLength OPTIONAL
);

extern
PVOID NTAPI
RtlImageDirectoryEntryToData(
IN PVOID          BaseAddress,
IN BOOLEAN        ImageLoaded,
IN ULONG		   Directory,
OUT PULONG        Size);

typedef struct _SYSTEM_HANDLE_INFORMATION{
	ULONG ProcessID;                //���̵ı�ʶID 
	UCHAR ObjectTypeNumber;         //�������� 
	UCHAR Flags;					//0x01 = PROTECT_FROM_CLOSE,0x02 = INHERIT 
	USHORT Handle;					//����������ֵ 
	PVOID  Object;					//��������ָ���ں˶����ַ 
	ACCESS_MASK GrantedAccess;      //�������ʱ��׼��Ķ���ķ���Ȩ 
}SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

PVOID HookPort_GetSymbolAddress(PANSI_STRING SymbolName, PVOID NtImageBase);

PVOID NTAPI HookPort_GetAndReplaceSymbol(PVOID ImageBase, PANSI_STRING SymbolName, PVOID ReplaceValue, PVOID *SymbolAddr);

PVOID NTAPI HookPort_QuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass);

BOOLEAN  HookPort_FindModuleBaseAddress(ULONG func_addr, PVOID *pModuleBase_a2, ULONG *ModuleSize_a3, PVOID *FilterRuleName, ULONG RuleNameLen);


// �˺������һ��޸�ModuleNameָ����ģ���е�FunctionNameָ���ĺ���
PULONG  HookPort_HookImportedFunction(PVOID pModuleBase, ULONG ModuleSize, CONST CHAR *FunctionName, CONST CHAR *ModuleName, PVOID *RetValue);

//�ָ��ڴ汣�� 
VOID PageProtectOn();

////ȥ���ڴ汣��
VOID PageProtectOff();

//�ͷ�MDL
VOID  HookPort_RemoveLockMemory(PMDL pmdl);
//ӳ��MDL
PVOID HookPort_LockMemory(PVOID VirtualAddress, ULONG Length, PVOID *Mdl_a3, ULONG Version_Win10_Flag);

//��ȡCPU��Ŀ
ULONG HookPort_CheckCpuNumber(IN RTL_OSVERSIONINFOEXW osverinfo);

//��ȡcsrss�Ľ���id
HANDLE	HookPort_GetApiPortProcessId(IN RTL_OSVERSIONINFOEXW osverinfo);

//���ݱ���������ĳ�Ա(DriverObject->DriverStart)��ȡ����LoadOrderIndex 
BOOLEAN  HookPort_GetModuleLoadOrderIndex(IN PVOID pModuleBase, OUT ULONG *LoadOrderIndex);

//���ݺ�������ȡָ���ں˻�ַ
BOOLEAN NTAPI HookPort_GetModuleBaseAddress(IN CONST CHAR *ModuleName, OUT PVOID *pModuleBase, OUT ULONG *ModuleSize, OUT USHORT *LoadOrderIndex);

//���Ϸ��汾��
ULONG HookPort_CheckSysVersion(IN RTL_OSVERSIONINFOEXW osverinfo, IN PVOID *NtImageBase);