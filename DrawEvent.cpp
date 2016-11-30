// DrawEvent.cpp: implementation of the CDrawEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "CntrItem.h"
#include "practice.h"
#include "DrawEvent.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CDrawEvent, CObject, VERSIONABLE_SCHEMA | 1);

CDrawEvent::CDrawEvent()
{
	m_nForeColor = RGB(0,0,0);
	m_nPenSize = 2;
	Clear();
}

CDrawEvent::~CDrawEvent()
{
	Clear();
}

void CDrawEvent::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		// Store the color
		ar << m_nForeColor;

		// Store the brush
		ar << m_nPenSize;
		
		// Store the array size
		long nCount = m_aryDrawToPoints.GetSize();
		ar << nCount;

		// Store the array
		for (long i=0; i<nCount; i++) {
			ar << (*((CPoint *)m_aryDrawToPoints.GetAt(i)));
		}
	} else {
		// Empty the array
		Clear();

		// Load the color
		ar >> m_nForeColor;

		// Store the brush
		ar >> m_nPenSize;
		
		// Load the array size
		long nCount;
		ar >> nCount;
		
		// Load the array
		CPoint *pPoint;
		for (long i=0; i<nCount; i++) {
			pPoint = new CPoint;
			ar >> (*pPoint);
			m_aryDrawToPoints.Add(pPoint);
		}
	}
}

void CDrawEvent::Draw(CDC *pDC, int offset_x /* =0 */, int offset_y /* =0 */)
{
	// Prepare drawing tools
	
	CPen pen(PS_SOLID, m_nPenSize, m_nForeColor);
	CPen *pOldPen = pDC->SelectObject(&pen);

	int multiplier = 1;

	if(pDC->GetMapMode()!=MM_TEXT)
		multiplier = -1;
	
	// Draw
	CPoint *pPoint = NULL;
	long nCount = m_aryDrawToPoints.GetSize();
	if (nCount > 0) {
		pPoint = (CPoint *)m_aryDrawToPoints.GetAt(0);
		if (pPoint) {
			pDC->MoveTo(pPoint->x + offset_x, multiplier * (pPoint->y + offset_y));
			for (long i=1; i<nCount; i++) {
				pPoint = (CPoint *)m_aryDrawToPoints.GetAt(i);
				if (pPoint) {
					pDC->LineTo(pPoint->x + offset_x, multiplier * (pPoint->y + offset_y));
				}
			}
		}
	}

	// Release drawing tools
	pDC->SelectObject(pOldPen);

	//pDC->SetMapMode(mapMode);
}

void CDrawEvent::Clear()
{
	long nCount = m_aryDrawToPoints.GetSize();
	CPoint *pPoint;
	for (long i=nCount-1; i>=0; i--) {
		pPoint = (CPoint *)m_aryDrawToPoints.GetAt(i);
		delete pPoint;
		m_aryDrawToPoints.RemoveAt(i);
	}
}
