#ifndef _GET_MIPROCESSLOADERENTRY_H_
#define _GET_MIPROCESSLOADERENTRY_H_


#include <ntddk.h>


#ifndef _WIN64
    // 32
    typedef NTSTATUS(__stdcall *typedef_MiProcessLoaderEntry)(PVOID, BOOLEAN);
#else 
    // 64
    typedef NTSTATUS(__fastcall *typedef_MiProcessLoaderEntry)(PVOID, BOOLEAN);
#endif


// 从 MmLoadSystemImage 中获取对应的 MiProcessLoaderEntry 特征码
// 其中, 32和64位的 Win7, Win8.1 直接从 MmLoadSystemImage 中搜索 MiProcessLoaderEntry
// 32和64位的 Win10 需要从 MmLoadSystemImage 中搜索 MiConstructLoaderEntry, 再从 MiConstructLoaderEntry 中搜索 MiProcessLoaderEntry
PVOID GetFuncAddr_MiProcessLoaderEntry();

// 从 NtSetSystemInformation 中获取 MmLoadSystemImage 函数地址
PVOID GetFuncAddr_MmLoadSystemImage();

// 搜索特征码
PVOID SearchSpecialCode(PVOID pSearchBeginAddr, ULONG ulSearchLength, PUCHAR pSpecialCode, ULONG ulSpecialCodeLength);


#endif





