#ifndef _ENUM_REMOVE_T_H_
#define _ENUM_REMOVE_T_H_


#include <ntddk.h>


// 遍历回调
BOOLEAN EnumNotifyRoutine_T();

// 移除回调
NTSTATUS RemoveNotifyRoutine_T(PVOID pNotifyRoutineAddress);

// 获取 PspCreateThreadNotifyRoutine 数组地址
PVOID GetPspCreateThreadNotifyRoutine();

// 根据特征码获取 PspCreateThreadNotifyRoutine 数组地址
PVOID SearchPspCreateThreadNotifyRoutine(PUCHAR pFirstSpecialData, ULONG ulFirstSpecialDataSize, PUCHAR pSecondSpecialData, ULONG ulSecondSpecialDataSize);

// 指定内存区域的特征码扫描
PVOID SearchMemory_T(PVOID pStartAddress, PVOID pEndAddress, PUCHAR pMemoryData, ULONG ulMemoryDataSize);


#endif