#pragma once
#include <ntifs.h>
#include <ntdef.h>
#include "WhiteList.h"
#include "Regedit.h"
#include "SystemProcessDataList.h"


//����ϸ������������ͣ�File��Process��Section��Thread���в���
//File��   Υ����������ʾ����ָ���İ�������������
//Process��Υ����������ʾ����ָ���İ����������������IE
//Section��Υ�������·����\\Device\\PhysicalMemory��\\KnownDlls\\ 
//Thread�� Υ����������ʾ����ָ���İ����������������IE
NTSTATUS NTAPI Safe_26C42(IN HANDLE In_SourceHandle, IN ULONG In_Options, IN ACCESS_MASK In_DesiredAccess, IN HANDLE In_TargetProcessHandle, IN HANDLE In_SourceProcessHandle);

//�ж�DuplicateObject����ִ�к�Ĵ�����
BOOLEAN NTAPI CheckResult_After_DuplicateObject(NTSTATUS In_Status);

//����ļ�����ָ���ǲ��Ƿ�ҳ�ļ�
BOOLEAN NTAPI Safe_RunFsRtlIsPagingFile(IN PFILE_OBJECT In_FileObject);

//û����
BOOLEAN NTAPI Safe_26794(IN HANDLE In_TargetProcessHandle);

//��ѯע���HIVELIST
BOOLEAN NTAPI Safe_QuerHivelist(IN ACCESS_MASK GrantedAccess, IN HANDLE In_SourceHandle, IN HANDLE In_SourceProcessHandle);

//���ƾ��
NTSTATUS NTAPI Fake_ZwDuplicateObject(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg);