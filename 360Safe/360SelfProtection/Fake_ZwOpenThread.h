#pragma once
#include <ntifs.h>
#include "MemCheck.h"
#include "Data.h"
#include "SystemProcessDataList.h"
#include "WinBase.h"
//���߳�
//ԭ����ִ�к���
//���򿪵������̵߳ľ����ֱ�ӰѾ�����㲢�ҷ��ش���ֵ
NTSTATUS NTAPI After_ZwOpenThread_Func(IN ULONG FilterIndex, IN PVOID ArgArray, IN NTSTATUS InResult, IN PULONG RetFuncArgArray);

//���߳�
NTSTATUS NTAPI Fake_ZwOpenThread(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg);