#include "ntddk.h"
#include "ntstrsafe.h"

/**
 * 由于权限等原因，处在用户层的恶意程序与处在内核层的杀软对抗中存在天然劣势
 * 所以也来到了内核驱动层
 * Rootkit：
 * 一种特殊的恶意软件，功能是在安装目标上隐藏自身和指定文件、进程和网络链接等
 * 一般和恶意程序结合使用，通过加载特殊的驱动，修改系统内核达到隐藏目的
 */

/**
 * 驱动调试比较麻烦
 * 先是bcdedit命令对被调试机进行设置，并且在msconfig中高级选项打开调试、串口
 * 对两个虚拟机设置串口属性，苹果pd虚拟机是用插口来表示的（从自己的理解上，插口就是串口下面分出来的，都属于同一个com口），被调试机为客户端、调试机为服务端
 * 一定要关闭两端防火墙，不然配置好了也一直连不上
 * 生成和系统架构要对应
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

	return STATUS_SUCCESS;
}