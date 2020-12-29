#ifndef _MM_LOAD_DLL_H_
#define _MM_LOAD_DLL_H_


#include <Windows.h>


typedef BOOL(__stdcall *typedef_DllMain)(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved);


void ShowError(char *lpszText);

// 模拟LoadLibrary加载内存DLL文件到进程中
// lpData: 内存DLL文件数据的基址
// dwSize: 内存DLL文件的内存大小
// 返回值: 内存DLL加载到进程的加载基址
LPVOID MmLoadLibrary(LPVOID lpData, DWORD dwSize);

// 根据PE结构,获取PE文件加载到内存后的镜像大小
// lpData: 内存DLL文件数据的基址
// 返回值: 返回PE文件结构中IMAGE_NT_HEADERS.OptionalHeader.SizeOfImage值的大小
DWORD GetSizeOfImage(LPVOID lpData);

// 将内存DLL数据按SectionAlignment大小对齐映射到进程内存中
// lpData: 内存DLL文件数据的基址
// lpBaseAddress: 内存DLL数据按SectionAlignment大小对齐映射到进程内存中的内存基址
// 返回值: 成功返回TRUE，否则返回FALSE
BOOL MmMapFile(LPVOID lpData, LPVOID lpBaseAddress);

// 对齐SectionAlignment
// dwSize: 表示未对齐前内存的大小
// dwAlignment: 对齐大小值
// 返回值: 返回内存对齐之后的值
DWORD Align(DWORD dwSize, DWORD dwAlignment);

// 修改PE文件重定位表信息
// lpBaseAddress: 内存DLL数据按SectionAlignment大小对齐映射到进程内存中的内存基址
// 返回值: 成功返回TRUE，否则返回FALSE
BOOL DoRelocationTable(LPVOID lpBaseAddress);

// 填写PE文件导入表信息
// lpBaseAddress: 内存DLL数据按SectionAlignment大小对齐映射到进程内存中的内存基址
// 返回值: 成功返回TRUE，否则返回FALSE
BOOL DoImportTable(LPVOID lpBaseAddress);

// 修改PE文件加载基址IMAGE_NT_HEADERS.OptionalHeader.ImageBase
// lpBaseAddress: 内存DLL数据按SectionAlignment大小对齐映射到进程内存中的内存基址
// 返回值: 成功返回TRUE，否则返回FALSE
BOOL SetImageBase(LPVOID lpBaseAddress);

// 调用DLL的入口函数DllMain,函数地址即为PE文件的入口点IMAGE_NT_HEADERS.OptionalHeader.AddressOfEntryPoint
// lpBaseAddress: 内存DLL数据按SectionAlignment大小对齐映射到进程内存中的内存基址
// 返回值: 成功返回TRUE，否则返回FALSE
BOOL CallDllMain(LPVOID lpBaseAddress);

// 模拟GetProcAddress获取内存DLL的导出函数
// lpBaseAddress: 内存DLL文件加载到进程中的加载基址
// lpszFuncName: 导出函数的名字
// 返回值: 返回导出函数的的地址
LPVOID MmGetProcAddress(LPVOID lpBaseAddress, PCHAR lpszFuncName);

// 释放从内存加载的DLL到进程内存的空间
// lpBaseAddress: 内存DLL数据按SectionAlignment大小对齐映射到进程内存中的内存基址
// 返回值: 成功返回TRUE，否则返回FALSE
BOOL MmFreeLibrary(LPVOID lpBaseAddress);


#endif