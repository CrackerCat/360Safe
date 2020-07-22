#include "Fake_ZwSetSystemTime.h"

//����ϵͳʱ��
//1������ʱ�䳬��2030��ֱ�ӷ��ش���
NTSTATUS NTAPI Fake_ZwSetSystemTime(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS       result = STATUS_SUCCESS;
	NTSTATUS       Status = STATUS_SUCCESS;
	TIME_FIELDS    Out_TimeFields = { 0 };
	LARGE_INTEGER  Out_LocalTime = { 0 };
	ULONG          SpecialWhiteNumber = NULL;
	ULONG          Tag = 0x206B6444;
	ULONG          Flag_v6 = NULL;
	PQUERY_PASS_R0SENDR3_DATA  pQuery_Pass = NULL;
	SpecialWhiteNumber = g_SpecialWhite_List.SpecialWhiteListNumber;
	//0����ȡZwSetSystemTimeԭʼ����
	PLARGE_INTEGER In_NewTime = *(ULONG*)((ULONG)ArgArray);
	if (In_NewTime)
	{
		//�жϲ����Ϸ���
		if (myProbeRead(In_NewTime, sizeof(LARGE_INTEGER), sizeof(CHAR)))
		{
			KdPrint(("ProbeRead(Fake_ZwSetSystemTime��In_NewTime) error \r\n"));
			return result;
		}
		//��ȡ����ʱ��
		ExSystemTimeToLocalTime(In_NewTime, &Out_LocalTime);
		//��ϵͳʱ��ת����һ��TIME_FIELDS�ṹ
		RtlTimeToTimeFields(&Out_LocalTime, &Out_TimeFields);
		//ʱ�䳬��XXX��ֱ�ӱ���
		if (Out_TimeFields.Year > MAXYEAR
			&& SpecialWhiteNumber							//�ж�R3��������������
			)
		{
			//new�ռ䣬���洫�ݸ�R3�Ľ�������
			pQuery_Pass = (PQUERY_PASS_R0SENDR3_DATA)Safe_AllocBuff(NonPagedPool, sizeof(QUERY_PASS_R0SENDR3_DATA), Tag);
			if (!pQuery_Pass)
			{
				result = STATUS_ACCESS_DENIED;
			}
			//������ݣ���������R3���Ի������û����� ����or����
			pQuery_Pass->Unknown_CurrentThreadId_5 = PsGetCurrentThreadId();
			pQuery_Pass->Unknown_Flag_2 = 0xA;
			pQuery_Pass->CheckWhitePID = PsGetCurrentProcessId();
			pQuery_Pass->Unknown_CurrentThreadId_4 = PsGetCurrentThreadId();
			pQuery_Pass->Unknown_Flag_6 = 1;
			//Flag_v6 = Safe_push_request_in_and_waitfor_finish(pQuery_Pass, 1);
			//���أ�0 ���У�1 or 2 ����
			if (Flag_v6 == 1 || Flag_v6 == 2)
			{
				result = STATUS_ACCESS_DENIED;
			}
			else
			{
				result = STATUS_SUCCESS;
			}
		}

	}
	return result;
}