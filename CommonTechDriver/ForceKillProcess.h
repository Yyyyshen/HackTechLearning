#ifndef _FORCE_KILL_PROCESS_H_
#define _FORCE_KILL_PROCESS_H_


#include <ntifs.h>


// 强制结束指定进程
NTSTATUS ForceKillProcess(HANDLE hProcessId);

// 获取 PspTerminateThreadByPointer 函数地址
PVOID GetPspLoadImageNotifyRoutine();

// 根据特征码获取 PspTerminateThreadByPointer 数组地址
PVOID SearchPspTerminateThreadByPointer(PUCHAR pSpecialData, ULONG ulSpecialDataSize);

// 指定内存区域的特征码扫描
PVOID SearchMemory(PVOID pStartAddress, PVOID pEndAddress, PUCHAR pMemoryData, ULONG ulMemoryDataSize);


#endif