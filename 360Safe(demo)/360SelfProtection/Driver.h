#pragma once
#include <ntifs.h>
#include "defs.h"
#include "System.h"
#include "WinKernel.h"
#include "Regedit.h"
#include "DebugPrint.h"
#include "SSDT.h"
#include "Command.h"
#include "VirtualMemoryDataList.h"
#include "Fake_KeUserModeCallback.h"
#include "Fake_CreateProcessNotifyRoutine.h"
#include "Fake_ZwOpenMutant.h"
#include "Fake_ZwWriteFile.h"
#include "Fake_ZwOpenFile.h"
#include "Fake_ZwOpenThread.h"
#include "Fake_ZwSetSystemInformation.h"
#include "Fake_ZwAlpcSendWaitReceivePort.h"
#include "Fake_ZwOpenSection.h"
#include "Fake_ZwCreateFile.h"
#include "Fake_ZwGetNextProcess.h"
#include "Fake_ZwGetNextThread.h"
#include "Fake_ZwCreateSection.h"
#include "Fake_ZwDeleteFile.h"
#include "Fake_ZwCreateProcess.h"
#include "Fake_ZwOpenKey.h"
#include "Fake_ZwUnmapViewOfSection.h"
#include "Fake_ZwSuspendProcess.h"
#include "Fake_ZwSuspendThread.h"
#include "Fake_ZwAllocateVirtualMemory.h"
#include "Fake_ZwLoadDriver.h"
#include "Fake_ZwOpenProcess.h"
#include "Fake_ZwWriteVirtualMemory.h"
#include "Fake_ZwCreateThread.h"
#include "Fake_ZwSetSystemTime.h"
#include "Fake_ZwCreateSymbolicLinkObject.h"
#include "Fake_ZwTerminateProcess.h"
#include "Fake_ZwDuplicateObject.h"
#include "Fake_ZwMakeTemporaryObject.h"
#include "Fake_ZwEnumerateValueKey.h"

PDRIVER_OBJECT Global_DriverObject;

PDEVICE_OBJECT Global_SpShadowDeviceObject;

PDEVICE_OBJECT Global_SelfProtectionDeviceObject;

struct _DRIVER_OBJECT *Global_HookPort_DriverObject;			//HookPortDeviceObject->DriverObject

//�豸�������������
#define	SpShadow_DeviceName			L"\\Device\\360SpShadow0"
#define	SpShadow_LinkName			L"\\DosDevices\\360SpShadow0"

#define	SelfProtection_DeviceName	L"\\Device\\360SelfProtection"
#define	SelfProtection_LinkName		L"\\DosDevices\\360SelfProtection"

#define	HookPort_DeviceName			L"\\Device\\360HookPort"

#define	WIN32KSYS					L"win32k.sys"
#define	SELFPROTECTION_POOLTAG		'King'

#define FILTERFUNCNT 0x9E //���˺����ĸ��� 

typedef struct _FILTERFUN_RULE_TABLE{
	ULONG 	Size; 									//���ṹ�Ĵ�С,Ϊ0x51C	 
	struct _FILTERFUN_RULE_TABLE 	*Next; 			//ƫ��Ϊ0x4,ָ����һ���ڵ� 
	ULONG 	IsFilterFunFilledReady;             	//ƫ��Ϊ0x8,��־,�������˺������Ƿ�׼���� 
	PULONG 	SSDTRuleTableBase;                  	//ƫ��Ϊ0xC,��SSDT�����Ĺ��˹����,��Ĵ�СΪSSDTCnt*4 
	PULONG 	ShadowSSDTRuleTableBase;         		//ƫ��Ϊ0x10,��ShadowSSDT�����Ĺ��˹����,��Ĵ�СΪShadowSSDTCnt*4
	UCHAR	FilterRuleName[16];						//ƫ��Ϊ0x14~0x20���������
	PVOID   pModuleBase;							//ƫ��Ϊ0x24,δ��ȷ
	ULONG   ModuleSize;								//ƫ��Ϊ0x28,δ��ȷ
	PULONG 	FakeServiceRoutine[FILTERFUNCNT];    	//ƫ��Ϊ0x2C,���˺�������,���й��˺���0x9E��  (����)
	PULONG 	FakeServiceRuleFlag[FILTERFUNCNT];    	//ƫ��Ϊ0x2A4,���˺�������,���й��˺���0x9E�� (����)
}FILTERFUN_RULE_TABLE, *PFILTERFUN_RULE_TABLE;

/*
// sizeof(HOOKPORT_EXTENSION) = 18u
�豸��չ��������ӹ���Ľӿڣ�����������Ҫ���ӹ���ʱֻ��Ҫ��ȡHookport��������չ���������HookPort_FilterRule_Init��ʼ��һ������HookPort_SetFilterSwitchFunction ���ù�����˺�����
HookPort_FilterRule_Init		 ��ʼ�������½������ӵ���������
HookPort_SetFilterSwitchFunction ���ù�����˺���
HookPort_SetFilterRuleFlag       ���ù��򿪹�
HookPort_SetFilterRuleName       ���ù�������
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

//dword_1B10C
PFILTERFUN_RULE_TABLE	gFilterFun_Rule_table_head = NULL;

//dword_1B11C
PFILTERFUN_RULE_TABLE	gFilterFun_Rule_table_head_Temp = NULL;	//����

//��ʼ�������½������ӵ���������
ULONG (NTAPI *HookPort_AllocFilterRuleTable)(_DWORD);

//���ù�������
ULONG (NTAPI *HookPort_SetFilterRuleName)(PFILTERFUN_RULE_TABLE, CHAR*);

//���ù��򿪹�
VOID (NTAPI *HookPort_SetFilterRule)(PFILTERFUN_RULE_TABLE	After_rule, ULONG index, ULONG	rule);

//���ù�����˺���
BOOLEAN (NTAPI *HookPort_SetFilterSwitchFunction)(PFILTERFUN_RULE_TABLE After_rule, ULONG index, PVOID func_addr);

//��ʼ�����й�����˺���
ULONG Safe_Initialize_SetFilterSwitchFunction();

//��ʼ�����й��򿪹�
VOID NTAPI Safe_Initialize_SetFilterRule(PDEVICE_OBJECT pHookPortDeviceObject);

//���HookPort_SetFilterSwitchFunction�Ƿ��ȡ�ɹ�
ULONG NTAPI Safe_Run_SetFilterSwitchFunction(PFILTERFUN_RULE_TABLE After_rule, ULONG index, PVOID func_addr);

//���HookPort_SetFilterRule�Ƿ��ȡ�ɹ�
ULONG NTAPI Safe_Run_SetFilterRule(PFILTERFUN_RULE_TABLE After_rule, ULONG index, ULONG	rule);

//�õ��豸������Ϣ
BOOLEAN Safe_GetSymbolicLinkObjectData();

//���ݰ汾��ȡƫ��ֵ
BOOLEAN NTAPI Safe_Initialize_Data();

//Win10δ���ĺ���
PVOID Safe_1391C_Win10();


//�õ�SSDT��SSSDT�Ļ���ַ
NTSTATUS NTAPI Safe_GetSSDTorSSSDTData();

//��ʼ��360Safe�������
VOID NTAPI Safe_InitializeSafeWhiteProcessList();

/**************************Fake���������ģ�***************************/
//�����ļ�
#define ZwCreateFile_FilterIndex							0x8

//д�ļ�
#define	ZwWriteFile_FilterIndex								0xB

//��������
#define	ZwCreateProcess_FilterIndex							0xD

//��������Ex
#define	ZwCreateProcessEx_FilterIndex						0xE

//�����߳�
#define ZwCreateThread_FilterIndex							0x10 

//���߳�
#define ZwOpenThread_FilterIndex							0x11

//ɾ���ļ�
#define ZwDeleteFile_FilterIndex							0x12

//���ļ�
#define ZwOpenFile_FilterIndex								0x13 

//��������
#define ZwTerminateProcess_FilterIndex						0x15 

//�����д����
#define ZwWriteVirtualMemory_FilterIndex	                0x1A

//�����ļ�ӳ��
#define ZwCreateSection_FilterIndex							0x1E

//��section object
#define ZwOpenSection_FilterIndex							0x1F

//������������
#define ZwCreateSymbolicLinkObject_FilterIndex				0x20

//��������
#define ZwLoad_Un_Driver_FilterIndex						0x22

//��������
#define ZwSetSystemInformation_FilterIndex					0x24

//����ʱ��
#define ZwSetSystemTime_FilterIndex                         0x25

//�򿪽���
#define ZwOpenProcess_FilterIndex							0x2F

//��ע����ֵ
#define ZwOpenKey_FilterIndex								0x32

//�������
#define ZwZwDuplicateObject_FilterIndex						0x33

//RPCͨѶ
#define ZwAlpcSendWaitReceivePort_FilterIndex               0x44

//���̻ص�
#define	CreateProcessNotifyRoutine_FilterIndex				0x45

//ȡ��ӳ��Ŀ����̵��ڴ�
#define ZwUnmapViewOfSection_FilterIndex					0x46

//����DLLע���
#define	ClientLoadLibrary_FilterIndex						0x4B

//����ռ�
#define ZwAllocateVirtualMemory_FilterIndex					0x4E

//�򿪻�����
#define ZwOpenMutant_FilterIndex							0x51

//�����߳�
#define ZwGetNextThread_FilterIndex							0x53

//��������
#define ZwGetNextProcess_FilterIndex						0x54

//ö��valuekey
#define ZwEnumerateValueKey_FilterIndex						0x59

//���ö���ת������ʱ����
#define ZwMakeTemporaryObject_FilterIndex                   0x7F

//�̹߳���
#define ZwSuspendThread_FilterIndex							0x93

//���̹���
#define	ZwSuspendProcess_FilterIndex					    0x94

//ȡ��ӳ��Ŀ����̵��ڴ� Win8~Win10
#define ZwUnmapViewOfSectionIndex_Win8_Win10_FilterIndex	0x96
/**************************Fake���������ģ�***************************/

/**************************�������õı���****************************/
//Safe_Initialize_Data
//dword_34E64
ULONG Global_InitializeDataFlag = 0;		//��ֹ���γ�ʼ������1��������ִ�У� ��0������δִ�У�
/**************************�������õı���****************************/