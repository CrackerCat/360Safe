/*
�������ܣ�
1�����˵�\\Device\\PhysicalMemory���е�ַ����

�ο����ϣ�
1��ͻ��HIPS�ķ���˼·֮duplicate physical memory    
��ַ��https://bbs.pediy.com/thread-89068.htm
2����PhysicalMemory����                            
��ַ��https://bbs.pediy.com/thread-94203.htm
*/

#include "Fake_ZwCreateSymbolicLinkObject.h"



//������������
NTSTATUS NTAPI Fake_ZwCreateSymbolicLinkObject(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS            result = STATUS_SUCCESS;
	NTSTATUS            Status = STATUS_SUCCESS;
	HANDLE              FileHandle = NULL;
	ULONG               ulAttributes = OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE;
	OBJECT_ATTRIBUTES   ObjectAttributes = { 0 };
	SYSTEM_INFORMATIONFILE_XOR System_InformationFile_XOR = { 0 };			//�ļ���Ϣ
	UNICODE_STRING      PhysicalMemoryString;								//��ֹR3ֱ�ӷ���\\Device\\PhysicalMemory���������̷��ʳ���
	UNICODE_STRING      DevicesLanmanRedirectorString;						//��ֹ�ǽ��̷��ʹ����ļ���
	UNICODE_STRING      DosDevicesComString;								//��������\\DosDevices\\COM
	UNICODE_STRING      DeviceMailSlotString;								//��������\\Device\\MailSlot
	UNICODE_STRING      DeviceNamedPipeString;								//��������\\Device\\NamedPipe
	UNICODE_STRING      DeviceMupString;									//��������\\Device\\Mup
	UNICODE_STRING      DeviceLPTString;									//��������\\Device\\LPT
	UNICODE_STRING      DeviceWebDavRedirectorString;						//��������\\Device\\WebDavRedirector
	//0����ȡZwCreateSymbolicLinkObjectԭʼ����
	ACCESS_MASK			In_DesiredAccess = *(ULONG*)((ULONG)ArgArray + 4);
	POBJECT_ATTRIBUTES  In_ObjectAttributes = *(ULONG*)((ULONG)ArgArray + 8);
	PUNICODE_STRING		In_TargetName = *(ULONG*)((ULONG)ArgArray + 0xC);
	//1����ʼ���ַ�������
	RtlInitUnicodeString(&PhysicalMemoryString, L"\\Device\\PhysicalMemory");
	RtlInitUnicodeString(&DevicesLanmanRedirectorString, L"\\Device\\LanmanRedirector");
	RtlInitUnicodeString(&DosDevicesComString, L"\\DosDevices\\COM");
	RtlInitUnicodeString(&DeviceMailSlotString, L"\\Device\\MailSlot");
	RtlInitUnicodeString(&DeviceNamedPipeString, L"\\Device\\NamedPipe");
	RtlInitUnicodeString(&DeviceMupString, L"\\Device\\Mup");
	RtlInitUnicodeString(&DeviceLPTString, L"\\Device\\LPT");
	RtlInitUnicodeString(&DeviceWebDavRedirectorString, L"\\Device\\WebDavRedirector");
	//2��������Ӧ�ò����
	if (ExGetPreviousMode())
	{
		//�жϲ����Ϸ���
		if (myProbeRead(In_TargetName, sizeof(UNICODE_STRING), sizeof(CHAR)))
		{
			KdPrint(("ProbeRead(Fake_ZwCreateSymbolicLinkObject��In_TargetName) error \r\n"));
			return result;
		}
		//�������з���·����\\Device\\PhysicalMemory��ֻ�а����������߲����ʸ����
		if (RtlEqualUnicodeString(In_TargetName, &PhysicalMemoryString, TRUE) && !Safe_QueryWhitePID(PsGetCurrentProcessId()))
		{
			result = STATUS_ACCESS_DENIED;
		}
		//�������з���·����\\Device\\LanmanRedirector �����ļ��У�ֻ�а����������߲����ʸ����
		else if (RtlEqualUnicodeString(In_TargetName, &DosDevicesComString, TRUE))
		{
			InitializeObjectAttributes(
				&ObjectAttributes,								 // ���س�ʼ����ϵĽṹ��
				In_TargetName,									 // �ļ���������
				ulAttributes,									 // ��������
				NULL, NULL);									 // һ��ΪNULL
			Status = Safe_IoCreateFile(&ObjectAttributes, &FileHandle);
			if (Status != STATUS_GUARD_PAGE_VIOLATION)
			{
				if (NT_SUCCESS(Status))
				{
					//��ȡ�ļ���Ϣ
					Status = Safe_GetInformationFile(FileHandle, (ULONG)&System_InformationFile_XOR, KernelMode);
					ZwClose(FileHandle);
					//��֤�ļ���Ϣ
					if (NT_SUCCESS(Status))
					{
						//��ѯXOR�ڲ����б���
						if (Safe_QueryInformationFileList(
							System_InformationFile_XOR.IndexNumber_LowPart,
							System_InformationFile_XOR.u.IndexNumber_HighPart,
							System_InformationFile_XOR.VolumeSerialNumber))
						{
							//�ǰ�����ֱ�Ӵ��󷵻�
							if (!Safe_QueryWhitePID(PsGetCurrentProcessId()))
							{
								return STATUS_ACCESS_DENIED;
							}
						}
					}
				}
			}
			else
			{
				//���󷵻�
				result = Status;
			}
		}
		//�������ķ���·��
		else if (RtlEqualUnicodeString(In_TargetName, &DosDevicesComString, TRUE) ||
			RtlEqualUnicodeString(In_TargetName, &DeviceMailSlotString, TRUE) ||
			RtlEqualUnicodeString(In_TargetName, &DeviceNamedPipeString, TRUE) ||
			RtlEqualUnicodeString(In_TargetName, &DeviceMupString, TRUE) ||
			RtlEqualUnicodeString(In_TargetName, &DeviceLPTString, TRUE) ||
			RtlEqualUnicodeString(In_TargetName, &DeviceWebDavRedirectorString, TRUE)
			)
		{
			result = STATUS_SUCCESS;
		}
	}
	return result;
}