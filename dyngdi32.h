// uxtheme.h: interface for the uxtheme class.
//
//This class dynamically loads and uses the GDI32.dll if it exists
//This DLL exists only on Windows 2000 and up, and is not redistributable
//We could simply link to the dll through the provided .lib file, but that would require the dll for practice to start
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DYNGDI32_H__1E2F51B6_CDCA_4219_BCFA_78953C59F15F__INCLUDED_)
#define AFX_DYNGDI32_H__1E2F51B6_CDCA_4219_BCFA_78953C59F15F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef BOOL (WINAPI * AlphaBlendType) (HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);

// Basic wrapper for the dll
class GDI32Dll
{
public:
	GDI32Dll();
	virtual ~GDI32Dll();
	BOOL AlphaBlend( IN HDC, IN int, IN int, IN int, IN int, IN HDC, IN int, IN int, IN int, IN int, IN BLENDFUNCTION);

protected:
	HMODULE m_dll;
	AlphaBlendType m_pAlphaBlend;
};

// Modular wrapper for the HTHEME reference (this is the class you'd use from the outside)
class GDI32
{
public:
	GDI32();
	virtual ~GDI32();
	BOOL AlphaBlend( IN HDC, IN int, IN int, IN int, IN int, IN HDC, IN int, IN int, IN int, IN int, IN BLENDFUNCTION);
	

protected:
	static GDI32Dll m_dllGDI32;
};

#endif // !defined(AFX_DYNGDI32_H__1E2F51B6_CDCA_4219_BCFA_78953C59F15F__INCLUDED_)