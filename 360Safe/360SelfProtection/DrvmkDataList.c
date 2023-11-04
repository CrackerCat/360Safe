/*
1��Drvmk���ı�����360�������ذ�������ֻҪ������Ӹ�������Ϣ�����������Ͳ�����
2��R0�������в�������Ϣ Send  R3��R3���͸��Ի������û������жϣ�����or����Ȼ��Ѷ�Ӧ����Ϣд���б���
*/
#include "DrvmkDataList.h"

//��ȡTextOutCache��ֵ������ݣ���������һ��·��ָ��xxx\\xxx\\xxx\\drvmk.dat
NTSTATUS NTAPI Safe_InitializeTextOutCacheList(IN PCWSTR In_Data, IN ULONG Type, IN ULONG DataLength, IN ULONG Flag)
{
	NTSTATUS       result,Status;
	HANDLE	       FileHandle = NULL;
	PVOID		   pBuffer = NULL;		// ָ��g_Drvmk_List�ṹ�׵�ַ
	ULONG          ulLength = NULL;		// ��ȡ�����ֽ�
	LARGE_INTEGER  ByteOffset = { 0 };	// �����￪ʼ��ȡ
	UNICODE_STRING ObjectNameString;
	FILE_STANDARD_INFORMATION FileStInformation = { 0 };
	result = STATUS_SUCCESS;
	//�жϺϷ���
	if (Type == REG_SZ && DataLength && g_Drvmk_List)
	{
		pBuffer = g_Drvmk_List;
		//1. ��ʼ��OBJECT_ATTRIBUTES������
		OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
		IO_STATUS_BLOCK IoStatusBlock = { 0 };
		RtlInitUnicodeString(&ObjectNameString, In_Data);
		InitializeObjectAttributes(&ObjectAttributes, &ObjectNameString, OBJ_CASE_INSENSITIVE, NULL, NULL);
		//2����Drvmk.dat�ļ�
		Status = IoCreateFile(
			&FileHandle,
			GENERIC_READ | SYNCHRONIZE,
			&ObjectAttributes,
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
		if (!NT_SUCCESS(Status))
		{
			result = Status;
			return result;
		}
		//3����ȡ�ļ�������Ϣ
		Status = ZwQueryInformationFile(FileHandle, &IoStatusBlock, (PVOID)&FileStInformation, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
		if (!NT_SUCCESS(Status) ||
			FileStInformation.EndOfFile.HighPart ||
			(ulLength = FileStInformation.EndOfFile.LowPart,			//����ReadFileʵ�ʶ�ȡ����
			FileStInformation.EndOfFile.LowPart < 4)
			)
		{
			DbgPrint("Cannot Query File Size! %08X\n", Status);
			ZwClose(FileHandle);
			result = Status;
			return result;
		}
		//3��1 �ļ����ڽṹ���ܴ�С
		if (FileStInformation.EndOfFile.LowPart > (sizeof(SYS_BLACK_WHITE_DATA) - 4)	//����-4����Ϊȥ��KSPIN_LOCK  SpinLock�ĵ�ַ�ռ䣬ͼʡ�½����ӽ��ṹ��
			)
		{
			//���ﲻ���ܳ�����
			//���갴���ֵ��
			ByteOffset.LowPart = FileStInformation.EndOfFile.LowPart - (sizeof(SYS_BLACK_WHITE_DATA) - 4);
			ulLength = sizeof(SYS_BLACK_WHITE_DATA) - 4;
			FileStInformation.EndOfFile.LowPart = sizeof(SYS_BLACK_WHITE_DATA) - 4;
		}
		//4�� ��ȡ�ļ�
		Status = ZwReadFile(
			FileHandle,    // �ļ����
			NULL,          // �ź�״̬(һ��ΪNULL)
			NULL, NULL,    // ����
			&IoStatusBlock,// ���ܺ����Ĳ������
			pBuffer,       // �����ȡ���ݵĻ���
			ulLength,      // ��Ҫ��ȡ�ĳ���
			&ByteOffset,   // ��ȡ����ʼƫ��
			NULL);         // һ��ΪNULL
		//5�� �ͷž��
		ZwClose(FileHandle);
		if (!NT_SUCCESS(Status))
		{
			//��g_Drvmk_List�ṹ�����ͷ�
			g_Drvmk_List->ListNumber = 0;
			ExFreePool(g_Drvmk_List);
			result = Status;
			return result;
		}
		//6����������������g_Drvmk_List->ListNumber��ֵ
		g_Drvmk_List->ListNumber = (FileStInformation.EndOfFile.LowPart - 4) / sizeof(PE_HASH_DATA);
		result = STATUS_SUCCESS;
	}
	else
	{
		//���󷵻�
		result = STATUS_UNSUCCESSFUL;
	}
	return result;
}


//��ʼ�������������غͷ��н�����Ϣ�ģ�R3��R0������
VOID NTAPI Safe_Initialize_List()
{
	ULONG Tag = 0x206B6444;

	KeInitializeSpinLock(&g_request_list_lock);			//����g_can_check_hook_request_list_added_by_r3��g_request_list
	InitializeListHead(&g_can_check_hook_request_list_added_by_r3);
	InitializeListHead(&g_request_list);

	KeInitializeSpinLock(&g_SpinLock_wait_info_list);
	InitializeListHead(&g_wait_info_list.list);
	
	//����TextOutCache��ֵ������ݣ���������һ��·��ָ��xxx\\xxx\\xxx\\drvmk.dat
	//���������ĺڰ�����
	g_Drvmk_List = Safe_AllocBuff(NonPagedPool, sizeof(SYS_BLACK_WHITE_DATA), Tag);
	if (!g_Drvmk_List)
	{
		return NULL;
	}
	KeInitializeSpinLock(&g_Drvmk_List->SpinLock);	//����g_Drvmk_Listʹ��
}

//************************************     
// ��������: Safe_QueryDrvmkDataList     
// ����˵������ѯ�ڰ�����    
// IDA��ַ ��
// ��    �ߣ�Mr.M      
// �� �� ֵ: ULONG NTAPI         
// ��    ��: IN PPE_HASH_DATA In_QueryDrvmkData          [In]��ϣֵ
//************************************ 
ULONG NTAPI Safe_QueryDrvmkDataList(IN PPE_HASH_DATA In_QueryDrvmkData)
{
	KIRQL NewIrql;
	ULONG Index = 0;						//�±�����
	ULONG DrvmkListNumber = 0;				//�б��ܸ���
	ULONG Pass_Flag = 2;					//����ֵ��0���� 1���� 2���     Ĭ����2
	DrvmkListNumber = g_Drvmk_List->ListNumber;

	//����
	NewIrql = KfAcquireSpinLock(&g_Drvmk_List->SpinLock);
	//�ж���������
	if (DrvmkListNumber)
	{
		//ѭ�������Ƿ����
		for (Index = 0; Index < DrvmkListNumber; Index++)
		{
			if (RtlEqualMemory(In_QueryDrvmkData, &g_Drvmk_List->Pe_Hash_Data[Index], sizeof(PE_HASH_DATA) - sizeof(ULONG)))
			{
				//�ҵ�����ѭ��
				break;
			}
		}
		if (Index >= DrvmkListNumber)
		{
			//�Ҳ��� ʧ�ܷ���
			Pass_Flag = 2;
		}
		else
		{
			//����LoadDriver_Flag��ʶ���ض�Ӧ��ֵ
			if (g_Drvmk_List->Pe_Hash_Data[Index].LoadDriver_Flag)
			{
				//1��ʾ ���أ����Է���0
				Pass_Flag = 0;
			}
			else
			{
				//0��ʾ ���У����Է���1
				Pass_Flag = 1;
			}
		}
	}
	//����
	KfReleaseSpinLock(&g_Drvmk_List->SpinLock, NewIrql);
	return Pass_Flag;
}

//************************************     
// ��������: Safe_InsertDrvmkDataList     
// ����˵������Ӻڰ�����    
// IDA��ַ ��
// ��    �ߣ�Mr.M      
// �� �� ֵ: PVOID NTAPI         
// ��    ��: IN PPE_HASH_DATA In_InsertDrvmkData          [In]��ϣֵ
//************************************ 
PVOID NTAPI Safe_InsertDrvmkDataList(IN PPE_HASH_DATA In_InsertDrvmkData)
{
	KIRQL NewIrql;
	ULONG Index = 0;						//�±�����
	ULONG DrvmkListNumber = 0;				//�б��ܸ���
	DrvmkListNumber = g_Drvmk_List->ListNumber;
	//����
	NewIrql = KfAcquireSpinLock(&g_Drvmk_List->SpinLock);
	//�ж���������,���Ҹ��� <= 0x270E
	if (DrvmkListNumber <= DRVMKNUMBER)
	{
		//1����������  ��������+1 
		//2���Ѵ���    ����
		while (!RtlEqualMemory(In_InsertDrvmkData,&g_Drvmk_List->Pe_Hash_Data[Index], sizeof(PE_HASH_DATA)))
		{
			//�������µ�������Ϣ�Ͳ���
			if (Index >= DrvmkListNumber)
			{
				RtlCopyMemory(&g_Drvmk_List->Pe_Hash_Data[Index], In_InsertDrvmkData, sizeof(PE_HASH_DATA));
				//����++
				g_Drvmk_List->ListNumber++;
				break;
			}
			else
			{
				//����
				++Index;
			}
		}
	}
	//����
	KfReleaseSpinLock(&g_Drvmk_List->SpinLock, NewIrql);
}

//************************************     
// ��������: Safe_DeleteDrvmkDataList     
// ����˵����ɾ���ڰ�����    
// IDA��ַ ��
// ��    �ߣ�Mr.M      
// �� �� ֵ: PVOID NTAPI         
//************************************ 
PVOID NTAPI Safe_DeleteDrvmkDataList(PPE_HASH_DATA In_DeleteDrvmkData)
{
	KIRQL NewIrql;
	ULONG Index = 0;						//�±�����
	ULONG DrvmkListNumber = 0;				//�б��ܸ���
	DrvmkListNumber = g_Drvmk_List->ListNumber;
	//����
	NewIrql = KfAcquireSpinLock(&g_Drvmk_List->SpinLock);
	//�ж���������
	if (DrvmkListNumber)
	{
		for (Index = 0; Index < DrvmkListNumber; Index++)
		{
			if (RtlEqualMemory(In_DeleteDrvmkData, &g_Drvmk_List->Pe_Hash_Data[Index], sizeof(PE_HASH_DATA)))
			{
				//����˳����̵���Ϣ(��һ����ǰŲ)
				for (ULONG i = Index; i <= DrvmkListNumber; i++)
				{
					RtlCopyMemory(&g_Drvmk_List->Pe_Hash_Data[Index], &g_Drvmk_List->Pe_Hash_Data[Index + 1], sizeof(PE_HASH_DATA));
				}
				//�������̸���-1
				--g_Drvmk_List->ListNumber;
				break;
			}
		}
	}
	//����
	KfReleaseSpinLock(&g_Drvmk_List->SpinLock, NewIrql);
}