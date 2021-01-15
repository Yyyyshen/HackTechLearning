/**
 * 反监控技术
 * 恶意程序和杀软都会有监控操作
 * 在相互对抗中，可以暴力摘除对方的监控回调
 */

#include "EnumRemove_O.h" //反对象监控

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
 * 也类似上面三种方式
 */
#include "EnumRemove_R.h"
VOID TESTANTIREG()
{
	EnumCallback();
}

/**
 * 反对象监控
 * 对线回调函数存储在名为CallbackList表头的双向链表中
 * 依然是类似的方式，但相对更容易些，可以直接从POBJECT_TYPE数据类型中获取表头地址
 */
//#include "EnumRemove_O.h"		放最前面防止重定义报错
VOID TESTANTIOBJ()
{
	// 获取进程对象类型回调
	EnumProcessObCallback();

	// 获取线程对象类型回调
	EnumThreadObCallback();
}

/**
 * 反Minifilter文件监控
 * 枚举Minifilter驱动程序回调，有直接的内核函数FltEnumerateFilters，要注意不同系统下结构体不同
 * 需要使用fltKernel.h及导入FltMgr.lib
 */
#include "EnumRemove_F.h"
VOID TESTANTIMINIFILTER()
{
	EnumCallback_F();
}


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

	//都是类似的方式


	DbgPrint("Leave CreateDevice\n");
	return status;
}