#include "Fake_ZwSetSystemInformation.h"


NTSTATUS NTAPI After_ZwSetSystemInformation_Func(IN ULONG FilterIndex, IN PVOID ArgArray, IN NTSTATUS InResult, IN PULONG RetFuncArgArray)
{
	NTSTATUS       Status = STATUS_SUCCESS;
	NTSTATUS       Result = STATUS_SUCCESS;
	//0����ȡZwSetSystemInformationԭʼ����
	SYSTEM_INFORMATION_CLASS In_SystemInformationClass = *(ULONG*)((ULONG)ArgArray);
	PVOID In_SystemInformation = *(ULONG*)((ULONG)ArgArray + 4);
	ULONG In_SystemInformationLength = *(ULONG*)((ULONG)ArgArray + 8);
	//��
	return Result;
}

//ZwSetSystemInformation��XP��Ч��
NTSTATUS NTAPI Fake_ZwSetSystemInformation(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS       Status = STATUS_SUCCESS;
	NTSTATUS       Result = STATUS_SUCCESS;
	UNICODE_STRING Win32kString = { 0 };
	SYSTEM_INFORMATIONFILE_XOR System_InformationFile_New = { 0 };			//�����ļ���Ϣ
	SYSTEM_INFORMATIONFILE_XOR System_InformationFile_Old = { 0 };			//ԭʼ�ļ���Ϣ
	PSYSTEM_LOAD_AND_CALL_IMAGE GregsImage = NULL;
	RtlInitUnicodeString(&Win32kString, L"\\SystemRoot\\System32\\win32k.sys");
	//0����ȡZwSetSystemInformationԭʼ����
	SYSTEM_INFORMATION_CLASS In_SystemInformationClass = *(ULONG*)((ULONG)ArgArray);
	PVOID In_SystemInformation = *(ULONG*)((ULONG)ArgArray + 4);
	ULONG In_SystemInformationLength = *(ULONG*)((ULONG)ArgArray + 8);
	//1��������Ӧ�ò����
	if (ExGetPreviousMode())
	{
		//ֻ�������м�����������
		switch (In_SystemInformationClass)
		{
			case SystemLoadAndCallImage:
			{
				//�����������õ�
				__try
				{
					//�жϲ����Ϸ���
					ProbeForRead(In_SystemInformation, sizeof(SYSTEM_LOAD_AND_CALL_IMAGE), sizeof(CHAR));
					GregsImage = (PSYSTEM_LOAD_AND_CALL_IMAGE)In_SystemInformation;
					if (GregsImage->ModuleName.Length)
					{
						ProbeForRead(GregsImage->ModuleName.Buffer, GregsImage->ModuleName.Length, sizeof(CHAR));
						//����������\\SystemRoot\\System32\\win32k.sys
						//����һ����·���ѵ����������ͬ�Ľ��?????????
						//����
						if (RtlEqualUnicodeString(GregsImage, &Win32kString,TRUE))
						{
							//�ж�Win32k�Ƿ��޸�
							Status = Safe_KernelCreateFile(&GregsImage->ModuleName, (ULONG)&System_InformationFile_New);	//��ȡ����
 							Result = Safe_KernelCreateFile(&Win32kString, (ULONG)&System_InformationFile_Old);				//��ȡϵͳĿ¼��ԭʼWin32k
							if (NT_SUCCESS(Status) && NT_SUCCESS(Result)&&
								(System_InformationFile_New.IndexNumber_LowPart == System_InformationFile_Old.IndexNumber_LowPart) &&
								(System_InformationFile_New.VolumeSerialNumber == System_InformationFile_Old.VolumeSerialNumber) &&
								(System_InformationFile_New.u.IndexNumber_HighPart == System_InformationFile_Old.u.IndexNumber_HighPart))
							{
								//δ���޸���������
								Result = STATUS_SUCCESS;
							}
							else
							{
								//ʧ�ܷ���
								Safe_18A72_SendR3(PsGetCurrentProcessId(), PsGetCurrentThreadId(), 0x0);
								Result = STATUS_ACCESS_DENIED;
							}
						}
						else
						{
							//������������·��ֱ����
							Result = STATUS_SUCCESS;
						}
						//���õ��ú���
						*(ULONG*)ret_func = After_ZwSetSystemInformation_Func;
					}
					else
					{
						Result = STATUS_SUCCESS;
					}
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					Result = STATUS_SUCCESS;
					return Result;
				}
				break;
			}
			case SystemHotpatchInformation:
			{
				//�����������õ�
				if (g_SystemHotpatchInformation_Switch &&	g_VersionFlag == WINDOWS_VERSION_XP)
				{
					//���󷵻�
					Safe_18A72_SendR3(PsGetCurrentProcessId(), PsGetCurrentThreadId(), 0x7);
					Result = STATUS_ACCESS_DENIED;
				}
				else
				{
					Result = STATUS_SUCCESS;
				}
				break;
			}
			case SystemLoadGdiDriverInSystemSpace:
			{
				//������û�н�������
				// ���xp Ĭ�Ϸ��� STATUS_INFO_LENGTH_MISMATCH
				break;
			}
			case SystemLoadImage:
			{
				//������û�н�������
				// ���xp Ĭ�Ϸ��� STATUS_INFO_LENGTH_MISMATCH
				break;
			}
			default:
			{
				//������Ȥ������
				Result = STATUS_SUCCESS;
				break;
			}
		}
	}
	return Result;
}
