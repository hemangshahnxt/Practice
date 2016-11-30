// LinkEMNToBillDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LinkEMNToBillDlg.h"
#include "InternationalUtils.h"
#include "AuditTrail.h"

// (j.jones 2009-06-24 15:41) - PLID 24076 - created

// CLinkEMNToBillDlg dialog

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



enum BillListColumns {

	blcID = 0,
	blcParentID,
	blcBillID,
	blcDescription,
	blcDate,
	blcAmount,
	blcIsLinked,
};

CLinkEMNToBillDlg::CLinkEMNToBillDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLinkEMNToBillDlg::IDD, pParent)
{
	m_nPatientID = -1;
	m_nEMNID = -1;
}

void CLinkEMNToBillDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_EMN_DESC_LABEL, m_nxstaticDesc);
	DDX_Control(pDX, IDC_EMN_DATE_LABEL, m_nxstaticDate);
	DDX_Control(pDX, IDC_CHECK_INCLUDE_ALREADY_LINKED, m_checkIncludeLinked);
}


BEGIN_MESSAGE_MAP(CLinkEMNToBillDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_CHECK_INCLUDE_ALREADY_LINKED, OnCheckIncludeAlreadyLinked)
END_MESSAGE_MAP()

// CLinkEMNToBillDlg message handlers

BOOL CLinkEMNToBillDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		CString strText;
		strText.Format("Link Existing Bill To EMN for Patient '%s", m_strPatientName);
		SetWindowText(strText);

		m_nxstaticDesc.SetWindowText(m_strEMNDesc);
		m_nxstaticDate.SetWindowText(FormatDateTimeForInterface(m_dtEMNDate, NULL, dtoDate));

		m_List = BindNxDataList2Ctrl(IDC_LINK_EMN_TO_BILL_LIST, false);

		//set the list content
		// (j.gruber 2011-06-22 15:43) - PLID 44894 - don't show corrected bills or original or voided charges
		CString strFrom;
		strFrom.Format("("
			"SELECT BillsT.ID, NULL AS ParentID, BillsT.ID AS BillID, Date, Description, "
			"dbo.GetBillTotal(BillsT.ID) AS Total, "
			"Convert(bit, CASE WHEN BillsT.ID IN (SELECT BillID FROM BilledEMNsT) THEN 1 ELSE 0 END) AS IsLinked "
			"FROM BillsT "
			" LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
			"WHERE Deleted = 0 AND PatientID = %li AND BillsT.EntryType = 1 AND BillCorrectionsT.ID IS NULL "
			"AND BillsT.ID IN (SELECT BillID FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE LineItemT.Deleted = 0 AND LineItemT.PatientID = %li ) "
			""
			"UNION SELECT NULL AS ID, BillID AS ParentID, BillID, Date, "
			"CASE WHEN Len(ItemCode) = 0 THEN '' ELSE ItemCode + ' - ' END + Description AS Description, "
			"dbo.GetChargeTotal(ChargesT.ID) AS Total, "
			"Convert(bit, CASE WHEN BillID IN (SELECT BillID FROM BilledEMNsT) THEN 1 ELSE 0 END) AS IsLinked "
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

	}NxCatchAll("Error in CLinkEMNToBillDlg::OnInitDialog");

	return TRUE;
}

void CLinkEMNToBillDlg::OnOk()
{
	try {

		//require a selection
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
			if(IDNO == MessageBox("The selected bill has already been linked to an EMN. "
				"Are you sure you wish to link it to this EMN as well?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}

		//warn if the bill does not have all the charges that the EMN has
		_RecordsetPtr rs = CreateParamRecordset("SELECT EMRChargesT.Description, "
			"CPTCodeT.Code "
			"FROM EMRChargesT "
			"LEFT JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
			"WHERE Deleted = 0 AND EMRID = {INT} "
			"AND ServiceID NOT IN (SELECT ServiceID "
			"	FROM ChargesT "
			"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"	WHERE LineItemT.Deleted = 0 AND BillID = {INT})",
			m_nEMNID, nBillID);

		CString strCharges;

		while(!rs->eof) {

			CString strDescription = AdoFldString(rs, "Description", "");
			CString strCode = AdoFldString(rs, "Code", "");

			if(!strCharges.IsEmpty()) {
				strCharges += "\n";				
			}

			if(!strCode.IsEmpty()) {
				strCharges += strCode;
				strCharges += " - ";
			}

			strCharges += strDescription;

			rs->MoveNext();
		}
		rs->Close();

		if(!strCharges.IsEmpty()) {
			CString strWarn;
			strWarn.Format("The selected bill does not contain the following charges from the EMN:\n\n"
				"%s\n\n"
				"Are you sure you wish to link to this bill?", strCharges);

			if(IDNO == MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}

		//if we get here, link the two together

		CString strSqlBatch;
		CNxParamSqlArray aryParams;

		// (j.jones 2012-01-19 15:07) - PLID 47653 - this can optionally track an insured party ID
		// but since we didn't actually bill it, we can't fill InsuredPartyID
		// (j.armen 2014-01-30 17:15) - PLID 60566 - Idenitate BilledEMNsT
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO BilledEMNsT (BillID, EMNID) VALUES ({INT}, {INT})", nBillID, m_nEMNID);

		// (j.jones 2012-01-17 12:31) - PLID 47537 - now attempt to fill for all existing charges (ignoring deleted bills and charges)
		_RecordsetPtr rsCharges = CreateParamRecordset("SELECT ChargesT.ID AS ChargeID, ChargesT.ServiceID "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"WHERE ChargesT.BillID = {INT} AND LineItemT.Type = 10 "
			"AND ChargesT.EMRChargeID Is Null "
			"AND ChargesT.ServiceID IN (SELECT ServiceID FROM EMRChargesT WHERE Deleted = 0 AND EMRID = {INT})", nBillID, m_nEMNID);

		while(!rsCharges->eof) {
			//for each charge, find a matching charge on the EMN, preferably one we have not used
			long nChargeID = rsCharges->Fields->Item["ChargeID"]->Value.lVal;
			long nServiceID = rsCharges->Fields->Item["ServiceID"]->Value.lVal;

			//find a charge on the EMN with the same service ID, and in the case where
			//multiple charges may exist for the same service ID, prioritize the one that
			//was not already linked to a charge
			//(this does not bother trying to match on quantity or amount, all we need to do
			//is tag the EMR charge as having been billed, the precise charge is not crucial)
			_RecordsetPtr rsEMRCharges = CreateParamRecordset("SELECT TOP 1 EMRChargesT.ID "
				"FROM EMRChargesT "
				"WHERE Deleted = 0 AND EMRID = {INT} AND ServiceID = {INT} "
				"ORDER BY (CASE WHEN EMRChargesT.ID NOT IN (SELECT EMRChargeID FROM ChargesT "
				"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"	WHERE LineItemT.Type = 10 AND Deleted = 0) THEN 0 ELSE 1 END) ASC",
				m_nEMNID, nServiceID);
			if(!rsEMRCharges->eof) {
				//found a charge, now link them
				long nEMRChargeID = rsEMRCharges->Fields->Item["ID"]->Value.lVal;
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE ChargesT SET EMRChargeID = {INT} WHERE ID = {INT}", nEMRChargeID, nChargeID);
			}
			rsEMRCharges->Close();

			rsCharges->MoveNext();
		}
		rsCharges->Close();

		ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);

		//and audit the link
		long nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1) {
			CString strOldValue;
			strOldValue.Format("EMN: %s", m_strEMNDesc);
			CString strNewValue;
			strNewValue.Format("Linked To Bill: %s (%s)", VarString(pRow->GetValue(blcDescription)),
				FormatDateTimeForInterface(VarDateTime(pRow->GetValue(blcDate)), NULL, dtoDate));
			AuditEvent(m_nPatientID, m_strPatientName, nAuditID, aeiEMNLinkedToBill, m_nEMNID, strOldValue, strNewValue, aepMedium, aetCreated);
		}

		CNxDialog::OnOK();

	}NxCatchAll("Error in CLinkEMNToBillDlg::OnOk");
}

void CLinkEMNToBillDlg::OnCancel()
{
	try {

		CNxDialog::OnCancel();

	}NxCatchAll("Error in CLinkEMNToBillDlg::OnCancel");
}

void CLinkEMNToBillDlg::OnCheckIncludeAlreadyLinked()
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

		//this setting is not stored

	}NxCatchAll("Error in CLinkEMNToBillDlg::OnCheckIncludeAlreadyLinked");
}
