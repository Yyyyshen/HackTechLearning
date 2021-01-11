#include "ScreenCapture.h"

BOOL ScreenCapture()
{
	//变量
	HWND hDesktopWnd = NULL;
	HDC hdc = NULL;
	HDC mdc = NULL;
	int dwScreenWidth = 0;
	int dwScreenHeight = 0;
	HBITMAP bmp = NULL;
	HBITMAP holdbmp = NULL;
	//获取桌面窗口句柄
	hDesktopWnd = ::GetDesktopWindow();
	//获取窗口DC
	hdc = ::GetDC(hDesktopWnd);
	//创建兼容DC
	mdc = ::CreateCompatibleDC(hdc);
	//获取屏幕宽高
	dwScreenWidth = ::GetSystemMetrics(SM_CXSCREEN);
	dwScreenHeight = ::GetSystemMetrics(SM_CYSCREEN);
	//创建兼容位图
	bmp = ::CreateCompatibleBitmap(hdc, dwScreenWidth, dwScreenHeight);
	//选中位图
	holdbmp = (HBITMAP)::SelectObject(mdc, bmp);
	//将窗口内容绘制到位图
	::BitBlt(mdc, 0, 0, dwScreenWidth, dwScreenHeight, hdc, 0, 0, SRCCOPY);

	//绘制鼠标
	PaintMouse(mdc);

	//保存为图片
	SaveBmp(bmp);
	//释放资源
	::DeleteObject(hDesktopWnd);
	::DeleteDC(hdc);
	::DeleteDC(mdc);
	::DeleteObject(bmp);
	::DeleteObject(holdbmp);
	return TRUE;
}

BOOL PaintMouse(HDC hdc)
{
	HDC bufdc = NULL;
	CURSORINFO cursorInfo = { 0 };
	ICONINFO iconInfo = { 0 };
	HBITMAP bmpOldMask = NULL;

	bufdc = ::CreateCompatibleDC(hdc);
	::RtlZeroMemory(&iconInfo, sizeof(iconInfo));
	cursorInfo.cbSize = sizeof(cursorInfo);
	// 获取光标信息
	::GetCursorInfo(&cursorInfo);
	// 获取光标图标信息
	::GetIconInfo(cursorInfo.hCursor, &iconInfo);
	// 绘制 白底黑鼠标(AND)
	bmpOldMask = (HBITMAP)::SelectObject(bufdc, iconInfo.hbmMask);
	::BitBlt(hdc, cursorInfo.ptScreenPos.x, cursorInfo.ptScreenPos.y, 20, 20,
		bufdc, 0, 0, SRCAND);
	// 绘制 黑底彩色鼠标(OR)
	::SelectObject(bufdc, iconInfo.hbmColor);
	::BitBlt(hdc, cursorInfo.ptScreenPos.x, cursorInfo.ptScreenPos.y, 20, 20,
		bufdc, 0, 0, SRCPAINT);

	// 释放资源
	::SelectObject(bufdc, bmpOldMask);
	::DeleteObject(iconInfo.hbmColor);
	::DeleteObject(iconInfo.hbmMask);
	::DeleteDC(bufdc);
	return TRUE;
}
