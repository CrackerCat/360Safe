#pragma once
#include <ntddk.h>
#include "Data.h"
//��ȡϵͳ�汾��Ϣ
ULONG HookPort_PsGetVersion();

typedef NTSTATUS(NTAPI * PFN_RtlGetVersion)(OUT PRTL_OSVERSIONINFOW lpVersionInformation);

