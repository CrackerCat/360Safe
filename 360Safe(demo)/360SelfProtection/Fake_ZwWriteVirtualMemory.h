#pragma once
#include <ntifs.h>
#include "MemCheck.h"
#include "Data.h"
#include "SystemProcessDataList.h"
#include "Object.h"
#include "VirtualMemoryDataList.h"
#include "SafeWarning.h"

//�޸ı������̵�ַ or �޸�PEB���� ��������������������������
BOOLEAN NTAPI Safe_CheckWriteMemory_PEB(IN HANDLE In_Handle, IN ULONG In_BaseAddress, SIZE_T In_BufferLength);

///�����д����
NTSTATUS NTAPI Fake_ZwWriteVirtualMemory(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg);