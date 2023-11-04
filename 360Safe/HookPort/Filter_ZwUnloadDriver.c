#include "Filter_ZwUnloadDriver.h"

NTSTATUS NTAPI Filter_ZwUnloadDriver(IN PUNICODE_STRING  DriverServiceName)
{
	NTSTATUS Result, OutResult;
	PULONG   FuncTable[16] = { 0 };
	PULONG   ArgTable[16] = { 0 };
	ULONG    RetNumber = NULL;
	PVOID    pArgArray = &DriverServiceName;//�������飬ָ��ջ�����ڱ����������в���
	NTSTATUS(NTAPI *ZwUnloadDriverPtr)(PUNICODE_STRING);
	//KdPrint(("Filter_ZwUnloadDriver\t\n"));
	Result = HookPort_DoFilter(ZwLoad_Un_Driver_FilterIndex, pArgArray, FuncTable, ArgTable, &RetNumber, &OutResult);
	if (Result)
	{
		//��ȡԭʼ������ַ
		ZwUnloadDriverPtr = HookPort_GetOriginalServiceRoutine(g_SSDT_Func_Index_Data.ZwUnloadDriverIndex);
		//����ԭʼ����
		Result = ZwUnloadDriverPtr(DriverServiceName);
		if (NT_SUCCESS(Result))
		{
			//HookPort_GetOriginalServiceRoutine(ZwLoad_Un_Driver_FilterIndex);
			Result = HookPort_ForRunFuncTable(ZwLoad_Un_Driver_FilterIndex, pArgArray, Result, FuncTable, ArgTable, RetNumber);
		}
	}
	else
	{
		Result = OutResult;
	}
	return Result;
}