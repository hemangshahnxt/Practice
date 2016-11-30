// PatientDialog.cpp: implementation of the CPatientDialog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "practice.h"
#include "PatientDialog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPatientDialog::CPatientDialog(int IDD, CWnd* pParent)
	: CNxDialog(IDD, pParent)
{
	m_lastColor = 0;
}

CPatientDialog::~CPatientDialog()
{

}

void CPatientDialog::SetColor(OLE_COLOR nNewColor)
{
	m_lastColor = nNewColor;
	// (a.walling 2008-05-30 16:46) - PLID 30099
#ifndef NXDIALOG_NOCLIPCHILDEN
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
#else
	Invalidate(FALSE);//need to redraw because the color changed, which overwrote the labels
#endif
}

DWORD CPatientDialog::GetCurrentColor()
{
	return m_lastColor;
}