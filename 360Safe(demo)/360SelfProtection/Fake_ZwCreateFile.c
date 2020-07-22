#include "Fake_ZwCreateFile.h"

//************************************     
// ��������: After_ZwCreateFile_Func     
// ����˵����ԭʼ����ִ�к��飬��������·�����ֹ�û��򿪣���������㣬������ô�򿪣�  
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2020/03/31     
// �� �� ֵ: NTSTATUS NTAPI     
// ��    ��: IN ULONG FilterIndex      [In]After_ZwOpenFileIndex���
// ��    ��: IN PVOID ArgArray         [In]ZwOpenFile�������׵�ַ
// ��    ��: IN NTSTATUS Result        [In]����ԭʼZwOpenFile����ֵ
// ��    ��: IN PULONG RetFuncArgArray [In]�뷵�صĺ���ָ���Ӧ��һ������,�ڵ���RetFuncArray�е�һ������ʱ��Ҫ�����ڱ������ж�Ӧ�Ĳ���
//************************************  
NTSTATUS NTAPI After_ZwCreateFile_Func(IN ULONG FilterIndex, IN PVOID ArgArray, IN NTSTATUS InResult, IN PULONG RetFuncArgArray)
{
	NTSTATUS       Status, result;
	ULONG          ListIndex = 0;
	HANDLE         Handle_v5 = NULL;
	SYSTEM_INFORMATIONFILE_XOR System_InformationFile = { 0 };			//�ļ���Ϣ
	result = STATUS_SUCCESS;
	//0����ȡZwCreateFileԭʼ����
	IN HANDLE             In_FileHandle = *(ULONG*)((ULONG)ArgArray);
	IN ACCESS_MASK        In_DesiredAccess = *(ULONG*)((ULONG)ArgArray + 4);
	IN POBJECT_ATTRIBUTES In_ObjectAttributes = *(ULONG*)((ULONG)ArgArray + 8);
	IN PIO_STATUS_BLOCK	  In_IoStatusBlock = *(ULONG*)((ULONG)ArgArray + 0xC);
	IN PLARGE_INTEGER	  In_AllocationSize = *(ULONG*)((ULONG)ArgArray + 0x10);
	IN ULONG			  In_FileAttributes = *(ULONG*)((ULONG)ArgArray + 0x14);
	IN ULONG			  In_ShareAccess = *(ULONG*)((ULONG)ArgArray + 0x18);
	IN ULONG			  In_CreateDisposition = *(ULONG*)((ULONG)ArgArray + 0x1C);
	//1���ж��ϴε���ԭʼ��������ֵ
	if (!NT_SUCCESS(InResult))
	{
		return InResult;
	}
	//2���ж�HookPort�汾�Ϸ���,�ҷ����ľ���0x3F1
	//if (g_HookPort_Version >= 0x3F1)
	//{
	//	if ((In_ShareAccess == FILE_SHARE_WRITE) || (In_ShareAccess == (FILE_SHARE_WRITE || FILE_SHARE_READ)) || (!In_ShareAccess) || (In_ShareAccess == (FILE_SHARE_READ | FILE_SHARE_DELETE)))
	//	{

	//	}
	//}
	//3����ȡ�ļ�������Ϣ
	Handle_v5 = *(HANDLE*)In_FileHandle;
	Status = Safe_GetInformationFile(Handle_v5, (ULONG)&System_InformationFile, UserMode);
	if (!NT_SUCCESS(Status))
	{
		return result;
	}
	//4�����Ҹ��ļ���Ϣ�Ƿ����б��У��ҵ�����1��ʧ�ܷ���0
	Status = Safe_QueryInformationFileList(System_InformationFile.IndexNumber_LowPart,
		System_InformationFile.u.IndexNumber_HighPart,
		System_InformationFile.VolumeSerialNumber);
	//�����б���
	if (Status == 0)
	{
		//�жϴ�·���ǲ���//3600��//3600safe��//3600SafeBox��
		if (!Safe_CheckProtectPath(Handle_v5, UserMode))
		{
			Handle_v5 = *(HANDLE*)In_FileHandle;
		}
		else
		{
			result = STATUS_SUCCESS;
			return result;
		}
	}
	//��������ֱ�Ӿ�����㣬��ֹ����
	Safe_ZwNtClose(Handle_v5, g_VersionFlag);
	*(HANDLE*)In_FileHandle = 0;
	result = STATUS_ACCESS_DENIED;
	return result;
}

NTSTATUS NTAPI Fake_ZwCreateFile(IN ULONG CallIndex,IN PVOID ArgArray,IN PULONG ret_func,IN PULONG ret_arg)
{
	NTSTATUS    Status, result;
	PVOID		Object = NULL;
	HANDLE      FileHandle = NULL;
	HANDLE      TempRootDirectory = NULL;
	HANDLE		Handle_v4 = NULL;
	HANDLE		Handle_v5 = NULL;
	BOOLEAN     RootDirectoryFlag = NULL;								//�ͷű�ʶ��
	SYSTEM_INFORMATIONFILE_XOR System_InformationFile_XOR = { 0 };		//�ļ���Ϣ
	OBJECT_ATTRIBUTES TempObjectAttributes = { 0 };						//��ʱ����
	result = STATUS_SUCCESS;
	//0����ȡZwCreateFileԭʼ����
	IN HANDLE             In_FileHandle = *(ULONG*)((ULONG)ArgArray);
	IN ACCESS_MASK        In_DesiredAccess = *(ULONG*)((ULONG)ArgArray + 4);
	IN POBJECT_ATTRIBUTES In_ObjectAttributes = *(ULONG*)((ULONG)ArgArray + 8);
	IN PIO_STATUS_BLOCK	  In_IoStatusBlock = *(ULONG*)((ULONG)ArgArray + 0xC);
	IN PLARGE_INTEGER	  In_AllocationSize = *(ULONG*)((ULONG)ArgArray + 0x10);
	IN ULONG			  In_FileAttributes = *(ULONG*)((ULONG)ArgArray + 0x14);
	IN ULONG			  In_ShareAccess = *(ULONG*)((ULONG)ArgArray + 0x18);
	IN ULONG			  In_CreateDisposition = *(ULONG*)((ULONG)ArgArray + 0x1C);
	//1��������Ӧ�ò����
	if (!ExGetPreviousMode())
	{
		return result;
	}
	//2����ȡ������
	Handle_v4 = PsGetCurrentProcessId();
	//2��1��ָ������wininit.exe�������������
	if (Safe_QueryWhitePID(Handle_v4) || g_dynData->SystemInformation.Wininit_ProcessId && PsGetCurrentProcessId() == (HANDLE)g_dynData->SystemInformation.Wininit_ProcessId)
	{
		return result;
	}
	//��ȡZwOpenSymbolicLinkObject������ַ��ִ��
	Status = Safe_RunZwOpenSymbolicLinkObject(
		&Handle_v5,																	//[Out]���
		1,																			//[In]Ȩ��
		In_ObjectAttributes,														//[In]
		g_VersionFlag,																//[In]�汾
		g_HookPort_Nt_Win32k_Data.SSDTTable_Data.SSDT_KeServiceTableBase,			//[In]SSDT�����ַ
		g_HookPort_Nt_Win32k_Data.SSDTTable_Data.SSDT_KeNumberOfServices			//[In]SSDT�����
		);
	if (NT_SUCCESS(Status))
	{
		Safe_ZwNtClose(Handle_v5, g_HighgVersionFlag);
		result = STATUS_SUCCESS;
		return result;
	}
	//����HookPort�汾�ж�ִ������,����0x3F1�԰汾����������,�ҷ�������0x3F1�ģ���������ִ�в�������
	//xxxxxxxxxxxxxxx
	//�Ƕ�д�ļ�
	if (In_ShareAccess != FILE_SHARE_WRITE && In_ShareAccess != (FILE_SHARE_WRITE | FILE_SHARE_READ))
	{
		//�豸���м������һ������ShareAccess Ϊ0����ʾ�������Զ�ռ���ʷ�ʽ���ļ�
		if (In_ShareAccess)
		{
			//!(*(WCHAR*)((ULONG)ArgArray + 0x1D) &0x10)��һ���ڸ��
			//ȡIn_CreateDisposition��16Ϊ�ĸ�8λ��& FILE_OVERWRITE_IF | FILE_MAXIMUM_DISPOSITION ������������������  ������
			if ((In_ShareAccess != (FILE_SHARE_DELETE | FILE_SHARE_READ)) && (In_ShareAccess != FILE_SHARE_DELETE) && !(*(WCHAR*)((ULONG)ArgArray + 0x1D) & 0x10))
			{
				//�����ڸ����������
				if (!(In_DesiredAccess & 0x520D0156) && ((In_FileAttributes & FILE_ATTRIBUTE_READONLY) || !(In_DesiredAccess & 0xFEEDFF7F)))
				{			
					result = STATUS_SUCCESS;
					return result;
				}
			}
		}
	}
	//!(*(WCHAR*)((ULONG)ArgArray + 0x1D) &0x10)��һ���ڸ��
	//ȡIn_CreateDisposition��16Ϊ�ĸ�8λ��& FILE_OVERWRITE_IF | FILE_MAXIMUM_DISPOSITION ������������������  ������
	if ((In_ShareAccess != (FILE_SHARE_DELETE | FILE_SHARE_READ)) && (In_ShareAccess != FILE_SHARE_DELETE) && !(*(WCHAR*)((ULONG)ArgArray + 0x1D) & 0x10))
	{
		*ret_func = After_ZwCreateFile_Func;
		result = STATUS_SUCCESS;
		return result;
	}
	if (myProbeRead(In_ObjectAttributes, sizeof(OBJECT_ATTRIBUTES), sizeof(CHAR)))
	{
		KdPrint(("ProbeRead(Fake_ZwCreateFile��In_ObjectAttributes) error \r\n"));
		return 0;
	}
	TempRootDirectory = In_ObjectAttributes->RootDirectory;
	if (!In_ObjectAttributes->ObjectName->Length)
	{
		if (TempRootDirectory)
		{
			//��ȡ�ļ���Ϣ
			Status = Safe_GetInformationFile(TempRootDirectory, (ULONG)&System_InformationFile_XOR, UserMode);
			//��֤�ļ���Ϣ
			if (NT_SUCCESS(Status))
			{
				//��ѯXOR�ڲ����б���
				if (Safe_QueryInformationFileList(
					System_InformationFile_XOR.IndexNumber_LowPart,
					System_InformationFile_XOR.u.IndexNumber_HighPart,
					System_InformationFile_XOR.VolumeSerialNumber))
				{
					result = STATUS_ACCESS_DENIED;
					return result;
				}
			}
		}
		result = STATUS_SUCCESS;
		return result;
	}
	//win7����Win7���ϰ汾����
	if (g_HighgVersionFlag && TempRootDirectory)
	{
		//1���õ��ļ�����ָ��
		Status = ObReferenceObjectByHandle(TempRootDirectory, NULL, NULL, UserMode, &Object, NULL);
		if (!NT_SUCCESS(Status))
		{
			TempRootDirectory = 0;
		}
		else
		{
			Status = ObOpenObjectByPointer(Object, OBJ_KERNEL_HANDLE, NULL, NULL, NULL, KernelMode, &TempRootDirectory);
			if (NT_SUCCESS(Status))
			{
				//RootDirectoryFlagΪ���ʾʹ��ObOpenObjectByPointer��ȡ�ģ������Ҫ�ͷŵ�
				RootDirectoryFlag = TRUE;
			}
			else
			{
				TempRootDirectory = 0;
			}
			//���ü����ǵ�-1
			ObfDereferenceObject(Object);
		}
	}
	InitializeObjectAttributes(
		&TempObjectAttributes,
		In_ObjectAttributes->ObjectName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		TempRootDirectory,
		NULL
		);
	Status = Safe_IoCreateFile(&TempObjectAttributes, &FileHandle);
	//RootDirectoryFlagΪ���ʾʹ��ObOpenObjectByPointer��ȡ�ģ������Ҫ�ͷŵ�
	if (RootDirectoryFlag)
	{
		ZwClose(TempRootDirectory);
	}
	if (Status != STATUS_GUARD_PAGE_VIOLATION)
	{
		if (!NT_SUCCESS(Status))
		{
			//�ж�HookPort�汾�Ϸ���,�ҷ����ľ���0x3F1,��һ���Ƕ���ģ���Ϊ�Ҳ����������汾��������
			//if ((unsigned int)g_HookPort_Version < 0x3F1)
			//	return 0;
			if (In_ShareAccess)
			{
				if (In_ShareAccess != (FILE_SHARE_DELETE | FILE_SHARE_READ))
				{
					result = STATUS_SUCCESS;
					return result;
				}
			}
			*ret_func = After_ZwCreateFile_Func;
			result = STATUS_SUCCESS;
			return result;
		}
		else
		{
			//��ȡ�ļ���Ϣ
			Status = Safe_GetInformationFile(FileHandle, (ULONG)&System_InformationFile_XOR, KernelMode);
			//��֤�ļ���Ϣ
			if (NT_SUCCESS(Status))
			{
				//��ѯXOR�ڲ����б���
				if (Safe_QueryInformationFileList(
					System_InformationFile_XOR.IndexNumber_LowPart,
					System_InformationFile_XOR.u.IndexNumber_HighPart,
					System_InformationFile_XOR.VolumeSerialNumber))
				{
					ZwClose(FileHandle);
					result = STATUS_ACCESS_DENIED;
					return result;
				}
				//��ֹ�û��򿪱���·��
				if (Safe_CheckProtectPath(FileHandle, KernelMode))
				{
					ZwClose(FileHandle);
					result = STATUS_SUCCESS;
					return result;
				}
			}
			ZwClose(FileHandle);
			result = STATUS_ACCESS_DENIED;
			return result;
		}
	}
	result = STATUS_GUARD_PAGE_VIOLATION;
	return result;
}
