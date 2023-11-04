#pragma once
#include <ntifs.h>
#include "WinKernel.h"
#include "Data.h"
#include "DebugPrint.h"
#include "IDT.h"
#include "FilterHook.h"

#define CPUNUMBERMAX  0x32													//CPU������󲻳���32
//Fake_ZwSetEvent������صı�־λ
HANDLE Global_Fake_ZwSetEvent_Handle;										//��ٵ�ZwSetEvent��������ţ�
ULONG  Global_ZwSetEventHookFlag;											//�ж�Fake_ZwSetEvent�����Ƿ�ִ�гɹ�
ULONG  Global_IdtHook_Or_InlineHook;										//�жϲ���IDT����ͳInlineHook��ʽ

//Fake_ZwSetEvent����ʹ�õı���
PVOID p_jmpstub_code;														//new�ռ�洢�������תָ��
ULONG p_jmpstub_codeLen ;													//new�ռ䳤��
ULONG g_DpcFlag_dword_1B41C;
KSPIN_LOCK g_SpinLock_WhiteList;

// ���Ǵ�αKiFastCallEntry�ķ��ص�ַ
// dword_1A6F4
PVOID g_KiFastCallEntry_Fake_rtn_address;


KDPC g_Dpc[CPUNUMBERMAX];

//360HOOK��
PVOID g_KiFastCallEntry_360HookPoint;										//�߰汾>2003
PVOID g_Fake_KiSystemServiceFuncAddress;									//�Ͱ汾<2003
//IDT��ʽ��hook
ULONG Global_KiTrap04;

//HookPort_Hook_153D0����ʹ�õģ�����Dpc������
typedef struct _TsFlt_DPC
{
	PKSPIN_LOCK pSpinLock;
	PULONG pFlag;
}TsFlt_DPC, *PTsFlt_DPC;

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


//��������5�ֽ�hook
VOID NTAPI HookPort_InlineHook5Byte_1521C(ULONG JmpAddress_a1, ULONG MdlAddress_a2, ULONG a3, ULONG a4);

//������5�ֽ�hook
VOID NTAPI HookPort_InterlockedCompareExchange64_15236(ULONG* a1, ULONG a2, ULONG a3, ULONG a4);

/**********ͨ��hook ZwSetEvent������ʽ���޸�KiFastCallEntry***********/
//�߰汾>2003 KiFastCallEntry

NTSTATUS NTAPI HookPort_InstallZwSetEventHook();
NTSTATUS NTAPI Fake_ZwSetEvent(HANDLE EventHandle, PULONG PreviousState);
ULONG sub_1567A(IN RTL_OSVERSIONINFOEXW osverinfo);															//��ȫû��������������ڸ������������
/**********ͨ��hook ZwSetEvent������ʽ���޸�KiFastCallEntry***********/

/**********Hook KiSystemService**************************************/
//�Ͱ汾<2003 KiSystemService

//InlineHook hook��KiSystemService
BOOLEAN HookPort_SetFakeKiSystemServiceAddress();

//��ȡKiSystemService��Hook��
ULONG HookPort_GetKiSystemService_HookPoint(IN ULONG MmUserProbeAddress, IN ULONG NtImageBase, IN ULONG NtImageSize, OUT ULONG *Index);

//�п������
//��Fake_ZwSetEvent����һ��
ULONG HookPort_SetFakeKiSystemServiceData(ULONG ImageBase_a1, ULONG ImageSize_a2);
/**********Hook KiSystemService**************************************/
