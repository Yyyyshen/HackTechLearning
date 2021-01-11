#ifndef _RAW_INPUT_TEST_H_
#define _RAW_INPUT_TEST_H_


#include <stdio.h>
#include <Windows.h>


// 注册原始输入设备
BOOL Init(HWND hWnd);

// 获取原始输入数据
BOOL GetData(LPARAM lParam);

// 保存按键信息
void SaveKey(USHORT usVKey);


#endif