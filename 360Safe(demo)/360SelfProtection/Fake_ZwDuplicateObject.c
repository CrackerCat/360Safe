#include "Fake_ZwDuplicateObject.h"

#define WHILEDRIVERNAMENUMBER_ZWDUPLICATEOBJECT 0xA
//Ҫ���صİ�������������
PWCHAR g_WhiteDriverName_ZwDuplicateObject[WHILEDRIVERNAMENUMBER_ZWDUPLICATEOBJECT + 1] = {
	0				//�Զ���
};

//�ж�DuplicateObject����ִ�к�Ĵ�����
BOOLEAN NTAPI CheckResult_After_DuplicateObject(NTSTATUS In_Status)
{
	BOOLEAN        Result = TRUE;
	ULONG          ReturnLength = NULL;
	ULONG          HandleCount = NULL;			//�������
	NTSTATUS       Status = STATUS_SUCCESS;
	Status = Safe_ZwQueryInformationProcess(NtCurrentProcess() , ProcessHandleCount, &HandleCount, sizeof(HandleCount), &ReturnLength);
	Result = (In_Status == STATUS_INSUFFICIENT_RESOURCES) && (!NT_SUCCESS(Status) || HandleCount == g_dynData->dword_3323C);
	return Result;
}

//û����
BOOLEAN NTAPI Safe_26794(IN HANDLE In_TargetProcessHandle)
{
	BOOLEAN        Result = TRUE;
	NTSTATUS       Status = STATUS_SUCCESS;
	PEPROCESS      TargetProcess = NULL;
	if (In_TargetProcessHandle)
	{
		//��ȡĿ����̵�Eprocess�ṹ
		Status = ObReferenceObjectByHandle(In_TargetProcessHandle,NULL,PsProcessType,UserMode,&TargetProcess,NULL);
		if (NT_SUCCESS(Status))
		{
			if (Safe_QuerySystemInformationList(TargetProcess, SYSTEMROOT_SYSTEM32_CSRSS_EXE))
			{
				Result = TRUE;
			}
			else if ((g_Win2K_XP_2003_Flag && Safe_QuerySystemInformationList(IoGetCurrentProcess(), SYSTEMROOT_SYSTEM32_SVCHOST_EXE)) ||	//�ж��Ƿ����
				(Safe_InsertSystemInformationList(TargetProcess, SYSTEMROOT_SYSTEM32_CSRSS_EXE,g_VersionFlag)) ||								//�����������
				(g_Win2K_XP_2003_Flag && Safe_QuerySystemInformationList(IoGetCurrentProcess(), SYSTEMROOT_SYSTEM32_SVCHOST_EXE))           //��Ӳ����ǾͲ�ѯ�Ƿ��Ѵ���
				)
			{
				Result = TRUE;
			}
			ObfDereferenceObject(TargetProcess);
		}
	}
	else
	{
		Result = FALSE;
	}
	return Result;
}
//����ϸ������������ͣ�File��Process��Section��Thread���в���
//File��   Υ����������ʾ����ָ���İ�������������
//Process��Υ����������ʾ����ָ���İ����������������IE
//Section��Υ�������·����\\Device\\PhysicalMemory��\\KnownDlls\\ 
//Thread�� Υ����������ʾ����ָ���İ����������������IE
NTSTATUS NTAPI Safe_26C42(IN HANDLE In_SourceHandle, IN ULONG In_Options, IN ACCESS_MASK In_DesiredAccess, IN HANDLE In_TargetProcessHandle, IN HANDLE In_SourceProcessHandle)
{
	NTSTATUS                   result = STATUS_SUCCESS;
	NTSTATUS                   Status = STATUS_SUCCESS;
	ACCESS_MASK                Local_DesiredAccess_Process =										//0x520D0BAF
		(GENERIC_WRITE | GENERIC_ALL) |                                                            //0x50000000 = GENERIC_WRITE | GENERIC_ALL
		(MAXIMUM_ALLOWED) |                                                                        //0x02000000 = MAXIMUM_ALLOWED
		(WRITE_OWNER | WRITE_DAC | DELETE) |   	                                                   //0x000D0000 = WRITE_OWNER | WRITE_DAC | DELETE
		(PROCESS_SUSPEND_RESUME | PROCESS_SET_QUOTA | PROCESS_SET_INFORMATION) |                   //0x00000B00 = PROCESS_SUSPEND_RESUME | PROCESS_SET_QUOTA | PROCESS_SET_INFORMATION
		(PROCESS_CREATE_PROCESS | PROCESS_VM_WRITE) |							                   //0x000000A0 = PROCESS_CREATE_PROCESS(Required to create a process) | PROCESS_VM_WRITE(WriteProcessMemory)
		(PROCESS_TERMINATE | PROCESS_CREATE_THREAD | PROCESS_SET_SESSIONID | PROCESS_VM_OPERATION);//0x0000000F = PROCESS_TERMINATE |PROCESS_CREATE_THREAD | PROCESS_SET_SESSIONID | PROCESS_VM_OPERATION	
	ACCESS_MASK                Local_DesiredAccess_File = 0x520D0156;
	ACCESS_MASK                Local_DesiredAccess_Section =										//0x52010002
		(GENERIC_WRITE | GENERIC_ALL) |																//0x50000000 = GENERIC_WRITE | GENERIC_ALL
		(MAXIMUM_ALLOWED) |																			//0x02000000 = MAXIMUM_ALLOWED
		(DELETE) |   																				//0x00010000 = DELETE
		(SECTION_MAP_WRITE);																		//0x00000002 = SECTION_MAP_WRITE
	ACCESS_MASK                Local_DesiredAccess_Thread = 										//0x520D00B7
		(GENERIC_WRITE | GENERIC_ALL) |																//0x50000000 = GENERIC_WRITE | GENERIC_ALL
		(MAXIMUM_ALLOWED) |																			//0x02000000 = MAXIMUM_ALLOWED
		(WRITE_OWNER | WRITE_DAC | DELETE) |   														//0x000D0000 = WRITE_OWNER | WRITE_DAC | DELETE
		(THREAD_SET_THREAD_TOKEN | THREAD_SET_INFORMATION | THREAD_SET_CONTEXT) |					//0x000000B0 = THREAD_SET_THREAD_TOKEN | THREAD_SET_INFORMATION | THREAD_SET_CONTEXT															   //0x000000B0 = 
		(THREAD_SUSPEND_RESUME | THREAD_ALERT | THREAD_TERMINATE);									//0x00000007 = THREAD_SUSPEND_RESUME | THREAD_ALERT | THREAD_TERMINATE

	PFILE_OBJECT               FileObject = NULL;
	CHAR		               ObjectInformation[0x500] = { 0 };
	ULONG                      ReturnLength = NULL;
	SIZE_T				       PhysicalMemorySize = 0x16;		        //\\Device\\PhysicalMemory�ַ�����С
	SIZE_T				       KnownDllsSize = 0xB;						//\\KnownDlls\\�ַ�����С
	PDRIVER_OBJECT             pDeviceObject = NULL;
	UNICODE_STRING             TempString1 = { 0 };
	BOOLEAN                    In_Options_Flag = TRUE;					//�ж�In_Options & 2
	ACCESS_MASK                Out_GrantedAccess = NULL;
	SYSTEM_INFORMATIONFILE_XOR System_InformationFile = { 0 };			//�ļ���Ϣ
	PPUBLIC_OBJECT_TYPE_INFORMATION	pPubObjTypeInfo = NULL;
	In_Options_Flag = In_Options & DUPLICATE_SAME_ACCESS;
	//1����ȡ��Ҫ���������Ȩ�ޣ�Դ���̵Ķ��������������4��ʼ����4Ϊ��λ������
	Status = Safe_GetGrantedAccess(In_SourceHandle, &Out_GrantedAccess);
	if (!NT_SUCCESS(Status))
	{
		result = STATUS_SUCCESS;
		return  result;
	}
	//2���Ͱ汾Ȩ��
	if (!g_Win2K_XP_2003_Flag)
	{
		Local_DesiredAccess_Process |= GENERIC_EXECUTE;  //0x720D0BAF;
	}
	//3����ѯע���HiveList�ṹ
	if (Safe_QuerHivelist(Out_GrantedAccess, In_SourceHandle, In_SourceProcessHandle))
	{
		result = STATUS_ACCESS_DENIED;
		return  result;
	}
	//4�����ݾ������ΪFile
	if (Safe_QueryObjectType(In_SourceHandle, L"File"))
	{
		//4��1 �õ��ļ�����ָ��
		Status = ObReferenceObjectByHandle(In_SourceHandle, FILE_ANY_ACCESS, *IoFileObjectType, UserMode, (PVOID*)&FileObject, NULL);
		if (!NT_SUCCESS(Status) && !FileObject && !FileObject->DeviceObject)
		{
		_Fun_Exit:
			//DUPLICATE_SAME_ACCESSȨ��
			if (In_Options_Flag)
			{
				//�жϵ�ǰ��������ԭʼȨ��
				if (!(Out_GrantedAccess & Local_DesiredAccess_File))
				{
					result = STATUS_SUCCESS;
					return  result;
				}
			}
			//�ж��µľ��Ȩ��
			else if (!(In_DesiredAccess & Local_DesiredAccess_File))
			{
				result = STATUS_SUCCESS;
				return  result;
			}
			//��ȡ�ļ�������Ϣ
			Status = Safe_GetInformationFile(In_SourceHandle, (ULONG)&System_InformationFile, UserMode);
			if (!NT_SUCCESS(Status))
			{
				result = STATUS_SUCCESS;
				return result;
			}
			//���Ҹ���Ϣ�Ƿ����б��У��ҵ�����1��ʧ�ܷ���0
			Status = Safe_QueryInformationFileList(System_InformationFile.IndexNumber_LowPart,
				System_InformationFile.u.IndexNumber_HighPart,
				System_InformationFile.VolumeSerialNumber);
			if (Status == 0)
			{
				//�����б�����������
				result = STATUS_SUCCESS;
			}
			else
			{
				//���б��д��󷵻�
				result = STATUS_ACCESS_DENIED;
			}
			return  result;
		}
		//4��2 �ж��ǲ��Ǵ����ܱ�������������·��
		//ԴĿ����̵ľ���Ǳ������̣�  �����ж�
		//ԴĿ����̵ľ���ǷǱ������̣�ֱ������
		pDeviceObject = FileObject->DeviceObject;
		if (Safe_QueryWintePID_ProcessHandle(In_SourceProcessHandle))
		{
			if (pDeviceObject && pDeviceObject->Flags)
			{
				//��ֹ������ܱ�������������
				for (ULONG i = 0; i < WHILEDRIVERNAMENUMBER_ZWDUPLICATEOBJECT; i++)
				{
					//��ΪDriverName��UUNICODE_STRING������������Ҫת����
					RtlInitUnicodeString(&TempString1, g_WhiteDriverName_ZwDuplicateObject[i]);
					if (RtlEqualUnicodeString(&pDeviceObject->DeviceObject->DriverObject->DriverName,&TempString1, TRUE))
					{
						//�ҵ���ֱ�Ӵ��󷵻�
						ObfDereferenceObject(FileObject);
						result = STATUS_ACCESS_DENIED;
						return  result;
					}
				}
			}
		}
		//4��3 �ⲿ��û�����ף�DriverInit == 0x14 ��������������
		if (pDeviceObject)
		{
			if (pDeviceObject->DriverInit == 0x14 &&			//ָ��DriverEntry�����ģ��⻹����ں�������0x14�ģ�������
				In_Options_Flag								    // �¾��ӵ����ԭʼ�����ͬ�İ�ȫ��������
				)
			{
				//��ȷ����
				ObfDereferenceObject(FileObject);
				result = STATUS_SUCCESS;
				return  result;
			}
		}
		ObfDereferenceObject(FileObject);
		goto _Fun_Exit;
	}
	//5�����ݾ������ΪProcess
	if (Safe_QueryObjectType(In_SourceHandle, L"Process"))
	{
		//ԴĿ����̵Ķ������Ǳ������̣�  �����ж�
		//ԴĿ����̵Ķ������ǷǱ������̣�ֱ������
		if (!Safe_QueryWintePID_ProcessHandle(In_SourceHandle))
		{
			result = STATUS_SUCCESS;
			return  result;
		}
		//DUPLICATE_SAME_ACCESSȨ��
		if (In_Options_Flag)
		{
			//�жϵ�ǰ��������ԭʼȨ��
			if (!(Out_GrantedAccess & Local_DesiredAccess_Process))
			{
				result = STATUS_SUCCESS;
				return  result;
			}
		}
		//�ж��µľ��Ȩ��
		else if (!(In_DesiredAccess & Local_DesiredAccess_Process))
		{
			result = STATUS_SUCCESS;
			return  result;
		}
		//��һ��ʵ��û�������ж�Ŀ����̾��
		if (Safe_26794(In_TargetProcessHandle))
		{
			result = STATUS_SUCCESS;
			return  result;
		}
		//������̷�IE
		if (Safe_CmpImageFileName("iexplore.exe"))
		{
			result = STATUS_CALLBACK_BYPASS;
			return  result;
		}
		//֪ͨ�û���R3 ���ػ��Ƿ���
		Safe_18A72_SendR3(PsGetCurrentProcessId(), PsGetCurrentThreadId(), 0xD);
		result = STATUS_ACCESS_DENIED;
		return  result;

	}
	//6�����ݾ������ΪThread,����ʽ��Processһ��
	if (Safe_QueryObjectType(In_SourceHandle, L"Thread"))
	{
		//ԴĿ����̵Ķ������Ǳ������̣�  �����ж�
		//ԴĿ����̵Ķ������ǷǱ������̣�ֱ������
		if (!Safe_QueryWintePID_ThreadHandle(In_SourceHandle))
		{
			result = STATUS_SUCCESS;
			return  result;
		}
		//DUPLICATE_SAME_ACCESSȨ��
		if (In_Options_Flag)
		{
			//�жϵ�ǰ��������ԭʼȨ��
			if (!(Out_GrantedAccess & Local_DesiredAccess_Thread))
			{
				result = STATUS_SUCCESS;
				return  result;
			}
		}
		//�ж��µľ��Ȩ��
		else if (!(In_DesiredAccess & Local_DesiredAccess_Thread))
		{
			result = STATUS_SUCCESS;
			return  result;
		}
		//��һ��ʵ��û�������ж�Ŀ����̾��
		if (Safe_26794(In_TargetProcessHandle))
		{
			result = STATUS_SUCCESS;
			return  result;
		}
		//������̷�IE
		if (Safe_CmpImageFileName("iexplore.exe"))
		{
			result = STATUS_CALLBACK_BYPASS;
			return  result;
		}
		//��process���õ���û��֪ͨR3�û���
		result = STATUS_ACCESS_DENIED;
		return  result;
	}
	//7�����ݾ������ΪSection
	if (Safe_QueryObjectType(In_SourceHandle, L"Section"))
	{
		//DUPLICATE_SAME_ACCESSȨ��
		if (In_Options_Flag)
		{
			//�жϵ�ǰ��������ԭʼȨ��
			if (!(Out_GrantedAccess & Local_DesiredAccess_Section))
			{
				result = STATUS_SUCCESS;
				return  result;
			}
		}
		//�ж��µľ��Ȩ��
		else if (!(In_DesiredAccess & Local_DesiredAccess_Section))
		{
			result = STATUS_SUCCESS;
			return  result;
		}
		//��ȡ���·������ֹ������·����KnownDlls��KnownDlls
		Status = Safe_UserMode_ZwQueryObject(g_HighgVersionFlag, *(HANDLE*)In_SourceHandle, ObjectNameInformation, ObjectInformation, sizeof(ObjectInformation), &ReturnLength);
		pPubObjTypeInfo = (PPUBLIC_OBJECT_TYPE_INFORMATION)ObjectInformation;
		if (!NT_SUCCESS(Status) ||
			!pPubObjTypeInfo ||
			!pPubObjTypeInfo->TypeName.Length
			)
		{
			result = STATUS_SUCCESS;
			return result;
		}
		//PhysicalMemory���    ��ȷ���0�����ȷ��ط�0
		if (!_wcsnicmp(pPubObjTypeInfo->TypeName.Buffer, L"\\Device\\PhysicalMemory", PhysicalMemorySize))
		{
			result = STATUS_ACCESS_DENIED;
		}
		// KnownDlls���        ��ȷ���0�����ȷ��ط�0
		else if (!_wcsnicmp(pPubObjTypeInfo->TypeName.Buffer, L"\\KnownDlls\\", KnownDllsSize))
		{
			result = STATUS_ACCESS_DENIED;
		}
		else
		{
			//�������أ�������·����
			result = STATUS_SUCCESS;
		}
		return result;
	}
	//8����������ֱ���������� ��������
	result = STATUS_SUCCESS;
	return result;
}

//���ƾ��
NTSTATUS NTAPI Fake_ZwDuplicateObject(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS       result = STATUS_SUCCESS;
	NTSTATUS       Status = STATUS_SUCCESS;
	ULONG          HandleCount = NULL;			//�������
	ULONG          MaxHandleCount = 0xF8000;	//����ʲô��˼��������������
	ULONG          ReturnLength = NULL;
	PEPROCESS      SourceProcess = NULL;
	PEPROCESS      TargetProcess = NULL;
	PEPROCESS      Temp_Eprocess = NULL;		//��ʱʹ��
	PETHREAD       Temp_Thread = NULL;			//��ʱʹ��
	HANDLE         Temp_TargetHandle = NULL;	//��ʱʹ�ã�����ԭʼZwDuplicateObjectʱ����շ���ֵ
	//0����ȡZwDuplicateObjectԭʼ����
	HANDLE  In_SourceProcessHandle = *(ULONG*)((ULONG)ArgArray);		//Դ����PID
	HANDLE  In_SourceHandle = *(ULONG*)((ULONG)ArgArray + 4);			//Ҫ�����ĸ����
	HANDLE  In_TargetProcessHandle = *(ULONG*)((ULONG)ArgArray + 0x8);	//Ŀ�����PID
	PHANDLE In_TargetHandle = *(ULONG*)((ULONG)ArgArray + 0xC);			//ָ��,������
	ACCESS_MASK In_DesiredAccess = *(ULONG*)((ULONG)ArgArray + 0x10);	//Ȩ��
	ULONG In_Attributes = *(ULONG*)((ULONG)ArgArray + 0x14);
	ULONG In_Options = *(ULONG*)((ULONG)ArgArray + 0x18);
	//1��������Ӧ�ò����
	if (!ExGetPreviousMode())
	{
		return result;
	}
	//2���������Ǳ������̷���
	if (Safe_QueryWhitePID(PsGetCurrentProcessId()))
	{
		return result;
	}
	//3 �����Process�����˳�
	if (Safe_QueryObjectType(In_SourceProcessHandle, L"Process"))
	{
		//4��û��DUPLICATE_CLOSE_SOURCE Ȩ��,ѡ��DUPLICATE_CLOSE_SOURCEʱ,Դ����ͻ��Զ��ر���
		if (!(In_Options & DUPLICATE_CLOSE_SOURCE))
		{
		_CheckParameter:
			//4��1 ���˵���PROCESS����
			if (!Safe_QueryObjectType(In_TargetProcessHandle, L"Process")
				|| Safe_QueryProcessHandleOrHandleCount(In_SourceProcessHandle)		//��һ��������ʲô�أ���ȡ�������������Ƿ�Ϸ����������
				|| Safe_QueryProcessHandleOrHandleCount(In_TargetProcessHandle))    //��һ��������ʲô�أ���ȡ�������������Ƿ�Ϸ����������
			{
				return result;
			}
			//4��2 ����TargetProcessHandle�Ǳ����������������
			if (Safe_QueryWintePID_ProcessHandle(In_TargetProcessHandle))
			{
				//4��3 ��ȡ�������
				Status = Safe_ZwQueryInformationProcess(In_TargetProcessHandle, ProcessHandleCount, &HandleCount, sizeof(HandleCount), &ReturnLength);
				if (!NT_SUCCESS(Status))
				{
					//��ȡʧ�ܾ������Ϊ0
					HandleCount = 0;
				}
				//4��4 ��֭�Ƚϣ���������
				//dword_3323CĬ����0x0FF8000�к����壿����
				if ((g_dynData->dword_3323C >= HandleCount) && (g_dynData->dword_3323C - HandleCount <= MaxHandleCount))
				{
					result = STATUS_ACCESS_DENIED;
					return result;
				}
			}
			//4��5 ������copy
			if ((In_SourceProcessHandle == NtCurrentProcess() &&
				In_TargetProcessHandle == NtCurrentProcess() &&
				(In_Options & DUPLICATE_SAME_ACCESS) && // �¾��ӵ����ԭʼ�����ͬ�İ�ȫ��������
				!(In_Attributes & OBJ_INHERIT))		  //��һ���ж��к������أ�û������
				)
			{
				result = STATUS_SUCCESS;
				return  result;
			}
			//4��6 �ֱ��ȡSourceProcessHandle��TargetProcessHandle��Eprocess
			Status = ObReferenceObjectByHandle(In_SourceProcessHandle,
				PROCESS_DUP_HANDLE,
				PsProcessType,
				UserMode,
				&SourceProcess,
				NULL);
			if (!NT_SUCCESS(Status))
			{
				result = STATUS_SUCCESS;
				return  result;
			}
			Status = ObReferenceObjectByHandle(In_TargetProcessHandle,
				PROCESS_DUP_HANDLE,
				PsProcessType,
				UserMode,
				&TargetProcess,
				NULL);
			if (!NT_SUCCESS(Status))
			{
				ObfDereferenceObject(SourceProcess);
				result = STATUS_SUCCESS;
				return  result;
			}
			//4��8 Դ��Ŀ�ľ����һ����
			if (TargetProcess == SourceProcess &&
				In_Options & DUPLICATE_SAME_ACCESS && // �¾��ӵ����ԭʼ�����ͬ�İ�ȫ��������
				!(In_Attributes & OBJ_INHERIT)		  //��һ���ж��к������أ�û������
				)
			{
				ObfDereferenceObject(SourceProcess);
				ObfDereferenceObject(TargetProcess);
				result = STATUS_SUCCESS;
				return  result;
			}
			//�����ú��治��Ҫʹ����
			ObfDereferenceObject(SourceProcess);
			ObfDereferenceObject(TargetProcess);
			SourceProcess = NULL;
			TargetProcess = NULL;
			//4��7 SourceProcessΪ����
			if (In_SourceProcessHandle == NtCurrentProcess())
			{
				//����ϸ������������ͣ�File��Process��Section��Thread���в���
				//File��   Υ����������ʾ����ָ���İ�������������
				//Process��Υ����������ʾ����ָ���İ����������������IE
				//Section��Υ�������·����\\Device\\PhysicalMemory��\\KnownDlls\\ 
				//Thread�� Υ����������ʾ����ָ���İ����������������IE
				return Safe_26C42(In_SourceHandle, In_Options, In_DesiredAccess, In_TargetProcessHandle, In_SourceProcessHandle);
			}
			//4��8 SourceProcess������
			Status = ObReferenceObjectByHandle(In_SourceProcessHandle,
				PROCESS_DUP_HANDLE,
				PsProcessType,
				UserMode,
				&SourceProcess,
				NULL);
			if (NT_SUCCESS(Status))
			{
				//Ŀ����̾�� == ������
				if (SourceProcess == IoGetCurrentProcess())
				{
					ObfDereferenceObject(SourceProcess);
					//����ϸ������������ͣ�File��Process��Section��Thread���в���
					//File��   Υ����������ʾ����ָ���İ�������������
					//Process��Υ����������ʾ����ָ���İ����������������IE
					//Section��Υ�������·����\\Device\\PhysicalMemory��\\KnownDlls\\ 
					//Thread�� Υ����������ʾ����ָ���İ����������������IE
					return Safe_26C42(In_SourceHandle, In_Options, In_DesiredAccess, In_TargetProcessHandle, In_SourceProcessHandle);
				}
				ObfDereferenceObject(SourceProcess);
			}
			//4��9  ����ԭʼZwDuplicateObject���������к������ú��ж�
			Status = Safe_ZwIoDuplicateObject(In_SourceProcessHandle, In_SourceHandle, NtCurrentProcess(), &Temp_TargetHandle, NULL, NULL, DUPLICATE_SAME_ACCESS, g_HighgVersionFlag, g_VersionFlag);
			//4��10 ��������Ķ�������Ϊ����
			if (NT_SUCCESS(Status))
			{
				//����ϸ������������ͣ�File��Process��Section��Thread���в���
				//File��   Υ����������ʾ����ָ���İ�������������
				//Process��Υ����������ʾ����ָ���İ����������������IE
				//Section��Υ�������·����\\Device\\PhysicalMemory��\\KnownDlls\\ 
				//Thread�� Υ����������ʾ����ָ���İ����������������IE
				result = Safe_26C42(Temp_TargetHandle, In_Options, In_DesiredAccess, In_TargetProcessHandle, In_SourceProcessHandle);
				Safe_ZwNtClose(Temp_TargetHandle, g_HighgVersionFlag);
				return result;
			}
			//4��11 ʧ�ܷ��أ����ݴ����뷵�ز�ͬ��ֵ
			return CheckResult_After_DuplicateObject(Status) != FALSE ? STATUS_ACCESS_DENIED : STATUS_SUCCESS;
		}
		//5��ԴĿ����̾���Ǳ�������ֱ����բ
		if (Safe_QueryWintePID_ProcessHandle(In_SourceProcessHandle))
		{
			Safe_18A72_SendR3(PsGetCurrentProcessId(), PsGetCurrentThreadId(), 0xC);
			result = STATUS_ACCESS_DENIED;
			return result;
		}
		//6��......
		if (In_TargetProcessHandle && !Safe_CheckSysProcess_Csrss_Lsass(In_SourceProcessHandle) ||
			In_SourceProcessHandle == NtCurrentProcess())
		{
			//��Դ��Windows�龰������4.8
			//In_TargetProcessHandleΪ0������ζ��Ŀ����̾���Դ���̣�����ζ�Ų����и���
			if (!In_TargetProcessHandle)		
			{
				result = STATUS_SUCCESS;
				return result;
			}
			else
			{
				goto _CheckParameter;
			}
		}
		//7�����ݽ����߳̾���õ���ӦEprocess����PETHREAD�ṹ����ֹ�����˱�������������
		//7��1 ����ԭʼZwDuplicateObject���������к������ú��ж�
		Status = Safe_ZwIoDuplicateObject(In_SourceProcessHandle, In_SourceHandle, NtCurrentProcess(), &Temp_TargetHandle, NULL, NULL, DUPLICATE_SAME_ACCESS, g_HighgVersionFlag, g_VersionFlag);
		if (!NT_SUCCESS(Status))
		{
			//ʧ�ܴ��󷵻�
			return CheckResult_After_DuplicateObject(Status) != FALSE ? STATUS_ACCESS_DENIED : STATUS_SUCCESS;
		}
		//7��2 ��ȡ��Ӧ��Eprocess or PETHREAD�ṹ
		Status = ObReferenceObjectByHandle(Temp_TargetHandle, NULL, PsProcessType, UserMode, &Temp_Eprocess, NULL);//��ȡEprocess
		if (NT_SUCCESS(Status))
		{
			//ͨ��Eprocess��ʽ���ң������Ǳ������̴��󷵻�
			if (Safe_QueryWhitePID_PsGetProcessId(Temp_Eprocess))
			{
				result = STATUS_ACCESS_DENIED;
			}
			ObfDereferenceObject(Temp_Eprocess);
			Safe_ZwNtClose(Temp_TargetHandle, g_HighgVersionFlag);
			return result;
		}
		else
		{
			//�ǽ����Ǿ����߳��ˣ���ȡPETHREAD
			Status = ObReferenceObjectByHandle(Temp_TargetHandle, NULL, PsThreadType, UserMode, &Temp_Thread, NULL);//��ȡPETHREAD
			if (NT_SUCCESS(Status))
			{
				//ͨ��PETHREAD��ʽ���ң������Ǳ������̴��󷵻�
				if (Safe_QueryWhitePID_PsGetThreadProcessId(Temp_Thread))
				{
					result = STATUS_ACCESS_DENIED;
				}
				ObfDereferenceObject(Temp_Thread);
				Safe_ZwNtClose(Temp_TargetHandle, g_HighgVersionFlag);
				return result;
			}
		}
		//�ǵ��ͷž��
		if (Temp_TargetHandle)
		{
			Safe_ZwNtClose(Temp_TargetHandle, g_HighgVersionFlag);
			Temp_TargetHandle = NULL;
		}
		//��Դ��Windows�龰������4.8
		//8��In_TargetProcessHandleΪ0������ζ��Ŀ����̾���Դ���̣�����ζ�Ų����и���
		if (!In_TargetProcessHandle)		
		{
			result = STATUS_SUCCESS;
			return result;
		}
		else
		{
			goto _CheckParameter;
		}
	}
	return result;
}