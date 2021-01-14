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
  */

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegPath)
{
	DbgPrint("Enter DriverEntry\n");
	NTSTATUS status = STATUS_SUCCESS;

	//

	DbgPrint("Leave DriverEntry\n");
	return status;
}