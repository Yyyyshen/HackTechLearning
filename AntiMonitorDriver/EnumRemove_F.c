#include "EnumRemove_F.h"


VOID ShowError_F(PCHAR lpszText, NTSTATUS ntStatus)
{
	DbgPrint("%s Error[0x%X]\n", lpszText, ntStatus);
}


// 遍历回调
BOOLEAN EnumCallback_F()
{
	NTSTATUS status = STATUS_SUCCESS;
	ULONG ulFilterListSize = 0;
	PFLT_FILTER *ppFilterList = NULL;
	ULONG i = 0;
	LONG lOperationsOffset = 0;
	PFLT_OPERATION_REGISTRATION pFltOperationRegistration = NULL;

	// 获取 Minifilter 过滤器Filter 的数量
	FltEnumerateFilters(NULL, 0, &ulFilterListSize);
	// 申请内存
	ppFilterList = (PFLT_FILTER *)ExAllocatePool(NonPagedPool, ulFilterListSize *sizeof(PFLT_FILTER));
	if (NULL == ppFilterList)
	{
		DbgPrint("ExAllocatePool Error!\n");
		return FALSE;
	}
	// 获取 Minifilter 中所有过滤器Filter 的信息
	status = FltEnumerateFilters(ppFilterList, ulFilterListSize, &ulFilterListSize);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("FltEnumerateFilters Error![0x%X]\n", status);
		return FALSE;
	}

	DbgPrint("ulFilterListSize=%d\n", ulFilterListSize);

	// 获取 PFLT_FILTER 中 Operations 偏移
	lOperationsOffset = GetOperationsOffset();
	if (0 == lOperationsOffset)
	{
		DbgPrint("GetOperationsOffset Error\n");
		return FALSE;
	}
	
	// 开始遍历 Minifilter 中各个过滤器Filter 的信息
	__try
	{
		for (i = 0; i < ulFilterListSize; i++)
		{
			// 获取 PFLT_FILTER 中 Operations 成员地址
			pFltOperationRegistration = (PFLT_OPERATION_REGISTRATION)(*(PVOID *)((PUCHAR)ppFilterList[i] + lOperationsOffset));

			__try
			{
				// 同一过滤器下的回调信息
				DbgPrint("-------------------------------------------------------------------------------\n");
				while (IRP_MJ_OPERATION_END != pFltOperationRegistration->MajorFunction)   
				{
					if (IRP_MJ_MAXIMUM_FUNCTION > pFltOperationRegistration->MajorFunction)     // MajorFunction ID Is: 0~27
					{
						// 显示
						DbgPrint("[Filter=%p]IRP=%d, PreFunc=0x%p, PostFunc=0x%p\n", ppFilterList[i], pFltOperationRegistration->MajorFunction,
							pFltOperationRegistration->PreOperation, pFltOperationRegistration->PostOperation);
					}
					// 获取下一个消息回调信息
					pFltOperationRegistration = (PFLT_OPERATION_REGISTRATION)((PUCHAR)pFltOperationRegistration + sizeof(FLT_OPERATION_REGISTRATION));
				}
				DbgPrint("-------------------------------------------------------------------------------\n");
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				DbgPrint("[2_EXCEPTION_EXECUTE_HANDLER]\n");
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("[1_EXCEPTION_EXECUTE_HANDLER]\n");
	}

	// 释放内存
	ExFreePool(ppFilterList);
	ppFilterList = NULL;

	return TRUE;
}


// 移除回调
NTSTATUS RemoveCallback_F(PFLT_FILTER pFilter)
{
	LONG lOperationsOffset = 0;
	PFLT_OPERATION_REGISTRATION pFltOperationRegistration = NULL;

	// 开始遍历 过滤器Filter 的信息
	// 获取 PFLT_FILTER 中 Operations 成员地址
	pFltOperationRegistration = (PFLT_OPERATION_REGISTRATION)(*(PVOID *)((PUCHAR)pFilter + lOperationsOffset));
	__try
	{
		// 同一过滤器下的回调信息
		while (IRP_MJ_OPERATION_END != pFltOperationRegistration->MajorFunction)
		{
			if (IRP_MJ_MAXIMUM_FUNCTION > pFltOperationRegistration->MajorFunction)     // MajorFunction ID Is: 0~27
			{
				// 替换回调函数
				pFltOperationRegistration->PreOperation = New_MiniFilterPreOperation;
				pFltOperationRegistration->PostOperation = New_MiniFilterPostOperation;

				// 显示
				DbgPrint("[Filter=%p]IRP=%d, PreFunc=0x%p, PostFunc=0x%p\n", pFilter, pFltOperationRegistration->MajorFunction,
					pFltOperationRegistration->PreOperation, pFltOperationRegistration->PostOperation);
			}
			// 获取下一个消息回调信息
			pFltOperationRegistration = (PFLT_OPERATION_REGISTRATION)((PUCHAR)pFltOperationRegistration + sizeof(FLT_OPERATION_REGISTRATION));
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("[EXCEPTION_EXECUTE_HANDLER]\n");
	}
	
	return STATUS_SUCCESS;
}


// 获取 Operations 偏移
LONG GetOperationsOffset()
{
	RTL_OSVERSIONINFOW osInfo = { 0 };
	LONG lOperationsOffset = 0;

	// 获取系统版本信息, 判断系统版本
	RtlGetVersion(&osInfo);
	if (6 == osInfo.dwMajorVersion)
	{
		if (1 == osInfo.dwMinorVersion)
		{
			// Win7
#ifdef _WIN64
			// 64 位
			// 0x188
			lOperationsOffset = 0x188;
#else
			// 32 位
			// 0xCC
			lOperationsOffset = 0xCC;
#endif	
		}
		else if (2 == osInfo.dwMinorVersion)
		{
			// Win8
#ifdef _WIN64
			// 64 位
			
#else
			// 32 位
			
#endif
		}
		else if (3 == osInfo.dwMinorVersion)
		{
			// Win8.1
#ifdef _WIN64
			// 64 位
			// 0x198
			lOperationsOffset = 0x198;
#else
			// 32 位
			// 0xD4
			lOperationsOffset = 0xD4;
#endif			
		}
	}
	else if (10 == osInfo.dwMajorVersion)
	{
		// Win10
#ifdef _WIN64
		// 64 位
		// 0x1A8
		lOperationsOffset = 0x1A8;
#else
		// 32 位
		// 0xE4
		lOperationsOffset = 0xE4;
#endif
	}

	return lOperationsOffset;
}


// 新的回调函数 消息处理前
FLT_PREOP_CALLBACK_STATUS New_MiniFilterPreOperation(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_Flt_CompletionContext_Outptr_ PVOID *CompletionContext
)
{
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(CompletionContext);

	DbgPrint("[New_MiniFilterPreOperation]\n");

	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}


// 新的回调函数 消息处理后
FLT_POSTOP_CALLBACK_STATUS New_MiniFilterPostOperation(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_In_opt_ PVOID CompletionContext,
_In_ FLT_POST_OPERATION_FLAGS Flags
)
{
	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(CompletionContext);
	UNREFERENCED_PARAMETER(Flags);

	DbgPrint("[New_MiniFilterPostOperation]\n");

	return FLT_POSTOP_FINISHED_PROCESSING;
}