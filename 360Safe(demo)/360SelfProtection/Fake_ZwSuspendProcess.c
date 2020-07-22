#include "Fake_ZwSuspendProcess.h"


//���̹���
NTSTATUS NTAPI Fake_ZwSuspendProcess(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS    result;
	result = STATUS_SUCCESS;
	//0����ȡZwSuspendProcessԭʼ����
	IN HANDLE   In_ProcessHandle = *(ULONG*)((ULONG)ArgArray);
	//1���ں�ģʽ�������Ǳ�������ֱ�ӷ���
	if (!ExGetPreviousMode() || (Safe_QueryWhitePID(PsGetCurrentProcessId())))
	{
		result = STATUS_SUCCESS;
	}
	else
	{
		//2���û�ģʽ
		//2��1������
		if (myProbeRead(In_ProcessHandle, sizeof(ULONG), sizeof(CHAR)))
		{
			KdPrint(("ProbeRead(Fake_ZwSuspendProcess��In_ProcessHandle) error \r\n"));
			return result;
		}
		//�����Ǵ򿪱�������ֱ�ӷ��ش���ֵ
		result = Safe_QueryWintePID_ProcessHandle(In_ProcessHandle) != 0 ? STATUS_ACCESS_DENIED : STATUS_SUCCESS;
	}
	return result;
}