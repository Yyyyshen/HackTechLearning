#ifndef _ENUM_REMOVE_M_H_
#define _ENUM_REMOVE_M_H_


#include <ntddk.h>


// 遍历回调
BOOLEAN EnumNotifyRoutine_M();

// 移除回调
NTSTATUS RemoveNotifyRoutine_M(PVOID pNotifyRoutineAddress);

// 获取 PspLoadImageNotifyRoutine 数组地址
PVOID GetPspLoadImageNotifyRoutine();

// 根据特征码获取 PspLoadImageNotifyRoutine 数组地址
PVOID SearchPspLoadImageNotifyRoutine(PUCHAR pSpecialData, ULONG ulSpecialDataSize);

// 指定内存区域的特征码扫描
PVOID SearchMemory_M(PVOID pStartAddress, PVOID pEndAddress, PUCHAR pMemoryData, ULONG ulMemoryDataSize);


#endif