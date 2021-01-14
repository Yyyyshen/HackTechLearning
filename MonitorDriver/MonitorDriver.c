#include "ntddk.h"

/**
 * 监控技术
 * 64位系统下的Patch Guard使得许多通过修改系统关键内存数据的HOOK方法失效
 * 微软考虑到用户程序的开发，开放了方便调用的系统回调API函数
 * 常见的是：创建进程、模块加载、注册表、对象回调等
 * 满足程序监控需求，并且比自己的HOOK更加底层和稳定
 */

 /**
  * 进程创建监控
  * PsSetCreateProccessNotifyRoutineEx
  * 调用类似上述函数时，会要求程序强制完整性签名，否则调用失败
  * 强制完整性检查是确保正在加载的二进制文件在加载前需要签名的策略
  * IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY标志在链接时通过使用/integritycheck链接器标志在PE标头中进行设置
  * 
  * 做这项配置可以通过VS属性页中链接器的其他选项输入/INTEGRITYCHECK
  * 
  * 还可以用代码来设置，通过研究发现，内核通过MmVerifyCallbackFunction验证限制函数调用是否合法
  * 但此函数只是简单验证了DriverObject->DriverSection->Flags值是否包含0x20
  * 所以只要把该标志按位或0x20即可
  */
#include "NotifyRoutine.h"
VOID TESTPROCESS(PDRIVER_OBJECT pDriverObject)
{
	// 编程方式绕过签名检查
	BypassCheckSign(pDriverObject);

	// 设置回调函数
	SetProcessNotifyRoutine();
}

/**
 * 模块加载监控
 */
VOID TESTMODULE()
{

}

VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	// 删除进程监控回调函数
	RemoveProcessNotifyRoutine();
}


NTSTATUS DriverDefaultHandle(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return status;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegPath)
{
	DbgPrint("Enter DriverEntry\n");
	NTSTATUS status = STATUS_SUCCESS;
	pDriverObject->DriverUnload = DriverUnload;
	for (ULONG i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObject->MajorFunction[i] = DriverDefaultHandle;
	}

	//进程监控
	TESTPROCESS(pDriverObject);

	//

	DbgPrint("Leave DriverEntry\n");
	return status;
}