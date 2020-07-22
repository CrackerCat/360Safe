#pragma once
#include <ntifs.h>
#include "WinBase.h"
#include "MemCheck.h"
#include "Data.h"
#include "SystemProcessDataList.h"

NTKERNELAPI
NTSTATUS
PsLookupProcessThreadByCid(
	__in PCLIENT_ID Cid,
	__deref_opt_out PEPROCESS *Process,
	__deref_out PETHREAD *Thread
);

//����˵����
//1���򿪵��Ǳ������̣����򿪵ľ�����¸���һ�ݣ���Ȩ�˸��ģ�ԭʼ��ֱ��Close����
//2���򿪵��ǷǱ�������ֱ������
NTSTATUS NTAPI After_ZwOpenProcess_Func(IN ULONG FilterIndex, IN PVOID ArgArray, IN NTSTATUS InResult, IN PULONG RetFuncArgArray);

//�򿪽���
NTSTATUS NTAPI Fake_ZwOpenProcess(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg);