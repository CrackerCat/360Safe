#pragma once
#include <ntifs.h>
#include "Object.h"

//���ö���ת������ʱ����
NTSTATUS NTAPI Fake_ZwMakeTemporaryObject(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg);