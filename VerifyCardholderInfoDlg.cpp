// VerifyCardholderInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "VerifyCardholderInfoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVerifyCardholderInfoDlg dialog

// (j.gruber 2007-08-03 11:29) - PLID 26582  - created for
CVerifyCardholderInfoDlg::CVerifyCardholderInfoDlg(CString strCardholderName, long nPatientID, CWnd* pParent)
	: CNxDialog(CVerifyCardholderInfoDlg::IDD, pParent)
{
	m_strCardholderName = strCardholderName;
	m_nPatientID = nPatientID;
	//{{AFX_DATA_INIT(CVerifyCardholderInfoDlg)
	//}}AFX_DATA_INIT
}


void CVerifyCardholderInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVerifyCardholderInfoDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CARD_CVVD, m_edtCVVD);
	DDX_Control(pDX, IDC_CARDHOLDER_NAME, m_editCardholderName);
	DDX_Control(pDX, IDC_CARDHOLDER_ADDRESS2, m_edtAddress2);
	DDX_Control(pDX, IDC_CARDHOLDER_ADDRESS1, m_edtAddress1);
	DDX_Control(pDX, IDC_CARDHOLDER_ZIP_CODE, m_editZipCode);
	DDX_Control(pDX, IDC_CODE_ILLEGIBLE, m_btnIllegible);
	DDX_Control(pDX, IDC_CODE_NOT_PROVIDED, m_btnNotProvided);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CVerifyCardholderInfoDlg, CNxDialog)
	//{{AFX_MSG_MAP(CVerifyCardholderInfoDlg)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CODE_ILLEGIBLE, OnCodeIllegible)
	ON_BN_CLICKED(IDC_CODE_NOT_PROVIDED, OnCodeNotProvided)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVerifyCardholderInfoDlg message handlers

void CVerifyCardholderInfoDlg::OnOK() 
{
	try {
		//check that everything is required filled in
		CString strName, strAddress1, strAddress2, strZip;
		CString strSecCode;

		GetDlgItemText(IDC_CARDHOLDER_NAME, strName);
		strName.TrimLeft();
		strName.TrimRight();
		if (strName.IsEmpty()) {
			MessageBox("Please enter the name on the card or click cancel.");
			return;
		}

		GetDlgItemText(IDC_CARDHOLDER_ADDRESS1, strAddress1);
		strAddress1.TrimLeft();
		strAddress1.TrimRight();
		if (strAddress1.IsEmpty()) {
			MessageBox("Please enter the cardholder information in the Address 1 box or click cancel.");
			return;
		}

		GetDlgItemText(IDC_CARDHOLDER_ZIP_CODE, strZip);
		strZip.TrimLeft();
		strZip.TrimRight();
		if (strZip.IsEmpty()) {
			MessageBox("Please enter the cardholder zip code or click cancel.");
			return;
		}

		GetDlgItemText(IDC_CARDHOLDER_ADDRESS2, strAddress2);
		strAddress2.TrimLeft();
		strAddress2.TrimRight();

		GetDlgItemText(IDC_CARD_CVVD, strSecCode);
		strSecCode.TrimLeft();
		strSecCode.TrimRight();	


		//if we got here, we are good to go
		m_strCardholderName = strName;
		m_strCardholderAddress1 = strAddress1;
		m_strCardholderAddress2 = strAddress2;
		m_strCardholderZip = strZip;
		m_strCardholderSecurityCode = strSecCode;

		if (IsDlgButtonChecked(IDC_CODE_ILLEGIBLE)) {
			m_bCodeIllegible = TRUE;
		}
		else {
			m_bCodeIllegible = FALSE;
		}

		if (IsDlgButtonChecked(IDC_CODE_NOT_PROVIDED)) {
			m_bCodeNotProvidedByCustomer = TRUE;
		}
		else {
			m_bCodeNotProvidedByCustomer = FALSE;
		}
		
		CDialog::OnOK();
	}NxCatchAll("Error in CVerifyCardholderInfoDlg::OnOK");
}

void CVerifyCardholderInfoDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

BOOL CVerifyCardholderInfoDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	try {
	
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//pull the credit card name from the payment and the billing information from the patient's module
		SetDlgItemText(IDC_CARDHOLDER_NAME, m_strCardholderName);

		ADODB::_RecordsetPtr rs = CreateRecordset("SELECT Address1, Address2, Zip FROM PersonT WHERE ID = %li", m_nPatientID);

		if (! rs->eof) {

			m_strCardholderAddress1 = AdoFldString(rs, "Address1", "");
			m_strCardholderAddress2 = AdoFldString(rs, "Address2", "");
			m_strCardholderZip = AdoFldString(rs, "Zip", "");

			SetDlgItemText(IDC_CARDHOLDER_ADDRESS1, m_strCardholderAddress1);
			SetDlgItemText(IDC_CARDHOLDER_ADDRESS2, m_strCardholderAddress2);
			SetDlgItemText(IDC_CARDHOLDER_ZIP_CODE, m_strCardholderZip);
		}

		m_brush.CreateSolidBrush(PaletteColor(0x009CC294));
		m_edtAddress2.LimitText(20);
		m_editCardholderName.LimitText(50);
		m_edtAddress1.LimitText(20);
		m_editZipCode.LimitText(10);
		m_edtCVVD.LimitText(4);
		
	}NxCatchAll("Error in CVerifyCardholderInfoDlg::OnInitDialog");
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

HBRUSH CVerifyCardholderInfoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	// (a.walling 2008-05-12 13:30) - PLID 29497 - Deprecated, use base class
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	/*
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() != IDC_CARD_CVVD) {

	
		if (nCtlColor == CTLCOLOR_STATIC) {
			extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(0x009CC294));
			return m_brush;
		}
	}

	
	// TODO: Return a different brush if the default is not desired
	return hbr;
	*/
}

void CVerifyCardholderInfoDlg::OnCodeIllegible() 
{
	if (IsDlgButtonChecked(IDC_CODE_ILLEGIBLE)) {

		//uncheck the other checkbox
		CheckDlgButton(IDC_CODE_NOT_PROVIDED, 0);
		//gray out the box
		GetDlgItem(IDC_CARD_CVVD)->EnableWindow(FALSE);
	}
	else {

		//ungray the box in case they want to enter something
		GetDlgItem(IDC_CARD_CVVD)->EnableWindow(TRUE);
	}
	
}

void CVerifyCardholderInfoDlg::OnCodeNotProvided() 
{
	if (IsDlgButtonChecked(IDC_CODE_NOT_PROVIDED)) {

		//uncheck the other checkbox
		CheckDlgButton(IDC_CODE_ILLEGIBLE, 0);
		//gray out the box
		GetDlgItem(IDC_CARD_CVVD)->EnableWindow(FALSE);
	}
	else {

		//ungray the box in case they want to enter something
		GetDlgItem(IDC_CARD_CVVD)->EnableWindow(TRUE);
	}	
}
