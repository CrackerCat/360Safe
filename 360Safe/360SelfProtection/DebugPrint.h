#pragma once
#include <ntddk.h>


//Hookʧ����ע���д��ʧ�ܱ��

NTSTATUS NTAPI HookPort_RtlWriteRegistryValue(CHAR ValueData);




