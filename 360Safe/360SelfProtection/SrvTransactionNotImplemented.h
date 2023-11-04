#pragma once
#include <ntifs.h>
#include <ntimage.h>
#include "Data.h"
#include "WinKernel.h"
#include "SafeWarning.h"


//�޸��ض�λ
typedef struct _TYPE {
	USHORT Offset : 12;
	USHORT Type : 4;
}TYPE, *PTYPE;

//�������ܣ�
//��̬��λ��Srv.sys��SrvTransaction2DispatchTable��ַ
//����ֵ��SrvTransaction2DispatchTable�ĵ�ַ
ULONG NTAPI Safe_GetSrvTransaction2DispatchTable(IN PVOID pModuleBase, IN ULONG ModuleSize, OUT ULONG* TimeDateStamp, OUT ULONG* CheckSum);

//Mdl hook
NTSTATUS NTAPI Safe_ReplaceSrvTransaction2DispatchTable(IN PVOID pModuleBase, IN ULONG ModuleSize, IN ULONG TimeDateStamp, IN ULONG CheckSum, IN PVOID OriginalSrvTransaction2DispatchTable, IN PVOID NewOriginalSrvTransaction2DispatchTable, IN PUNICODE_STRING SrvSysPathString);

//����֮��©��(CVE-2017-0144),�滻srv!SrvTransaction2DispatchTable��0x0e
BOOLEAN NTAPI Safe_HookSrvTransactionNotImplemented();

//Fake����
NTSTATUS NTAPI Fake_SrvTransactionNotImplemented_0xE(PVOID a1);