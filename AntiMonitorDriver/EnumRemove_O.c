#include "EnumRemove_O.h"


VOID ShowError_O(PCHAR lpszText, NTSTATUS ntStatus)
{
	DbgPrint("%s Error[0x%X]\n", lpszText, ntStatus);
}


// 获取进程对象类型回调
BOOLEAN EnumProcessObCallback()
{
	POB_CALLBACK pObCallback = NULL;

	// 直接获取 CallbackList 链表
	LIST_ENTRY CallbackList = ((POBJECT_TYPE)(*PsProcessType))->CallbackList;

	// 开始遍历
	pObCallback = (POB_CALLBACK)CallbackList.Flink;
	do
	{
		if (FALSE == MmIsAddressValid(pObCallback))
		{
			break;
		}
		if (NULL != pObCallback->ObHandle)
		{
			// 显示
			DbgPrint("[PsProcessType]pObCallback->ObHandle = 0x%p\n", pObCallback->ObHandle);
			DbgPrint("[PsProcessType]pObCallback->PreCall = 0x%p\n", pObCallback->PreCall);
			DbgPrint("[PsProcessType]pObCallback->PostCall = 0x%p\n", pObCallback->PostCall);
		}
		// 获取下一链表信息
		pObCallback = (POB_CALLBACK)pObCallback->ListEntry.Flink;
		
	} while (CallbackList.Flink != (PLIST_ENTRY)pObCallback);

	return TRUE;
}


// 获取线程对象类型回调
BOOLEAN EnumThreadObCallback()
{
	POB_CALLBACK pObCallback = NULL;

	// 直接获取 CallbackList 链表
	LIST_ENTRY CallbackList = ((POBJECT_TYPE)(*PsThreadType))->CallbackList;

	// 开始遍历
	pObCallback = (POB_CALLBACK)CallbackList.Flink;
	do
	{
		if (FALSE == MmIsAddressValid(pObCallback))
		{
			break;
		}
		if (NULL != pObCallback->ObHandle)
		{
			// 显示
			DbgPrint("[PsThreadype]pObCallback->ObHandle = 0x%p\n", pObCallback->ObHandle);
			DbgPrint("[PsThreadType]pObCallback->PreCall = 0x%p\n", pObCallback->PreCall);
			DbgPrint("[PsThreadType]pObCallback->PostCall = 0x%p\n", pObCallback->PostCall);
		}
		// 获取下一链表信息
		pObCallback = (POB_CALLBACK)pObCallback->ListEntry.Flink;

	} while (CallbackList.Flink != (PLIST_ENTRY)pObCallback);

	return TRUE;
}


// 移除回调
NTSTATUS RemoveObCallback(PVOID RegistrationHandle)
{
	ObUnRegisterCallbacks(RegistrationHandle);
	
	return STATUS_SUCCESS;
}