
/**
 * 监控技术
 * 64位系统下的Patch Guard使得许多通过修改系统关键内存数据的HOOK方法失效
 * 微软考虑到用户程序的开发，开放了方便调用的系统回调API函数
 * 常见的是：创建进程、模块加载、注册表、对象回调等
 * 满足程序监控需求，并且比自己的HOOK更加底层和稳定
 */

#include "LoadImageNotify.h" //模块监控

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
 * PsSetLoadImageNotifyRoutine
 * 恶意程序可以通过模块加载回调来实现进程注入，通过修改PE结构的导入表，添加DLL，实现DLL注入
 * 还可以拒绝特定驱动加载，例如拒绝杀软或其他分析程序驱动的加载
 *
 * 当程序回调函数接收到加载信息时，模块是已经加载完成的，所以不能直接控制该过程
 * 但可以通过自定义方法卸载已经加载的模块
 *
 * 卸载模块的思路：
 * 对于驱动模块，直接在入口点函数DriverEntry中返回NTSTATUS错误码，这样已加载的驱动就会出错退出，导致驱动启动失败
 * 对于DLL模块，DllMain入口函数无法确定DLL是否加载成功，通过内核API函数MmUnmapViewOfSection用来卸载已加载模块
 *
 * 另外，当加载进程模块的时候，系统会有一个内部锁，为了避免死锁，加载回调函数时，不能进行映射、分配、查询、释放等操作
 * 想卸载DLL模块，必须等进程中所有模块加载完毕才能操作，所以需要创建多线程延迟等待
 */
 //#include "LoadImageNotify.h" 放前面
NTSTATUS TESTMODULE()
{
	return SetNotifyRoutine();
}

/**
 * 注册表监控
 * CmRegisterCallback
 * 可以通过设置回调函数返回值来进行控制是否允许指定键的操作
 * 错误码设置为STATUS_SUCCESS
 */
#include "NotifyRoutine_Reg.h"
VOID TESTREG()
{
	// 设置回调函数
	SetRegisterCallback();
}

/**
 * 对象监控
 * 常用来保护恶意程序自身不被杀软或用户强制结束
 * 在结束进程时，首先对需要打开的进程获取进程句柄
 * 对此，可以使系统获取句柄失败，实现进程保护
 */
#include "NotifyRoutine_Obj.h"
VOID TESTOBJ(PDRIVER_OBJECT pDriverObject)
{
	// 编程方式绕过签名检查
	BypassCheckSign(pDriverObject);

	// 设置进程回调函数
	SetProcessCallbacks();

	// 设置线程回调函数
	SetThreadCallbacks();
}

/**
 * 文件监控和网络监控在其他项目中
 * 
 * 其中文件监控使用了Minifilter框架，VS中有现成模板，例子在项目：MonitorFileDriver
 * 
 * 网络监控需要使用WFP框架，VISTA之后的系统，系统防火墙的过滤钩子不再适用，都是用WFP
 * 过滤器引擎时WFP核心组件，用来过滤TCP/IP数据，例子在项目：MonitorNetDriver
 */


VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	// 删除进程监控回调函数
	//RemoveProcessNotifyRoutine();

	// 移除模块监控回调
	//RemoveNotifyRoutine();

	// 删除注册表监控回调函数
	//RemoveRegisterCallback();

	// 删除对象监控回调函数
	RemoveProcessCallbacks();
	// 删除对象监控 线程回调函数
	RemoveThreadCallbacks();
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
	//TESTPROCESS(pDriverObject);

	//模块监控
	//status = TESTMODULE();

	//注册表监控
	//TESTREG();

	//对象监控
	TESTOBJ(pDriverObject);

	DbgPrint("Leave DriverEntry\n");
	return status;
}