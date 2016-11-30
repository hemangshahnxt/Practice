#include "StdAfx.h"
#include "StampSearchDlg.h"
#include "StampSearchCtrl.h"

// (a.walling 2012-08-28 08:15) - PLID 52321 - StampSearch - Host dialog and interaction with EMR

IMPLEMENT_DYNAMIC(CStampSearchDlg, CNxDialog)

CStampSearchDlg::CStampSearchDlg(CWnd* pParent)
	: CNxDialog(IDD, pParent, "CStampSearchDlg")
	, m_nClickedStampID(-1)
{
}

CStampSearchDlg::~CStampSearchDlg()
{
}

BEGIN_MESSAGE_MAP(CStampSearchDlg, CNxDialog)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CStampSearchDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		m_pCtrl.reset(new CStampSearchCtrl);

		CRect rcClient;
		GetClientRect(&rcClient);

		SetWindowText("Stamp search");

		m_pCtrl->Create(rcClient, this, -1);

		m_pCtrl->SetFocus();
		
		return FALSE;
	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (a.walling 2012-08-28 08:15) - PLID 52321 - Stamp was chosen, set the internal var and close the dialog
void CStampSearchDlg::OnStampClicked(long nStampID)
{
	try {
		m_nClickedStampID = nStampID;
		OnOK();
	} NxCatchAll(__FUNCTION__);
}

void CStampSearchDlg::OnCancel()
{
	CNxDialog::OnCancel();
}

BOOL CStampSearchDlg::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CStampSearchDlg::OnSize(UINT nType, int cx, int cy)
{
	try {
		CNxDialog::OnSize(nType, cx, cy);

		if (m_pCtrl && m_pCtrl->GetSafeHwnd()) {
			CRect rcClient;
			GetClientRect(&rcClient);
			m_pCtrl->MoveWindow(rcClient);
		}
	} NxCatchAll(__FUNCTION__);
}
