#include "Fake_ZwOpenMutant.h"

//�򿪻����� ���࿪
//ԭ����ִ�к���
//��ֹ��ָ��������
NTSTATUS NTAPI After_ZwOpenMutant_Func(IN ULONG FilterIndex, IN PVOID ArgArray, IN NTSTATUS InResult, IN PULONG RetFuncArgArray)
{
	NTSTATUS       result = STATUS_SUCCESS;
	NTSTATUS       Status = STATUS_SUCCESS;
	CHAR		   ObjectInformation[0x500] = { 0 };
	ULONG          ReturnLength = NULL;
	UNICODE_STRING Q360dsmainmuteString = { 0 };
	PPUBLIC_OBJECT_TYPE_INFORMATION	pPubObjTypeInfo = NULL;
	RtlInitUnicodeString(&Q360dsmainmuteString, L"Q360dsmainmute");
	//0����ȡZwOpenMutantԭʼ����
	PHANDLE        In_MutantHandle = *(ULONG*)((ULONG)ArgArray);
	//1���ж��ϴε���ԭʼ��������ֵ
	if (!NT_SUCCESS(InResult))
	{
		return InResult;
	}
	//����ַ�Ϸ���
	if (myProbeRead(In_MutantHandle, sizeof(HANDLE), sizeof(CHAR)))
	{
		KdPrint(("ProbeRead(After_ZwOpenMutant_Func��In_MutantHandle) error \r\n"));
		return result;
	}
	Status = Safe_UserMode_ZwQueryObject(g_HighgVersionFlag, *(HANDLE*)In_MutantHandle, ObjectNameInformation, ObjectInformation, sizeof(ObjectInformation), &ReturnLength);
	pPubObjTypeInfo = (PPUBLIC_OBJECT_TYPE_INFORMATION)ObjectInformation;
	if (NT_SUCCESS(Status) && pPubObjTypeInfo->TypeName.Length)
	{
		//���࿪����ֹ��ָ��������
		if (RtlEqualUnicodeString(&pPubObjTypeInfo->TypeName, &Q360dsmainmuteString, TRUE))
		{
			//������㣬��ֹ����
			Safe_ZwNtClose(*(HANDLE*)In_MutantHandle, g_VersionFlag);
			*(HANDLE*)In_MutantHandle = 0;
			//Safe_18A72_SendR3(PsGetCurrentProcessId(), PsGetCurrentThreadId(),0xF);
			result = STATUS_ACCESS_DENIED;
		}
	}
	
	return result;
}

//�򿪻����� ���࿪
NTSTATUS NTAPI Fake_ZwOpenMutant(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS       result = STATUS_SUCCESS;
	//1��������Ӧ�ò����
	if (ExGetPreviousMode())
	{
		//�����߷Ǳ������̣���Ҫ�����ж�
		if (!Safe_QueryWhitePID(PsGetCurrentProcessId()))
		{
			*(ULONG*)ret_func = After_ZwOpenMutant_Func;
		}
	}
	return result;
}