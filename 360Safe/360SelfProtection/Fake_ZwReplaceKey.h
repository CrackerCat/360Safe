#pragma once
#include <ntifs.h>
#include "MemCheck.h"
#include "Regedit.h"
//ȡ��ע���ֵ��
NTSTATUS NTAPI Fake_ZwReplaceKey(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg);