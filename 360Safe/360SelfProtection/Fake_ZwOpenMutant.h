#pragma once
#include <ntifs.h>
#include "MemCheck.h"
#include "Data.h"
#include "SystemProcessDataList.h"

//�򿪻����� ���࿪
//ԭ����ִ�к���
//��ֹ��ָ��������
NTSTATUS NTAPI After_ZwOpenMutant_Func(IN ULONG FilterIndex, IN PVOID ArgArray, IN NTSTATUS InResult, IN PULONG RetFuncArgArray);

//�򿪻����� ���࿪
NTSTATUS NTAPI Fake_ZwOpenMutant(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg);