#ifndef _DRIVEr_H_
#define _DRIVEr_H_

#include <ntstatus.h>

#include <ntddk.h>


#define DEV_NAME L"\\Device\\MY_TDI_DEV_NAME"
#define SYM_NAME L"\\DosDevices\\MY_TDI_SYM_NAME"


VOID DriverUnload(PDRIVER_OBJECT pDriverObject);
NTSTATUS DriverDefaultHandle(PDEVICE_OBJECT pDevObj, PIRP pIrp);

NTSTATUS CreateDevice(PDRIVER_OBJECT pDriverObject);

#endif