#ifndef _TDI_TCP_H_
#define _TDI_TCP_H_


#include <ntddk.h>
#include <tdikrnl.h>

// TCP驱动设备名称
#define COMM_TCP_DEV_NAME L"\\Device\\Tcp"

// 地址转换的宏
#define INETADDR(a, b, c, d) (a + (b<<8) + (c<<16) + (d<<24))
#define HTONL(a) (((a & 0xFF)<<24) + ((a & 0xFF00)<<8) + ((a & 0xFF0000)>>8) + (a&0xFF000000)>>24)
#define HTONS(a) (((a & 0xFF)<<8) + ((a & 0xFF00)>>8))


VOID ShowError(PCHAR lpszText, NTSTATUS ntStatus);
// 完成回调函数
NTSTATUS TdiCompletionRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp, PVOID pContext);
// TDI初始化设置
NTSTATUS TdiOpen(PDEVICE_OBJECT *ppTdiAddressDevObj, PFILE_OBJECT *ppTdiEndPointFileObject, HANDLE *phTdiAddress, HANDLE *phTdiEndPoint);
// TDI TCP连接服务器
NTSTATUS TdiConnection(PDEVICE_OBJECT pTdiAddressDevObj, PFILE_OBJECT pTdiEndPointFileObject, LONG *pServerIp, LONG lServerPort);
// TDI TCP发送信息
NTSTATUS TdiSend(PDEVICE_OBJECT pTdiAddressDevObj, PFILE_OBJECT pTdiEndPointFileObject, PUCHAR pSendData, ULONG ulSendDataLength);
// TDI TCP接收信息
ULONG_PTR TdiRecv(PDEVICE_OBJECT pTdiAddressDevObj, PFILE_OBJECT pTdiEndPointFileObject, PUCHAR pRecvData, ULONG ulRecvDataLength);
// TDI关闭释放
VOID TdiClose(PFILE_OBJECT pTdiEndPointFileObject, HANDLE hTdiAddress, HANDLE hTdiEndPoint);


#endif