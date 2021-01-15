#include "ntddk.h"

/**
 * 反监控技术
 * 恶意程序和杀软都会有监控操作
 * 在相互对抗中，可以暴力摘除对方的监控回调
 */

 /**
  * 反进程监控
  * PsSetCreateProcessNotifyRoutine回调函数会存放在一个名为PspCreateProcessNotifyRoutine的数组中
  * 该数组记录了所有进程创建回调函数的加密地址，需要解密后获取正确的函数地址
  */
#include "EnumRemove.h"
VOID TESTANTIPROCESS()
{
	//遍历回调函数
	EnumNotifyRoutine();
}

/**
 * 反线程监控
 * 与反进程类似
 */
#include "EnumRemove_T.h"
VOID TESTANTITHREAD()
{
	//遍历线程回调
	EnumNotifyRoutine_T();
}

/**
 * 反模块监控
 * 与反进程类似
 */
#include "EnumRemove_M.h"
VOID TESTANTIMODULE()
{
	EnumNotifyRoutine_M();
}

/**
 * 反注册表监控
 * 注册表回调函数存储在一个名为CallbackListHead表头的双向链表结构里
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

	//反进程监控
	TESTANTIPROCESS();

	//


	DbgPrint("Leave CreateDevice\n");
	return status;
}