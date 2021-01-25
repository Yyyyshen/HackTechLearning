/**
 * WFP网络监控
 * 不稳定，偶尔蓝屏  irql not less or equal
 * 
 * 2021/01/25 学习了另一本书的WFP内容，测试例子通过，并无蓝屏情况，回来看一下这个demo
 */

#include "WFP_Network.h"

//需要创建设备对象，否则callout函数会注册失败 FwpsCalloutRegister 返回错误码 0xC022001C
#define DEV_NAME L"\\Device\\MY_WFP_DEV_NAME"
#define SYM_NAME L"\\DosDevices\\MY_WFP_SYM_NAME"

NTSTATUS CreateDevice(PDRIVER_OBJECT pDriverObject)
{
	DbgPrint("Enter CreateDevice\n");
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT pDevObj = NULL;
	UNICODE_STRING ustrDevName, ustrSymName;
	RtlInitUnicodeString(&ustrDevName, DEV_NAME);
	RtlInitUnicodeString(&ustrSymName, SYM_NAME);

	status = IoCreateDevice(pDriverObject, 0, &ustrDevName, FILE_DEVICE_NETWORK, 0, FALSE, &pDevObj);
	if (!NT_SUCCESS(status))
	{
		ShowError("IoCreateDevice", status);
		return status;
	}
	status = IoCreateSymbolicLink(&ustrSymName, &ustrDevName);
	if (!NT_SUCCESS(status))
	{
		ShowError("IoCreateSymbolicLink", status);
		return status;
	}

	DbgPrint("Leave CreateDevice\n");
	return status;
}

VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	// 删除回调函数和过滤器,关闭引擎
	WfpUnload();

	// 删除设备
	UNICODE_STRING ustrSymName;
	RtlInitUnicodeString(&ustrSymName, SYM_NAME);
	IoDeleteSymbolicLink(&ustrSymName);
	if (pDriverObject->DeviceObject)
	{
		IoDeleteDevice(pDriverObject->DeviceObject);
	}
}

NTSTATUS DriverDefaultHandle(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return status;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING RegistryPath)
{
	DbgPrint("Enter DriverEntry\n");
	NTSTATUS status = STATUS_SUCCESS;

	pDriverObject->DriverUnload = DriverUnload;
	for (ULONG i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObject->MajorFunction[i] = DriverDefaultHandle;
	}

	// 创建设备
	CreateDevice(pDriverObject);

	// 启动WFP
	WfpLoad(pDriverObject->DeviceObject);

	DbgPrint("Leave DriverEntry\n");
	return status;
}