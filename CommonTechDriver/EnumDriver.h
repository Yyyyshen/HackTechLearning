#ifndef _DRIVER_ENUM_H_
#define _DRIVER_ENUM_H_


#include <ntddk.h>


// 注意32位与64位的对齐大小
#ifndef _WIN64
    #pragma pack(1)                               
#endif

typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	union
	{
		LIST_ENTRY HashLinks;
		struct
		{
			PVOID SectionPointer;
			ULONG CheckSum;
		};
	};
	union
	{
		ULONG TimeDateStamp;
		PVOID LoadedImports;
	};
	PVOID EntryPointActivationContext;
	PVOID PatchInformation;
	LIST_ENTRY ForwarderLinks;
	LIST_ENTRY ServiceTagLinks;
	LIST_ENTRY StaticLinks;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

#ifndef _WIN64
    #pragma pack()
#endif


// 驱动模块遍历
BOOLEAN EnumDriver(PDRIVER_OBJECT pDriverObject);

// 驱动模块隐藏(Bypass Patch Guard)
BOOLEAN HideDriver_Bypass_PatchGuard(PDRIVER_OBJECT pDriverObject, UNICODE_STRING ustrHideDriverName);


#endif