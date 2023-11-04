/*
�ο����ϣ�
1��ǳ��windows�������            
��ַ��https://bbs.pediy.com/thread-74430-1.htm
*/
#include "Fake_ZwMakeTemporaryObject.h"

#define WHILEDEVICENUMBER_ZWMAKETEMPORARYOBJECT 0xD
//Ҫ���صİ������豸����
PWCHAR g_WhiteDeviceName_ZwMakeTemporaryObject[WHILEDEVICENUMBER_ZWMAKETEMPORARYOBJECT + 1] = {
	L"\\Device\\360AntiHacker",
	L"\\Device\\360Camera",
	L"\\Device\\360HookPort",
	L"\\Device\\360SelfProtection",
	L"\\Device\\360SearchHotkey",
	L"\\Device\\360SpShadow0",
	L"\\Device\\360TdiFilter",
	L"\\Device\\BAPI",
	L"\\Device\\DsArk",
	L"\\Device\\DsArk",
	L"\\Device\\qutmipc",
	L"\\FileSystem\\Filters\\qutmdrv",			//sFilters
	L"\\Device\\360AntiHijack",
};


//************************************     
// ��������: Fake_ZwMakeTemporaryObject     
// ����˵������ֹ����ɾ���ܱ��������ö���  
//           ��������ɾ��һ���־ö����Ωһ����, ��ͨ������ZwMakeTemporaryObject����, ������ת��Ϊ��ʱ����
//           InitializeObjectAttributes��������OBJ_PERMANENT��ʶ�������������ö���
//           ɾ��һ�������ԵĶ���, ��Ҫ�������в���:
//           1����ObDereferenceObject ��һ�������ԵĶ�������ü������ٵ�0
//           2����ZwOpenXxx orZwCreateXxx ����ø������Զ����һ�����
//           3��þ�������ZwMakeTemporaryObject ��һ�������ԵĶ���ת����һ����ʱ�Ķ���
//           4�õõ��ľ������ZwClose ɾ���ö���
//           ����ȷ��һ����������ʱ�ĺ����õ���InitializeObjectAttributes ���Attributes ����
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �� �� ֵ: NTSTATUS  NTAPI     
// ��    ��: IN ULONG  CallIndex     
// ��    ��: IN PVOID  ArgArray     
// ��    ��: IN PULONG ret_func     
// ��    ��: IN PULONG ret_arg     
//************************************  
NTSTATUS NTAPI Fake_ZwMakeTemporaryObject(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS       result = STATUS_SUCCESS;
	NTSTATUS       Status = STATUS_SUCCESS;
	PVOID		   Object = NULL;
	HANDLE         Out_Handle = NULL;
	UNICODE_STRING OutDestinationString = { 0 };					//Safe_ZwQuerySymbolicLinkObject�����Ĳ��ҵķ�������·��
	UNICODE_STRING TempString1 = { 0 };
	BOOLEAN        ZwClose_Flag = FALSE;       
	//0����ȡZwMakeTemporaryObjectԭʼ����
	HANDLE  In_Handle = *(ULONG*)((ULONG)ArgArray);
	//1��������Ӧ�ò����
	if (!ExGetPreviousMode())
	{
		return result;
	}
	//2�����˵���SymbolicLink����
	if (!Safe_QueryObjectType(In_Handle, L"SymbolicLink"))
	{
		return result;
	}
	//3���������Ǳ������̣�ֱ�ӷ���
	//   �����߷Ǳ������̣���ֹɾ���ܱ�������
	if (Safe_QueryWhitePID(PsGetCurrentProcessId()))
	{
		return result;
	}
	//4��Win7����Win7+����
	if (g_HighgVersionFlag)
	{
		Status = ObReferenceObjectByHandle(In_Handle, NULL, NULL, UserMode, &Object, NULL);
		if (!NT_SUCCESS(Status))
		{
			return result;
		}
		Status = ObOpenObjectByPointer(Object, OBJ_KERNEL_HANDLE, NULL, NULL, NULL, KernelMode, &Out_Handle);
		ObfDereferenceObject(Object);
		if (NT_SUCCESS(Status))
		{
			//��־λ��1����ʾIn_HANDLE��ObOpenObjectByPointer���ص�
			ZwClose_Flag = TRUE;
			In_Handle = Out_Handle;
		}
		else
		{
			return result;
		}
	}
	//5�����ݾ���ҵ���Ӧ�ķ�������·��
	if (!Safe_ZwQuerySymbolicLinkObject(In_Handle, &OutDestinationString))
	{
		//�Ҳ�����������·��ֱ�ӷ���
		return result;
	}
	//�ͷ�ObOpenObjectByPointer��ȡ�ľ��
	if (ZwClose_Flag)
	{
		ZwClose_Flag = FALSE;
		ZwClose(Out_Handle);
		Out_Handle = NULL;
	}
	//6����ֹ����ɾ���ܱ����Ķ���
	for (ULONG i = 0; i < WHILEDEVICENUMBER_ZWMAKETEMPORARYOBJECT; i++)
	{
		//��ΪDriverName��UUNICODE_STRING������������Ҫת����
		RtlInitUnicodeString(&TempString1, g_WhiteDeviceName_ZwMakeTemporaryObject[i]);
		if (RtlEqualUnicodeString(&OutDestinationString, &TempString1, TRUE))
		{
			//�ҵ��˴��󷵻�
			result = STATUS_ACCESS_DENIED;
			break;
		}
	}
	//�ǵ��ͷŷ���ռ�
	ExFreePool(OutDestinationString.Buffer);
	return result;
}