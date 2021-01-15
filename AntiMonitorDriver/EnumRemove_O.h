#ifndef _ENUM_REMOVE_O_H_
#define _ENUM_REMOVE_O_H_


#include <ntifs.h>
#include <wdm.h>


typedef struct _OBJECT_TYPE_INITIALIZER
{
	USHORT Length;					  // Uint2B
	UCHAR ObjectTypeFlags;			  // UChar
	ULONG ObjectTypeCode;			  // Uint4B
	ULONG InvalidAttributes;		  // Uint4B
	GENERIC_MAPPING GenericMapping;	  // _GENERIC_MAPPING
	ULONG ValidAccessMask;			 // Uint4B
	ULONG RetainAccess;				  // Uint4B
	POOL_TYPE PoolType;				 // _POOL_TYPE
	ULONG DefaultPagedPoolCharge;	 // Uint4B
	ULONG DefaultNonPagedPoolCharge; // Uint4B
	PVOID DumpProcedure;			 // Ptr64     void
	PVOID OpenProcedure;			// Ptr64     long
	PVOID CloseProcedure;			// Ptr64     void
	PVOID DeleteProcedure;				// Ptr64     void
	PVOID ParseProcedure;			// Ptr64     long
	PVOID SecurityProcedure;			// Ptr64     long
	PVOID QueryNameProcedure;			// Ptr64     long
	PVOID OkayToCloseProcedure;			// Ptr64     unsigned char
#if (NTDDI_VERSION >= NTDDI_WINBLUE)    // Win8.1
	ULONG WaitObjectFlagMask;			// Uint4B
	USHORT WaitObjectFlagOffset;		// Uint2B
	USHORT WaitObjectPointerOffset;		// Uint2B
#endif
}OBJECT_TYPE_INITIALIZER, *POBJECT_TYPE_INITIALIZER;

typedef struct _OBJECT_TYPE
{
	LIST_ENTRY TypeList;			     // _LIST_ENTRY
	UNICODE_STRING Name;				 // _UNICODE_STRING
	PVOID DefaultObject;				 // Ptr64 Void
	UCHAR Index;						 // UChar
	ULONG TotalNumberOfObjects;			 // Uint4B
	ULONG TotalNumberOfHandles;			 // Uint4B
	ULONG HighWaterNumberOfObjects;		 // Uint4B
	ULONG HighWaterNumberOfHandles;		 // Uint4B
	OBJECT_TYPE_INITIALIZER TypeInfo;	 // _OBJECT_TYPE_INITIALIZER
	EX_PUSH_LOCK TypeLock;				 // _EX_PUSH_LOCK
	ULONG Key;						     // Uint4B
	LIST_ENTRY CallbackList;			 // _LIST_ENTRY
}OBJECT_TYPE, *POBJECT_TYPE;

#pragma pack(1)
typedef struct _OB_CALLBACK
{
	LIST_ENTRY ListEntry;
	ULONGLONG Unknown;
	HANDLE ObHandle;
	PVOID ObTypeAddr;
	PVOID	PreCall;
	PVOID PostCall;
}OB_CALLBACK, *POB_CALLBACK;
#pragma pack()


// 获取进程对象类型回调
BOOLEAN EnumProcessObCallback();

// 获取线程对象类型回调
BOOLEAN EnumThreadObCallback();

// 移除回调
NTSTATUS RemoveObCallback(PVOID RegistrationHandle);


#endif