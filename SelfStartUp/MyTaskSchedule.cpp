#include "MyTaskSchedule.h"


void ShowError(const char* lpszText, DWORD dwErrCode)
{
	char szErr[MAX_PATH] = { 0 };
	::wsprintf(szErr, "%s Error!\nError Code Is:0x%08x\n", lpszText, dwErrCode);
	::MessageBox(NULL, szErr, "ERROR", MB_OK | MB_ICONERROR);
}


CMyTaskSchedule::CMyTaskSchedule(void)
{
	m_lpITS = NULL;
	m_lpRootFolder = NULL;
	// 初始化COM
	HRESULT hr = ::CoInitialize(NULL);
	if (FAILED(hr))
	{
		ShowError("CoInitialize", hr);
	}
	// 创建一个任务服务（Task Service）实例
	hr = ::CoCreateInstance(CLSID_TaskScheduler,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_ITaskService,
		(LPVOID*)(&m_lpITS));
	if (FAILED(hr))
	{
		ShowError("CoCreateInstance", hr);
	}
	// 连接到任务服务（Task Service）
	hr = m_lpITS->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
	if (FAILED(hr))
	{
		ShowError("ITaskService::Connect", hr);
	}
	// 获取Root Task Folder的指针，这个指针指向的是新注册的任务
	hr = m_lpITS->GetFolder(_bstr_t("\\"), &m_lpRootFolder);
	if (FAILED(hr))
	{
		ShowError("ITaskService::GetFolder", hr);
	}
}


CMyTaskSchedule::~CMyTaskSchedule(void)
{
	if (m_lpITS)
	{
		m_lpITS->Release();
	}
	if (m_lpRootFolder)
	{
		m_lpRootFolder->Release();
	}
	// 卸载COM
	::CoUninitialize();
}


BOOL CMyTaskSchedule::Delete(const char* lpszTaskName)
{
	if (NULL == m_lpRootFolder)
	{
		return FALSE;
	}
	CComVariant variantTaskName(NULL);
	variantTaskName = lpszTaskName;
	HRESULT hr = m_lpRootFolder->DeleteTask(variantTaskName.bstrVal, 0);
	if (FAILED(hr))
	{
		return FALSE;
	}

	return TRUE;
}


BOOL CMyTaskSchedule::DeleteFolder(const char* lpszFolderName)
{
	if (NULL == m_lpRootFolder)
	{
		return FALSE;
	}
	CComVariant variantFolderName(NULL);
	variantFolderName = lpszFolderName;
	HRESULT hr = m_lpRootFolder->DeleteFolder(variantFolderName.bstrVal, 0);
	if (FAILED(hr))
	{
		return FALSE;
	}

	return TRUE;
}


BOOL CMyTaskSchedule::NewTask(const char* lpszTaskName, const char* lpszProgramPath, const char* lpszParameters, const char* lpszAuthor)
{
	if (NULL == m_lpRootFolder)
	{
		return FALSE;
	}
	// 如果存在相同的计划任务，则删除
	Delete(lpszTaskName);
	// 创建任务定义对象来创建任务
	ITaskDefinition* pTaskDefinition = NULL;
	HRESULT hr = m_lpITS->NewTask(0, &pTaskDefinition);
	if (FAILED(hr))
	{
		ShowError("ITaskService::NewTask", hr);
		return FALSE;
	}

	/* 设置注册信息 */
	IRegistrationInfo* pRegInfo = NULL;
	CComVariant variantAuthor(NULL);
	variantAuthor = lpszAuthor;
	hr = pTaskDefinition->get_RegistrationInfo(&pRegInfo);
	if (FAILED(hr))
	{
		ShowError("pTaskDefinition::get_RegistrationInfo", hr);
		return FALSE;
	}
	// 设置作者信息
	hr = pRegInfo->put_Author(variantAuthor.bstrVal);
	pRegInfo->Release();

	/* 设置登录类型和运行权限 */
	IPrincipal* pPrincipal = NULL;
	hr = pTaskDefinition->get_Principal(&pPrincipal);
	if (FAILED(hr))
	{
		ShowError("pTaskDefinition::get_Principal", hr);
		return FALSE;
	}
	// 设置登录类型
	hr = pPrincipal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);
	// 设置运行权限
	// 最高权限
	hr = pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
	pPrincipal->Release();

	/* 设置其他信息 */
	ITaskSettings* pSettting = NULL;
	hr = pTaskDefinition->get_Settings(&pSettting);
	if (FAILED(hr))
	{
		ShowError("pTaskDefinition::get_Settings", hr);
		return FALSE;
	}
	// 设置其他信息
	hr = pSettting->put_StopIfGoingOnBatteries(VARIANT_FALSE);
	hr = pSettting->put_DisallowStartIfOnBatteries(VARIANT_FALSE);
	hr = pSettting->put_AllowDemandStart(VARIANT_TRUE);
	hr = pSettting->put_StartWhenAvailable(VARIANT_FALSE);
	hr = pSettting->put_MultipleInstances(TASK_INSTANCES_PARALLEL);
	pSettting->Release();

	/* 创建执行动作 */
	IActionCollection* pActionCollect = NULL;
	hr = pTaskDefinition->get_Actions(&pActionCollect);
	if (FAILED(hr))
	{
		ShowError("pTaskDefinition::get_Actions", hr);
		return FALSE;
	}
	IAction* pAction = NULL;
	// 创建执行操作
	hr = pActionCollect->Create(TASK_ACTION_EXEC, &pAction);
	pActionCollect->Release();

	/* 设置执行程序路径和参数 */
	CComVariant variantProgramPath(NULL);
	CComVariant variantParameters(NULL);
	IExecAction* pExecAction = NULL;
	hr = pAction->QueryInterface(IID_IExecAction, (PVOID*)(&pExecAction));
	if (FAILED(hr))
	{
		pAction->Release();
		ShowError("IAction::QueryInterface", hr);
		return FALSE;
	}
	pAction->Release();
	// 设置程序路径和参数
	variantProgramPath = lpszProgramPath;
	variantParameters = lpszParameters;
	pExecAction->put_Path(variantProgramPath.bstrVal);
	pExecAction->put_Arguments(variantParameters.bstrVal);
	pExecAction->Release();

	/* 创建触发器，实现用户登陆自启动 */
	ITriggerCollection* pTriggers = NULL;
	hr = pTaskDefinition->get_Triggers(&pTriggers);
	if (FAILED(hr))
	{
		ShowError("pTaskDefinition::get_Triggers", hr);
		return FALSE;
	}
	// 创建触发器
	ITrigger* pTrigger = NULL;
	hr = pTriggers->Create(TASK_TRIGGER_LOGON, &pTrigger);
	if (FAILED(hr))
	{
		ShowError("ITriggerCollection::Create", hr);
		return FALSE;
	}

	/* 注册任务计划  */
	IRegisteredTask* pRegisteredTask = NULL;
	CComVariant variantTaskName(NULL);
	variantTaskName = lpszTaskName;
	hr = m_lpRootFolder->RegisterTaskDefinition(variantTaskName.bstrVal,
		pTaskDefinition,
		TASK_CREATE_OR_UPDATE,
		_variant_t(),
		_variant_t(),
		TASK_LOGON_INTERACTIVE_TOKEN,
		_variant_t(""),
		&pRegisteredTask);
	if (FAILED(hr))
	{
		pTaskDefinition->Release();
		ShowError("ITaskFolder::RegisterTaskDefinition", hr);
		return FALSE;
	}
	pTaskDefinition->Release();
	pRegisteredTask->Release();

	return TRUE;
}


BOOL CMyTaskSchedule::IsExist(const char* lpszTaskName)
{
	if (NULL == m_lpRootFolder)
	{
		return FALSE;
	}
	HRESULT hr = S_OK;
	CComVariant variantTaskName(NULL);
	CComVariant variantEnable(NULL);
	variantTaskName = lpszTaskName;                     // 任务计划名称
	IRegisteredTask* pRegisteredTask = NULL;
	// 获取任务计划
	hr = m_lpRootFolder->GetTask(variantTaskName.bstrVal, &pRegisteredTask);
	if (FAILED(hr) || (NULL == pRegisteredTask))
	{
		return FALSE;
	}
	pRegisteredTask->Release();

	return TRUE;
}


BOOL CMyTaskSchedule::IsTaskValid(const char* lpszTaskName)
{
	if (NULL == m_lpRootFolder)
	{
		return FALSE;
	}
	HRESULT hr = S_OK;
	CComVariant variantTaskName(NULL);
	CComVariant variantEnable(NULL);
	variantTaskName = lpszTaskName;                     // 任务计划名称
	IRegisteredTask* pRegisteredTask = NULL;
	// 获取任务计划
	hr = m_lpRootFolder->GetTask(variantTaskName.bstrVal, &pRegisteredTask);
	if (FAILED(hr) || (NULL == pRegisteredTask))
	{
		return FALSE;
	}
	// 获取任务状态
	TASK_STATE taskState;
	hr = pRegisteredTask->get_State(&taskState);
	if (FAILED(hr))
	{
		pRegisteredTask->Release();
		return FALSE;
	}
	pRegisteredTask->Release();
	// 无效
	if (TASK_STATE_DISABLED == taskState)
	{
		return FALSE;
	}

	return TRUE;
}


BOOL CMyTaskSchedule::Run(const char* lpszTaskName, const char* lpszParam)
{
	if (NULL == m_lpRootFolder)
	{
		return FALSE;
	}
	HRESULT hr = S_OK;
	CComVariant variantTaskName(NULL);
	CComVariant variantParameters(NULL);
	variantTaskName = lpszTaskName;
	variantParameters = lpszParam;

	// 获取任务计划
	IRegisteredTask* pRegisteredTask = NULL;
	hr = m_lpRootFolder->GetTask(variantTaskName.bstrVal, &pRegisteredTask);
	if (FAILED(hr) || (NULL == pRegisteredTask))
	{
		return FALSE;
	}
	// 运行
	hr = pRegisteredTask->Run(variantParameters, NULL);
	if (FAILED(hr))
	{
		pRegisteredTask->Release();
		return FALSE;
	}
	pRegisteredTask->Release();

	return TRUE;
}


BOOL CMyTaskSchedule::IsEnable(const char* lpszTaskName)
{
	if (NULL == m_lpRootFolder)
	{
		return FALSE;
	}
	HRESULT hr = S_OK;
	CComVariant variantTaskName(NULL);
	CComVariant variantEnable(NULL);
	variantTaskName = lpszTaskName;                     // 任务计划名称
	IRegisteredTask* pRegisteredTask = NULL;
	// 获取任务计划
	hr = m_lpRootFolder->GetTask(variantTaskName.bstrVal, &pRegisteredTask);
	if (FAILED(hr) || (NULL == pRegisteredTask))
	{
		return FALSE;
	}
	// 获取是否已经启动
	pRegisteredTask->get_Enabled(&variantEnable.boolVal);
	pRegisteredTask->Release();
	if (ATL_VARIANT_FALSE == variantEnable.boolVal)
	{
		return FALSE;
	}

	return TRUE;
}


BOOL CMyTaskSchedule::SetEnable(const char* lpszTaskName, BOOL bEnable)
{
	if (NULL == m_lpRootFolder)
	{
		return FALSE;
	}
	HRESULT hr = S_OK;
	CComVariant variantTaskName(NULL);
	CComVariant variantEnable(NULL);
	variantTaskName = lpszTaskName;                     // 任务计划名称
	variantEnable = bEnable;                            // 是否启动
	IRegisteredTask* pRegisteredTask = NULL;
	// 获取任务计划
	hr = m_lpRootFolder->GetTask(variantTaskName.bstrVal, &pRegisteredTask);
	if (FAILED(hr) || (NULL == pRegisteredTask))
	{
		return FALSE;
	}
	// 设置是否启动
	pRegisteredTask->put_Enabled(variantEnable.boolVal);
	pRegisteredTask->Release();

	return TRUE;
}

