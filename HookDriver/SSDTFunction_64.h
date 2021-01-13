#ifndef _SSDT_FUNCTION_H_
#define _SSDT_FUNCTION_H_


#include <ntddk.h>
#include <ntimage.h>


#pragma pack(1)
typedef struct _SERVICE_DESCIPTOR_TABLE_64
{
	PULONG ServiceTableBase;		  // SSDT基址
	PVOID ServiceCounterTableBase; // SSDT中服务被调用次数计数器
	ULONGLONG NumberOfService;     // SSDT服务个数
	PVOID ParamTableBase;		  // 系统服务参数表基址
}SSDTEntry_64, *PSSDTEntry_64;
#pragma pack()


// 获取 SSDT 函数地址
PVOID GetSSDTFunction_64(PCHAR pszFunctionName);

// 根据特征码, 从 KiSystemCall64 中获取 SSDT 地址
PVOID GetSSDTAddress();


#endif