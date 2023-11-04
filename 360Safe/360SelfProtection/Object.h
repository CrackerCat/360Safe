#pragma once
#include <ntifs.h>
#include "Data.h"
#include "WinKernel.h"

NTSTATUS(NTAPI *pSeDeleteObjectAuditAlarmWithTransaction)(PVOID, HANDLE, ULONG);

//����Object��ѯע���·��
NTSTATUS NTAPI Safe_ObGetObjectNamePath(IN HANDLE In_ObjectHandle, OUT POBJECT_NAME_INFORMATION Out_ObjectNameInfo, IN ULONG In_Length);

//����ָ����Object����
//�ɹ����أ�1
//ʧ�ܷ��أ�0
BOOLEAN NTAPI Safe_QueryObjectType(IN HANDLE ObjectHandle, IN PWCHAR pObjectTypeName);

NTSTATUS NTAPI Safe_Run_SeDeleteObjectAuditAlarm(IN HANDLE In_Handle);



