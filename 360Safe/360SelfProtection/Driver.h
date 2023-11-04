#pragma once
#include <ntifs.h>
#include "HookPortDeviceExtension.h"
#include "defs.h"
#include "System.h"
#include "WinKernel.h"
#include "Regedit.h"
#include "DebugPrint.h"
#include "SSDT.h"
#include "Command.h"
#include "VirtualMemoryDataList.h"

//�豸�������������
#define	SpShadow_DeviceName			L"\\Device\\360SpShadow0"
#define	SpShadow_LinkName			L"\\DosDevices\\360SpShadow0"

#define	SelfProtection_DeviceName	L"\\Device\\360SelfProtection"
#define	SelfProtection_LinkName		L"\\DosDevices\\360SelfProtection"

#define	HookPort_DeviceName			L"\\Device\\360HookPort"

#define	WIN32KSYS					L"win32k.sys"
#define	SELFPROTECTION_POOLTAG		'King'


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

/**************************�������õı���****************************/
//Safe_Initialize_Data
//dword_34E64
ULONG Global_InitializeDataFlag;		//��ֹ���γ�ʼ������1��������ִ�У� ��0������δִ�У�
/**************************�������õı���****************************/