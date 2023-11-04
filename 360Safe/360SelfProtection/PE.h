#pragma once
#include <ntddk.h>
#include <ntimage.h>
#include "defs.h"

extern
PVOID NTAPI
RtlImageDirectoryEntryToData(
IN PVOID          BaseAddress,
IN BOOLEAN        ImageLoaded,
IN ULONG		   Directory,
OUT PULONG        Size);

PVOID Safe_GetSymbolAddress(PANSI_STRING SymbolName, PVOID NtImageBase);

//���ݺ�������ImageBaseȥ�������ҵ���Ӧ�ĺ�����ַ
PVOID NTAPI Safe_GetAndReplaceSymbol(PVOID ImageBase, PANSI_STRING SymbolName, PVOID ReplaceValue, PVOID *SymbolAddr);

//���PE�ļ�������Ϣ
BOOLEAN NTAPI Safe_CheckPeFile(IN PVOID pModuleBase);

//RVAת����VA
ULONG NTAPI Safe_RvaToVa(IN PVOID pModuleBase, ULONG dwRva);

/************************PE�ṹ����ǩ�����*****************************/
//��Դ��
BOOLEAN NTAPI Safe_17C8A(IN PVOID pModuleBase, IN ULONG ModuleSize);
/************************PE�ṹ����ǩ�����*****************************/