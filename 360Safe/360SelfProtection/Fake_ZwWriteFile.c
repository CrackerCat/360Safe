#include "Fake_ZwWriteFile.h"


//NTSTATUS NTAPI After_ZwWriteFile(IN HANDLE  FileHandle, IN HANDLE  Event  OPTIONAL, IN PIO_APC_ROUTINE  ApcRoutine  OPTIONAL, IN PVOID  ApcContext  OPTIONAL, OUT PIO_STATUS_BLOCK  IoStatusBlock, IN PVOID  Buffer, IN ULONG  Length, IN PLARGE_INTEGER  ByteOffset  OPTIONAL, IN PULONG  Key  OPTIONAL)
NTSTATUS NTAPI Fake_ZwWriteFile(ULONG CallIndex, PVOID ArgArray, PULONG ret_func, PULONG ret_arg)
{
	NTSTATUS       Status,result;
	PFILE_OBJECT   FileObject = NULL;
	PDRIVER_OBJECT pDeviceObject = NULL;
	ULONG		   DeviceType = 0;
	PDEVICE_OBJECT DeviceObject1 = NULL;
	PDEVICE_OBJECT pDiskDeviceObject = NULL;
	ULONG          Tag = 0x206B6444;
	PQUERY_PASS_R0SENDR3_DATA  pQuery_Pass = NULL;
	ULONG		   Flag = NULL;
	result = STATUS_SUCCESS;
	//��ZwWriteFile���������
	IN HANDLE  In_FileHandle = *(ULONG*)((ULONG)ArgArray);
	IN PVOID   In_ApcContext = *(ULONG*)((ULONG)ArgArray+0xC);
	//1��������Ӧ�ò����
	if (!ExGetPreviousMode())
	{
		return result;
	}
	//���������Ŀǰֻ����360Tray.exe����+1
	if (!g_SpecialWhite_List.SpecialWhiteListNumber)
	{
		return result;
	}
	//1���õ��ļ�����ָ��
	Status = ObReferenceObjectByHandle(In_FileHandle, FILE_ANY_ACCESS, *IoFileObjectType, UserMode, (PVOID*)&FileObject, NULL);
	//1��1�жϲ����Ƿ�ɹ�
	if (!NT_SUCCESS(Status))
	{
		return result;
	}
	//2 �ж��豸�������������ļ��豸���͵�
	pDeviceObject = FileObject->DeviceObject;
	if ((!FileObject->DeviceObject) || (!FileObject->DeviceObject->DriverObject) || (FileObject->DeviceObject->DeviceType != FILE_DEVICE_DISK))
	{
		//�ر��豸���
		ObfDereferenceObject(FileObject);
		return result;
	}
	//3��ͨ���豸����ļ�����ָ��õ��豸����ָ��
	DeviceObject1 = IoGetBaseFileSystemDeviceObject(FileObject);
	//4����ȡ���̶���
	Status = Safe_IoGetDiskDeviceObjectPrt(DeviceObject1, &pDiskDeviceObject);
	if (DeviceObject1 && NT_SUCCESS(Status))
	{
		ObfDereferenceObject(pDiskDeviceObject);
		if (FileObject->FileName.Buffer || FileObject->FileName.Length)
		{
			ObfDereferenceObject(FileObject);
			return result;
		}
	}
	//5����ȡ���̶���ʧ�ܣ�����������
	//�ر��豸���
	ObfDereferenceObject(FileObject);
	if (PsGetCurrentProcessId() != g_Thread_Information.CurrentProcessId_0) 
	{
		if (PsGetCurrentProcessId() != g_Thread_Information.CurrentProcessId_1)
		{
			
			pQuery_Pass = (PQUERY_PASS_R0SENDR3_DATA)Safe_AllocBuff(NonPagedPool, sizeof(QUERY_PASS_R0SENDR3_DATA), Tag);
			if (!pQuery_Pass)
			{
				return result;
			}
			pQuery_Pass->Unknown_Flag_2 = 3;
			pQuery_Pass->CheckWhitePID = PsGetCurrentProcessId();								//���������ж��Ƿ��ǰ���������
			pQuery_Pass->Unknown_CurrentThreadId_4 = PsGetCurrentThreadId();
			pQuery_Pass->Unknown_CurrentThreadId_5 = PsGetCurrentThreadId();
			pQuery_Pass->Unknown_Flag_6 = 1;
			pQuery_Pass->Unknown_Flag_8A = 0x200;
			pQuery_Pass->Unknown_Flag_8B = 0;
			pQuery_Pass->ApcContext = In_ApcContext;
			//R3��R0ͨѶ����ûд
			//Flag = Safe_push_request_in_and_waitfor_finish(pQuery_Pass, 1);
			//���͵�R3Ȼ����ε�ַ�ռ��ͷ�
			if (pQuery_Pass)
			{
				ExFreePool(pQuery_Pass);
				pQuery_Pass = NULL;
			}
			switch (Flag)
			{
				case 0:						    //�������̣�����0
				{
					g_Thread_Information.CurrentProcessId_0 = (ULONG)PsGetCurrentProcessId();
					result = STATUS_SUCCESS;
					break;
				}
				case 2:							//���󷵻أ�����2
				{
					result = STATUS_ACCESS_DENIED;
					break;
				}
				case 3:							 //�������̣�����3
				{
					result = STATUS_SUCCESS;
					break;
				}
				default:						//һ�㲻��ִ�е�����
				{
					g_Thread_Information.CurrentProcessId_1 = (ULONG)PsGetCurrentProcessId();
					result = STATUS_ACCESS_DENIED;
					break;
				}
			}
		}
		else
		{
			result = STATUS_ACCESS_DENIED;
			return result;
		}
	}
	return result;
}

//��ȡIoGetDiskDeviceObject������ַ�������øú���
NTSTATUS NTAPI Safe_IoGetDiskDeviceObjectPrt(PDEVICE_OBJECT FileSystemDeviceObject,PDEVICE_OBJECT* DiskDeviceObject)
{
	NTSTATUS       Status;
	UNICODE_STRING IoGetDiskDeviceObjectString;
	NTSTATUS (*IoGetDiskDeviceObjectPtr)(PDEVICE_OBJECT FileSystemDeviceObject, PDEVICE_OBJECT *DiskDeviceObject);
	//1���ж��ǲ��ǵ�һ�ν�ȥ���������MmGetSystemRoutineAddress��ʽ��ȡIoGetDiskDeviceObject������ַ����������
	IoGetDiskDeviceObjectPtr = g_Thread_Information.pIoGetDiskDeviceObjectPtr;
	if (!IoGetDiskDeviceObjectPtr)
	{
		RtlInitUnicodeString(&IoGetDiskDeviceObjectString, L"IoGetDiskDeviceObject");
		g_Thread_Information.pIoGetDiskDeviceObjectPtr = (ULONG)MmGetSystemRoutineAddress(&IoGetDiskDeviceObjectString);
		IoGetDiskDeviceObjectPtr = g_Thread_Information.pIoGetDiskDeviceObjectPtr;
		//1��1 ���ǻ�ȡʧ��ֱ�ӷ���
		if (!IoGetDiskDeviceObjectPtr)
		{
			Status = STATUS_UNSUCCESSFUL;
			return Status;
		}
	}
	//2������IoGetDiskDeviceObject����
	Status = IoGetDiskDeviceObjectPtr(FileSystemDeviceObject, DiskDeviceObject);
	return Status;
}
