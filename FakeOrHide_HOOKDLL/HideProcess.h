#ifndef _HIDE_PROCESS_H_
#define _HIDE_PROCESS_H_


#include <Windows.h>
#include <Winternl.h>

//HOOK函数声明要加WINAPI，否则默认使用C语言调用约定，导致在函数返回过程中，因堆栈不平衡而报错
typedef NTSTATUS(WINAPI* typedef_ZwQuerySystemInformation)(
	SYSTEM_INFORMATION_CLASS SystemInformationClass,
	PVOID SystemInformation,
	ULONG SystemInformationLength,
	PULONG ReturnLength
	);


NTSTATUS New_ZwQuerySystemInformation(
	SYSTEM_INFORMATION_CLASS SystemInformationClass,
	PVOID SystemInformation,
	ULONG SystemInformationLength,
	PULONG ReturnLength
);


void HookApi();

void UnhookApi();

#endif