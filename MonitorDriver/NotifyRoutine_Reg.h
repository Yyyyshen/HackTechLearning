#ifndef _NOTIFY_ROUTINE_REG_H_
#define _NOTIFY_ROUTINE_REG_H_


#include <ntddk.h>


// 未导出函数声明
PUCHAR PsGetProcessImageFileName(PEPROCESS pEProcess);

NTSTATUS ObQueryNameString(
	_In_ PVOID Object,
	_Out_writes_bytes_opt_(Length) POBJECT_NAME_INFORMATION ObjectNameInfo,
	_In_ ULONG Length,
	_Out_ PULONG ReturnLength
);


// 设置回调函数
NTSTATUS SetRegisterCallback();

// 删除回调函数
VOID RemoveRegisterCallback();

// 回调函数
NTSTATUS RegisterMonCallback(
	_In_ PVOID CallbackContext,
	// 操作类型（只是操作编号，不是指针）
	_In_opt_ PVOID Argument1,
	// 操作详细信息的结构体指针 
	_In_opt_ PVOID Argument2
	);

// 获取注册表完整路径
BOOLEAN GetRegisterObjectCompletePath(PUNICODE_STRING pRegistryPath, PVOID pRegistryObject);

// 判断是否是保护注册表路径
BOOLEAN IsProtectReg(UNICODE_STRING ustrRegPath);



// 注册表回调Cookie
LARGE_INTEGER g_liRegCookie;


#endif