#ifndef _ENUM_INFO_H_
#define _ENUM_INFO_H_


#include <Windows.h>
#include <TlHelp32.h>
#include <stdio.h>

BOOL EnumProcess();
BOOL EnumThread();
BOOL EnumProcessModule(DWORD dwProcessId);

#endif