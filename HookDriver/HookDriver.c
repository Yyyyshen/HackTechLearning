
/**
 * 在32位系统中，RootKit本质就是Hook技术，恶意程序与杀软对抗、互相挂钩更底层的函数或对象等
 * 在64位系统中，Patch Guard保护会对系统关键内存进行检测，保存的内存数据被更改了则触发蓝屏保护
 * 大多数HOOK都是基于修改系统内存，不适用于64位系统，除非绕过保护
 * 内核下HOOK技术很多：
 * SSDT、过滤驱动、IAT、EAT、GDT等
 */

 /**
  * SSDT HOOK
  * System Services Descriptor Table 系统服务描述符表
  * 作用是把用户层Ring3的Win32API函数和内核层Ring0的内核API函数联系起来
  * Ring3下一些函数最终对应于ntdll.dll里一个NtXXX函数
  * 比如CreateFile，调用NtCreateFile，将系统服务号放入EAX，然后调用系统服务分发函数KiSystemServices进入内核
  * 最终在Ring0中通过传入的EAX系统服务号（函数索引号）得到对应系统服务内核地址
  *
  * 通过修改这个地址索引表的函数地址可以对常用函数进行挂钩，从而过滤或监控一些核心系统动作
  */

#include "SSDTHook.h" //使用了ntifs.h，要放在最前面声明
VOID TESTSSDT_HOOK()
{
	SSDTHook();
}

#include "SSDTFunctionIndex.h"
  //获取SSDT函数索引号
VOID TESTSSDT_INDEX()
{
	// 从 ntdll.dll 中获取 SSDT 函数索引号
	UNICODE_STRING ustrDllFileName;
	RtlInitUnicodeString(&ustrDllFileName, L"\\??\\C:\\Windows\\System32\\ntdll.dll");
	ULONG ulSSDTFunctionIndex = GetSSDTFunctionIndex(ustrDllFileName, "ZwOpenProcess");
	DbgPrint("ZwOpenProcess[%d]\n", ulSSDTFunctionIndex);
}
//获取 SSDT 函数地址
#ifdef _WIN64
#include "SSDTFunction_64.h"
#else
#include "SSDTFunction_32.h"
#endif
VOID TESTSSDT_FUNC()
{
#ifdef _WIN64
	GetSSDTFunction_64("NtOpenProcess");
#else
	GetSSDTFunction_32("NtOpenProcess");
#endif
}
//头文件声明放到最前面去了，不然会有重定义冲突
VOID TESTSSDT()
{
	//TESTSSDT_INDEX();
	//TESTSSDT_FUNC();
	TESTSSDT_HOOK();
}

/**
 * 过滤驱动
 * 内核层下，可以把按键记录做的非常底层，可以绕过绝大多数反按键记录保护程序
 * 这里学习一个键盘过滤驱动，进行IRP HOOK
 */
VOID TESTFILTER()
{
	//代码较多，分一个项目来弄，见：HookKeyBoardDriver
}



VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	//取消SSDTHOOK
	SSDTUnhook();
}


NTSTATUS DriverDefaultHandle(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return status;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	DbgPrint("Enter DriverEntry\n");
	NTSTATUS status = STATUS_SUCCESS;
	DriverObject->DriverUnload = DriverUnload;
	for (ULONG i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		DriverObject->MajorFunction[i] = DriverDefaultHandle;
	}

	//SSDT HOOK
	//TESTSSDT();

	//过滤
	TESTFILTER();

	DbgPrint("Leave DriverEntry\n");
	return status;
}