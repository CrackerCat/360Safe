#pragma once
#include <ntifs.h>
#include "MemCheck.h"
#include "Data.h"
#include "WhiteList.h"

#define MAXYEAR		0x7EE		//��ֹ�����޸ı���ʱ�䣬�������������ݲ��ܳ���2030��

//����ϵͳʱ��
//1������ʱ�䳬��2030��ֱ�ӷ��ش���
NTSTATUS NTAPI Fake_ZwSetSystemTime(IN ULONG CallIndex, IN PVOID ArgArray, IN PULONG ret_func, IN PULONG ret_arg);