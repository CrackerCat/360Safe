#pragma once
#include <ntifs.h>
#include "WinBase.h"
#include "Object.h"
#include "Data.h"
//��������
NTSTATUS NTAPI Fake_ZwTerminateProcess(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg);