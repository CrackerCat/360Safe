#pragma once
#include <ntifs.h>
#include "Data.h"
#include "WinKernel.h"
#include "SafeWarning.h"

//��ȡIoGetDiskDeviceObject������ַ�������øú���
NTSTATUS NTAPI Safe_IoGetDiskDeviceObjectPrt( PDEVICE_OBJECT FileSystemDeviceObject,  PDEVICE_OBJECT* DiskDeviceObject);

NTSTATUS NTAPI Fake_ZwWriteFile(ULONG CallIndex, PVOID ArgArray, PULONG ret_func, PULONG ret_arg);