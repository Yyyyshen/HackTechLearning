#pragma once
#include <ntddk.h>
#include <Ntddkbd.h>


#define DEV_NAME L"\\Device\\KEYBOARD_LOG_DEV_NAME"
#define SYM_NAME L"\\DosDevices\\KEYBOARD_LOG_SYM_NAME"
#define IOCTL_TEST CTL_CODE(FILE_DEVICE_KEYBOARD, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)


typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT pAttachDevObj;
	ULONG ulIrpInQuene;

}DEVICE_EXTENSION, * PDEVICE_EXTENSION;



VOID DriverUnload(PDRIVER_OBJECT pDriverObject);
NTSTATUS DriverDefaultHandle(PDEVICE_OBJECT pDevObj, PIRP pIrp);
NTSTATUS DriverControlHandle(PDEVICE_OBJECT pDevObj, PIRP pIrp);
NTSTATUS DriverRead(PDEVICE_OBJECT pDevObj, PIRP pIrp);
NTSTATUS DriverPower(PDEVICE_OBJECT pDevObj, PIRP pIrp);
NTSTATUS ReadCompleteRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp, PVOID pContext);
NTSTATUS CreateDevice(PDRIVER_OBJECT pDriverObject);
NTSTATUS AttachKdbClass(PDEVICE_OBJECT pDevObj);
