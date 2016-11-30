// Graphics.cpp: implementation of custom Graphics functions.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "practice.h"
#include "Graphics.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern CPracticeApp theApp;

//#define SHOW_DIB

HBITMAP DDBToDIB(HDC hdc, HBITMAP ddb)
{
	return ddb;//this doesn't work

	HBITMAP dib;
	BITMAPINFO bitmapinfo;
	BYTE *p = NULL;
	int result;
	long w, h;

	BITMAP tmpBmp;
	GetObject(ddb, sizeof(tmpBmp), &tmpBmp);
	h = tmpBmp.bmHeight;
	w = tmpBmp.bmWidth;

	SelectPalette(hdc, theApp.m_palette, FALSE);

	bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapinfo.bmiHeader.biHeight = tmpBmp.bmHeight;
	bitmapinfo.bmiHeader.biWidth = tmpBmp.bmWidth;
	bitmapinfo.bmiHeader.biPlanes = 1;
	bitmapinfo.bmiHeader.biBitCount = 24;
	bitmapinfo.bmiHeader.biCompression = 0;

	result = GetDIBits(hdc, ddb, 0, tmpBmp.bmHeight, p, &bitmapinfo, DIB_RGB_COLORS);
	p = new BYTE[bitmapinfo.bmiHeader.biSizeImage];
	result = GetDIBits(hdc, ddb, 0, tmpBmp.bmHeight, p, &bitmapinfo, DIB_RGB_COLORS);

	dib = CreateDIBitmap(hdc, &bitmapinfo.bmiHeader, CBM_INIT, p, &bitmapinfo, DIB_RGB_COLORS);

#ifdef SHOW_DIB
	extern CPracticeApp theApp;

	SelectPalette (hdc, (HPALETTE)theApp.m_palette.GetSafeHandle(), FALSE);
	RealizePalette(hdc);
	result = StretchDIBits(hdc, 
		0, 0, w, h, 
		0, 0, w, h, 
		p, &bitmapinfo, DIB_RGB_COLORS, SRCCOPY);

	while(1);
#endif

	delete p;

	return dib;
}

HBITMAP CopyBitmap(HBITMAP bitmap)
{
	HBITMAP dib;
	BITMAPINFO bitmapinfo;
	BYTE *p = NULL;
	int result;
	long w, h;
	HDC hdc = ::GetDC(NULL);

	BITMAP tmpBmp;
	GetObject(bitmap, sizeof(tmpBmp), &tmpBmp);
	h = tmpBmp.bmHeight;
	w = tmpBmp.bmWidth;

	bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapinfo.bmiHeader.biHeight = tmpBmp.bmHeight;
	bitmapinfo.bmiHeader.biWidth = tmpBmp.bmWidth;
	bitmapinfo.bmiHeader.biPlanes = 1;
	bitmapinfo.bmiHeader.biBitCount = 24;
	bitmapinfo.bmiHeader.biCompression = 0;

	result = GetDIBits(hdc, bitmap, 0, tmpBmp.bmHeight, p, &bitmapinfo, DIB_RGB_COLORS);
	ASSERT(result);
	p = new BYTE[bitmapinfo.bmiHeader.biSizeImage];
	result = GetDIBits(hdc, bitmap, 0, tmpBmp.bmHeight, p, &bitmapinfo, DIB_RGB_COLORS);
	ASSERT(result);

	dib = CreateDIBitmap(hdc, &bitmapinfo.bmiHeader, CBM_INIT, p, &bitmapinfo, DIB_RGB_COLORS);
	ASSERT(dib);
	delete p;

	return dib;
}

int StretchDIBitsSlow(
  HDC hdc,                      // handle to DC
  int XDest,                    // x-coord of destination upper-left corner
  int YDest,                    // y-coord of destination upper-left corner
  int nDestWidth,               // width of destination rectangle
  int nDestHeight,              // height of destination rectangle
  int XSrc,                     // x-coord of source upper-left corner
  int YSrc,                     // y-coord of source upper-left corner
  int nSrcWidth,                // width of source rectangle
  int nSrcHeight,               // height of source rectangle
  CONST VOID *lpBits,           // bitmap bits
  CONST BITMAPINFO *lpBitsInfo, // bitmap data
  UINT iUsage,                  // usage options
  DWORD dwRop                   // raster operation code
)
{
	//My own stretchblt - slow, but all else has failed
	DWORD color, pos;
	long xsrc, ysrc,
		 stride = (((lpBitsInfo->bmiHeader.biWidth * lpBitsInfo->bmiHeader.biBitCount) + 31) & ~31) >> 3,
		 byteCount = lpBitsInfo->bmiHeader.biBitCount / 8;
	BYTE *p = (BYTE *)lpBits;

	double w = (double)nDestWidth,
		   h = (double)nDestHeight,
		   srcw = (double)lpBitsInfo->bmiHeader.biWidth,
		   srch = (double)lpBitsInfo->bmiHeader.biHeight;

	for (int y = YDest; y < YDest + nDestHeight; y++)
		for (int x = XDest; x < XDest + nDestWidth; x++)
		{	
			xsrc = (long)((double)(x - XDest) / w * srcw);
			ysrc = (long)((double)(nDestHeight + YDest - y) / h * srch) - 1;
			pos = ysrc * stride + xsrc * byteCount;

			color = (p[pos] << 16) + (p[pos + 1] << 8) + p[pos + 2];
			SetPixelV(hdc, x, y, color);
		}
	return YDest + nDestHeight;
}
