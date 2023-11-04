/*
�ο����ϣ�
1���ں��з���HKCUע���           
��ַ��https://blog.csdn.net/cssxn/article/details/103089140
*/
#include "Fake_ZwEnumerateValueKey.h"

#define WHILEKEYNAMENUMBER 1
//���صİ���������
PWCHAR g_CmpWhiteDeviceNameString[WHILEKEYNAMENUMBER + 1] = {
	L"360Disabled"
};



//************************************     
// ��������: After_ZwCreateSection_Func     
// ����˵����ԭʼ����ִ�к���  
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
NTSTATUS NTAPI After_ZwEnumerateValueKey_Func(IN ULONG FilterIndex, IN PVOID ArgArray, IN NTSTATUS InResult, IN PULONG RetFuncArgArray)
{
	NTSTATUS                    Status = STATUS_SUCCESS;
	NTSTATUS	                result = STATUS_SUCCESS;
	// PVOID KeyValueInformation, ULONG Length, PULONG ResultLength)
	//0����ȡZwEnumerateValueKeyԭʼ����
	HANDLE						In_KeyHandle = *(ULONG*)((ULONG)ArgArray);
	KEY_VALUE_INFORMATION_CLASS In_KeyValueInformationClass = *(ULONG*)((ULONG)ArgArray + 8);
	ULONG						In_Length = *(ULONG*)((ULONG)ArgArray + 0x10);
	ULONG						In_ResultLength = *(ULONG*)((ULONG)ArgArray + 0x14);
	PKEY_VALUE_FULL_INFORMATION In_KeyValueInformation = *(ULONG*)((ULONG)ArgArray + 0xC);			//ֻ��KeyValueFullInformation�����
	//1���ж��ϴε���ԭʼ��������ֵ,���Ҽ��KeyValueInformationClass���е���һ�٣�ֻ��ΪKeyValueFullInformation�Ž�����
	if (!NT_SUCCESS(InResult) && (In_KeyValueInformationClass != KeyValueFullInformation))
	{
		return InResult;
	}
	//2���������Ϸ���
	if (myProbeRead(In_KeyValueInformation, sizeof(KEY_VALUE_FULL_INFORMATION), sizeof(CHAR)))
	{
		KdPrint(("ProbeRead(After_ZwEnumerateValueKey_Func��In_KeyValueInformation) error \r\n"));
		return result;
	}
	//��
	return result;
}


//ö��valuekey
NTSTATUS NTAPI Fake_ZwEnumerateValueKey(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS       result = STATUS_SUCCESS;
	NTSTATUS       Status = STATUS_SUCCESS;
	HANDLE         Object = NULL;
	ULONG          NumberOfBytes = 0x1024;
	ULONG          ReturnLength = NULL;
	UNICODE_STRING CurrentVersion_RunPath = { 0 };
	UNICODE_STRING Out_CurrentUserKeyPath = { 0 };		//ע�����Ҫ�ͷţ�RtlFreeUnicodeString(&Out_CurrentUserKeyPath)
	UNICODE_STRING FullPathNameString = { 0 };			//����·����\\REGISTRY\\USER\\S-1-5-18  +  \\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run
	ULONG          FullPathNameSize = 0;				//����·���Ĵ�С
	ULONG          Tag = 0x206B6444u;
	PVOID          pBuff = NULL;						//new�����Ŀռ䣬��Ҫ�ͷ�
	POBJECT_NAME_INFORMATION pFileNameInfo = NULL;		//new�����Ŀռ䣬��Ҫ�ͷ�

	RtlInitUnicodeString(&CurrentVersion_RunPath, L"\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	//0����ȡZwEnumerateValueKeyԭʼ����
	HANDLE In_KeyHandle = *(ULONG*)((ULONG)ArgArray);
	KEY_VALUE_INFORMATION_CLASS In_KeyValueInformationClass = *(ULONG*)((ULONG)ArgArray + 8);
	//1��������Ӧ�ò����
	if (ExGetPreviousMode())
	{
		//2�����˲�������
		if (!Safe_QueryWhitePID(PsGetCurrentProcessId()) &&				//�Ǳ�������Ҫ���
			In_KeyValueInformationClass == KeyValueFullInformation &&   //��ֵ�������Ӽ���Ӧ�����ֺ�����
			Safe_CmpImageFileName("explorer.exe"))
		{
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
			//3������·����\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run  ����������
			if (RtlEqualUnicodeString(&pFileNameInfo->Name, &CurrentVersion_RunPath, TRUE))
			{
				//����·��,��Ҫ����ִ�к���
				ExFreePool(pFileNameInfo);
				pFileNameInfo = NULL;
				*ret_func = After_ZwEnumerateValueKey_Func;
				return result;
			}
			//4����ȡ��ǰ�û���SID
			Status = Safe_RunRtlFormatCurrentUserKeyPath(&Out_CurrentUserKeyPath);
			if (NT_SUCCESS(Status))
			{
				//5���ϳ�·����\\REGISTRY\\USER\\S-1-5-18  +  \\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run
				//��ֹ���⣺����ԭ���ռ������Ӧ�ò���������˰�
				FullPathNameSize = (Out_CurrentUserKeyPath.MaximumLength + CurrentVersion_RunPath.MaximumLength) * 2;
				pBuff = Safe_AllocBuff(NonPagedPool, FullPathNameSize, Tag);
				if (pBuff)
				{
					FullPathNameString.MaximumLength = FullPathNameSize;
					FullPathNameString.Buffer = pBuff;
					RtlAppendUnicodeStringToString(&FullPathNameString, &Out_CurrentUserKeyPath);
					RtlAppendUnicodeToString(&FullPathNameString, CurrentVersion_RunPath.Buffer);
					//6������·����\\REGISTRY\\USER\\S-1-5-18\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run  ����������
					if (RtlEqualUnicodeString(&pFileNameInfo->Name, &FullPathNameString, TRUE))
					{
						*ret_func = After_ZwEnumerateValueKey_Func;
					}
					ExFreePool(pBuff);
					pBuff = NULL;
				}
				RtlFreeUnicodeString(&Out_CurrentUserKeyPath);
				//�ͷſռ�
				if (pFileNameInfo)
				{
					ExFreePool(pFileNameInfo);
					pFileNameInfo = NULL;
				}
				return result;
			}
			else
			{
				//�ͷſռ�
				if (pFileNameInfo)
				{
					ExFreePool(pFileNameInfo);
					pFileNameInfo = NULL;
				}
			}

		}
	}
	return result;
}