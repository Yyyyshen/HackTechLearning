#ifndef _WFP_NETWORK_H_
#define _WFP_NETWORK_H_

#include <ntddk.h>
#include <fwpsk.h>
#include <fwpmk.h>


VOID ShowError(PCHAR lpszText, NTSTATUS ntStatus);

// 启动WFP
NTSTATUS WfpLoad(PDEVICE_OBJECT pDevObj);

// 卸载WFP
NTSTATUS WfpUnload();

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
	OUT HANDLE *engine);

/**
 * Callout各函数在不同系统版本fn号都不同
 * 书中例子只到Win8
 * 已经根据系统API进行补充
 */
// 注册Callout
NTSTATUS RegisterCallout(
	PDEVICE_OBJECT pDevObj,
	IN const GUID *calloutKey,
	IN FWPS_CALLOUT_CLASSIFY_FN classifyFn,
	IN FWPS_CALLOUT_NOTIFY_FN notifyFn,
	IN FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN flowDeleteNotifyFn,
	OUT ULONG32 *calloutId);

// 设置过滤点
NTSTATUS SetFilter(
	IN const GUID *layerKey,
	IN const GUID *calloutKey,
	OUT ULONG64 *filterId,
	OUT HANDLE *engine);

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
	);
#elif (NTDDI_VERSION >= NTDDI_WIN8)
    VOID NTAPI classifyFn(
		_In_ const FWPS_INCOMING_VALUES0* inFixedValues,
		_In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
		_Inout_opt_ void* layerData,
		_In_opt_ const void* classifyContext,
		_In_ const FWPS_FILTER2* filter,
		_In_ UINT64 flowContext,
		_Inout_ FWPS_CLASSIFY_OUT0* classifyOut
		);
#elif (NTDDI_VERSION >= NTDDI_WIN7)                       
    VOID NTAPI classifyFn(
		_In_ const FWPS_INCOMING_VALUES0* inFixedValues,
		_In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
		_Inout_opt_ void* layerData,
		_In_opt_ const void* classifyContext,
		_In_ const FWPS_FILTER1* filter,
		_In_ UINT64 flowContext,
		_Inout_ FWPS_CLASSIFY_OUT0* classifyOut
		);
#else
    VOID NTAPI classifyFn(
		_In_ const FWPS_INCOMING_VALUES0* inFixedValues,
		_In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
		_Inout_opt_ void* layerData,
		_In_ const FWPS_FILTER0* filter,
		_In_ UINT64 flowContext,
		_Inout_ FWPS_CLASSIFY_OUT0* classifyOut
		);
#endif

// Callout函数 notifyFn
#if (NTDDI_VERSION >= NTDDI_WIN10_RS3)
	NTSTATUS NTAPI notifyFn(
		_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
		_In_ const GUID* filterKey,
		_Inout_ FWPS_FILTER3* filter
	);
#elif (NTDDI_VERSION >= NTDDI_WIN8)
	NTSTATUS NTAPI notifyFn(
		_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
		_In_ const GUID* filterKey,
		_Inout_ FWPS_FILTER2* filter
		);
#elif (NTDDI_VERSION >= NTDDI_WIN7)
	NTSTATUS NTAPI notifyFn(
		_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
		_In_ const GUID* filterKey,
		_Inout_ FWPS_FILTER1* filter
		);
#else
	NTSTATUS NTAPI notifyFn(
		_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
		_In_ const GUID* filterKey,
		_Inout_ FWPS_FILTER0* filter
		);
#endif

// Callout函数 flowDeleteFn
VOID NTAPI flowDeleteFn(
	_In_ UINT16 layerId,
	_In_ UINT32 calloutId,
	_In_ UINT64 flowContext
	);

// 协议判断
NTSTATUS ProtocalIdToName(UINT16 protocalId, PCHAR lpszProtocalName);


// 过滤器引擎句柄
HANDLE g_hEngine;
// 过滤器引擎中的callout的运行时标识符
ULONG32 g_AleConnectCalloutId;
// 过滤器的运行时标识符
ULONG64 g_AleConnectFilterId;


#endif