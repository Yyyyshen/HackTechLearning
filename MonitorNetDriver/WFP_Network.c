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
	IN const GUID* layerKey,
	IN const GUID* calloutKey,
	IN FWPS_CALLOUT_CLASSIFY_FN classifyFn,
	IN FWPS_CALLOUT_NOTIFY_FN notifyFn,
	IN FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN flowDeleteNotifyFn,
	OUT ULONG32* calloutId,
	OUT ULONG64* filterId,
	OUT HANDLE* engine)
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
	IN const GUID* calloutKey,
	IN FWPS_CALLOUT_CLASSIFY_FN classifyFn,
	IN FWPS_CALLOUT_NOTIFY_FN notifyFn,
	IN FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN flowDeleteNotifyFn,
	OUT ULONG32* calloutId)
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
	IN const GUID* layerKey,
	IN const GUID* calloutKey,
	OUT ULONG64* filterId,
	OUT HANDLE* engine)
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
//#if (NTDDI_VERSION >= NTDDI_WIN10_RS3)
//	VOID NTAPI classifyFn(
//		_In_ const FWPS_INCOMING_VALUES0* inFixedValues,
//		_In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
//		_Inout_opt_ void* layerData,
//		_In_opt_ const void* classifyContext,
//		_In_ const FWPS_FILTER3* filter,
//		_In_ UINT64 flowContext,
//		_Inout_ FWPS_CLASSIFY_OUT0* classifyOut
//	)
//#elif (NTDDI_VERSION >= NTDDI_WIN8)
//    VOID NTAPI classifyFn(
//		_In_ const FWPS_INCOMING_VALUES0* inFixedValues,
//		_In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
//		_Inout_opt_ void* layerData,
//		_In_opt_ const void* classifyContext,
//		_In_ const FWPS_FILTER2* filter,
//		_In_ UINT64 flowContext,
//		_Inout_ FWPS_CLASSIFY_OUT0* classifyOut
//		)
//#elif (NTDDI_VERSION >= NTDDI_WIN7)                       
//    VOID NTAPI classifyFn(
//		_In_ const FWPS_INCOMING_VALUES0* inFixedValues,
//		_In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
//		_Inout_opt_ void* layerData,
//		_In_opt_ const void* classifyContext,
//		_In_ const FWPS_FILTER1* filter,
//		_In_ UINT64 flowContext,
//		_Inout_ FWPS_CLASSIFY_OUT0* classifyOut
//		)
//#else
//    VOID NTAPI classifyFn(
//		_In_ const FWPS_INCOMING_VALUES0* inFixedValues,
//		_In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
//		_Inout_opt_ void* layerData,
//		_In_ const FWPS_FILTER0* filter,
//		_In_ UINT64 flowContext,
//		_Inout_ FWPS_CLASSIFY_OUT0* classifyOut
//		)
//#endif
VOID NTAPI classifyFn(
	_In_ const FWPS_INCOMING_VALUES* inFixedValues,
	_In_ const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
	_Inout_opt_ void* layerData,
	_In_opt_ const void* classifyContext,
	_In_ const FWPS_FILTER* filter,
	_In_ UINT64 flowContext,
	_Inout_ FWPS_CLASSIFY_OUT* classifyOut
)
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
	CHAR szProtocalName[12] = { 0 };
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
	if (NULL != wcsstr((PWCHAR)szProcessPath, L"svchost.exe"))//禁止cmd联网不能直接判断cmd.exe，实际情况是，cmd发出的网络连接是通过进程： svchost.exe
	{
		KdPrint(("svchost.exe[FWP_ACTION_BLOCK]\n"));
		// 拒绝连接
		classifyOut->actionType = FWP_ACTION_BLOCK;
		classifyOut->rights = classifyOut->rights & (~FWPS_RIGHT_ACTION_WRITE);
		classifyOut->flags = classifyOut->flags | FWPS_CLASSIFY_OUT_FLAG_ABSORB;
	}
	// 协议判断
	ProtocalIdToName(inFixedValues->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_PROTOCOL].value.uint16, szProtocalName);
	// 显示
	DbgPrint("Protocal=%s, LocalIp=%u.%u.%u.%u:%d, RemoteIp=%u.%u.%u.%u:%d, IRQL=%d, PID=%I64d\n",
		szProtocalName,
		(ulLocalIp >> 24) & 0xFF,
		(ulLocalIp >> 16) & 0xFF,
		(ulLocalIp >> 8) & 0xFF,
		(ulLocalIp) & 0xFF,
		uLocalPort,
		(ulRemoteIp >> 24) & 0xFF,
		(ulRemoteIp >> 16) & 0xFF,
		(ulRemoteIp >> 8) & 0xFF,
		(ulRemoteIp) & 0xFF,
		uRemotePort,
		kCurrentIrql,
		processId);
	//蓝屏信息定位到打印信息函数，错误栈往上看是Rtl操作字符是分配内存报错，
	//估计是由于路径都比较长，频繁打印内存紧张导致分配失败，最后蓝屏，去掉路径打印后就稳定了

	/**
	 * 错误栈：

*******************************************************************************
*                                                                             *
*                        Bugcheck Analysis                                    *
*                                                                             *
*******************************************************************************

IRQL_NOT_LESS_OR_EQUAL (a)
An attempt was made to access a pageable (or completely invalid) address at an
interrupt request level (IRQL) that is too high.  This is usually
caused by drivers using improper addresses.
If a kernel debugger is available get the stack backtrace.
Arguments:
Arg1: 000000553fe710a0, memory referenced
Arg2: 0000000000000002, IRQL
Arg3: 0000000000000000, bitfield :
	bit 0 : value 0 = read operation, 1 = write operation
	bit 3 : value 0 = not an execute operation, 1 = execute operation (only on chips which support this level of status)
Arg4: fffff8051d2d1726, address which referenced memory

Debugging Details:
------------------

KEY_VALUES_STRING: 1

	Key  : Analysis.CPU.Sec
	Value: 2

	Key  : Analysis.DebugAnalysisProvider.CPP
	Value: Create: 8007007e on 955F

	Key  : Analysis.DebugData
	Value: CreateObject

	Key  : Analysis.DebugModel
	Value: CreateObject

	Key  : Analysis.Elapsed.Sec
	Value: 2

	Key  : Analysis.Memory.CommitPeak.Mb
	Value: 76

	Key  : Analysis.System
	Value: CreateObject


BUGCHECK_CODE:  a

BUGCHECK_P1: 553fe710a0

BUGCHECK_P2: 2

BUGCHECK_P3: 0

BUGCHECK_P4: fffff8051d2d1726

READ_ADDRESS: fffff8051d21b3b8: Unable to get MiVisibleState
Unable to get NonPagedPoolStart
Unable to get NonPagedPoolEnd
Unable to get PagedPoolStart
Unable to get PagedPoolEnd
fffff8051d0d23c8: Unable to get Flags value from nt!KdVersionBlock
fffff8051d0d23c8: Unable to get Flags value from nt!KdVersionBlock
unable to get nt!MmSpecialPagesInUse
 000000553fe710a0

BLACKBOXBSD: 1 (!blackboxbsd)


BLACKBOXNTFS: 1 (!blackboxntfs)


BLACKBOXPNP: 1 (!blackboxpnp)


BLACKBOXWINLOGON: 1

CUSTOMER_CRASH_COUNT:  1

PROCESS_NAME:  svchost.exe

DPC_STACK_BASE:  FFFFBF0A20830FB0

TRAP_FRAME:  ffffbf0a2082d5d0 -- (.trap 0xffffbf0a2082d5d0)
NOTE: The trap frame does not contain all registers.
Some register values may be zeroed or incorrect.
rax=ffffe705c049a3c0 rbx=0000000000000000 rcx=0000000000000000
rdx=000000553fe71000 rsi=0000000000000000 rdi=0000000000000000
rip=fffff8051d2d1726 rsp=ffffbf0a2082d760 rbp=ffffbf0a2082d950
 r8=ffffbf0a2082d818  r9=ffffbf0a2082d828 r10=ffffbf0a2082dac0
r11=0000000000000000 r12=0000000000000000 r13=0000000000000000
r14=0000000000000000 r15=0000000000000000
iopl=0         nv up ei pl zr na po nc
nt!RtlpIsUtf8Process+0x36:
fffff805`1d2d1726 4883baa000000000 cmp     qword ptr [rdx+0A0h],0 ds:00000055`3fe710a0=????????????????
Resetting default scope

STACK_TEXT:
ffffbf0a`2082d488 fffff805`1ce7b2e9 : 00000000`0000000a 00000055`3fe710a0 00000000`00000002 00000000`00000000 : nt!KeBugCheckEx
ffffbf0a`2082d490 fffff805`1ce7762b : 00000000`00000000 00000000`00000000 ff070000`00000000 0000ffff`ffffffff : nt!KiBugCheckDispatch+0x69
ffffbf0a`2082d5d0 fffff805`1d2d1726 : ffffffff`ffffff00 00000000`000000c0 00000000`00000000 fffff805`1d3ad9c7 : nt!KiPageFault+0x46b
ffffbf0a`2082d760 fffff805`1d3ad9c7 : fcffffff`ffffff1f fffff805`1cd5b614 00000000`00000000 01000000`00000000 : nt!RtlpIsUtf8Process+0x36
ffffbf0a`2082d780 fffff805`1ce48347 : ffffbf0a`2082d8b0 00000000`00000033 00000000`00000800 00000000`00000000 : nt!RtlUnicodeToMultiByteN+0x27
ffffbf0a`2082d7d0 fffff805`1ce483cf : 00000000`00000000 fffff805`1ce49d70 0000e0ff`ffffffff 00000000`0000005c : nt!wctomb_s_l+0x83
ffffbf0a`2082d810 fffff805`1ce499db : ffffbf0a`2082dde0 00000000`00000000 00000000`00000800 00000000`00000000 : nt!wctomb_s+0xf
ffffbf0a`2082d850 fffff805`1ce45263 : 00000000`00000000 ffffbf0a`00000000 00000000`0000007f ffffbf0a`2082dc30 : nt!output_l+0x583
ffffbf0a`2082db10 fffff805`1ce451e1 : 00000000`0000007f ffffbf0a`2082dc90 00000000`00000000 00000000`00000010 : nt!vsnprintf_l+0x77
ffffbf0a`2082db80 fffff805`1cdebf8f : 00000000`00000001 00000000`00000030 00000000`00000030 ffffe705`ba010000 : nt!vsnprintf+0x11
ffffbf0a`2082dbc0 fffff805`1cdc5d07 : 00000000`00000000 ffffbf0a`2082dc10 ffffe705`ba010340 ffffe705`ba010800 : nt!RtlStringCbVPrintfA+0x3f
ffffbf0a`2082dbf0 fffff805`1cdc5bdc : 00000000`000000ff 00000000`000000ff 00000000`00000037 ffffbf0a`2082dd00 : nt!vDbgPrintExWithPrefixInternal+0xe7
ffffbf0a`2082dcf0 fffff805`1c6423c8 : fffff805`1c642650 ffffbf0a`2082dee0 00000000`000000ff 00000000`000000ff : nt!DbgPrint+0x3c
	
	*/
}

// Callout函数 notifyFn
//#if (NTDDI_VERSION >= NTDDI_WIN10_RS3)
//NTSTATUS NTAPI notifyFn(
//	_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
//	_In_ const GUID* filterKey,
//	_Inout_ FWPS_FILTER3* filter
//)
//#elif (NTDDI_VERSION >= NTDDI_WIN8)
//NTSTATUS NTAPI notifyFn(
//	_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
//	_In_ const GUID* filterKey,
//	_Inout_ FWPS_FILTER2* filter
//)
//#elif (NTDDI_VERSION >= NTDDI_WIN7)
//NTSTATUS NTAPI notifyFn(
//	_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
//	_In_ const GUID* filterKey,
//	_Inout_ FWPS_FILTER1* filter
//)
//#else
//NTSTATUS NTAPI notifyFn(
//	_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
//	_In_ const GUID* filterKey,
//	_Inout_ FWPS_FILTER0* filter
//)
//#endif
NTSTATUS NTAPI notifyFn(
	_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
	_In_ const GUID* filterKey,
	_Inout_ FWPS_FILTER* filter
)
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