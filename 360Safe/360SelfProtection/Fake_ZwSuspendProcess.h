#pragma once
#include <ntifs.h>
#include "MemCheck.h"
#include "Data.h"
#include "WhiteList.h"

//���̹���
NTSTATUS NTAPI Fake_ZwSuspendProcess(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg);