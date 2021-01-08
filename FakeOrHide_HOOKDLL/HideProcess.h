#ifndef _HIDE_PROCESS_H_
#define _HIDE_PROCESS_H_


#include <Windows.h>
#include <Winternl.h>


typedef NTSTATUS(*typedef_ZwQuerySystemInformation)(
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