#include "Fake_ZwSuspendThread.h"


//�̹߳���
NTSTATUS NTAPI Fake_ZwSuspendThread(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS    result;
	result = STATUS_SUCCESS;
	//0����ȡZwSuspendProcessԭʼ����
	IN HANDLE   In_ThreadHandle = *(ULONG*)((ULONG)ArgArray);
	//1���ں�ģʽ�������Ǳ�������ֱ�ӷ���
	if (!ExGetPreviousMode() || (Safe_QueryWhitePID(PsGetCurrentProcessId())))
	{
		result = STATUS_SUCCESS;
	}
	else
	{
		//2���û�ģʽ
		//2��1������
		if (myProbeRead(In_ThreadHandle, sizeof(ULONG), sizeof(CHAR)))
		{
			KdPrint(("ProbeRead(Fake_ZwSuspendThread��In_ThreadHandle) error \r\n"));
			return result;
		}
		//�����̻߳�ȡPID
		//�����Ǵ򿪱�������ֱ�ӷ��ش���ֵ
		result = Safe_QueryWintePID_ThreadHandle(In_ThreadHandle) != 0 ? STATUS_ACCESS_DENIED : STATUS_SUCCESS;
	}
	return result;
}

