// Graphics.h: This file was create to hold all the custom graphics routines nextech uses
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRAPHICS_H__3FBA9C01_EBEC_40A4_8B0B_A89D144AA222__INCLUDED_)
#define AFX_GRAPHICS_H__3FBA9C01_EBEC_40A4_8B0B_A89D144AA222__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

HBITMAP DDBToDIB(HDC hdc, HBITMAP ddb);
HBITMAP CopyBitmap(HBITMAP bitmap);
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
);

#endif // !defined(AFX_GRAPHICS_H__3FBA9C01_EBEC_40A4_8B0B_A89D144AA222__INCLUDED_)
