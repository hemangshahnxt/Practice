// UserDrawerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UserDrawerDlg.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"
#include "AuditTrail.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



//also used in EditDrawersDlg.cpp
#define IDPREVIEW  17895

/////////////////////////////////////////////////////////////////////////////
// CUserDrawerDlg dialog


CUserDrawerDlg::CUserDrawerDlg(CWnd* pParent)
	: CNxDialog(CUserDrawerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUserDrawerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_strName = "";
	m_cyOpenAmt = m_cyCash = m_cyCheck = m_cyCharge = m_cyGift = COleCurrency(0, 0);
	m_cyUserCash = m_cyUserCheck = m_cyUserCharge = m_cyUserGift = COleCurrency(0, 0);
	m_nID = -1;
	m_bEditing = FALSE;
}


void CUserDrawerDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUserDrawerDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_SYS_CASH, m_nxeditSysCash);
	DDX_Control(pDX, IDC_SYS_CHECK, m_nxeditSysCheck);
	DDX_Control(pDX, IDC_SYS_CHARGE, m_nxeditSysCharge);
	DDX_Control(pDX, IDC_SYS_GIFT, m_nxeditSysGift);
	DDX_Control(pDX, IDC_SYS_TOTAL, m_nxeditSysTotal);
	DDX_Control(pDX, IDC_USER_CASH, m_nxeditUserCash);
	DDX_Control(pDX, IDC_USER_CHECK, m_nxeditUserCheck);
	DDX_Control(pDX, IDC_USER_CHARGE, m_nxeditUserCharge);
	DDX_Control(pDX, IDC_USER_GIFT, m_nxeditUserGift);
	DDX_Control(pDX, IDC_USER_TOTAL, m_nxeditUserTotal);
	DDX_Control(pDX, IDC_DRAWER_NAME_LABEL, m_nxstaticDrawerNameLabel);
	DDX_Control(pDX, IDC_OPEN_AMT_LABEL, m_nxstaticOpenAmtLabel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_PREVIEW_DRAWER, m_btnPreviewDrawer);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUserDrawerDlg, CNxDialog)
	//{{AFX_MSG_MAP(CUserDrawerDlg)
	ON_EN_KILLFOCUS(IDC_USER_CASH, OnKillfocusUserCash)
	ON_EN_KILLFOCUS(IDC_USER_CHECK, OnKillfocusUserCheck)
	ON_EN_KILLFOCUS(IDC_USER_CHARGE, OnKillfocusUserCharge)
	ON_EN_KILLFOCUS(IDC_USER_GIFT, OnKillfocusUserGift)
	ON_BN_CLICKED(IDC_PREVIEW_DRAWER, OnPreviewDrawer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUserDrawerDlg message handlers

#define SET_CURRENCY(idc, cur) SetDlgItemText(idc, FormatCurrencyForInterface(cur));

BOOL CUserDrawerDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	if(m_nID == -1) {
		//no drawer, we can't operate under these conditions!
		MsgBox("Invalid ID to close!");
		CDialog::OnCancel();
		return TRUE;
	}

	//load the data for this drawer
	try {
		// (c.haag 2008-05-02 10:15) - PLID 29879 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnPreviewDrawer.AutoSet(NXB_PRINT_PREV);

		//check to see how much money has been put into this drawer.
		//Updated 4/26/2004 to include tips
		//DRT 2/9/2005 - PLID 15566 - Changed query to handle Amount Received & Change Given calculations.  Note that
		//	the 'CashReceived' field isn't just cash, it's the "Amount Received" on the payment dialog.
		//DRT 6/29/2005 - PLID 16010 - Made changes to allow refunds of check and charge type.
		//TES 4/16/2015 - PLID 65615 - Support refunded tips in this calculation
		// (r.gonet 2015-05-01 10:32) - PLID 65748 - Updated to support refunded tips on gift certificate refunds.
		_RecordsetPtr prs = CreateRecordset(
			"SELECT SUM(TotalCash) AS TotalCash, SUM(TotalCheck) AS TotalCheck,  "
			"SUM(TotalCharge) AS TotalCharge, SUM(TotalGift) AS TotalGift FROM  "
			"	/* Select the CashReceived field for each payment.  This will total us the amount "
			"	   they received, not counting any change returned.*/ "
			"	(SELECT SUM(CASE WHEN PayMethod = 1 THEN CashReceived WHEN PayMethod = 7 THEN Amount ELSE 0 END) AS TotalCash,  "
			"	SUM(CASE WHEN PayMethod = 2 THEN CashReceived WHEN PayMethod = 8 THEN Amount ELSE 0 END) AS TotalCheck,  "
			"	SUM(CASE WHEN PayMethod = 3 THEN CashReceived WHEN PayMethod = 9 THEN Amount ELSE 0 END) AS TotalCharge,  "
			"	SUM(CASE WHEN PayMethod IN (4,10) THEN LineItemT.Amount ELSE 0 END) AS TotalGift  "
			"	FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
			"	WHERE Deleted = 0 AND DrawerID = %li "
			" "
			"	UNION  "
			"	/* Tips! */ "
			"	SELECT SUM(CASE WHEN PayMethod IN (1,7) THEN PaymentTipsT.Amount ELSE 0 END) AS TotalCash,  "
			"	SUM(CASE WHEN PayMethod IN (2,8) THEN PaymentTipsT.Amount ELSE 0 END) AS TotalCheck,  "
			"	SUM(CASE WHEN PayMethod IN (3,9) THEN PaymentTipsT.Amount ELSE 0 END) AS TotalCharge,  "
			"	SUM(CASE WHEN PayMethod IN (4,10) THEN PaymentTipsT.Amount ELSE 0 END) AS TotalGift  "
			"	FROM PaymentTipsT INNER JOIN LineItemT ON PaymentTipsT.PaymentID = LineItemT.ID  "
			"	WHERE LineItemT.Deleted = 0 AND PaymentTipsT.DrawerID = %li "
			" "
			"	UNION "
			"	/* Change given.  All change is cash.  Select the CashReceived column minus the amount of the charge - any difference "
			"	   is change that was returned.  This amount is negative for the sum in the main query.  */ "
			"	SELECT SUM(CASE WHEN PayMethod = 1 OR PayMethod = 2 OR PayMethod = 3 THEN Amount - CashReceived ELSE 0 END) AS TotalCash,  "
			"	0 AS TotalCheck,  "
			"	0 AS TotalCharge,  "
			"	0 AS TotalGift  "
			"	FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
			"	WHERE Deleted = 0 AND DrawerID = %li "
			"	) SubQ", m_nID, m_nID, m_nID);

		if(!prs->eof) {
			m_cyCash = AdoFldCurrency(prs, "TotalCash", COleCurrency(0, 0));
			m_cyCheck = AdoFldCurrency(prs, "TotalCheck", COleCurrency(0, 0));
			m_cyCharge = AdoFldCurrency(prs, "TotalCharge", COleCurrency(0, 0));
			m_cyGift = AdoFldCurrency(prs, "TotalGift", COleCurrency(0, 0));
		}

		//also need some other values
		prs = CreateRecordset("SELECT Name, OpenAmt, DateClosed, CloseCash, CloseCheck, CloseCharge, CloseGift FROM CashDrawersT WHERE ID = %li", m_nID);
		if(!prs->eof) {
			m_strName = AdoFldString(prs, "Name", "");
			m_cyOpenAmt = AdoFldCurrency(prs, "OpenAmt", COleCurrency(0, 0));

			_variant_t var = prs->Fields->Item["DateClosed"]->Value;
			if(var.vt == VT_DATE) {
				m_bEditing = TRUE;

				//also fill in the values they already entered for the user amts
				m_cyUserCash = AdoFldCurrency(prs, "CloseCash", COleCurrency(0, 0));
				m_cyUserCheck = AdoFldCurrency(prs, "CloseCheck", COleCurrency(0, 0));
				m_cyUserCharge = AdoFldCurrency(prs, "CloseCharge", COleCurrency(0, 0));
				m_cyUserGift = AdoFldCurrency(prs, "CloseGift", COleCurrency(0, 0));

			}
			else
				m_bEditing = FALSE;
		}

	} NxCatchAll("Error loading drawer.");

	//fill in the values to the boxes
	SetDlgItemText(IDC_DRAWER_NAME_LABEL, m_strName);
	SET_CURRENCY(IDC_OPEN_AMT_LABEL, m_cyOpenAmt);

	SET_CURRENCY(IDC_SYS_CASH, m_cyCash + m_cyOpenAmt);		//the opening amount is cash, so add that in so they match up correctly
	SET_CURRENCY(IDC_SYS_CHECK, m_cyCheck);
	SET_CURRENCY(IDC_SYS_CHARGE, m_cyCharge);
	SET_CURRENCY(IDC_SYS_GIFT, m_cyGift);
	SET_CURRENCY(IDC_SYS_TOTAL, m_cyCash + m_cyCheck + m_cyCharge + m_cyGift + m_cyOpenAmt);

	SET_CURRENCY(IDC_USER_CASH, m_cyUserCash);
	SET_CURRENCY(IDC_USER_CHECK, m_cyUserCheck);
	SET_CURRENCY(IDC_USER_CHARGE, m_cyUserCharge);
	SET_CURRENCY(IDC_USER_GIFT, m_cyUserGift);

	UpdateUserTotal();

	//put focus in the user cash box and highlight it
	CNxEdit* pWnd = (CNxEdit*)GetDlgItem(IDC_USER_CASH);
	pWnd->SetFocus();
	pWnd->SetSel(0, -1);

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

COleCurrency CUserDrawerDlg::GetUserCash()
{
	return m_cyUserCash;
}

COleCurrency CUserDrawerDlg::GetUserCheck()
{
	return m_cyUserCheck;
}

COleCurrency CUserDrawerDlg::GetUserCharge()
{
	return m_cyUserCharge;
}

COleCurrency CUserDrawerDlg::GetUserGift()
{
	return m_cyUserGift;
}

void CUserDrawerDlg::UpdateUserTotal()
{
	COleCurrency cy;
	cy = m_cyUserCash + m_cyUserCheck + m_cyUserCharge + m_cyUserGift;

	SET_CURRENCY(IDC_USER_TOTAL, cy);
}

void CUserDrawerDlg::OnKillfocusUserCash() 
{
	//ensure that this currency is valid, if it is not, we have to keep focus here until it is
	CString str;
	GetDlgItemText(IDC_USER_CASH, str);
	if(str.IsEmpty()) {
		COleCurrency cy(0, 0);
		str = FormatCurrencyForInterface(cy);
		SetDlgItemText(IDC_USER_CASH, str);
	}

	if(!IsValidCurrencyText(str)) {
		m_cyUserCash.SetStatus(COleCurrency::invalid);
		return;
	}

	m_cyUserCash = ParseCurrencyFromInterface(str);
	SET_CURRENCY(IDC_USER_CASH, m_cyUserCash);
	UpdateUserTotal();
}

void CUserDrawerDlg::OnKillfocusUserCheck() 
{
	//ensure that this currency is valid, if it is not, we have to keep focus here until it is
	CString str;
	GetDlgItemText(IDC_USER_CHECK, str);
	if(str.IsEmpty()) {
		COleCurrency cy(0, 0);
		str = FormatCurrencyForInterface(cy);
		SetDlgItemText(IDC_USER_CHECK, str);
	}

	if(!IsValidCurrencyText(str)) {
		m_cyUserCheck.SetStatus(COleCurrency::invalid);
		return;
	}

	m_cyUserCheck = ParseCurrencyFromInterface(str);
	SET_CURRENCY(IDC_USER_CHECK, m_cyUserCheck);
	UpdateUserTotal();
}

void CUserDrawerDlg::OnKillfocusUserCharge() 
{
	//ensure that this currency is valid, if it is not, we have to keep focus here until it is
	CString str;
	GetDlgItemText(IDC_USER_CHARGE, str);
	if(str.IsEmpty()) {
		COleCurrency cy(0, 0);
		str = FormatCurrencyForInterface(cy);
		SetDlgItemText(IDC_USER_CHARGE, str);
	}

	if(!IsValidCurrencyText(str)) {
		m_cyUserCharge.SetStatus(COleCurrency::invalid);
		return;
	}

	m_cyUserCharge = ParseCurrencyFromInterface(str);
	SET_CURRENCY(IDC_USER_CHARGE, m_cyUserCharge);
	UpdateUserTotal();
}

void CUserDrawerDlg::OnKillfocusUserGift() 
{
	//ensure that this currency is valid, if it is not, we have to keep focus here until it is
	CString str;
	GetDlgItemText(IDC_USER_GIFT, str);
	if(str.IsEmpty()) {
		COleCurrency cy(0, 0);
		str = FormatCurrencyForInterface(cy);
		SetDlgItemText(IDC_USER_GIFT, str);
	}

	if(!IsValidCurrencyText(str)) {
		m_cyUserGift.SetStatus(COleCurrency::invalid);
		return;
	}

	m_cyUserGift = ParseCurrencyFromInterface(str);
	SET_CURRENCY(IDC_USER_GIFT, m_cyUserGift);
	UpdateUserTotal();
}


BOOL CUserDrawerDlg::Save() {

	//it's possible, if you click in a field, change it, and just press enter, that
	//the total won't get updated correctly.
	OnKillfocusUserCash();
	OnKillfocusUserCheck();
	OnKillfocusUserCharge();
	OnKillfocusUserGift();

	//if any of our currencies are invalid, we can't save
	if(m_cyUserCash.GetStatus() == COleCurrency::invalid || m_cyUserCheck.GetStatus() == COleCurrency::invalid || m_cyUserCharge.GetStatus() == COleCurrency::invalid || m_cyUserGift.GetStatus() == COleCurrency::invalid) {
		MsgBox("You have an invalid currency value.  Please fix this before attempting to save.");
		return FALSE;
	}

	//if the totals don't match up, make sure to warn them
	CString strSys, strUser;
	GetDlgItemText(IDC_SYS_TOTAL, strSys);
	GetDlgItemText(IDC_USER_TOTAL, strUser);

	if(strSys.CompareNoCase(strUser)) {
		if(MsgBox(MB_YESNO, "Your totals do not match.  Are you sure you wish to continue?\r\n"
			" - If you choose YES, this data will be saved and available to view on reports.\r\n"
			" - If you choose NO, you will be returned to the dialog to correct your numbers.") != IDYES)
			return FALSE;
	}

	try {
		//Save it!
		COleDateTime dt = COleDateTime::GetCurrentTime();

		//update the data to the new values.  If they are editing, this WILL NOT change the close date
		CString strDate = "";
		if(!m_bEditing) {
			strDate.Format("DateClosed = '%s',", FormatDateTimeForSql(dt));
		}

		ExecuteSql("UPDATE CashDrawersT SET %s "
			"CloseCash = convert(money, '%s'), "
			"CloseCheck = convert(money, '%s'), "
			"CloseCharge = convert(money, '%s'), "
			"CloseGift = convert(money, '%s') "
			"WHERE ID = %li", strDate, 
			FormatCurrencyForSql(m_cyUserCash), FormatCurrencyForSql(m_cyUserCheck), FormatCurrencyForSql(m_cyUserCharge), FormatCurrencyForSql(m_cyUserGift), m_nID);

		//Audit!
		// (j.anspach 05-24-2005 15:33 PLID 15685) - Make it audit in Financial (not Misc.)
		long nItem, nPriority, nType;
		CString strOld, strNew;
		if(m_bEditing) {
			//editing existing and changing values
			nItem = aeiCashDrawerEdit;
			nPriority = aepHigh;
			nType = aetChanged;
			strOld = "Closed";
			strNew.Format("Edited:  Cash = %s, Check = %s, Charge = %s, Gift Cert. = %s ",
				FormatCurrencyForInterface(m_cyUserCash), FormatCurrencyForInterface(m_cyUserCheck), FormatCurrencyForInterface(m_cyUserCharge), FormatCurrencyForInterface(m_cyUserGift));
		}
		else {
			//not editing, this is a new save
			nItem = aeiCashDrawerClose;
			nPriority = aepMedium;
			nType = aetCreated;
			strOld = "Open";
			strNew.Format("Closed:  Cash = %s, Check = %s, Charge = %s, Gift Cert. = %s ",
				FormatCurrencyForInterface(m_cyUserCash), FormatCurrencyForInterface(m_cyUserCheck), FormatCurrencyForInterface(m_cyUserCharge), FormatCurrencyForInterface(m_cyUserGift));
		}
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, m_strName, nAuditID, nItem, m_nID, strOld, strNew, nPriority, nType);

	} NxCatchAllCall("Error saving drawer.", return FALSE);

	//we are good! 
	return TRUE;
}



void CUserDrawerDlg::OnOK() 
{
	if (!Save()) {
		return;
	}		

	CDialog::OnOK();
}

void CUserDrawerDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CUserDrawerDlg::OnPreviewDrawer() 
{
	
	if (IDOK != MsgBox(MB_OKCANCEL, "This will save and close this cash drawer.")) {
		return;
	}

	if (!Save()) {
		return;
	}

	EndDialog(IDPREVIEW);

	CReportInfo infReport = CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(501)];

	//steal the extra values field for the procedure number
	infReport.AddExtraValue(AsString(m_nID));
	infReport.bExtended = TRUE;

	//run the dltd
	infReport.strReportFile += "dtld";

	//Set up the parameters.
	CPtrArray paParams;
	CRParameterInfo *paramInfo;
	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = "01/01/1000";
	paramInfo->m_Name = "DateFrom";
	paParams.Add(paramInfo);
	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = "12/31/5000";
	paramInfo->m_Name = "DateTo";
	paParams.Add(paramInfo);



	//now just run the report
	RunReport(&infReport, &paParams, true, (CWnd *)this, "NexForms Content");
	ClearRPIParameterList(&paParams);	//DRT - PLID 18085 - Cleanup after ourselves
	
		
}
