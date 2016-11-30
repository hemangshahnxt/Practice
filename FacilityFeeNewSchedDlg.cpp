// FacilityFeeNewSchedDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FacilityFeeNewSchedDlg.h"
#include "InternationalUtils.h"
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFacilityFeeNewSchedDlg dialog


CFacilityFeeNewSchedDlg::CFacilityFeeNewSchedDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CFacilityFeeNewSchedDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFacilityFeeNewSchedDlg)
		m_nHours = 0;
		m_nMinutes = 0;
		m_cyFee = COleCurrency(0,0);
		m_bIsFacilityFee = TRUE;
	//}}AFX_DATA_INIT
}


void CFacilityFeeNewSchedDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFacilityFeeNewSchedDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_EDIT_NEW_HOURS, m_nxeditEditNewHours);
	DDX_Control(pDX, IDC_EDIT_NEW_MINUTES, m_nxeditEditNewMinutes);
	DDX_Control(pDX, IDC_EDIT_FEE, m_nxeditEditFee);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFacilityFeeNewSchedDlg, CNxDialog)
	//{{AFX_MSG_MAP(CFacilityFeeNewSchedDlg)
	ON_WM_CTLCOLOR()
	ON_EN_KILLFOCUS(IDC_EDIT_NEW_HOURS, OnKillfocusEditHours)
	ON_EN_KILLFOCUS(IDC_EDIT_NEW_MINUTES, OnKillfocusEditMinutes)
	ON_EN_KILLFOCUS(IDC_EDIT_FEE, OnKillfocusEditFee)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFacilityFeeNewSchedDlg message handlers

BOOL CFacilityFeeNewSchedDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (z.manning, 04/30/2008) - PLID 29860 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	if(!m_bIsFacilityFee) {
		SetWindowText("New Scheduled Anesthesia Fee");
	}
	
	SetDlgItemInt(IDC_EDIT_NEW_HOURS, 0);
	SetDlgItemInt(IDC_EDIT_NEW_MINUTES, 0);
	SetDlgItemText(IDC_EDIT_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFacilityFeeNewSchedDlg::OnOK() 
{
	try {

		m_nHours = GetDlgItemInt(IDC_EDIT_NEW_HOURS);
		m_nMinutes = GetDlgItemInt(IDC_EDIT_NEW_MINUTES);

		CString strFee;
		GetDlgItemText(IDC_EDIT_FEE, strFee);
		m_cyFee = ParseCurrencyFromInterface(strFee);		
	
		CDialog::OnOK();

	}NxCatchAll("Error adding new fee.");
}

void CFacilityFeeNewSchedDlg::OnKillfocusEditHours() 
{
	long nHours = GetDlgItemInt(IDC_EDIT_NEW_HOURS);
	if(nHours > 24) {
		AfxMessageBox("You cannot enter more than 24 hours.");
		SetDlgItemInt(IDC_EDIT_NEW_HOURS, 24);
	}
	else if(nHours == 24) {
		long nMinutes = GetDlgItemInt(IDC_EDIT_NEW_MINUTES);
		if(nMinutes > 0) {
			AfxMessageBox("You cannot enter more than 24 hours and 0 minutes.");
			SetDlgItemInt(IDC_EDIT_NEW_MINUTES, 0);
		}
	}
}

void CFacilityFeeNewSchedDlg::OnKillfocusEditMinutes() 
{
	long nMinutes = GetDlgItemInt(IDC_EDIT_NEW_MINUTES);
	long nHours = GetDlgItemInt(IDC_EDIT_NEW_HOURS);

	if(nHours == 24) {
		if(nMinutes > 0) {
			AfxMessageBox("You cannot enter more than 24 hours and 0 minutes.");
			SetDlgItemInt(IDC_EDIT_NEW_MINUTES, 0);
			return;
		}
	}

	if(nMinutes > 59) {
		AfxMessageBox("You cannot enter more than 59 minutes.");
		SetDlgItemInt(IDC_EDIT_NEW_MINUTES, 59);
	}
}

void CFacilityFeeNewSchedDlg::OnKillfocusEditFee() 
{
	try {
		CString strFee;
		GetDlgItemText(IDC_EDIT_FEE, strFee);

		if (strFee.GetLength() == 0) {
			MsgBox("Please fill in the 'Fee' box.");
			SetDlgItemText(IDC_EDIT_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		COleCurrency cy = ParseCurrencyFromInterface(strFee);
		if(cy.GetStatus() == COleCurrency::invalid) {
			MsgBox("Please enter a valid amount in the 'Fee' box.");
			SetDlgItemText(IDC_EDIT_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		if (cy < COleCurrency::COleCurrency(0,0))
		{
			MsgBox("Fees cannot be negative. Please enter a positive number.");
			SetDlgItemText(IDC_EDIT_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		//see how much the regional settings allows to the right of the decimal
		CString strICurr;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
		int nDigits = atoi(strICurr);
		CString strDecimal;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
		if(cy.Format().Find(strDecimal) != -1 && cy.Format().Find(strDecimal) + (nDigits+1) < cy.Format().GetLength()) {
			MsgBox("Please fill only %li places to the right of the %s in the 'Fee' box.",nDigits,strDecimal == "," ? "comma" : "decimal");
			SetDlgItemText(IDC_EDIT_FEE, FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE));
			return;
		}

		CString str = FormatCurrencyForInterface(cy, TRUE, TRUE);
		SetDlgItemText(IDC_EDIT_FEE, str);

	}NxCatchAll("Error validating fee.");
}
