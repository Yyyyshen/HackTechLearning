#ifndef _SSDT_HOOK_H_
#define _SSDT_HOOK_H_


#include <ntifs.h>


// SSDT Hook
BOOLEAN SSDTHook();

// SSDT Unhook
BOOLEAN SSDTUnhook();

// 新函数
NTSTATUS New_ZwQueryDirectoryFile(
	IN HANDLE               FileHandle,
	IN HANDLE               Event OPTIONAL,
	IN PIO_APC_ROUTINE      ApcRoutine OPTIONAL,
	IN PVOID                ApcContext OPTIONAL,
	OUT PIO_STATUS_BLOCK    IoStatusBlock,
	OUT PVOID               FileInformation,
	IN ULONG                Length,
	IN FILE_INFORMATION_CLASS FileInformationClass,
	IN BOOLEAN              ReturnSingleEntry,
	IN PUNICODE_STRING      FileMask OPTIONAL,
	IN BOOLEAN              RestartScan
	);

// 从各种文件信息类型中获取文件名称
VOID GetEntryFileName(IN PVOID pData, IN FILE_INFORMATION_CLASS FileInfo, PWCHAR pwszFileName, ULONG ulBufferSize);

// 在各种文件信息类型中设置下一个文件的偏移
VOID SetNextEntryOffset(IN PVOID pData, IN FILE_INFORMATION_CLASS FileInfo, IN ULONG Offset);

// 从各种文件信息类型中获取下一个文件的偏移
ULONG GetNextEntryOffset(IN PVOID pData, IN FILE_INFORMATION_CLASS FileInfo);


// 保存原函数地址
PVOID g_pOldSSDTFunctionAddress;


#endif