#ifndef _PIPE_CMD_H_
#define _PIPE_CMD_H_


#include <Windows.h>


// 执行 cmd 命令, 并获取执行结果数据
BOOL PipeCmd(char *pszCmd, char *pszResultBuffer, DWORD dwResultBufferSize);


#endif