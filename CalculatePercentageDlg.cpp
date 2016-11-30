// CalculatePercentageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GlobalFinancialUtils.h"
#include "CalculatePercentageDlg.h"
#include "InternationalUtils.h"
#include "GlobalDrawingUtils.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCalculatePercentageDlg dialog


CCalculatePercentageDlg::CCalculatePercentageDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCalculatePercentageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCalculatePercentageDlg)
		m_cyOriginalAmt = COleCurrency(0,0);
		m_strOriginalAmt = "";
		m_cyFinalAmt = COleCurrency(0,0);
		m_dblPercentage = 0.0;
	//}}AFX_DATA_INIT
}


void CCalculatePercentageDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCalculatePercentageDlg)
	DDX_Control(pDX, IDC_CALC_PERCENT_BKG, m_bkg);
	DDX_Control(pDX, IDC_ORIGINAL_AMOUNT, m_nxeditOriginalAmount);
	DDX_Control(pDX, IDC_EDIT_PERCENTAGE, m_nxeditEditPercentage);
	DDX_Control(pDX, IDC_EDIT_CALCULATED_AMOUNT, m_nxeditEditCalculatedAmount);
	DDX_Control(pDX, IDC_LABEL_ORIGINAL_AMT, m_nxstaticLabelOriginalAmt);
	DDX_Control(pDX, IDC_CURRENCY_LABEL1, m_nxstaticCurrencyLabel1);
	DDX_Control(pDX, IDC_LABEL_PERCENT, m_nxstaticLabelPercent);
	DDX_Control(pDX, IDC_PERCENT, m_nxstaticPercent);
	DDX_Control(pDX, IDC_CALCULATED_TOTAL, m_nxstaticCalculatedTotal);
	DDX_Control(pDX, IDC_CURRENCY_LABEL2, m_nxstaticCurrencyLabel2);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CALCULATE, m_btnCalculate);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCalculatePercentageDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCalculatePercentageDlg)
	ON_BN_CLICKED(IDC_CALCULATE, OnCalculate)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCalculatePercentageDlg message handlers

BOOL CCalculatePercentageDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (c.haag 2008-04-23 14:58) - PLID 29761 - NxIconify the buttons
	m_btnOK.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	m_btnCalculate.AutoSet(NXB_MODIFY);

	m_bkg.SetColor(m_color);
	
	m_bg.CreateSolidBrush(PaletteColor(m_color));

	CString strICurr;
	NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRENCY, strICurr.GetBuffer(2), 2, TRUE);
	strICurr.ReleaseBuffer();
	strICurr.TrimRight();
	if(strICurr == "2") {
		SetDlgItemText(IDC_CURRENCY_LABEL1,GetCurrencySymbol() + " ");
		SetDlgItemText(IDC_CURRENCY_LABEL2,GetCurrencySymbol() + " ");
	}
	else {
		SetDlgItemText(IDC_CURRENCY_LABEL1,GetCurrencySymbol());
		SetDlgItemText(IDC_CURRENCY_LABEL2,GetCurrencySymbol());
	}
	
	//load the given amount
	//CString str = FormatCurrencyForInterface(m_cyOriginalAmt, FALSE);
	CString str = m_strOriginalAmt;
	SetDlgItemText(IDC_ORIGINAL_AMOUNT,str);

	//load previous percentage
	float percent = 0.0;
	str.Format("%0.09g",GetRemotePropertyFloat("PaymentPercentage",&percent,0,"<None>",TRUE));
	SetDlgItemText(IDC_EDIT_PERCENTAGE,str);
	m_dblPercentage = (double)percent;

	OnCalculate();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCalculatePercentageDlg::OnCalculate() 
{
	//first get the original amount

	CString strOriginalAmount;
	double dblOriginalAmt = 0.0;	

	GetDlgItem(IDC_ORIGINAL_AMOUNT)->GetWindowText(strOriginalAmount);
	if (strOriginalAmount.GetLength() == 0) {
		MsgBox("Please fill in the 'Original Amount' box.");
		return;
	}
	COleCurrency cyTmp = ParseCurrencyFromInterface(strOriginalAmount);
	if(cyTmp.GetStatus() == COleCurrency::invalid) {
		MsgBox("Please enter a valid currency in the 'Original Amount' box.");
		return;
	}
	else {
		m_cyOriginalAmt = cyTmp;
		// (z.manning, 05/21/2008) - PLID 30128 - Convert the currency to double to use later
		// when calcuating the percent.
		dblOriginalAmt = cyTmp.m_cur.int64 / 10000.;
	}

	//JMJ 3/8/2004 - PLID 11324 - this is a simple fix so if they are making an adjustment and
	//the amount is (0.00) they will still have (0.00) in the percent screen
	BOOL bNegativeZero = FALSE;
	if(strOriginalAmount.Find("(") != -1 && m_cyOriginalAmt == COleCurrency(0,0)) {
		bNegativeZero = TRUE;
	}

	//next get the Percentage

	CString strPercentage;

	GetDlgItem(IDC_EDIT_PERCENTAGE)->GetWindowText(strPercentage);
	if (strPercentage.GetLength() == 0) {
		MsgBox("Please fill in the 'Percentage' box.");
		return;
	}
	for(int i = 0; i<strPercentage.GetLength(); i++) {
		if((strPercentage.GetAt(i)<'0' || strPercentage.GetAt(i)>'9') &&
			strPercentage.GetAt(i)!= '.' && strPercentage.GetAt(i)!='%' && strPercentage.GetAt(i)!='-' &&
			strPercentage.Replace(".",".") <= 1) {
			MsgBox("Please fill correct information in the 'Percentage' box.");
			return;
		}
	}

	m_dblPercentage = atof(strPercentage);
	if(m_dblPercentage > 9999.9999) {
		MsgBox("Please enter a percentage that is less than 10,000");
		return;
	}
	else if((double)((int)(m_dblPercentage * 10000)) != (m_dblPercentage*10000)) {
		MsgBox("Please enter no more than four decimal places in the percentage field");
		return;
	}

	// (z.manning, 05/21/2008) - PLID 30128 - The old way of doing this does not work. If you read
	// the MSDN documentation on COleCurrency you can only multiply/divide a COleCurrency by an
	// integral number. Thus our previous calculation was producing unpredictable results in certain
	// situations.
	//m_cyFinalAmt = m_cyOriginalAmt * (m_dblPercentage / 100);
	double dblFinalAmt = dblOriginalAmt * (m_dblPercentage / 100.);
	// (z.manning, 05/21/2008) - PLID 30128 - If our resulting value is not within long's valid range
	// then our call to SetCurrency later will have a invalid value.
	if(dblFinalAmt > LONG_MAX || dblFinalAmt < LONG_MIN) {
		MessageBox("The result of the calculation is out of range.");
		SetDlgItemText(IDC_EDIT_CALCULATED_AMOUNT, FormatCurrencyForInterface(COleCurrency(0,0), FALSE));
		return;
	}
	// (z.manning, 05/21/2008) - PLID 30128 - Get the integer part of our result and use it to
	// set the currency result.
	long nFinalAmt = (long)dblFinalAmt;
	m_cyFinalAmt.SetCurrency(nFinalAmt, (long)((dblFinalAmt - (double)nFinalAmt) * 10000));
	
	RoundCurrency(m_cyFinalAmt);

	CString strNewAmt = FormatCurrencyForInterface(m_cyFinalAmt, FALSE);

	//JMJ 3/8/2004 - PLID 11324 - this is a simple fix so if they are making an adjustment and
	//the amount is (0.00) they will still have (0.00) in the percent screen
	if(bNegativeZero && m_cyFinalAmt == COleCurrency(0,0)) {
		//keep international settings
		CString str = FormatCurrencyForInterface(COleCurrency(-1,0),FALSE);
		str.Replace("1","0");
		SetDlgItemText(IDC_EDIT_CALCULATED_AMOUNT,str);
		int nStart, nFinish;
		GetNonNegativeAmountExtent(nStart, nFinish);
		((CNxEdit*)GetDlgItem(IDC_EDIT_CALCULATED_AMOUNT))->SetSel(nStart, nFinish);
	}
	else {
		SetDlgItemText(IDC_EDIT_CALCULATED_AMOUNT,strNewAmt);
		((CNxEdit*)GetDlgItem(IDC_EDIT_CALCULATED_AMOUNT))->SetSel(0, -1);
	}	
}

void CCalculatePercentageDlg::OnOK() 
{
	//get the Total Amount

	CString strTotalAmount;

	GetDlgItem(IDC_EDIT_CALCULATED_AMOUNT)->GetWindowText(strTotalAmount);
	if (strTotalAmount.GetLength() == 0) {
		MsgBox("Please fill in the 'New Amount' box.");
		return;
	}

	if(!IsValidCurrencyText(strTotalAmount)) {
		MsgBox("Please fill correct information in the 'New Amount' box.");
		return;
	}

	m_cyFinalAmt = ParseCurrencyFromInterface(strTotalAmount);

	//save the percentage for future use (if it's valid).
	if(m_dblPercentage < 10000 && (double)((int)(m_dblPercentage * 10000)) != (m_dblPercentage*10000) ) {
		SetRemotePropertyFloat("PaymentPercentage",(float)m_dblPercentage,0,"<None>");
	}	
	
	CDialog::OnOK();
}

void CCalculatePercentageDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	GetDlgItem(IDC_EDIT_PERCENTAGE)->SetFocus();
	((CNxEdit*)GetDlgItem(IDC_EDIT_PERCENTAGE))->SetSel(0, -1);
}

void CCalculatePercentageDlg::GetNonNegativeAmountExtent(int &nStart, int &nFinish) {
	nStart = -1;
	nFinish = -1;
	int nCurrentChar = 0;
	CString strAmount;
	GetDlgItemText(IDC_EDIT_CALCULATED_AMOUNT, strAmount);
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
