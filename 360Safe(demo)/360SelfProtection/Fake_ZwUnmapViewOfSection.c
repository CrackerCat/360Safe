/*
˵����
���ܽ��̾���ʹ��ZwUnmapViewOfSection�������
*/
#include "Fake_ZwUnmapViewOfSection.h"

//ȡ��ӳ��Ŀ����̵��ڴ�
NTSTATUS NTAPI Fake_ZwUnmapViewOfSection(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS    Status, result;
	PROCESS_BASIC_INFORMATION PBI = { 0 };
	ULONG ReturnLength = NULL;
	result = STATUS_SUCCESS;
	//0����ȡZwUnmapViewOfSection����
	HANDLE In_ProcessHandle = *(ULONG*)((ULONG)ArgArray);
	//1��������Ӧ�ò����
	if (ExGetPreviousMode())
	{
		//������ȡ��ӳ����ǰ���������ֱ�Ӵ��󷵻أ���֪ͨ�û����ػ��Ƿ���
		if ((In_ProcessHandle != NtCurrentProcess()) &&
			(!Safe_QueryWhitePID(PsGetCurrentProcessId())) &&			//�ж��ǲ��ǰ��������ã�����Ƿ��� ���Ǽ����ж�		
			(NT_SUCCESS(Safe_ZwQueryInformationProcess(In_ProcessHandle, ProcessBasicInformation, &PBI, sizeof(PROCESS_BASIC_INFORMATION), &ReturnLength))) &&	//��ȡ����PID
			Safe_QueryWhitePID(PBI.UniqueProcessId)						//�ж�Ҫ������PID�ǲ��ǰ���������������� ���Ƿ���
			)
		{
			//�������ػ��Ƿ���
			Safe_18A72_SendR3(PsGetCurrentProcessId(), PsGetCurrentThreadId(), 2);
			//ʧ�ܷ���
			result = STATUS_ACCESS_DENIED;
		}
		else
		{
			result = STATUS_SUCCESS;
		}
	}
	return result;
}