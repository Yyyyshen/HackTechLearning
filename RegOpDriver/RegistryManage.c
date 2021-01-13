#include "RegistryManage.h"


VOID ShowError(PUCHAR pszText, NTSTATUS ntStatus)
{
	DbgPrint("%s Error[0x%X]\n", pszText, ntStatus);
}


// 创建或者打开已存在注册表键
BOOLEAN MyCreateRegistryKey(UNICODE_STRING ustrRegistry)
{
	HANDLE hRegister = NULL;
	OBJECT_ATTRIBUTES objectAttributes = {0};
	ULONG ulResult = 0;
	NTSTATUS status = STATUS_SUCCESS;

	// 创建或者打开已存在注册表键
	InitializeObjectAttributes(&objectAttributes, &ustrRegistry, OBJ_CASE_INSENSITIVE, NULL, NULL);
	status = ZwCreateKey(&hRegister,
		KEY_ALL_ACCESS,
		&objectAttributes,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		&ulResult);
	if (!NT_SUCCESS(status))
	{
		ShowError("ZwCreateKey", status);
		return FALSE;
	}
	if (REG_CREATED_NEW_KEY == ulResult)
	{
		DbgPrint("The register item is createed!\n");
	}
	else if (REG_OPENED_EXISTING_KEY == ulResult)
	{
		DbgPrint("The register item has been created, and now is opened!\n");
	}

	// 关闭注册表键句柄
	ZwClose(hRegister);
	return TRUE;
}


// 打开注册表键
BOOLEAN MyOpenRegistryKey(UNICODE_STRING ustrRegistry)
{
	OBJECT_ATTRIBUTES objectAttributes = {0};
	HANDLE hRegister = NULL;
	NTSTATUS status = STATUS_SUCCESS;

	// 打开注册表键
	InitializeObjectAttributes(&objectAttributes, &ustrRegistry, OBJ_CASE_INSENSITIVE, NULL, NULL);
	status = ZwOpenKey(&hRegister, KEY_ALL_ACCESS, &objectAttributes);
	if (!NT_SUCCESS(status))
	{
		ShowError("ZwOpenKey", status);
		return FALSE;
	}
	DbgPrint("Open register successfully!\n");

	// 关闭注册表键句柄
	ZwClose(hRegister);
	return TRUE;
}


// 添加或者修改注册表键值
BOOLEAN MySetRegistryKeyValue(UNICODE_STRING ustrRegistry, UNICODE_STRING ustrKeyValueName, ULONG ulKeyValueType, PVOID pKeyValueData, ULONG ulKeyValueDataSize)
{
	HANDLE hRegister = NULL;
	OBJECT_ATTRIBUTES objectAttributes = {0};
	NTSTATUS status = STATUS_SUCCESS;
	// 打开注册表键
	InitializeObjectAttributes(&objectAttributes, &ustrRegistry, OBJ_CASE_INSENSITIVE, NULL, NULL);
	status = ZwOpenKey(&hRegister, KEY_ALL_ACCESS, &objectAttributes);
	if (!NT_SUCCESS(status))
	{
		ShowError("ZwOpenKey", status);
		return FALSE;
	}
	// 添加或者修改键值
	status = ZwSetValueKey(hRegister, &ustrKeyValueName, 0, ulKeyValueType, pKeyValueData, ulKeyValueDataSize);
	if (!NT_SUCCESS(status))
	{
		ZwClose(hRegister);
		ShowError("ZwSetValueKey", status);
		return FALSE;
	}
	// 关闭注册表键句柄
	ZwClose(hRegister);
	return TRUE;
}


// 删除注册表键
BOOLEAN MyDeleteRegistryKey(UNICODE_STRING ustrRegistry)
{
	HANDLE hRegister = NULL;
	OBJECT_ATTRIBUTES objectAttributes = {0};
	NTSTATUS status = STATUS_SUCCESS;
	// 打开注册表键
	InitializeObjectAttributes(&objectAttributes, &ustrRegistry, OBJ_CASE_INSENSITIVE, NULL, NULL);
	status = ZwOpenKey(&hRegister, KEY_ALL_ACCESS, &objectAttributes);
	if (!NT_SUCCESS(status))
	{
		ShowError("ZwOpenKey", status);
		return FALSE;
	}
	// 删除注册表键
	status = ZwDeleteKey(hRegister);
	if (!NT_SUCCESS(status))
	{
		ZwClose(hRegister);
		ShowError("ZwDeleteKey", status);
		return FALSE;
	}
	// 关闭注册表键句柄
	ZwClose(hRegister);
	return TRUE;
}


// 删除注册表键值
BOOLEAN MyDeleteRegistryKeyValue(UNICODE_STRING ustrRegistry, UNICODE_STRING ustrKeyValueName)
{
	HANDLE hRegister = NULL;
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	NTSTATUS status = STATUS_SUCCESS;
	// 打开注册表键
	InitializeObjectAttributes(&objectAttributes, &ustrRegistry, OBJ_CASE_INSENSITIVE, NULL, NULL);
	status = ZwOpenKey(&hRegister, KEY_ALL_ACCESS, &objectAttributes);
	if (!NT_SUCCESS(status))
	{
		ShowError("ZwOpenKey", status);
		return FALSE;
	}
	// 删除注册表键
	status = ZwDeleteValueKey(hRegister, &ustrKeyValueName);
	if (!NT_SUCCESS(status))
	{
		ZwClose(hRegister);
		ShowError("ZwDeleteValueKey", status);
		return FALSE;
	}
	// 关闭注册表键句柄
	ZwClose(hRegister);
	return TRUE;
}


// 查询注册表键值
BOOLEAN MyQueryRegistryKeyValue(UNICODE_STRING ustrRegistry, UNICODE_STRING ustrKeyValueName)
{
	HANDLE hRegister = NULL;
	OBJECT_ATTRIBUTES objectAttributes = {0};
	NTSTATUS status = STATUS_SUCCESS;
	ULONG ulBufferSize = 0;
	PKEY_VALUE_PARTIAL_INFORMATION pKeyValuePartialInfo = NULL;

	// 打开注册表键
	InitializeObjectAttributes(&objectAttributes, &ustrRegistry, OBJ_CASE_INSENSITIVE, NULL, NULL);
	status = ZwOpenKey(&hRegister, KEY_ALL_ACCESS, &objectAttributes);
	if (!NT_SUCCESS(status))
	{
		ShowError("ZwOpenKey", status);
		return FALSE;
	}
	// 先获取查询注册表键值所需缓冲区的大小
	status = ZwQueryValueKey(hRegister, &ustrKeyValueName, KeyValuePartialInformation, NULL, 0, &ulBufferSize);
	if (0 == ulBufferSize)
	{
		ZwClose(hRegister);
		ShowError("ZwQueryValueKey", status);
		return FALSE;
	}
	// 申请缓冲区
	pKeyValuePartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePool(NonPagedPool, ulBufferSize);
	// 查询注册表键值并获取查询结果
	status = ZwQueryValueKey(hRegister, &ustrKeyValueName, KeyValuePartialInformation, pKeyValuePartialInfo, ulBufferSize, &ulBufferSize);
	if (!NT_SUCCESS(status))
	{
		ExFreePool(pKeyValuePartialInfo);
		ZwClose(hRegister);
		ShowError("ZwQueryValueKey", status);
		return FALSE;
	}
	// 显示查询结果
	DbgPrint("KeyValueName=%wZ, KeyValueType=%d, KeyValueData=%S\n",
		&ustrKeyValueName, pKeyValuePartialInfo->Type, pKeyValuePartialInfo->Data);

	// 释放内存, 关闭句柄
	ExFreePool(pKeyValuePartialInfo);
	ZwClose(hRegister);
	return TRUE;
}