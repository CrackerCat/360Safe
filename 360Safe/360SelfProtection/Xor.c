#include "Xor.h"

NTSTATUS NTAPI Safe_KernelCreateFile(IN PANSI_STRING SymbolName, OUT PSYSTEM_INFORMATIONFILE_XOR System_Information)
{
	HANDLE          hFile = NULL;
	NTSTATUS        Status = STATUS_UNSUCCESSFUL;
	IO_STATUS_BLOCK StatusBlock = { 0 };
	ULONG           ulShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
	ULONG           ulCreateOpt = FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE;
	// 1. ��ʼ��OBJECT_ATTRIBUTES������
	OBJECT_ATTRIBUTES objAttrib = { 0 };
	ULONG             ulAttributes = OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE;
	InitializeObjectAttributes(
		&objAttrib,		 // ���س�ʼ����ϵĽṹ��
		SymbolName,      // �ļ���������
		ulAttributes,   // ��������
		NULL,           // ��Ŀ¼(һ��ΪNULL)
		NULL);          // ��ȫ����(һ��ΪNULL)
	//2�������ļ�����,��ZwCreateFile���ӵײ�
	Status = IoCreateFile(
		&hFile,							// �����ļ����
		FILE_READ_ATTRIBUTES,			// �ļ���������
		&objAttrib,						// OBJECT_ATTRIBUTES
		&StatusBlock,					// ���ܺ����Ĳ������
		0,								// ��ʼ�ļ���С
		FILE_ATTRIBUTE_NORMAL,			// �½��ļ�������
		ulShareAccess,				    // �ļ�����ʽ
		FILE_OPEN,						// ���ļ�
		ulCreateOpt,					// �򿪲����ĸ��ӱ�־λ
		NULL,							// ��չ������
		NULL,							// ��չ����������
		CreateFileTypeNone,				// ������CreateFileTypeNone
		NULL,							// InternalParameters
		IO_NO_PARAMETER_CHECKING		// Options
		);
	//����ʧ�ܵ���ZwCreateFile
	if (!NT_SUCCESS(Status))
	{
		Status = ZwCreateFile(
			&hFile,                // �����ļ����
			GENERIC_ALL,           // �ļ���������
			&objAttrib,            // OBJECT_ATTRIBUTES
			&StatusBlock,          // ���ܺ����Ĳ������
			0,                     // ��ʼ�ļ���С
			FILE_ATTRIBUTE_NORMAL, // �½��ļ�������
			ulShareAccess,         // �ļ�����ʽ
			FILE_OPEN_IF,          // �ļ�������򿪲������򴴽�
			ulCreateOpt,           // �򿪲����ĸ��ӱ�־λ
			NULL,                  // ��չ������
			0);                    // ��չ����������
		if (!NT_SUCCESS(Status))
		{
			//ʧ�ܷ���
			return Status;
		}
	}
	//3�����ļ��ɹ���ѯ�ļ���Ϣ
	Status = Safe_GetInformationFile(hFile, System_Information, KernelMode);
	//4���ͷž��
	ZwClose(hFile);
	return Status;
}


//************************************     
// ��������: Safe_GetInformationFile     
// ����˵������ȡ����Ϣ���ļ���Ϣ    
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ�� 
// �� �� ֵ: NTSTATUS NTAPI     
// ��    ��: IN HANDLE Handle                                      [In]Ŀ¼���
// ��    ��: OUT PSYSTEM_INFORMATIONFILE_XOR System_Information    [Out]����ļ���Ϣ
// ��    ��: IN KPROCESSOR_MODE AccessMode                         [In]�û���or�ں˲�
//************************************  
NTSTATUS NTAPI Safe_GetInformationFile(IN HANDLE Handle, OUT PSYSTEM_INFORMATIONFILE_XOR System_Information, IN KPROCESSOR_MODE AccessMode)
{
	NTSTATUS        Status = STATUS_UNSUCCESSFUL;
	IO_STATUS_BLOCK StatusBlock = { 0 };
	PFILE_OBJECT    FileObject = NULL;
	ULONG			DeviceType = 0;
	ULONG           FastfatFlag = 0;
	FILE_FS_VOLUME_INFORMATION FsInformation = { 0 };
	FILE_INTERNAL_INFORMATION  FileInformation = { 0 };
	FILE_BASIC_INFORMATION	   FileBaInformation = { 0 };
	struct _DRIVER_OBJECT *DriverObject;
	//1���жϾ���ĺϷ���4�ı���
	if (((ULONG)Handle & 3) == 3 || !Handle)// �жϾ���Ϸ���
	{
		return Status;
	}
	//2���õ��ļ�����ָ��
	Status = ObReferenceObjectByHandle(Handle, FILE_ANY_ACCESS, *IoFileObjectType, AccessMode, (PVOID*)&FileObject, NULL);
	//2��1�жϲ����Ƿ�ɹ�
	if (!NT_SUCCESS(Status) && !FileObject)
	{
		return Status;
	}
	//2��2 �ж��豸����
	if (!FileObject->DeviceObject)
	{
		//�ر��豸���
		ObfDereferenceObject(FileObject);
		Status = STATUS_UNSUCCESSFUL;
		return Status;
	}
	//3�����˵��ض��ļ��豸����
	DeviceType = FileObject->DeviceObject->DeviceType;
	if (DeviceType != FILE_DEVICE_DISK_FILE_SYSTEM    &&   //�����ļ�ϵͳ�豸
		DeviceType != FILE_DEVICE_DISK			      &&   //�����豸
		DeviceType != FILE_DEVICE_FILE_SYSTEM	      &&   //�ļ�ϵͳ�豸
		DeviceType != FILE_DEVICE_UNKNOWN		      &&   //δ֪����
		DeviceType != FILE_DEVICE_CD_ROM		      &&   //CD�����豸
		DeviceType != FILE_DEVICE_CD_ROM_FILE_SYSTEM  &&   //CD�����ļ�ϵͳ�豸
		DeviceType != FILE_DEVICE_NETWORK_FILE_SYSTEM      //�����ļ�ϵͳ�豸
		)
	{
		if (DeviceType != FILE_DEVICE_NETWORK_REDIRECTOR)  //�����豸
		{
			//�ر��豸���
			ObfDereferenceObject(FileObject);
			Status = STATUS_UNSUCCESSFUL;
			return Status;
		}
	}
	if (DeviceType == FILE_DEVICE_MULTI_UNC_PROVIDER)	   //��UNC�豸
	{
		if (!FileObject->FileName.Buffer || !FileObject->FileName.Length)
		{
			//�ر��豸���
			ObfDereferenceObject(FileObject);
			Status = STATUS_UNSUCCESSFUL;
			return Status;
		}
	}
	//�ж�DriverName
	DriverObject = FileObject->DeviceObject->DriverObject;
	if (DriverObject)
	{
		//�ļ�ϵͳ
		if (_wcsnicmp(DriverObject->DriverName.Buffer, L"\\Driver\\Fastfat", 0xF) == 0)
		{
			FastfatFlag = 1;
		}
	}
	//�ر��豸���
	ObfDereferenceObject(FileObject);
	//4������KernelMode or UserMode�ж�ʹ���ĸ�����
	//��ѯ�����Ϣ
	//AccessMode == 1ִ��Safe_UserModexxx,����ZwQueryVolumeInformationFile
	Status = AccessMode ? Safe_UserMode_ZwQueryVolumeInformationFile(Handle, &StatusBlock, (PVOID)&FsInformation, sizeof(FILE_FS_VOLUME_INFORMATION), FileFsVolumeInformation, g_HighgVersionFlag) : ZwQueryVolumeInformationFile(Handle, &StatusBlock, (PVOID)&FsInformation, sizeof(FILE_FS_VOLUME_INFORMATION), FileFsVolumeInformation);
	if (NT_SUCCESS(Status))
	{
		//AccessMode == 1ִ��Safe_UserModexxx,����ZwQueryInformationFile
		//��ȡ���ļ�ΨһID
		Status = AccessMode ? Safe_UserMode_ZwQueryInformationFile(Handle, &StatusBlock, (PVOID)&FileInformation, sizeof(FILE_INTERNAL_INFORMATION), FileInternalInformation, g_HighgVersionFlag) : ZwQueryInformationFile(Handle, &StatusBlock, (PVOID)&FileInformation, sizeof(FILE_INTERNAL_INFORMATION), FileInternalInformation, g_HighgVersionFlag);
		if (NT_SUCCESS(Status))
		{
			if ((FileInformation.IndexNumber.HighPart) || (FileInformation.IndexNumber.HighPart == FastfatFlag))
			{
				System_Information->u.IndexNumber_HighPart = FileInformation.IndexNumber.HighPart;	//����ý���Ψһ��ʶID
				System_Information->IndexNumber_LowPart = FileInformation.IndexNumber.LowPart;	    //����ý���Ψһ��ʶID
				System_Information->VolumeSerialNumber = FsInformation.VolumeSerialNumber;			//�������к����
			}
			else
			{
				//AccessMode == 1ִ��Safe_UserModexxx,����ZwQueryInformationFile
				Status = AccessMode ? Safe_UserMode_ZwQueryInformationFile(Handle, &StatusBlock, (PVOID)&FileBaInformation, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation, g_HighgVersionFlag) : ZwQueryInformationFile(Handle, &StatusBlock, (PVOID)&FileBaInformation, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation, g_HighgVersionFlag);
				if (NT_SUCCESS(Status))
				{
					System_Information->u.XorResult = FileBaInformation.CreationTime.LowPart ^ FileBaInformation.ChangeTime.HighPart;		//��������֭����
					System_Information->IndexNumber_LowPart = FileInformation.IndexNumber.LowPart;	//����ý���Ψһ��ʶID
					System_Information->VolumeSerialNumber = FsInformation.VolumeSerialNumber;		//�������к����
					return STATUS_SUCCESS;
				}
			}
		}
	}
	return Status;

}