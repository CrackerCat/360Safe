#include "Fake_ZwOpenProcess.h"

#define WHILEPROCESSNAMENUMBER_ZWOPENPROCESS 0x4
//Ҫ���еİ�������������
PWCHAR g_WhiteProcessName_ZwOpenProcess[WHILEPROCESSNAMENUMBER_ZWOPENPROCESS + 1] = {
	0			//�Զ���
};

//����˵����
//1���򿪵��Ǳ������̣����򿪵ľ�����¸���һ�ݣ���Ȩ�˸��ģ�ԭʼ��ֱ��Close����
//2���򿪵��ǷǱ�������ֱ������
NTSTATUS NTAPI After_ZwOpenProcess_Func(IN ULONG FilterIndex, IN PVOID ArgArray, IN NTSTATUS InResult, IN PULONG RetFuncArgArray)
{
	NTSTATUS       Status = STATUS_SUCCESS;
	NTSTATUS       result = STATUS_SUCCESS;
	ACCESS_MASK    DesiredAccess_Flag =															   //0x520D0BAF
		(GENERIC_WRITE | GENERIC_ALL) |                                                            //0x50000000 = GENERIC_WRITE | GENERIC_ALL
		(MAXIMUM_ALLOWED) |                                                                        //0x02000000 = MAXIMUM_ALLOWED
		(WRITE_OWNER | WRITE_DAC | DELETE) |   	                                                   //0x000D0000 = WRITE_OWNER | WRITE_DAC | DELETE
		(PROCESS_SUSPEND_RESUME | PROCESS_SET_QUOTA | PROCESS_SET_INFORMATION) |                   //0x00000B00 = PROCESS_SUSPEND_RESUME | PROCESS_SET_QUOTA | PROCESS_SET_INFORMATION
		(PROCESS_CREATE_PROCESS | PROCESS_VM_WRITE) |							                   //0x000000A0 = PROCESS_CREATE_PROCESS(Required to create a process) | PROCESS_VM_WRITE(WriteProcessMemory)
		(PROCESS_TERMINATE | PROCESS_CREATE_THREAD | PROCESS_SET_SESSIONID | PROCESS_VM_OPERATION);//0x0000000F = PROCESS_TERMINATE |PROCESS_CREATE_THREAD | PROCESS_SET_SESSIONID | PROCESS_VM_OPERATION			
	HANDLE         Handle_v5 = NULL;
	PEPROCESS      pPeprocess = NULL;
	BOOLEAN        Terminate_Flag = FALSE;			//������PROCESS_TERMINATE
	HANDLE         TargetHandle = NULL;
	UCHAR          ImageFileNameBuff[0x1000] = { 0 };
	ACCESS_MASK    TestDesiredAccess = NULL;		//��ʱ
	//0����ȡZwOpenProcessԭʼ����
	PHANDLE		   In_ProcessHandle = *(ULONG*)((ULONG)ArgArray);
	ACCESS_MASK	   In_DesiredAccess = *(ULONG*)((ULONG)ArgArray + 4);
	PCLIENT_ID	   In_ClientId = *(ULONG*)((ULONG)ArgArray + 0xC);
	//1���ж��ϴε���ԭʼ��������ֵ
	if (!NT_SUCCESS(InResult))
	{
		return InResult;
	}
	//2���Ͱ汾Ȩ��
	if (!g_Win2K_XP_2003_Flag)
	{
		DesiredAccess_Flag |= GENERIC_EXECUTE;  //0x720D0BAF;
	}
	DesiredAccess_Flag = ~DesiredAccess_Flag;
	//ȡ���Ļ�ʧȥ�˱Ƚ���Ҫ��Ȩ�����£�
	//DesiredAccess_Flag=0xADF2F450
	//1��PROCESS_VM_OPERATION       //���������ڴ�ռ��Ȩ��(����VirtualProtectEx��WriteProcessMemory) 
	//2��PROCESS_VM_WRITE           //��ȡ�����ڴ�ռ��Ȩ�ޣ���ʹ��WriteProcessMemory
	//3�������̡߳�����֮���

	//2��1�жϲ����Ϸ���
	if (myProbeRead(In_ProcessHandle, sizeof(CLIENT_ID), sizeof(ULONG)))
	{
		KdPrint(("ProbeRead(After_ZwOpenProcess_Func��In_ProcessHandle) error \r\n"));
		return result;
	}
	Handle_v5 = *(HANDLE*)In_ProcessHandle;
	//3���ж�Ҫ�򿪵ľ���ǲ��ǰ��������̣�����Ǽ����жϣ�����ֱ���˳�
	if (Safe_QueryWintePID_ProcessHandle(Handle_v5) &&
		!Safe_CheckSysProcess()					//�˶�csrss.exe��svchost.exe��dllhost.exe
		)
	{
		//��ֹ������
		if (Safe_InsertSystemInformationList(IoGetCurrentProcess(), SYSTEMROOT_SYSTEM32_SVCHOST_EXE,g_VersionFlag))
		{
		_FunctionRet:				//�����˳����˸��ܱ������̾��Ȩ��
			if (Terminate_Flag)
			{
				//������������̱���PROCESS_TERMINATE����
				Terminate_Flag = FALSE;
				TestDesiredAccess = In_DesiredAccess & DesiredAccess_Flag | PROCESS_TERMINATE;
			}
			else
			{
				//�ܱ�������������
				TestDesiredAccess = In_DesiredAccess & DesiredAccess_Flag;
			}
			//����������ҽ�Ȩ
			Status = ZwDuplicateObject(
				NtCurrentProcess(),						//__in HANDLE SourceProcessHandle,
				Handle_v5,								//__in HANDLE SourceHandle,
				NtCurrentProcess(),						//__in_opt HANDLE TargetProcessHandle,
				&TargetHandle,							//__out_opt PHANDLE TargetHandle,
				TestDesiredAccess,						//__in ACCESS_MASK DesiredAccess,
				NULL,									//__in ULONG HandleAttributes,
				NULL									//__in ULONG Options
				);
			if (NT_SUCCESS(Status))
			{
				//������ԭʼ�ľ��
				if (Handle_v5)
				{
					Safe_ZwNtClose(Handle_v5, g_VersionFlag);
				}
				//������Ȩ�����˸���
				*(HANDLE*)In_ProcessHandle = TargetHandle;
				result = STATUS_SUCCESS;
			}
			else
			{
				_InvalidRet:		//�����˳���Copy���ʧ�ܡ�����������˳�
				//����ʧ�ܾ��ֱ�����㣬���ظ�����ֵ
				Safe_ZwNtClose(Handle_v5, g_VersionFlag);
				*(HANDLE*)In_ProcessHandle = 0;
				result = STATUS_ACCESS_DENIED;

			}
			return result;
		}
		//���Ȩ�޲�Ӧ����PROCESS_ALL_ACCESS =0x001fffff ������������
		else if (In_DesiredAccess == 0x1F0FFF && Safe_CmpImageFileName("Mcshield.exe"))
		{
			goto _FunctionRet;
		}
		//�������� or ���н������̱�־
		else if (RetFuncArgArray == TRUE || DesiredAccess_Flag & PROCESS_TERMINATE)
		{
			//�жϸ������ǲ���������̣����������
			if (Safe_CmpImageFileName("taskmgr.exe"))
			{
				Status = ObReferenceObjectByHandle(Handle_v5, NULL, PsProcessType, KernelMode, &pPeprocess, 0);
				if (NT_SUCCESS(Status))
				{
					//��ȡҪ�򿪾����·�����ж��ǲ��Ǵ����ֵ�
					Safe_PsGetProcessImageFileName(pPeprocess, &ImageFileNameBuff, sizeof(ImageFileNameBuff));
					//����360*****��360*****�Ľ������ö�Ӧ��Ȩ��(�˸�)��Ψһ������PROCESS_TERMINATE
					for (ULONG i = 0; i < WHILEPROCESSNAMENUMBER_ZWOPENPROCESS; i++)
					{
						if (_stricmp(&ImageFileNameBuff, g_WhiteProcessName_ZwOpenProcess[i]) == 0)
						{
							//����������ñ�־λ������PROCESS_TERMINATE����
							Terminate_Flag = TRUE;
							break;
						}
					}
					//���ü���-1
					if (pPeprocess)
					{
						ObfDereferenceObject(pPeprocess);
						pPeprocess = NULL;
					}
				}
			}
			//����Copy������˸
			goto _FunctionRet;
		}
		else
		{
			//һ�㲻��ִ�е�����
			//��Ч�˳������ֱ��������󷵻�
			goto _InvalidRet;
		}
	}
	else
	{
		//�Ǳ������̣�ֱ�ӷ��� ����ԭʼȨ��
		result = STATUS_SUCCESS;
	}
	return result;
}

//�򿪽���
NTSTATUS NTAPI Fake_ZwOpenProcess(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS       Status = STATUS_SUCCESS;
	NTSTATUS       result = STATUS_SUCCESS;
	ACCESS_MASK    DesiredAccess_Flag =															   //0x520D0BAF
		(GENERIC_WRITE | GENERIC_ALL) |                                                            //0x50000000 = GENERIC_WRITE | GENERIC_ALL
		(MAXIMUM_ALLOWED) |                                                                        //0x02000000 = MAXIMUM_ALLOWED
		(WRITE_OWNER | WRITE_DAC | DELETE) |   	                                                   //0x000D0000 = WRITE_OWNER | WRITE_DAC | DELETE
		(PROCESS_SUSPEND_RESUME | PROCESS_SET_QUOTA | PROCESS_SET_INFORMATION) |                   //0x00000B00 = PROCESS_SUSPEND_RESUME | PROCESS_SET_QUOTA | PROCESS_SET_INFORMATION
		(PROCESS_CREATE_PROCESS | PROCESS_VM_WRITE) |							                   //0x000000A0 = PROCESS_CREATE_PROCESS(Required to create a process) | PROCESS_VM_WRITE(WriteProcessMemory)
		(PROCESS_TERMINATE | PROCESS_CREATE_THREAD | PROCESS_SET_SESSIONID | PROCESS_VM_OPERATION);//0x0000000F = PROCESS_TERMINATE |PROCESS_CREATE_THREAD | PROCESS_SET_SESSIONID | PROCESS_VM_OPERATION			
	PEPROCESS      ClientProcess = NULL;
	PETHREAD       ClientThread = NULL;
	BOOLEAN        Protection_Flag = FALSE;						//���ʵ����ܱ���������1
	UCHAR          ImageFileNameBuff[0x256] = { 0 };
	//0����ȡZwOpenProcessԭʼ����
	PHANDLE     In_ProcessHandle = *(ULONG*)((ULONG)ArgArray);
	ACCESS_MASK In_DesiredAccess = *(ULONG*)((ULONG)ArgArray + 4);
	PCLIENT_ID  In_ClientId = *(ULONG*)((ULONG)ArgArray + 0xC);
	//1��������Ӧ�ò����
	if (ExGetPreviousMode())
	{
		//2���Ͱ汾Ȩ��
		if (!g_Win2K_XP_2003_Flag)
		{
			DesiredAccess_Flag |= GENERIC_EXECUTE;  //0x720D0BAF
		}
		//3���ж�Ȩ��
		if (DesiredAccess_Flag & In_DesiredAccess)
		{
			//�ж��ǲ��Ǳ������̵���ZwOpenProcess�������ֱ�ӷ��أ����Ǽ����ж�
			if (!Safe_QueryWhitePID(PsGetCurrentProcessId()))
			{
				if (In_DesiredAccess & PROCESS_TERMINATE)
				{
					if (In_ClientId)
					{
						//�жϲ����Ϸ���
						if (myProbeRead(In_ClientId, sizeof(CLIENT_ID), sizeof(ULONG)))
						{
							KdPrint(("ProbeRead(Fake_ZwOpenProcess��In_ClientId) error \r\n"));
							return result;
						}
						//��ȡETHREAD / EPROCESS
						if (In_ClientId->UniqueThread)
						{
							//ǧ��������ͷ�ObfDereferenceObject(ClientProcess)��ObfDereferenceObject(ClientThread)
							Status = PsLookupProcessThreadByCid(&In_ClientId, &ClientProcess, &ClientThread);
						}
						else
						{
							Status = PsLookupProcessByProcessId(In_ClientId->UniqueProcess, &ClientProcess);
						}
						//��ȡʧ��ֱ���˳�
						if (!NT_SUCCESS(Status))
						{
							*(ULONG*)ret_func = After_ZwOpenProcess_Func;
							return result;
						}
						//�ж���򿪵Ľ����ǲ��Ǳ������̣������˳�  �Ǽ����ж�
						if (!Safe_QueryWhitePID_PsGetProcessId(ClientProcess))
						{
							//�ǰ���������ֱ���˳���
							if (ClientThread)
							{
								ObfDereferenceObject(ClientThread);
							}
							ObfDereferenceObject(ClientProcess);
							*(ULONG*)ret_func = After_ZwOpenProcess_Func;
							return result;
						}
						//�ܱ��������м�����Ҫ��������Ȩ��(�⼸�����̲��ڰ������б���)				
						if (ClientThread)
						{
							ObfDereferenceObject(ClientThread);
						}
						if (!Safe_InsertSystemInformationList(IoGetCurrentProcess(), SYSTEMROOT_SYSTEM32_CSRSS_EXE, g_VersionFlag))
						{
							//��ȡҪ�򿪾����·�����ж��ǲ��Ǵ����ֵ�
							Safe_PsGetProcessImageFileName(ClientProcess, &ImageFileNameBuff, sizeof(ImageFileNameBuff));
							//����360*****��360*****�Ľ���ֱ�ӷ���,���������������ö�Ӧ��Ȩ��
							for (ULONG i = 0; i < WHILEPROCESSNAMENUMBER_ZWOPENPROCESS; i++)
							{
								if (_stricmp(&ImageFileNameBuff, g_WhiteProcessName_ZwOpenProcess[i]) == 0)
								{
									//�������ֱ�ӷ���
									break;
								}
								else
								{
									//�������̽����˸�Ȩ��

									//�ж��ǲ��������OpenPorcess(PROCESS_TERMINATE,xx,PID)Ȼ�����TerminateProcess��������
									ULONG Flag = *(ULONG*)((ULONG)ArgArray + 4) & PROCESS_TERMINATE == 0;
									//ȡ�����������̵�Ȩ��
									*(ULONG*)((ULONG)ArgArray + 4) &= PROCESS_TERMINATE;
									//����PROCESS_TERMINATEȨ�ޣ��͸����ø�Ĭ��Ȩ��
									if (Flag)
									{
										*(ULONG*)((ULONG)ArgArray + 4) = PROCESS_QUERY_INFORMATION;
									}
									Protection_Flag = TRUE;
								}
							}
						}
						ObfDereferenceObject(ClientProcess);
					}
				}
				*(ULONG*)ret_func = After_ZwOpenProcess_Func;
				*(ULONG*)ret_arg = Protection_Flag;						//���ʵ����ܱ���������1
			}
		}
	}
	return result;
}