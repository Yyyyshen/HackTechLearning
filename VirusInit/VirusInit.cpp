// VirusInit.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "windows.h"
/**
 * 恶意程序常见初始化操作技术
 * 
 * 互斥量运行单一实例
 * DLL延迟加载
 * 资源释放
 * 
 */

//互斥量使用
//为保证恶意程序不会因多次运行而容易被发现，最好运行单一实例
BOOL IsAreadyRun()
{
    //互斥量句柄
    HANDLE hMutex = NULL;
    //创建互斥量，注意名称
    hMutex = ::CreateMutex(NULL, FALSE, L"TEST");
    //返回值为NULL,则失败，若返回一个句柄，可能为新创建，也可能是已经存在，需要通过GetLastError进行确认
    if (hMutex)
    {
        if (ERROR_ALREADY_EXISTS == ::GetLastError())
        {
            return TRUE;
        }
    }
    return FALSE;
}

//DLL延迟加载



//资源释放



int main()
{
    if (IsAreadyRun())
    {
        std::cout << "Already running!" << std::endl;
        return 0;
    }
    std::cout << "Hello World!\n";
    return 0;
}

