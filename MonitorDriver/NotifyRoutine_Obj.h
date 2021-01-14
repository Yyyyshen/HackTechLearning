#ifndef _NOTIFY_ROUTINE_OBJ_H_
#define _NOTIFY_ROUTINE_OBJ_H_


#include <ntifs.h>
#include <ntddk.h>


// 未导出函数声明
PUCHAR PsGetProcessImageFileName(PEPROCESS pEProcess);



// 设置进程回调函数
NTSTATUS SetProcessCallbacks();

// 设置线程回调函数
NTSTATUS SetThreadCallbacks();

// 删除进程回调函数
VOID RemoveProcessCallbacks();

// 删除线程回调函数
VOID RemoveThreadCallbacks();

// 进程回调函数
OB_PREOP_CALLBACK_STATUS ProcessPreCall(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pObPreOperationInfo);

// 线程回调函数
OB_PREOP_CALLBACK_STATUS ThreadPreCall(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pObPreOperationInfo);

// 判断是否为保护进程
BOOLEAN IsProtectProcess(PEPROCESS pEProcess);


// 进程回调对象句柄
HANDLE g_obProcessHandle;

// 线程回调对象句柄
HANDLE g_obThreadHandle;


#endif