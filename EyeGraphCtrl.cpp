#include "stdafx.h"
#include "EyeGraphCtrl.h"
#include "Graphics.h"
#include "GlobalDataUtils.h"
#include <math.h>

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// EyeGraphCtrl

CEyeGraphCtrl::CEyeGraphCtrl()
{
	//This has to be here for testing, but it probably shouldn't be.
	m_Type = SphereScatter;
	m_pPointArray = new CArray<DataPoint, DataPoint&>;
}

CEyeGraphCtrl::~CEyeGraphCtrl()
{
}


BEGIN_MESSAGE_MAP(CEyeGraphCtrl, CStatic)
	//{{AFX_MSG_MAP(CEyeGraphCtrl)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CEyeGraphCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CRect rThis;
	GetClientRect(&rThis);

	//Let's first of all decide that this thing is going to be 1000 by 1000.
	dc.SetMapMode(MM_ANISOTROPIC);
	dc.SetWindowExt (1000, 1000);
	dc.SetViewportExt(rThis.Width(), rThis.Height());

	//Fill in the background
	CBrush brWhite;
	brWhite.CreateSolidBrush(0xFFFFFF);
	CRect rFull(0,0,1000,1000);
	dc.FillRect(rFull, &brWhite);

	DrawCaptions(&dc);
	DrawPoints(&dc);
	DrawAxes(&dc);
	
	// Do not call CStatic::OnPaint() for painting messages
}


void CEyeGraphCtrl::DrawAxes(CPaintDC *pDC)
{
	//The actual axes are always the same.

	CPen pnBlack;
	pnBlack.CreateStockObject(BLACK_PEN);
	CPen* pOldPen = pDC->SelectObject(&pnBlack);
	pDC->MoveTo(170,50);
	pDC->LineTo(170,770);
	pDC->LineTo(890,770);

	//Now, the hash marks, which will be different each time.
	int nX, nY, i;
	CRect rCaption;
	CString strCaption;
	CPen pnRed (PS_SOLID, 2, RGB(255,0,0));
	CPen pnGreen (PS_SOLID, 2, RGB(0,255,0));
	switch(m_Type) {
	case SphereScatter:
	case DefocusScatter:
		//OK, this is always 0-16, so...
		//x-axis hashmarks.
		pDC->MoveTo(170, 770);
		nX = 170;
		for(i = 0; i < 16; i++) {
			nX += 45;
			pDC->MoveTo(nX, 770);
			pDC->LineTo(nX, 760);
			rCaption.SetRect(nX-22,780,nX+22,830);
			strCaption.Format("%li", i+1);
			pDC->DrawText(strCaption, rCaption, DT_CENTER);
		}

		//y-axis hashmarks
		pDC->MoveTo(170, 770);
		nY = 770;
		for(i = 0; i < 16; i++) {
			nY -= 45;
			pDC->MoveTo(170, nY);
			pDC->LineTo(180, nY);
			rCaption.SetRect(110,nY-22,160,nY+22);
			strCaption.Format("%li", i+1);
			pDC->DrawText(strCaption, rCaption, DT_CENTER);
		}
		
		//Now, let's draw the diagonal lines.
		//Center line
		pDC->SelectObject(&pnGreen);
		pDC->MoveTo(170, 770);
		pDC->LineTo(890, 50);

		pDC->SelectObject(&pnRed);
		pDC->MoveTo(170,725);
		pDC->LineTo(845, 50);

		pDC->MoveTo(215,770);
		pDC->LineTo(890,95);
		break;


	default:
		MessageBox("Invalid Type!");
		break;
	}

	pDC->SelectObject(pOldPen);

		
}


void CEyeGraphCtrl::DrawCaptions(CPaintDC *pDC)
{
	//Based on the type of the graph:
	switch(m_Type) {
	case SphereScatter:
	case DefocusScatter:
		{
		//This will always be the same (for now).
		
		//Main title (this is the only thing different between the two
		CRect rTitle(50,900,950,950);
		CString strCaption;
		if(m_Type == SphereScatter) {
			if(m_strCaption == "") {
				strCaption = "Spherical Equivalent Refraction";
			}
			else {
				strCaption = m_strCaption;
			}
		}
		else {
			if(m_strCaption == "") {
				strCaption = "Defocus Equivalent Refraction";
			}
			else {
				strCaption = m_strCaption;
			}
		}
		pDC->DrawText(strCaption, &rTitle, DT_CENTER);

		//x-axis title
		CRect rTitleX(50,840,950,890);
		pDC->DrawText("Attempted (D)", &rTitleX, DT_CENTER);

		//y-axis title
		CRect rTitleY(50,50,100,950);
		//Uh-oh!  This is rotated!
		LOGFONT lfVert;
		::ZeroMemory(&lfVert, sizeof(lfVert));
		lfVert.lfWeight = FW_BOLD;
		lfVert.lfEscapement = 900;
		lfVert.lfOrientation = 900;
		::lstrcpy(lfVert.lfFaceName, _T("Arial"));
		
		CFont fVert;
		fVert.CreateFont(50, 0, 900, 900, 700, 0, 0, 0, 0, 0, 0, 0, 0, _T("Arial"));
		//fVert.CreatePointFontIndirect(&lfVert);

		CFont *pOldFont = pDC->SelectObject(&fVert);
		/*LOGFONT lfTemp;
		pOldFont->GetLogFont(&lfTemp);
		MsgBox("%li", lfTemp.lfHeight);*/
		
		CRect rThis;
		GetClientRect(rThis);
		pDC->TextOut(50,550,"Achieved (D)");
		pDC->SelectObject(pOldFont);

		break;
		}

	default:
		MessageBox("Invalid Type!");
		break;
	}
}

void CEyeGraphCtrl::DrawPoints(CPaintDC *pDC)
{
	//Based on the type of graph
	switch(m_Type) {
	case SphereScatter:
	case DefocusScatter:
		{
			try {
			//For now, open the query, in reality, we'll want to get an array or something, so as not to 
			//open a recordset in the OnPaint function
				for(int i = 0; i < m_pPointArray->GetSize(); i++) {
					int nX = (int)(170.0 + (fabs(m_pPointArray->GetAt(i).dLogicalX) * 45.0));
					int nY = (int)(770.0 - (fabs(m_pPointArray->GetAt(i).dLogicalY) * 45.0));
					//Make sure they're in range.
					if(nX >= 170 && nX <= 950 && nY >= 50 && nY <= 770) {
						CBrush brBlack;
						brBlack.CreateStockObject(BLACK_BRUSH);
						CRect rDot(nX-5,nY-5,nX+5,nY+5);
						pDC->FillRect(rDot, &brBlack);
					}
				}
			}NxCatchAll("Error in CEyeGraphCtrl::DrawPoints()");
		}
		break;
	default:
		MessageBox("Invalid Type!");
		break;
	}
}


BOOL CEyeGraphCtrl::PreTranslateMessage(MSG* pMsg) 
{
	
	return CStatic::PreTranslateMessage(pMsg);
}
