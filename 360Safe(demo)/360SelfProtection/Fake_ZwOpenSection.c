/*
˵����
�ص��չ˶������\\Device\\PhysicalMemory��\\KnownDlls\\
���1��
����\\Device\\PhysicalMemoryֱ�Ӿ�����㣬���󷵻�
���2��
����\\KnownDlls\\
���copy�ɹ�����Ȩȥ��DELETEȨ�޲���R3ͨѶ���ɹ�����
���copyʧ�ܣ�������㣬���󷵻�
�ο����ϣ�
1����PhysicalMemory���� 						   
��ַ��https://bbs.pediy.com/thread-94203.htm
2��������һ��360��ȫ��ʿ��HOOK(��)�����ܹ���ʵ��   
��ַ��https://bbs.pediy.com/thread-99460.htm?source=1
*/
#include "Fake_ZwOpenSection.h"


//��section object
//ԭ����ִ�к���
NTSTATUS NTAPI After_ZwOpenSection_Func(IN ULONG FilterIndex, IN PVOID ArgArray, IN NTSTATUS InResult, IN PULONG RetFuncArgArray)
{
	NTSTATUS       result = STATUS_SUCCESS;
	NTSTATUS       Status = STATUS_SUCCESS;
	SIZE_T         PhysicalMemorySize = 0x16;					//\\Device\\PhysicalMemory�ַ�����С
	SIZE_T         KnownDllsSize = 0xB;						   //\\KnownDlls\\�ַ�����С
	CHAR		   ObjectInformation[0x500] = { 0 };
	ULONG          ReturnLength = NULL;
	HANDLE         TargetHandle = NULL;
	ACCESS_MASK    Out_GrantedAccess = NULL;
	PPUBLIC_OBJECT_TYPE_INFORMATION	pPubObjTypeInfo = NULL;
	//0����ȡZwOpenSectionԭʼ����
	PHANDLE SectionHandle = *(ULONG*)((ULONG)ArgArray);			//���������
	//1���ж��ϴε���ԭʼ��������ֵ
	if (!NT_SUCCESS(InResult))
	{
		return InResult;
	}
	//����ַ�Ϸ���
	if (myProbeRead(SectionHandle, sizeof(HANDLE), sizeof(CHAR)))
	{
		KdPrint(("ProbeRead(After_ZwOpenSection_Func��SectionHandle) error \r\n"));
		return result;
	}
	//2���������·��
	Status = Safe_UserMode_ZwQueryObject(g_HighgVersionFlag, *(HANDLE*)SectionHandle, ObjectNameInformation, ObjectInformation, sizeof(ObjectInformation), &ReturnLength);
	pPubObjTypeInfo = (PPUBLIC_OBJECT_TYPE_INFORMATION)ObjectInformation;
	if (!NT_SUCCESS(Status) ||
		!pPubObjTypeInfo->TypeName.Length
		)
	{
		return result;
	}
	//2��1 PhysicalMemory���
	if (!_wcsnicmp(pPubObjTypeInfo->TypeName.Buffer, L"\\Device\\PhysicalMemory", PhysicalMemorySize))
	{
		//������㣬��ֹ����
		Safe_ZwNtClose(*(HANDLE*)SectionHandle, g_VersionFlag);
		*(HANDLE*)SectionHandle = 0;
		//Safe_18A72_SendR3(PsGetCurrentProcessId(), PsGetCurrentThreadId(),0x8);
		result = STATUS_ACCESS_DENIED;
	}
	//2��2 KnownDlls���
	else if (!_wcsnicmp(pPubObjTypeInfo->TypeName.Buffer, L"\\KnownDlls\\", KnownDllsSize))
	{
		//2��3 ��ȡ�þ��ԭʼȨ��
		Status  = Safe_GetGrantedAccess(*(HANDLE*)SectionHandle, &Out_GrantedAccess);
		if (NT_SUCCESS(Status))
		{
			//2��4 ����������ҽ�Ȩ,ɾ��DELETEȨ��
			//0xFFFEFFFF = 16  DELETE  Delete access.
			Status = ZwDuplicateObject(
				NtCurrentProcess(),						//__in HANDLE SourceProcessHandle,
				*(HANDLE*)SectionHandle,				//__in HANDLE SourceHandle,
				NtCurrentProcess(),						//__in_opt HANDLE TargetProcessHandle,
				&TargetHandle,							//__out_opt PHANDLE TargetHandle,
				Out_GrantedAccess,						//__in ACCESS_MASK DesiredAccess,
				NULL,									//__in ULONG HandleAttributes,
				NULL									//__in ULONG Options
				);
			if (NT_SUCCESS(Status))
			{
				//�滻���˸��ľ��
				Safe_ZwNtClose(*(HANDLE*)SectionHandle, g_VersionFlag);
				*(HANDLE*)SectionHandle = TargetHandle;
				//Safe_18A72_SendR3(PsGetCurrentProcessId(), PsGetCurrentThreadId(),0x9);
				result = STATUS_SUCCESS;
			}
			else
			{
				//�������ɹ�ֱ�����ԭʼ���
				Safe_ZwNtClose(*(HANDLE*)SectionHandle, g_VersionFlag);
				*(HANDLE*)SectionHandle = 0;
				result = STATUS_ACCESS_DENIED;
			}
		}
	}
	return result;
}

//��section object
NTSTATUS NTAPI Fake_ZwOpenSection(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS       result = STATUS_SUCCESS;
	NTSTATUS       Status = STATUS_SUCCESS;
	//Section Access Rights
	ACCESS_MASK    DesiredAccess_Flag =														   //0x52010002
		(GENERIC_WRITE | GENERIC_ALL) |                                                        //0x50000000 = GENERIC_WRITE | GENERIC_ALL
		(MAXIMUM_ALLOWED) |                                                                    //0x02000000 = MAXIMUM_ALLOWED
		(DELETE) |   																		   //0x00010000 = DELETE
		(SECTION_MAP_WRITE);																   //0x00000002 = SECTION_MAP_WRITE
	
	//0����ȡZwOpenSectionԭʼ����
	ACCESS_MASK	   In_DesiredAccess = *(ULONG*)((ULONG)ArgArray + 4);
	//1��������Ӧ�ò����
	if (ExGetPreviousMode())
	{
		//����Ȩ�޲�����
		if (In_DesiredAccess & DesiredAccess_Flag)
		{
			//�����߷Ǳ������̣���Ҫ�����ж�
			if (!Safe_QueryWhitePID(PsGetCurrentProcessId()))
			{
				//�������ú���
				*(ULONG*)ret_func = After_ZwOpenSection_Func;
			}
		}
	}
	return result;
}