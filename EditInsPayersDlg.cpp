// EditInsPayersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "EditInsPayersDlg.h"
#include "AdvPayerIDDlg.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CEditInsPayersDlg dialog

// (j.jones 2008-09-15 09:17) - PLID 31374 - added enum for columns
enum PayerListColumns {

	plcID = 0,
	plcPayerID,
	plcName,
};

CEditInsPayersDlg::CEditInsPayersDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditInsPayersDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditInsPayersDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditInsPayersDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditInsPayersDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_DELETE, m_btnDelete);
	DDX_Control(pDX, IDC_BTN_ADVANCED, m_btnAdvanced);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditInsPayersDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditInsPayersDlg)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_BTN_ADVANCED, OnBtnAdvanced)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditInsPayersDlg message handlers

void CEditInsPayersDlg::OnAdd() 
{
	try {

		CString PayerID, PayerName;

		CString strInput = "Enter new Payer ID";

		// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer IDs, and thus removed format type
		/*
		if(m_FormatType == 2)
			strInput = "Enter new THIN ID";
		*/

		// (j.jones 2008-09-09 14:02) - PLID 31138 - removed Regence

		int nResult = InputBoxLimited(this, strInput, PayerID, "",50,false,false,NULL);
		while (nResult == IDOK && PayerID == "")
		{	
			MsgBox("You cannot enter a blank ID");
			nResult = InputBoxLimited(this, strInput, PayerID, "",50,false,false,NULL);
		}
		if(nResult == IDOK) {
			nResult = InputBoxLimited(this, "Enter the Payer name", PayerName, "",150,false,false,NULL);
			while(nResult == IDOK && PayerName == "") {
				MsgBox("You cannot enter a blank Payer name");
				nResult = InputBoxLimited(this, "Enter the Payer name", PayerName, "",150,false,false,NULL);
			}
			if (nResult == IDOK )
			{
				long ID = NewNumber("EBillingInsCoIDs","ID");
				// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer IDs, and thus removed format type
				ExecuteParamSql("INSERT INTO EBillingInsCoIDs (ID,InsCo,EBillingID) VALUES ({INT}, {STRING}, {STRING})", ID, PayerName, PayerID);
				IRowSettingsPtr pRow;
				pRow = m_PayerList->GetRow(-1);
				pRow->PutValue(plcID, ID);
				pRow->PutValue(plcPayerID, _bstr_t(PayerID));
				pRow->PutValue(plcName, _bstr_t(PayerName));
				long row = m_PayerList->AddRow(pRow);
				m_PayerList->CurSel = row;
			}
			RefreshButtons();
		}		
	}NxCatchAll("Error adding payer.");
}

void CEditInsPayersDlg::OnDelete() 
{
	long CurSel = m_PayerList->GetCurSel();
	if(CurSel == -1) {
		return;
	}

	long nAuditTransactionID = -1;

	try {

		long nID = VarLong(m_PayerList->GetValue(CurSel,plcID));
		CString strPayerID = VarString(m_PayerList->GetValue(CurSel,plcPayerID));

		// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer IDs, and thus removed format type

		// (j.jones 2008-09-15 09:32) - PLID 31374 - warn if the code is in use
		// (j.jones 2009-12-16 16:05) - PLID 36237 - supported UB Payer IDs
		// (j.jones 2012-08-07 11:10) - PLID 51914 - supported the payer ID structure change
		if(ReturnsRecordsParam("SELECT PersonID FROM InsuranceCoT WHERE HCFAPayerID = {INT} OR EligPayerID = {INT} OR UBPayerID = {INT}", nID, nID, nID)) {
			if(IDNO == MessageBox("This code is in use as a HCFA Payer ID, UB Payer ID, or an Eligibility Payer ID on at least one insurance company.\n"
				"Deleting this code will remove it from each company that uses it. Are you sure you wish to continue?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}
		// (j.jones 2009-08-05 09:16) - PLID 34467 - warn if the code is in use by a insurance / location combination,
		// but don't bother warning if they agreed to the previous warning
		// (j.jones 2009-12-16 16:05) - PLID 36237 - supported UB Payer IDs
		else if(ReturnsRecordsParam("SELECT InsuranceCoID FROM InsuranceLocationPayerIDsT "
			"WHERE ClaimPayerID = {INT} OR EligibilityPayerID = {INT} OR UBPayerID = {INT}", nID, nID, nID)) {
			if(IDNO == MessageBox("This code is in use as a HCFA Payer ID, UB Payer ID, or an Eligibility Payer ID on at least one insurance company / location combination.\n"
				"Deleting this code will remove it from each company and location that uses it. Are you sure you wish to continue?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}

		CString strSqlBatch;

		// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer IDs, and thus removed format type
		
		// (j.jones 2008-09-15 09:32) - PLID 31374 - update any company that may have this code in use
		// (j.jones 2012-08-07 11:10) - PLID 51914 - supported the payer ID structure change
		_RecordsetPtr rs = CreateParamRecordset("SELECT PersonID, Name, "
			"CASE WHEN HCFAPayerID = {INT} THEN 1 ELSE 0 END AS IsEbilling, "
			"CASE WHEN EligPayerID = {INT} THEN 1 ELSE 0 END AS IsEligibility, "
			"CASE WHEN UBPayerID = {INT} THEN 1 ELSE 0 END AS IsUB "
			"FROM InsuranceCoT WHERE HCFAPayerID = {INT} OR EligPayerID = {INT} OR UBPayerID = {INT}",
			nID, nID, nID, nID, nID, nID);
		BOOL bFoundEbilling = FALSE;
		BOOL bFoundEligibility = FALSE;
		BOOL bFoundUB = FALSE;
		while(!rs->eof) {

			long nPersonID = AdoFldLong(rs, "PersonID", -1);
			CString strName = AdoFldString(rs, "Name", "");
			long nIsEbilling = AdoFldLong(rs, "IsEbilling", 0);
			long nIsEligibility = AdoFldLong(rs, "IsEligibility", 0);
			long nIsUB = AdoFldLong(rs, "IsUB", 0);

			if(nIsEbilling == 1) {
				bFoundEbilling = TRUE;

				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(-1, strName, nAuditTransactionID, aeiInsCoPayer, nPersonID, strPayerID, "", aepMedium, aetChanged);
			}
			//should not be an else/if, it can be assigned to more than one
			if(nIsEligibility == 1) {
				bFoundEligibility = TRUE;

				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(-1, strName, nAuditTransactionID, aeiInsCoEligibility, nPersonID, strPayerID, "", aepMedium, aetChanged);
			}
			//should not be an else/if, it can be assigned to more than one
			if(nIsUB == 1) {
				bFoundUB = TRUE;

				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(-1, strName, nAuditTransactionID, aeiInsCoUBPayerID, nPersonID, strPayerID, "", aepMedium, aetChanged);
			}

			rs->MoveNext();
		}
		rs->Close();

		//if anything was found, just run one update statement
		// (j.jones 2012-08-07 11:10) - PLID 51914 - supported the payer ID structure change
		if(bFoundEbilling) {
			AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceCoT SET HCFAPayerID = NULL WHERE HCFAPayerID = %li", nID);
		}
		if(bFoundEligibility) {
			AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceCoT SET EligPayerID = NULL WHERE EligPayerID = %li", nID);
		}
		// (j.jones 2009-12-16 16:05) - PLID 36237 - supported UB Payer IDs
		if(bFoundUB) {
			AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceCoT SET UBPayerID = NULL WHERE UBPayerID = %li", nID);
		}

		// (j.jones 2009-08-05 08:42) - PLID 34467 - handle InsuranceLocationPayerIDsT
		// (j.jones 2009-12-16 16:05) - PLID 36237 - supported UB Payer IDs, and streamlined this code
		// to mimic the more efficient InsuranceCoT-checking code

		bFoundEbilling = FALSE;
		bFoundEligibility = FALSE;
		bFoundUB = FALSE;
		rs = CreateParamRecordset("SELECT InsuranceCoT.PersonID AS ID, "
			"InsuranceCoT.Name AS InsCoName, LocationsT.Name AS LocName, "
			"CASE WHEN InsuranceLocationPayerIDsT.ClaimPayerID = {INT} THEN 1 ELSE 0 END AS IsEbilling, "
			"CASE WHEN InsuranceLocationPayerIDsT.EligibilityPayerID = {INT} THEN 1 ELSE 0 END AS IsEligibility, "
			"CASE WHEN InsuranceLocationPayerIDsT.UBPayerID = {INT} THEN 1 ELSE 0 END AS IsUB "
			"FROM InsuranceLocationPayerIDsT "
			"INNER JOIN InsuranceCoT ON InsuranceLocationPayerIDsT.InsuranceCoID = InsuranceCoT.PersonID "
			"INNER JOIN LocationsT ON InsuranceLocationPayerIDsT.LocationID = LocationsT.ID "
			"WHERE InsuranceLocationPayerIDsT.ClaimPayerID = {INT} "
			"OR InsuranceLocationPayerIDsT.EligibilityPayerID = {INT} "
			"OR InsuranceLocationPayerIDsT.UBPayerID = {INT} ",
			nID, nID, nID, nID, nID, nID);
		while(!rs->eof) {

			long nInsCoID = AdoFldLong(rs, "ID");
			CString strInsCoName = AdoFldString(rs, "InsCoName", "");
			CString strLocName = AdoFldString(rs, "LocName", "");
			long nIsEbilling = AdoFldLong(rs, "IsEbilling", 0);
			long nIsEligibility = AdoFldLong(rs, "IsEligibility", 0);
			long nIsUB = AdoFldLong(rs, "IsUB", 0);

			CString strOldValue, strNewValue;

			strOldValue.Format("%s: %s", strLocName, strPayerID);
			strNewValue = "<Use Default>";

			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			if(nIsEbilling) {
				AuditEvent(-1, strInsCoName, nAuditTransactionID, aeiInsCoLocationClaimPayerID, nInsCoID, strOldValue, strNewValue, aepMedium, aetChanged);
			}
			if(nIsEligibility) {
				AuditEvent(-1, strInsCoName, nAuditTransactionID, aeiInsCoLocationEligibilityPayerID, nInsCoID, strOldValue, strNewValue, aepMedium, aetChanged);
			}
			if(nIsUB) {
				AuditEvent(-1, strInsCoName, nAuditTransactionID, aeiInsCoLocationUBPayerID, nInsCoID, strOldValue, strNewValue, aepMedium, aetChanged);
			}

			rs->MoveNext();
		}
		rs->Close();

		AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceLocationPayerIDsT SET ClaimPayerID = NULL WHERE ClaimPayerID = %li", nID);
		AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceLocationPayerIDsT SET EligibilityPayerID = NULL WHERE EligibilityPayerID = %li", nID);
		// (j.jones 2009-12-16 16:05) - PLID 36237 - supported UB Payer IDs
		AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceLocationPayerIDsT SET UBPayerID = NULL WHERE UBPayerID = %li", nID);
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM InsuranceLocationPayerIDsT WHERE ClaimPayerID Is Null AND EligibilityPayerID Is Null");

		//now remove the code
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EBillingInsCoIDs WHERE ID = %li", VarLong(m_PayerList->GetValue(CurSel,plcID)));

		ExecuteSqlBatch(strSqlBatch);

		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
			nAuditTransactionID = -1;
		}

		m_PayerList->RemoveRow(CurSel);
		RefreshButtons();
	}NxCatchAllCall("Error deleting payer.",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

BOOL CEditInsPayersDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-29 10:02) - PLID 29817 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnAdvanced.AutoSet(NXB_MODIFY);
		
		m_PayerList = BindNxDataListCtrl(this,IDC_INS_PAYERS,GetRemoteData(),true);

		RefreshButtons();
	}
	NxCatchAll("Error in CEditInsPayersDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CEditInsPayersDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditInsPayersDlg)
	ON_EVENT(CEditInsPayersDlg, IDC_INS_PAYERS, 10 /* EditingFinished */, OnEditingFinishedInsPayers, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEditInsPayersDlg, IDC_INS_PAYERS, 9 /* EditingFinishing */, OnEditingFinishingInsPayers, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEditInsPayersDlg, IDC_INS_PAYERS, 2 /* SelChanged */, OnSelChangedInsPayers, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEditInsPayersDlg::OnEditingFinishedInsPayers(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if(nRow == -1) {
		return;
	}

	long nAuditTransactionID = -1;

	try {

		if(!bCommit) {
			return;
		}

		if(nCol == plcPayerID) {

			long nID = VarLong(m_PayerList->GetValue(nRow,plcID));
			CString strNewPayerID = VarString(varNewValue,"");
			CString strOldPayerID = VarString(varOldValue,"");

			if(strNewPayerID != strOldPayerID) {

				// (j.jones 2009-08-05 09:20) - PLID 34467 - don't need to update InsuranceLocationPayerIDsT, but we do need to audit
				_RecordsetPtr rs = CreateParamRecordset("SELECT InsuranceCoT.PersonID AS ID, "
					"InsuranceCoT.Name AS InsCoName, LocationsT.Name AS LocName "
					"FROM InsuranceLocationPayerIDsT "
					"INNER JOIN InsuranceCoT ON InsuranceLocationPayerIDsT.InsuranceCoID = InsuranceCoT.PersonID "
					"INNER JOIN LocationsT ON InsuranceLocationPayerIDsT.LocationID = LocationsT.ID "
					"WHERE InsuranceLocationPayerIDsT.ClaimPayerID = {INT}", nID);
				while(!rs->eof) {

					long nInsCoID = AdoFldLong(rs, "ID");
					CString strInsCoName = AdoFldString(rs, "InsCoName", "");
					CString strLocName = AdoFldString(rs, "LocName", "");

					CString strOldValue, strNewValue;

					strOldValue.Format("%s: %s", strLocName, strOldPayerID);
					strNewValue = strNewPayerID;

					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(-1, strInsCoName, nAuditTransactionID, aeiInsCoLocationClaimPayerID, nInsCoID, strOldValue, strNewValue, aepMedium, aetChanged);

					rs->MoveNext();
				}
				rs->Close();

				rs = CreateParamRecordset("SELECT InsuranceCoT.PersonID AS ID, "
					"InsuranceCoT.Name AS InsCoName, LocationsT.Name AS LocName "
					"FROM InsuranceLocationPayerIDsT "
					"INNER JOIN InsuranceCoT ON InsuranceLocationPayerIDsT.InsuranceCoID = InsuranceCoT.PersonID "
					"INNER JOIN LocationsT ON InsuranceLocationPayerIDsT.LocationID = LocationsT.ID "
					"WHERE InsuranceLocationPayerIDsT.EligibilityPayerID = {INT}", nID);
				while(!rs->eof) {

					long nInsCoID = AdoFldLong(rs, "ID");
					CString strInsCoName = AdoFldString(rs, "InsCoName", "");
					CString strLocName = AdoFldString(rs, "LocName", "");

					CString strOldValue, strNewValue;

					strOldValue.Format("%s: %s", strLocName, strOldPayerID);
					strNewValue = strNewPayerID;

					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(-1, strInsCoName, nAuditTransactionID, aeiInsCoLocationEligibilityPayerID, nInsCoID, strOldValue, strNewValue, aepMedium, aetChanged);

					rs->MoveNext();
				}
				rs->Close();

				CString strSqlBatch;
				AddStatementToSqlBatch(strSqlBatch, "UPDATE EBillingInsCoIDs SET EBillingID = '%s' WHERE ID = %li",_Q(strNewPayerID),m_PayerList->GetValue(nRow, plcID).lVal);

				// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer IDs, and thus removed format type
				
				// (j.jones 2008-09-15 09:24) - PLID 31374 - update any company that may have this code in use
				// (j.jones 2012-08-07 11:10) - PLID 51914 - supported the payer ID structure change, now we don't
				// need to update each record, but we can at least keep the existing auditing code
				rs = CreateParamRecordset("SELECT PersonID, Name, "
					"CASE WHEN HCFAPayerID = {INT} THEN 1 ELSE 0 END AS IsEbilling, "
					"CASE WHEN EligPayerID = {INT} THEN 1 ELSE 0 END AS IsEligibility, "
					"CASE WHEN UBPayerID = {INT} THEN 1 ELSE 0 END AS IsUB "
					"FROM InsuranceCoT WHERE HCFAPayerID = {INT} OR EligPayerID = {INT} OR UBPayerID = {INT}",
					nID, nID, nID, nID, nID, nID);
				while(!rs->eof) {

					long nPersonID = AdoFldLong(rs, "PersonID", -1);
					CString strName = AdoFldString(rs, "Name", "");
					long nIsEbilling = AdoFldLong(rs, "IsEbilling", 0);
					long nIsEligibility = AdoFldLong(rs, "IsEligibility", 0);
					long nIsUB = AdoFldLong(rs, "IsUB", 0);

					if(nIsEbilling == 1) {
						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						AuditEvent(-1, strName, nAuditTransactionID, aeiInsCoPayer, nPersonID, strOldPayerID, strNewPayerID, aepMedium, aetChanged);
					}
					if(nIsEligibility == 1) {
						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						AuditEvent(-1, strName, nAuditTransactionID, aeiInsCoEligibility, nPersonID, strOldPayerID, strNewPayerID, aepMedium, aetChanged);
					}
					if(nIsUB == 1) {
						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						AuditEvent(-1, strName, nAuditTransactionID, aeiInsCoUBPayerID, nPersonID, strOldPayerID, strNewPayerID, aepMedium, aetChanged);
					}

					rs->MoveNext();
				}
				rs->Close();

				ExecuteSqlBatch(strSqlBatch);

				if(nAuditTransactionID != -1) {
					CommitAuditTransaction(nAuditTransactionID);
					nAuditTransactionID = -1;
				}
			}
		}
		else if(nCol == plcName) {
			ExecuteSql("UPDATE EBillingInsCoIDs SET InsCo = '%s' WHERE ID = %li",_Q(CString(varNewValue.bstrVal)),m_PayerList->GetValue(nRow, plcID).lVal);
		}
	}NxCatchAllCall("Error editing payer.",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

void CEditInsPayersDlg::OnEditingFinishingInsPayers(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	if(nRow == -1)
		return;

	CString str = strUserEntered;

	str.TrimRight();
	
	if(str == "") {
		AfxMessageBox("You cannot enter a blank value.");		
		// (a.walling 2010-08-16 17:08) - PLID 40131 - Fix leak and crash
		VariantClear(pvarNewValue);
		*pvarNewValue = _variant_t(varOldValue).Detach();
		*pbCommit = FALSE;
	}

	long nID = VarLong(m_PayerList->GetValue(nRow,plcID));

	// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer IDs, and thus removed format type

	// (j.jones 2008-09-15 09:19) - PLID 31374 - warn if the code is in use
	if(nCol == plcPayerID && VarString(pvarNewValue) != VarString(varOldValue)) {
		// (j.jones 2012-08-07 11:10) - PLID 51914 - supported the payer ID structure change
		if(ReturnsRecordsParam("SELECT PersonID FROM InsuranceCoT WHERE HCFAPayerID = {INT} OR EligPayerID = {INT} OR UBPayerID = {INT}", nID, nID, nID)) {
			if(IDNO == MessageBox("This code is in use as a HCFA Payer ID, UB Payer ID, or an Eligibility Payer ID on at least one insurance company.\n"
				"Changing this code will change it for each company that uses it. Are you sure you wish to continue?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				
				// (a.walling 2010-08-16 17:08) - PLID 40131 - Fix leak and crash
				VariantClear(pvarNewValue);
				*pvarNewValue = _variant_t(varOldValue).Detach();

				*pbCommit = FALSE;
			}
		}
		// (j.jones 2009-08-05 09:16) - PLID 34467 - warn if the code is in use by a insurance / location combination,
		// but don't bother warning if they agreed to the previous warning
		else if(ReturnsRecordsParam("SELECT InsuranceCoID FROM InsuranceLocationPayerIDsT "
			"WHERE ClaimPayerID = {INT} OR EligibilityPayerID = {INT} OR UBPayerID = {INT}", nID, nID, nID)) {
			if(IDNO == MessageBox("This code is in use as a HCFA Payer ID, UB Payer ID, or an Eligibility Payer ID on at least one insurance company / location combination.\n"
				"Changing this code will change it for each company and location that uses it. Are you sure you wish to continue?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				
				// (a.walling 2010-08-16 17:08) - PLID 40131 - Fix leak and crash
				VariantClear(pvarNewValue);
				*pvarNewValue = _variant_t(varOldValue).Detach();

				*pbCommit = FALSE;
			}
		}
	}
}

void CEditInsPayersDlg::OnBtnAdvanced() 
{
	try {

		// (j.jones 2012-05-15 14:27) - PLID 47255 - this is now a menu of options

		enum {
			eAdvSettingDlg = -100,
			eCopyHCFAToUB = -101,
			eCopyHCFAToElig = -102,
			eCopyUBToHCFA = -103,
			eCopyUBToElig = -104,
			eCopyEligToHCFA = -105,
			eCopyEligToUB = -106,
		};

		CMenu mnuMain;
		mnuMain.CreatePopupMenu();
		mnuMain.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eAdvSettingDlg, "Apply Payer IDs For Multiple Insurance Companies...");

		CMenu pCopyHCFAMenu;
		pCopyHCFAMenu.CreatePopupMenu();
		pCopyHCFAMenu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eCopyHCFAToUB, "UB Payer ID");
		pCopyHCFAMenu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eCopyHCFAToElig, "Eligibility Payer ID");
		mnuMain.AppendMenu(MF_ENABLED|MF_POPUP, (UINT)pCopyHCFAMenu.m_hMenu, "Copy HCFA Payer IDs to...");

		CMenu pCopyUBMenu;
		pCopyUBMenu.CreatePopupMenu();
		pCopyUBMenu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eCopyUBToHCFA, "HCFA Payer ID");
		pCopyUBMenu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eCopyUBToElig, "Eligibility Payer ID");
		mnuMain.AppendMenu(MF_ENABLED|MF_POPUP, (UINT)pCopyUBMenu.m_hMenu, "Copy UB Payer IDs to...");

		CMenu pCopyEligMenu;
		pCopyEligMenu.CreatePopupMenu();
		pCopyEligMenu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eCopyEligToHCFA, "HCFA Payer ID");
		pCopyEligMenu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eCopyEligToUB, "UB Payer ID");
		mnuMain.AppendMenu(MF_ENABLED|MF_POPUP, (UINT)pCopyEligMenu.m_hMenu, "Copy Eligibility Payer IDs to...");

		CRect rc;
		m_btnAdvanced.GetWindowRect(rc);
		int nCmdId = mnuMain.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, rc.right, rc.top, this, NULL);	
		switch(nCmdId){
			case eAdvSettingDlg: {
				//open the dialog
				CAdvPayerIDDlg dlg(this);
				dlg.DoModal();
				break;
			}
			case eCopyHCFAToUB:
				// (j.jones 2012-05-15 14:27) - PLID 47255 - copy the HCFA payer ID to UB payer ID
				if(IDYES == MessageBox("This will copy each insurance company's HCFA Payer ID to the UB Payer ID field, "
					"if a HCFA Payer ID is entered and no UB Payer ID is entered.\n\n"
					"Are you sure you wish to update the UB Payer ID for these companies?", "Practice", MB_ICONQUESTION|MB_YESNO)) {

					long nRecordsAffected = 0;
					// (j.jones 2012-08-07 11:10) - PLID 51914 - supported the payer ID structure change,
					// which greatly simplified this query
					ExecuteParamSql(GetRemoteData(), &nRecordsAffected, "UPDATE InsuranceCoT SET InsuranceCoT.UBPayerID = InsuranceCoT.HCFAPayerID "
						"FROM InsuranceCoT "
						"WHERE UBPayerID Is Null AND InsuranceCoT.HCFAPayerID Is Not Null");

					CString strNewValue;

					//give a success message
					if(nRecordsAffected == 1) {
						strNewValue = "One insurance company was updated to use its HCFA Payer ID as its UB Payer ID.";
						AfxMessageBox(strNewValue);
					}
					else if(nRecordsAffected > 0) {
						strNewValue.Format("%li insurance companies were updated to use their HCFA Payer ID as their UB Payer ID.", nRecordsAffected);
						AfxMessageBox(strNewValue);
					}
					else {
						//tell them nothing changed
						AfxMessageBox("No insurance companies have a blank UB Payer ID and a filled HCFA Payer ID. No changes were made.");
					}

					//audit
					if(nRecordsAffected > 0) {
						long nAuditID = BeginNewAuditEvent();
						AuditEvent(-1, "", nAuditID, aeiInsuranceMassUpdatePayerIDs, -1, "", strNewValue, aepMedium, aetChanged);
					}
				}
				break;
			case eCopyHCFAToElig:
				// (j.jones 2012-05-15 14:27) - PLID 47255 - copy the HCFA payer ID to Eligibility payer ID
				if(IDYES == MessageBox("This will copy each insurance company's HCFA Payer ID to the Eligibility Payer ID field, "
					"if a HCFA Payer ID is entered and no Eligibility Payer ID is entered.\n\n"
					"Are you sure you wish to update the Eligibility Payer ID for these companies?", "Practice", MB_ICONQUESTION|MB_YESNO)) {

					long nRecordsAffected = 0;
					// (j.jones 2012-08-07 11:10) - PLID 51914 - supported the payer ID structure change
					ExecuteParamSql(GetRemoteData(), &nRecordsAffected, "UPDATE InsuranceCoT SET InsuranceCoT.EligPayerID = InsuranceCoT.HCFAPayerID "
						"FROM InsuranceCoT "
						"WHERE EligPayerID Is Null AND InsuranceCoT.HCFAPayerID Is Not Null");

					CString strNewValue;

					//give a success message
					if(nRecordsAffected == 1) {
						strNewValue = "One insurance company was updated to use its HCFA Payer ID as its Eligibility Payer ID.";
						AfxMessageBox(strNewValue);
					}
					else if(nRecordsAffected > 0) {
						strNewValue.Format("%li insurance companies were updated to use their HCFA Payer ID as their Eligibility Payer ID.", nRecordsAffected);
						AfxMessageBox(strNewValue);
					}
					else {
						//tell them nothing changed
						AfxMessageBox("No insurance companies have a blank Eligibility Payer ID and a filled HCFA Payer ID. No changes were made.");
					}

					//audit
					if(nRecordsAffected > 0) {
						long nAuditID = BeginNewAuditEvent();
						AuditEvent(-1, "", nAuditID, aeiInsuranceMassUpdatePayerIDs, -1, "", strNewValue, aepMedium, aetChanged);
					}
				}
				break;
			case eCopyUBToHCFA:
				// (j.jones 2012-05-15 14:27) - PLID 47255 - copy the UB payer ID to HCFA payer ID
				if(IDYES == MessageBox("This will copy each insurance company's UB Payer ID to the HCFA Payer ID field, "
					"if a UB Payer ID is entered and no HCFA Payer ID is entered.\n\n"
					"Are you sure you wish to update the HCFA Payer ID for these companies?", "Practice", MB_ICONQUESTION|MB_YESNO)) {

					long nRecordsAffected = 0;
					// (j.jones 2012-08-07 11:10) - PLID 51914 - supported the payer ID structure change
					ExecuteParamSql(GetRemoteData(), &nRecordsAffected, "UPDATE InsuranceCoT SET InsuranceCoT.HCFAPayerID = InsuranceCoT.UBPayerID "
						"FROM InsuranceCoT "
						"WHERE HCFAPayerID Is Null AND InsuranceCoT.UBPayerID Is Not Null");

					CString strNewValue;

					//give a success message
					if(nRecordsAffected == 1) {
						strNewValue = "One insurance company was updated to use its UB Payer ID as its HCFA Payer ID.";
						AfxMessageBox(strNewValue);
					}
					else if(nRecordsAffected > 0) {
						strNewValue.Format("%li insurance companies were updated to use their UB Payer ID as their HCFA Payer ID.", nRecordsAffected);
						AfxMessageBox(strNewValue);
					}
					else {
						//tell them nothing changed
						AfxMessageBox("No insurance companies have a blank HCFA Payer ID and a filled UB Payer ID. No changes were made.");
					}

					//audit
					if(nRecordsAffected > 0) {
						long nAuditID = BeginNewAuditEvent();
						AuditEvent(-1, "", nAuditID, aeiInsuranceMassUpdatePayerIDs, -1, "", strNewValue, aepMedium, aetChanged);
					}
				}
				break;
			case eCopyUBToElig:
				// (j.jones 2012-05-15 14:27) - PLID 47255 - copy the UB payer ID to Eligibility payer ID
				if(IDYES == MessageBox("This will copy each insurance company's UB Payer ID to the Eligibility Payer ID field, "
					"if a UB Payer ID is entered and no Eligibility Payer ID is entered.\n\n"
					"Are you sure you wish to update the Eligibility Payer ID for these companies?", "Practice", MB_ICONQUESTION|MB_YESNO)) {

					long nRecordsAffected = 0;
					// (j.jones 2012-08-07 11:10) - PLID 51914 - supported the payer ID structure change
					ExecuteParamSql(GetRemoteData(), &nRecordsAffected, "UPDATE InsuranceCoT SET InsuranceCoT.EligPayerID = InsuranceCoT.UBPayerID "
						"FROM InsuranceCoT "
						"WHERE EligPayerID Is Null AND InsuranceCoT.UBPayerID Is Not Null");

					CString strNewValue;

					//give a success message
					if(nRecordsAffected == 1) {
						strNewValue = "One insurance company was updated to use its UB Payer ID as its Eligibility Payer ID.";
						AfxMessageBox(strNewValue);
					}
					else if(nRecordsAffected > 0) {
						strNewValue.Format("%li insurance companies were updated to use their UB Payer ID as their Eligibility Payer ID.", nRecordsAffected);
						AfxMessageBox(strNewValue);
					}
					else {
						//tell them nothing changed
						AfxMessageBox("No insurance companies have a blank Eligibility Payer ID and a filled UB Payer ID. No changes were made.");
					}

					//audit
					if(nRecordsAffected > 0) {
						long nAuditID = BeginNewAuditEvent();
						AuditEvent(-1, "", nAuditID, aeiInsuranceMassUpdatePayerIDs, -1, "", strNewValue, aepMedium, aetChanged);
					}
				}
				break;
			case eCopyEligToHCFA:
				// (j.jones 2012-05-15 14:27) - PLID 47255 - copy the Eligibility payer ID to HCFA payer ID
				if(IDYES == MessageBox("This will copy each insurance company's Eligibility Payer ID to the HCFA Payer ID field, "
					"if an Eligibility Payer ID is entered and no HCFA Payer ID is entered.\n\n"
					"Are you sure you wish to update the HCFA Payer ID for these companies?", "Practice", MB_ICONQUESTION|MB_YESNO)) {

					long nRecordsAffected = 0;
					// (j.jones 2012-08-07 11:10) - PLID 51914 - supported the payer ID structure change
					ExecuteParamSql(GetRemoteData(), &nRecordsAffected, "UPDATE InsuranceCoT SET InsuranceCoT.HCFAPayerID = InsuranceCoT.EligPayerID "
						"FROM InsuranceCoT "
						"WHERE HCFAPayerID Is Null AND InsuranceCoT.EligPayerID Is Not Null");

					CString strNewValue;

					//give a success message
					if(nRecordsAffected == 1) {
						strNewValue = "One insurance company was updated to use its Eligibility Payer ID as its HCFA Payer ID.";
						AfxMessageBox(strNewValue);
					}
					else if(nRecordsAffected > 0) {
						strNewValue.Format("%li insurance companies were updated to use their Eligibility Payer ID as their HCFA Payer ID.", nRecordsAffected);
						AfxMessageBox(strNewValue);
					}
					else {
						//tell them nothing changed
						AfxMessageBox("No insurance companies have a blank HCFA Payer ID and a filled Eligibility Payer ID. No changes were made.");
					}

					//audit
					if(nRecordsAffected > 0) {
						long nAuditID = BeginNewAuditEvent();
						AuditEvent(-1, "", nAuditID, aeiInsuranceMassUpdatePayerIDs, -1, "", strNewValue, aepMedium, aetChanged);
					}
				}
				break;
			case eCopyEligToUB:
				// (j.jones 2012-05-15 14:27) - PLID 47255 - copy the Eligibility payer ID to UB payer ID
				if(IDYES == MessageBox("This will copy each insurance company's Eligibility Payer ID to the UB Payer ID field, "
					"if an Eligibility Payer ID is entered and no UB Payer ID is entered.\n\n"
					"Are you sure you wish to update the UB Payer ID for these companies?", "Practice", MB_ICONQUESTION|MB_YESNO)) {

					long nRecordsAffected = 0;
					// (j.jones 2012-08-07 11:10) - PLID 51914 - supported the payer ID structure change
					ExecuteParamSql(GetRemoteData(), &nRecordsAffected, "UPDATE InsuranceCoT SET InsuranceCoT.UBPayerID = InsuranceCoT.EligPayerID "
						"FROM InsuranceCoT "
						"WHERE UBPayerID Is Null AND InsuranceCoT.EligPayerID Is Not Null");

					CString strNewValue;

					//give a success message
					if(nRecordsAffected == 1) {
						strNewValue = "One insurance company was updated to use its Eligibility Payer ID as its UB Payer ID.";
						AfxMessageBox(strNewValue);
					}
					else if(nRecordsAffected > 0) {
						strNewValue.Format("%li insurance companies were updated to use their Eligibility Payer ID as their UB Payer ID.", nRecordsAffected);
						AfxMessageBox(strNewValue);
					}
					else {
						//tell them nothing changed
						AfxMessageBox("No insurance companies have a blank UB Payer ID and a filled Eligibility Payer ID. No changes were made.");
					}

					//audit
					if(nRecordsAffected > 0) {
						long nAuditID = BeginNewAuditEvent();
						AuditEvent(-1, "", nAuditID, aeiInsuranceMassUpdatePayerIDs, -1, "", strNewValue, aepMedium, aetChanged);
					}
				}
				break;
		}
	}NxCatchAll(__FUNCTION__);
}

void CEditInsPayersDlg::OnSelChangedInsPayers(long nNewSel) 
{
	RefreshButtons();
}

void CEditInsPayersDlg::RefreshButtons() 
{
	if(m_PayerList->GetCurSel() == sriNoRow){
		GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
	}
	else{
		GetDlgItem(IDC_DELETE)->EnableWindow(TRUE);
	}
}

