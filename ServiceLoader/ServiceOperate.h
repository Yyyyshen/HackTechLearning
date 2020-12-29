#ifndef _SERVICE_OPERATE_H_
#define _SERVICE_OPERATE_H_


#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")


// 0 加载服务    1 启动服务    2 停止服务    3 删除服务
BOOL SystemServiceOperate(char *lpszDriverPath, int iOperateType);


#endif