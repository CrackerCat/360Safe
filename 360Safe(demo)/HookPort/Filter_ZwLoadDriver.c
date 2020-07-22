#include "Filter_ZwLoadDriver.h"

NTSTATUS NTAPI Filter_ZwLoadDriver(IN PUNICODE_STRING  DriverServiceName)
{
	NTSTATUS Result, OutResult;

	PULONG   FuncTable[16] = { 0 };
	PULONG   ArgTable[16] = { 0 };
	ULONG    RetNumber = NULL;
	PVOID    pArgArray = &DriverServiceName;//�������飬ָ��ջ�����ڱ����������в���
	NTSTATUS(NTAPI *ZwLoadDriverPtr)(PUNICODE_STRING);
	//KdPrint(("Filter_ZwLoadDriver\t\n"));
	Result = HookPort_DoFilter(ZwLoad_Un_Driver_FilterIndex, pArgArray, FuncTable, ArgTable, &RetNumber, &OutResult);
	if (Result)
	{
		//��ȡԭʼ������ַ
		ZwLoadDriverPtr = HookPort_GetOriginalServiceRoutine(g_SSDT_Func_Index_Data.ZwLoadDriverIndex);
		//����ԭʼ����
		Result = ZwLoadDriverPtr(DriverServiceName);
		if (NT_SUCCESS(Result))
		{
			Result = HookPort_ForRunFuncTable(ZwLoad_Un_Driver_FilterIndex, pArgArray, Result, FuncTable, ArgTable, RetNumber);
		}
	}
	else
	{
		Result = OutResult;
	}
	return Result;
}