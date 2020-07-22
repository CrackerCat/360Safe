/*
˵����
����������ʾʱ��Ӧ�ò��ͨ�Ž������ⲿ�ִ�����û������ֱ�ӳ������ֳɵ�
�ο����ϣ�
1������������ʾʱ��Ӧ�ò��ͨ�Ž��� 
��ַ��https://bbs.pediy.com/thread-144884.htm
*/
#include "SafeWarning.h"

//�˶�IRP��SessionId == ��ǰ���̻SessionId
//����ֵ������1��������0
BOOLEAN NTAPI Safe_is_irp_reqeust_from_local(PIRP Irp_a1)
{
	BOOLEAN Result = TRUE;
	ULONG PID = NULL;
	KIRQL NewIrql = NULL;
	ULONG Index = 0;
	ULONG SelfSessionId = NULL;
	NTSTATUS Status;
	ULONG uSessionId = NULL;
	UNICODE_STRING DestinationString;
	ULONG SpecialWhiteListNumber = 0;
	PVOID pSessionIDAddress = 0xFFDF02D8;	//Win10_14393���°汾һ���̶���ַ���Ի�ȡ��SessionId
	SpecialWhiteListNumber = g_SpecialWhite_List.SpecialWhiteListNumber;
	if (WINDOWS_VERSION_2K != g_VersionFlag)
	{
		PID = IoGetRequestorProcessId(Irp_a1);
		NewIrql = KfAcquireSpinLock(&g_SpecialWhite_List.SpinLock);
		if (SpecialWhiteListNumber)
		{
			while (g_SpecialWhite_List.SpecialWhiteListPID[Index] != PID || g_SpecialWhite_List.SpecialWhiteListSessionId[Index] != SPECIALSIGN)
			{
				if (++Index >= SpecialWhiteListNumber)
					goto LABEL_6;
			}
			//�������������ֱ�ӷ���
			KfReleaseSpinLock(&g_SpecialWhite_List.SpinLock, NewIrql);
			Result = 1;
			return Result;
		}
	LABEL_6:
		//��ͨ����
		KfReleaseSpinLock(&g_SpecialWhite_List.SpinLock, NewIrql);
		if (pIoGetRequestorSessionId
			|| (RtlInitUnicodeString(&DestinationString, L"IoGetRequestorSessionId"),
			(pIoGetRequestorSessionId = MmGetSystemRoutineAddress(&DestinationString)) != 0))
		{
			//����IRP���������ڽ���������SessionId
			Status = pIoGetRequestorSessionId(Irp_a1, &uSessionId);
			if (NT_SUCCESS(Status))
			{
				//��ȡ����SessionId
				if (g_dynData->pRtlGetActiveConsoleId_Win10_14393)
				{
					SelfSessionId = g_dynData->pRtlGetActiveConsoleId_Win10_14393();
					return SelfSessionId == uSessionId;
				}
				if (MmIsAddressValid(pSessionIDAddress))
				{
					SelfSessionId = *(ULONG*)pSessionIDAddress;
					return SelfSessionId == uSessionId;
				}
			}
		}
		Result = 1;
	}
	return Result;
}

PVOID NTAPI Safe_check_irp_request_in_list()
{
	//struct _LIST_ENTRY *v0; // esi@1

	//PIRP pIrp; // edi@4

	//struct _LIST_ENTRY *v2; // eax@7

	//struct _LIST_ENTRY *v3; // esi@7

	//PMDL pmdl; // eax@10

	//PVOID request_buffer1; // eax@11

	//unsigned int size; // esi@13

	//PMDL pMdl; // eax@13

	//KIRQL v8; // al@17

	//struct _LIST_ENTRY *v9; // esi@19

	//ULONG v10; // edi@19

	//struct _LIST_ENTRY *v11; // ecx@19

	//int result; // eax@22

	//IRP *v13; // [sp+Ch] [bp-14h]@8

	//unsigned int v14; // [sp+10h] [bp-10h]@13

	//void *request_buffer2; // [sp+14h] [bp-Ch]@13

	//ULONG copied_size; // [sp+18h] [bp-8h]@13

	//char v17; // [sp+1Eh] [bp-2h]@1

	//ULONG MaxSize = sizeof(QUERY_PASS_R0SENDR3_DATA) - sizeof(LIST_ENTRY);		//ȥ��ǰ��LIST_ENTRY��������Ĳ���ʵ������ ������0x840 = 0x848(QUERY_PASS_R0SENDR3_DATA) - 0x8(LIST_ENTRY)

	//KIRQL i;
	//v0 = g_can_check_hook_request_list_added_by_r3.Flink;
	//for (i = KfAcquireSpinLock(&g_request_list_lock);
	//	!IsListEmpty(&g_can_check_hook_request_list_added_by_r3);
	//	i = KfAcquireSpinLock(&g_request_list_lock))
	//{
	//	if (IsListEmpty(&g_request_list))
	//	{
	//		break;
	//	}
	//	if (v0 == &g_can_check_hook_request_list_added_by_r3)
	//	{
	//		pIrp = v13;
	//	}
	//	else
	//	{
	//		while (1)

	//		{

	//			pIrp = (IRP *)&v0[-11];

	//			v13 = (IRP *)&v0[-11];

	//			if (InterlockedExchange((volatile LONG *)&v0[-4], 0))
	//			{
	//				break;
	//			}

	//			v0 = v0->Flink;

	//			if (v0 == &g_can_check_hook_request_list_added_by_r3)
	//			{
	//				goto LABEL_9;
	//			}

	//		}

	//		v2 = v0->Flink;

	//		v3 = v0->Blink;

	//		v3->Flink = v2;

	//		v2->Blink = v3;

	//		v17 = 1;

	//	}
	//LABEL_9:
	//	if (!v17)
	//	{
	//		break;
	//	}
	//	KfReleaseSpinLock(&g_request_list_lock, i);

	//	pmdl = pIrp->MdlAddress;

	//	request_buffer1 = pmdl->MdlFlags & 5 ? pmdl->MappedSystemVa : MmMapLockedPagesSpecifyCache(
	//		pmdl,
	//		0,
	//		MmCached,
	//		0,
	//		0,
	//		NormalPagePriority);

	//	copied_size = 0;

	//	request_buffer2 = request_buffer1;

	//	pMdl = pIrp->MdlAddress;

	//	size = pMdl->ByteCount;                     // �����е��������ݶ����Ƶ�request_buffer2��  

	//	v14 = pMdl->ByteCount;

	//	if (size > 4)
	//	{
	//		*(ULONG *)request_buffer2 = 0;
	//	}
	//	//�˶�IRP��SessionId == ��ǰ���̻SessionId������ֵ������1��������0
	//	if (Safe_is_irp_reqeust_from_local(pIrp))
	//	{
	//		v8 = KfAcquireSpinLock(&g_request_list_lock);
	//		if (IsListEmpty(&g_request_list))
	//		{
	//			break;
	//		}
	//		do
	//		{
	//			v9 = g_request_list.Flink->Flink;

	//			v10 = (ULONG)g_request_list.Flink;

	//			v11 = g_request_list.Flink->Blink;

	//			v11->Flink = g_request_list.Flink->Flink;

	//			v9->Blink = v11;

	//			--g_request_counter;

	//			KfReleaseSpinLock(&g_request_list_lock, v8);

	//			//ȥ��ǰ��һ��_LIST_ENTRY�ṹ
	//			memcpy(request_buffer2, (VOID *)(v10 + 8), MaxSize);

	//			copied_size += MaxSize;

	//			request_buffer2 = (CHAR *)request_buffer2 + MaxSize;

	//			v14 -= MaxSize;

	//			ExFreePool((PVOID)v10);

	//			v8 = KfAcquireSpinLock(&g_request_list_lock);

	//			pIrp = v13;

	//		} while (v14 >= MaxSize);
	//		KfReleaseSpinLock(&g_request_list_lock, v8);
	//	}
	//	else
	//	{
	//		copied_size = size;
	//	}
	//	pIrp->IoStatus.Status = 0;

	//	pIrp->IoStatus.Information = copied_size;

	//	IofCompleteRequest(pIrp, 0);

	//	v0 = g_can_check_hook_request_list_added_by_r3.Flink;
	//}
	//KfReleaseSpinLock(&g_request_list_lock, i);
}

//************************************     
// ��������: Safe_push_request_in_and_waitfor_finish     
// ����˵����    
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��   
// �� �� ֵ: ULONG NTAPI             
// 0 �������أ��Ϸ���
// 2 ���󷵻أ����Ϸ���
// 3 �������������̣��Ϸ���
// ��    ��: IN PQUERY_PASS_R0SENDR3_DATA In_pBuff     
// ��    ��: IN ULONG In_Flag     
//************************************ 
ULONG NTAPI Safe_push_request_in_and_waitfor_finish(IN PQUERY_PASS_R0SENDR3_DATA In_pBuff, IN ULONG In_Flag)
{
	ULONG Result = NULL;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	KIRQL NewIrql = NULL;
	PLIST_ENTRY v5 = NULL;
	PLIST_ENTRY v7 = NULL;
	PLIST_ENTRY v8 = NULL;
	WAITFOR_INFO wait_info = { 0 };
	LARGE_INTEGER Timeout = { 0 };
	ULONG AddendMax = 0xA;
	//1���жϺϷ���
	if (g_Addend > AddendMax)
	{
		//���󷵻�
		Safe_ExFreePool(In_pBuff);
		Result = 2;
		return Result;
	}
	//2�������ǰ���������ֱ�ӷ���
	if (Safe_QueryWhitePID(In_pBuff->CheckWhitePID) || !Safe_QuerySpecialWhiteSessionId())// 1���ж��ǲ��ǰ���������    2���ж��������������SessionId�Ƿ���ڵ�ǰ���̵�SessionId
	{
		//����������
		Safe_ExFreePool(In_pBuff);
		Result = 3;
		return Result;
	}
	if (KeGetCurrentIrql() >= DISPATCH_LEVEL)
	{
		return FALSE;
	}
	//3���ǰ���������
	NewIrql = KfAcquireSpinLock(&g_request_list_lock);
	v5 = g_request_list.Blink;
	In_pBuff->Entry.Blink = g_request_list.Blink;
	In_pBuff->Entry.Flink = &g_request_list;			// �����������  
	v5->Flink = (LIST_ENTRY *)In_pBuff;
	++g_request_counter;
	g_request_list.Blink = (LIST_ENTRY *)In_pBuff;
	KfReleaseSpinLock(&g_request_list_lock, NewIrql);
	//4������ִ��
	if (In_Flag)
	{
		wait_info.tid = PsGetCurrentThreadId();
		wait_info.bypass_or_not = 0;					//2�����أ�0��3ͨ��
		KeInitializeEvent(&wait_info.Event, 0, 0);
		ExfInterlockedInsertTailList(&g_wait_info_list, &wait_info.list, &g_SpinLock_wait_info_list);
		Exfi386InterlockedIncrementLong(&g_Addend);
		Safe_check_irp_request_in_list();
		*(ULONGLONG *)&Timeout.QuadPart = -600000000i64 * g_Addend;
		Status = KeWaitForSingleObject(&wait_info.Event, 0, 0, 0, &Timeout);
		NewIrql = KfAcquireSpinLock(&g_SpinLock_wait_info_list);
		v7 = wait_info.list.Flink;
		v8 = wait_info.list.Blink;            // ��waitfor_info�����     
		wait_info.list.Blink->Flink = wait_info.list.Flink;
		v7->Blink = v8;
		--g_Addend;
		KfReleaseSpinLock(&g_SpinLock_wait_info_list, NewIrql);
		if (STATUS_TIMEOUT == Status)
		{
			Result = 2;
		}
		else
		{
			Result = wait_info.bypass_or_not;
		}

	}
	else
	{
		Safe_check_irp_request_in_list();
		Result = 0;
	}
	return Result;
}

//************************************     
// ��������: Safe_18A72_SendR3     
// ����˵��������������ʾ��Ӧ�ò�ͨѶ����    
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��   
// �� �� ֵ: PVOID NTAPI     
// ��    ��: IN HANDLE In_PorcessID     
// ��    ��: IN HANDLE In_ThreadID     
// ��    ��: IN ULONG ProcessinfoclassIndex    
//           0x10����ʾ������湥��������֮����
//************************************ 
PVOID NTAPI Safe_18A72_SendR3(IN HANDLE In_PorcessID, IN HANDLE In_ThreadID, IN PROCESSINFOCLASS  ProcessInformationClass)
{
	//�ԣ�����д���������ٲ���
}

//************************************     
// ��������: Safe_CheckSys_SignatureOrHash     
// ����˵������������Ϸ���   
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��   
// �� �� ֵ: NTSTATUS NTAPI     
// ��    ��: IN HANDLE In_PorcessID                     [In]PsGetCurrentProcessId()
// ��    ��: IN HANDLE In_ThreadID                      [In]PsGetCurrentThreadId()
// ��    ��: IN PUNICODE_STRING In_pImagePathString     [In]������·��
// ��    ��: OUT PVOID Out_Hash                         [Out]�����Ĺ�ϣֵ
// ��    ��: OUT ULONG Out_FileSize                     [Out]�ļ���С
// ��    ��: OUT ULONG Out_PassFlag                     [Out]����or���ر�ʶ
// ��    ��: IN ULONG In_Flag_a6                        [In]δ֪
//************************************  
NTSTATUS NTAPI Safe_CheckSys_SignatureOrHash(IN HANDLE In_PorcessID, IN HANDLE In_ThreadID, IN PUNICODE_STRING In_pImagePathString, OUT PVOID Out_Hash, OUT ULONG Out_FileSize, OUT ULONG Out_PassFlag, IN ULONG In_Flag_a6)
{
	NTSTATUS                  result = STATUS_SUCCESS;
	NTSTATUS	              Status = STATUS_SUCCESS;
	HANDLE                    FileHandle = NULL;
	IO_STATUS_BLOCK	          StatusBlock = { 0 };
	ULONG                     KeyBuff[0x50] = { 0 };
	SIZE_T					  ulLength = NULL;		   // new�����ֽ�
	PVOID                     pBuff = NULL;			   // new�ռ�
	ULONG                     Tag = 0x206B6444u;
	FILE_STANDARD_INFORMATION FileStInformation = { 0 };
	//1. ��ʼ��OBJECT_ATTRIBUTES������
	OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
	IO_STATUS_BLOCK IoStatusBlock = { 0 };
	InitializeObjectAttributes(&ObjectAttributes, In_pImagePathString, OBJ_CASE_INSENSITIVE, NULL, NULL);
	//2�����ļ�
	Status = ZwOpenFile(&FileHandle, FILE_READ_ATTRIBUTES, &ObjectAttributes, &StatusBlock, FILE_DOES_NOT_EXIST, FILE_SYNCHRONOUS_IO_NONALERT);
	if (!NT_SUCCESS(Status))
	{
		goto _FunctionRet;
	}
	//3����ȡ�ļ�������Ϣ
	Status = ZwQueryInformationFile(FileHandle, &IoStatusBlock, (PVOID)&FileStInformation, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
	//3��1 �жϺϷ���
	if (!NT_SUCCESS(Status) || (FileStInformation.EndOfFile.QuadPart > 0x6400000ui64) || !(FileStInformation.EndOfFile.QuadPart))
	{
		DbgPrint("Cannot Query File Size! %08X\n", Status);
		goto _FunctionRet;
	}
	//3��2 newͬ�ȿռ�
	ulLength = FileStInformation.EndOfFile.LowPart;
	pBuff = Safe_AllocBuff(PagedPool, ulLength, Tag);
	if (!pBuff)
	{
		Status = STATUS_UNSUCCESSFUL;
		goto _FunctionRet;
	}
	//4�� ��ȡ�ļ�
	Status = ZwReadFile(
		FileHandle,    // �ļ����
		NULL,          // �ź�״̬(һ��ΪNULL)
		NULL, NULL,    // ����
		&IoStatusBlock,// ���ܺ����Ĳ������
		pBuff,         // �����ȡ���ݵĻ���
		ulLength,      // ��Ҫ��ȡ�ĳ���
		NULL,		   // ��ȡ����ʼƫ��
		NULL);         // һ��ΪNULL
	if (!NT_SUCCESS(Status))
	{
		goto _FunctionRet;
	}
	//���ǩ���Ĵ�����û������
	//��
	//�Ϸ�ǩ��ֱ���˳�������ֵOut_PassFlag = 1
	//���Ϸ�ǩ������У�飬��ѯ�Ƿ����б���
	//5�����漸�����Ǽ����PE�ļ��Ĺ�ϣֵ����
	//Safe_14F7C_Hash(KeyBuff);
	//Safe_15846_Hash(KeyBuff, pBuff, ulLength);
	//Safe_158F8_Hash(Out_Hash, KeyBuff);
	//6����ѯ�б����Ƿ����
	*(ULONG*)Out_PassFlag = Safe_QueryDrvmkDataList(Out_Hash, ulLength);
	*(ULONG*)Out_FileSize = ulLength;
_FunctionRet:
	result = Status;
	if (pBuff)
	{
		ExFreePool(pBuff);
		pBuff = NULL;
	}
	if (FileHandle)
	{
		ZwClose(FileHandle);
		FileHandle = NULL;
	}
	return result;
}
//************************************     
// ��������: Safe_1D044_SendR3     
// ����˵��������������ʾ��Ӧ�ò�ͨѶ����    
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��   
// �� �� ֵ: NTSTATUS NTAPI     
// ��    ��: IN HANDLE In_PorcessID     
// ��    ��: IN HANDLE In_ThreadID     
// ��    ��: IN ULONG  In_Flag     
// ��    ��: IN PUNICODE_STRING In_ImagePathString     
//************************************  
NTSTATUS NTAPI Safe_1D044_SendR3(IN HANDLE In_PorcessID, IN HANDLE In_ThreadID, IN ULONG In_Flag, IN PUNICODE_STRING In_ImagePathString)
{
	PQUERY_PASS_R0SENDR3_DATA  pQuery_Pass = NULL;
	NTSTATUS                   result = STATUS_SUCCESS;
	NTSTATUS                   Status = STATUS_SUCCESS;
	ULONG                      Flag_v4 = NULL;
	ULONG                      Pass_Flag = NULL;				//�ж��Ƿ����  0����   1����    2��һ�ν�����
	ULONG                      SpecialWhiteNumber = NULL;		//������R3��������ż���ִ�У�������淢������Ϣ��R3��û��Ҫ��
	ULONG                      uHash[0x10] = { 0 };				//�����ϣֵ
	ULONG                      FileSize = 0;					//�ļ���С
	ULONG                      Flag_v16 = NULL;					//����R3�û����صĽ��
	ULONG                      Tag = 0x206B6444;
	SpecialWhiteNumber = g_SpecialWhite_List.SpecialWhiteListNumber;
	//1���жϺϷ���
	if (Flag_v4 == 6 || !SpecialWhiteNumber)
	{
		Flag_v4 = 0;
	}
	//Ĭ����1
	if (!g_Regedit_Data.g_SpShadow0_Data_DWORD)
	{
		return result;
	}
	//2���жϸ������ĺϷ������磺ǩ�����������ȵ�
	Status = Safe_CheckSys_SignatureOrHash(In_PorcessID, In_ThreadID, In_ImagePathString, &uHash, &FileSize, &Pass_Flag, Flag_v4);
	if (NT_SUCCESS(Status))
	{
		//3��������R3��������ż���ִ�У�������淢������Ϣ��R3��û��Ҫ��
		if (SpecialWhiteNumber)
		{
			//0����   1����    2����ִ�����̣�Ҫ���ģ�
			switch (Pass_Flag)
			{
				case 0:		//���ؽ��� ���ش���ֵ����
				{			
					result = STATUS_ACCESS_DENIED;
					break;
				}
				case 1:		//���������� �������� STATUS_SUCCESS
				{		
					result = STATUS_SUCCESS;
					break;
				}
				case 2:		//��������
				{
					//3��1 new�ռ䣬���洫�ݸ�R3�Ľ�������
					pQuery_Pass = (PQUERY_PASS_R0SENDR3_DATA)Safe_AllocBuff(NonPagedPool, sizeof(QUERY_PASS_R0SENDR3_DATA), Tag);
					if (pQuery_Pass)
					{

						//������ݣ���������R3���Ի������û����� ����or����
						pQuery_Pass->CheckWhitePID = In_PorcessID;
						pQuery_Pass->Unknown_CurrentThreadId_4 = In_ThreadID;
						pQuery_Pass->Unknown_CurrentThreadId_5 = PsGetCurrentThreadId();
						pQuery_Pass->Hash[0] = uHash[0];
						pQuery_Pass->Hash[1] = uHash[1];
						pQuery_Pass->Hash[2] = uHash[2];
						pQuery_Pass->Hash[3] = uHash[3];
						pQuery_Pass->Unknown_Flag_6 = 1;
						pQuery_Pass->Unknown_Flag_830 = In_Flag;
						pQuery_Pass->FileSize = FileSize;
						pQuery_Pass->Unknown_Flag_2 = 2;
						//�ַ�����󳤶�Ӧ�ò�����520�ɣ�һ�����520�����ȵ�·��ֱ����������
						if (In_ImagePathString->Length < 520)
						{
							RtlCopyMemory(pQuery_Pass->ImagePathBuff, In_ImagePathString->Buffer, In_ImagePathString->Length);
						}
						//������Ϣ��R3
						//Flag_v16 = Safe_push_request_in_and_waitfor_finish(pQuery_Pass, 1);
						//����R3�ķ���ֵ������Ӧ����
						if (Flag_v16 == 0)				//��Ӱ����������б�����������
						{
							Safe_InsertDrvmkDataList(uHash, FileSize, 0);
							result = STATUS_SUCCESS;
						}
						else if (Flag_v16 == 1)			//��Ӻ����������б������ش���ֵ
						{
							Safe_InsertDrvmkDataList(uHash, FileSize, 1);
							result = STATUS_ACCESS_DENIED;
						}
						else if (Flag_v16 == 2)			//ֱ�ӷ��ش���ֵ
						{
							result = STATUS_ACCESS_DENIED;
						}
						else                            //ֱ����������
						{
							result = STATUS_SUCCESS;
						}
						//��������ͷ�
						if (pQuery_Pass)
						{
							ExFreePool(pQuery_Pass);
							pQuery_Pass = NULL;
						}
					}
					break;
				}
				default:
				{
					break;
				}
			}
		}
	}
	return result;
}
