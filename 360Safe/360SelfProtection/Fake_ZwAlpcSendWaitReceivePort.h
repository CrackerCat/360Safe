#pragma once
#include <ntifs.h>
#include "MemCheck.h"
#include "Data.h"
#include "Regedit.h"

//RPCͨѶ�ڸ���ƽ̨��������API������ͬ��������
//win2000 : NtFsControlFile
//xp, 2003 : NtRequestWaitReplyPort
//vista, 2008.win7 : NtAlpcSendWaitReceivePort
NTSTATUS NTAPI Fake_ZwAlpcSendWaitReceivePort(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg);