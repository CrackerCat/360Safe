#pragma once
#include <ntifs.h>
#include "Data.h"
#include "ZwNtFunc.h"


NTSTATUS NTAPI Safe_KernelCreateFile(IN PANSI_STRING SymbolName, OUT PSYSTEM_INFORMATIONFILE_XOR System_Information);

//��ȡ�ļ��Ļ�����Ϣ���������ļ���ϢУ��֮��ģ�������������У��
//һ����ļ�ʱ�����ñ���һ��ԭʼ��Ϣ�����ڶ��ε��ü��
NTSTATUS NTAPI Safe_GetInformationFile(IN HANDLE Handle, OUT PSYSTEM_INFORMATIONFILE_XOR System_Information, IN KPROCESSOR_MODE AccessMode);
