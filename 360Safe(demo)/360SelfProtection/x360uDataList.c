/*
˵��:
1��ÿ��SAFEMONPATH_DIRECTORY�ṹ(����·��)��Ӧһ��SAFEMONDATA_DIRECTORY(��·�����̵���ϸ��Ϣ)�ṹ
2��һ������·����Ϣ��һ�������·�����̵�EPROCESS��PID���ȵ���Ϣ
3����·���洢��������ȫĿ¼�����Ǹ�Ŀ¼����򿪾͸�����ɫͨ��
*/
#include "x360uDataList.h"


//��ѯ��DosPath�Ƿ����б��У�����Ƿ���Index
ULONG NTAPI Safe_QuerSafeMonPathList(IN PWCHAR DosPath, OUT ULONG ret_arg)
{
	KIRQL       NewIrql;
	ULONG	    result = NULL;
	ULONG       Index = 0;
	ULONG       ListNumber = NULL;
	PVOID       pListAddr = NULL;			//ָ��g_SafeMonPath_List->DosPath  ��ά�����׵�ַ
	//����
	NewIrql = KfAcquireSpinLock(&g_SafeMonPath_List->SpinLock);
	ListNumber = g_SafeMonPath_List->ListNumber;
	pListAddr = &g_SafeMonPath_List->DosPath;
	//�ж���������
	if (ListNumber)
	{
		for (Index = 0; Index < ListNumber; Index++)
		{
			//�ж�dos·���Ƿ����б���
			if (_wcsicmp(DosPath, pListAddr) == 0)
			{
				//�ҵ����˳�
				break;
			}
			//�������ƫ�Ƶ���һ��
			pListAddr = ((ULONG)pListAddr + DOSPATHSIZE);
		}
		if (Index >= ListNumber)
		{
			//���󷵻�
			result = 0;
		}
		else
		{
			if (ret_arg)
			{
				*(ULONG*)ret_arg = g_SafeMonPath_List->ArrayIndex[Index];			//��DosPath·���������е��±���������ʵ��ֱ�ӷ���*(ULONG*)ret_arg = Indexͬ��
			}
			result = 1;
		}
	}
	else
	{
		result = 0;
	}
	//����
	KfReleaseSpinLock(&g_SafeMonPath_List->SpinLock, NewIrql);
	return result;
}

//ɾ������������Ϣ
//�ɹ����ض�Ӧ���±꣬ʧ�ܷ���0
ULONG NTAPI Safe_DeleteSafeMonDataList(_In_ HANDLE SafeMonSectionHandle)
{
	ULONG	  result = NULL;
	PEPROCESS Process = NULL;
	PEPROCESS SafeMonProcess = NULL;
	NTSTATUS  Status = NULL;
	PVOID     SectionObject = NULL;
	KIRQL     NewIrql;
	ULONG     ListNumber = NULL;
	Process = IoGetCurrentProcess();
	//1����ȡSectionObject
	Status = ObReferenceObjectByHandle(SafeMonSectionHandle, 0, MmSectionObjectType, UserMode, &SectionObject, 0);
	if (!NT_SUCCESS(Status))
	{
		result = FALSE;
		return result;
	}
	//�������ü���
	ObDereferenceObject(SectionObject);
	//����
	NewIrql = KfAcquireSpinLock(&g_SafeMonData_List->SpinLock);
	ListNumber = g_SafeMonData_List->ListNumber;
	//�ж���������
	if (ListNumber)
	{
		for (ULONG Index = 0; Index < ListNumber; Index++)
		{
			//�ҵ���ɾ��
			if (g_SafeMonData_List->SafeMonSectionHandle[Index] == SafeMonSectionHandle &&
				g_SafeMonData_List->SafeMonSectionObject[Index] == SectionObject &&
				g_SafeMonData_List->SafeMonProcess[Index] == SafeMonProcess)
			{
				//���浱ǰɾ���������±�
				result = g_SafeMonData_List->SafeMonIndex[Index];													//��ʼƫ��4003*4,����SafeMon,���Ҹ�dos·�����б�ڼ��ret_arg = ���������±�
				//����˳����̵���Ϣ(��һ����ǰŲ)
				for (ULONG i = Index; i <= ListNumber; i++)
				{
					g_SafeMonData_List->SafeMonSectionHandle[i] = g_SafeMonData_List->SafeMonSectionHandle[i + 1]; //��ʼƫ��0000*4,����SafeMod��SectionHandle
					g_SafeMonData_List->SafeMonSectionObject[i] = g_SafeMonData_List->SafeMonSectionObject[i + 1]; //��ʼƫ��2002*4,����SectionObject
					g_SafeMonData_List->SafeMonIndex[i]         = g_SafeMonData_List->SafeMonIndex[i + 1];		   //��ʼƫ��4003*4,����SafeMon,���Ҹ�dos·�����б�ڼ��ret_arg = ���������±�
					g_SafeMonData_List->SafeMonProcess[i]       = g_SafeMonData_List->SafeMonProcess[i + 1];	   //��ʼƫ��6001*4,����SafeMod��Eprocess�ṹ
				}
				//�������̸���-1
				--g_SafeMonData_List->ListNumber;
				break;
			}
		}
	}
	//����
	KfReleaseSpinLock(&g_SafeMonData_List->SpinLock, NewIrql);
	return result;
}

//�������������Ϣ
// �ɹ�����1��ʧ�ܷ���0
BOOLEAN Safe_InsertSafeMonDataList(_In_ HANDLE SafeMonSectionHandle, _In_ ULONG SafeMonIndex)
{
	PEPROCESS SafeMonProcess = NULL;
	NTSTATUS  Status = NULL;
	BOOLEAN   result = TRUE;
	PVOID     SectionObject = NULL;
	KIRQL     NewIrql;
	ULONG     ListNumber = NULL;
	ULONG     Index = NULL;								//ѭ��������������int i=0 ;i++
	SafeMonProcess = IoGetCurrentProcess();
	//1����ȡSectionObject
	Status = ObReferenceObjectByHandle(SafeMonSectionHandle, 0, MmSectionObjectType, UserMode, &SectionObject, 0);
	if (!NT_SUCCESS(Status))
	{
		result = FALSE;
		return result;
	}
	//�������ü���
	ObDereferenceObject(SectionObject);
	//����
	NewIrql = KfAcquireSpinLock(&g_SafeMonData_List->SpinLock);
	ListNumber = g_SafeMonData_List->ListNumber;
	//�ж���������
	if (ListNumber)
	{
		//�����ڲ����
		while (g_SafeMonData_List->SafeMonSectionHandle[Index] != SafeMonSectionHandle &&
			g_SafeMonData_List->SafeMonSectionObject[Index] != SectionObject &&
			g_SafeMonData_List->SafeMonProcess[Index] != SafeMonProcess
			)
		{
			//�������µ���Ϣ�Ͳ���
			if (Index++ >= ListNumber)
			{
				//�������̸���<=0x7CE
				if (ListNumber <= SAFEMODMMDATALISTNUMBER)
				{
					g_SafeMonData_List->SafeMonSectionHandle[ListNumber] = SafeMonSectionHandle;
					g_SafeMonData_List->SafeMonSectionObject[ListNumber] = SectionObject;
					g_SafeMonData_List->SafeMonProcess[ListNumber] = SafeMonProcess;
					g_SafeMonData_List->SafeMonIndex[ListNumber] = SafeMonIndex;
					g_SafeMonData_List->ListNumber++;
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

		}
	}
	//����
	KfReleaseSpinLock(&g_SafeMonData_List->SpinLock, NewIrql);
	return result;
}