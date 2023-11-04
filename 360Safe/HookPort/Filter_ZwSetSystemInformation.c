#include "Filter_ZwSetSystemInformation.h"

NTSTATUS NTAPI Filter_ZwSetSystemInformation(IN SYSTEM_INFORMATION_CLASS  SystemInformationClass,IN OUT PVOID  SystemInformation,IN ULONG  SystemInformationLength)
{
	NTSTATUS Result = STATUS_SUCCESS;
	NTSTATUS OutResult = STATUS_SUCCESS;

	PULONG   FuncTable[16] = { 0 };
	PULONG   ArgTable[16] = { 0 };
	ULONG    RetNumber = NULL;							//һ���ж�����Fake����
	PVOID	 pArgArray = &SystemInformationClass;		//�������飬ָ��ջ�����ڱ����������в���
	NTSTATUS(NTAPI *ZwSetSystemInformationPtr)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG);
	//ԭʼ����ִ��ǰ���
	Result = HookPort_DoFilter(ZwCreateFile_FilterIndex, pArgArray, FuncTable, ArgTable, &RetNumber, &OutResult);
	if (Result)
	{
		//��ȡԭʼ������ַ
		ZwSetSystemInformationPtr = HookPort_GetOriginalServiceRoutine(g_SSDT_Func_Index_Data.ZwCreateFileIndex);

		//����ԭʼ����
		Result = ZwSetSystemInformationPtr(SystemInformationClass, SystemInformation, SystemInformationLength);
		if (NT_SUCCESS(Result))
		{
			//ԭʼ����ִ�к���
			Result = HookPort_ForRunFuncTable(ZwCreateFile_FilterIndex, pArgArray, Result, FuncTable, ArgTable, RetNumber);
		}
	}
	else
	{
		Result = OutResult;
	}
	return Result;
}