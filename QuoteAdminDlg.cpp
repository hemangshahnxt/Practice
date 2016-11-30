// QuoteAdminDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "pracprops.h"
#include "QuoteAdminDlg.h"
#include "QuoteNotes.h"
#include "InternationalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQuoteAdminDlg dialog

CQuoteAdminDlg::CQuoteAdminDlg(CWnd* pParent)
	: CNxDialog(CQuoteAdminDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQuoteAdminDlg)
	//}}AFX_DATA_INIT
}

void CQuoteAdminDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQuoteAdminDlg)
	DDX_Control(pDX, IDC_RADIO_FLAT_FEE_REQUIRED, m_btnFlatFeeRequired);
	DDX_Control(pDX, IDC_RADIO_PERCENT_REQUIRED, m_btnPercentRequired);
	DDX_Control(pDX, IDC_RADIO_NONE_REQUIRED, m_btnNoneRequired);
	DDX_Control(pDX, IDC_USE_EMAIL, m_btnUseEmail);
	DDX_Control(pDX, IDC_DEFAULT_EXPIRES_CHECK, m_btnQuotesExpireDefault);
	DDX_Control(pDX, IDC_SEPARATE_QUOTE_TAX, m_btnSeparateTax);
	DDX_Control(pDX, IDC_SHOW_PROC_DESCRIPTIONS, m_btnShowProcDesc);
	DDX_Control(pDX, IDC_SHOW_QUOTE_DESC, m_btnShowQuoteDesc);
	DDX_Control(pDX, IDC_USELETTERHEAD, m_btnUserLetterhead);
	DDX_Control(pDX, IDC_USE_WITSIGNATURE, m_btnUseWitnessSignature);
	DDX_Control(pDX, IDC_USE_SIGNATURE, m_btnUseSignature);
	DDX_Control(pDX, IDC_USE_QUOTE_DATE, m_btnUseQuoteDate);
	DDX_Control(pDX, IDC_USE_OTHER_PHONE, m_btnOverridePracPhone);
	DDX_Control(pDX, IDC_PATIENT_COORD_FORMAT, m_pat_coord_format);
	DDX_Control(pDX, IDC_PERCENT, m_percent);
	DDX_Control(pDX, IDC_QUANTITY, m_quantity);
	DDX_Control(pDX, IDC_DISCOUNT, m_discount);
	DDX_Control(pDX, IDC_PAID_TO_OTHERS, m_paid_to_others);
	DDX_Control(pDX, IDC_PATIENT_COORD, m_pat_coord);
	DDX_Control(pDX, IDC_NXCOLORCTRL6, m_bkg1);
	DDX_Control(pDX, IDC_NXCOLORCTRL10, m_bkg2);
	DDX_Control(pDX, IDC_EDIT_PRE_DETAIL, m_nxeditEditPreDetail);
	DDX_Control(pDX, IDC_EDIT_POST_DETAIL, m_nxeditEditPostDetail);
	DDX_Control(pDX, IDC_REPORT_TITLE, m_nxeditReportTitle);
	DDX_Control(pDX, IDC_SUBREP_TITLE, m_nxeditSubrepTitle);
	DDX_Control(pDX, IDC_PRAC_OTHER_PHONE, m_nxeditPracOtherPhone);
	DDX_Control(pDX, IDC_DEFAULT_EXPDAYS, m_nxeditDefaultExpdays);
	DDX_Control(pDX, IDC_EDIT_DEPOSIT_PERCENTAGE, m_nxeditEditDepositPercentage);
	DDX_Control(pDX, IDC_EDIT_DEPOSIT_FLAT_FEE, m_nxeditEditDepositFlatFee);
	DDX_Control(pDX, IDC_OK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_LIST_DISC_BY_CATEGORY, m_btnShowDiscountsByCategory);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CQuoteAdminDlg, CNxDialog)
	//{{AFX_MSG_MAP(CQuoteAdminDlg)
	ON_BN_CLICKED(IDC_OK, OnOk)
	ON_BN_CLICKED(IDC_USELETTERHEAD, OnUseletterhead)
	ON_BN_CLICKED(IDC_SHOW_QUOTE_DESC, OnShowQuoteDesc)
	ON_CBN_SELCHANGE(IDC_PATIENT_COORD_FORMAT, OnSelchangePatientCoordFormat)
	ON_CBN_SELCHANGE(IDC_PATIENT_COORD, OnSelchangePatientCoord)
	ON_BN_CLICKED(IDC_USE_OTHER_PHONE, OnUseOtherPhone)
	ON_BN_CLICKED(IDC_RADIO_NONE_REQUIRED, OnRadioDepositTypeChanged)
	ON_EN_KILLFOCUS(IDC_EDIT_DEPOSIT_PERCENTAGE, OnKillfocusEditDepositPercentage)
	ON_BN_CLICKED(IDC_RADIO_PERCENT_REQUIRED, OnRadioDepositTypeChanged)
	ON_BN_CLICKED(IDC_RADIO_FLAT_FEE_REQUIRED, OnRadioDepositTypeChanged)
	ON_EN_KILLFOCUS(IDC_EDIT_DEPOSIT_FLAT_FEE, OnKillfocusEditDepositFlatFee)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQuoteAdminDlg message handlers

BOOL CQuoteAdminDlg::OnInitDialog() 
{		
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-02 08:52) - PLID 29876 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_bkg1.SetColor(m_nColor);
		m_bkg2.SetColor(m_nColor);
				
		SetDlgItemText (IDC_EDIT_PRE_DETAIL, GetRemotePropertyMemo ("QuoteTopText", "", 0, "<None>"));
		SetDlgItemText (IDC_EDIT_POST_DETAIL, GetRemotePropertyMemo ("QuoteBottomText", "", 0, "<None>"));

		m_pat_coord.SetCurSel(GetRemotePropertyInt("QuotePatCoord", 2, 0, "<None>"));
		m_pat_coord_format.SetCurSel(GetRemotePropertyInt("QuotePatCoordFormat", 2, 0, "<None>"));
		m_paid_to_others.SetCurSel(GetRemotePropertyInt("QuotePaidOthers", 2, 0, "<None>"));
		m_discount.SetCurSel(GetRemotePropertyInt("QuoteDiscount", 2, 0, "<None>"));
		m_percent.SetCurSel(GetRemotePropertyInt("QuotePercent", 2, 0, "<None>"));
		m_quantity.SetCurSel(GetRemotePropertyInt("QuoteQuantity", 2, 0, "<None>"));
		((CButton*)GetDlgItem(IDC_USE_SIGNATURE))->SetCheck(GetRemotePropertyInt ("QuoteUsePatSig", -1, 0, "<None>"));
		((CButton*)GetDlgItem(IDC_USE_WITSIGNATURE))->SetCheck(GetRemotePropertyInt ("QuoteUseWitSig", -1, 0, "<None>"));
		((CButton*)GetDlgItem(IDC_USE_EMAIL))->SetCheck(GetRemotePropertyInt ("QuoteUseEmail", 0, 0, "<None>"));	
		SetDlgItemText (IDC_REPORT_TITLE, GetRemotePropertyText ("QuoteReportTitle", "Surgery Proposal", 0, "<None>", true));
		SetDlgItemText (IDC_SUBREP_TITLE, GetRemotePropertyText("QuoteSubReportTitle", "Surgery and Related Services", 0, "<None>", true));	
		SetDlgItemInt(IDC_DEFAULT_EXPDAYS,GetRemotePropertyInt("QuoteDefExpDays",30,0,"<None>",true));
		((CButton*)GetDlgItem(IDC_SHOW_PROC_DESCRIPTIONS))->SetCheck(GetRemotePropertyInt ("QuoteShowProcDescriptions", 1, 0, "<None>"));

		((CButton*)GetDlgItem(IDC_USELETTERHEAD))->SetCheck(GetRemotePropertyInt ("QuoteUseAlternate", 0, 0, "<None>"));
		((CButton*)GetDlgItem(IDC_USE_QUOTE_DATE))->SetCheck(GetRemotePropertyInt ("QuoteUseQuoteDate", 1, 0, "<None>"));	

		if(GetRemotePropertyInt("QuoteShowDescription", 1, 0, "<None>", true)) {
			CheckDlgButton(IDC_SHOW_QUOTE_DESC, BST_CHECKED);
		}

		((CButton*)GetDlgItem(IDC_SEPARATE_QUOTE_TAX))->SetCheck(GetRemotePropertyInt ("QuoteSeparateTotals", 0, 0, "<None>"));	

		((CButton*)GetDlgItem(IDC_DEFAULT_EXPIRES_CHECK))->SetCheck(GetRemotePropertyInt ("QuoteDefaultExpires", 0, 0, "<None>"));

		// (j.gruber 2009-03-19 17:56) - PLID 33349 - new quote settings
		((CButton*)GetDlgItem(IDC_LIST_DISC_BY_CATEGORY))->SetCheck(GetRemotePropertyInt ("QuoteShowDiscountsByCategory", 0, 0, "<None>"));

		//grey out the format box if they choose never
		if (m_pat_coord.GetCurSel() == 1) {

			GetDlgItem(IDC_PATIENT_COORD_FORMAT)->EnableWindow(FALSE);
		}
		else {

			GetDlgItem(IDC_PATIENT_COORD_FORMAT)->EnableWindow(TRUE);
		}


		if (GetRemotePropertyInt("QuoteUseOtherPhone",0,0,"<None>",true)) {
			
			CheckDlgButton(IDC_USE_OTHER_PHONE, BST_CHECKED);
			GetDlgItem(IDC_PRAC_OTHER_PHONE)->EnableWindow(TRUE);
			SetDlgItemText(IDC_PRAC_OTHER_PHONE, GetRemotePropertyText("QuoteOtherPracPhone", "", 0, "<None>", true));
		}
		else {

			CheckDlgButton(IDC_USE_OTHER_PHONE, BST_UNCHECKED);
			GetDlgItem(IDC_PRAC_OTHER_PHONE)->EnableWindow(FALSE);
			SetDlgItemText(IDC_PRAC_OTHER_PHONE, "");
		}

		long nDepositTypeRequired = GetRemotePropertyInt("QuoteDepositTypeRequired",0,0,"<None>",true);
		if(nDepositTypeRequired == 0)
			CheckDlgButton(IDC_RADIO_NONE_REQUIRED, TRUE);
		else if(nDepositTypeRequired == 1)
			CheckDlgButton(IDC_RADIO_PERCENT_REQUIRED, TRUE);
		else if(nDepositTypeRequired == 2)
			CheckDlgButton(IDC_RADIO_FLAT_FEE_REQUIRED, TRUE);
		OnRadioDepositTypeChanged();

		SetDlgItemInt(IDC_EDIT_DEPOSIT_PERCENTAGE, GetRemotePropertyInt("QuoteDepositPercent",0,0,"<None>",true));
		SetDlgItemText(IDC_EDIT_DEPOSIT_FLAT_FEE, GetRemotePropertyText("QuoteDepositFee",FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE),0,"<None>",true));
	}
	NxCatchAll("Error in CQuoteAdminDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE	
}
	

void CQuoteAdminDlg::OnOk() 
{
	CString str;

	//save all the info that they entered in ConfigRT
	GetDlgItemText (IDC_EDIT_PRE_DETAIL, str);
	SetRemotePropertyMemo ("QuoteTopText", str, 0, "<None>");

	GetDlgItemText (IDC_EDIT_POST_DETAIL, str);
	SetRemotePropertyMemo ("QuoteBottomText", str, 0, "<None>");
	
	SetRemotePropertyInt ("QuotePatCoord", m_pat_coord.GetCurSel(), 0, "<None>");
	SetRemotePropertyInt ("QuotePatCoordFormat", m_pat_coord_format.GetCurSel(), 0, "<None>");
	SetRemotePropertyInt ("QuotePaidOthers", m_paid_to_others.GetCurSel(), 0, "<None>");
	SetRemotePropertyInt ("QuoteDiscount", m_discount.GetCurSel(), 0, "<None>");
	SetRemotePropertyInt ("QuotePercent", m_percent.GetCurSel(), 0, "<None>");
	SetRemotePropertyInt ("QuoteQuantity", m_quantity.GetCurSel(), 0, "<None>");
	SetRemotePropertyInt ("QuoteUsePatSig", ((CButton*)GetDlgItem(IDC_USE_SIGNATURE))->GetCheck(), 0, "<None>");
	SetRemotePropertyInt ("QuoteUseWitSig", ((CButton*)GetDlgItem(IDC_USE_WITSIGNATURE))->GetCheck(), 0, "<None>");
	SetRemotePropertyInt ("QuoteUseEmail", ((CButton*)GetDlgItem(IDC_USE_EMAIL))->GetCheck(), 0, "<None>");
	SetRemotePropertyInt ("QuoteSeparateTotals", ((CButton*)GetDlgItem(IDC_SEPARATE_QUOTE_TAX))->GetCheck(), 0, "<None>");
	SetRemotePropertyInt("QuoteUseAlternate", ((CButton*)GetDlgItem(IDC_USELETTERHEAD))->GetCheck(), 0, "<None>");
	SetRemotePropertyInt("QuoteShowDescription", IsDlgButtonChecked(IDC_SHOW_QUOTE_DESC), 0, "<None>");
	SetRemotePropertyInt("QuoteUseOtherPhone", IsDlgButtonChecked(IDC_USE_OTHER_PHONE), 0, "<None>");
	SetRemotePropertyInt ("QuoteUseQuoteDate", ((CButton*)GetDlgItem(IDC_USE_QUOTE_DATE))->GetCheck(), 0, "<None>");
	// (j.gruber 2009-03-20 08:32) - PLID 33349 - quote setting for showing discounts by category
	SetRemotePropertyInt ("QuoteShowDiscountsByCategory", ((CButton*)GetDlgItem(IDC_LIST_DISC_BY_CATEGORY))->GetCheck(), 0, "<None>");

	if (IsDlgButtonChecked(IDC_USE_OTHER_PHONE)) {
		GetDlgItemText(IDC_PRAC_OTHER_PHONE, str);
		SetRemotePropertyText("QuoteOtherPracPhone", str, 0, "<None>");
	}
	
	    
	GetDlgItemText(IDC_REPORT_TITLE, str);
	SetRemotePropertyText("QuoteReportTitle", str, 0, "<None>");
	GetDlgItemText(IDC_SUBREP_TITLE, str);
	SetRemotePropertyText("QuoteSubReportTitle", str, 0, "<None>");
	
	SetRemotePropertyInt ("QuoteDefaultExpires", ((CButton*)GetDlgItem(IDC_DEFAULT_EXPIRES_CHECK))->GetCheck(), 0, "<None>");

	SetRemotePropertyInt ("QuoteShowProcDescriptions", ((CButton*)GetDlgItem(IDC_SHOW_PROC_DESCRIPTIONS))->GetCheck(), 0, "<None>");

	long expdays;
	expdays = GetDlgItemInt(IDC_DEFAULT_EXPDAYS);
	SetRemotePropertyInt("QuoteDefExpDays",expdays,0,"<None>");

	long nDepositTypeRequired = 0;
	if(IsDlgButtonChecked(IDC_RADIO_PERCENT_REQUIRED)) {
		nDepositTypeRequired = 1;

		long nPercentage = GetDlgItemInt(IDC_EDIT_DEPOSIT_PERCENTAGE);
		SetRemotePropertyInt("QuoteDepositPercent",nPercentage,0,"<None>");
	}
	else if(IsDlgButtonChecked(IDC_RADIO_FLAT_FEE_REQUIRED)) {
		nDepositTypeRequired = 2;

		CString strDepositFee;
		GetDlgItemText(IDC_EDIT_DEPOSIT_FLAT_FEE, strDepositFee);
		SetRemotePropertyText("QuoteDepositFee",strDepositFee,0,"<None>");
	}
	SetRemotePropertyInt("QuoteDepositTypeRequired",nDepositTypeRequired,0,"<None>");

	CDialog::OnOK();	
}

void CQuoteAdminDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CQuoteAdminDlg::OnUseletterhead() 
{

	
}

void CQuoteAdminDlg::OnShowQuoteDesc() 
{
	
}

void CQuoteAdminDlg::OnSelchangePatientCoordFormat() 
{
	// TODO: Add your control notification handler code here
	
}

void CQuoteAdminDlg::OnSelchangePatientCoord() 
{
	//grey out the format box if they choose never
	if (m_pat_coord.GetCurSel() == 1) {

		GetDlgItem(IDC_PATIENT_COORD_FORMAT)->EnableWindow(FALSE);
	}
	else {

		GetDlgItem(IDC_PATIENT_COORD_FORMAT)->EnableWindow(TRUE);
	}


	
}

void CQuoteAdminDlg::OnUseOtherPhone() 
{
	if (IsDlgButtonChecked(IDC_USE_OTHER_PHONE)) {

		//enable the phone number
		GetDlgItem(IDC_PRAC_OTHER_PHONE)->EnableWindow(TRUE);
		SetDlgItemText(IDC_PRAC_OTHER_PHONE, GetRemotePropertyText("QuoteOtherPracPhone", "", 0, "<None>", true));
	}
	else {

		CheckDlgButton(IDC_USE_OTHER_PHONE, BST_UNCHECKED);
		GetDlgItem(IDC_PRAC_OTHER_PHONE)->EnableWindow(FALSE);
		SetDlgItemText(IDC_PRAC_OTHER_PHONE, "");
	}

	
}

BOOL CQuoteAdminDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int nID;
	static CString str;

	switch (HIWORD(wParam)) {
		case EN_CHANGE:
		switch (nID = LOWORD(wParam)) {

			case IDC_PRAC_OTHER_PHONE:
				GetDlgItemText(nID, str);
				str.TrimRight();
				if (str != "") {
					if(GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true)) {
						FormatItem (nID, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true));
					}
				}
			break;	

			default:
			break;
		}
	}

	
	return CNxDialog::OnCommand(wParam, lParam);
}

void CQuoteAdminDlg::OnRadioDepositTypeChanged()
{
	if(IsDlgButtonChecked(IDC_RADIO_NONE_REQUIRED)) {
		GetDlgItem(IDC_EDIT_DEPOSIT_PERCENTAGE)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_DEPOSIT_FLAT_FEE)->EnableWindow(FALSE);
	}
	else if(IsDlgButtonChecked(IDC_RADIO_PERCENT_REQUIRED)) {
		GetDlgItem(IDC_EDIT_DEPOSIT_PERCENTAGE)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_DEPOSIT_FLAT_FEE)->EnableWindow(FALSE);
	}
	else if(IsDlgButtonChecked(IDC_RADIO_FLAT_FEE_REQUIRED)) {
		GetDlgItem(IDC_EDIT_DEPOSIT_PERCENTAGE)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_DEPOSIT_FLAT_FEE)->EnableWindow(TRUE);
	}
}

void CQuoteAdminDlg::OnKillfocusEditDepositPercentage() 
{
	try {
		long nPercentage = GetDlgItemInt(IDC_EDIT_DEPOSIT_PERCENTAGE);
		if(nPercentage > 100) {
			AfxMessageBox("You cannot have a deposit percentage requirement greater than 100%.");
			SetDlgItemInt(IDC_EDIT_DEPOSIT_PERCENTAGE, GetRemotePropertyInt("QuoteDepositPercent",0,0,"<None>",true));
			return;
		}
		else if(nPercentage == 0) {
			AfxMessageBox("You cannot have a deposit percentage requirement of 0%.");
			SetDlgItemInt(IDC_EDIT_DEPOSIT_PERCENTAGE, GetRemotePropertyInt("QuoteDepositPercent",0,0,"<None>",true));
			return;
		}
	}NxCatchAll("Error validating deposit percentage.");
}

void CQuoteAdminDlg::OnKillfocusEditDepositFlatFee() 
{
	try {
		CString strDepositFee;
		GetDlgItemText(IDC_EDIT_DEPOSIT_FLAT_FEE, strDepositFee);

		if (strDepositFee.GetLength() == 0) {
			MsgBox("Please fill in the 'Flat Fee' box.");
			SetDlgItemText(IDC_EDIT_DEPOSIT_FLAT_FEE, GetRemotePropertyText("QuoteDepositFee",FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE),0,"<None>",true));
			return;
		}

		COleCurrency cy = ParseCurrencyFromInterface(strDepositFee);
		if(cy.GetStatus() == COleCurrency::invalid) {
			MsgBox("Please enter a valid amount in the 'Flat Fee' box.");
			SetDlgItemText(IDC_EDIT_DEPOSIT_FLAT_FEE, GetRemotePropertyText("QuoteDepositFee",FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE),0,"<None>",true));
			return;
		}

		if (cy <= COleCurrency::COleCurrency(0,0))
		{
			MsgBox("Fees cannot be negative or zero. Please enter a positive number.");
			SetDlgItemText(IDC_EDIT_DEPOSIT_FLAT_FEE, GetRemotePropertyText("QuoteDepositFee",FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE),0,"<None>",true));
			return;
		}

		//see how much the regional settings allows to the right of the decimal
		CString strICurr;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
		int nDigits = atoi(strICurr);
		CString strDecimal;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
		if(cy.Format().Find(strDecimal) != -1 && cy.Format().Find(strDecimal) + (nDigits+1) < cy.Format().GetLength()) {
			MsgBox("Please fill only %li places to the right of the %s in the 'Flat Fee' box.",nDigits,strDecimal == "," ? "comma" : "decimal");
			SetDlgItemText(IDC_EDIT_DEPOSIT_FLAT_FEE, GetRemotePropertyText("QuoteDepositFee",FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE),0,"<None>",true));
			return;
		}

		CString str = FormatCurrencyForInterface(cy, TRUE, TRUE);
		SetDlgItemText(IDC_EDIT_DEPOSIT_FLAT_FEE, str);

	}NxCatchAll("Error validating deposit fee.");	
}
