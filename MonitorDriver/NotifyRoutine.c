#include "NotifyRoutine.h"


VOID ShowError(PCHAR pszText, NTSTATUS ntStatus)
{
	DbgPrint("%s Error[0x%X]\n", pszText, ntStatus);
}


// 编程方式绕过签名检查
BOOLEAN BypassCheckSign(PDRIVER_OBJECT pDriverObject)
{
#ifdef _WIN64
	typedef struct _KLDR_DATA_TABLE_ENTRY
	{
		LIST_ENTRY listEntry;
		ULONG64 __Undefined1;
		ULONG64 __Undefined2;
		ULONG64 __Undefined3;
		ULONG64 NonPagedDebugInfo;
		ULONG64 DllBase;
		ULONG64 EntryPoint;
		ULONG SizeOfImage;
		UNICODE_STRING path;
		UNICODE_STRING name;
		ULONG   Flags;
		USHORT  LoadCount;
		USHORT  __Undefined5;
		ULONG64 __Undefined6;
		ULONG   CheckSum;
		ULONG   __padding1;
		ULONG   TimeDateStamp;
		ULONG   __padding2;
	} KLDR_DATA_TABLE_ENTRY, *PKLDR_DATA_TABLE_ENTRY;
#else
	typedef struct _KLDR_DATA_TABLE_ENTRY
	{
		LIST_ENTRY listEntry;
		ULONG unknown1;
		ULONG unknown2;
		ULONG unknown3;
		ULONG unknown4;
		ULONG unknown5;
		ULONG unknown6;
		ULONG unknown7;
		UNICODE_STRING path;
		UNICODE_STRING name;
		ULONG   Flags;
	} KLDR_DATA_TABLE_ENTRY, *PKLDR_DATA_TABLE_ENTRY;
#endif

	PKLDR_DATA_TABLE_ENTRY pLdrData = (PKLDR_DATA_TABLE_ENTRY)pDriverObject->DriverSection;
	pLdrData->Flags = pLdrData->Flags | 0x20;

	return TRUE;
}


// 设置回调函数
NTSTATUS SetProcessNotifyRoutine()
{
	NTSTATUS status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)ProcessNotifyExRoutine, FALSE);
	if (!NT_SUCCESS(status))
	{
		ShowError("PsSetCreateProcessNotifyRoutineEx", status);
	}
	return status;
}


// 删除回调函数
NTSTATUS RemoveProcessNotifyRoutine()
{
	NTSTATUS status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)ProcessNotifyExRoutine, TRUE);
	if (!NT_SUCCESS(status))
	{
		ShowError("PsSetCreateProcessNotifyRoutineEx", status);
	}
	return status;
}


// 回调函数
VOID ProcessNotifyExRoutine(PEPROCESS pEProcess, HANDLE hProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo)
{
	// CreateInfo 为 NULL 时，表示进程退出；不为 NULL 时，表示进程创建
	if (NULL == CreateInfo)
	{
		return;
	}
	// 获取进程名称
	PCHAR pszImageFileName = PsGetProcessImageFileName(pEProcess);
	// 显示创建进程信息
	DbgPrint("[%s][%d][%wZ]\n", pszImageFileName, hProcessId, CreateInfo->ImageFileName);
	// 禁止指定进程(*.exe)创建 
	if (0 == _stricmp(pszImageFileName, "cmd.exe"))
	{
		// 禁止创建
		CreateInfo->CreationStatus = STATUS_UNSUCCESSFUL;

		DbgPrint("[禁止创建]\n");
	}
}