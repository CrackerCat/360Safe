/*
˵����
��Ҫ����ע���ע��
���ģ�
1������HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion \Windows\AppInit_DLLs��ע���ע�룬����ο�����1��
2������\\SHELLEXECUTEHOOKS����������
3������\\SAFER\\CODEIDENTIFIERS��������Ʋ��ԣ�ƽʱû�Ӵ�������֪ʶä��������ο�����2��
��������ϼ�����Ϊֱ�����أ���������㣬������STATUS_ACCESS_DENIED
�ο����ϣ�
1�����ֳ�����ע������
��ַ��https://bbs.pediy.com/thread-227075.htm
2��Determine Allow-Deny List and Application Inventory for Software Restriction Policies
��ַ��https://docs.microsoft.com/en-us/windows-server/identity/software-restriction-policies/determine-allow-deny-list-and-application-inventory-for-software-restriction-policies
*/
#include "Fake_ZwOpenKey.h"


//************************************     
// ��������: After_ZwOpenKey_Func     
// ����˵����ԭʼ����ִ�к��飬����ע���ע�� 
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2020/03/31     
// �� �� ֵ: NTSTATUS NTAPI     
// ��    ��: IN ULONG FilterIndex      [In]After_ZwOpenFileIndex���
// ��    ��: IN PVOID ArgArray         [In]ZwOpenFile�������׵�ַ
// ��    ��: IN NTSTATUS Result        [In]����ԭʼZwOpenFile����ֵ
// ��    ��: IN PULONG RetFuncArgArray [In]���ر�ɾ����SafeMon�б������±�
//************************************  
NTSTATUS NTAPI After_ZwOpenKey_Func(IN ULONG FilterIndex, IN PVOID ArgArray, IN NTSTATUS InResult, IN PULONG RetFuncArgArray)
{
	NTSTATUS  Status, result;
	PEPROCESS pPeprocess = NULL;
	HANDLE   Object = NULL;
	POBJECT_NAME_INFORMATION pFileNameInfo = NULL;
	ULONG NumberOfBytes = 0x1024;
	ULONG ReturnLength = NULL;
	ULONG Tag = 0x206B6444u;
	BOOLEAN ErrorFlag = TRUE;				//�ɹ�1��ʧ��0
	result = STATUS_SUCCESS;
	//0����ȡZwOpenKeyԭʼ����
	PHANDLE   In_KeyHandle = *(ULONG*)((ULONG)ArgArray);
	//1���ж��ϴε���ԭʼ��������ֵ
	if (!NT_SUCCESS(InResult))
	{
		return InResult;
	}
	//����ַ�Ϸ���
	if (myProbeRead(In_KeyHandle, sizeof(HANDLE), sizeof(CHAR)))
	{
		KdPrint(("ProbeRead(After_ZwOpenKey_Func��In_KeyHandle) error \r\n"));
		return result;
	}
	Status = ObReferenceObjectByHandle(*(ULONG*)In_KeyHandle, 0, 0, UserMode, &Object, 0);
	if (!NT_SUCCESS(Status))
	{
		return result;
	}
	pFileNameInfo = (POBJECT_NAME_INFORMATION)Safe_AllocBuff(NonPagedPool, NumberOfBytes, Tag);
	if (!pFileNameInfo)
	{
		ObfDereferenceObject(Object);
		return result;
	}
	//2��������ͨ��ע���_OBJECTָ���ѯע���·��ObQueryNameString
	Status = ObQueryNameString(Object, pFileNameInfo, NumberOfBytes, &ReturnLength);
	//������
	ObfDereferenceObject(Object);
	if (!NT_SUCCESS(Status) || !pFileNameInfo->Name.Buffer || !pFileNameInfo->Name.Length)
	{
		ExFreePool(pFileNameInfo);
		return result;
	}
	//3���жϸ���Υ��·��
	// ������
	if (wcsstr(pFileNameInfo->Name.Buffer, L"\\SHELLEXECUTEHOOKS"))
	{
		//������
		ErrorFlag = FALSE;
		result = STATUS_ACCESS_DENIED;
	}
	//����AppCertDllsע�����HKLM\System\CurrentControlSet\Control\Session Manager\AppCertDlls��д��dll��·�������Խ���ע������µ�DLL���ص�����CreateProcess��CreateProcessAsUser��CreateProcessWithLogonW��CreateProcessWithTokenW��WinExec��ÿ�������С�ֵ��ע�����win xp - win 10 Ĭ�ϲ��������ע�����
	else if (wcsstr(pFileNameInfo->Name.Buffer, L"CONTROL\\SESSION MANAGER\\APPCERTDLLS"))
	{
		 //AppCertDllsע��
		ErrorFlag = FALSE;
		result = STATUS_OBJECT_NAME_NOT_FOUND;
	}
	//������Ʋ��ԣ�ƽʱû��ע����
	else if (wcsstr(pFileNameInfo->Name.Buffer, L"\\SAFER\\CODEIDENTIFIERS"))
	{
		//������Ʋ���
		ErrorFlag = FALSE;
		result = STATUS_ACCESS_DENIED;
	}
	else
	{
		//�Ϸ�����
		ErrorFlag = TRUE;
		result = STATUS_SUCCESS;
	}
	//ʧ�ܷ���Ҫ��վ��
	if (!ErrorFlag)
	{
		Safe_ZwNtClose(*(ULONG*)In_KeyHandle, g_VersionFlag);
		*(ULONG*)In_KeyHandle = 0;
	}
	//�ͷ�nre�ռ�
	ExFreePool(pFileNameInfo);
	pFileNameInfo = NULL;
	return result;
}

//��ע����ֵ
NTSTATUS NTAPI Fake_ZwOpenKey(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS    Status, result;
	result = STATUS_SUCCESS;
	//0����ȡZwOpenKey��ԭʼ����
	ACCESS_MASK    DesiredAccess = *(ULONG*)((ULONG)ArgArray + 4);
	//1��������Ӧ�ò����
	if (ExGetPreviousMode())
	{
		//2����Ȩ����������ִ�к���
		if (KEY_READ == DesiredAccess || MAXIMUM_ALLOWED == DesiredAccess)
		{
			//�ж��ǲ��Ǳ������̣��Ƿ��أ�1  ���Ƿ���0
			if (Safe_QueryWhitePID(PsGetCurrentProcessId()))
			{
				//3���������ú���
				*(ULONG*)ret_func = After_ZwOpenKey_Func;
			}
		}
	}
	return result;
}
