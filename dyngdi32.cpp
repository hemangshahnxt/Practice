// dyngdi32.cpp: implementation of the gdi32 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "dyngdi32.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GDI32Dll::GDI32Dll()
{
	m_dll = ::LoadLibrary("GDI32.Dll");
	if (m_dll)
	{
		m_pAlphaBlend = (AlphaBlendType)GetProcAddress(m_dll, "GdiAlphaBlend");
	}
	else 
	{	
		m_pAlphaBlend = NULL;
	}
}

GDI32Dll::~GDI32Dll()
{
	if (m_dll)
		::FreeLibrary(m_dll);
}

BOOL GDI32Dll::AlphaBlend( IN HDC hdcDest, IN int nXOriginDest, IN int nYOriginDest, IN int nWidthDest, IN int nHeightDest, 
						  IN HDC hdcSrc, IN int nXOriginSrc, IN int nYOriginSrc, IN int nWidthSrc, IN int nHeightSrc, 
						  IN BLENDFUNCTION blendFunction)
{
	if (m_pAlphaBlend)
		return m_pAlphaBlend(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hdcSrc, nXOriginSrc, nYOriginSrc,
			nWidthSrc, nHeightSrc, blendFunction);
	else return FALSE;
}



GDI32Dll GDI32::m_dllGDI32;


GDI32::GDI32()
{
}

GDI32::~GDI32()
{
}

BOOL GDI32::AlphaBlend( IN HDC hdcDest, IN int nXOriginDest, IN int nYOriginDest, IN int nWidthDest, IN int nHeightDest, 
						  IN HDC hdcSrc, IN int nXOriginSrc, IN int nYOriginSrc, IN int nWidthSrc, IN int nHeightSrc, 
						  IN BLENDFUNCTION blendFunction)
{
	return m_dllGDI32.AlphaBlend(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hdcSrc, nXOriginSrc, nYOriginSrc,
			nWidthSrc, nHeightSrc, blendFunction);
}
