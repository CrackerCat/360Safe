#pragma once
#include <ntddk.h>
#include "WinKernel.h"
#include "DebugPrint.h"
//KeServiceDescriptorTable�ı�ṹ
typedef struct ServiceDescriptorEntry {
	ULONG *ServiceTableBase;				 // ������ַ
	ULONG *ServiceCounterTableBase;			 // �������ַ
	ULONG NumberOfServices;					 // ������ĸ���
	UCHAR *ParamTableBase;					 // �������Ĳ��������������ʼ��ַ�������ÿһ����Առ1�ֽڣ���¼��ֵ�Ƕ�Ӧ�����Ĳ�������*4
} ServiceDescriptorTableEntry, *PServiceDescriptorTableEntry;

//��ȡSSDT��ַ
NTSTATUS NTAPI HookPort_GetSSDTTableAddress(OUT PVOID* SSDT_KeServiceTableBase, OUT ULONG* SSDT_KeNumberOfServices, OUT PVOID* SSDT_KeParamTableBase, IN PVOID* NtImageBase);