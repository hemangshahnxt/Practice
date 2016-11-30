// EditChargeRespDetailDlg.cpp : implementation file
//

#include "stdafx.h"
#include "billingRc.h"
#include "EditChargeRespDetailDlg.h"
#include "DontShowDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CEditChargeRespDetailDlg dialog


CEditChargeRespDetailDlg::CEditChargeRespDetailDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditChargeRespDetailDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditChargeRespDetailDlg)
		m_bWarnedDateChange = FALSE;
	//}}AFX_DATA_INIT
}


CEditChargeRespDetailDlg::CEditChargeRespDetailDlg(long nBillID, long nChargeID /* = -1*/, CWnd* pParent /* = NULL*/)
	: CNxDialog(CEditChargeRespDetailDlg::IDD, pParent)
{
	m_bWarnedDateChange = FALSE;

	//initialize the variables
	m_nBillID = nBillID;
	m_nChargeID = nChargeID;
}

void CEditChargeRespDetailDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditChargeRespDetailDlg)
	DDX_Control(pDX, IDC_CLOSE_CHRESP, m_btnCloseChResp);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditChargeRespDetailDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditChargeRespDetailDlg)
	ON_BN_CLICKED(IDC_CLOSE_CHRESP, OnCloseChresp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditChargeRespDetailDlg message handlers

BEGIN_EVENTSINK_MAP(CEditChargeRespDetailDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditChargeRespDetailDlg)
	ON_EVENT(CEditChargeRespDetailDlg, IDC_CRDETAIL_LIST, 16 /* SelChosen */, OnSelChosenDetailList, VTS_I4)
	ON_EVENT(CEditChargeRespDetailDlg, IDC_CHARGE_LIST, 16 /* SelChosen */, OnSelChosenChargeList, VTS_I4)
	ON_EVENT(CEditChargeRespDetailDlg, IDC_CRDETAIL_LIST, 9 /* EditingFinishing */, OnEditingFinishingCrdetailList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEditChargeRespDetailDlg, IDC_CRDETAIL_LIST, 10 /* EditingFinished */, OnEditingFinishedCrdetailList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CEditChargeRespDetailDlg::OnInitDialog() 
{

	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 15:57) - PLID 29871 - NxIconify the close button
		m_btnCloseChResp.AutoSet(NXB_CLOSE);

		//Bind the datalists
		m_pChargeList = BindNxDataListCtrl(this, IDC_CHARGE_LIST, GetRemoteData(), false);
		m_pDetailList = BindNxDataListCtrl(this, IDC_CRDETAIL_LIST, GetRemoteData(), false);

		// (j.jones 2011-09-13 15:35) - PLID 44887 - hide original & void charges
		CString strFromClause;
		strFromClause.Format("LineItemT "
			"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
			"WHERE LineItemT.Deleted = 0 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND ChargesT.BillID = %li", m_nBillID);

		m_pChargeList->FromClause = (LPCTSTR)strFromClause;

		//Requery the charge list
		m_pChargeList->Requery();

		//Add the <All Charges> Line to the charge list
		IRowSettingsPtr pRow;
		pRow = m_pChargeList->GetRow(-1);
		pRow->PutValue(0, (long)0);
		pRow->PutValue(1, _variant_t("<All Charges For This Bill>"));
		m_pChargeList->InsertRow(pRow, 0);


		if(m_nChargeID == -1) {
			//if no ChargeID, set the charge list to default to <All Charges>
			m_pChargeList->CurSel = 0;
		}
		else {
			//otherwise, select the charge
			m_pChargeList->SetSelByColumn(0, m_nChargeID);
		}

		//set the clauses for the Details List

		// (j.jones 2011-09-13 15:35) - PLID 44887 - hide original & void charges
		strFromClause = "ChargeRespDetailT "
			"INNER JOIN ChargeRespT ON ChargeRespDetailT.ChargeRespID = ChargeRespT.ID "
			"INNER JOIN LineItemT ON LineItemT.ID = ChargeRespT.ChargeID "
			"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"LEFT JOIN (SELECT DetailID, Sum(Round(Convert(money, Amount), 2)) AS AmountPaid "
			"	FROM ApplyDetailsT GROUP BY DetailID) AS AppliesQ ON ChargeRespDetailT.ID = AppliesQ.DetailID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID ";

		CString strWhereClause = "";
		if(m_nChargeID != -1) {
			strWhereClause.Format("(LineItemT.Type = 10) AND (LineItemT.Deleted = 0) "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND (ChargeRespT.ChargeID = %li)", m_nChargeID);
		}
		else {
			strWhereClause.Format("(LineItemT.Type = 10) AND (LineItemT.Deleted = 0) "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND (ChargesT.BillID = %li)", m_nBillID);
		}
		
		m_pDetailList->FromClause = (LPCTSTR)strFromClause;
		m_pDetailList->WhereClause = (LPCTSTR)strWhereClause;

		//Requery the list
		m_pDetailList->Requery();
		
		
		return TRUE;  // return TRUE unless you set the focus to a control
					// EXCEPTION: OCX Property Pages should return FALSE
	}NxCatchAllCall("Error in OnInitDialog", return FALSE;);
}

void CEditChargeRespDetailDlg::OnSelChosenDetailList(long nRow) 
{
		
}

void CEditChargeRespDetailDlg::OnSelChosenChargeList(long nRow) 
{

	try {

		//First, get the chargeID
		long nChargeID = VarLong(m_pChargeList->GetValue(nRow, 0), 0);

		// (j.jones 2011-09-13 15:35) - PLID 44887 - hide original & void charges
		CString strWhereClause = "";
		if(nChargeID <= 0) {
			strWhereClause.Format("(LineItemT.Type = 10) AND (LineItemT.Deleted = 0) "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND (ChargesT.BillID = %li)", m_nBillID);
		}
		else {
			strWhereClause.Format("(LineItemT.Type = 10) AND (LineItemT.Deleted = 0) "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND (ChargeRespT.ChargeID = %li)", nChargeID);
		}
		
		m_pDetailList->WhereClause = (LPCTSTR)strWhereClause;

		//MsgBox(m_pDetailList->GetSqlActive());
		m_pDetailList->Requery();
	}NxCatchAll("Error in CEditChargeRespDetailDlg::OnSelChosenChargeList");
	
}

void CEditChargeRespDetailDlg::OnCloseChresp() 
{

	OnOK();
	
}

void CEditChargeRespDetailDlg::OnEditingFinishingCrdetailList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	
	try {
		//we only edit the date 
		if (nCol == 6) {

			//check to see if the date is valid
			CString strDate = VarString(strUserEntered);
			COleDateTime dtDate;
			dtDate = VarDateTime(*pvarNewValue);
			COleDateTime dtNow, dtMin, dtMax;
			dtNow = COleDateTime::GetCurrentTime();
			dtMin.SetDate(1900, 1, 1);
			dtMax.SetDate(9999, 12, 31);

			if(pvarNewValue->vt != VT_DATE || dtDate.GetStatus() != COleDateTime::valid || dtDate > dtNow || dtDate < dtMin || dtDate > dtMax ) {
				
				MsgBox("Please enter a valid date.");
				*pbCommit = FALSE;
			}
			else {

				// (j.jones 2006-07-03 15:51) - PLID 21319 - if the date is the same as another for the same
				// charge resp, give a warning, because that's not allowed

				long nDetailID = m_pDetailList->GetValue(nRow, 0);

				COleDateTime dtDate = VarDateTime(pvarNewValue);
				//We only want to save dates to this table, not times.
				dtDate.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), 0, 0, 0);

				CString strDate = FormatDateTimeForSql(dtDate);

				if(ReturnsRecords("SELECT ID FROM ChargeRespDetailT WHERE Date = '%s' AND ID <> %li "
					"AND ChargeRespID IN (SELECT ChargeRespID FROM ChargeRespDetailT WHERE ID = %li) ", strDate, nDetailID, nDetailID)) {

					MsgBox("You cannot have two entries for the same charge responsibility with the same date.\n"
						"Please change the date to be a unique date for this charge responsibility.");

					*pbCommit = FALSE;					
				}
				else {
					*pbCommit = TRUE;
				}

				if(pvarNewValue->date != varOldValue.date && !m_bWarnedDateChange) {

					//warn them about the implications of this change
					if (DontShowMeAgain(this, "Changing the assignment date for this responsibility will affect calculations regarding how long the given\n"
										"responsibility has owed the given amount. Features such as Aged Receivables reports and Finance Charges will\n"
										"be affected by this change.\n\n"
										"Are you sure you wish to change the responsibility date?", "ChargeRespDatechange", "Editing Charge Responsibility Dates", FALSE, TRUE) == IDNO) {
						*pbCommit = FALSE;			
					}
					else {
						//if they approved it once, don't bother them again for the rest of this dialog
						m_bWarnedDateChange = TRUE;
					}
				}
			}
		}	
	}NxCatchAll("Error in CEditChargeRespDetailDlg::OnEditingFinishingCrdetailList");
}

void CEditChargeRespDetailDlg::OnEditingFinishedCrdetailList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	
	try {
		if (nCol == 6) {
			if(bCommit) {

				//if we are here, the date is valid, so let's save it
				long nDetailID = m_pDetailList->GetValue(nRow, 0);

				COleDateTime dtDate = VarDateTime(varNewValue);
				//We only want to save dates to this table, not times.
				dtDate.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), 0, 0, 0);

				CString strDate = FormatDateTimeForSql(dtDate);

				ExecuteSql("UPDATE ChargeRespDetailT SET Date = '%s' WHERE ID = %li", strDate, nDetailID);
			}
		}
	}NxCatchAll("Error in OnEditingFinishedCrDetailList");
	
}

