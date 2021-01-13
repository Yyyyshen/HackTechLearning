#include "HookKeyBoardDriver.h"


NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegPath)
{
	DbgPrint("Enter DriverEntry\n");
	NTSTATUS status = STATUS_SUCCESS;
	ULONG i = 0;

	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObject->MajorFunction[i] = DriverDefaultHandle;
	}

	pDriverObject->DriverUnload = DriverUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = DriverDefaultHandle;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverDefaultHandle;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverControlHandle;

	pDriverObject->MajorFunction[IRP_MJ_READ] = DriverRead;
	pDriverObject->MajorFunction[IRP_MJ_POWER] = DriverPower;

	// 创建键盘设备
	status = CreateDevice(pDriverObject);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("CreateDevice Error[0x%X]\n", status);
		return status;
	}

	// 附件设备到键盘设备栈上
	status = AttachKdbClass(pDriverObject->DeviceObject);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("AttachKdbClass Error[0x%X]\n", status);
		return status;
	}

	DbgPrint("Leave DriverEntry\n");
	return status;
}


VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	DbgPrint("Enter DriverUnload\n");
	PDEVICE_OBJECT pDevObj = pDriverObject->DeviceObject;
	LARGE_INTEGER liDelay = { 0 };

	if (NULL == pDevObj)
	{
		return;
	}
	if (NULL == pDevObj->DeviceExtension)
	{
		return;
	}

	// 先把过滤驱动设备和键盘设备分离, 以免产生新的IRP_MJ_READ
	IoDetachDevice(((PDEVICE_EXTENSION)pDevObj->DeviceExtension)->pAttachDevObj);

	// 对于为完成的IRP，因为只前通过IoSetCompletionRoutine已经设置IO完成例程
	// 那么对于未完成的IRP ，在完成之后会调用 该层设备的函数
	// 需要手动按键, 是pending状态的IRP完成返回
	liDelay.QuadPart = -1000000;
	while (0 < ((PDEVICE_EXTENSION)pDevObj->DeviceExtension)->ulIrpInQuene)
	{
		KdPrint(("剩余挂起IRP:%d\n", ((PDEVICE_EXTENSION)pDevObj->DeviceExtension)->ulIrpInQuene));
		KeDelayExecutionThread(KernelMode, FALSE, &liDelay);
		/*
		// 该方法在Win10上不适用
		if (1 == ((PDEVICE_EXTENSION)pDevObj->DeviceExtension)->ulIrpInQuene)
		{
			//取消掉最后一个IRP
			IoCancelIrp(((PDEVICE_EXTENSION)pDevObj->DeviceExtension)->pLastIrp);
		}
		*/
	}

	UNICODE_STRING ustrSymName;
	RtlInitUnicodeString(&ustrSymName, SYM_NAME);
	IoDeleteSymbolicLink(&ustrSymName);

	if (pDriverObject->DeviceObject)
	{
		IoDeleteDevice(pDriverObject->DeviceObject);
	}

	DbgPrint("Leave DriverUnload\n");
}


NTSTATUS DriverDefaultHandle(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	DbgPrint("Enter DriverDefaultHandle\n");

	NTSTATUS status = STATUS_SUCCESS;

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	DbgPrint("Leave DriverDefaultHandle\n");
	return status;
}


NTSTATUS DriverControlHandle(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	DbgPrint("Enter DriverControlHandle\n");
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIoStackLocation = IoGetCurrentIrpStackLocation(pIrp);
	ULONG ulInputLength = pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength;
	ULONG ulOutputLength = pIoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	ULONG ulControlCode = pIoStackLocation->Parameters.DeviceIoControl.IoControlCode;
	ULONG ulInfo = 0;
	PVOID pBuf = pIrp->AssociatedIrp.SystemBuffer;

	switch (ulControlCode)
	{
	case IOCTL_TEST:
	{
		break;
	}
	default:
		break;
	}

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = ulInfo;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	DbgPrint("Enter DriverControlHandle\n");
	return status;
}


NTSTATUS DriverPower(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	DbgPrint("Enter DriverPower\n");
	NTSTATUS status = STATUS_SUCCESS;

	PoStartNextPowerIrp(pIrp);
	IoSkipCurrentIrpStackLocation(pIrp);
	status = PoCallDriver(((PDEVICE_EXTENSION)pDevObj->DeviceExtension)->pAttachDevObj, pIrp);

	DbgPrint("Leave DriverPower\n");
	return status;
}


NTSTATUS DriverRead(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	DbgPrint("\nEnter DriverRead\n");
	NTSTATUS status = STATUS_SUCCESS;

	// 复制 pIrp 的 IO_STACK_LOCATION 到下一设备
	IoCopyCurrentIrpStackLocationToNext(pIrp);

	// 设置完成实例
	IoSetCompletionRoutine(pIrp, ReadCompleteRoutine, pDevObj, TRUE, TRUE, TRUE);

	// 记录IRP数量
	((PDEVICE_EXTENSION)pDevObj->DeviceExtension)->ulIrpInQuene++;

	// 发送IRP到下一设备上
	status = IoCallDriver(((PDEVICE_EXTENSION)pDevObj->DeviceExtension)->pAttachDevObj, pIrp);

	DbgPrint("Leave DriverRead\n");
	return status;
}


NTSTATUS ReadCompleteRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp, PVOID pContext)
{
	NTSTATUS status = pIrp->IoStatus.Status;
	PKEYBOARD_INPUT_DATA pKeyboardInputData = NULL;
	ULONG ulKeyCount = 0, i = 0;

	if (NT_SUCCESS(status))
	{
		pKeyboardInputData = (PKEYBOARD_INPUT_DATA)pIrp->AssociatedIrp.SystemBuffer;
		ulKeyCount = (ULONG)pIrp->IoStatus.Information / sizeof(KEYBOARD_INPUT_DATA);

		// 获取按键数据
		for (i = 0; i < ulKeyCount; i++)
		{
			// Key Press
			if (KEY_MAKE == pKeyboardInputData[i].Flags)
			{
				// 按键扫描码
				DbgPrint("[Down][0x%X]\n", pKeyboardInputData[i].MakeCode);
			}
			// Key Release
			else if (KEY_BREAK == pKeyboardInputData[i].Flags)
			{
				// 按键扫描码
				DbgPrint("[Up][0x%X]\n", pKeyboardInputData[i].MakeCode);
			}
			//可以添加上这这一句，然后按键全部被改为了 按下 A
//			pKeyboardInputData[i].MakeCode = 0x1e;
		}
	}

	if (pIrp->PendingReturned)
	{
		IoMarkIrpPending(pIrp);
	}

	// 减少IRP在队列的数量
	((PDEVICE_EXTENSION)pDevObj->DeviceExtension)->ulIrpInQuene--;

	status = pIrp->IoStatus.Status;
	return status;
}


NTSTATUS CreateDevice(PDRIVER_OBJECT pDriverObject)
{
	DbgPrint("Enter CreateDevice\n");
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING ustrDevName, ustrSymName;
	PDEVICE_OBJECT pDevObj = NULL;

	RtlInitUnicodeString(&ustrDevName, DEV_NAME);
	RtlInitUnicodeString(&ustrSymName, SYM_NAME);

	// 创建键盘设备
	status = IoCreateDevice(pDriverObject, sizeof(DEVICE_EXTENSION), &ustrDevName, FILE_DEVICE_KEYBOARD, 0, FALSE, &pDevObj);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("IoCreateDevice Error[0x%X]\n", status);
		return status;
	}

	// 创建符号连接
	status = IoCreateSymbolicLink(&ustrSymName, &ustrDevName);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("IoCreateSymbolicLink Error[0x%X]\n", status);
		return status;
	}

	DbgPrint("Enter CreateDevice\n");
	return status;
}


NTSTATUS AttachKdbClass(PDEVICE_OBJECT pDevObj)
{
	DbgPrint("Enter HookKdbClass\n");
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING ustrObjectName;
	PFILE_OBJECT pFileObj = NULL;
	PDEVICE_OBJECT pKeyboardClassDeviceObject = NULL, pAttachDevObj = NULL;

	RtlInitUnicodeString(&ustrObjectName, L"\\Device\\KeyboardClass0");

	// 获取键盘设备对象的指针
	status = IoGetDeviceObjectPointer(&ustrObjectName, GENERIC_READ | GENERIC_WRITE, &pFileObj, &pKeyboardClassDeviceObject);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("IoGetDeviceObjectPointer Error[0x%X]\n", status);
		return status;
	}
	// 减少引用
	ObReferenceObject(pFileObj);

	// 将当前设备附加到键盘设备的设备栈顶上, 返回的是原来设备栈上的栈顶设备, 即当前设栈上附加设备的下一个设备
	pAttachDevObj = IoAttachDeviceToDeviceStack(pDevObj, pKeyboardClassDeviceObject);
	if (NULL == pAttachDevObj)
	{
		DbgPrint("IoAttachDeviceToDeviceStack Error\n");
		return STATUS_UNSUCCESSFUL;
	}

	// 设置此设备的标志，要与附加到的设备栈上的设备标志保持一致
	pDevObj->Flags = pDevObj->Flags | DO_BUFFERED_IO | DO_POWER_PAGABLE;
	pDevObj->ActiveThreadCount = pDevObj->ActiveThreadCount & (~DO_DEVICE_INITIALIZING);


	// 保存下一设备到DeviceExtension
	((PDEVICE_EXTENSION)pDevObj->DeviceExtension)->pAttachDevObj = pAttachDevObj;
	((PDEVICE_EXTENSION)pDevObj->DeviceExtension)->ulIrpInQuene = 0;

	DbgPrint("Leave HookKdbClass\n");
	return status;
}







