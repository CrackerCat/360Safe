#include "Object.h"

////���ܽ�UNICODE_STRINGת����_OBJECT_NAME_INFORMATION����������
//����Object��ѯע���·��
NTSTATUS NTAPI Safe_ObGetObjectNamePath(IN HANDLE In_ObjectHandle, OUT POBJECT_NAME_INFORMATION Out_ObjectNameInfo, IN ULONG In_Length)
{
	NTSTATUS Status = NULL;
	PVOID    Object = NULL;
	HANDLE   ObjectHandle = NULL;
	ULONG    ReturnLength = NULL;
	ULONG (NTAPI *pObGetObjectType)(HANDLE);
	UNICODE_STRING32 ObGetObjectTypeString;
	//1����ȡObGetObjectType
	Status = ObReferenceObjectByHandle(In_ObjectHandle, 0, 0, UserMode, &ObjectHandle, 0);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}
	pObGetObjectType = g_Thread_Information.pObGetObjectType;
	//�ж��Ƿ����ֵ
	if (!g_Thread_Information.pObGetObjectType)
	{
		RtlInitUnicodeString(&ObGetObjectTypeString, L"ObGetObjectType");
		pObGetObjectType = MmGetSystemRoutineAddress(&ObGetObjectTypeString);
		g_Thread_Information.pObGetObjectType = (ULONG)pObGetObjectType;
		//�޷��ҵ�ֱ���˳�
		if (!pObGetObjectType)
		{
			ObfDereferenceObject(ObjectHandle);
			return Status;
		}
	}
	//2������ObGetObjectType
	Object = pObGetObjectType(ObjectHandle);
	//3��������ͨ��ע���_OBJECTָ���ѯע���·��ObQueryNameString
	if (Object)
	{
		Status = ObQueryNameString(Object, Out_ObjectNameInfo, In_Length, &ReturnLength);
	}
	//������
	ObfDereferenceObject(ObjectHandle);
	return Status;
}

//����ָ����Object����
//�ɹ����أ�1
//ʧ�ܷ��أ�0
BOOLEAN NTAPI Safe_QueryObjectType(IN HANDLE ObjectHandle, IN PWCHAR pObjectTypeName)
{
	NTSTATUS Status = NULL;
	BOOLEAN  bReturn = FALSE;
	ULONG    ReturnLength = NULL;
	ULONG    FullPathSize = 0x1024;
	UNICODE_STRING32 CmpObjectTypeNameString;
	UNICODE_STRING32 CmpObjectPath;				//��ʱʹ��		
	POBJECT_NAME_INFORMATION pFullPath;
	ULONG Tag = 0x206B6444u;
	RtlInitUnicodeString(&CmpObjectTypeNameString, pObjectTypeName);
	pFullPath = (POBJECT_NAME_INFORMATION)Safe_AllocBuff(NonPagedPool, FullPathSize, Tag);
	if (!pFullPath)
	{
		bReturn = FALSE;
		return bReturn;
	}
	//win7����Win7���ϰ汾����
	if (g_HighgVersionFlag)
	{
		Status = Safe_ObGetObjectNamePath(ObjectHandle, pFullPath, 0xC8);
		//�ж��ַ�������
		if (!NT_SUCCESS(Status))
		{
			ExFreePool(pFullPath);
			bReturn = FALSE;
			return bReturn;
		}
		//ȡObjectNameInfo��󼸸��ֽ������Ƚ����磺xxxx//File,ֻȡ//��������ݣ�File
		CmpObjectPath.Length = CmpObjectTypeNameString.Length;
		CmpObjectPath.Buffer = (ULONG)pFullPath->Name.Buffer + pFullPath->Name.Length - CmpObjectTypeNameString.Length;
		//�ж��ַ���
		Status = RtlEqualUnicodeString(&CmpObjectPath, &CmpObjectTypeNameString, TRUE);
	}
	else
	{
		Status = ZwQueryObject(ObjectHandle, ObjectTypeInformation, &pFullPath, 0xC8, &ReturnLength);
		//�ж��ַ�������
		if (!NT_SUCCESS(Status) || (pFullPath->Name.Length != CmpObjectTypeNameString.Length))
		{
			ExFreePool(pFullPath);
			bReturn = FALSE;
			return bReturn;
		}
		//�ж��ַ���
		Status = RtlEqualUnicodeString(&pFullPath->Name, &CmpObjectTypeNameString, TRUE);
	}
	ExFreePool(pFullPath);
	bReturn = Status ? 1 : 0;
	return bReturn;
}

NTSTATUS NTAPI Safe_Run_SeDeleteObjectAuditAlarm(IN HANDLE In_Handle)
{
	NTSTATUS result = STATUS_SUCCESS;
	NTSTATUS Status = NULL;
	PVOID	 Object = NULL;
	OBJECT_HANDLE_INFORMATION HandleInformation = { 0 };
	if (g_HighgVersionFlag)
	{
		Status = ObReferenceObjectByHandle(In_Handle, 0, 0, 1, &Object,&HandleInformation);
		if (NT_SUCCESS(Status))
		{
			ObMakeTemporaryObject(Object);
			if (HandleInformation.HandleAttributes & 4)
			{
				if (!pSeDeleteObjectAuditAlarmWithTransaction)
				{
					pSeDeleteObjectAuditAlarmWithTransaction = (NTSTATUS(NTAPI *)(PVOID, HANDLE, ULONG))MmGetSystemRoutineAddress(&SeDeleteObjectAuditAlarmWithTransaction);
					if (!pSeDeleteObjectAuditAlarmWithTransaction)
						SeDeleteObjectAuditAlarm(Object,In_Handle);
				}
				pSeDeleteObjectAuditAlarmWithTransaction(Object, In_Handle, 0);
			}
			//������
			ObfDereferenceObject(Object);
		}
		result = Status;
	}
	else
	{
		//��������ɾ��һ���־ö����Ωһ����, ��ͨ������ZwMakeTemporaryObject����, ������ת��Ϊ��ʱ����
		result = ZwMakeTemporaryObject(In_Handle);
	}
	return result;
}