#pragma once
#include <ntddk.h>
#include "FilterHook.h"

//������˺���û�ж�Ӧ��Fake_XXXX
#define	ZwContinue_FilterIndex	0x87
NTSTATUS NTAPI Filter_ZwContinue(PCONTEXT Context, BOOLEAN TestAlert);