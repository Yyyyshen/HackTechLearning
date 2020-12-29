#ifndef _MM_LOAD_EXE_H_
#define _MM_LOAD_EXE_H_


#include <Windows.h>


void ShowErrorExe(char *lpszText);


// 判断是否有重定位表存在
BOOL IsExistRelocationTable(LPVOID lpBaseAddress);


// 模拟PE加载器加载内存EXE文件到进程中
// lpData: 内存EXE文件数据的基址
// dwSize: 内存EXE文件的内存大小
// 返回值: 内存EXE加载到进程的加载基址
LPVOID MmRunExe(LPVOID lpData, DWORD dwSize);


// 根据PE结构,获取PE文件加载到内存后的镜像大小
// lpData: 内存DLL文件数据的基址
// 返回值: 返回PE文件结构中IMAGE_NT_HEADERS.OptionalHeader.SizeOfImage值的大小
DWORD GetSizeOfImageExe(LPVOID lpData);


// 将内存DLL数据按SectionAlignment大小对齐映射到进程内存中
// lpData: 内存DLL文件数据的基址
// lpBaseAddress: 内存DLL数据按SectionAlignment大小对齐映射到进程内存中的内存基址
// 返回值: 成功返回TRUE，否则返回FALSE
BOOL MmMapFileExe(LPVOID lpData, LPVOID lpBaseAddress);


// 对齐SectionAlignment
// dwSize: 表示未对齐前内存的大小
// dwAlignment: 对齐大小值
// 返回值: 返回内存对齐之后的值
DWORD AlignExe(DWORD dwSize, DWORD dwAlignment);


// 修改PE文件重定位表信息
// lpBaseAddress: 内存DLL数据按SectionAlignment大小对齐映射到进程内存中的内存基址
// 返回值: 成功返回TRUE，否则返回FALSE
BOOL DoRelocationTableExe(LPVOID lpBaseAddress);


// 填写PE文件导入表信息
// lpBaseAddress: 内存DLL数据按SectionAlignment大小对齐映射到进程内存中的内存基址
// 返回值: 成功返回TRUE，否则返回FALSE
BOOL DoImportTableExe(LPVOID lpBaseAddress);


// 修改PE文件加载基址IMAGE_NT_HEADERS.OptionalHeader.ImageBase
// lpBaseAddress: 内存DLL数据按SectionAlignment大小对齐映射到进程内存中的内存基址
// 返回值: 成功返回TRUE，否则返回FALSE
BOOL SetImageBaseExe(LPVOID lpBaseAddress);


// 跳转到PE的入口点处执行, 函数地址即为PE文件的入口点IMAGE_NT_HEADERS.OptionalHeader.AddressOfEntryPoint
// lpBaseAddress: 内存PE数据按SectionAlignment大小对齐映射到进程内存中的内存基址
// 返回值: 成功返回TRUE，否则返回FALSE
BOOL CallExeEntry(LPVOID lpBaseAddress);


#endif