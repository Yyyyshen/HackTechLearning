#include "ntddk.h"
#include "ntstrsafe.h"

/**
 * 由于权限等原因，处在用户层的恶意程序与处在内核层的杀软对抗中存在天然劣势
 * 所以也来到了内核驱动层
 * Rootkit：
 * 一种特殊的恶意软件，功能是在安装目标上隐藏自身和指定文件、进程和网络链接等
 * 一般和恶意程序结合使用，通过加载特殊的驱动，修改系统内核达到隐藏目的
 */

VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
	if (DriverObject != NULL)
	{
		DbgPrint("[%ws]Driver Upload,Driver Object Address:%p", __FUNCTIONW__, DriverObject);
	}
	return;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	DbgPrint("[%ws]Hello Kernel World, CurrentProcessId = 0x%p , CurrentIRQL = 0x%u\n", __FUNCTIONW__, PsGetCurrentProcessId(), KeGetCurrentIrql());
	if (RegistryPath != NULL)
	{
		DbgPrint("[%ws]Driver RegistryPath:%wZ\n", __FUNCTIONW__, RegistryPath);
	}

	if (DriverObject != NULL)
	{
		DbgPrint("[%ws]Driver Object Address:%p\n", __FUNCTIONW__, DriverObject);
		DriverObject->DriverUnload = DriverUnload;
	}

	WCHAR strBuf[128] = { 0 };

	UNICODE_STRING uFirstString = { 0 };
	RtlInitEmptyUnicodeString(&uFirstString, strBuf, sizeof(strBuf));
	RtlUnicodeStringCopyString(&uFirstString, L"Hello,Kernel\n"); //只能在PASSIVE_LEVEL下使用
	DbgPrint("String:%wZ", &uFirstString);

	return STATUS_SUCCESS;
}