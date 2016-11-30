// ManageQuotePrepaysDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ManageQuotePrepaysDlg.h"
#include "PaymentDlg.h"
#include "GlobalFinancialUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_NEW_PREPAY		41238
#define ID_MANAGE_PREPAYS	41239
#define ID_EDIT_PREPAY		41240

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CManageQuotePrepaysDlg dialog


CManageQuotePrepaysDlg::CManageQuotePrepaysDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CManageQuotePrepaysDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CManageQuotePrepaysDlg)
		m_DefaultQuoteID = -1;
		m_PatientID = -1;
	//}}AFX_DATA_INIT
}


void CManageQuotePrepaysDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CManageQuotePrepaysDlg)
	DDX_Control(pDX, IDC_CHECK_SHOW_UNLINKED_PREPAYS, m_btnShowUnlinked);
	DDX_Control(pDX, IDC_BTN_NEW_PREPAY, m_btnNewPrePay);
	DDX_Control(pDX, IDC_PREPAY_LINK_BTN, m_btnPrePayLink);
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CManageQuotePrepaysDlg, CNxDialog)
	//{{AFX_MSG_MAP(CManageQuotePrepaysDlg)
	ON_BN_CLICKED(IDC_BTN_NEW_PREPAY, OnBtnNewPrepay)
	ON_BN_CLICKED(IDC_PREPAY_LINK_BTN, OnPrepayLinkBtn)
	ON_COMMAND(ID_NEW_PREPAY, OnBtnNewPrepay)
	ON_COMMAND(ID_MANAGE_PREPAYS, OnPrepayLinkBtn)
	ON_COMMAND(ID_EDIT_PREPAY, OnEditPrepay)
	ON_BN_CLICKED(IDC_CHECK_SHOW_UNLINKED_PREPAYS, OnCheckShowUnlinkedPrepays)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CManageQuotePrepaysDlg message handlers

BOOL CManageQuotePrepaysDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		// (c.haag 2008-05-01 17:31) - PLID 29876 - NxIconify buttons
		m_btnNewPrePay.AutoSet(NXB_NEW);
		m_btnPrePayLink.AutoSet(NXB_MODIFY);
		m_btnOK.AutoSet(NXB_CLOSE);
	
		m_PrePayList = BindNxDataListCtrl(this,IDC_LINKED_PREPAY_LIST,GetRemoteData(),false);
		m_QuoteList = BindNxDataListCtrl(this,IDC_QUOTE_LIST,GetRemoteData(),false);

		// (j.gruber 2009-03-18 11:12) - PLID 33574 - update discount structure

		//build the quote list
		CString str;
		// (j.jones 2011-06-17 14:07) - PLID 38347 - fixed quote total calculation to account for modifiers
		str.Format("(SELECT [PatientBillsQ].ID, [PatientBillsQ].Description, [PatientBillsQ].Date, CASE WHEN HasBeenBilled Is Null THEN 0 ELSE 1 END AS HasBeenBilled, "
				"(CASE WHEN PackagesT.QuoteID Is Not Null THEN PackagesT.CurrentAmount ELSE "
				"Sum(Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))) + "
				"(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate-1)) + "
				"(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate2-1)) "
				")),2)) END) AS Total "
				"FROM ((SELECT BillsT.*, (SELECT TOP 1 ID FROM BilledQuotesT WHERE BilledQuotesT.QuoteID = BillsT.ID AND BilledQuotesT.BillID IN (SELECT ID FROM BillsT WHERE Deleted = 0 AND EntryType = 1)) AS HasBeenBilled FROM BillsT WHERE (((BillsT.PatientID)=%li) AND ((BillsT.Deleted)=0)) AND BillsT.Active = 1) AS PatientBillsQ LEFT JOIN (SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"WHERE (((LineItemT.PatientID)=%li) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=10))) AS PatientChargesQ ON [PatientBillsQ].ID = [PatientChargesQ].BillID) "
				"LEFT JOIN ChargesT ON [PatientChargesQ].ID = ChargesT.ID "
				"LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID "
				"LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID "
				"LEFT JOIN PackagesT ON PatientBillsQ.ID = PackagesT.QuoteID "
				"WHERE ((([PatientBillsQ].EntryType)=2)) "
				"GROUP BY [PatientBillsQ].ID, [PatientBillsQ].Date, [PatientBillsQ].Description, PatientBillsQ.HasBeenBilled, PackagesT.QuoteID, PackagesT.CurrentAmount) AS QuotesQ",m_PatientID,m_PatientID);

		m_QuoteList->FromClause = _bstr_t(str);
		m_QuoteList->Requery();

		OnSelChosenQuoteList(m_QuoteList->SetSelByColumn(0,m_DefaultQuoteID));

	}NxCatchAll("Error loading quote prepay interface.");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CManageQuotePrepaysDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CManageQuotePrepaysDlg)
	ON_EVENT(CManageQuotePrepaysDlg, IDC_QUOTE_LIST, 16 /* SelChosen */, OnSelChosenQuoteList, VTS_I4)
	ON_EVENT(CManageQuotePrepaysDlg, IDC_LINKED_PREPAY_LIST, 6 /* RButtonDown */, OnRButtonDownLinkedPrepayList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CManageQuotePrepaysDlg, IDC_LINKED_PREPAY_LIST, 18 /* RequeryFinished */, OnRequeryFinishedLinkedPrepayList, VTS_I2)
	ON_EVENT(CManageQuotePrepaysDlg, IDC_LINKED_PREPAY_LIST, 2 /* SelChanged */, OnSelChangedLinkedPrepayList, VTS_I4)
	ON_EVENT(CManageQuotePrepaysDlg, IDC_LINKED_PREPAY_LIST, 3 /* DblClickCell */, OnDblClickCellLinkedPrepayList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CManageQuotePrepaysDlg::OnSelChosenQuoteList(long nRow) 
{
	try {

		UpdateButtons();

		if(nRow == -1) {
			m_PrePayList->Clear();
			return;
		}

		long QuoteID = VarLong(m_QuoteList->GetValue(nRow,0),-1);

		CString str;
		//JMJ - I decided not to filter on PrePayments only because they may be stupid and change the
		//prepayment status. So just look at the quote ID here.
		str.Format("SELECT PaymentsT.ID, LineItemT.Date, LineItemT.Description, LineItemT.Amount AS Total, LineItemT.Amount - (CASE WHEN AppliedQ.Amount Is Null THEN 0 ELSE AppliedQ.Amount END) + (CASE WHEN AppliesQ.Amount Is Null THEN 0 ELSE AppliesQ.Amount END) AS AmtAvail, 1 AS IsLinked "
			"FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT Sum(Amount) AS Amount, SourceID FROM AppliesT GROUP BY SourceID) AS AppliedQ ON PaymentsT.ID = AppliedQ.SourceID "
			"LEFT JOIN (SELECT Sum(Amount) AS Amount, DestID FROM AppliesT GROUP BY DestID) AS AppliesQ ON PaymentsT.ID = AppliesQ.DestID "
			"WHERE LineItemT.Deleted = 0 AND LineItemT.PatientID = %li AND PaymentsT.QuoteID = %li", m_PatientID, QuoteID);

		if(IsDlgButtonChecked(IDC_CHECK_SHOW_UNLINKED_PREPAYS)) {
			// (j.dinatale 2011-08-31 10:27) - PLID 45278 - Exclude any line items that are the original or voiding line item of a financial correction
			//JMJ - but here, for unlinked prepayments, only show prepayments
			CString str2;
			str2.Format(" UNION "
			"SELECT PaymentsT.ID, LineItemT.Date, LineItemT.Description, LineItemT.Amount AS Total, LineItemT.Amount - (CASE WHEN AppliedQ.Amount Is Null THEN 0 ELSE AppliedQ.Amount END) + (CASE WHEN AppliesQ.Amount Is Null THEN 0 ELSE AppliesQ.Amount END) AS AmtAvail, 0 AS IsLinked "
			"FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT Sum(Amount) AS Amount, SourceID FROM AppliesT GROUP BY SourceID) AS AppliedQ ON PaymentsT.ID = AppliedQ.SourceID "
			"LEFT JOIN (SELECT Sum(Amount) AS Amount, DestID FROM AppliesT GROUP BY DestID) AS AppliesQ ON PaymentsT.ID = AppliesQ.DestID "
			"LEFT JOIN LineItemCorrectionsT OriginalLineItemsT ON LineItemT.ID = OriginalLineItemsT.OriginalLineItemID "
			"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
			"WHERE LineItemT.Deleted = 0 AND LineItemT.PatientID = %li AND PaymentsT.PrePayment = 1 AND PaymentsT.QuoteID IS NULL "
			"AND OriginalLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL ", m_PatientID);
			str += str2;
		}

		CString sql = "(" + str + ") AS PrePays";

		m_PrePayList->FromClause = _bstr_t(sql);
		m_PrePayList->Requery();

	}NxCatchAll("Error loading prepayment list.");
}

void CManageQuotePrepaysDlg::OnBtnNewPrepay() 
{
	try {

		if(m_QuoteList->CurSel == -1) {
			AfxMessageBox("Please select a quote before trying to make a payment.");
			return;
		}

		// (j.jones 2007-02-20 10:50) - PLID 23706 - added a check for the billing license
		if(CheckCurrentUserPermissions(bioPayment,sptCreate) && g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrUse)) {

			long QuoteID = m_QuoteList->GetValue(m_QuoteList->CurSel,0).lVal;
			COleCurrency cyRemBalance;

			//this is more exact than searching the datalist
			// (j.gruber 2009-03-18 11:29) - PLID 33574 0 new discount structure
			// (j.jones 2011-06-17 14:08) - PLID 38347 - fixed quote total calculation to account for modifiers
			_RecordsetPtr rs = CreateRecordset("SELECT (CASE WHEN PackagesT.QuoteID Is Not Null THEN PackagesT.CurrentAmount ELSE "
							"Sum(Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))) + "
							"(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate-1)) + "
							"(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate2-1)) "
							")),2)) END) AS Total "
							"FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
							"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
							"LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID "
							"LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID "
							"LEFT JOIN PackagesT ON BillsT.ID = PackagesT.QuoteID "
							"WHERE BillsT.EntryType = 2 AND BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND LineItemT.Type = 11 AND BillsT.ID = %li "
							"GROUP BY PackagesT.CurrentAmount, PackagesT.QuoteID ",QuoteID);

			if(rs->eof) {
				rs->Close();
				AfxMessageBox("This quote has been deleted.");
				return;
			}

			if(rs->Fields->Item["Total"]->Value.vt == VT_CY)
				cyRemBalance = rs->Fields->Item["Total"]->Value.cyVal;
			else
				cyRemBalance = COleCurrency(0,0);

			rs->Close();

			cyRemBalance -= CalculatePrePayments(m_PatientID, QuoteID, false);

			CPaymentDlg dlg(this);
			dlg.m_varBillID = (long)QuoteID;
			dlg.m_bIsPrePayment = TRUE;
			dlg.m_QuoteID = QuoteID;
			dlg.m_cyFinalAmount = cyRemBalance;
			dlg.DoModal(__FUNCTION__, __LINE__);

			OnSelChosenQuoteList(m_QuoteList->GetCurSel());
		}

	}NxCatchAll("Error making new prepayment.");
}

void CManageQuotePrepaysDlg::OnPrepayLinkBtn() 
{
	try {

		BOOL bLink = FALSE;
		long nPaymentID = -1;

		if(m_QuoteList->CurSel == -1) {
			AfxMessageBox("Please select a quote before trying to access a payment.");
			return;
		}

		if(m_PrePayList->CurSel == -1) {
			AfxMessageBox("Please select a payment.");
			return;
		}

		//check the link status of the current row
		IRowSettingsPtr pRow = m_PrePayList->GetRow(m_PrePayList->CurSel);
		if(VarLong(pRow->GetValue(5)) == 0) {
			bLink = TRUE;
		}

		nPaymentID = VarLong(pRow->GetValue(0),-1);

		if(!bLink) {
			//they want to unlink a prepayment
			if(IDNO == MessageBox("Are you SURE you wish to unlink this PrePayment from this quote?\n"
				"(The payment will not be deleted, it will just be made available for general use.)","Practice",MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
			
			ExecuteSql("UPDATE PaymentsT SET QuoteID = NULL WHERE ID = %li",nPaymentID);
			if(IsDlgButtonChecked(IDC_CHECK_SHOW_UNLINKED_PREPAYS)) {
				pRow->PutValue(5,(long)0);
				pRow->PutForeColor(RGB(127,127,127));
			}
			else {
				m_PrePayList->RemoveRow(m_PrePayList->CurSel);
			}

			//TES 7/31/2007 - PLID 24582 - If this prepayment is attached to any PICs that it is not also applied
			// to the active bill for, prompt them to detach (if it's applied to the bill, it would stay attached 
			// anyway, so don't confuse them).
			// (j.dinatale 2012-07-11 15:27) - PLID 51474 - handle ProcInfoBillsT
			_RecordsetPtr rsProcInfos = CreateRecordset(
				"SELECT ID FROM ProcInfoT WHERE ID IN "
				"(SELECT ProcInfoID FROM ProcInfoPaymentsT WHERE PayID = %li) AND "
				"ID NOT IN "
				"(SELECT ProcInfoBillsT.ProcInfoID "
				"FROM ChargesT "
				"INNER JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
				"LEFT JOIN ProcInfoBillsT ON ChargesT.BillID = ProcInfoBillsT.BillID "
				"WHERE AppliesT.SourceID = %li)", nPaymentID, nPaymentID);
			if(!rsProcInfos->eof) {
				if(IDYES == MsgBox(MB_YESNO, "This prepayment was also attached to one or more PICs.  Would you like to detach it from them?")) {
					CString strProcInfoIDs;
					while(!rsProcInfos->eof) {
						strProcInfoIDs += AsString(AdoFldLong(rsProcInfos, "ID")) + ",";
						rsProcInfos->MoveNext();
					}
					rsProcInfos->Close();
					strProcInfoIDs.TrimRight(",");
					ExecuteSql("DELETE FROM ProcInfoPaymentsT WHERE ProcInfoID IN (%s) AND PayID = %li", 
						strProcInfoIDs, nPaymentID);
				}
			}
		}
		else {
			//they want to link a prepayment
			long nQuoteID = VarLong(m_QuoteList->GetValue(m_QuoteList->GetCurSel(),0));
			ExecuteSql("UPDATE PaymentsT SET QuoteID = %li WHERE ID = %li",nQuoteID,nPaymentID);
			pRow->PutValue(5,(long)1);
			pRow->PutForeColor(RGB(0,0,0));

			//TES 10/26/2006 - This may cause a step to be marked done in the ladder.
			PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_PaymentApplied, m_PatientID, COleDateTime::GetCurrentTime(), nPaymentID, true, -1);


			//TES 8/6/2007 - PLID 20720 - If this quote is on any PICs, prompt to add this prepayment to them.
			// (j.dinatale 2012-07-11 15:27) - PLID 51474 - handle ProcInfoBillsT
			_RecordsetPtr rsProcInfos = CreateRecordset("SELECT ID FROM ProcInfoT WHERE ID IN "
				"(SELECT ID FROM ProcInfoT WHERE ActiveQuoteID = %li) "
				"AND ID NOT IN "
				"(SELECT ProcInfoID FROM ProcInfoPaymentsT WHERE PayID = %li) "
				"AND ID NOT IN "
				"(SELECT ProcInfoBillsT.ProcInfoID FROM ProcInfoBillsT INNER JOIN BillsT ON ProcInfoBillsT.BillID = BillsT.ID INNER JOIN "
				" ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
				" WHERE AppliesT.SourceID = %li)",
				nQuoteID, nPaymentID, nPaymentID);
			if(!rsProcInfos->eof) {
				if(IDYES == MsgBox(MB_YESNO, "This quote is attached to one or more PICs.  Would you like to attach this prepayment to those PICs?")) {
					while(!rsProcInfos->eof) {
						ExecuteSql("INSERT INTO ProcInfoPaymentsT (ProcInfoID, PayID) VALUES (%li, %li)",
							AdoFldLong(rsProcInfos, "ID"), nPaymentID);
						rsProcInfos->MoveNext();
					}
				}
			}

		}

		UpdateButtons();

	}NxCatchAll("Error linking prepayment.");
}

void CManageQuotePrepaysDlg::OnCheckShowUnlinkedPrepays() 
{
	OnSelChosenQuoteList(m_QuoteList->GetCurSel());
}

void CManageQuotePrepaysDlg::OnRButtonDownLinkedPrepayList(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {

		CMenu pMenu;
		m_PrePayList->CurSel = nRow;

		UpdateButtons();

		CString str;
		GetDlgItem(IDC_PREPAY_LINK_BTN)->GetWindowText(str);

		pMenu.CreatePopupMenu();	
		pMenu.InsertMenu(0, MF_BYPOSITION, ID_NEW_PREPAY, "Make a PrePayment");
		if(nRow != -1) {
			pMenu.InsertMenu(1, MF_BYPOSITION, ID_MANAGE_PREPAYS, str);
			pMenu.InsertMenu(2, MF_BYPOSITION|MF_SEPARATOR);
			pMenu.InsertMenu(3, MF_BYPOSITION, ID_EDIT_PREPAY, "Edit PrePayment");
		}

		CPoint pt;
		GetCursorPos(&pt);
		pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);

	}NxCatchAll("Error displaying menu.");
}

void CManageQuotePrepaysDlg::OnOK() 
{	
	CDialog::OnOK();
}

void CManageQuotePrepaysDlg::OnRequeryFinishedLinkedPrepayList(short nFlags) 
{
	try {

		for(int i=0;i<m_PrePayList->GetRowCount();i++) {
			IRowSettingsPtr pRow = m_PrePayList->GetRow(i);
			if(VarLong(pRow->GetValue(5)) == 0) {
				//if not linked, marked as gray
				pRow->PutForeColor(RGB(127,127,127));
			}
		}

	}NxCatchAll("Error displaying prepayment list.");
}

void CManageQuotePrepaysDlg::UpdateButtons()
{
	try {

		long nRow = m_QuoteList->GetCurSel();

		if(nRow == -1) {
			//nothing selected in the quote list, so disable the buttons (not the checkbox)
			GetDlgItem(IDC_BTN_NEW_PREPAY)->EnableWindow(FALSE);
			GetDlgItem(IDC_PREPAY_LINK_BTN)->EnableWindow(FALSE);
		}
		else {
			//enable the buttons
			GetDlgItem(IDC_BTN_NEW_PREPAY)->EnableWindow(TRUE);			

			if(m_PrePayList->CurSel == -1) {
				GetDlgItem(IDC_PREPAY_LINK_BTN)->EnableWindow(FALSE);
				SetDlgItemText(IDC_PREPAY_LINK_BTN,"Unlink PrePayment");				
			}
			else {
				GetDlgItem(IDC_PREPAY_LINK_BTN)->EnableWindow(TRUE);

				//change the prepay link text based on the link status of the current row
				IRowSettingsPtr pRow = m_PrePayList->GetRow(m_PrePayList->CurSel);
				if(VarLong(pRow->GetValue(5)) == 0) {
					SetDlgItemText(IDC_PREPAY_LINK_BTN,"Link PrePayment");
				}
				else {
					SetDlgItemText(IDC_PREPAY_LINK_BTN,"Unlink PrePayment");
				}
			}
		}

	}NxCatchAll("Error updating buttons.");
}

void CManageQuotePrepaysDlg::OnSelChangedLinkedPrepayList(long nNewSel) 
{
	UpdateButtons();
}

void CManageQuotePrepaysDlg::OnDblClickCellLinkedPrepayList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex != -1) {

		m_PrePayList->CurSel = nRowIndex;

		OnEditPrepay();
	}
}

void CManageQuotePrepaysDlg::OnEditPrepay()
{
	try {

		if(m_QuoteList->CurSel == -1) {
			AfxMessageBox("Please select a quote first.");
			return;
		}

		// (j.jones 2007-02-20 10:50) - PLID 23706 - added a check for the billing license
		if(CheckCurrentUserPermissions(bioPayment,sptWrite) && g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrUse)) {

			long QuoteID = m_QuoteList->GetValue(m_QuoteList->CurSel,0).lVal;

			IRowSettingsPtr pRow = m_PrePayList->GetRow(m_PrePayList->CurSel);
			long nPaymentID = VarLong(pRow->GetValue(0));

			CPaymentDlg dlg(this);
			dlg.m_varPaymentID = (long)nPaymentID;
			dlg.m_bIsPrePayment = TRUE;
			dlg.m_varBillID = (long)QuoteID;
			dlg.m_QuoteID = QuoteID;
			dlg.m_boIsNewPayment = FALSE;
			dlg.DoModal(__FUNCTION__, __LINE__);

			OnSelChosenQuoteList(m_QuoteList->GetCurSel());

		}

	}NxCatchAll("Error editing PrePayment.");
}
