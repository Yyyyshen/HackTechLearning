#include "TdiTcp.h"


VOID ShowError(PCHAR lpszText, NTSTATUS ntStatus)
{
	KdPrint(("%s Error!\nError Code Is: 0x%8X\n", lpszText, ntStatus));
}


// 完成回调函数
NTSTATUS TdiCompletionRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp, PVOID pContext)
{
	DbgPrint("Enter TDICompletionRoutine\n");

	if (NULL != pContext)
	{
		KeSetEvent((PKEVENT)pContext, IO_NO_INCREMENT, FALSE);
	}

	DbgPrint("Leave TDICompletionRoutine\n");
	return STATUS_MORE_PROCESSING_REQUIRED;
}


// TDI初始化设置
NTSTATUS TdiOpen(PDEVICE_OBJECT *ppTdiAddressDevObj, PFILE_OBJECT *ppTdiEndPointFileObject, HANDLE *phTdiAddress, HANDLE *phTdiEndPoint)
{
	DbgPrint("Enter OpenTdi\n");

	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PFILE_FULL_EA_INFORMATION pAddressEaBuffer = NULL;
	ULONG ulAddressEaBufferLength = 0;
	PTA_IP_ADDRESS pTaIpAddr = NULL;
	UNICODE_STRING ustrTDIDevName;
	OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
	IO_STATUS_BLOCK iosb = { 0 };
	HANDLE hTdiAddress = NULL;
	PFILE_OBJECT pTdiAddressFileObject = NULL;
	PDEVICE_OBJECT pTdiAddressDevObj = NULL;
	PFILE_FULL_EA_INFORMATION pContextEaBuffer = NULL;
	ULONG ulContextEaBufferLength = 0;
	HANDLE hTdiEndPoint = NULL;
	PFILE_OBJECT pTdiEndPointFileObject = NULL;
	KEVENT irpCompleteEvent = { 0 };
	PIRP pIrp = NULL;	

	do
	{
		// 为本地地址拓展属性结构申请内存及初始化
		ulAddressEaBufferLength = sizeof(FILE_FULL_EA_INFORMATION) + TDI_TRANSPORT_ADDRESS_LENGTH + sizeof(TA_IP_ADDRESS);
		pAddressEaBuffer = (PFILE_FULL_EA_INFORMATION)ExAllocatePool(NonPagedPool, ulAddressEaBufferLength);
		if (NULL == pAddressEaBuffer)
		{
			ShowError("ExAllocatePool[Address]", 0);
			break;
		}
		RtlZeroMemory(pAddressEaBuffer, ulAddressEaBufferLength);
		RtlCopyMemory(pAddressEaBuffer->EaName, TdiTransportAddress, (1 + TDI_TRANSPORT_ADDRESS_LENGTH)); 
		pAddressEaBuffer->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
		pAddressEaBuffer->EaValueLength = sizeof(TA_IP_ADDRESS);
		// 初始化本机IP地址与端口
		pTaIpAddr = (PTA_IP_ADDRESS)((PUCHAR)pAddressEaBuffer->EaName + pAddressEaBuffer->EaNameLength + 1);
		pTaIpAddr->TAAddressCount = 1;
		pTaIpAddr->Address[0].AddressLength = TDI_ADDRESS_LENGTH_IP;
		pTaIpAddr->Address[0].AddressType = TDI_ADDRESS_TYPE_IP;
		pTaIpAddr->Address[0].Address[0].sin_port = 0;     // 0表示本机任意随机端口
		pTaIpAddr->Address[0].Address[0].in_addr = 0;      // 0表示本机本地IP地址
		RtlZeroMemory(pTaIpAddr->Address[0].Address[0].sin_zero, sizeof(pTaIpAddr->Address[0].Address[0].sin_zero));
		// 创建TDI驱动设备字符串与初始化设备对象
		RtlInitUnicodeString(&ustrTDIDevName, COMM_TCP_DEV_NAME);
		InitializeObjectAttributes(&ObjectAttributes, &ustrTDIDevName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
		// 根据本地地址拓展属性结构创建本地地址对象
		status = ZwCreateFile(&hTdiAddress, GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
			&ObjectAttributes, &iosb, NULL, FILE_ATTRIBUTE_NORMAL,
			FILE_SHARE_READ, FILE_OPEN, 0, pAddressEaBuffer, ulAddressEaBufferLength);
		if (!NT_SUCCESS(status))
		{
			ShowError("ZwCreateFile[Address]", status);
			break;
		}
		// 根据本地地址对象句柄获取对应的本地地址文件对象
		status = ObReferenceObjectByHandle(hTdiAddress,
			FILE_ANY_ACCESS, 0, KernelMode, &pTdiAddressFileObject, NULL);
		if (!NT_SUCCESS(status))
		{
			ShowError("ObReferenceObjectHandle[EndPoint]", status);
			break;
		}
		
		// 获取本地地址文件对象对应的驱动设备
		pTdiAddressDevObj = IoGetRelatedDeviceObject(pTdiAddressFileObject);
		if (NULL == pTdiAddressDevObj)
		{
			ShowError("IoGetRelatedDeviceObject", 0);
			break;
		}

		// 为上下文拓展属性申请内存并初始化
		ulContextEaBufferLength = FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName) + TDI_CONNECTION_CONTEXT_LENGTH + 1 + sizeof(CONNECTION_CONTEXT);
		pContextEaBuffer = (PFILE_FULL_EA_INFORMATION)ExAllocatePool(NonPagedPool, ulContextEaBufferLength);
		if (NULL == pContextEaBuffer)
		{
			ShowError("ExAllocatePool[EndPoint]", 0);
			break;
		}
		RtlZeroMemory(pContextEaBuffer, ulContextEaBufferLength);
		RtlCopyMemory(pContextEaBuffer->EaName, TdiConnectionContext, (1 + TDI_CONNECTION_CONTEXT_LENGTH));
		pContextEaBuffer->EaNameLength = TDI_CONNECTION_CONTEXT_LENGTH;
		pContextEaBuffer->EaValueLength = sizeof(CONNECTION_CONTEXT);
		// 根据上下文创建TDI端点对象
		status = ZwCreateFile(&hTdiEndPoint, GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
			&ObjectAttributes, &iosb, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ,
			FILE_OPEN, 0, pContextEaBuffer, ulContextEaBufferLength);
		if (!NT_SUCCESS(status))
		{
			ShowError("ZwCreateFile[EndPoint]", status);
			break;
		}
		// 根据TDI端点对象句柄获取对应的端点文件对象
		status = ObReferenceObjectByHandle(hTdiEndPoint,
			FILE_ANY_ACCESS, 0, KernelMode, &pTdiEndPointFileObject, NULL);
		if (!NT_SUCCESS(status))
		{
			ShowError("ObReferenceObjectHandle[EndPoint]", status);
			break;
		}

		// 设置事件
		KeInitializeEvent(&irpCompleteEvent, NotificationEvent, FALSE);

		// 将TDI端点与本地地址对象关联, 创建TDI的I/O请求包:TDI_ASSOCIATE_ADDRESS
		pIrp = TdiBuildInternalDeviceControlIrp(TDI_ASSOCIATE_ADDRESS,
			pTdiAddressDevObj, pTdiEndPointFileObject, &irpCompleteEvent, &iosb);
		if (NULL == pIrp)
		{
			ShowError("TdiBuildInternalDeviceControlIrp", 0);
			break;
		}
		// 拓展I/O请求包
		TdiBuildAssociateAddress(pIrp, pTdiAddressDevObj, pTdiEndPointFileObject, NULL, NULL, hTdiAddress);

		// 设置完成实例的回调函数
		IoSetCompletionRoutine(pIrp, TdiCompletionRoutine, &irpCompleteEvent, TRUE, TRUE, TRUE);

		// 发送I/O请求包并等待执行
		status = IoCallDriver(pTdiAddressDevObj, pIrp);
		if (STATUS_PENDING == status)
		{
			KeWaitForSingleObject(&irpCompleteEvent, Executive, KernelMode, FALSE, NULL);
		}

		// 返回数据
		*ppTdiAddressDevObj = pTdiAddressDevObj;
		*ppTdiEndPointFileObject = pTdiEndPointFileObject;
		*phTdiAddress = hTdiAddress;
		*phTdiEndPoint = hTdiEndPoint;

	}while (FALSE);

	// 释放内存
	if (pTdiAddressFileObject)
	{
        ObDereferenceObject(pTdiAddressFileObject);    // 测试
	}
	if (pContextEaBuffer)
	{
		ExFreePool(pContextEaBuffer);
	}
	if (pAddressEaBuffer)
	{
		ExFreePool(pAddressEaBuffer);
	}

	DbgPrint("Leave OpenTdi\n");
	return status;
}


// TDI TCP连接服务器
NTSTATUS TdiConnection(PDEVICE_OBJECT pTdiAddressDevObj, PFILE_OBJECT pTdiEndPointFileObject, LONG *pServerIp, LONG lServerPort)
{
	DbgPrint("Enter TdiConnection\n");

	NTSTATUS status = STATUS_SUCCESS;
	IO_STATUS_BLOCK iosb = { 0 };
	PIRP pIrp = NULL;
	KEVENT connEvent = { 0 };
	TA_IP_ADDRESS serverTaIpAddr = { 0 };
	ULONG serverIpAddr = 0;
	USHORT serverPort = 0;
	TDI_CONNECTION_INFORMATION serverConnection = { 0 };

	// 创建连接事件
	KeInitializeEvent(&connEvent, NotificationEvent, FALSE);

	// 创建TDI连接I/O请求包:TDI_CONNECT
	pIrp = TdiBuildInternalDeviceControlIrp(TDI_CONNECT, pTdiAddressDevObj, pTdiEndPointFileObject, &connEvent, &iosb);
	if (NULL == pIrp)
	{
		ShowError("TdiBuildInternalDeviceControlIrp_TDI_CONNECT", 0);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	// 初始化服务器IP地址与端口
	serverIpAddr = INETADDR(pServerIp[0], pServerIp[1], pServerIp[2], pServerIp[3]);
	serverPort = HTONS(lServerPort);
	serverTaIpAddr.TAAddressCount = 1;
	serverTaIpAddr.Address[0].AddressLength = TDI_ADDRESS_LENGTH_IP;
	serverTaIpAddr.Address[0].AddressType = TDI_ADDRESS_TYPE_IP;
	serverTaIpAddr.Address[0].Address[0].sin_port = serverPort;
	serverTaIpAddr.Address[0].Address[0].in_addr = serverIpAddr;
	serverConnection.UserDataLength = 0;
	serverConnection.UserData = 0;
	serverConnection.OptionsLength = 0;
	serverConnection.Options = 0;
	serverConnection.RemoteAddressLength = sizeof(TA_IP_ADDRESS);
	serverConnection.RemoteAddress = &serverTaIpAddr;

	// 把上述的地址与端口信息增加到I/O请求包中,增加连接信息
	TdiBuildConnect(pIrp, pTdiAddressDevObj, pTdiEndPointFileObject, NULL, NULL, NULL, &serverConnection, 0);

	// 设置完成实例回调函数
	IoSetCompletionRoutine(pIrp, TdiCompletionRoutine, &connEvent, TRUE, TRUE, TRUE);

	// 发送I/O请求包并等待执行
	status = IoCallDriver(pTdiAddressDevObj, pIrp);
	if (STATUS_PENDING == status)
	{
		KeWaitForSingleObject(&connEvent, Executive, KernelMode, FALSE, NULL);
	}

	DbgPrint("Leave TdiConnection\n");
	return status;
}


// TDI TCP发送信息
NTSTATUS TdiSend(PDEVICE_OBJECT pTdiAddressDevObj, PFILE_OBJECT pTdiEndPointFileObject, PUCHAR pSendData, ULONG ulSendDataLength)
{
	DbgPrint("Enter TdiSend\n");

	NTSTATUS status = STATUS_SUCCESS;
	KEVENT sendEvent;
	PIRP pIrp = NULL;
	IO_STATUS_BLOCK iosb = {0};
	PMDL pSendMdl = NULL;

	// 初始化事件
	KeInitializeEvent(&sendEvent, NotificationEvent, FALSE);

	// 创建I/O请求包:TDI_SEND
	pIrp = TdiBuildInternalDeviceControlIrp(TDI_SEND, pTdiAddressDevObj, pTdiEndPointFileObject, &sendEvent, &iosb);
	if (NULL == pIrp)
	{
		ShowError("TdiBuildInternalDeviceControlIrp", 0);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	// 创建MDL
	pSendMdl = IoAllocateMdl(pSendData, ulSendDataLength, FALSE, FALSE, pIrp);
	if (NULL == pSendMdl)
	{
		ShowError("IoAllocateMdl", 0);
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	MmProbeAndLockPages(pSendMdl, KernelMode, IoModifyAccess);

	// 拓展I/O请求包,添加发送信息
	TdiBuildSend(pIrp, pTdiAddressDevObj, pTdiEndPointFileObject, NULL, NULL, pSendMdl, 0, ulSendDataLength);

	// 设置完成实例回调函数
	IoSetCompletionRoutine(pIrp, TdiCompletionRoutine, &sendEvent, TRUE, TRUE, TRUE);

	// 发送I/O请求包并等待执行
	status = IoCallDriver(pTdiAddressDevObj, pIrp);
	if (STATUS_PENDING == status)
	{
		KeWaitForSingleObject(&sendEvent, Executive, KernelMode, FALSE, NULL);
	}

	// 释放MDL
	if (pSendMdl)
	{
		IoFreeMdl(pSendMdl);
	}

	DbgPrint("Leave TdiSend\n");
	return status;
}


// TDI TCP接收信息
ULONG_PTR TdiRecv(PDEVICE_OBJECT pTdiAddressDevObj, PFILE_OBJECT pTdiEndPointFileObject, PUCHAR pRecvData, ULONG ulRecvDataLength)
{
	DbgPrint("Enter TdiRecv\n");

	NTSTATUS status = STATUS_SUCCESS;
	KEVENT recvEvent;
	PIRP pIrp = NULL;
	IO_STATUS_BLOCK iosb = { 0 };
	PMDL pRecvMdl = NULL;
	ULONG_PTR ulRecvSize = 0;

	// 初始化事件
	KeInitializeEvent(&recvEvent, NotificationEvent, FALSE);

	// 创建I/O请求包:TDI_SEND
	pIrp = TdiBuildInternalDeviceControlIrp(TDI_RECV, pTdiAddressDevObj, pTdiEndPointFileObject, &recvEvent, &iosb);
	if (NULL == pIrp)
	{
		ShowError("TdiBuildInternalDeviceControlIrp", 0);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	// 创建MDL
	pRecvMdl = IoAllocateMdl(pRecvData, ulRecvDataLength, FALSE, FALSE, pIrp);
	if (NULL == pRecvMdl)
	{
		ShowError("IoAllocateMdl", 0);
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	MmProbeAndLockPages(pRecvMdl, KernelMode, IoModifyAccess);

	// 拓展I/O请求包,添加发送信息
	TdiBuildReceive(pIrp, pTdiAddressDevObj, pTdiEndPointFileObject, NULL, NULL, pRecvMdl, TDI_RECEIVE_NORMAL, ulRecvDataLength);

	// 设置完成实例回调函数
	IoSetCompletionRoutine(pIrp, TdiCompletionRoutine, &recvEvent, TRUE, TRUE, TRUE);

	// 发送I/O请求包并等待执行
	status = IoCallDriver(pTdiAddressDevObj, pIrp);
	if (STATUS_PENDING == status)
	{
		KeWaitForSingleObject(&recvEvent, Executive, KernelMode, FALSE, NULL);
	}

	// 获取实际接收的数据大小
	ulRecvSize = pIrp->IoStatus.Information;

	// 释放MDL
	if (pRecvMdl)
	{
		IoFreeMdl(pRecvMdl);
	}

	DbgPrint("Leave TdiRecv\n");
	return status;
}


// TDI关闭释放
VOID TdiClose(PFILE_OBJECT pTdiEndPointFileObject, HANDLE hTdiAddress, HANDLE hTdiEndPoint)
{
	DbgPrint("Enter TdiClose\n");

	if (pTdiEndPointFileObject)
	{
		ObDereferenceObject(pTdiEndPointFileObject);
	}
	if (hTdiEndPoint)
	{
		ZwClose(hTdiEndPoint);
	}
	if (hTdiAddress)
	{
		ZwClose(hTdiAddress);
	}

	DbgPrint("Leave TdiClose\n");
}
