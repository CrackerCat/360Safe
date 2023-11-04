#pragma once
#include <ntifs.h>
#include "WhiteList.h"
#include "Data.h"
#include "Xor.h"

#define MAX_PATH          260

typedef struct _CLientLoadLibraryParam
{
	ULONG dwSize;//+0                      == InputLength
	ULONG dwStringLength; //+4
	ULONG ReservedZero1;//+8
	ULONG ReservedZero2;//+C
	ULONG ReservedZero3;//+10
	ULONG ReservedZero4;//+14
	USHORT Length;		
	USHORT MaximumLength;
	//ULONG ReservedZero5;//+18 () +1A () //����Ҫ��
	ULONG ptrDllString;//+1C            //ƫ����Ҫ+����ַ
	ULONG ReservedZero6;//+20
	ULONG ptrApiString;//+24			//ƫ����Ҫ+����ַ  
	WCHAR szDllName[MAX_PATH];
	WCHAR szApiName[MAX_PATH];
}CLientLoadLibraryParam, *PCLientLoadLibraryParam;


//����DLLע���
NTSTATUS NTAPI Fake_ClientLoadLibrary(ULONG CallIndex, PVOID ArgArray, PULONG ret_func, PULONG ret_arg);