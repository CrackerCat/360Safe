#include "CreateProcessDataList.h"

//����������
BOOLEAN Safe_InsertCreateProcessDataList(IN PEPROCESS Process, IN ULONG SafeModArrayIndex)
{
	KIRQL NewIrql;
	NTSTATUS	status, result;
	ULONG Index = 0;						//�±�����
	ULONG GotoFalg;							//����ͬgoto���õ�Falg
	ULONG CreateProcessListNumber = 0;
	CreateProcessListNumber = g_CreateProcessData_List.CreateProcessListNumber;
	result = TRUE;
	NewIrql = KfAcquireSpinLock(&g_CreateProcessData_List.SpinLock);
	//1����������  ��������+1���ɹ�����TRUE������<=0x4FE����ʧ��FALSE������>0x4FE��
	while (Process != (ULONG)g_CreateProcessData_List.Eprocess[Index])
	{
		//�������µ���Ϣ�Ͳ���
		if (Index >= CreateProcessListNumber)
		{
			//���̸���<=0x4FE
			if (CreateProcessListNumber <= CREATEPROCESSNUMBER)
			{
				g_CreateProcessData_List.Eprocess[CreateProcessListNumber] = Process;
				g_CreateProcessData_List.ArrayIndex[CreateProcessListNumber] = SafeModArrayIndex;
				//��������1
				g_CreateProcessData_List.CreateProcessListNumber++;
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
	KfReleaseSpinLock(&g_CreateProcessData_List.SpinLock, NewIrql);
	return result;
}

//�ж��ǲ�����������Eprocess
//1������ǣ�������������Ϣ��������Ĩ��
//2��������ǣ�ֱ���˳�
//����ֵ��ɾ����SafeMon���Ҹ�dos·�����б�ڼ��ret_arg = ���������±�	
ULONG Safe_DeleteCreateProcessDataList(_In_ PEPROCESS Process)
{
	KIRQL NewIrql;
	ULONG Index = 0;						//�±�����
	ULONG SafeModArrayIndex = 0;
	ULONG CreateProcessListNumber = 0;
	CreateProcessListNumber = g_CreateProcessData_List.CreateProcessListNumber;
	//����
	NewIrql = KfAcquireSpinLock(&g_CreateProcessData_List.SpinLock);
	//�ж���������
	if (CreateProcessListNumber)
	{
		for (ULONG Index = 0; Index < CreateProcessListNumber; Index++)
		{
			//�ڱ����ҵ���Ӧ��EProcess��Ҫɾ���ģ�
			if ((ULONG)Process == g_CreateProcessData_List.ArrayIndex[Index])
			{
				//��ȡSafeMon���Ҹ�dos·�����б�ڼ��ret_arg = ���������±�	
				SafeModArrayIndex = g_CreateProcessData_List.ArrayIndex[Index];
				//����˳����̵���Ϣ(��һ����ǰŲ)
				for (ULONG i = Index; i < CreateProcessListNumber; i++)
				{
					g_CreateProcessData_List.Eprocess[i] = g_CreateProcessData_List.Eprocess[i + 1];			//����Eprocess
					g_CreateProcessData_List.ArrayIndex[i] = g_CreateProcessData_List.ArrayIndex[i + 1];		//SafeMon���Ҹ�dos·�����б�ڼ��ret_arg = ���������±�	
				}
				//������Ϣ����-1
				--g_CreateProcessData_List.CreateProcessListNumber;
				break;
			}
			else
			{
				SafeModArrayIndex = 0;
				break;
			}
		}
	}
	//����
	KfReleaseSpinLock(&g_CreateProcessData_List.SpinLock, NewIrql);
	return SafeModArrayIndex;
}