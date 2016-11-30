// OpportunityAddDiscountDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "OpportunityAddDiscountDlg.h"
#include "InternationalUtils.h"
#include "OverrideUserDlg.h"
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

//DRT 5-6/2007 - PLID 25892

/////////////////////////////////////////////////////////////////////////////
// COpportunityAddDiscountDlg dialog


COpportunityAddDiscountDlg::COpportunityAddDiscountDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(COpportunityAddDiscountDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COpportunityAddDiscountDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_cySubTotal = COleCurrency(0, 0);
	m_cyFinalDiscount = COleCurrency(0, 0);
	m_nDiscountUserID = -1;
	m_dblMaxDiscountPercent = 0.0;
}


void COpportunityAddDiscountDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COpportunityAddDiscountDlg)
	DDX_Control(pDX, IDC_RAD_DOLLAR, m_btnDollar);
	DDX_Control(pDX, IDC_RAD_PERCENT, m_btnPercent);
	DDX_Control(pDX, IDC_LABEL_PERCENT_OFF, m_labelPercent);
	DDX_Control(pDX, IDC_LABEL_DOLLAR, m_labelDollar);
	DDX_Control(pDX, IDC_MAX_DISCOUNT_ALLOWED, m_labelMaxDiscount);
	DDX_Control(pDX, IDC_CURRENT_DISCOUNT_USER, m_labelUser);
	DDX_Control(pDX, IDC_DOLLAR_VALUE, m_nxeditDollarValue);
	DDX_Control(pDX, IDC_PERCENT_VALUE, m_nxeditPercentValue);
	DDX_Control(pDX, IDC_LABEL_SUBTOTAL, m_nxstaticLabelSubtotal);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COpportunityAddDiscountDlg, CNxDialog)
	//{{AFX_MSG_MAP(COpportunityAddDiscountDlg)
	ON_BN_CLICKED(IDC_RAD_PERCENT, OnRadPercent)
	ON_BN_CLICKED(IDC_RAD_DOLLAR, OnRadDollar)
	ON_EN_CHANGE(IDC_DOLLAR_VALUE, OnChangeDollarValue)
	ON_EN_KILLFOCUS(IDC_DOLLAR_VALUE, OnKillfocusDollarValue)
	ON_EN_CHANGE(IDC_PERCENT_VALUE, OnChangePercentValue)
	ON_EN_KILLFOCUS(IDC_PERCENT_VALUE, OnKillfocusPercentValue)
	ON_BN_CLICKED(IDC_OVERRIDE_DISCOUNT, OnOverrideDiscount)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpportunityAddDiscountDlg message handlers

BOOL COpportunityAddDiscountDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//Colorize!
		//TODO - While sales is still nitpicking over colors, this is loaded by a non-cached configrt
		//	proprty so I can change it on the fly.
		g_propManager.EnableCaching(FALSE);
		DWORD dwColor = GetRemotePropertyInt("InternalPropBGColor", 10542240, 0, "<None>", false);
		g_propManager.EnableCaching(TRUE);
		((CNxColor*)GetDlgItem(IDC_OPP_DISCOUNT_COLOR1))->SetColor(dwColor);

		m_labelPercent.SetColor(dwColor);
		m_labelDollar.SetColor(dwColor);
		m_labelMaxDiscount.SetColor(dwColor);
		m_labelUser.SetColor(dwColor);

		//Default to dollar, set the focus to the dollar value
		CheckDlgButton(IDC_RAD_DOLLAR, TRUE);
		GetDlgItem(IDC_DOLLAR_VALUE)->SetFocus();

		//Setup the current subtotal value for display
		SetDlgItemText(IDC_LABEL_SUBTOTAL, FormatCurrencyForInterface(m_cySubTotal));

		//determine the current user and the maximum percentage discount allowed
		m_nDiscountUserID = GetCurrentUserID();
		m_strDiscountUserName = GetCurrentUserName();
		_RecordsetPtr prsMax = CreateRecordset("SELECT MaximumPercentage FROM OpportunityMaxDiscountsT WHERE UserID = %li", m_nDiscountUserID);
		if(!prsMax->eof) {
			m_dblMaxDiscountPercent = AdoFldDouble(prsMax, "MaximumPercentage");
		}

		EnsureControls();

	} NxCatchAll("Error in OnInitDialog");

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COpportunityAddDiscountDlg::OnOK() 
{
	try {
		COleCurrency cyDiscount;
		if(IsDlgButtonChecked(IDC_RAD_DOLLAR)) {
			//Dollar currently chosen
			CString strDollar;
			GetDlgItemText(IDC_DOLLAR_VALUE, strDollar);

			//Parse string to currency and ensure it's valid
			cyDiscount.ParseCurrency(strDollar);
			if(cyDiscount.GetStatus() != COleCurrency::valid)
				cyDiscount = COleCurrency(0, 0);
		}
		else {
			//Percent currently chosen, but we already calculated the value when setting up the preview -- just nab
			//	it from there.
			CString strDollar = m_labelPercent.GetText();

			//Parse string to currency and ensure it's valid
			cyDiscount.ParseCurrency(strDollar);
			if(cyDiscount.GetStatus() != COleCurrency::valid)
				cyDiscount = COleCurrency(0, 0);
		}

		//Threw out my double comparison -- it was too imprecise.  Instead, we can simply take the total proposal cost
		//	and divide that by the max discount percent.  This gives us out maximum possible discount.  Then we can
		//	compare against that, to the cent, to ensure noone goes over.  
		//Note:  This may result in some slightly odd display issues -- for example, a $2,500.05 cent discount is 
		//	10.0002% off.  This will round and the display will show 10%, looking legit, but this function will refuse it.
		COleCurrency cyMaxDiscount = m_cySubTotal * (m_dblMaxDiscountPercent / 100);

		//The user may never pass the threshold for max discount percent.
		if(cyDiscount > cyMaxDiscount) {
			//Current discount is too large, we must fail.
			CString strMsg;
			strMsg.Format("You may not specify a discount larger than your maximum allowed (currently %.2f).  Please lower the discount amount.", m_dblMaxDiscountPercent);
			MessageBox(strMsg);
			return;
		}

		//Everything matches up, copy this to our "output" variable and quit
		m_cyFinalDiscount = cyDiscount;

		CDialog::OnOK();
	} NxCatchAll("Error in OnOK");
}

void COpportunityAddDiscountDlg::OnCancel() 
{
	try {


		CDialog::OnCancel();
	} NxCatchAll("Error in OnCancel");
}

void COpportunityAddDiscountDlg::OnRadPercent() 
{
	try {
		EnsureControls();

		//Set the focus to the value
		GetDlgItem(IDC_PERCENT_VALUE)->SetFocus();

	} NxCatchAll("Error in OnRadPercent");
}

void COpportunityAddDiscountDlg::OnRadDollar() 
{
	try {
		EnsureControls();

		//Set the focus to the value
		GetDlgItem(IDC_DOLLAR_VALUE)->SetFocus();
	} NxCatchAll("Error in OnRadDollar");
}

void COpportunityAddDiscountDlg::EnsureControls()
{
	BOOL bPercent = FALSE;
	UINT nShowPercent = SW_HIDE;
	UINT nShowDollar = SW_HIDE;

	if(IsDlgButtonChecked(IDC_RAD_PERCENT)) {
		bPercent = TRUE;
		nShowPercent = SW_SHOW;
		nShowDollar = SW_HIDE;
	}
	else {
		bPercent = FALSE;
		nShowPercent = SW_HIDE;
		nShowDollar = SW_SHOW;
	}

	//Do the work

	//Enable the one not checked
	GetDlgItem(IDC_PERCENT_VALUE)->EnableWindow(bPercent);
	GetDlgItem(IDC_DOLLAR_VALUE)->EnableWindow(!bPercent);

	//Hide the text of the one not checked
	ShowDlgItem(IDC_LABEL_PERCENT_OFF, nShowPercent);
	ShowDlgItem(IDC_LABEL_DOLLAR, nShowDollar);

	InvalidateDlgItem(IDC_LABEL_PERCENT_OFF);
	InvalidateDlgItem(IDC_LABEL_DOLLAR);

	m_labelUser.SetText(m_strDiscountUserName);
	m_labelUser.Invalidate();
	m_labelMaxDiscount.SetText(FormatString("%.2f%%", m_dblMaxDiscountPercent));
	m_labelMaxDiscount.Invalidate();
}

//Updates the interface when the dollar value changes.  If the values are valid, sets the dblPercent
//	parameter to the current percentage (to 2 digits).  If the values are not valid, returns false.
bool COpportunityAddDiscountDlg::HandleDollarChange(double &dblPercent)
{
	//Just in case, make sure we have a subtotal
	if(m_cySubTotal == COleCurrency(0, 0)) 
		return false;

	CString str;
	GetDlgItemText(IDC_DOLLAR_VALUE, str);

	//convert to currency
	COleCurrency cy;
	cy.ParseCurrency(str);

	if(cy.GetStatus() != COleCurrency::valid)
		//We could be in the middle of changing the value, so it's possible this is invalid.  Just quit
		return false;

	//It is valid, so determine what percentage this would be.  Note that the precision here isn't
	//	that great, but good enough for our results.
	__int64 cy1, cy2;
	cy1 = cy.m_cur.int64;
	cy2 = m_cySubTotal.m_cur.int64;
	double dbl = (double)cy1 / (double)cy2;

	//For display, multiply by 100
	{
		double dblDisplay = dbl * 100.0;

		str.Format("%.2f", dblDisplay);
		m_labelDollar.SetText(str);
		InvalidateDlgItem(IDC_LABEL_DOLLAR);
	}

	//change it back to a double so we are set to 2 digits
	dblPercent = atof(str);

	return true;
}

void COpportunityAddDiscountDlg::OnChangeDollarValue() 
{
	try {
		//This will update the interface
		double dbl = 0.0;
		HandleDollarChange(dbl);

		//for the en_changed, we don't care whether it worked or not.

	} NxCatchAll("Eror in OnChangeDollarValue");
}

void COpportunityAddDiscountDlg::OnKillfocusDollarValue() 
{
	try {
		//Format the whole thing correctly
		CString str;
		GetDlgItemText(IDC_DOLLAR_VALUE, str);

		COleCurrency cy;
		cy.ParseCurrency(str);

		if(cy.GetStatus() != COleCurrency::valid)
			cy = COleCurrency(0, 0);

		SetDlgItemText(IDC_DOLLAR_VALUE, FormatCurrencyForInterface(cy));

	} NxCatchAll("Error in OnKillfocusDollarValue");
}

void COpportunityAddDiscountDlg::OnChangePercentValue() 
{
	try {
		CString str;
		GetDlgItemText(IDC_PERCENT_VALUE, str);
		double dbl = atof(str);

		//Divide by 100 to make a percent (for example they will enter "10", we want ".1")
		dbl /= 100.0;

		//Multiply the subtotal by our percentage
		COleCurrency cy = m_cySubTotal * dbl;

		//Slap this value in the preview
		m_labelPercent.SetText(FormatCurrencyForInterface(cy));
		InvalidateDlgItem(IDC_LABEL_PERCENT_OFF);

	} NxCatchAll("Error in OnChangePercentValue");
}

void COpportunityAddDiscountDlg::OnKillfocusPercentValue() 
{
	try {

	} NxCatchAll("Error in OnKillfocusPercentValue");
}

void COpportunityAddDiscountDlg::OnOverrideDiscount() 
{
	try {
		COverrideUserDlg dlg(this);
		//We only want to show users that have max discounts setup, may not be all users.
		dlg.m_strOverrideWhereClause = "Archived = 0 AND UsersT.PersonID IN (SELECT UserID FROM OpportunityMaxDiscountsT)";
		if(dlg.DoModal() == IDOK) {
			_RecordsetPtr prsMax = CreateRecordset("SELECT MaximumPercentage FROM OpportunityMaxDiscountsT WHERE UserID = %li", dlg.m_nApprovedUserID);
			if(!prsMax->eof) {
				//We got the user, set our interface and members appropriately
				m_dblMaxDiscountPercent = AdoFldDouble(prsMax, "MaximumPercentage");
				m_nDiscountUserID = dlg.m_nApprovedUserID;
				m_strDiscountUserName = dlg.m_strApprovedUserName;

				//Makes sure the names are updated on screen
				EnsureControls();
			}
		}

	} NxCatchAll("Error on OnOverrideDiscount");
}