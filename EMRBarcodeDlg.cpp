// EMRBarcodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EmrRc.h"
#include "EMRBarcodeDlg.h"
#include "BarcodeUtils.h"


// CEMRBarcodeDlg dialog

// (j.dinatale 2011-07-26 17:41) - PLID 44702 - Created

IMPLEMENT_DYNAMIC(CEMRBarcodeDlg, CNxDialog)

CEMRBarcodeDlg::CEMRBarcodeDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRBarcodeDlg::IDD, pParent)
{
	m_nEMRID = -1;
	m_nPersonID = -1;
}

CEMRBarcodeDlg::~CEMRBarcodeDlg()
{
	m_nEMRID = -1;
	m_nPersonID = -1;
}

void CEMRBarcodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_BARCODE_EMR, m_nxsBarcode);
	DDX_Control(pDX, IDC_BARCODENUM_EMR, m_nxsNumericCode);
	DDX_Control(pDX, IDC_BARCODEINFOLINK, m_nxsInfoLink);
}


BEGIN_MESSAGE_MAP(CEMRBarcodeDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CEMRBarcodeDlg::OnBnClickedOk)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()


// CEMRBarcodeDlg message handlers
BOOL CEMRBarcodeDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try{
		m_btnOK.AutoSet(NXB_OK);

		// (j.dinatale 2011-09-13 10:16) - PLID 45369 - need a link for information about camera integration
		m_nxsInfoLink.SetType(dtsHyperlink);
		m_nxsInfoLink.SetHzAlign(DT_CENTER);
		m_nxsInfoLink.SetText("Camera Integration Information");

		m_fBarcodeFont.CreatePointFont(250, LPCTSTR("AdvC128c"));
		CStatic *Label = (CStatic *)GetDlgItem(IDC_BARCODE_EMR);
		Label->SetFont(&m_fBarcodeFont);
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CEMRBarcodeDlg::SetInfo(long nPersonID, long nEMRID)
{
	// left exception handling off this one, since we want to pass the exception to the parent if we somehow have a problem
	m_nPersonID = nPersonID;
	m_nEMRID = nEMRID;

	if(GetSafeHwnd() == NULL){
		Create(IDD_EMR_BARCODE, m_pParent);
	}

	BarcodeUtils::Code128Generator BarcodeGen;

	CString strNumericCode;
	strNumericCode.Format("%li-%li", nPersonID, nEMRID);
	CString strBarcode = BarcodeGen.GenerateCodeB(strNumericCode);

	m_nxsBarcode.SetWindowText(strBarcode);
	m_nxsNumericCode.SetWindowText(strNumericCode);
}

void CEMRBarcodeDlg::OnBnClickedOk()
{
	try{
		ShowWindow(SW_HIDE);
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2011-09-13 10:16) - PLID 45369 - need to shellexecute and open the webpage we have set up
LRESULT CEMRBarcodeDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try{
		UINT nIdc = (UINT)wParam;
		if(nIdc == IDC_BARCODEINFOLINK){
			ShellExecute(NULL, NULL, "http://www.nextech.com/photo-barcode-scanning.aspx", NULL, NULL, SW_SHOW);
		}
	}NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.dinatale 2011-09-13 10:16) - PLID 45369 - check if the cursor is in our link label, if it is, we need the link cursor
BOOL CEMRBarcodeDlg::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message)
{
	try
	{
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		CRect rcWebSite;
		m_nxsInfoLink.GetWindowRect(rcWebSite);
		ScreenToClient(&rcWebSite);

		if(rcWebSite.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}NxCatchAll(__FUNCTION__);

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}
