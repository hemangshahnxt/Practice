// ApplyToRefundDlg.cpp : implementation file
//

#include "stdafx.h"
#include "billingrc.h"
#include "ApplyToRefundDlg.h"
#include "GlobalFinancialUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CApplyToRefundDlg dialog


CApplyToRefundDlg::CApplyToRefundDlg(CWnd* pParent)
	: CNxDialog(CApplyToRefundDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CApplyToRefundDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nPayID = -1;
	m_cyPayAmount = COleCurrency(0,0);
	m_nPatientID = -1;
}


void CApplyToRefundDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CApplyToRefundDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CApplyToRefundDlg, CNxDialog)
	//{{AFX_MSG_MAP(CApplyToRefundDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CApplyToRefundDlg message handlers

BEGIN_EVENTSINK_MAP(CApplyToRefundDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CApplyToRefundDlg)
	ON_EVENT(CApplyToRefundDlg, IDC_AVAL_PAYS_LIST, 3 /* DblClickCell */, OnDblClickCellAvalPaysList, VTS_DISPATCH VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()



void CApplyToRefundDlg::OnCancel()  {

	try {
		// (a.walling 2007-08-03 09:46) - PLID 26899 - Check for license
		// (j.gruber 2008-02-12 12:13) - PLID 28776 - need a return so we don't get the CC processing message box if we don't have it
		// (d.thompson 2010-09-02) - PLID 40371 - Any cc license satisfies this
		if (!g_pLicense || !g_pLicense->HasCreditCardProc_Any(CLicense::cflrUse)) {
			CNxDialog::OnCancel();
			return;
		}
		
		// (j.jones 2015-10-08 11:12) - PLID 67170 - don't warn if ICCP is enabled
		if (IsICCPEnabled() || IDYES == MsgBox(MB_YESNO, "In order to automatically process this refund via credit card processing, you must choose the payment you are refunding.\nAre you sure you want to continue without choosing a payment to refund?")) {
			//they want out, let 'em
			CNxDialog::OnCancel();
			return;
		}
		else {
			return;
		}
	}NxCatchAll("Error in CApplyToRefundDlg::OnCancel");
}


void CApplyToRefundDlg::OnOK() 
{
	try { 
		NXDATALIST2Lib::IRowSettingsPtr pRow;

		pRow = m_pPayList->CurSel;

		//get the ID
		if (pRow) {

			// (j.jones 2015-09-30 10:34) - PLID 67171 - before selecting this payment, check to see if
			// ICCP is enabled and if this was an old Chase/Intuit processed card
			long nPaymentID = VarLong(pRow->GetValue(0));

			//this is a global function - it just opens an IDOK messagebox, not yes/no,
			//so we do not need to check a return value
			CheckWarnCreditCardPaymentRefundMismatch(nPaymentID);

			//now fill our member variables and return
			m_nPayID = nPaymentID;
			m_cyPayAmount = VarCurrency(pRow->GetValue(4));
			
			CNxDialog::OnOK();
		}
		else {
			// (a.walling 2007-08-03 09:46) - PLID 26899 - Check for license
			// (d.thompson 2010-09-02) - PLID 40371 - Any credit card license satisfies this
			if (!g_pLicense || !g_pLicense->HasCreditCardProc_Any(CLicense::cflrUse)) {
				CNxDialog::OnCancel();
				return;
			}

			//there isn't a row, pop up our warning
			// (j.jones 2015-10-08 11:12) - PLID 67170 - don't warn if ICCP is enabled
			if (IsICCPEnabled() || IDYES == MsgBox(MB_YESNO, "In order to automatically process this refund via credit card processing, you must choose the payment you are refunding.\nAre you sure you want to continue without choosing a payment to refund?")) {
				//they want out, let 'em
				CNxDialog::OnCancel();
				return;
			}
			else {
				return;
			}
		}
	}NxCatchAll("Error in CApplyToRefundDlg::OnOK() ");
}

BOOL CApplyToRefundDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-23 15:57) - PLID 29761 - NxIconified the OK button
		m_btnOK.AutoSet(NXB_OK);
		
		// (a.walling 2007-11-13 09:24) - PLID 28059 - Bad Bind; this is a DL2.
		m_pPayList = BindNxDataList2Ctrl(IDC_AVAIL_PAYS_LIST, GetRemoteData(), FALSE);

		// (j.dinatale 2011-09-09 09:34) - PLID 45399 - Moved the from clause from resources to here. We need to filter out all the
		//		voiding and original line items from corrections because corrections are now reverse payments instead of adjustments.
		CString strWhere;
		// (a.walling 2008-07-07 17:13) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
		strWhere.Format("LineItemT.Type = 1 AND PatientID = %li AND LineItemT.Deleted = 0 "
			"AND OrigLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL ", m_nPatientID);

		m_pPayList->FromClause = _bstr_t("LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON LineItemT.ID = OrigLineItemsT.OriginalLineItemID "
			"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID");
		m_pPayList->WhereClause = _bstr_t(strWhere);

		m_pPayList->Requery();

		m_brush.CreateSolidBrush(PaletteColor(0x9CC294));
	}NxCatchAll("Error in CApplyToRefundDlg::OnInitDialog() ");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CApplyToRefundDlg::OnDblClickCellAvalPaysList(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			// (j.jones 2015-09-30 10:34) - PLID 67171 - just select the row and click ok
			m_pPayList->PutCurSel(pRow);
			OnOK();
			return;
		}

	}NxCatchAll(__FUNCTION__);	
}
