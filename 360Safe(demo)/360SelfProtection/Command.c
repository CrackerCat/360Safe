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
	PIO_STACK_LOCATION	IrpStack;
	UNREFERENCED_PARAMETER(DeviceObject);
	//��
	Irp->IoStatus.Status = Status;							//��ʾIRP���״̬
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Status;
}

NTSTATUS Safe_CreateCloseCleanup(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS			Status = STATUS_SUCCESS;
	PIO_STACK_LOCATION	IrpStack;
	UNREFERENCED_PARAMETER(DeviceObject);
	//��
	Irp->IoStatus.Status = Status;							//��ʾIRP���״̬
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Status;
}

NTSTATUS Safe_Read(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS			Status = STATUS_SUCCESS;
	PIO_STACK_LOCATION	IrpStack;
	UNREFERENCED_PARAMETER(DeviceObject);
	//��
	Irp->IoStatus.Status = Status;							//��ʾIRP���״̬
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Status;
}

NTSTATUS Safe_DeviceControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS			Status = STATUS_SUCCESS;
	PIO_STACK_LOCATION	IrpStack;
	UNREFERENCED_PARAMETER(DeviceObject);
	IrpStack = IoGetCurrentIrpStackLocation(Irp);
	//1����������,�����Ǳ�������
	if (Safe_QueryWhitePID(PsGetCurrentProcessId()))
	{
		//��
	}
	Irp->IoStatus.Status = Status;							//��ʾIRP���״̬
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Status;
}