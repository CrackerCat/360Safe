//����DLLע���,��ֹDLL�ٳ�
//�ο����ϣ�
//1��һ���ƹ�ȫ�ֹ��Ӱ�װ���ص�˼·
//��ַ��https://bbs.pediy.com/thread-92717.htm
#include "Fake_KeUserModeCallback.h"

NTSTATUS NTAPI Fake_ClientLoadLibrary(ULONG CallIndex, PVOID ArgArray, PULONG ret_func, PULONG ret_arg)
{
	NTSTATUS	result = STATUS_SUCCESS;
	UNICODE_STRING prl_hookString = { 0 };
	UNICODE_STRING DestinationString = { 0 };
	UNICODE_STRING ptrDllString = { 0 };
	 ULONG MsctfDllSize = 5;
	 ULONG MsctfDllPathMinSize = 0x12;
	 ULONG prl_hookDllSize = 8;
	 ULONG prl_hookPathMinSize = 0x18;
	 WCHAR szDllName[MAX_PATH] = { 0 };
	 WCHAR Wildcard[] = { L"\\??\\" };
	 WCHAR SystemRootWildcard[] = { L"\\SystemRoot\\system32" };
	 PCLientLoadLibraryParam pCLientLoadLibraryParam = NULL;
	 SYSTEM_INFORMATIONFILE_XOR System_InformationFile = { 0 };		
	 RtlInitUnicodeString(&prl_hookString, L"C:\\Program Files\\Parallels\\Parallels Tools\\Services\\prl_hook.dll");
	 //��ClientLoadLibrary���������
	 IN PVOID In_InputBuffer = *(ULONG*)((ULONG)ArgArray + 4);
	 IN PVOID In_InputLength = *(ULONG*)((ULONG)ArgArray + 8);
	//�ж��ǲ��Ǳ������̣��Ƿ��أ�1  ���Ƿ���0
	result = Safe_QueryWhitePID(PsGetCurrentProcessId());
	if (result)
	{
		//�жϳ���
		if (!In_InputLength)
		{
			return STATUS_SUCCESS;
		}
		//����Ҫ�Ƚϵ�DLL�ַ�����Ϣ
		pCLientLoadLibraryParam = (PCLientLoadLibraryParam)In_InputBuffer;
		ptrDllString.MaximumLength = pCLientLoadLibraryParam->MaximumLength;
		ptrDllString.Buffer = ((ULONG)In_InputBuffer + (ULONG)pCLientLoadLibraryParam->ptrDllString);		//����+ƫ��
		ptrDllString.Length = 2 * wcslen(ptrDllString.Buffer);
		if (ptrDllString.Length >= MsctfDllPathMinSize)
		{
			//�ο�MJ0011
			//��ֹ�޸�msctf�ƹ���ȫ�����ȫ�ֹ�������
			if (_wcsnicmp((PWSTR)((CHAR *)ptrDllString.Buffer + (ptrDllString.Length - MsctfDllPathMinSize)), L"msctf.dll", MsctfDllSize) == 0)
			{
				if (wcschr(ptrDllString.Buffer, '\\'))
				{
					//���� //??//ͨ���
					RtlCopyMemory(szDllName, Wildcard, wcslen(Wildcard) * 2);
					RtlCopyMemory((PVOID)(szDllName + 4), ptrDllString.Buffer, ptrDllString.Length);
					RtlInitUnicodeString(&DestinationString, (PCWSTR)szDllName);
				}
				else
				{
					//����\\SystemRoot\\system32ͨ���
					RtlCopyMemory(szDllName, SystemRootWildcard, wcslen(SystemRootWildcard) * 2);
					RtlCopyMemory((PVOID)(szDllName + 0x14), ptrDllString.Buffer, ptrDllString.Length);
					RtlInitUnicodeString(&DestinationString, (PCWSTR)szDllName);
				}
				//����DLL��ֹ�����޸���У��
				result = Safe_KernelCreateFile(&DestinationString, (ULONG)&System_InformationFile);// ��ֹ�����޸���У��
				if (!NT_SUCCESS(result))
				{
					return result;
				}
				//������й�����DLL�Ƿ��޸�(msctf.dll)
				if ((System_InformationFile.IndexNumber_LowPart == g_System_InformationFile_Data[SYSTEMROOT_SYSTEM32_MSCTF_DLL].IndexNumber_LowPart) &&
					(System_InformationFile.VolumeSerialNumber == g_System_InformationFile_Data[SYSTEMROOT_SYSTEM32_MSCTF_DLL].VolumeSerialNumber) &&
					(System_InformationFile.u.IndexNumber_HighPart == g_System_InformationFile_Data[SYSTEMROOT_SYSTEM32_MSCTF_DLL].u.IndexNumber_HighPart)
					)
				{
					//�ɹ�����
					return STATUS_SUCCESS;
				}
				else
				{
					//ʧ�ܷ��أ�msctf.dll�������޸�
					return STATUS_ACCESS_DENIED;
				}
			}
		}
		//prl_hook.dll����������,���ŵ�DLL�����������������׵�С�����Ը�������
		if ((ptrDllString.Length >= prl_hookPathMinSize) && (_wcsnicmp((PWSTR)((CHAR *)ptrDllString.Buffer + (ptrDllString.Length - prl_hookPathMinSize)), L"prl_hook.dll", prl_hookDllSize) == 0))
		{
			if (!RtlEqualUnicodeString(&prl_hookString, &ptrDllString, TRUE) || !g_Thread_Information.UniqueProcessId)
			{
				return STATUS_ACCESS_DENIED;
			}
			else
			{
				return STATUS_SUCCESS;
			}
			
		}
		//������յĺ���������������������
		result = RtlEqualUnicodeString(&ptrDllString, &g_IllegalityDllPath, TRUE) != 0 ? STATUS_SUCCESS : STATUS_ACCESS_DENIED;
	}
	//�Ǳ�������ֱ���˳�
	return result;
}