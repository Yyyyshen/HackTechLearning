#include "NotifyRoutine_Obj.h"


// 设置进程回调函数
NTSTATUS SetProcessCallbacks()
{
	NTSTATUS status = STATUS_SUCCESS;
	OB_CALLBACK_REGISTRATION obCallbackReg = { 0 };
	OB_OPERATION_REGISTRATION obOperationReg = { 0 };

	RtlZeroMemory(&obCallbackReg, sizeof(OB_CALLBACK_REGISTRATION));
	RtlZeroMemory(&obOperationReg, sizeof(OB_OPERATION_REGISTRATION));

	// 设置 OB_CALLBACK_REGISTRATION
	obCallbackReg.Version = ObGetFilterVersion();
	obCallbackReg.OperationRegistrationCount = 1;
	obCallbackReg.RegistrationContext = NULL;
	RtlInitUnicodeString(&obCallbackReg.Altitude, L"321000");
	obCallbackReg.OperationRegistration = &obOperationReg;

	// 设置 OB_OPERATION_REGISTRATION
	// Thread 和 Process 的区别所在
	obOperationReg.ObjectType = PsProcessType;                                   
	obOperationReg.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
	// Thread 和 Process 的区别所在
	obOperationReg.PreOperation = (POB_PRE_OPERATION_CALLBACK)(&ProcessPreCall); 

	// 注册回调函数
	status = ObRegisterCallbacks(&obCallbackReg, &g_obProcessHandle);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("ObRegisterCallbacks Error[0x%X]\n", status);
		return status;
	}

	return status;
}


// 设置线程回调函数
NTSTATUS SetThreadCallbacks()
{
	NTSTATUS status = STATUS_SUCCESS;
	OB_CALLBACK_REGISTRATION obCallbackReg = { 0 };
	OB_OPERATION_REGISTRATION obOperationReg = { 0 };

	RtlZeroMemory(&obCallbackReg, sizeof(OB_CALLBACK_REGISTRATION));
	RtlZeroMemory(&obOperationReg, sizeof(OB_OPERATION_REGISTRATION));

	// 设置 OB_CALLBACK_REGISTRATION
	obCallbackReg.Version = ObGetFilterVersion();
	obCallbackReg.OperationRegistrationCount = 1;
	obCallbackReg.RegistrationContext = NULL;
	RtlInitUnicodeString(&obCallbackReg.Altitude, L"321001");
	obCallbackReg.OperationRegistration = &obOperationReg;

	// 设置 OB_OPERATION_REGISTRATION
	// Thread 和 Process 的区别所在
	obOperationReg.ObjectType = PsThreadType;                                   
	obOperationReg.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
	// Thread 和 Process 的区别所在
	obOperationReg.PreOperation = (POB_PRE_OPERATION_CALLBACK)(&ThreadPreCall); 

	// 注册回调函数
	status = ObRegisterCallbacks(&obCallbackReg, &g_obThreadHandle);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("ObRegisterCallbacks Error[0x%X]\n", status);
		return status;
	}

	return status;
}


// 删除进程回调函数
VOID RemoveProcessCallbacks()
{
	// 卸载回调
	if (NULL != g_obProcessHandle)
	{
		ObUnRegisterCallbacks(g_obProcessHandle);
		g_obProcessHandle = NULL;
	}
}


// 删除线程回调函数
VOID RemoveThreadCallbacks()
{
	// 卸载回调
	if (NULL != g_obThreadHandle)
	{
		ObUnRegisterCallbacks(g_obThreadHandle);
		g_obThreadHandle = NULL;
	}
}


// 进程回调函数
OB_PREOP_CALLBACK_STATUS ProcessPreCall(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pObPreOperationInfo)
{
	PEPROCESS pEProcess = NULL;

	// 判断对象类型
	if (*PsProcessType != pObPreOperationInfo->ObjectType)
	{
		return OB_PREOP_SUCCESS;
	}
	// 获取进程结构对象
	pEProcess = (PEPROCESS)pObPreOperationInfo->Object;
	// 判断是否市保护PID, 若是, 则拒绝结束进程  //这里有点问题，我自己测试时可以结束进程但打不开进程
	if (IsProtectProcess(pEProcess))
	{
		// 操作类型: 创建句柄
		if (OB_OPERATION_HANDLE_CREATE == pObPreOperationInfo->Operation)
		{
			if (1 == (1 & pObPreOperationInfo->Parameters->CreateHandleInformation.OriginalDesiredAccess))
			{
				pObPreOperationInfo->Parameters->CreateHandleInformation.DesiredAccess = 0;
			}
		}
		// 操作类型: 复制句柄
		else if (OB_OPERATION_HANDLE_DUPLICATE == pObPreOperationInfo->Operation)
		{
			if (1 == (1 & pObPreOperationInfo->Parameters->DuplicateHandleInformation.OriginalDesiredAccess))
			{
				pObPreOperationInfo->Parameters->DuplicateHandleInformation.DesiredAccess = 0;
			}
		}
	}

	return OB_PREOP_SUCCESS;
}


// 线程回调函数
OB_PREOP_CALLBACK_STATUS ThreadPreCall(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pObPreOperationInfo)
{
	PEPROCESS pEProcess = NULL;

	// 判断对象类型
	if (*PsThreadType != pObPreOperationInfo->ObjectType)
	{
		return OB_PREOP_SUCCESS;
	}
	// 获取线程对应的进程 PEPROCESS
	pEProcess = IoThreadToProcess((PETHREAD)pObPreOperationInfo->Object);
	// 判断是否市保护PID, 若是, 则拒绝结束线程
	if (IsProtectProcess(pEProcess))
	{
		// 操作类型: 创建句柄
		if (OB_OPERATION_HANDLE_CREATE == pObPreOperationInfo->Operation)
		{
			if (1 == (1 & pObPreOperationInfo->Parameters->CreateHandleInformation.OriginalDesiredAccess))
			{
				pObPreOperationInfo->Parameters->CreateHandleInformation.DesiredAccess = 0;
			}
		}
		// 操作类型: 复制句柄
		else if (OB_OPERATION_HANDLE_DUPLICATE == pObPreOperationInfo->Operation)
		{
			if (1 == (1 & pObPreOperationInfo->Parameters->DuplicateHandleInformation.OriginalDesiredAccess))
			{
				pObPreOperationInfo->Parameters->DuplicateHandleInformation.DesiredAccess = 0;
			}
		}
	}

	return OB_PREOP_SUCCESS;
}


// 判断是否为保护进程
BOOLEAN IsProtectProcess(PEPROCESS pEProcess)
{
	PUCHAR pProcessName = PsGetProcessImageFileName(pEProcess);
	if (NULL != pProcessName)
	{
		if (0 == _stricmp(pProcessName, "Winobj.exe"))
		{
			DbgPrint("[Protect]");
			return TRUE;
		}

		DbgPrint("ProcessName=%s\n", pProcessName);
	}
	return FALSE;
}