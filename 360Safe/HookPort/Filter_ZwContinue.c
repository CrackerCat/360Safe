#include "Filter_ZwContinue.h"

//���û�ж�Ӧ��Fake����
NTSTATUS NTAPI Filter_ZwContinue(PCONTEXT Context, BOOLEAN TestAlert)
{
	NTSTATUS Result, OutResult;
	PVOID	 pArgArray = &Context;//�������飬ָ��ջ�����ڱ����������в���

	//HOOKPORT_DEBUG_PRINT(HOOKPORT_DISPLAY_INFO, "Filter_ZwContinue");
	NTSTATUS(NTAPI *ZwContinuePtr)(PCONTEXT, BOOLEAN);
	Result = HookPort_DoFilter(ZwContinue_FilterIndex, pArgArray, 0, 0, 0, &OutResult);
	if (Result)
	{
		//��ȡԭʼ������ַ
		ZwContinuePtr = HookPort_GetOriginalServiceRoutine(g_SSDT_Func_Index_Data.ZwContinueIndex);

		//����ԭʼ����
		Result = ZwContinuePtr(Context, TestAlert);
	}
	return Result;
}