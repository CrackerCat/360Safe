#pragma once
#include <ntifs.h>
#include "Data.h"
#include "WinKernel.h"
#include "WhiteList.h"
#include "DrvmkDataList.h"




NTSTATUS(NTAPI *pIoGetRequestorSessionId)(PIRP, PULONG);

PVOID NTAPI Safe_check_irp_request_in_list();

//�˶�IRP��SessionId == ��ǰ���̻SessionId
BOOLEAN NTAPI Safe_is_irp_reqeust_from_local(PIRP Irp_a1);

ULONG NTAPI Safe_push_request_in_and_waitfor_finish(IN PQUERY_PASS_R0SENDR3_DATA In_pBuff, IN ULONG In_Flag);


//���δ�������ֲ�֪��ȡʲô
//����������ʾ��Ӧ�ò�ͨѶ����
PVOID NTAPI Safe_18A72_SendR3(IN HANDLE In_PorcessID, IN HANDLE In_ThreadID, IN PROCESSINFOCLASS  ProcessInformationClass);

//���δ�������ֲ�֪��ȡʲô
//����������ʾ��Ӧ�ò�ͨѶ����
NTSTATUS NTAPI Safe_1D044_SendR3(IN HANDLE In_PorcessID, IN HANDLE In_ThreadID, IN ULONG In_Flag, IN PUNICODE_STRING In_ImagePathString);

//��������Ϸ���
NTSTATUS NTAPI Safe_CheckSys_SignatureOrHash(IN HANDLE In_PorcessID, IN HANDLE In_ThreadID, IN PUNICODE_STRING In_pImagePathString, OUT PVOID Out_Hash, OUT ULONG Out_FileSize, OUT ULONG Out_PassFlag, IN ULONG In_Flag_a6);