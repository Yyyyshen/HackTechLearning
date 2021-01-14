#ifndef _NOTIFY_ROUTINE_H_
#define _NOTIFY_ROUTINE_H_


#include <ntddk.h>


// 未导出函数声明
PCHAR PsGetProcessImageFileName(PEPROCESS pEProcess);


// 编程方式绕过签名检查
BOOLEAN BypassCheckSign(PDRIVER_OBJECT pDriverObject);

// 设置回调函数
NTSTATUS SetProcessNotifyRoutine();

// 删除回调函数
NTSTATUS RemoveProcessNotifyRoutine();

// 回调函数
VOID ProcessNotifyExRoutine(PEPROCESS pEProcess, HANDLE hProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo);


#endif