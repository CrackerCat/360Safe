#include "Fake_ZwAlpcSendWaitReceivePort.h"

//RPCͨѶ�ڸ���ƽ̨��������API������ͬ��������
//win2000 : NtFsControlFile
//xp, 2003 : NtRequestWaitReplyPort
//vista, 2008.win7 : NtAlpcSendWaitReceivePort
NTSTATUS NTAPI Fake_ZwAlpcSendWaitReceivePort(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS	Result = STATUS_SUCCESS;
	//0����ȡZwAllocateVirtualMemoryԭʼ����
	HANDLE In_PortHandle = *(ULONG*)((ULONG)ArgArray);
	PVOID  In_SendMessage = *(ULONG*)((ULONG)ArgArray + 8);
	//1��������Ӧ�ò����
	if (ExGetPreviousMode())
	{
		Result = Safe_RPCDispatcher(In_SendMessage, In_PortHandle);
	}
	return Result;
}