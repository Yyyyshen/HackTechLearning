#include "WFP_Network.h"


GUID GUID_ALE_AUTH_CONNECT_CALLOUT_V4 = { 0x6812fc83, 0x7d3e, 0x499a, 0xa0, 0x12, 0x55, 0xe0, 0xd8, 0x5f, 0x34, 0x8b };


VOID ShowError(PCHAR lpszText, NTSTATUS ntStatus)
{
	KdPrint(("%s[Error:0x%X]\n", lpszText, ntStatus));
}


// 启动WFP
NTSTATUS WfpLoad(PDEVICE_OBJECT pDevObj)
{
	NTSTATUS status = STATUS_SUCCESS;

	// 注册Callout并设置过滤点
	status = RegisterCalloutForLayer(
		pDevObj,
		&FWPM_LAYER_ALE_AUTH_CONNECT_V4,
		&GUID_ALE_AUTH_CONNECT_CALLOUT_V4,
		classifyFn,
		notifyFn,
		flowDeleteFn,
		&g_AleConnectCalloutId,
		&g_AleConnectFilterId,
		&g_hEngine);
	if (!NT_SUCCESS(status))
	{
		ShowError("RegisterCalloutForLayer", status);
		return status;
	}

	return status;
}


// 卸载WFP
NTSTATUS WfpUnload()
{
	if (NULL != g_hEngine)
	{
		// 删除FilterId
		FwpmFilterDeleteById(g_hEngine, g_AleConnectFilterId);
		// 删除CalloutId
		FwpmCalloutDeleteById(g_hEngine, g_AleConnectCalloutId);
		// 清空Filter
		g_AleConnectFilterId = 0;
		// 反注册CalloutId
		FwpsCalloutUnregisterById(g_AleConnectCalloutId);
		// 清空CalloutId
		g_AleConnectCalloutId = 0;
		// 关闭引擎
		FwpmEngineClose(g_hEngine);
		g_hEngine = NULL;
	}

	return STATUS_SUCCESS;
}


// 注册Callout并设置过滤点
NTSTATUS RegisterCalloutForLayer(
	IN PDEVICE_OBJECT pDevObj,
	IN const GUID *layerKey,
	IN const GUID *calloutKey,
	IN FWPS_CALLOUT_CLASSIFY_FN classifyFn,
	IN FWPS_CALLOUT_NOTIFY_FN notifyFn,
	IN FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN flowDeleteNotifyFn,
	OUT ULONG32 *calloutId,
	OUT ULONG64 *filterId,
	OUT HANDLE *engine)
{
	NTSTATUS status = STATUS_SUCCESS;

	// 注册Callout
	status = RegisterCallout(
		pDevObj,
		calloutKey,
		classifyFn,
		notifyFn,
		flowDeleteNotifyFn,
		calloutId);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	// 设置过滤点
	status = SetFilter(
		layerKey,
		calloutKey,
		filterId,
		engine);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	return status;
}


// 注册Callout
NTSTATUS RegisterCallout(
	PDEVICE_OBJECT pDevObj,
	IN const GUID *calloutKey,
	IN FWPS_CALLOUT_CLASSIFY_FN classifyFn,
	IN FWPS_CALLOUT_NOTIFY_FN notifyFn,
	IN FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN flowDeleteNotifyFn,
	OUT ULONG32 *calloutId)
{
	NTSTATUS status = STATUS_SUCCESS;
	FWPS_CALLOUT sCallout = { 0 };
	
	// 设置Callout
	sCallout.calloutKey = *calloutKey;
	sCallout.classifyFn = classifyFn;
	sCallout.flowDeleteFn = flowDeleteNotifyFn;
	sCallout.notifyFn = notifyFn;

	// 注册Callout
	status = FwpsCalloutRegister(pDevObj, &sCallout, calloutId);
	if (!NT_SUCCESS(status))
	{
		ShowError("FwpsCalloutRegister", status);
		return status;
	}

	return status;
}


// 设置过滤点
NTSTATUS SetFilter(
	IN const GUID *layerKey,
	IN const GUID *calloutKey,
	OUT ULONG64 *filterId,
	OUT HANDLE *engine)
{
	HANDLE hEngine = NULL;
	NTSTATUS status = STATUS_SUCCESS;
	FWPM_SESSION session = { 0 };
	FWPM_FILTER mFilter = { 0 };
	FWPM_CALLOUT mCallout = { 0 };
	FWPM_DISPLAY_DATA mDispData = { 0 };

	// 创建Session
	session.flags = FWPM_SESSION_FLAG_DYNAMIC;
	status = FwpmEngineOpen(NULL,
		RPC_C_AUTHN_WINNT,
		NULL,
		&session,
		&hEngine);
	if (!NT_SUCCESS(status))
	{
		ShowError("FwpmEngineOpen", status);
		return status;
	}

	// 开始事务
	status = FwpmTransactionBegin(hEngine, 0);
	if (!NT_SUCCESS(status))
	{
		ShowError("FwpmTransactionBegin", status);
		return status;
	}

	// 设置Callout参数
	mDispData.name = L"MY WFP TEST";
	mDispData.description = L"WORLD OF DEMON";
	mCallout.applicableLayer = *layerKey;
	mCallout.calloutKey = *calloutKey;
	mCallout.displayData = mDispData;
	// 添加Callout到Session中
	status = FwpmCalloutAdd(hEngine, &mCallout, NULL, NULL);
	if (!NT_SUCCESS(status))
	{
		ShowError("FwpmCalloutAdd", status);
		return status;
	}

	// 设置过滤器参数
	mFilter.action.calloutKey = *calloutKey;
	mFilter.action.type = FWP_ACTION_CALLOUT_TERMINATING;
	mFilter.displayData.name = L"MY WFP TEST";
	mFilter.displayData.description = L"WORLD OF DEMON";
	mFilter.layerKey = *layerKey;
	mFilter.subLayerKey = FWPM_SUBLAYER_UNIVERSAL;
	mFilter.weight.type = FWP_EMPTY;
	// 添加过滤器
	status = FwpmFilterAdd(hEngine, &mFilter, NULL, filterId);
	if (!NT_SUCCESS(status))
	{
		ShowError("FwpmFilterAdd", status);
		return status;
	}

	// 提交事务
	status = FwpmTransactionCommit(hEngine);
	if (!NT_SUCCESS(status))
	{
		ShowError("FwpmTransactionCommit", status);
		return status;
	}

	*engine = hEngine;

	return status;
}


// Callout函数 classifyFn
#if (NTDDI_VERSION >= NTDDI_WIN10_RS3)
	VOID NTAPI classifyFn(
		_In_ const FWPS_INCOMING_VALUES0* inFixedValues,
		_In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
		_Inout_opt_ void* layerData,
		_In_opt_ const void* classifyContext,
		_In_ const FWPS_FILTER3* filter,
		_In_ UINT64 flowContext,
		_Inout_ FWPS_CLASSIFY_OUT0* classifyOut
	)
#elif (NTDDI_VERSION >= NTDDI_WIN8)
    VOID NTAPI classifyFn(
		_In_ const FWPS_INCOMING_VALUES0* inFixedValues,
		_In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
		_Inout_opt_ void* layerData,
		_In_opt_ const void* classifyContext,
		_In_ const FWPS_FILTER2* filter,
		_In_ UINT64 flowContext,
		_Inout_ FWPS_CLASSIFY_OUT0* classifyOut
		)
#elif (NTDDI_VERSION >= NTDDI_WIN7)                       
    VOID NTAPI classifyFn(
		_In_ const FWPS_INCOMING_VALUES0* inFixedValues,
		_In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
		_Inout_opt_ void* layerData,
		_In_opt_ const void* classifyContext,
		_In_ const FWPS_FILTER1* filter,
		_In_ UINT64 flowContext,
		_Inout_ FWPS_CLASSIFY_OUT0* classifyOut
		)
#else
    VOID NTAPI classifyFn(
		_In_ const FWPS_INCOMING_VALUES0* inFixedValues,
		_In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
		_Inout_opt_ void* layerData,
		_In_ const FWPS_FILTER0* filter,
		_In_ UINT64 flowContext,
		_Inout_ FWPS_CLASSIFY_OUT0* classifyOut
		)
#endif
{
	/*
	。WFP 的回调函数里提供了丰富的信息，这是 WFP 最大的优点， 不用我们为获得各种相关信息而绞尽脑汁。
	比如在 FWPM_LAYER_ALE_AUTH_CONNECT_V4 的回调函数里，我们能获得进程 ID、进程路径、本地、远
	程的 IP 地址/端口号以及协议代码。 但最爽的是此回调函数的最后一个参数，能让我们指定一个值，决定是
	放行还是拦截.
	*/
	ULONG ulLocalIp = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_LOCAL_ADDRESS].value.uint32;
	UINT16 uLocalPort = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_LOCAL_PORT].value.uint16;
	ULONG ulRemoteIp = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_REMOTE_ADDRESS].value.uint32;
	UINT16 uRemotePort = inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_REMOTE_PORT].value.uint16;
	KIRQL kCurrentIrql = KeGetCurrentIrql();
	ULONG64 processId = inMetaValues->processId;
	UCHAR szProcessPath[256] = { 0 };
	CHAR szProtocalName[256] = { 0 };
	RtlZeroMemory(szProcessPath, 256);
	ULONG i = 0;
	// 获取进程路径
	for (i = 0; i < inMetaValues->processPath->size; i++)
	{
		// 里面是宽字符存储的
		szProcessPath[i] = inMetaValues->processPath->data[i];
	}
	
	// 允许连接
	classifyOut->actionType = FWP_ACTION_PERMIT;    

	// 禁止指定进程网络连接
	if (NULL != wcsstr((PWCHAR)szProcessPath, L"cmd.exe"))
	{
		KdPrint(("cmd.exe[FWP_ACTION_BLOCK]\n"));
		// 拒绝连接
		classifyOut->actionType = FWP_ACTION_BLOCK;
		classifyOut->rights = classifyOut->rights & (~FWPS_RIGHT_ACTION_WRITE);
		classifyOut->flags = classifyOut->flags | FWPS_CLASSIFY_OUT_FLAG_ABSORB;
	}
	// 协议判断
	ProtocalIdToName(inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_PROTOCOL].value.uint16, szProtocalName);
	// 显示
	DbgPrint("Protocal=%s, LocalIp=%u.%u.%u.%u:%d, RemoteIp=%u.%u.%u.%u:%d, IRQL=%d, PID=%I64d, Path=%S\n",
		szProtocalName,
		(ulLocalIp >> 24) & 0xFF,
		(ulLocalIp >> 16) & 0xFF,
		(ulLocalIp >> 8) & 0xFF, 
		(ulLocalIp)& 0xFF,
		uLocalPort,
		(ulRemoteIp >> 24) & 0xFF, 
		(ulRemoteIp >> 16) & 0xFF, 
		(ulRemoteIp >> 8) & 0xFF, 
		(ulRemoteIp)& 0xFF,
		uRemotePort,
		kCurrentIrql,
		processId, 
		(PWCHAR)szProcessPath);
}

// Callout函数 notifyFn
#if (NTDDI_VERSION >= NTDDI_WIN10_RS3)
	NTSTATUS NTAPI notifyFn(
		_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
		_In_ const GUID* filterKey,
		_Inout_ FWPS_FILTER3* filter
	)
#elif (NTDDI_VERSION >= NTDDI_WIN8)
	NTSTATUS NTAPI notifyFn(
		_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
		_In_ const GUID* filterKey,
		_Inout_ FWPS_FILTER2* filter
		)
#elif (NTDDI_VERSION >= NTDDI_WIN7)
	NTSTATUS NTAPI notifyFn(
		_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
		_In_ const GUID* filterKey,
		_Inout_ FWPS_FILTER1* filter
		)
#else
	NTSTATUS NTAPI notifyFn(
		_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
		_In_ const GUID* filterKey,
		_Inout_ FWPS_FILTER0* filter
		)
#endif
{
	NTSTATUS status = STATUS_SUCCESS;


	return status;
}


// Callout函数 flowDeleteFn
VOID NTAPI flowDeleteFn(
	_In_ UINT16 layerId,
	_In_ UINT32 calloutId,
	_In_ UINT64 flowContext
	)
{
	return;
}


// 协议判断
NTSTATUS ProtocalIdToName(UINT16 protocalId, PCHAR lpszProtocalName)
{
	NTSTATUS status = STATUS_SUCCESS;

	switch (protocalId)
	{
	case 1:
	{
		// ICMP
		RtlCopyMemory(lpszProtocalName, "ICMP", 5);
		break;
	}
	case 2:
	{
		// IGMP
		RtlCopyMemory(lpszProtocalName, "IGMP", 5);
		break;
	}
	case 6:
	{
		// TCP
		RtlCopyMemory(lpszProtocalName, "TCP", 4);
		break;
	}
	case 17:
	{
		// UDP
		RtlCopyMemory(lpszProtocalName, "UDP", 4);
		break;
	}
	case 27:
	{
		// RDP
		RtlCopyMemory(lpszProtocalName, "RDP", 6);
		break;
	}
	default:
	{
		// UNKNOW
		RtlCopyMemory(lpszProtocalName, "UNKNOWN", 8);
		break;
	}
	}

	return status;
}