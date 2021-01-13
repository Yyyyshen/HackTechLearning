#include "ntddk.h"

/**
 * 注册表操作
 * 与文件操作相似
 * 有系统直接提供的内核API来对注册表进行操作，但容易被监控
 * 也有更底层的HIVE文件操作注册表方式
 */

 /**
  * 内核API方式
  */
#include "RegistryManage.h"
VOID TESTAPI()
{
	// 创建注册表键
	UNICODE_STRING ustrRegistry;
	RtlInitUnicodeString(&ustrRegistry, L"\\Registry\\Machine\\Software\\DemonGan");
	MyCreateRegistryKey(ustrRegistry);

	// 打开注册表键
	MyOpenRegistryKey(ustrRegistry);

	// 添加注册表键值
	UNICODE_STRING ustrKeyValueName;
	WCHAR wstrKeyValueData[] = L"I am DemonGan";
	RtlInitUnicodeString(&ustrKeyValueName, L"Name");
	MySetRegistryKeyValue(ustrRegistry, ustrKeyValueName, REG_SZ, wstrKeyValueData, sizeof(wstrKeyValueData));

	// 查询注册表键值
	MyQueryRegistryKeyValue(ustrRegistry, ustrKeyValueName);

	// 删除注册表键值
	MyDeleteRegistryKeyValue(ustrRegistry, ustrKeyValueName);

	// 删除注册表键
	MyDeleteRegistryKey(ustrRegistry);
}

/**
 * HIVE文件方式
 */
VOID TESTHIVE()
{
	//见另一个项目：RegOpHIVE
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

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	DbgPrint("Enter DriverEntry\n");
	NTSTATUS status = STATUS_SUCCESS;
	DriverObject->DriverUnload = DriverUnload;
	for (ULONG i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		DriverObject->MajorFunction[i] = DriverDefaultHandle;
	}

	//内核API方式操作注册表
	//TESTAPI();

	//HIVE文件方式
	TESTHIVE();

	DbgPrint("Leave DriverEntry\n");
	return status;
}