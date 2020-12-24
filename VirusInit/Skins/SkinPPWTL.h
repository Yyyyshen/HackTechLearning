#ifndef _SKINPPWTL_H_
#define _SKINPPWTL_H_

#ifdef _SKINPP_STATIC
	#define SKINPPWTL_API
#else
	#ifdef SKINPPWTL_EXPORTS
		#define SKINPPWTL_API __declspec(dllexport)
	#else
		#define SKINPPWTL_API __declspec(dllimport)
	#endif
#endif

#define WM_TOOLBARPAINTPRE (WM_USER + 802)
#define WM_TOOLBARPAINTEND (WM_USER + 803)

#define SM_LBUTTONUP (WM_USER + 804)

//按钮状态
enum BUTTONSTATE 
{
	NORMAL	= 0,
	PRESS	= 1,
	DISABLE	= 2,
	HOT		= 3,
	FOCUS   = 4,
	LAST	= 5
};

//绘制类型
enum DRAWTYPE
{
	BUTTON = 0,
	SCROLLARROWUP = 1,
	SCROLLARROWDOWN = 2,
	SPLITTERBARHORZ = 3,
	SPLITTERBARVERT = 4,
	SPLITTERBARBORDER = 5,
	LISTHEAD = 6
};

//获得皮肤资源的类型
enum SKINOBJTYPE
{
	DIALOGTYPE = 0,
	BUTTONTYPE = 1,
	CHECKBOXTYPE = 2,
	RADIOBOXTYPE = 3,
	STATICTYPE = 4,
	TRACKBARTYPE = 5,
};

struct ListBoxItem
{
	HIMAGELIST	hImageList;
	int			nImageIndex;

	ListBoxItem()
	{
		hImageList = NULL;
		nImageIndex = -1;
	}
};

#define REST_BITMAP      0x0001 //.bmp
#define REST_ICON        0x0002 //.ico
#define REST_CURSOR      0x0003 //.cur
#define REST_ANIMATE	 0x0004 //.ani

typedef struct _ResourceInfo
{
	HGDIOBJ hGdiObj;//[OUT]
	DWORD   dwType; //[OUT]
	int		nWidth; //[OUT]
	int		nHeight;//[OUT]

	TCHAR	szResImageName[_MAX_FNAME];//[IN]
	BOOL	bHorzSplit;//[IN]
	int		nLength;//[IN]
	int		nCount; //[IN]
	int		nIndex; //[IN]
	
	_ResourceInfo()
	{
		hGdiObj = NULL;
		dwType = REST_BITMAP;
		nWidth = 0;
		nHeight = 0;

		//wcscpy_s(szResImageName,_T(""));
		bHorzSplit = TRUE;
		nLength = -1;
		nCount = -1;
		nIndex = -1;
	}
	
}ResInfo,* PRESINFO;

//////////////////////////////////////////////////////////////////////////

//加载皮肤
//SkinFile		:皮肤路径,注意可以是*.ssk,也可以是皮肤目录中的INI文件.
//bFromIni		:该参数指定皮肤文件是从*.ssk读取,还是从INI文件读取.
SKINPPWTL_API BOOL  skinppLoadSkin(TCHAR* szSkinFile,BOOL bFromIni = FALSE);

SKINPPWTL_API BOOL skinppLoadSkinFromRes(HINSTANCE hInstance,LPCTSTR szResourceName,
										 LPCTSTR szResourceType,TCHAR* szSkinFileName);

//移除皮肤
SKINPPWTL_API BOOL  skinppRemoveSkin();

//退出界面库，做清理工作。
SKINPPWTL_API BOOL  skinppExitSkin();

//设置ListBox控件的自画信息
//hWnd			: ListBox控件的句柄
//nIndex		: Item项的索引
//pListBoxItem	: Item项自画的结构信息
SKINPPWTL_API void skinppSetListBoxItemDrawInfo(HWND hWnd,int nIndex,struct ListBoxItem* pListBoxItem);

//获得换肤后的系统颜色
//nColorIndex	: 要获取的颜色类型
SKINPPWTL_API COLORREF skinppGetSkinSysColor(int nColorIndex);

//获得Windows系统默认的颜色
//nColorIndex	: 要获取的颜色类型
SKINPPWTL_API COLORREF skinppGetDefaultSysColor(int nColorIndex);

//hWnd			: 对话框窗口的句柄
//nResID		: 对话框资源ID
SKINPPWTL_API BOOL skinppSetWindowResID(HWND hWnd,int nResID);//[多语言]

SKINPPWTL_API BOOL skinppSetFreeDlgID(HWND hWnd,int nResID);

SKINPPWTL_API BOOL skinppSetSkinResID(HWND hWnd,int nResID);

//设置ListHeader窗口的排序信息
//hWnd			: ListHeader的窗口句柄
//nSortColumn	: 要对ListHeader排序的列的索引
//bSortAscending: 是否为升序
SKINPPWTL_API void skinppSetListHeaderSortInfo(HWND hWnd,int nSortColumn,BOOL bSortAscending = TRUE);

//在给定的HDC上,指定相应的绘制类型和状态,在相应的矩形区域中进行绘制.
//hdc			:目标DC
//rect			:绘制区域
//eDrawType		:绘制类型，目前支持SPLITTERBARHORZ,SPLITTERBARVERT,SPLITTERBARBORDER
//nState		:选择绘制状态
SKINPPWTL_API void  skinppDrawSkinObject(HDC hdc,RECT rect,DRAWTYPE eDrawType,int nState);

//通过资源ID,获得相应类型的皮肤资源位图句柄
//nSkinObjType	: 皮肤类型,目前支持	DIALOGTYPE,BUTTONTYPE,CHECKBOXTYPE,RADIOBOXTYPE
//nResID		: 资源ID
//nState		: 状态,对BUTTONTYPE,CHECKBOXTYPE,RADIOBOXTYPE有效
SKINPPWTL_API HBITMAP skinppGetResFromID(SKINOBJTYPE nSkinObjType,int nResID,int nState =0 );

//设置是否自己画对话框背景,该方法用在需要自己对背景进行处理的情况下.
//hWnd			: 对话框的句柄
//bErase		: TRUE 为自己画背景,FALSE 为Skin++画,如果没有调用该方法,Skin++将画对话框背景.
SKINPPWTL_API void skinppSetDialogEraseBkgnd(HWND hWnd,BOOL bErase);

//设置对话框背景是否剪切子控件区域。
//hWnd			: 对话框句柄
//bNoClip		: TRUE为不需要剪切,FALSE为需要剪切区域
//bAllChild		: TRUE为该窗体的所有子对话框都剪切.
SKINPPWTL_API void skinppSetDialogBkClipRgn(HWND hWnd,BOOL bClip,BOOL bAllChild = TRUE);

//通过皮肤资源名称获得皮肤资源中位图
//szName		: 皮肤资源名称
//HBITMAP		: 返回资源中的位图
SKINPPWTL_API HBITMAP skinppGetBitmapRes(LPCTSTR szName);

//通过资源名称取资源的内存指针
//szName		: 资源名称
//nSize			: 资源大小
//pByte			: 返回值，成功返回非NULL，失败返回NULL
SKINPPWTL_API BYTE*  skinppGetSkinResource(LPCTSTR szName,int& nSize);

//通过皮肤资源的名称获得位图不被拉伸的区域值
//szName		: 皮肤资源名称
//nTopHeight	: 返回不被拉伸的顶高
//nBottomHeight : 返回不被拉伸的底高
//nLeftWidth	: 返回不被拉伸的左宽
//nRightWidth	: 返回不被拉伸的右宽
SKINPPWTL_API BOOL skinppGetBitmapResRect(LPCTSTR szName,int& nTopHeight,int& nBottomHeight,
												   int& nLeftWidth,int& nRightWidth);

//设置窗口自画是否自己来处理,该方法用于自画部分需要自己处理的情况下
//hWnd			: 要自画的窗口句柄
//bCustomDraw	: TRUE为自己处理自画,FALSE为交给Skin++处理自画
SKINPPWTL_API void skinppSetCustomDraw(HWND hWnd,BOOL bCustomDraw);

//设置菜单的皮肤标识
//hWnd			: 拥有菜单的窗口句柄
//nSkinObjectID	: 菜单皮肤的标识
SKINPPWTL_API void skinppSetMenuSkinObjectID(HWND hWnd,int nSkinObjectID);

//设置是否对自画菜单进行换肤
//bSkin			: TRUE为换肤
SKINPPWTL_API void skinppSetSkinOwnerMenu(BOOL bSkin);

//对菜单进行换肤控制
//hMenu   : 想换肤的菜单句柄
//bNoSkin : 是否换肤,TRUE为不换肤,FALSE为换肤
SKINPPWTL_API void  skinppSetDrawMenu(HMENU hMenu,BOOL bNoSkin);
	
//对指定的窗口去掉皮肤,并且保证不会再被换肤,即使使用SetSkinHwnd也不会换肤.
//hWnd			: 指定的窗口句柄
//bChildNoSkin	: 是否对该窗口中的子窗口去掉皮肤
SKINPPWTL_API void skinppSetNoSkinHwnd(HWND hWnd,BOOL bChildNoSkin = TRUE);

//对指定的窗口进行换肤
//hWnd			: 指定的窗口句柄
//szClassName	: 要子类化的Skin类型 WC_DIALOGBOX/WC_CONTROLBAR等
SKINPPWTL_API void skinppSetSkinHwnd(HWND hWnd,LPCTSTR szClassName = NULL);

//对指定的窗口临时去掉皮肤,可以通过SetSkinHwnd进行再次换肤
SKINPPWTL_API void skinppRemoveSkinHwnd(HWND hWnd);

//是对SetNoSkinHwnd的进一步处理,可以解决使用SetNoSkinHwnd引起的Debug版的断言错
#define SETNOSKINHWND(x) {\
    HWND w=(x).UnsubclassWindow();\
	skinppSetNoSkinHwnd(w);\
	(x).SubclassWindow(w);\
} 

//是对RemoveSkinHwnd的进一步处理,可以解决使用RemoveSkinHwnd引起的Debug版的断言错
#define REMOVESKINHWND(x){\
    HWND w=(x).UnsubclassWindow();\
	skinppRemoveSkinHwnd(w);\
	(x).SubclassWindow(w);\
}

SKINPPWTL_API HGDIOBJ skinppGetResFromID(PRESINFO pResInfo);


#endif //_SKINPPWTL_H_

