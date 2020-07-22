#include "SSDT.h"

//************************************     
// ��������: SafePort_GetSSDTTableAddress     
// ����˵������ȡSSDT��ַ    
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2020/03/23     
// �� �� ֵ: NTSTATUS NTAPI     
// ��    ��: OUT PVOID * SSDT_KeServiceTableBase     //[Out]SSDT_KeServiceTableBase
// ��    ��: OUT ULONG * SSDT_KeNumberOfServices     //[Out]SSDT_KeNumberOfServices
// ��    ��: OUT PVOID * SSDT_KeParamTableBase       //[Out]SSDT_KeParamTableBase
// ��    ��: IN PVOID * NtImageBase					 //[In]Nt�ں˵Ļ���ַ
//************************************  
NTSTATUS NTAPI Safe_GetSSDTTableAddress(OUT PVOID* SSDT_KeServiceTableBase, OUT ULONG* SSDT_KeNumberOfServices, OUT PVOID* SSDT_KeParamTableBase, IN PVOID* NtImageBase)
{
	NTSTATUS                     Status = STATUS_UNSUCCESSFUL;
	ANSI_STRING					 KeServiceDescriptorTableString;
	PCHAR						 SymbolAddr = NULL;
	PServiceDescriptorTableEntry KeServiceDescriptorTable = NULL;
	//3����ȡSSDT��ַ
	RtlInitAnsiString(&KeServiceDescriptorTableString, "KeServiceDescriptorTable");
	SymbolAddr = Safe_GetSymbolAddress(&KeServiceDescriptorTableString, NtImageBase);
	if (SymbolAddr)
	{
		KeServiceDescriptorTable = (PServiceDescriptorTableEntry)SymbolAddr;
		*SSDT_KeServiceTableBase = KeServiceDescriptorTable->ServiceTableBase;
		*SSDT_KeNumberOfServices = KeServiceDescriptorTable->NumberOfServices;
		*SSDT_KeParamTableBase = KeServiceDescriptorTable->ParamTableBase;
		if (!KeServiceDescriptorTable && MmIsAddressValid(KeServiceDescriptorTable))
		{
			HookPort_RtlWriteRegistryValue(3);
			return STATUS_UNSUCCESSFUL;
		}
		if (!*SSDT_KeNumberOfServices)
		{
			HookPort_RtlWriteRegistryValue(5);
			return STATUS_UNSUCCESSFUL;
		}
		if (!*SSDT_KeServiceTableBase || !*SSDT_KeParamTableBase)
		{
			HookPort_RtlWriteRegistryValue(6);
			return STATUS_UNSUCCESSFUL;
		}
		Status = STATUS_SUCCESS;
	}
	else
	{
		//����SSDT��ʧ��
		Status = STATUS_UNSUCCESSFUL;
	}
	return Status;
}