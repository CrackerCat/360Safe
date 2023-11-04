/*
˵����
����KnownDLLs�ٳ�
�ο����ϣ�
1���������»���DLL�ٳ�©�����淨 
��ַ��https://www.freebuf.com/articles/78807.html
*/
#include "Fake_ZwCreateSection.h"

#define WHILEDEVICENUMBER_ZWCREATESECTION 0x7
//��ֹ���ʵİ�����
PWCHAR g_WhiteDriverName_ZwCreateSection[WHILEDEVICENUMBER_ZWCREATESECTION+1] = {
	L"\\safemon\\360Tray.exe",
	L"\\safemon\\QHSafeTray.exe",
	L"\\deepscan\\zhudongfangyu.exe",
	L"\\deepscan\\QHActiveDefense.exe",
	L"\\360SD.EXE",
	L"\\360RP.EXE",
	L"\\360RPS.EXE"
};

//************************************     
// ��������: After_ZwCreateSection_Func     
// ����˵����ԭʼ����ִ�к���,��������·�����ֹ�û��򿪣���������㣬������ô�򿪣�   
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2020/03/31     
// �� �� ֵ: NTSTATUS NTAPI     
// ��    ��: IN ULONG FilterIndex      
// ��    ��: IN PVOID ArgArray         
// ��    ��: IN NTSTATUS Result        
// ��    ��: IN PULONG RetFuncArgArray 
//************************************ 
NTSTATUS NTAPI After_ZwCreateSection_Func(IN ULONG FilterIndex, IN PVOID ArgArray, IN NTSTATUS InResult, IN PULONG RetFuncArgArray)
{
	NTSTATUS       Status, result;
	CHAR		   ObjectInformation[0x500] = { 0 };						//��������д�̶����Ȼ�������0x210u�����������
	ULONG          ReturnLength = NULL;
	SIZE_T         KnownDllsSize = 0xB;
	PPUBLIC_OBJECT_TYPE_INFORMATION	pPubObjTypeInfo = NULL;
	result = STATUS_SUCCESS;
	//0����ȡZwCreateSectionԭʼ����
	PHANDLE            In_SectionHandle = *(ULONG*)((ULONG)ArgArray);					
	ACCESS_MASK        In_DesiredAccess = *(ULONG*)((ULONG)ArgArray + 4);
	POBJECT_ATTRIBUTES In_ObjectAttributes = *(ULONG*)((ULONG)ArgArray + 8);
	PLARGE_INTEGER     In_MaximumSize = *(ULONG*)((ULONG)ArgArray + 0xC);
	ULONG              In_SectionPageProtection = *(ULONG*)((ULONG)ArgArray + 0x10);
	ULONG              In_AllocationAttributes = *(ULONG*)((ULONG)ArgArray + 0x14);
	HANDLE             In_FileHandle = *(ULONG*)((ULONG)ArgArray + 0x18);
	//1���ж��ϴε���ԭʼ��������ֵ
	if (!NT_SUCCESS(InResult))
	{
		return InResult;
	}
	//����ַ�Ϸ���
	if (myProbeRead(In_SectionHandle, sizeof(HANDLE), sizeof(CHAR)))
	{
		KdPrint(("ProbeRead(After_ZwCreateSection_Func��In_SectionHandle) error \r\n"));
		return result;
	}
	//������SafeMod����������Ľ�����Ϣ��ֱ����ӵ��б���Ȼ��ͷ��ؼ���
	if (RetFuncArgArray)
	{
		Safe_InsertSafeMonDataList(*(HANDLE*)In_SectionHandle, RetFuncArgArray);
		return result;
	}
	//���������̽�����֤��ִ�е����ﶼ��һЩ�Ǳ��������ϵͳ���̣���Щ��Ҫ���
	Status = Safe_UserMode_ZwQueryObject(g_HighgVersionFlag, *(HANDLE*)In_SectionHandle, ObjectNameInformation, ObjectInformation, sizeof(ObjectInformation), &ReturnLength);
	pPubObjTypeInfo = (PPUBLIC_OBJECT_TYPE_INFORMATION)ObjectInformation;
	if (!NT_SUCCESS(Status) ||
		!pPubObjTypeInfo->TypeName.Length 
		)
	{
		return result;
	}
	//KnownDlls���
	//��ȷ���0�����ȷ��ط�0
	if (_wcsnicmp(pPubObjTypeInfo->TypeName.Buffer, L"\\KnownDlls\\", KnownDllsSize))
	{
		//����
		return result;
	}
	//������������飨KnownDlls·����
	if (g_dynData->SystemInformation.Userinit_Flag || (!Safe_InsertSystemInformationList(IoGetCurrentProcess(), SYSTEMROOT_SYSTEM32_SMSS_EXE, g_VersionFlag)))
	{
		//����SeDeleteObjectAuditAlarm���һ�٣�������������������û������һ�����á���֪���Ŀ��Ը�������
		Safe_Run_SeDeleteObjectAuditAlarm(*(HANDLE*)In_SectionHandle);
		//����ֱ�Ӿ�����㣬��ֹ����
		Safe_ZwNtClose(*(HANDLE*)In_SectionHandle, g_VersionFlag);
		*(HANDLE*)In_SectionHandle = 0;
		//Safe_18A72_SendR3(PsGetCurrentProcessId(), PsGetCurrentThreadId(),0x9);
		result = STATUS_ACCESS_DENIED;
		return result;
	}
	return result;
}

//��
BOOLEAN NTAPI Safe_18FDE(PDEVICE_OBJECT DeviceObject)
{
	ULONG DeviceType = NULL;
	BOOLEAN Result = FALSE;
	UNICODE_STRING DestinationString;
	PFILE_OBJECT	 FileObject = NULL;
	struct _KSEMAPHORE *v5;
	PDEVICE_OBJECT   pUnknownDeviceObject = NULL;
	//1�����˵��ض��ļ��豸����
	DeviceType = DeviceObject->DeviceType;
	if (DeviceType == FILE_DEVICE_DISK ||			//�����豸
		DeviceType == FILE_DEVICE_CD_ROM ||			//CD�����豸
		DeviceType == FILE_DEVICE_TAPE_FILE_SYSTEM //�Ŵ��ļ�ϵͳ
		)
	{
		RtlInitUnicodeString(&DestinationString, L"\\Device\\MountPointManager");
		if (IoGetDeviceObjectPointer(&DestinationString, GENERIC_ALL, (PFILE_OBJECT *)&FileObject, &pUnknownDeviceObject) >= 0)
		{
			PVOID v3 = pUnknownDeviceObject->DeviceExtension;
			ULONG v4 = 0;
			if (!v3
				|| (v5 = (struct _KSEMAPHORE *)((CHAR *)v3 + 0x1C), KeReadStateSemaphore((PRKSEMAPHORE)((CHAR *)v3 + 0x1C))))
			{
				ObfDereferenceObject(FileObject);
				Result = 1;
			}
			else
			{
				if (KeWaitForSingleObject(v5, 0, 0, 0, NULL) != STATUS_TIMEOUT)
				{
					KeReleaseSemaphore(v5, 0, 1, 0);
					v4 = 1;
				}
				ObfDereferenceObject(FileObject);
				Result = v4;
			}
		}
		else
		{
			Result = TRUE;
		}
	}
	else
	{
		Result = FALSE;
	}
	return Result;
}
//���ݾ����ȡDos·��
NTSTATUS NTAPI Safe_DbgFileName(IN HANDLE Handle, OUT PUNICODE_STRING FullPathNameString, IN ULONG FullPathNameSize)
{
	UNICODE_STRING DosName;
	NTSTATUS       Status, result;
	PFILE_OBJECT   FileObject = NULL;
	PVOID          pBuff = NULL;
	ULONG		   Tag = 0x206B6444u;
	BOOLEAN		   WillFreeTargetVolumeName = TRUE;
	UNICODE_STRING TargetFileVolumeName;
	result = STATUS_SUCCESS;
	//1����File���ͼ���ִ��
	if (!Handle || !Safe_QueryObjectType(Handle, L"File"))
	{
		result = STATUS_UNSUCCESSFUL;
		return result;
	}
	//2���õ��ļ�����ָ��
	Status = ObReferenceObjectByHandle(Handle, GENERIC_READ, *IoFileObjectType, KernelMode, (PVOID*)&FileObject, NULL);
	//2��1�жϲ����Ƿ�ɹ�
	if (!NT_SUCCESS(Status) && !FileObject)
	{
		result = Status;
		return result;
	}
	//2��2 �ж��ַ���
	if (!FileObject->FileName.Buffer || !FileObject->FileName.Length)
	{
		//�ر��豸���
		ObfDereferenceObject(FileObject);
		result = STATUS_UNSUCCESSFUL;
		return result;
	}
	//3��new�ռ���������DosName��ַ
	pBuff = Safe_AllocBuff(NonPagedPool, FullPathNameSize, Tag);
	if (!pBuff)
	{
		//�ر��豸���
		ObfDereferenceObject(FileObject);
		result = STATUS_NO_MEMORY;
		return result;
	}
	////��
	//if(!Safe_18FDE(FileObject->DeviceObject))
	//{
	//	ExFreePool(pBuff);
	//	result = STATUS_FILE_LOCK_CONFLICT;
	//	ObfDereferenceObject(FileObject);
	//	return result;
	//}

	//�������ת����Dos·������
	TargetFileVolumeName.Buffer = 0;
	TargetFileVolumeName.Length = 0;
	TargetFileVolumeName.MaximumLength = 0;
	//��ΪFILE_OBJECT.FileName��·���ǲ����̷��ģ�������Ҫ�ٻ�ȡ�̷�Ȼ��ƴ������
	//4��1 ȡ���̷�DOS��
	Status = RtlVolumeDeviceToDosName(FileObject->DeviceObject, &TargetFileVolumeName);
	if (!NT_SUCCESS(Status))
	{
		RtlInitUnicodeString(&TargetFileVolumeName, L"\\");
		WillFreeTargetVolumeName = FALSE;
	}
	if (TargetFileVolumeName.Length + FileObject->FileName.Length >= 518)
	{
		ObfDereferenceObject(FileObject);
		if (WillFreeTargetVolumeName)
		{
			if (MmIsAddressValid(TargetFileVolumeName.Buffer))
				ExFreePool(TargetFileVolumeName.Buffer);
		}
		ExFreePool(pBuff);
		result = STATUS_INVALID_PARAMETER;
		return result;
	}
	//5�������ļ������̷� + FILE_OBJECT.FileName
	RtlInitUnicodeString(FullPathNameString, (PCWSTR)pBuff);
	FullPathNameString->MaximumLength = FullPathNameSize;
	Status = RtlAppendUnicodeStringToString(FullPathNameString, &TargetFileVolumeName);
	if (!NT_SUCCESS(Status))
	{
		ObfDereferenceObject(FileObject);
		if (WillFreeTargetVolumeName)
		{
			if (MmIsAddressValid(TargetFileVolumeName.Buffer))
				ExFreePool(TargetFileVolumeName.Buffer);
		}
		ExFreePool(pBuff);
		result = STATUS_INVALID_PARAMETER;
		return result;
	}
	Status = RtlAppendUnicodeStringToString(FullPathNameString, &FileObject->FileName);
	if (!NT_SUCCESS(Status))
	{
		ObfDereferenceObject(FileObject);
		if (WillFreeTargetVolumeName)
		{
			if (MmIsAddressValid(TargetFileVolumeName.Buffer))
				ExFreePool(TargetFileVolumeName.Buffer);
		}
		ExFreePool(pBuff);
		result = STATUS_INVALID_PARAMETER;
		return result;
	}
	ObfDereferenceObject(FileObject);
	if (WillFreeTargetVolumeName && MmIsAddressValid(TargetFileVolumeName.Buffer))
	{
		ExFreePool(TargetFileVolumeName.Buffer);
	}
	result = STATUS_SUCCESS;
	return result;
}


NTSTATUS NTAPI Fake_ZwCreateSection(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS       Status, result;
	UNICODE_STRING FullPathNameString;													//FullPathNameString.buff��������new�����ģ���Ҫ�ͷ�
	ULONG          FullPathNameSize = 0x1024;											//����·����󳤶�
	ULONG		   SEC_IMAGE = 0x1000000;
	SYSTEM_INFORMATIONFILE_XOR System_InformationFile = { 0 };							//�ļ���Ϣ
	result = STATUS_SUCCESS;
	//0����ȡZwCreateSectionԭʼ����
	PHANDLE            In_SectionHandle = *(ULONG*)((ULONG)ArgArray);					//������ܷ��ص�section�ľ������ʹ����ļ�������һ����·
	ACCESS_MASK        In_DesiredAccess = *(ULONG*)((ULONG)ArgArray + 4);
	POBJECT_ATTRIBUTES In_ObjectAttributes = *(ULONG*)((ULONG)ArgArray + 8);
	PLARGE_INTEGER     In_MaximumSize = *(ULONG*)((ULONG)ArgArray + 0xC);
	ULONG              In_SectionPageProtection = *(ULONG*)((ULONG)ArgArray + 0x10);
	ULONG              In_AllocationAttributes = *(ULONG*)((ULONG)ArgArray + 0x14);
	HANDLE             In_FileHandle = *(ULONG*)((ULONG)ArgArray + 0x18);
	//1��������Ӧ�ò����
	if (ExGetPreviousMode())
	{
		//��ʼ������
		*(ULONG*)ret_arg = 0;							//���������ܱ������̣��ͷ��ظ��ܱ������̵���Ϣ�����е��ú���
		if (In_DesiredAccess == SECTION_ALL_ACCESS
			&& In_SectionPageProtection == PAGE_EXECUTE
			&& In_AllocationAttributes == SEC_IMAGE
			&& !g_Win2K_XP_2003_Flag
			)
		{
			//2�����ݾ����ȡDos·��
			Status = Safe_DbgFileName(In_FileHandle, &FullPathNameString, FullPathNameSize);
			if (!NT_SUCCESS(Status))
			{
				if (//Status == STATUS_FILE_LOCK_CONFLICT
					NT_SUCCESS(Safe_GetInformationFile(In_FileHandle, &System_InformationFile, UserMode))
					)
				{

					//  ��
					// ��ֹ���ܱ�������
					//	Safe_CheckCreateProcessCreationFlags();
				}
			}
			else
			{
				//3�����Ҹ�dos·�����б�ڼ��ret_arg = ���������±꣬ 
				if (Safe_QuerSafeMonPathList(FullPathNameString.Buffer, &ret_arg)
					//Safe_CheckProcessNameSign(FullPathNameString);				�������PE����ǩ��֮���
					)
				{
					Safe_CheckCreateProcessCreationFlags();
				}
				ExFreePool(FullPathNameString.Buffer);
			}
			if (ret_arg)
			{
				*ret_func = After_ZwCreateSection_Func;
				return result;
			}
		}
		if (In_ObjectAttributes)
		{
			*ret_func = After_ZwCreateSection_Func;
		}
	}
	return result;
}