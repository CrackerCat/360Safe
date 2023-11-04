#pragma once
#include <ntddk.h>
#include "FilterHook.h"

//dword_1B0F8
PVOID		pOriginalKeUserModeCallbackAddr;

typedef
NTSTATUS
(NTAPI *pKeUserModeCallback)(
IN ULONG ApiNumber,
IN PVOID InputBuffer,
IN ULONG InputLength,
OUT PVOID *OutputBuffer,
IN PULONG OutputLength
);
//dword_1B0FC
pKeUserModeCallback OriginalKeUserModeCallback;

NTSTATUS NTAPI Filter_KeUserModeCallbackDispatcher(ULONG ApiNumber, PVOID InputBuffer, ULONG InputLength, PVOID *OutputBuffer, PULONG OutputLength);

//����DLLע���
#define	ClientLoadLibrary_FilterIndex	0x4B
NTSTATUS NTAPI Filter_ClientLoadLibrary(IN ULONG ApiNumber, IN PVOID InputBuffer, IN ULONG InputLength, OUT PVOID *OutputBuffer, IN PULONG OutputLength);


//��������
#define	fnHkOPTINLPEVENTMSG_XX1_FilterIndex 0x7A  //��֪�����庬�壬����������
#define	fnHkOPTINLPEVENTMSG_XX2_FilterIndex 0x60  //��֪�����庬�壬����������
NTSTATUS NTAPI Filter_fnHkOPTINLPEVENTMSG(IN ULONG ApiNumber, IN PVOID InputBuffer, IN ULONG InputLength, OUT PVOID *OutputBuffer, IN PULONG OutputLength);

//���ؼ�����Ϣ��
#define	fnHkINLPKBDLLHOOKSTRUCT_FilterIndex 0x7B  
NTSTATUS NTAPI Filter_fnHkINLPKBDLLHOOKSTRUCT(IN ULONG ApiNumber, IN PVOID InputBuffer, IN ULONG InputLength, OUT PVOID *OutputBuffer, IN PULONG OutputLength);

//����ģ����ص�
#define	ClientImmLoadLayout_XX1_FilterIndex 0x61  //��֪�����庬�壬����������
#define	ClientImmLoadLayout_XX2_FilterIndex 0x7  //��֪�����庬�壬����������
NTSTATUS NTAPI Filter_ClientImmLoadLayout(IN ULONG ApiNumber, IN PVOID InputBuffer, IN ULONG InputLength, OUT PVOID *OutputBuffer, IN PULONG OutputLength, PULONG Result);
