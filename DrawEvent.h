// DrawEvent.h: interface for the CDrawEvent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRAWEVENT_H__648FE830_625A_11D4_958C_00C04F4C8415__INCLUDED_)
#define AFX_DRAWEVENT_H__648FE830_625A_11D4_958C_00C04F4C8415__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDrawEvent : public CObject
{
protected:
	DECLARE_SERIAL(CDrawEvent);

public:
	virtual void Serialize(CArchive& ar);

public:
	void Clear();
	void Draw(CDC *pDC, int offset_x = 0, int offset_y = 0);
	CPtrArray m_aryDrawToPoints;
	COLORREF m_nForeColor;
	CDrawEvent();
	virtual ~CDrawEvent();
	int m_nPenSize;
};

#endif // !defined(AFX_DRAWEVENT_H__648FE830_625A_11D4_958C_00C04F4C8415__INCLUDED_)
