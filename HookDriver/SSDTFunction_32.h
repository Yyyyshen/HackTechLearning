#ifndef _SSDT_FUNCTION_H_
#define _SSDT_FUNCTION_H_


#include <ntddk.h>
#include <ntimage.h>


#pragma pack(1)
typedef struct _SERVICE_DESCIPTOR_TABLE_32
{
	PULONG ServiceTableBase;		  // SSDT基址
	PULONG ServiceCounterTableBase;   // SSDT中服务被调用次数计数器
	ULONG NumberOfService;            // SSDT服务个数
	PUCHAR ParamTableBase;		      // 系统服务参数表基址
}SSDTEntry_32, *PSSDTEntry_32;
#pragma pack()

// 直接获取Ntoskrnl.exe导出符号获取 SSDT 
extern SSDTEntry_32 __declspec(dllimport) KeServiceDescriptorTable; //32位才能编译过

// 获取 SSDT 函数地址
PVOID GetSSDTFunction_32(PCHAR pszFunctionName);


#endif