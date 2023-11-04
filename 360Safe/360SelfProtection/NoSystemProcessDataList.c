/*
˵����
SystemProcessDataList��NoSystemProcessDataList����������˵������
1��
SystemProcessDataList�����ض�ϵͳ�����ļ���Ϣһ��24��
��ؽṹ��
SystemInformationList����PID��Ϣ
SYSTEM_INFORMATIONFILE_XOR�����ļ���Ϣ

2��
NoSystemProcessDataList��������ض�ϵͳ���̵������ļ���Ϣ�����0x800��
��ؽṹ��
//�����ļ��ļ���ϢУ����Ϣ
//�ļ���ϢУ���SYSTEM_INFORMATIONFILE_XOR
typedef struct _ALL_INFORMATIONFILE_CRC
{
ULONG FileNumber;									// +0   �����С�Ķ���
SYSTEM_INFORMATIONFILE_XOR FileBuff[0x2000];		// +4   ��䣬����֪���ټ�
KSPIN_LOCK	SpinLock;								// ĩβ ������
}ALL_INFORMATIONFILE_CRC, *P_ALL_INFORMATIONFILE_CRC;

P_ALL_INFORMATIONFILE_CRC g_All_InformationFile_CRC;
*/
#include "NoSystemProcessDataList.h"

//************************************     
// ��������: Safe_InsertInformationFileList     
// ����˵����������б����ļ���Ϣ
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2020/04/01     
// �� �� ֵ: ULONG NTAPI    
// ��    ��: IN ULONG IndexNumber_LowPart     [IN]���ļ�ΨһID    FileInternalInformation FILE_INTERNAL_INFORMATION->IndexNumber.LowPart
// ��    ��: IN ULONG IndexNumber_HighPart    [IN]���ļ�ΨһID    FileInternalInformation FILE_INTERNAL_INFORMATION->IndexNumber.HighPart
// ��    ��: IN ULONG VolumeSerialNumber      [IN]���к����      FileFsVolumeInformation _FILE_FS_VOLUME_INFORMATION->VolumeSerialNumber;
//************************************ 
ULONG NTAPI Safe_InsertInformationFileList(IN ULONG IndexNumber_LowPart, IN ULONG IndexNumber_HighPart, IN ULONG VolumeSerialNumber)
{
	KIRQL NewIrql = NULL;
	ULONG Index = NULL;						//�����±�����
	ULONG result = FALSE;					//����ֵ
	//����
	NewIrql = KfAcquireSpinLock(&g_All_InformationFile_CRC->SpinLock);
	//1����������  ����������+1���ɹ�����TRUE������ < 0x1FFE����ʧ��FALSE������ > 0x1FFE��
	//2���Ѵ���    ���ӣ�Ĭ�Ϸ���FALSE��ʧ�ܣ�
	while (IndexNumber_LowPart != g_All_InformationFile_CRC->FileBuff[Index].IndexNumber_LowPart
		&& IndexNumber_HighPart != g_All_InformationFile_CRC->FileBuff[Index].u.IndexNumber_HighPart
		&& VolumeSerialNumber != g_All_InformationFile_CRC->FileBuff[Index].VolumeSerialNumber
		)
	{
		//�������µİ�������Ϣ�Ͳ���
		if (Index >= g_All_InformationFile_CRC->FileCRCListNumber)
		{
			//�ж��Ƿ񳬹����ֵ
			if (Index <= CRCLISTNUMBER)
			{
				//�嵽�����
				g_All_InformationFile_CRC->FileBuff[g_All_InformationFile_CRC->FileCRCListNumber].IndexNumber_LowPart = IndexNumber_LowPart;
				g_All_InformationFile_CRC->FileBuff[g_All_InformationFile_CRC->FileCRCListNumber].u.IndexNumber_HighPart = IndexNumber_HighPart;
				g_All_InformationFile_CRC->FileBuff[g_All_InformationFile_CRC->FileCRCListNumber].VolumeSerialNumber = VolumeSerialNumber;
				//��������1
				g_All_InformationFile_CRC->FileCRCListNumber++;
				//�ɹ�����
				result = TRUE;
				break;
			}
			else
			{
				//ʧ�ܷ���
				result = FALSE;
				break;
			}
		}
		else
		{
			//����
			++Index;
		}
	}
	//����
	KfReleaseSpinLock(&g_All_InformationFile_CRC->SpinLock, NewIrql);
	return result;
}

//************************************     
// ��������: Safe_DeleteInformationFileList     
// ����˵����ɾ�����б����ļ���Ϣ
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2020/04/01     
// �� �� ֵ: ULONG NTAPI    
// ��    ��: IN ULONG IndexNumber_LowPart     [IN]���ļ�ΨһID    FileInternalInformation FILE_INTERNAL_INFORMATION->IndexNumber.LowPart
// ��    ��: IN ULONG IndexNumber_HighPart    [IN]���ļ�ΨһID    FileInternalInformation FILE_INTERNAL_INFORMATION->IndexNumber.HighPart
// ��    ��: IN ULONG VolumeSerialNumber      [IN]���к����      FileFsVolumeInformation _FILE_FS_VOLUME_INFORMATION->VolumeSerialNumber;
//************************************  
ULONG NTAPI Safe_DeleteInformationFileList(IN ULONG IndexNumber_LowPart, IN ULONG IndexNumber_HighPart, IN ULONG VolumeSerialNumber)
{
	KIRQL NewIrql = NULL;
	ULONG result = TRUE;					//����ֵ
	//����
	NewIrql = KfAcquireSpinLock(&g_All_InformationFile_CRC->SpinLock);
	//�ж���������
	if (g_All_InformationFile_CRC->FileCRCListNumber)
	{
		for (ULONG Index = 0; Index < g_All_InformationFile_CRC->FileCRCListNumber; Index++)
		{
			//�ҵ����ظ��������б����±�
			if (
				IndexNumber_LowPart == g_All_InformationFile_CRC->FileBuff[Index].IndexNumber_LowPart
				&& IndexNumber_HighPart == g_All_InformationFile_CRC->FileBuff[Index].u.IndexNumber_HighPart
				&& VolumeSerialNumber == g_All_InformationFile_CRC->FileBuff[Index].VolumeSerialNumber
				)
			{
				//����˳����̵���Ϣ(��һ����ǰŲ)
				for (ULONG i = Index; i <= g_All_InformationFile_CRC->FileCRCListNumber;i++)
				{
					g_All_InformationFile_CRC->FileBuff[i].IndexNumber_LowPart = g_All_InformationFile_CRC->FileBuff[i + 1].IndexNumber_LowPart;
					g_All_InformationFile_CRC->FileBuff[i].u.IndexNumber_HighPart = g_All_InformationFile_CRC->FileBuff[i + 1].u.IndexNumber_HighPart;
					g_All_InformationFile_CRC->FileBuff[i].VolumeSerialNumber = g_All_InformationFile_CRC->FileBuff[i + 1].VolumeSerialNumber;
				}
				//����-1
				g_All_InformationFile_CRC->FileCRCListNumber--;
				break;
			}
		}
	}
	//����
	KfReleaseSpinLock(&g_All_InformationFile_CRC->SpinLock, NewIrql);
	return result;
}

//************************************     
// ��������: Safe_QueryInformationFileList     
// ����˵�������Ҹ��ļ���Ϣ�Ƿ����б��У��ҵ�����1��ʧ�ܷ���0
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2020/04/01     
// �� �� ֵ: ULONG NTAPI    �ҵ�����1���Ҳ�������0  
// ��    ��: IN ULONG IndexNumber_LowPart     [IN]���ļ�ΨһID    FileInternalInformation FILE_INTERNAL_INFORMATION->IndexNumber.LowPart
// ��    ��: IN ULONG IndexNumber_HighPart    [IN]���ļ�ΨһID    FileInternalInformation FILE_INTERNAL_INFORMATION->IndexNumber.HighPart
// ��    ��: IN ULONG VolumeSerialNumber      [IN]���к����      FileFsVolumeInformation _FILE_FS_VOLUME_INFORMATION->VolumeSerialNumber;
//************************************  
ULONG NTAPI Safe_QueryInformationFileList(IN ULONG IndexNumber_LowPart, IN ULONG IndexNumber_HighPart, IN ULONG VolumeSerialNumber)
{
	KIRQL NewIrql;
	ULONG result;
	ULONG GotoFalg;							//����ͬgoto���õ�Falg
	result = 0;
	//����
	NewIrql = KfAcquireSpinLock(&g_All_InformationFile_CRC->SpinLock);
	//�ж���������
	if (g_All_InformationFile_CRC->FileCRCListNumber)
	{
		for (ULONG Index = 0; Index < g_All_InformationFile_CRC->FileCRCListNumber; Index++)
		{
			//�ҵ����ظ��������б����±�
			if (
				IndexNumber_LowPart == g_All_InformationFile_CRC->FileBuff[Index].IndexNumber_LowPart
				&& IndexNumber_HighPart == g_All_InformationFile_CRC->FileBuff[Index].u.IndexNumber_HighPart
				&& VolumeSerialNumber == g_All_InformationFile_CRC->FileBuff[Index].VolumeSerialNumber
				)
			{
				result = 1;
				break;
			}
		}
	}
	//����
	KfReleaseSpinLock(&g_All_InformationFile_CRC->SpinLock, NewIrql);
	return result;
}

//************************************     
// ��������: Safe_QueryInformationFileList_Name     
// ����˵���������ļ��������Ʋ����Ƿ����б���
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2020/04/01     
// �� �� ֵ: ULONG NTAPI    �ҵ�����1���Ҳ�������0  
// ��    ��: IN PUNICODE_STRING ObjectName  �ļ���������
//************************************  
ULONG NTAPI Safe_QueryInformationFileList_Name(IN PUNICODE_STRING ObjectName)
{
	HANDLE FileHandle = NULL;
	ULONG Result = NULL;
	HANDLE Pid = NULL;
	NTSTATUS Status = NULL;
	SYSTEM_INFORMATIONFILE_XOR System_InformationFile_XOR = { 0 };			//�ļ���Ϣ
	// 1. ��ʼ��OBJECT_ATTRIBUTES������
	OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
	ULONG             ulAttributes = OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE;
	InitializeObjectAttributes(
		&ObjectAttributes,								 // ���س�ʼ����ϵĽṹ��
		ObjectName,										 // �ļ���������
		ulAttributes,									 // ��������
		NULL, NULL);									 // һ��ΪNULL
	Pid = PsGetCurrentProcessId();
	//�ǰ��������̼���
	if (!Safe_QueryWhitePID(Pid))
	{
		Status = Safe_IoCreateFile(&ObjectAttributes, &FileHandle);
		if (Status == STATUS_GUARD_PAGE_VIOLATION)
		{
			Result = 1;
			return Result;
		}
		if (NT_SUCCESS(Status))
		{
			//��ȡ�ļ���Ϣ
			Status = Safe_GetInformationFile(FileHandle, (ULONG)&System_InformationFile_XOR, KernelMode);
			if (NT_SUCCESS(Status))
			{
				//��ѯXOR�ڲ����б���
				if (Safe_QueryInformationFileList(
					System_InformationFile_XOR.IndexNumber_LowPart,
					System_InformationFile_XOR.u.IndexNumber_HighPart,
					System_InformationFile_XOR.VolumeSerialNumber))
				{
					Result = 1;
				}
			}
			ZwClose(FileHandle);
		}
	}
	return Result;
}