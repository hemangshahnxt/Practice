// LinkCaseHistoryToBillDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LinkCaseHistoryToBillDlg.h"
#include "InternationalUtils.h"
#include "AuditTrail.h"

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (j.jones 2009-08-10 12:17) - PLID 24077 - created

// CLinkCaseHistoryToBillDlg dialog

enum BillListColumns {

	blcID = 0,
	blcParentID,
	blcBillID,
	blcDescription,
	blcDate,
	blcAmount,
	blcIsLinked,
};

using namespace NXDATALIST2Lib;
using namespace ADODB;

IMPLEMENT_DYNAMIC(CLinkCaseHistoryToBillDlg, CNxDialog)

CLinkCaseHistoryToBillDlg::CLinkCaseHistoryToBillDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLinkCaseHistoryToBillDlg::IDD, pParent)
{
	m_nPatientID = -1;
	m_nCaseHistoryID = -1;
}

void CLinkCaseHistoryToBillDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CASE_HISTORY_DESC_LABEL, m_nxstaticDesc);
	DDX_Control(pDX, IDC_CASE_HISTORY_DATE_LABEL, m_nxstaticDate);
	DDX_Control(pDX, IDC_CHECK_INCLUDE_ALREADY_LINKED, m_checkIncludeLinked);
}


BEGIN_MESSAGE_MAP(CLinkCaseHistoryToBillDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_CHECK_INCLUDE_ALREADY_LINKED, OnCheckIncludeAlreadyLinked)
END_MESSAGE_MAP()


// CLinkCaseHistoryToBillDlg message handlers
BOOL CLinkCaseHistoryToBillDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		CString strText;
		strText.Format("Link Existing Bill To Case History for Patient '%s", m_strPatientName);
		SetWindowText(strText);

		m_nxstaticDesc.SetWindowText(m_strCaseHistoryDesc);
		m_nxstaticDate.SetWindowText(FormatDateTimeForInterface(m_dtCaseHistoryDate, NULL, dtoDate));

		m_List = BindNxDataList2Ctrl(IDC_LINK_CASE_HISTORY_TO_BILL_LIST, false);

		//set the list content
		// (j.gruber 2011-06-27 14:46) - PLID 44874 - don't show corrected bills or charges
		CString strFrom;
		strFrom.Format("("
			"SELECT BillsT.ID, NULL AS ParentID, BillsT.ID AS BillID, BillsT.Date, BillsT.Description, "
			"dbo.GetBillTotal(BillsT.ID) AS Total, "
			"Convert(bit, CASE WHEN BillsT.ID IN (SELECT BillID FROM BilledCaseHistoriesT) THEN 1 ELSE 0 END) AS IsLinked "
			"FROM BillsT "
			" LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
			"WHERE Deleted = 0 AND PatientID = %li AND BillsT.EntryType = 1 AND BillCorrectionsT.ID IS NULL "
			"AND BillsT.ID IN (SELECT BillID FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE LineItemT.Deleted = 0 AND LineItemT.PatientID = %li ) "
			""
			"UNION SELECT NULL AS ID, BillID AS ParentID, BillID, Date, "
			"CASE WHEN Len(ItemCode) = 0 THEN '' ELSE ItemCode + ' - ' END + Description AS Description, "
			"dbo.GetChargeTotal(ChargesT.ID) AS Total, "
			"Convert(bit, CASE WHEN BillID IN (SELECT BillID FROM BilledCaseHistoriesT) THEN 1 ELSE 0 END) AS IsLinked "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			" LEFT JOIN LineItemCorrectionsT ON LineItemT.ID = LineItemCorrectionsT.OriginalLineItemID "
			" LEFT JOIN LineItemCorrectionsT VoidsT ON LineItemT.ID = VoidsT.VoidingLineItemID "
			"WHERE Deleted = 0 AND PatientID = %li AND LineItemT.Type = 10 AND LineItemCorrectionsT.ID IS NULL AND VoidsT.ID IS NULL "
			"AND ChargesT.BillID IN (SELECT ID FROM BillsT WHERE BillsT.Deleted = 0 AND BillsT.PatientID = %li) "
			") AS BillListQ",
			m_nPatientID, m_nPatientID, m_nPatientID, m_nPatientID);

		m_List->PutFromClause(_bstr_t(strFrom));

		//always default to false
		m_checkIncludeLinked.SetCheck(FALSE);

		CString strWhere = "IsLinked = 0";
		m_List->PutWhereClause(_bstr_t(strWhere));

		m_List->GetColumn(blcIsLinked)->PutStoredWidth(0);

		m_List->Requery();

	}NxCatchAll("Error in CLinkCaseHistoryToBillDlg::OnInitDialog");

	return TRUE;
}

void CLinkCaseHistoryToBillDlg::OnOk()
{
	try {

		IRowSettingsPtr pRow = m_List->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("You must select a bill from the list to link with.");
			return;
		}

		//select the parent if we have one
		if(pRow->GetParentRow()) {
			pRow = pRow->GetParentRow();
		}

		long nBillID = VarLong(pRow->GetValue(blcBillID));

		//warn if the bill is already linked
		BOOL bIsLinked = VarBool(pRow->GetValue(blcIsLinked));
		if(bIsLinked) {
			if(IDNO == MessageBox("The selected bill has already been linked to an existing Case History. "
				"Are you sure you wish to link it to this Case History as well?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}

		//make sure that this record hasn't already been created 
		ExecuteParamSql("INSERT INTO BilledCaseHistoriesT (BillID, CaseHistoryID) "
			"SELECT {INT}, {INT} WHERE NOT EXISTS (SELECT BillID FROM BilledCaseHistoriesT WHERE BillID = {INT} AND CaseHistoryID = {INT})",
			nBillID, m_nCaseHistoryID, nBillID, m_nCaseHistoryID);

		// (j.jones 2009-08-12 13:29) - PLID 35179 - added auditing for this
		// audit even if the above does nothing, that code is just a failsafe that should rarely occur if ever

		CString strOldValue, strNewValue;
		strOldValue.Format("Case History: %s (%s)", m_strCaseHistoryDesc, FormatDateTimeForInterface(m_dtCaseHistoryDate, NULL, dtoDate));
		strNewValue.Format("Linked To Bill: %s (%s)", VarString(pRow->GetValue(blcDescription), ""), FormatDateTimeForInterface(VarDateTime(pRow->GetValue(blcDate)), NULL, dtoDate));

		long nAuditID = BeginNewAuditEvent();		
		AuditEvent(m_nPatientID, m_strPatientName, nAuditID, aeiLinkedCaseHistoryToBill, m_nCaseHistoryID, strOldValue, strNewValue, aepMedium, aetChanged);

		CNxDialog::OnOK();

	}NxCatchAll("Error in CLinkCaseHistoryToBillDlg::OnOk");
}

void CLinkCaseHistoryToBillDlg::OnCancel()
{
	try {

		CNxDialog::OnCancel();

	}NxCatchAll("Error in CLinkCaseHistoryToBillDlg::OnCancel");
}

void CLinkCaseHistoryToBillDlg::OnCheckIncludeAlreadyLinked()
{
	try {

		CString strWhere;

		if(!m_checkIncludeLinked.GetCheck()) {
			strWhere = "IsLinked = 0";
			m_List->GetColumn(blcIsLinked)->PutStoredWidth(0);
		}
		else {
			m_List->GetColumn(blcIsLinked)->PutStoredWidth(100);
		}

		m_List->PutWhereClause(_bstr_t(strWhere));
		m_List->Requery();

	}NxCatchAll("Error in CLinkCaseHistoryToBillDlg::OnCheckIncludeAlreadyLinked");
}
