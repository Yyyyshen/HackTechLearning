#ifndef _ENUM_REMOVE_F_H_
#define _ENUM_REMOVE_F_H_


#include <fltKernel.h>


// 遍历回调
BOOLEAN EnumCallback_F();

// 移除回调
NTSTATUS RemoveCallback_F(PFLT_FILTER pFilter);

// 获取 Operations 偏移
LONG GetOperationsOffset();

// 新的回调函数 消息处理前
FLT_PREOP_CALLBACK_STATUS
New_MiniFilterPreOperation(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_Flt_CompletionContext_Outptr_ PVOID *CompletionContext
);
// 新的回调函数 消息处理后
FLT_POSTOP_CALLBACK_STATUS
New_MiniFilterPostOperation(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_In_opt_ PVOID CompletionContext,
_In_ FLT_POST_OPERATION_FLAGS Flags
);


#endif