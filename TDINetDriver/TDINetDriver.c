#include "TDINetDriver.h"
#include "TdiTcp.h"
#include <ntstatus.h>

/**
 * 驱动层网络通信
 * 配合ChatServer使用测试
 * 
 * 内核中，主要使用 传输数据接口（Transport Data Interface,TDI）和 网络驱动接口规范（Network Driver Interface Specification,NDIS）实现
 * TDI直接使用现用TCP/IP通信，无需重新编写协议，网络防火墙可以检测到基于TDI开发的网络通信
 * NDIS可以直接在网络上读写原始报文，需要自定义通信协议，所以能绕过防火墙检测
 */

typedef struct _MY_DATA
{
	PDEVICE_OBJECT pTdiAddressDevObj;
	PFILE_OBJECT pTdiEndPointFileObject;
	HANDLE hTdiAddress;
	HANDLE hTdiEndPoint;
}MY_DATA, *PMY_DATA;


// 接收信息多线程
VOID RecvThreadProc(_In_ PVOID StartContext)
{
	PMY_DATA pMyData = (PMY_DATA)StartContext;
	NTSTATUS status = STATUS_SUCCESS;
	char szRecvData[1024] = { 0 };
	ULONG ulRecvDataLenngth = 1024;
	RtlZeroMemory(szRecvData, ulRecvDataLenngth);

	// TDI TCP接收信息
	do
	{
		ulRecvDataLenngth = TdiRecv(pMyData->pTdiAddressDevObj, pMyData->pTdiEndPointFileObject, szRecvData, ulRecvDataLenngth);
		if (0 < ulRecvDataLenngth)
		{
			DbgPrint("[TDI Recv]%s\n", szRecvData);
			break;;
		}

	} while (TRUE);

	// 释放
	TdiClose(pMyData->pTdiEndPointFileObject, pMyData->hTdiAddress, pMyData->hTdiEndPoint);
	ExFreePool(pMyData);
}


// TDI 通信测试
VOID TdiTest()
{
	NTSTATUS status = STATUS_SUCCESS;
	HANDLE hTdiAddress = NULL;
	HANDLE hTdiEndPoint = NULL;
	PDEVICE_OBJECT pTdiAddressDevObj = NULL;
	PFILE_OBJECT pTdiEndPointFileObject = NULL;
	LONG pServerIp[4] = {127, 0, 0, 1};
	LONG lServerPort = 12345;
	UCHAR szSendData[] = "I am Demon`Gan--->From TDI";
	ULONG ulSendDataLength = 1 + strlen(szSendData);
	HANDLE hThread = NULL;

	// TDI初始化
	status = TdiOpen(&pTdiAddressDevObj, &pTdiEndPointFileObject, &hTdiAddress, &hTdiEndPoint);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("TdiOpen Error[0x%X]\n", status);
		return;
	}
	// TDI TCP连接服务器
	status = TdiConnection(pTdiAddressDevObj, pTdiEndPointFileObject, pServerIp, lServerPort);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("TdiConnection Error[0x%X]\n", status);
		return;
	}
	// TDI TCP发送信息
	status = TdiSend(pTdiAddressDevObj, pTdiEndPointFileObject, szSendData, ulSendDataLength);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("TdiSend Error[0x%X]\n", status);
		return;
	}
	DbgPrint("[TDI Send]%s\n", szSendData);
	// 创建接收信息多线程, 循环接收信息
	PMY_DATA pMyData = ExAllocatePool(NonPagedPool, sizeof(MY_DATA));
	pMyData->pTdiAddressDevObj = pTdiAddressDevObj;
	pMyData->pTdiEndPointFileObject = pTdiEndPointFileObject;
	pMyData->hTdiAddress = hTdiAddress;
	pMyData->hTdiEndPoint = hTdiEndPoint;
	PsCreateSystemThread(&hThread, 0, NULL, NtCurrentProcess(), NULL, RecvThreadProc, pMyData);
}


NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegPath)
{
	DbgPrint("Enter DriverEntry\n");
	NTSTATUS status = STATUS_SUCCESS;

	pDriverObject->DriverUnload = DriverUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = DriverDefaultHandle;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverDefaultHandle;

	// TDI 通信测试
	TdiTest();

	DbgPrint("Leave DriverEntry\n");
	return STATUS_SUCCESS;
}

VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	DbgPrint("Enter DriverUnload\n");

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

NTSTATUS CreateDevice(PDRIVER_OBJECT pDriverObject)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT pDevObj = NULL;
	UNICODE_STRING ustrDevName, ustrSymName;

	RtlInitUnicodeString(&ustrDevName, DEV_NAME);
	RtlInitUnicodeString(&ustrSymName, SYM_NAME);
	status = IoCreateDevice(pDriverObject, 0, &ustrDevName, FILE_DEVICE_UNKNOWN, 0, FALSE, &pDevObj);
	if (!NT_SUCCESS(status))
	{
		ShowError("IoCreateDevice", status);
		return status;
	}
	status = IoCreateSymbolicLink(&ustrSymName, &ustrDevName);
	if (!NT_SUCCESS(status))
	{
		ShowError("IoCreateSymbolicLink", status);
		IoDeleteDevice(pDevObj);
		return status;
	}

	return status;
}