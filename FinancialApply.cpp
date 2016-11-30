// FinancialApply.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "FinancialApply.h"
#include "GlobalFinancialUtils.h"
#include "GlobalUtils.h"
#include "NxStandard.h"
#include "CalculatePercentageDlg.h"
#include "InternationalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CFinancialApply dialog

// (j.jones 2010-09-03 10:16) - PLID 40392 - added resp. combo columns
enum RespComboColumn {

	rccID = 0,
	rccName,
	rccPriority,
	rccCategoryTypeID,
	rccCategoryName,
};

CFinancialApply::CFinancialApply(CWnd* pParent /*=NULL*/)
	: CNxDialog(CFinancialApply::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFinancialApply)
		m_boIncreaseInsBalance = FALSE;
		m_boShiftBalance = FALSE;
		m_boAdjustBalance = FALSE;
		m_nResponsibility = 0;
		m_nShiftToResp = 0;
		m_PatientID = -1;
		m_boShowAdjCheck = FALSE;
		m_boShowIncreaseCheck = FALSE;
		m_boIsAdjustment = FALSE;
		m_boApplyToPay = FALSE;
		m_boZeroAmountAllowed = FALSE;
		m_pFont = NULL;
	//}}AFX_DATA_INIT
}

CFinancialApply::~CFinancialApply()
{
	//DRT 3/10/2005 - PLID 15902 - Need to clear up memory for the font!
	if(m_pFont) {
		delete m_pFont;
		m_pFont = NULL;
	}
}

void CFinancialApply::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFinancialApply)
	DDX_Control(pDX, IDC_LABEL_AVAILABLE_APPLY, m_labelAvailApply);
	DDX_Control(pDX, IDC_LABEL_NET_CHARGES, m_labelNetCharges);
	DDX_Control(pDX, IDC_CHECK_SHIFT, m_shiftCheck);
	DDX_Control(pDX, IDC_CHECK_INCREASE, m_increaseCheck);
	DDX_Control(pDX, IDC_CHECK_ADJUST, m_adjustCheck);
	DDX_Control(pDX, IDC_EDIT_PAYMENT, m_nxeditEditPayment);
	DDX_Control(pDX, IDC_LABEL_RESPONSIBILITY, m_nxstaticLabelResponsibility);
	DDX_Control(pDX, IDC_LABEL4, m_nxstaticLabel4);
	DDX_Control(pDX, IDC_LABEL3, m_nxstaticLabel3);
	DDX_Control(pDX, IDC_CURRENCY_SYMBOL_APPLY, m_nxstaticCurrencySymbolApply);
	DDX_Control(pDX, IDC_BTN_APPLY, m_btnApply);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CALC_PERCENT, m_btnCalcPercent);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFinancialApply, CNxDialog)
	//{{AFX_MSG_MAP(CFinancialApply)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BTN_APPLY, OnBtnApply)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CHECK_ADJUST, OnCheckAdjust)
	ON_BN_CLICKED(IDC_CHECK_INCREASE, OnCheckIncrease)
	ON_BN_CLICKED(IDC_CHECK_SHIFT, OnCheckShift)
	ON_BN_CLICKED(IDC_CALC_PERCENT, OnCalcPercent)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFinancialApply message handlers

BEGIN_EVENTSINK_MAP(CFinancialApply, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CFinancialApply)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

//Used only by OnClickBtnApply -BVB
inline COleCurrency cyabs (COleCurrency &cy)
{
	const COleCurrency cyZero = COleCurrency(0, 0);
	if (cy > cyZero)
		return cy;
	else return cyZero - cy;
}

inline short cysign (COleCurrency &cy)
{
	const COleCurrency cyZero = COleCurrency(0, 0);
	if (cy > cyZero)
		return 1;
	else if (cy < cyZero)
		return -1;
	else return 0;
}

void CFinancialApply::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	if (!bShow) return;

	m_pFont = new CFont;
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(m_pFont, 120, "Arial");

	m_labelNetCharges.SetFont(m_pFont);
	m_labelAvailApply.SetFont(m_pFont);
	
	// Design window
	if (m_nResponsibility != 0) {
		SetWindowText(GetStringOfResource(IDS_INSURED_APPLY));

		//DRT 5/12/03 - With the new resp type stuff, we can't really use the resources, more's the pity. 
		CString strRespName = GetNameFromRespTypeID(m_nResponsibility);
		SetDlgItemText(IDC_LABEL_RESPONSIBILITY, strRespName + " Ins. Balance");

		if(m_nResponsibility == -1) {
			//DRT 5/27/03 - There's really no reason to not allow shifting to patient resp 
			//		when applying to inactive - we are applying to a specific insured party, 
			//		that just happens to be inactive, not applying to inactive in general.
			//GetDlgItem(IDC_CHECK_SHIFT)->ShowWindow(SW_HIDE);
			//GetDlgItem(IDC_RESP_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CHECK_INCREASE)->ShowWindow(SW_HIDE);
		}

		if(!m_boShowIncreaseCheck) {
			GetDlgItem(IDC_CHECK_INCREASE)->ShowWindow(SW_HIDE);
		}
		else {
			// (j.jones 2011-11-02 14:50) - PLID 38686 - added ability to check this box
			// by default, by setting m_boIncreaseInsBalance to true
			m_increaseCheck.SetCheck(m_boIncreaseInsBalance);
		}
	}
	else {
		GetDlgItem(IDC_CHECK_SHIFT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RESP_COMBO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CHECK_INCREASE)->ShowWindow(SW_HIDE);		
	}

	if(!m_boShowAdjCheck)
		GetDlgItem(IDC_CHECK_ADJUST)->ShowWindow(SW_HIDE);

	// Set totals
	CString str = FormatCurrencyForInterface(m_cyNetCharges);
	GetDlgItem(IDC_LABEL_NET_CHARGES)->SetWindowText(str);
	str = FormatCurrencyForInterface(m_cyNetPayment);
	GetDlgItem(IDC_LABEL_AVAILABLE_APPLY)->SetWindowText(str);
	
	if (m_cyNetPayment > m_cyNetCharges) {
		m_cyApplyAmount = m_cyNetCharges;
		str = FormatCurrencyForInterface(m_cyNetCharges,FALSE,TRUE);
		GetDlgItem(IDC_EDIT_PAYMENT)->SetWindowText(str);
	}
	else if (m_boApplyToPay && m_cyNetPayment < COleCurrency(0,0) && (-m_cyNetPayment > m_cyNetCharges)) {
		m_cyApplyAmount = -m_cyNetCharges;
		str = FormatCurrencyForInterface(-m_cyNetCharges,FALSE,TRUE);
		GetDlgItem(IDC_EDIT_PAYMENT)->SetWindowText(str);
	}
	else {
		m_cyApplyAmount = m_cyNetPayment;
		str = FormatCurrencyForInterface(m_cyNetPayment,FALSE,TRUE);
		GetDlgItem(IDC_EDIT_PAYMENT)->SetWindowText(str);
	}

	GetDlgItem(IDC_EDIT_PAYMENT)->SetFocus();
	if (m_cyApplyAmount >= COleCurrency(0,0)) {
		((CNxEdit*)GetDlgItem(IDC_EDIT_PAYMENT))->SetSel(0, -1);
	}
	else {
		int nStart = 0, nFinish = -1;
		GetNonNegativeAmountExtent(nStart, nFinish);
		((CNxEdit*)GetDlgItem(IDC_EDIT_PAYMENT))->SetSel(nStart, nFinish);
	}

	// (j.jones 2011-11-02 16:34) - PLID 38686 - if we checked off this box, call OnCheckIncrease
	if(m_increaseCheck.GetCheck()) {
		OnCheckIncrease();
	}
}

void CFinancialApply::OnOK()
{
	OnBtnApply();
}

void CFinancialApply::OnBtnApply() 
{
	COleCurrency cy;
	CString str;
	COleVariant var;

	GetDlgItem(IDC_EDIT_PAYMENT)->GetWindowText(str);
	if (str.GetLength() == 0) {
		MsgBox(RCS(IDS_APPLY_AMT_BLANK));
		return;
	}

	////////////////////////////////////////////////////////
	// Make sure apply amount is not blank, has no bad characters,
	// and has no more than two places to the right of the
	// decimal point.
	////////////////////////////////////////////////////////
	GetDlgItem(IDC_EDIT_PAYMENT)->GetWindowText(str);
	if (str.GetLength() == 0) {
		MsgBox("Please fill in the 'Amount To Apply' box.");
		return;
	}

	if(!IsValidCurrencyText(str)) {
		MsgBox("Please fill correct information in the 'Amount To Apply' box.");
		return;
	}

	cy = ParseCurrencyFromInterface(str);

	if(cy.GetStatus() == COleCurrency::invalid) {
		MsgBox("Please fill correct information in the 'Amount To Apply' box.");
		return;
	}

	//see how much the regional settings allows to the right of the decimal
	CString strICurr;
	NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
	int nDigits = atoi(strICurr);
	CString strDecimal;
	NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
	if(cy.Format().Find(strDecimal) != -1 && cy.Format().Find(strDecimal) + (nDigits+1) < cy.Format().GetLength()) {
		MsgBox("Please fill only %li places to the right of the %s in the 'Amount To Apply' box.",nDigits,strDecimal == "," ? "comma" : "decimal");
		return;
	}

	// (j.jones 2011-11-02 14:51) - PLID 38686 - do not let m_boIncreaseInsBalance
	// be TRUE unless we are both showing the increase check and it is checked
	if(m_boShowIncreaseCheck && m_increaseCheck.GetCheck()) {
		m_boIncreaseInsBalance = TRUE;
	}
	else {
		m_boIncreaseInsBalance = FALSE;
	}

	//if (cy == COleCurrency(0, 0))
		//CDialog::OnCancel();

	if (!m_boIsAdjustment && (cy + m_cyNetCharges < COleCurrency(0, 0) && m_cyNetCharges >= COleCurrency(0,0))) {
		MsgBox(RCS(IDS_APPLY_AMT_INVALID),
			FormatCurrencyForInterface(cy));
		return;
	}

	else if (cyabs (cy) > cyabs (m_cyNetPayment)) {//BVB
		MsgBox(RCS(IDS_PAYMENT_AMT_INVALID),
			FormatCurrencyForInterface(m_cyNetPayment));
		return;
	}
	else if (cy == COleCurrency(0,0) && !m_boZeroAmountAllowed) {
		MsgBox(GetStringOfResource(IDS_APPLY_AMT_ZERO));
		return;
	}
	else if (cysign (cy) != cysign (m_cyNetPayment))//BVB
	{	if (cysign(cy) > 0) {
			MsgBox(RCS(IDS_AMT_NEGATIVE));
			return;
		}
		else if(cysign(cy) < 0) {
			MsgBox(RCS(IDS_AMT_POSITIVE));
			return;
		}		
	}
	else if (cy > m_cyNetCharges && !m_boIncreaseInsBalance) {
		MsgBox(GetStringOfResource(IDS_PAYMENT_AMT_INVALID),
			FormatCurrencyForInterface(m_cyNetCharges));
		return;
	}
	// m_cyApplyAmount = Payment to be applied
	m_cyApplyAmount = cy;

	if (m_shiftCheck.GetCheck())
		m_boShiftBalance = TRUE;
	else m_boShiftBalance = FALSE;

	if(m_RespCombo->GetCurSel() == -1)
		m_nShiftToResp = 0; //patient
	else
		m_nShiftToResp = VarLong(m_RespCombo->GetValue(m_RespCombo->GetCurSel(),rccID),0);

	if (m_adjustCheck.GetCheck())
		m_boAdjustBalance = TRUE;
	else m_boAdjustBalance = FALSE;

	CDialog::OnOK();	
}

HBRUSH CFinancialApply::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	//HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	switch(pWnd->GetDlgCtrlID())
	{
		case IDC_LABEL_RESPONSIBILITY:
		case IDC_LABEL_NET_CHARGES:
			//pDC->SetBkColor(0x9CC294);
			pDC->SetTextColor(0x800000);
			break;
		case IDC_LABEL4:
		case IDC_LABEL_AVAILABLE_APPLY:
			//pDC->SetBkColor(0x9CC294);
			pDC->SetTextColor(0x008000);
			break;
		//case IDC_LABEL1:
		case IDC_LABEL3:
		case IDC_LABEL14:
		case IDC_CURRENCY_SYMBOL_APPLY:
			//pDC->SetBkColor(0x9CC294);
			//return m_brush;
		default:
			break;
	}

	// (a.walling 2008-04-02 09:13) - PLID 29497 - Handle new NxColor's non-solid backgrounds
	HANDLE_GENERIC_TRANSPARENT_CTL_COLOR();
}

BOOL CFinancialApply::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 16:38) - PLID 29876 - NxIconify buttons
		m_btnApply.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnCalcPercent.AutoSet(NXB_MODIFY);
		
		m_brush.CreateSolidBrush(0x9CC294);

		CString strICurr;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRENCY, strICurr.GetBuffer(2), 2, TRUE);
		strICurr.ReleaseBuffer();
		strICurr.TrimRight();
		if(strICurr == "2") {
			SetDlgItemText(IDC_CURRENCY_SYMBOL_APPLY,GetCurrencySymbol() + " ");
		}
		else {
			SetDlgItemText(IDC_CURRENCY_SYMBOL_APPLY,GetCurrencySymbol());
		}

		m_RespCombo = BindNxDataListCtrl(this,IDC_RESP_COMBO,GetRemoteData(),false);
		CString str;
		str.Format("ID > 0 AND ID <> %li AND ID IN (SELECT RespTypeID FROM InsuredPartyT WHERE PatientID = %li)",m_nResponsibility,m_PatientID);
		m_RespCombo->PutWhereClause(_bstr_t(str));
		m_RespCombo->Requery();

		IRowSettingsPtr pRow;
		pRow = m_RespCombo->GetRow(-1);
		pRow->PutValue(rccID, long(0));
		pRow->PutValue(rccName, _bstr_t("Patient"));
		pRow->PutValue(rccPriority, long(0));
		pRow->PutValue(rccCategoryTypeID, g_cvarNull);
		pRow->PutValue(rccCategoryName, _bstr_t(""));
		m_RespCombo->AddRow(pRow);

		// (j.jones 2014-06-24 11:47) - PLID 60349 - moved this logic to a modular function
		long nNextRespTypeID = GetNextRespTypeByPriority(m_PatientID, m_nResponsibility);
		m_RespCombo->SetSelByColumn(rccID, nNextRespTypeID);

		if(m_RespCombo->CurSel == -1) {
			//select patient resp.
			m_RespCombo->SetSelByColumn(rccID,(long)0);
		}
	}
	NxCatchAll("Error in CFinancialApply::OnInitDialog");

	return TRUE;
}

void CFinancialApply::OnCheckAdjust() 
{
	// TODO: Add your control notification handler code here
	
}

void CFinancialApply::OnCheckIncrease() 
{
	CString str;
	if(!m_increaseCheck.GetCheck()) {
		m_cyApplyAmount = m_cyNetCharges;
		str = FormatCurrencyForInterface(m_cyNetCharges);
		GetDlgItem(IDC_EDIT_PAYMENT)->SetWindowText(str);
	}
	else {
		m_cyApplyAmount = m_cyNetPayment;
		str = FormatCurrencyForInterface(m_cyNetPayment);
		GetDlgItem(IDC_EDIT_PAYMENT)->SetWindowText(str);
	}	
}

void CFinancialApply::OnCheckShift() 
{
	// TODO: Add your control notification handler code here
	
}

void CFinancialApply::OnCalcPercent() 
{
	////////////////////////////////////////////////////////
	// Make sure apply amount is not blank, has no bad characters,
	// and has no more than two places to the right of the
	// decimal point.
	////////////////////////////////////////////////////////
	CString str;
	GetDlgItem(IDC_EDIT_PAYMENT)->GetWindowText(str);
	if (str.GetLength() == 0) {
		MsgBox("Please fill in the 'Amount To Apply' box.");
		return;
	}

	if(!IsValidCurrencyText(str)) {
		MsgBox("Please fill correct information in the 'Amount To Apply' box.");
		return;
	}

	COleCurrency cy = ParseCurrencyFromInterface(str);

	//see how much the regional settings allows to the right of the decimal
	CString strICurr;
	NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
	int nDigits = atoi(strICurr);
	CString strDecimal;
	NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
	if(cy.Format().Find(strDecimal) != -1 && cy.Format().Find(strDecimal) + (nDigits+1) < cy.Format().GetLength()) {
		MsgBox("Please fill only %li places to the right of the %s in the 'Amount To Apply' box.",nDigits,strDecimal == "," ? "comma" : "decimal");
		return;
	}

	CCalculatePercentageDlg dlg(this);
	dlg.m_color = 0x9CC294;
	//dlg.m_cyOriginalAmt = cy;
	dlg.m_strOriginalAmt = str;
	if(dlg.DoModal() == IDOK) {
		str = FormatCurrencyForInterface(dlg.m_cyFinalAmt,FALSE);
		SetDlgItemText(IDC_EDIT_PAYMENT,str);
		GetDlgItem(IDC_EDIT_PAYMENT)->SetFocus();
		if (dlg.m_cyFinalAmt >= COleCurrency(0,0)) {
			((CNxEdit*)GetDlgItem(IDC_EDIT_PAYMENT))->SetSel(0, -1);
		}
		else {
			int nStart = 0, nFinish = -1;
			GetNonNegativeAmountExtent(nStart, nFinish);
			((CNxEdit*)GetDlgItem(IDC_EDIT_PAYMENT))->SetSel(nStart, nFinish);
		}
	}
}

void CFinancialApply::GetNonNegativeAmountExtent(int &nStart, int &nFinish) {
	nStart = -1;
	nFinish = -1;
	int nCurrentChar = 0;
	CString strAmount;
	GetDlgItemText(IDC_EDIT_PAYMENT, strAmount);
	CString strValidChars = "1234567890";
	strValidChars += GetThousandsSeparator();
	strValidChars += GetDecimalSeparator();

	//Find the first character that is a number, thousands separator, or decimal separator.
	while(nCurrentChar < strAmount.GetLength() && nStart == -1) {
		if(strValidChars.Find(strAmount.Mid(nCurrentChar,1)) != -1) {
			nStart = nCurrentChar;
		}
		nCurrentChar++;
	}

	//Now, find the next character that is NOT a number, thousands, separator, or decimal separator.
	while(nCurrentChar < strAmount.GetLength() && nFinish == -1) {
		if(strValidChars.Find(strAmount.Mid(nCurrentChar,1)) == -1) {
			nFinish = nCurrentChar;
		}
		nCurrentChar++;
	}

	//Both are now filled in correctly (-1 is what we want if we couldn't find an appropriate position otherwise).
}
