#include "Command.h"


//������Ȥ��ͨ�ô���
NTSTATUS Safe_CommonProc(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	//ֱ����ɣ����سɹ�
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS Safe_Shutdown(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS			Status = STATUS_SUCCESS;
	UNREFERENCED_PARAMETER(DeviceObject);
	//��
	Irp->IoStatus.Status = Status;							//��ʾIRP���״̬
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Status;
}

NTSTATUS Safe_CreateCloseCleanup(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS			Status = STATUS_SUCCESS;
	UNREFERENCED_PARAMETER(DeviceObject);
	//��
	Irp->IoStatus.Status = Status;							//��ʾIRP���״̬
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Status;
}

NTSTATUS Safe_Read(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS			Status = STATUS_SUCCESS;
	UNREFERENCED_PARAMETER(DeviceObject);
	//��
	Irp->IoStatus.Status = Status;							//��ʾIRP���״̬
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Status;
}


NTSTATUS Safe_DeviceControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS			Status = STATUS_SUCCESS;  //����ֵ
	PIO_STACK_LOCATION	IrpStack;			      //��ǰ��pIrpջ
	//PVOID				Type3InputBuffer = NULL;  //�û�̬�����ַ
	PVOID				ioBuf = NULL;			  //SystemBuff���Ե������롢��� 
	ULONG				inBufLength = NULL;       //���뻺�����Ĵ�С
	ULONG               outBufLength = NULL;      //����������Ĵ�С 
	ULONG				ioControlCode = NULL;	  //DeviceIoControl�Ŀ��ƺ�
	ULONG				Tag = 0x206B6444;		  //pool tag
	//UNICODE_STRING      ObjectNameString = { 0 };
	SIZE_T              x360sdu_dat_Len = 0xA;	  //360sdu.dat �ַ�������
	SIZE_T              xboxu_dat_Len = 0x8;      //boxu.dat   �ַ�������
	Irp->IoStatus.Information = 0;
	IrpStack = IoGetCurrentIrpStackLocation(Irp);
	ioBuf = Irp->AssociatedIrp.SystemBuffer;	  //����������Խ�SystemBuffer��Ϊ�������ݣ�Ҳ���Խ�SystemBuffer��Ϊ�������
	inBufLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
	outBufLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
	ioControlCode = IrpStack->Parameters.DeviceIoControl.IoControlCode;
	//1����������,�����Ǳ�������
	if (Safe_QueryWhitePID(PsGetCurrentProcessId()))
	{
		//���ݿ��ƺ�ִ�в�ͬ����
		switch (ioControlCode)
		{
			case SAFE_UNKNOWN:
			{
				//�������ܣ�������·���Ϸ��ԣ���ֹ���������ܱ����ļ���
				//���ƺţ�0x222000
				//buff:       sizeof=0xN
				//+00 WCHAR   ProcessPath[X]
				if (inBufLength)
				{
					ULONG BuffPathLen = 0;
					BuffPathLen = 2 * wcslen(ioBuf) + 2;
					Status = Safe_2555E(ioBuf, 1, BuffPathLen, 0);
					if (NT_SUCCESS(Status))
					{
						if (!_wcsnicmp(((PWCHAR)ioBuf + BuffPathLen - 0x16), L"360sdu.dat", x360sdu_dat_Len))
						{
							g_Regedit_Data.Flag.RULE_360sd_Flag = 1;
						}
						else if (!_wcsnicmp(((PWCHAR)ioBuf + BuffPathLen - 0x12), L"boxu.dat", xboxu_dat_Len))
						{
							g_Regedit_Data.Flag.RULE_360SafeBox_Flag = 1;
						}
						else
						{
							//�����˳�
						}
					}
				}
				break;
			}
			case SAFE_UNKNOWN1:
			{
				//�������ܣ�������·���Ϸ���
				//���ƺţ�0x222004
				//buff:     sizeof=0xN
				//+00 WCHAR ProcessPath[X]

				if (inBufLength)
				{
					Status = Safe_2555E(ioBuf, 1, inBufLength, 2);
				}
				else
				{
					//���볤�ȷǷ�
					Status = STATUS_INVALID_PARAMETER;
				}
				break;
			}
			case SAFE_UNKNOWN2:
			{
				//�������ܣ�������·���Ϸ��ԣ�Ȼ��Ƿ��������ܱ����ļ���
				//���ƺţ�0x222008
				//buff:     sizeof=0xN
				//+00 WCHAR ProcessPath[X]
				if (inBufLength)
				{
					ULONG BuffPathLen = 0;
					BuffPathLen = 2 * wcslen(ioBuf) + 2;
					Status = Safe_2555E(ioBuf, 1, BuffPathLen, 1);
					if (NT_SUCCESS(Status))
					{
						if (!_wcsnicmp(((PWCHAR)ioBuf + BuffPathLen - 0x16), L"360sdu.dat", x360sdu_dat_Len))
						{
							g_Regedit_Data.Flag.RULE_360sd_Flag = 1;
						}
						else if (!_wcsnicmp(((PWCHAR)ioBuf + BuffPathLen - 0x12), L"boxu.dat", xboxu_dat_Len))
						{
							g_Regedit_Data.Flag.RULE_360SafeBox_Flag = 1;
						}
						else
						{
							//�����˳�
						}
					}
				}
				break;
			}
			case SAFE_INSERTWHITELIST__PID_2003:	
			{
				//�������ܣ����ָ������������PID(Win_2003) 
				//���ƺţ�0x22200C
				//buff:     sizeof=0x4
				//+00 DWORD ProcessId        In

				if (inBufLength == sizeof(ULONG))
				{
					//����Ӧ����һ��4�ֽ�PID
					ULONG User_In_InsertPID = *(ULONG*)ioBuf;
					Safe_InsertWhiteList_PID_Win2003((HANDLE)User_In_InsertPID, 0);
				}
				else
				{
					//���볤�ȷǷ�
					Status = STATUS_INVALID_PARAMETER;
				}
				break;
			}
			case SAFE_DELETEWHITELIST_PID:			
			{
				//�������ܣ�ɾ��ָ������������PID
				//���ƺţ�0x222010
				//buff:     sizeof=0x4
				//+00 DWORD ProcessId        In
				if (inBufLength == sizeof(ULONG))
				{
					//����Ӧ����һ��4�ֽ�PID
					ULONG User_In_DeletePID = *(ULONG*)ioBuf;
					Safe_DeleteWhiteList_PID((HANDLE)User_In_DeletePID);
				}
				else
				{
					//���볤�ȷǷ�
					Status = STATUS_INVALID_PARAMETER;
				}
				break;
			}
			case SAFE_GETVER:
			{
				//�������ܣ�����R3һ���汾��
				//���ƺţ�0x222014
				//buff:     sizeof=0x4
				//+00 DWORD GetVersions        Out
				if (outBufLength == sizeof(ULONG))
				{
					*(ULONG*)ioBuf = 0x3ED;							//�汾�ţ�����������
					Irp->IoStatus.Information = sizeof(ULONG);		//����������û���Ӱ��
				}
				else
				{
					//���볤�ȷǷ�
					Status = STATUS_INVALID_PARAMETER;
				}
				break;
			}
			case SAFE_INSERTSPECIALWHITELIST_PID:
			{
				//�������ܣ��������߽�����ӽ��������������
				//���ƺţ�0x222018
				//�޲���
				if (Global_SpShadowDeviceObject == DeviceObject)
				{
					Status = Safe_InsertSpecialWhiteList_PID();
				}
				else
				{
					Status = STATUS_INVALID_PARAMETER;
				}
				break;
			}
			case SAFE_INITIALIZE_SETEVENT:
			{
				//�������ܣ�����������ʾʱ��Ӧ�ò��ͨ�Ž����ĳ�ʼ���¼�
				//���ƺţ�0x22201C
				//buff:     sizeof=0x8
				//+00 HANDLE ThreadID		   In
				//+00 DWORD  bypass_or_not     In
				if (Global_SpShadowDeviceObject == DeviceObject)
				{
					if (inBufLength == 8)
					{
						ULONG User_In_ThreadID = *(ULONG*)((ULONG)ioBuf);
						ULONG  User_In_bypass_or_not = *(ULONG*)((ULONG)ioBuf + 4);
						//SafeWarning����ĺ�����ûʵ�ʲ��ԣ���Ҫʹ��
						Safe_setevent_called_by_iodispatcher((HANDLE)User_In_ThreadID, User_In_bypass_or_not);
					}
					else
					{
						Status = STATUS_INVALID_PARAMETER;
					}
				}
				break;
			}
			case SAFE_SET_FAKEFUNCTION:
			{
				//�������ܣ�����or����FAKE����
				//���ƺţ�0x222020
				//buff:     sizeof=0x4
				//+00 DWORD FakeFunSwitch        In
				if (inBufLength == sizeof(ULONG))
				{
					//Buff��ֵ����������һ��  ���һ�٣�������  ���־������ˣ�����������
					ULONG User_In_Switch = *(ULONG*)ioBuf;
					if (User_In_Switch)
					{
						//���¹ҹ�
						if (!g_x360SelfProtection_Switch)
						{
							Safe_Initialize_SetFilterSwitchFunction();
							g_x360SelfProtection_Switch = TRUE;
						}
					}
					//ȥ���󲿷�Fake����
					else if (g_x360SelfProtection_Switch)
					{
						//һ����0x9E��Fake����
						for (ULONG Index = 0; Index < FILTERFUNCNT; Index++)
						{
							if (Index != CreateProcessNotifyRoutine_FilterIndex,
								Index != ZwCreateProcess_FilterIndex,
								Index != ZwCreateSection_FilterIndex,
								Index != ZwCreateProcessEx_FilterIndex,
								Index != ZwCreateUserProcess_FilterIndex,
								Index != ZwAlpcSendWaitReceivePort_FilterIndex,
								Index != ZwRequestWaitReplyPort_FilterIndex,
								Index != ZwLoad_Un_Driver_FilterIndex,
								Index != ZwQuerySystemInformation_FilterIndex,
								Index != ZwWriteFile_FilterIndex
								)
							{
								//Fake������0
								Safe_Run_SetFilterSwitchFunction(g_FilterFun_Rule_table_head_Temp, Index, 0);
							}
						}
						g_x360SelfProtection_Switch = FALSE;
					}
					else
					{
						//�Ҳ��������˳�
					}
				}
				else
				{
					//���볤�ȷǷ�
					Status = STATUS_INVALID_PARAMETER;
				}
				break;
			}
			case SAFE_GET_FAKEFUNCTION_SWITCH:
			{
				//�������ܣ���ȡFake�������״̬��1�����0�ҹ�
				//���ƺţ�0x222024
				//buff:     sizeof=0x4
				//+00 DWORD GetFakeSwitch        Out
				if (outBufLength == sizeof(ULONG))
				{
					*(ULONG*)ioBuf = g_x360SelfProtection_Switch;
					Irp->IoStatus.Information = sizeof(ULONG);
				}
				else
				{
					//������ȷǷ�
					Status = STATUS_INVALID_PARAMETER;
				}
				break;
			}
			case SAFE_DELETEDRVMKDATALIST:
			{
				//�������ܣ�ɾ���ڰ�����������Ϣ
				//���ƺţ�0x222028
				//buff:     sizeof=0x18
				//����PE��ϣֵ���ļ���С������or���ر�־λ
				//��С0x18���ֽ�
				//typedef struct _PE_HASH_DATA
				//{
				//	ULONG  Hash[4];									//��ϣֵ
				//	ULONG  PESize;									//�ļ���С
				//	ULONG  LoadDriver_Flag;							//���� ����or���б�ʶ   1���� 0����
				//}PE_HASH_DATA, *PPE_HASH_DATA;

				if (Global_SpShadowDeviceObject == DeviceObject)
				{
					if (inBufLength == sizeof(PE_HASH_DATA))
					{
						PPE_HASH_DATA User_In_DeleteDrvmkDataList = ioBuf;
						Safe_DeleteDrvmkDataList(User_In_DeleteDrvmkDataList);
					}
					else
					{
						//������ȷǷ�
						Status = STATUS_INVALID_PARAMETER;
					}
				}
				break;
			}
			case SAFE_INSERTDRVMKDATALIST:
			{
				//�������ܣ���Ӻڰ�����������Ϣ
				//���ƺţ�0x22202C
				//buff:     sizeof=0x18
				//����PE��ϣֵ���ļ���С������or���ر�־λ
				//��С0x18���ֽ�
				//typedef struct _PE_HASH_DATA
				//{
				//	ULONG  Hash[4];									//��ϣֵ
				//	ULONG  PESize;									//�ļ���С
				//	ULONG  LoadDriver_Flag;							//���� ����or���б�ʶ   1���� 0����
				//}PE_HASH_DATA, *PPE_HASH_DATA;
				if (Global_SpShadowDeviceObject == DeviceObject)
				{
					if (inBufLength == sizeof(PE_HASH_DATA))
					{
						PPE_HASH_DATA User_In_InsertDrvmkDataList = ioBuf;
						Safe_InsertDrvmkDataList(User_In_InsertDrvmkDataList);
					}
					else
					{
						//������ȷǷ�
						Status = STATUS_INVALID_PARAMETER;
					}
				}
				break;
			}
			case SAFE_OFF_360SAFEBOX_SWITCH:
			{
				//�������ܣ��ر�360SafeBoxѡ��
				//���ƺţ�0x222034
				//buff:     sizeof=0x0
				g_Regedit_Data.Flag.RULE_360SafeBox_Flag = 0;
				break;
			}
			case SAFE_CHECK_INSERTWHITELIST__PID:
			{
				//�������ܣ��ȼ�����·�����Ϸ������Ӱ���������
				//���ƺţ�0x222038
				//buff:     sizeof=0xN
				//+00 WCHAR ProcessPath[X]
				if (inBufLength)
				{
					Status = Safe_2555E(ioBuf, 1, inBufLength, 3);
					if (NT_SUCCESS(Status))
					{
						//�������߽�����ӽ�������
						Safe_InsertWhiteList_PID(PsGetCurrentProcessId(), 0xFFFFFFF0);
					}
				}
				else
				{
					//���ش���ֵ
					Status = STATUS_INVALID_PARAMETER;
				}
				break;
			}
			case SAFE_DELETEWHITELIST_PID_SESSIONID:
			{
				//�������ܣ�ָ��ɾ������������
				//���ƺţ�0x22203C
				//buff:     sizeof=0x4
				//+00 DWORD ProcessId        In

				if (inBufLength == sizeof(ULONG))
				{
					//����Ӧ����һ��4�ֽ�PID
					ULONG User_In_DeletePID = *(ULONG*)ioBuf;
					Safe_DeleteWhiteList_PID_SessionId((HANDLE)User_In_DeletePID);
				}
				else
				{
					//������ȷǷ�
					Status = STATUS_INVALID_PARAMETER;
				}
				break;
			}
			case SAFE_INSERTWHITELIST__PID:
			{
				//���ָ������������PID
				//���ƺţ�0x222040
				//buff:     sizeof=0x4
				//+00 DWORD ProcessId        In

				if (inBufLength == sizeof(ULONG))
				{
					ULONG User_In_InsertPID = *(ULONG*)ioBuf;
					Safe_InsertWhiteList_PID((HANDLE)User_In_InsertPID, 0);
				}
				else
				{
					//���� or ������ȷǷ�
					Status = STATUS_INVALID_PARAMETER;
				}
				break;
			}
			case SAFE_NO_360SAFEBOX_SWITCH:
			{
				//�������ܣ�����360SafeBoxѡ��
				//���ƺţ�0x222044
				//buff:     sizeof=0x0

				g_Regedit_Data.Flag.RULE_360SafeBox_Flag = 1;
				break;
			}
			case SAFE_SET_SPSHADOW0_DATA_DWORD:
			{
				//�������ܣ�����Safe_SetRegedit_SpShadow0����
				//���ƺţ�0x222048
				//buff:     sizeof=0x4
				//+00 DWORD SpShadow0_Switch     In

				if (inBufLength == sizeof(ULONG))
				{
					g_Regedit_Data.g_SpShadow0_Data_DWORD = *(ULONG*)ioBuf;
				}
				else
				{
					//���� or ������ȷǷ�
					Status = STATUS_INVALID_PARAMETER;
				}
				break;
			}
			case SAFE_QUERYWHITE_PID:
			{
				//�������ܣ���ѯָ��PID��������ظ�R3
				//���ƺţ�  0x22204C
				//buff:     sizeof=0x4
				//+00 DWORD ProcessPID     In
				//+00 DWORD QueryResult    Out

				if (outBufLength == sizeof(ULONG) && inBufLength == sizeof(ULONG))
				{
					ULONG User_In_QueryPID = *(ULONG*)ioBuf;
					//����ֵ���ҵ�1���Ҳ���0
					*(ULONG*)ioBuf = Safe_QueryWhitePID((HANDLE)User_In_QueryPID);		//����������___©��
					Irp->IoStatus.Information = sizeof(ULONG);
				}
				else
				{
					//���� or ������ȷǷ�
					Status = STATUS_INVALID_PARAMETER;
				}
				break;
			}
			case SAFE_UNKNOWN7:
			{
				//�������ܣ���1��������
				//���ƺţ�0x222050
				//buff:     sizeof=0x0

				g_dynData->dword_34DAC[9] = 1;
				break;
			}
			case SAFE_UNKNOWN8:
			{
				//�������ܣ���0��������
				//���ƺţ�0x222054
				//buff:     sizeof=0x0

				g_dynData->dword_34DAC[9] = 0;
				break;
			}
			case SAFE_RESETDRVMKDATALIST:
			{
				//�������ܣ����úڰ�������������������
				//���ƺţ�0x222064
				//buff:     sizeof=0x0

				g_Drvmk_List = NULL;
				break;
			}
			case SAFE_SET_DWORD_34678:
			{
				//�������ܣ�R0����R3  ���
				//���ƺţ�0x222080
				//buff:     sizeof=0x4
				//+00 DWORD OutUnknown  In 

				if (outBufLength == sizeof(ULONG))
				{
					*(ULONG*)ioBuf = g_dword_34678;
					Irp->IoStatus.Information = sizeof(ULONG);
				}
				else
				{
					//������ȷǷ�
					Status = STATUS_INVALID_PARAMETER;
				}
				break;
			}
			case SAFE_GET_DWORD_34678:
			{
				//�������ܣ�R3����R0  ����
				//���ƺţ�0x222084
				//buff:     sizeof=0x4
				//+00 DWORD OutUnknown  Out

				if (inBufLength == sizeof(ULONG))
				{
					g_dword_34678 = *(ULONG*)ioBuf;
				}
				else
				{
					//������ȷǷ�
					Status = STATUS_INVALID_PARAMETER;
				}
				break;
			}
			case SAFE_UNKNOWN9:
			{
				//�������ܣ�����dword_34DAC[10]��������
				//���ƺţ�0x22208C
				//buff:     sizeof=0x4        
				//+00 DWORD InSwitch  In 
				if (inBufLength < sizeof(ULONG))
				{
					//������ȷǷ�
					Status = STATUS_INVALID_PARAMETER;
				}
				else
				{
					
					g_dword_3467C = *(ULONG*)ioBuf;
					g_dynData->dword_34DAC[0xA] = *(ULONG*)ioBuf;

				}
				break;
			}
			case SAFE_UNKNOWN6:
			{
				//�������ܣ�R3�����ַ�����R0ĳ��ȫ�ֱ���,�ô�δ֪
				//���ƺţ�0x222094
				//buff:     sizeof=0xN
				//+00 WCHAR BuffPath[X]

				if (inBufLength >= 0x50)
				{
					//����
					RtlZeroMemory(g_UnknownBuffPath, 0x50);
					//����R3����
					RtlCopyMemory(g_UnknownBuffPath, ioBuf, 0x50);
					//�ɹ���1
					g_dword_34D60_Swtich = 1;
				}
				else
				{
					//���볤�ȷǷ�
					Status = STATUS_INVALID_PARAMETER;
				}
				break;
			}
			case SAFE_GET_SYSTEMHOTPATCHINFORMATION_SWITCH:
			{
				//����˵�����֣�
				//Fake_ZwSetSystemInformation���������
				//if ( a2->SystemInformationClass == 0x45 && g_SystemHotpatchInformation_Switch && VersionFlag == 2 )

				//�������ܣ����� or �ر�SystemHotpatchInformation��鲿�֣���Fake_ZwSetSystemInformation��������
				//���ƺţ�0x222098
				//buff:     sizeof=0x4
				//+00 DWORD SystemHotpatchInformation_Switch  In 	

				if (inBufLength == sizeof(ULONG))
				{
					//�û����� ���� or �ر�SystemHotpatchInformation���
					ULONG User_In_SystemHotpatchInformation_Switch = *(ULONG*)ioBuf;
					g_SystemHotpatchInformation_Switch = User_In_SystemHotpatchInformation_Switch;
				}
				else
				{
					//���볤�ȷǷ�
					Status = STATUS_INVALID_PARAMETER;
				}
				break;
			}
			case SAFE_UNKNOWN3:
			{
				//�������ܣ����������̸������㣬���¼��\Registry\Machine\SYSTEM\CurrentControlSet\Services\360SelfProtection·���µ��ֶ��Ƿ���ڣ�
				//���ƺţ�0x22209C
				//buff:     sizeof=0x0
				if (g_x360SelfProtection_Switch)
				{
					Status = STATUS_ACCESS_DENIED;
				}
				else
				{
					UNICODE_STRING RegPath = { 0 };
					RtlInitUnicodeString(&RegPath, L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\360SelfProtection");
					Status = Safe_EnumerateValueKey(&RegPath, 0);
				}
				break;
			}
			case SAFE_UNKNOWN4:
			{
				//�������ܣ�������������
				//���ƺţ�0x2220A0
				//buff:     sizeof=0x0
				g_dynData->dword_34DAC[0] = 0;
				g_dynData->dword_34DAC[1] = 0;
				break;
			}
			case SAFE_UNKNOWN5:
			{
				//�������ܣ�������������
				//���ƺţ�0x2220A4
				//buff:     sizeof=0x0
				g_dynData->dword_34DAC[2] = 0;
				g_dynData->dword_34DAC[3] = 0;
				break;
			}
			case SAFE_SET_ILLEGALITYDLLPATH:
			{
				//�������ܣ�����һ��������DLL·������ClientLoadLibrary����ʱ���ж�
				//���ƺţ�0x2220A8
				//buff:     sizeof=0xN
				//CHAR		DllPath[X]

				//1�����R3���ݵ��ַ������볤�ȣ���󲻳���0xFFFF
				if (inBufLength && inBufLength <= ILLEGALITYDLLPATHMAXSIZE)
				{
					//�Ѵ������˳���ֻ������һ�Σ���������
					if (g_IllegalityDllPath.Buffer)
					{
						Status = STATUS_OBJECT_NAME_COLLISION;
					}
					else
					{
						//2������ռ�洢R3���ݵ�DLL·��
						g_IllegalityDllPath.Buffer = Safe_AllocBuff(NonPagedPool, inBufLength, Tag);
						if (g_IllegalityDllPath.Buffer)
						{
							RtlCopyMemory(g_IllegalityDllPath.Buffer,ioBuf, inBufLength);
							g_IllegalityDllPath.Length = inBufLength;
							g_IllegalityDllPath.MaximumLength = inBufLength + 2;
						}
						else
						{
							Status = STATUS_NO_MEMORY;
						}
					}
				}
				else
				{
					//���볤�ȷǷ�
					Status = STATUS_BUFFER_TOO_SMALL;
				}
				break;
			}
			case SAFE_GET_ISFILTERFUNFILLEDREADY:
			{
				//�������ܣ���ȡ���˺�����״̬����1����Fake������������0����Fake�����رա��������Ϊ��Դ��բ
				//���ƺţ�0x2220AC
				//buff:     sizeof=0x4
				//+00 DWORD Get_IsFilterFunFilledReady  Out
				if (outBufLength == sizeof(ULONG))
				{
					*(ULONG*)ioBuf = g_FilterFun_Rule_table_head_Temp->IsFilterFunFilledReady;
					Irp->IoStatus.Information = sizeof(ULONG);
				}
				else
				{
					//������ȷǷ�
					Status = STATUS_INVALID_PARAMETER;
				}
				break;
			}
			default:
			{
				//���󷵻أ�����������Ч��
				KdPrint(("Unknown ioControlCode:%X\t\n", ioControlCode));
				Status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}
		}
	}
	else
	{
		//�Ǳ������̵��� ֱ�Ӵ��󷵻�
		Status = STATUS_ACCESS_DENIED;
	}
	Irp->IoStatus.Status = Status;							//��ʾIRP���״̬
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Status;
}