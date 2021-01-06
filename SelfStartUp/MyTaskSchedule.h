#ifndef _MY_TASK_SCHEDULT_H_
#define _MY_TASK_SCHEDULT_H_


#include <Atlbase.h>
#include <comdef.h>
#include <taskschd.h>
#pragma comment(lib, "taskschd.lib")


class CMyTaskSchedule
{
private:

	ITaskService* m_lpITS;
	ITaskFolder* m_lpRootFolder;

public:

	CMyTaskSchedule(void);
	~CMyTaskSchedule(void);

public:

	// 删除指定任务计划
	BOOL Delete(const char* lpszTaskName);
	BOOL DeleteFolder(const char* lpszFolderName);

	// 创建任务计划
	BOOL NewTask(const char* lpszTaskName, const char* lpszProgramPath, const char* lpszParameters, const char* lpszAuthor = "");

	// 判断指定任务计划是否存在
	BOOL IsExist(const char* lpszTaskName);

	// 判断指定任务计划状态是否有效
	BOOL IsTaskValid(const char* lpszTaskName);

	// 运行指定任务计划
	BOOL Run(const char* lpszTaskName, const char* lpszParam);

	// 判断指定任务计划是否启动
	BOOL IsEnable(const char* lpszTaskName);

	// 设置指定任务计划是否启动还是禁用
	BOOL SetEnable(const char* lpszTaskName, BOOL bEnable);

};


#endif