/*
�ο����ϣ�
1��disallowrun ��ֹ�������
��ַ��http://www.cppblog.com/nt05/archive/2008/06/16/53490.html
2��autorun.inf������ɱ������޷�����,��ӳ��ٳ֣�Image File Execution Options������취
��ַ��https://blog.csdn.net/ananias/article/details/1642375?utm_medium=distribute.pc_relevant.none-task-blog-baidujs-3
*/
#include "Fake_ZwReplaceKey.h"



//ȡ��ע���ֵ��
NTSTATUS NTAPI Fake_ZwReplaceKey(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg)
{
	NTSTATUS    Status, result;
	result = STATUS_SUCCESS;
	//0����ȡZwReplaceKeyԭʼ����
	HANDLE      In_KeyHandle = *(ULONG*)((ULONG)ArgArray + 4);
	//1��������Ӧ�ò����
	if (ExGetPreviousMode())
	{
		//��ֹ�޸��ܱ���ע�������
		//����ֵ���Ϸ�����TRUE�����Ϸ�����FALSE
		result = Safe_ProtectRegKey(In_KeyHandle, 0, 0, 0, 0, 0) != STATUS_SUCCESS ? STATUS_SUCCESS : STATUS_ACCESS_DENIED;
	}
	else
	{
		result = STATUS_SUCCESS;
	}
	return result;
}