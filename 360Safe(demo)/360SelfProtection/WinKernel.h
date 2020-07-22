#pragma once
#include <ntifs.h>
#include "Xor.h"
#include "NoSystemProcessDataList.h"
#include "WhiteList.h"
#include "Data.h"
#include "PE.h"
#define	SELFPROTECTION_POOLTAG		'INFT'


#define ObjectNameInformation		1	

#ifdef _X86_
#define PEB_LDR_DATA_OFFSET           0xC
#elif _AMD64_
#endif

typedef struct _PEB_LDR_DATA
{
	ULONG		Length;
	ULONG		Initialized;
	PVOID		SsHandle;
	LIST_ENTRY	InLoadOrderModuleList; // ref. to PLDR_DATA_TABLE_ENTRY->InLoadOrderModuleList
	LIST_ENTRY	InMemoryOrderModuleList; // ref. to PLDR_DATA_TABLE_ENTRY->InMemoryOrderModuleList
	LIST_ENTRY	InInitializationOrderModuleList; // ref. to PLDR_DATA_TABLE_ENTRY->InInitializationOrderModuleList
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY		InLoadOrderLinks;
	LIST_ENTRY		InMemoryOrderLinks;
	LIST_ENTRY		InInitializationOrderLinks;
	PVOID			DllBase;
	PVOID			EntryPoint;
	ULONG			SizeOfImage;	// in bytes
	UNICODE_STRING	FullDllName;
	UNICODE_STRING	BaseDllName;
	ULONG			Flags;			// LDR_*
	USHORT			LoadCount;
	USHORT			TlsIndex;
	LIST_ENTRY		HashLinks;
	PVOID			SectionPointer;
	ULONG			CheckSum;
	ULONG			TimeDateStamp;
	//    PVOID			LoadedImports;					// seems they are exist only on XP !!!
	//    PVOID			EntryPointActivationContext;	// -same-
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

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

typedef struct _SYSTEM_LOAD_AND_CALL_IMAGE{
	UNICODE_STRING ModuleName;
} SYSTEM_LOAD_AND_CALL_IMAGE, *PSYSTEM_LOAD_AND_CALL_IMAGE;

typedef struct _SYSTEM_THREAD_INFORMATION
{
	LARGE_INTEGER   KernelTime;
	LARGE_INTEGER   UserTime;
	LARGE_INTEGER   CreateTime;
	ULONG           WaitTime;
	PVOID           StartAddress;
	CLIENT_ID       ClientId;
	KPRIORITY       Priority;
	KPRIORITY       BasePriority;
	ULONG           ContextSwitchCount;
	LONG            State;// ״̬,��THREAD_STATEö�������е�һ��ֵ
	LONG            WaitReason;//�ȴ�ԭ��, KWAIT_REASON�е�һ��ֵ
} SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFRMATION
{
	ULONG           NextEntryDelta;//ָ����һ���ṹ���ָ��
	ULONG           ThreadCount;//�����̵����߳���
	ULONG           Reserved1[6];//����
	LARGE_INTEGER   CreateTime;//���̴�����ʱ��
	LARGE_INTEGER   UserTime;//���û����ʹ��ʱ��
	LARGE_INTEGER   KernelTime;//���ں˲��ʹ��ʱ��
	UNICODE_STRING  ProcessName; // ������
	KPRIORITY       BasePriority;
	ULONG           ProcessId;//����ID
	ULONG           InheritedFromProcessId;
	ULONG           HandleCount; // ���̵ľ������
	ULONG           Reserved2[2]; // ����
	VM_COUNTERS     VmCounters;
	IO_COUNTERS     IoCounters;
	SYSTEM_THREAD_INFORMATION Threads[1]; // ���߳���Ϣ����
}SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

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

NTSTATUS ZwQueryDirectoryObject(
	IN HANDLE       DirectoryHandle,
	OUT PVOID       Buffer,
	IN ULONG        Length,
	IN BOOLEAN      ReturnSingleEntry,
	IN BOOLEAN      RestartScan,
	IN OUT PULONG   Context,
	OUT PULONG      ReturnLength OPTIONAL
	);

NTKERNELAPI
NTSTATUS
ObReferenceObjectByName(
	__in PUNICODE_STRING ObjectName,
	__in ULONG Attributes,
	__in_opt PACCESS_STATE AccessState,
	__in_opt ACCESS_MASK DesiredAccess,
	__in POBJECT_TYPE ObjectType,
	__in KPROCESSOR_MODE AccessMode,
	__inout_opt PVOID ParseContext,
	__out PVOID *Object
);

// ������Ϣ����  
typedef struct _DIRECTORY_BASIC_INFORMATION {
	UNICODE_STRING ObjectName;
	UNICODE_STRING ObjectTypeName;
} DIRECTORY_BASIC_INFORMATION, *PDIRECTORY_BASIC_INFORMATION;

//���ݺ�������ȡָ���ں˻�ַ
BOOLEAN NTAPI Safe_GetModuleBaseAddress(IN PUNICODE_STRING ModuleName, OUT PVOID *pModuleBase, OUT ULONG *ModuleSize, OUT USHORT *LoadOrderIndex);

//PsGetProcessImageFileName����
ULONG NTAPI Safe_PsGetProcessImageFileName(PEPROCESS Process, UCHAR* ImageFileName, ULONG ImageFileNameLen);

//�Ƚ�ImageFileName
//��ͬ����1����ͬ���ط�0
BOOLEAN NTAPI Safe_CmpImageFileName(UCHAR *ImageFileName);

//ͨ����̷�ʽʹ�� MDL �ƹ� KiServiceTable ��ֻ������
PVOID Safe_LockMemory(PVOID VirtualAddress, ULONG Length, PVOID *Mdl_a3);

//�ͷ�MDL�ռ�
PVOID  Safe_RemoveLockMemory(PMDL pmdl);

//new�ռ�
PVOID Safe_AllocBuff(POOL_TYPE PoolType, ULONG Size, ULONG Tag);

//�ͷſռ�
PVOID Safe_ExFreePool(IN PVOID pBuff);

//��ȡZwOpenSymbolicLinkObject������ַ��ִ��
NTSTATUS NTAPI Safe_RunZwOpenSymbolicLinkObject(OUT PHANDLE LinkHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, IN ULONG Version_Flag, IN PVOID ServiceTableBase, IN ULONG NumberOfServices);

// ��ѯ����
BOOLEAN NTAPI Safe_ZwQueryDirectoryObject(IN PUNICODE_STRING CmpString_a1, IN HANDLE DirectoryHandle, IN PUNICODE_STRING CmpString_a3);

//PsGetProcessId����
ULONG NTAPI Safe_pPsGetProcessId(PVOID VirtualAddress);

//ͨ��Handle��ȡEprocess->UniqueProcessId
HANDLE NTAPI Safe_GetUniqueProcessId(HANDLE Handle);

//���ҷ������ӣ���Open��
BOOLEAN NTAPI Safe_ZwQuerySymbolicLinkObject_Open(IN PUNICODE_STRING ObjectName, IN HANDLE DirectoryHandle, OUT PUNICODE_STRING Out_LinkTarget);

//���ҷ�������(����Open)
BOOLEAN NTAPI Safe_ZwQuerySymbolicLinkObject(IN HANDLE LinkHandle, OUT PUNICODE_STRING Out_LinkTarget);

//ZwCreateFile��ZwOpenFileʹ�õ�
//��ֹ�û����ܱ���·��
BOOLEAN NTAPI Safe_CheckProtectPath(IN HANDLE FileHandle, IN KPROCESSOR_MODE AccessMode);

//��ȡ���Ȩ��
NTSTATUS NTAPI Safe_GetGrantedAccess(IN HANDLE Handle, OUT PACCESS_MASK Out_GrantedAccess);

//����LDR����ȡ��DLL�������Ϣ
ULONG NTAPI Safe_PeLdrFindExportedRoutineByName(IN PCHAR In_SourceAPINameBuff, IN ULONG In_Flag);

//����ProcessHandleCount��ProcessHandleTracing��ProcessBasicInformation
NTSTATUS NTAPI Safe_ZwQueryInformationProcess(IN HANDLE ProcessHandle, IN PROCESSINFOCLASS ProcessInformationClass, OUT PVOID ProcessInformation, IN ULONG ProcessInformationLength, OUT ULONG ReturnLength);

//���������û��������ȡ�������ʧ��
BOOLEAN NTAPI Safe_QueryProcessHandleOrHandleCount(IN HANDLE ProcessHandle);

//��ѯ�߳���Ϣ��
//��������PID���ҵ������жϽ����̸߳�������1ʱ�򣨸մ���ʱ������㣩
BOOLEAN NTAPI Safe_FindEprocessThreadCount(IN HANDLE In_ProcessHandle, IN BOOLEAN In_Flag);

/************************PE�ṹ����ǩ�����(����)*****************************/
BOOLEAN NTAPI Safe_CheckProcessNameSign(IN UNICODE_STRING SourceString);
BOOLEAN NTAPI Safe_18108(IN PCWSTR SourceString);
/************************PE�ṹ����ǩ�����(����)*****************************/









