#include "Fake_ZwOpenFile.h"

//************************************     
// ��������: After_ZwOpenFile_Func     
// ����˵����ԭʼ����ִ�к���,��������·�����ֹ�û��򿪣���������㣬������ô�򿪣�   
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
NTSTATUS NTAPI After_ZwOpenFile_Func(IN ULONG FilterIndex, IN PVOID ArgArray, IN NTSTATUS InResult, IN PULONG RetFuncArgArray)
{
	NTSTATUS       Status, result;
	ULONG          ListIndex = 0;
	HANDLE         Handle_v5 = NULL;
	SYSTEM_INFORMATIONFILE_XOR System_InformationFile = { 0 };			//�ļ���Ϣ
	result = STATUS_SUCCESS;
	//0����ȡZwOpenFileԭʼ����
	PHANDLE In_FileHandle = *(ULONG*)((ULONG)ArgArray);
	ACCESS_MASK In_DesiredAccess = *(ULONG*)((ULONG)ArgArray + 4);
	POBJECT_ATTRIBUTES In_ObjectAttributes = *(ULONG*)((ULONG)ArgArray + 8);
	PIO_STATUS_BLOCK In_IoStatusBlock = *(ULONG*)((ULONG)ArgArray + 0xC);
	ULONG In_ShareAccess = *(ULONG*)((ULONG)ArgArray + 0x10);
	ULONG In_OpenOptions = *(ULONG*)((ULONG)ArgArray + 0x14);
	//KdPrint(("After_ZwOpenFile_Func\t\n"));
	//1���ж��ϴε���ԭʼ��������ֵ
	if (!NT_SUCCESS(InResult))
	{
		return InResult;
	}
	//����ַ�Ϸ���
	if (myProbeRead(In_FileHandle, sizeof(HANDLE), sizeof(CHAR)))
	{
		KdPrint(("ProbeRead(After_ZwOpenFile_Func��In_FileHandle) error \r\n"));
		return result;
	}
	//2����ȡ�ļ�������Ϣ
	Handle_v5 = *(HANDLE*)In_FileHandle;
	Status = Safe_GetInformationFile(Handle_v5, (ULONG)&System_InformationFile, UserMode);
	if (!NT_SUCCESS(Status))
	{
		return result;
	}
	//3�����Ҹ��ļ���Ϣ�Ƿ����б��У��ҵ�����1��ʧ�ܷ���0
	Status = Safe_QueryInformationFileList(System_InformationFile.IndexNumber_LowPart,
		System_InformationFile.u.IndexNumber_HighPart,
		System_InformationFile.VolumeSerialNumber);
	//�����б���
	if (Status == 0)
	{
		//�жϴ�·���ǲ���//360��//360safe��//360SafeBox��
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
	Safe_ZwNtClose(Handle_v5,g_VersionFlag);
	*(HANDLE*)In_FileHandle = 0;
	result = STATUS_ACCESS_DENIED;
	return result;
}

//�Ƚ�OpenFile�ļ��������� == \\Device\\LanmanServer
NTSTATUS NTAPI Safe_CmpLanmanServer(POBJECT_ATTRIBUTES ObjectAttributes)
{
	NTSTATUS       Status, result;
	PUNICODE_STRING ObjectName;
	result = STATUS_SUCCESS;

	UNICODE_STRING LanmanServerString;
	RtlInitUnicodeString(&LanmanServerString, L"\\Device\\LanmanServer");
	if (((ULONG)ObjectAttributes <= MmUserProbeAddress) &&		//�Ƿ�С��ϵͳ��ַ
		(!ObjectAttributes->RootDirectory) &&					//��ѡ�Ĵ���ĸ�����Ŀ¼·����ָ���ĳ�Ա��ObjectName�����RootDirectory��NULL, ObjectName����ָ��һ����ȫ�޶���������,����Ŀ����������·��������ǿ�RootDirectory, ObjectNameָ��һ���������������RootDirectoryĿ¼��RootDirectory�����������һ���ļ�ϵͳĿ¼��Ŀ¼����������������ƿռ䡣
		(ObjectAttributes->Attributes == OBJ_CASE_INSENSITIVE)  //�����ִ�Сд�Ƚ�
		)
	{
		//�ж�OpenFile�ļ��������� == \\Device\\LanmanServer
		ObjectName = ObjectAttributes->ObjectName;
		if (((ULONG)ObjectName <= MmUserProbeAddress) && ObjectName->Length == LanmanServerString.Length)
		{
			if (ObjectName->Buffer <= MmUserProbeAddress)
			{
				Status = RtlEqualUnicodeString(ObjectName, &LanmanServerString, TRUE);
				if (Status)
				{
					//���ø����ط�ֹ�ظ�����
					Status = InterlockedCompareExchange(&g_HookSrvTransactionNotImplementedFlag, 1, 0);
					//��һ�ν���
					if (!Status)
					{
						//����֮�����Ǹ�©��
						//����֮��©��(CVE - 2017 - 0144), �滻srv!SrvTransaction2DispatchTable��0x0e
						result = Safe_HookSrvTransactionNotImplemented();
					}
				}
			}
		}
	}
	return result;
}

NTSTATUS NTAPI Fake_ZwOpenFile(ULONG CallIndex, PVOID ArgArray, PULONG ret_func, PULONG ret_arg)
{

	NTSTATUS    Status = STATUS_SUCCESS;
	NTSTATUS	result = STATUS_SUCCESS;
	PVOID		Object = NULL;
	HANDLE      FileHandle = NULL;
	HANDLE      TempRootDirectory = NULL;
	HANDLE		Handle_v4 = NULL;
	HANDLE		Handle_v5 = NULL;										//ZwOpenSymbolicLinkObject�ľ��
	BOOLEAN     RootDirectoryFlag = FALSE;								//�ͷű�ʶ��
	SYSTEM_INFORMATIONFILE_XOR System_InformationFile_XOR = { 0 };		//�ļ���Ϣ
	OBJECT_ATTRIBUTES TempObjectAttributes = { 0 };						//��ʱ����
	//0����ȡZwOpenFileԭʼ����
	HANDLE             In_FileHandle = *(ULONG*)((ULONG)ArgArray);
	ACCESS_MASK        In_DesiredAccess = *(ULONG*)((ULONG)ArgArray + 4);
	POBJECT_ATTRIBUTES In_ObjectAttributes = *(ULONG*)((ULONG)ArgArray + 8);
	PIO_STATUS_BLOCK   In_IoStatusBlock = *(ULONG*)((ULONG)ArgArray + 0xC);
	ULONG              In_ShareAccess = *(ULONG*)((ULONG)ArgArray + 0x10);
	ULONG              In_OpenOptions = *(ULONG*)((ULONG)ArgArray + 0x14);
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
	//3���Ƚ�OpenFile�ļ��������� == \\Device\\LanmanServer
	//������������֮���Ǹ�©��
	Safe_CmpLanmanServer(In_ObjectAttributes);
	//����ַ�Ϸ���
	if (myProbeRead(In_FileHandle, sizeof(HANDLE), sizeof(CHAR)))
	{
		KdPrint(("ProbeRead(Fake_ZwOpenFile��In_FileHandle) error \r\n"));
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
		Safe_ZwNtClose(Handle_v5,g_HighgVersionFlag);
		result = STATUS_SUCCESS;
		return result;
	}
	//�жϹ�������,�������ж�����
	if (!(In_ShareAccess & 0x1000))
	{
		if (!(In_DesiredAccess & 0x520D0156) && (((ULONG)In_IoStatusBlock & 1) || !(In_DesiredAccess & 0xFEEDFF7F)))
		{
			result = STATUS_SUCCESS;
			return result;
		}
	}
	//û������userinit.exe
	if (!g_dynData->SystemInformation.Userinit_Flag)
	{
		//����g_System_InformationFile_Data[Index]��Ӧ��SystemInformationList�ṹ��PID��Eprocessֵ
		if (Safe_InsertSystemInformationList(IoGetCurrentProcess(), SYSTEMROOT_SYSTEM32_SMSS_EXE, g_VersionFlag))
		{
			result = STATUS_SUCCESS;
			return result;
		}
	}
	//�жϹ�������,�������ж�����
	if (!(In_ShareAccess & 0x1000))
	{
		*ret_func = After_ZwOpenFile_Func;
		result = STATUS_SUCCESS;
		return result;
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
		if (NT_SUCCESS(Status))
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
				if (!Safe_CheckProtectPath(FileHandle, KernelMode))
				{
					ZwClose(FileHandle);
					result = STATUS_ACCESS_DENIED;
					return result;
				}
			}
			ZwClose(FileHandle);
		}
		result = STATUS_SUCCESS;
		return result;
	}
	result = STATUS_GUARD_PAGE_VIOLATION;
	return result;
}
