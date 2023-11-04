#include "VirtualMemoryDataList.h"

//************************************     
// ��������: Safe_DeleteVirtualMemoryDataList     
// ����˵����ɾ���ڴ���Ϣ    
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��    
// �� �� ֵ: PVOID
// ��    ��: IN HANDLE In_ProcessId             ��ǰ����ID   
//************************************  
PVOID NTAPI Safe_DeleteVirtualMemoryDataList(IN HANDLE In_ProcessId)
{
	KIRQL       NewIrql;
	ULONG	    result = NULL;
	ULONG       SumListNumber = 0;
	ULONG       UniqueProcessId = 0;
	ULONG       OuterIndex = 0;			//��Ȧѭ��������
	ULONG		InsideIndex = 0;		//��Ȧѭ��������
	ULONG       RunFlag = TRUE;
	//����
	NewIrql = KfAcquireSpinLock(&g_VirtualMemoryData_List->SpinLock);
	//��ȡ�ܸ���
	SumListNumber = g_VirtualMemoryData_List->ListNumber;
	//��Ȧ
	if (SumListNumber)
	{
		for (OuterIndex = 0; OuterIndex < SumListNumber; OuterIndex++)
		{
			UniqueProcessId = g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].UniqueProcessId;
			//������ͬID ˵�����ڣ�������Ȧ���Ҹ�PID��������е�ַ�ռ�
			if (UniqueProcessId == In_ProcessId)
			{
				//������ǰŲ
				for (ULONG i = SumListNumber; i <= SumListNumber; i++)
				{
					g_VirtualMemoryData_List->VirtualMmBuff[i] = g_VirtualMemoryData_List->VirtualMmBuff[i + 1];
				}
				//��Ȧ����-1
				g_VirtualMemoryData_List->ListNumber--;
				break;
			}
		}
	}
	//��Ȧ
	SumListNumber = g_VirtualMemoryData_List->ListNumber;
	if (SumListNumber)
	{
		//��ȡ��Ȧ����
		ULONG InsideListNumber = g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].ListNumber;
		for (InsideIndex = 0; InsideIndex < InsideListNumber; InsideIndex++)
		{
			//�ж�ProcessId
			if (g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].ProcessId[InsideIndex] == In_ProcessId)
			{
				//�ҵ��ˣ���ǰŲ
				for (ULONG i = InsideIndex; i <= InsideListNumber; i++)
				{
					g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].ProcessId[InsideIndex] = g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].ProcessId[InsideIndex + 1];
					g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].RegionSize[InsideIndex] = g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].RegionSize[InsideIndex + 1];
					g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].BaseAddress[InsideIndex] = g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].BaseAddress[InsideIndex + 1];
					//��Ȧ����-1
					g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].ListNumber--;
					//����ѭ������ִ����Ȧ��ѭ�� ->for (InsideIndex = 0; InsideIndex < InsideListNumber; InsideIndex++)
					break;
				}
			}
		}
	}
	//����
	KfReleaseSpinLock(&g_VirtualMemoryData_List->SpinLock, NewIrql);
}


//ɾ���ڴ���Ϣ
//WINDOWS_VERSION_XP��Win2K��Ч
PVOID NTAPI Safe_DeleteVirtualMemoryDataList_XP_WIN2K(IN HANDLE In_UniqueProcessId, IN HANDLE In_ProcessId, IN ULONG In_Esp, IN ULONG In_ExpandableStackBottom, IN ULONG In_ExpandableStackSize)
{
	KIRQL       NewIrql;
	ULONG	    result = NULL;
	ULONG       SumListNumber = 0;
	ULONG       UniqueProcessId = 0;
	ULONG       OuterIndex = 0;			//��Ȧѭ��������
	ULONG		InsideIndex = 0;		//��Ȧѭ��������
	ULONG       RunFlag = TRUE; 
	ULONG       v5 = (In_Esp & 0xFFFFF000) - In_ExpandableStackSize + 0x1000;
	//����
	NewIrql = KfAcquireSpinLock(&g_VirtualMemoryData_List->SpinLock);
	//��ȡ�ܸ���
	SumListNumber = g_VirtualMemoryData_List->ListNumber;
	if (SumListNumber)
	{
		//��Ȧѭ��������UniqueProcessId
		for (OuterIndex = 0; OuterIndex < SumListNumber; OuterIndex++)
		{
			UniqueProcessId = g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].UniqueProcessId;
			//�ҵ����˳�ѭ��
			if (In_UniqueProcessId == UniqueProcessId)
			{
				break;
			}
		}
		//�жϸ����Ƿ񳬱�
		if (OuterIndex < SumListNumber)
		{
			//��ȡ��Ȧ����
			ULONG InsideListNumber = g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].ListNumber;
			for (InsideIndex = 0; InsideIndex < InsideListNumber; InsideIndex++)
			{
				//�ж�In_ExpandableStackSize �� RegionSize��Χ��
				SIZE_T RegionSize = g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].RegionSize[InsideIndex];
				ULONG BaseAddress = g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].BaseAddress[InsideIndex];
				if ((BaseAddress <= In_Esp) && (BaseAddress + RegionSize >= In_Esp))
				{
					break;
				}
				if ((BaseAddress < In_ExpandableStackBottom + In_ExpandableStackSize) && (BaseAddress + RegionSize > In_ExpandableStackBottom))
				{
					break;
				}
				if ((BaseAddress < v5 + In_ExpandableStackSize) && (BaseAddress + RegionSize > v5))
				{
					break;
				}
			}
			//�жϸ����Ƿ񳬱�
			if (InsideIndex < InsideListNumber)
			{
				//�ҵ�����ǰŲ
				for (ULONG i = InsideIndex; i <= InsideListNumber; i++)
				{
					g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].ProcessId[InsideIndex] = g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].ProcessId[InsideIndex + 1];
					g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].RegionSize[InsideIndex] = g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].RegionSize[InsideIndex + 1];
					g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].BaseAddress[InsideIndex] = g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].BaseAddress[InsideIndex + 1];
					//��Ȧ����-1
					g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].ListNumber--;
					break;
				}
			}
		}
	}
	//����
	KfReleaseSpinLock(&g_VirtualMemoryData_List->SpinLock, NewIrql);
}

//************************************     
// ��������: Safe_InsertVirtualMemoryDataList     
// ����˵��������ڴ���Ϣ    
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��    
// �� �� ֵ: BOOLEAN   �ɹ�����1��ʧ�ܷ���0  
// ��    ��: IN PVOID In_BaseAddress			�����׵�ַ
// ��    ��: IN SIZE_T In_RegionSize			�����С
// ��    ��: IN HANDLE In_UniqueProcessId       ����ID      pPsGetProcessId
// ��    ��: IN HANDLE In_ProcessId             ��ǰ����ID  PsGetCurrentProcessId 
//************************************  
BOOLEAN Safe_InsertVirtualMemoryDataList(IN PVOID In_BaseAddress, IN SIZE_T In_RegionSize, IN HANDLE In_UniqueProcessId, IN HANDLE In_ProcessId)
{
	KIRQL       NewIrql;
	ULONG	    result = NULL;
	ULONG       SumListNumber = 0;
	ULONG       UniqueProcessId = 0;
	ULONG       RunFlag = TRUE;
	//����
	NewIrql = KfAcquireSpinLock(&g_VirtualMemoryData_List->SpinLock);
	//��ȡ�ܸ���
	SumListNumber = g_VirtualMemoryData_List->ListNumber;
	//1���ж��ǲ��Ǹ�PID�ǲ��ǵ�һ�ν���
	//������ͬPIDֱ��ʹ���������·���
	for (ULONG OuterIndex = 0; OuterIndex < SumListNumber; OuterIndex++)
	{
		UniqueProcessId = g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].UniqueProcessId;
		//��ͬPIDֱ����Ӽ���
		//ÿ�����̿��Է���ܶ���ڴ����ݣ��������0x62��
		if (UniqueProcessId == In_UniqueProcessId)
		{
			//��ȡ��Ȧ����
			ULONG InsideListNumber = g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].ListNumber;
			//�жϸ����Ƿ񳬱�
			if (InsideListNumber <= PIDMMNEWNUMBER)
			{
				g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].BaseAddress[InsideListNumber] = In_BaseAddress;			//����ռ��׵�ַ
				g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].RegionSize[InsideListNumber] = In_RegionSize;			//�����С
				g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].ProcessId[InsideListNumber] = In_ProcessId;				//PsGetCurrentProcessId()
				g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].ListNumber++;										   //��Ȧʹ�ø���+1
				//�ɹ�ִ��
				result = TRUE;
				RunFlag = FALSE;
				break;
			}
			else
			{
				//ʧ��ִ��
				result = FALSE;
				RunFlag = FALSE;
				break;
			}
		}
	}
	//İ��PID���������һ��
	if (RunFlag)
	{
		//3���жϸ����Ƿ񳬱�
		if (SumListNumber < PIDMMNUMBER)
		{
			//4����һ�ν���,��Ȼ�Ǵ��±�0��ʼ��
			g_VirtualMemoryData_List->VirtualMmBuff[SumListNumber].UniqueProcessId = In_UniqueProcessId;	//����ID
			g_VirtualMemoryData_List->VirtualMmBuff[SumListNumber].BaseAddress[0] = In_BaseAddress;			//����ռ��׵�ַ
			g_VirtualMemoryData_List->VirtualMmBuff[SumListNumber].RegionSize[0] = In_RegionSize;			//�����С
			g_VirtualMemoryData_List->VirtualMmBuff[SumListNumber].ProcessId[0] = In_ProcessId;				//PsGetCurrentProcessId()
			g_VirtualMemoryData_List->VirtualMmBuff[SumListNumber].ListNumber = 1;							//��Ȧʹ�ø���
			//��Ȧ����+1
			g_VirtualMemoryData_List[SumListNumber].ListNumber++;
			result = TRUE;
		}
		else
		{
			result = FALSE;
		}
	}
	//����
	KfReleaseSpinLock(&g_VirtualMemoryData_List->SpinLock, NewIrql);
	return result;
}

//************************************     
// ��������: Safe_QueryVirtualMemoryDataList     
// ����˵������ѯ�ڴ���Ϣ    
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��    
// �� �� ֵ: BOOLEAN   �ɹ�����1��ʧ�ܷ���0  
// ��    ��: IN PVOID In_BaseAddress			�����׵�ַ
// ��    ��: IN SIZE_T In_RegionSize			�����С
// ��    ��: IN HANDLE In_UniqueProcessId       ����ID      pPsGetProcessId
// ��    ��: IN HANDLE In_ProcessId             ��ǰ����ID  PsGetCurrentProcessId 
//************************************  
BOOLEAN Safe_QueryVirtualMemoryDataList(IN PVOID In_BaseAddress, IN SIZE_T In_RegionSize, IN HANDLE In_UniqueProcessId, IN HANDLE In_ProcessId)
{
	KIRQL       NewIrql;
	ULONG	    result = FALSE;
	ULONG       SumListNumber = 0;
	ULONG       UniqueProcessId = 0;
	ULONG       RunFlag = TRUE;
	ULONG       OuterIndex = 0;			//��Ȧѭ��������
	ULONG		InsideIndex = 0;		//��Ȧѭ��������

	//����
	NewIrql = KfAcquireSpinLock(&g_VirtualMemoryData_List->SpinLock);
	//��ȡ�ܸ���
	SumListNumber = g_VirtualMemoryData_List->ListNumber;
	//1���ж���������
	if (SumListNumber)
	{
		//������ͬPID�Ž�����Ȧ����
		for (ULONG OuterIndex = 0; OuterIndex < SumListNumber; OuterIndex++)
		{
			UniqueProcessId = g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].UniqueProcessId;
			//������ͬID ˵�����ڣ�������Ȧ���Ҹ�PID��������е�ַ�ռ�
			if (UniqueProcessId == In_UniqueProcessId)
			{
				//��ȡ��Ȧ�ܸ���
				ULONG InsideListNumberMax = g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].ListNumber;
				//������PID���з���ĵ�ַ�ռ�
				for (ULONG InsideIndex = InsideListNumberMax; InsideIndex < InsideListNumberMax; InsideIndex++)
				{
					//�ж�ProcessId
					if (g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].ProcessId[InsideIndex] == In_ProcessId)
					{
						ULONG BaseAddress = g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].BaseAddress[InsideIndex];
						ULONG RegionSize = g_VirtualMemoryData_List->VirtualMmBuff[OuterIndex].RegionSize[InsideIndex];
						//���ж��ڴ淶Χ �Ƿ���xxx֮��
						if (
							((ULONG)In_BaseAddress >= BaseAddress) &&
							(((ULONG)In_BaseAddress + In_RegionSize) <= (BaseAddress + RegionSize))
							)
						{
							//�ҵ��˾��˳�
							result = TRUE;
							goto _FunctionRet;
						}
					}
				}
			}
		}
	}
	_FunctionRet:
	//����
	KfReleaseSpinLock(&g_VirtualMemoryData_List->SpinLock, NewIrql);
	return result;
}