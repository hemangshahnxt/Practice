// Color.h: interface for the CColor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COLOR_H__27E3803B_849B_4743_A6A2_2C8F93ADB7B3__INCLUDED_)
#define AFX_COLOR_H__27E3803B_849B_4743_A6A2_2C8F93ADB7B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CColor  
{
public:
	CColor(DWORD dwColor = 0);
	CColor(BYTE R, BYTE G, BYTE B);

	void SetR(BYTE R);
	void SetG(BYTE G);
	void SetB(BYTE B);

	void SetH(BYTE H);
	void SetS(BYTE S);
	void SetV(BYTE V);

	BYTE R();
	BYTE G();
	BYTE B();

	BYTE H();
	BYTE S();
	BYTE V();

	operator DWORD()
	{	return m_dwColor;
	}

	operator long()
	{	return m_dwColor;
	}

	operator _variant_t()
	{	return _variant_t((long)m_dwColor);
	}

	// (a.walling 2007-11-05 17:40) - PLID 27974 - VS2008 - Needs a return value
	void operator = (DWORD dwColor)
	{	m_dwColor = dwColor;
	}

protected:
	DWORD m_dwColor;
};

#endif // !defined(AFX_COLOR_H__27E3803B_849B_4743_A6A2_2C8F93ADB7B3__INCLUDED_)