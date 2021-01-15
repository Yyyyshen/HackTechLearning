#ifndef _ENUM_REMOVE_R_H_
#define _ENUM_REMOVE_R_H_


#include <ntddk.h>


typedef struct _CM_NOTIFY_ENTRY
{
	LIST_ENTRY  ListEntryHead;
	ULONG   UnKnown1;
	ULONG   UnKnown2;
	LARGE_INTEGER Cookie;
	PVOID   Context;
	PVOID   Function;
}CM_NOTIFY_ENTRY, *PCM_NOTIFY_ENTRY;


// 遍历回调
BOOLEAN EnumCallback();

// 移除回调
NTSTATUS RemoveCallback(LARGE_INTEGER Cookie);

// 获取 CallbackListHead 链表地址
PVOID GetCallbackListHead();

// 根据特征码获取 CallbackListHead 链表地址
PVOID SearchCallbackListHead(PUCHAR pSpecialData, ULONG ulSpecialDataSize, LONG lSpecialOffset);

// 指定内存区域的特征码扫描
PVOID SearchMemory_R(PVOID pStartAddress, PVOID pEndAddress, PUCHAR pMemoryData, ULONG ulMemoryDataSize);


#endif