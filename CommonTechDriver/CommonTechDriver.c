#include "ntddk.h"

/**
 * 恶意程序在内核层常用技术示例
 * 包括驱动隐藏、进程隐藏、更底层的TDI和NDIS等网络通信
 * 还有文件保护、进程保护、进程强杀、文件强删
 */

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


VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
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
	for (ULONG i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObject->MajorFunction[i] = DriverDefaultHandle;
	}

	//隐藏驱动
	//TESTDRIVERHIDE(pDriverObject);
	//隐藏进程
	//TESTPROCESSHIDE();

	DbgPrint("Leave CreateDevice\n");
	return status;
}