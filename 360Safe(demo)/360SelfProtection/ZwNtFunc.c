/*
˵�������ݰ汾����ʹ��Zw����Nt�Ⱥ���
*/

#include "ZwNtFunc.h"

//************************************     
// ��������: Safe_ZwIoDuplicateObject     
// ����˵�����˸�溯����ֻ֧��TargetProcessHandle == NtCurrentProcess()�ĵ���    
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��   
// �� �� ֵ: NTSTATUS NTAPI     
// ��    ��: IN HANDLE SourceProcessHandle     [In]ԭʼ��������
// ��    ��: IN HANDLE SourceHandle            [In]ԭʼ��������
// ��    ��: IN HANDLE TargetProcessHandle     [In]NtCurrentProcess()
// ��    ��: IN PHANDLE TargetHandle           [Out]����ֵ
// ��    ��: IN ACCESS_MASK DesiredAccess      [In]NULL
// ��    ��: IN ULONG HandleAttributes         [In]NULL
// ��    ��: IN ULONG Options                  [In]DUPLICATE_SAME_ACCESS
// ��    ��: IN BOOLEAN HighVersion_Flag       [In]�߰汾��ʶ��Win7��win7���ϰ汾��1
// ��    ��: IN ULONG Version_Flag             [In]�汾���
//************************************  
NTSTATUS NTAPI Safe_ZwIoDuplicateObject(IN HANDLE  SourceProcessHandle, IN HANDLE  SourceHandle, IN HANDLE  TargetProcessHandle, IN PHANDLE  TargetHandle, IN ACCESS_MASK  DesiredAccess, IN ULONG  HandleAttributes, IN ULONG  Options, IN BOOLEAN HighVersion_Flag, IN ULONG Version_Flag)
{
	NTSTATUS       Status = STATUS_SUCCESS;
	NTSTATUS       Result = STATUS_SUCCESS;
	HANDLE         HandleToProcess = NULL;
	PEPROCESS      SourceProcess = NULL;
	//���ݰ汾�жϵ���Io����zw
	//Win7��1
	if (HighVersion_Flag)
	{
		if (TargetProcessHandle == NtCurrentProcess())
		{
			//��ȡSourceProcessHandle��Eprocess
			Status = ObReferenceObjectByHandle(SourceProcessHandle,
				PROCESS_DUP_HANDLE,
				PsProcessType,
				UserMode,
				&SourceProcess,
				NULL);
			if (NT_SUCCESS(Status))
			{
				//Win7�汾
				if (Version_Flag == WINDOWS_VERSION_7)
				{
					Status = ObOpenObjectByPointer(SourceProcess, OBJ_KERNEL_HANDLE, NULL, PROCESS_DUP_HANDLE, PsProcessType, KernelMode, &HandleToProcess);
					if (NT_SUCCESS(Status))
					{
						Status = ZwDuplicateObject(
							HandleToProcess,
							SourceHandle,
							NtCurrentProcess(),
							TargetHandle,
							DesiredAccess,
							HandleAttributes,
							Options);
						ZwClose(HandleToProcess);
					}
				}
				else if (g_dynData->pObDuplicateObject)
				{
					Status = g_dynData->pObDuplicateObject(SourceProcess, SourceHandle, IoGetCurrentProcess(), TargetHandle, DesiredAccess, HandleAttributes, Options, KernelMode);
				}
				else
				{
					Status = STATUS_NOT_SUPPORTED;
				}
				ObfDereferenceObject(SourceProcess);
			}
		}
		else
		{
			Status = STATUS_NOT_SUPPORTED;
		}
		Result = Status;
	}
	else
	{
		Status = ZwDuplicateObject(
			SourceProcessHandle,
			SourceHandle,
			TargetProcessHandle,
			TargetHandle,
			DesiredAccess,
			HandleAttributes,
			Options);
		Result = Status;
	}
	return Result;
}

NTSTATUS NTAPI Safe_IoCreateFile(_In_ POBJECT_ATTRIBUTES ObjectAttributes, _Out_ PHANDLE FileHandle)
{
	IO_STATUS_BLOCK IoStatusBlock = { 0 };
	NTSTATUS Status;
	Status = STATUS_UNSUCCESSFUL;
	Status = IoCreateFile(FileHandle,
		GENERIC_READ | SYNCHRONIZE,
		ObjectAttributes,
		&IoStatusBlock,
		0,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		FILE_OPEN,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
		0,
		0,
		CreateFileTypeNone,
		0,
		IO_NO_PARAMETER_CHECKING
		);
	return Status;
}
//�ͷž��
NTSTATUS NTAPI Safe_ZwNtClose(IN HANDLE Handle, IN BOOLEAN HighVersion_Flag)
{
	NTSTATUS Status;
	Status = STATUS_UNSUCCESSFUL;
	//Win7��1
	if (HighVersion_Flag)
	{
		Status = NtClose(Handle);
	}
	else
	{
		Status = ZwClose(Handle);
	}
	return Status;
}

NTSTATUS NTAPI Safe_UserMode_ZwQueryVolumeInformationFile(_In_ HANDLE FileHandle, _Out_ PIO_STATUS_BLOCK IoStatusBlock, _Out_writes_bytes_(Length) PVOID FsInformation, _In_ ULONG Length, _In_ FS_INFORMATION_CLASS FsInformationClass, IN BOOLEAN HighVersion_Flag)
{
	NTSTATUS     Status;
	PFILE_OBJECT FileObject = NULL;
	//Safe_Initialize_Data
	//ִ����Ϻ��Ұ汾>=Win7��1
	if (HighVersion_Flag)
	{
		//1���õ��ļ�����ָ��
		Status = ObReferenceObjectByHandle(FileHandle, FILE_ANY_ACCESS, *IoFileObjectType, UserMode, (PVOID*)&FileObject, NULL);
		//1��1�жϲ����Ƿ�ɹ�
		if (NT_SUCCESS(Status))
		{
			Status = IoQueryVolumeInformation(FileObject, FsInformationClass, Length, FsInformation, &IoStatusBlock);
			ObfDereferenceObject(FileObject);
		}
	}
	else
	{
		Status = ZwQueryVolumeInformationFile(FileHandle, &IoStatusBlock, FsInformation, Length, FsInformationClass);
	}
	return Status;
}


//��AccessMode == 1��UserModeģʽ
NTSTATUS NTAPI Safe_UserMode_ZwQueryInformationFile(_In_ HANDLE FileHandle, _Out_ PIO_STATUS_BLOCK IoStatusBlock, _Out_writes_bytes_(Length) PVOID FileInformation, _In_ ULONG Length, _In_ FILE_INFORMATION_CLASS FileInformationClass, IN BOOLEAN HighVersion_Flag)
{
	NTSTATUS     Status;
	PFILE_OBJECT FileObject = NULL;
	PFILE_POSITION_INFORMATION pFilePositionInformation = NULL;
	//Safe_Initialize_Data
	//ִ����Ϻ��Ұ汾>=Win7��1
	if (HighVersion_Flag)
	{
		//1���õ��ļ�����ָ��
		Status = ObReferenceObjectByHandle(FileHandle, FILE_ANY_ACCESS, *IoFileObjectType, UserMode, (PVOID*)&FileObject, NULL);
		//1��1�жϲ����Ƿ�ɹ�
		if (NT_SUCCESS(Status))
		{
			//�ļ�λ����Ϣ
			if (FilePositionInformation == FileInformationClass)
			{
				if (Length >= 8)
				{
					pFilePositionInformation = FileInformation;
					pFilePositionInformation->CurrentByteOffset = FileObject->CurrentByteOffset;
					Status = 0;
				}
				else
				{
					Status = STATUS_INFO_LENGTH_MISMATCH;
				}
			}
			else
			{
				Status = IoQueryFileInformation(FileObject, FileInformationClass, Length, FileInformation, &IoStatusBlock);
			}
			ObfDereferenceObject(FileObject);
		}
	}
	else
	{
		Status = ZwQueryInformationFile(FileHandle, &IoStatusBlock, FileInformation, Length, FileInformationClass);
	}
	return Status;
}

//��AccessMode == 1��UserModeģʽ
NTSTATUS NTAPI Safe_UserMode_ZwQueryObject(_In_ BOOLEAN HighVersion_Flag, _In_ HANDLE ObjectHandle, _In_ OBJECT_INFORMATION_CLASS ObjectInformationClass, _Out_opt_  PVOID ObjectInformation, _In_ ULONG ObjectInformationLength, _Out_opt_  PULONG ReturnLength)
{
	NTSTATUS Status = NULL;
	ULONG ulRet = NULL;
	//win7����Win7���ϰ汾����
	if (HighVersion_Flag)
	{
		Status = ObReferenceObjectByHandle(ObjectHandle, 0, 0, UserMode, &ObjectHandle, 0);
		if (NT_SUCCESS(Status))
		{
			if (ObjectInformationClass == 1)
			{
				Status = ObQueryNameString(ObjectHandle, ObjectInformation, ObjectInformationLength, &ulRet);
				ObfDereferenceObject(ObjectHandle);
				if (ReturnLength)
				{
					*ReturnLength = (ULONG)ulRet;
				}
			}
			else
			{
				Status = STATUS_NOT_IMPLEMENTED;
			}
		}
	}
	else
	{
		Status = ZwQueryObject(
			ObjectHandle,
			ObjectInformationClass,
			ObjectInformation,
			ObjectInformationLength,
			ReturnLength);
	}
	return Status;
}