#include "BmpSave_CImage.h"


BOOL SaveBmp(HBITMAP hBmp)
{
	CImage image;

	// 附加位图句柄
	image.Attach(hBmp);

	// 保存成jpg格式图片
	image.Save("mybmp1.jpg");

	return TRUE;
}


BOOL ConverPicture()
{
	CImage image;

	// 加载图片
	image.Load("1.jpg");

	// 保存图片
	// bmp格式
	image.Save("ConverPicture.bmp");
	// png格式
	image.Save("ConverPicture.png");
	// jpg格式
	image.Save("ConverPicture.jpg");
	// gif格式
	image.Save("ConverPicture.gif");


	return TRUE;
}