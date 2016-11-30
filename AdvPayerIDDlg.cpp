// AdvPayerIDDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdvPayerIDDlg.h"
#include "AuditTrail.h"
#include "GlobalFinancialUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CAdvPayerIDDlg dialog

// (j.jones 2008-09-09 10:58) - PLID 18695 - added Ins. Type enum
enum InsTypeCombo {

	itcID = 0,
	itcName,
	itcANSI,
	itcNSF,
};

// (j.jones 2009-08-05 14:49) - PLID 31417 - added enum for the insurance columns
enum InsuranceListColumn {
	ilcID = 0,
	ilcName,
	ilcHCFAPayerIDCode,
	ilcEligibilityPayerIDCode,
	// (j.jones 2009-12-16 16:40) - PLID 36621 - added UB payer ID
	ilcUBPayerIDCode,
	ilcInsType,
};

CAdvPayerIDDlg::CAdvPayerIDDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAdvPayerIDDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAdvPayerIDDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAdvPayerIDDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvPayerIDDlg)
	DDX_Control(pDX, IDC_APPLY_ELIGIBILITY, m_btnApplyEligibility);
	DDX_Control(pDX, IDC_UNSELECT_ALL_INSCO, m_btnUnselectAllInsCo);
	DDX_Control(pDX, IDC_UNSELECT_ONE_INSCO, m_btnUnselectOneInsCo);
	DDX_Control(pDX, IDC_SELECT_ALL_INSCO, m_btnSelectAllInsCo);
	DDX_Control(pDX, IDC_SELECT_ONE_INSCO, m_btnSelectOneInsCo);
	DDX_Control(pDX, IDC_APPLY_PAYER, m_btnApplyPayer);
	//DDX_Control(pDX, IDC_APPLY_THIN, m_btnApplyThin);
	DDX_Control(pDX, IDC_APPLY_INS_TYPE, m_btnApplyInsType);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_INS_GROUPBOX, m_btnInsGroupbox);
	DDX_Control(pDX, IDC_APPLY_PAYER_LOC, m_btnApplyClaimLoc);
	DDX_Control(pDX, IDC_APPLY_ELIGIBILITY_LOC, m_btnApplyEligibilityLoc);
	DDX_Control(pDX, IDC_APPLY_UB_PAYER, m_btnApplyUB);
	DDX_Control(pDX, IDC_APPLY_UB_PAYER_LOC, m_btnApplyUBLoc);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAdvPayerIDDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAdvPayerIDDlg)
	ON_BN_CLICKED(IDC_SELECT_ONE_INSCO, OnSelectOneInsco)
	ON_BN_CLICKED(IDC_SELECT_ALL_INSCO, OnSelectAllInsco)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_INSCO, OnUnselectOneInsco)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_INSCO, OnUnselectAllInsco)
	ON_BN_CLICKED(IDC_APPLY_PAYER, OnApplyPayer)
	//ON_BN_CLICKED(IDC_APPLY_THIN, OnApplyThin)
	ON_BN_CLICKED(IDC_APPLY_INS_TYPE, OnApplyInsType)
	ON_BN_CLICKED(IDC_APPLY_ELIGIBILITY, OnApplyEligibility)
	ON_BN_CLICKED(IDC_APPLY_PAYER_LOC, OnApplyPayerLoc)
	ON_BN_CLICKED(IDC_APPLY_ELIGIBILITY_LOC, OnApplyEligibilityLoc)
	ON_BN_CLICKED(IDC_APPLY_UB_PAYER, OnBnClickedApplyUbPayer)
	ON_BN_CLICKED(IDC_APPLY_UB_PAYER_LOC, OnBnClickedApplyUbPayerLoc)
	//}}AFX_MSG_MAP		
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvPayerIDDlg message handlers

BOOL CAdvPayerIDDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-30 13:20) - PLID 29847 - NxIconify buttons
		m_btnApplyPayer.AutoSet(NXB_MODIFY);
		m_btnApplyEligibility.AutoSet(NXB_MODIFY);
		// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer ID
		//m_btnApplyThin.AutoSet(NXB_MODIFY);
		m_btnApplyInsType.AutoSet(NXB_MODIFY);
		m_btnOK.AutoSet(NXB_CLOSE);
		// (j.jones 2009-08-05 12:20) - PLID 35109 - added abilities to apply per location
		m_btnApplyClaimLoc.AutoSet(NXB_MODIFY);
		m_btnApplyEligibilityLoc.AutoSet(NXB_MODIFY);
		// (j.jones 2009-12-16 16:53) - PLID 36621 - added UB payer ID
		m_btnApplyUB.AutoSet(NXB_MODIFY);
		m_btnApplyUBLoc.AutoSet(NXB_MODIFY);
		
		m_UnselectedInsCoList = BindNxDataListCtrl(this,IDC_UNSELECTED_INS_LIST,GetRemoteData(),true);
		m_SelectedInsCoList = BindNxDataListCtrl(this,IDC_SELECTED_INS_LIST,GetRemoteData(),false);
		m_pEnvoyList = BindNxDataListCtrl(this,IDC_ENVOY,GetRemoteData(),true);
		// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer ID
		//m_pTHINList = BindNxDataListCtrl(this,IDC_THIN,GetRemoteData(),true);
		m_pInsTypeList = BindNxDataListCtrl(this,IDC_INS_TYPE_COMBO,GetRemoteData(),false);
		// (j.jones 2008-09-18 10:00) - PLID 31138 - added eligibility list
		m_pEligibilityList = BindNxDataListCtrl(this,IDC_ELIGIBILITY_COMBO,GetRemoteData(),true);
		// (j.jones 2009-08-05 12:23) - PLID 35109 - added location combo
		m_pLocationCombo = BindNxDataList2Ctrl(this,IDC_LOCATIONS_ADV_PAYER_ID_COMBO,GetRemoteData(),true);
		// (j.jones 2009-12-16 16:53) - PLID 36621 - added UB payer ID
		m_pUBPayerList = BindNxDataListCtrl(this,IDC_UB_PAYER_LIST,GetRemoteData(),true);

		m_pLocationCombo->SetSelByColumn(0, GetCurrentLocationID());

		m_btnSelectOneInsCo.AutoSet(NXB_RIGHT);
		m_btnSelectAllInsCo.AutoSet(NXB_RRIGHT);
		m_btnUnselectOneInsCo.AutoSet(NXB_LEFT);
		m_btnUnselectAllInsCo.AutoSet(NXB_LLEFT);

		// (j.jones 2008-09-09 10:58) - PLID 18695 - I re-worked this dropdown completely to show the type name,
		// ANSI code, and NSF code.
		///***If you ever add a new row to this list, make sure it is reflected in the GlobalFinancialUtils
		//enum for InsuranceTypeCode, and the functions GetANSISBR09CodeFromInsuranceType(),
		//GetNameFromInsuranceType(), and the EditInsuranceList
		AddNewRowToInsuranceTypeList(itcSelfPay);
		AddNewRowToInsuranceTypeList(itcCentralCertification);
		AddNewRowToInsuranceTypeList(itcOtherNonFederalPrograms);
		AddNewRowToInsuranceTypeList(itcPPO);
		AddNewRowToInsuranceTypeList(itcPOS);
		AddNewRowToInsuranceTypeList(itcEPO);
		AddNewRowToInsuranceTypeList(itcIndemnityInsurance);
		AddNewRowToInsuranceTypeList(itcHMO_MedicareRisk);
		AddNewRowToInsuranceTypeList(itcDentalMaintenanceOrganization);	// (j.jones 2010-10-15 14:36) - PLID 40953 - new for 5010
		AddNewRowToInsuranceTypeList(itcAutomobileMedical);
		AddNewRowToInsuranceTypeList(itcBCBS, TRUE);
		AddNewRowToInsuranceTypeList(itcChampus);
		AddNewRowToInsuranceTypeList(itcCommercial, TRUE);
		AddNewRowToInsuranceTypeList(itcCommercialSupplemental);	// (j.jones 2009-03-27 17:18) - PLID 33724
		AddNewRowToInsuranceTypeList(itcDisability);
		AddNewRowToInsuranceTypeList(itcFederalEmployeesProgram);	// (j.jones 2010-10-15 14:36) - PLID 40953 - new for 5010
		AddNewRowToInsuranceTypeList(itcHMO);
		AddNewRowToInsuranceTypeList(itcLiability);
		AddNewRowToInsuranceTypeList(itcLiabilityMedical);
		AddNewRowToInsuranceTypeList(itcMedicarePartA);				// (j.jones 2010-10-15 14:36) - PLID 40953 - new for 5010
		AddNewRowToInsuranceTypeList(itcMedicarePartB, TRUE);
		AddNewRowToInsuranceTypeList(itcMedicaid, TRUE);
		AddNewRowToInsuranceTypeList(itcOtherFederalProgram);
		AddNewRowToInsuranceTypeList(itcTitleV);
		AddNewRowToInsuranceTypeList(itcVeteranAdministrationPlan);
		AddNewRowToInsuranceTypeList(itcWorkersComp, TRUE);
		// (j.jones 2013-11-13 12:52) - PLID 58931 - added HRSA Other, for MU purposes
		AddNewRowToInsuranceTypeList(itcHRSAOther);
		AddNewRowToInsuranceTypeList(itcOther);
	}
	NxCatchAll("Error in CAdvPayerIDDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAdvPayerIDDlg::OnSelectOneInsco() 
{
	m_SelectedInsCoList->TakeCurrentRow(m_UnselectedInsCoList);
}

void CAdvPayerIDDlg::OnSelectAllInsco() 
{
	m_SelectedInsCoList->TakeAllRows(m_UnselectedInsCoList);	
}

void CAdvPayerIDDlg::OnUnselectOneInsco() 
{
	m_UnselectedInsCoList->TakeCurrentRow(m_SelectedInsCoList);	
}

void CAdvPayerIDDlg::OnUnselectAllInsco() 
{
	m_UnselectedInsCoList->TakeAllRows(m_SelectedInsCoList);
}

BEGIN_EVENTSINK_MAP(CAdvPayerIDDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAdvPayerIDDlg)
	ON_EVENT(CAdvPayerIDDlg, IDC_UNSELECTED_INS_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedInsList, VTS_I4 VTS_I2)
	ON_EVENT(CAdvPayerIDDlg, IDC_SELECTED_INS_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedInsList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CAdvPayerIDDlg::OnDblClickCellUnselectedInsList(long nRowIndex, short nColIndex) 
{
	m_UnselectedInsCoList->CurSel = nRowIndex;
	OnSelectOneInsco();	
}

void CAdvPayerIDDlg::OnDblClickCellSelectedInsList(long nRowIndex, short nColIndex) 
{
	m_SelectedInsCoList->CurSel = nRowIndex;
	OnUnselectOneInsco();	
}

void CAdvPayerIDDlg::OnApplyPayer() 
{
	long nAuditTransactionID = -1;

	try {

		if(m_SelectedInsCoList->GetRowCount() == 0) {
			AfxMessageBox("Please select at least one insurance company.");
			return;
		}

		if(m_pEnvoyList->GetCurSel() == -1) {
			AfxMessageBox("Please select a HCFA Payer ID.");
			return;
		}

		if(IDNO==MessageBox("This action will apply the selected HCFA Payer ID to all selected Insurance companies.\n"
			"Are you sure you wish to do this?","Practice",MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		// (j.jones 2012-08-08 14:21) - PLID 51915 - this now saves as an ID field, not a string
		long nNewPayerID = VarLong(m_pEnvoyList->GetValue(m_pEnvoyList->CurSel, 0), -1);
		CString strNewPayerID = VarString(m_pEnvoyList->GetValue(m_pEnvoyList->CurSel, 1), "");

		CWaitCursor pWait;

		CString strSqlBatch;

		CString strInsIDs;

		int i=0;
		for(i=0;i<m_SelectedInsCoList->GetRowCount();i++) {
				
			long nInsID = VarLong(m_SelectedInsCoList->GetValue(i,ilcID));
			
			// (j.jones 2012-08-08 14:13) - PLID 51915 - we now save the ID, not the code
			AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceCoT SET HCFAPayerID = %li WHERE PersonID = %li", nNewPayerID, nInsID);

			// (j.jones 2009-08-05 14:51) - PLID 31417 - audit this
			CString strOldPayerID = VarString(m_SelectedInsCoList->GetValue(i,ilcHCFAPayerIDCode), "");
			if(strOldPayerID != strNewPayerID) {
				CString strInsuranceCoName = VarString(m_SelectedInsCoList->GetValue(i,ilcName));

				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(-1, strInsuranceCoName, nAuditTransactionID, aeiInsCoPayer, nInsID, strOldPayerID, strNewPayerID, aepMedium, aetChanged);
			}

			// (j.jones 2009-08-05 13:53) - PLID 35109 - track a list of IDs of the insurance companies
			if(!strInsIDs.IsEmpty()) {
				strInsIDs += ",";
			}
			strInsIDs += AsString(nInsID);
		}

		// (j.jones 2009-08-05 13:58) - PLID 25109 - see if any of these insurance companies have a claim payer ID in use for any location
		if(!strInsIDs.IsEmpty()) {
			_RecordsetPtr rsLocCheck = CreateRecordset("SELECT EbillingInsCoIDs.EbillingID, "
				"InsuranceCoT.PersonID AS InsCoID, InsuranceCoT.Name AS InsCoName, LocationsT.Name AS LocName "
				"FROM InsuranceLocationPayerIDsT "
				"INNER JOIN InsuranceCoT ON InsuranceLocationPayerIDsT.InsuranceCoID = InsuranceCoT.PersonID "
				"INNER JOIN LocationsT ON InsuranceLocationPayerIDsT.LocationID = LocationsT.ID "
				"INNER JOIN EbillingInsCoIDs ON InsuranceLocationPayerIDsT.ClaimPayerID = EbillingInsCoIDs.ID "
				"WHERE ClaimPayerID Is Not Null "
				"AND InsuranceCoID IN (%s)", strInsIDs);
			if(!rsLocCheck->eof) {
				//if we have any results, prompt to clear
				int nRet = MessageBox("At least one of the insurance companies you are updating has a HCFA payer ID in use for a specific location.\n\n"
					"Do you wish to clear this information so these insurance companies will use the new HCFA payer ID for all locations?\n"
					"If not, the existing HCFA payer ID for the insurance company and location will remain unchanged.", "Practice", MB_ICONQUESTION|MB_YESNOCANCEL);
				
				if(nRet == IDCANCEL) {
					//abort saving
					strSqlBatch = "";

					// (j.jones 2009-08-05 14:21) - PLID 31417 - rollback our auditing
					if(nAuditTransactionID != -1) {
						RollbackAuditTransaction(nAuditTransactionID);
					}
					return;
				}
				else if(nRet == IDYES) {
					//update the data - remove rows that will now be blank, update the rest
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM InsuranceLocationPayerIDsT WHERE EligibilityPayerID Is Null AND UBPayerID Is Null AND InsuranceCoID IN (%s)", strInsIDs);
					AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceLocationPayerIDsT SET ClaimPayerID = NULL WHERE InsuranceCoID IN (%s)", strInsIDs);

					// (j.jones 2009-08-05 15:50) - PLID 31417 - audit this
					while(!rsLocCheck->eof) {
						CString strOldPayerID = AdoFldString(rsLocCheck, "EbillingID", "");
						if(strOldPayerID != "") {
							long nInsCoID = AdoFldLong(rsLocCheck, "InsCoID");
							CString strInsuranceCoName = AdoFldString(rsLocCheck, "InsCoName", "");
							CString strLocationName = AdoFldString(rsLocCheck, "LocName", "");
							
							CString strOldValue, strNewValue;

							strOldValue.Format("%s: %s", strLocationName, strOldPayerID);
							strNewValue = "<Use Default>";

							if(nAuditTransactionID == -1) {
								nAuditTransactionID = BeginAuditTransaction();
							}

							AuditEvent(-1, strInsuranceCoName, nAuditTransactionID, aeiInsCoLocationClaimPayerID, nInsCoID, strOldValue, strNewValue, aepMedium, aetChanged);
						}

						rsLocCheck->MoveNext();
					}
				}
			}
			rsLocCheck->Close();
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}

		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
			nAuditTransactionID = -1;
		}

		// (j.jones 2009-08-05 15:11) - PLID 31417 - now update the "old" values in the list
		for(i=0;i<m_SelectedInsCoList->GetRowCount();i++) {
			
			m_SelectedInsCoList->PutValue(i, ilcHCFAPayerIDCode, _bstr_t(strNewPayerID));
		}

		AfxMessageBox("Update complete.");

	}NxCatchAllCall("Error applying HCFA Payer ID.",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer ID

// (j.jones 2008-09-09 10:44) - PLID 18695 - converted to use InsType
void CAdvPayerIDDlg::OnApplyInsType() 
{
	long nAuditTransactionID = -1;

	try {

		if(m_SelectedInsCoList->GetRowCount() == 0) {
			AfxMessageBox("Please select at least one insurance company.");
			return;
		}

		if(m_pInsTypeList->GetCurSel() == -1) {
			AfxMessageBox("Please select an Insurance Type.");
			return;
		}

		if(IDNO==MessageBox("This action will apply the selected Insurance Type to all selected Insurance companies.\n"
			"Are you sure you wish to do this?","Practice",MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		InsuranceTypeCode eNewCode = (InsuranceTypeCode)VarLong(m_pInsTypeList->GetValue(m_pInsTypeList->CurSel, 0), (long)itcInvalid);

		CWaitCursor pWait;

		CString strSqlBatch;

		int i=0;
		for(i=0;i<m_SelectedInsCoList->GetRowCount();i++) {
				
			long nInsID = VarLong(m_SelectedInsCoList->GetValue(i,ilcID));
			
			CString strInsType = "NULL";			
			if(eNewCode != itcInvalid) {
				strInsType.Format("%li", (long)eNewCode);
			}
			AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceCoT SET InsType = %s WHERE PersonID = %li", strInsType,nInsID);

			// (j.jones 2009-08-05 14:51) - PLID 31417 - audit this
			InsuranceTypeCode eOldCode = (InsuranceTypeCode)VarLong(m_SelectedInsCoList->GetValue(i,ilcInsType), itcInvalid);
			if(eNewCode != eOldCode) {
				CString strInsuranceCoName = VarString(m_SelectedInsCoList->GetValue(i,ilcName));

				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(-1, strInsuranceCoName, nAuditTransactionID, aeiInsCoInsType, nInsID, GetNameFromInsuranceType(eOldCode), GetNameFromInsuranceType(eNewCode), aepMedium, aetChanged);
			}
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}

		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
			nAuditTransactionID = -1;
		}

		// (j.jones 2009-08-05 15:11) - PLID 31417 - now update the "old" values in the list
		for(i=0;i<m_SelectedInsCoList->GetRowCount();i++) {
			
			m_SelectedInsCoList->PutValue(i, ilcInsType, (long)eNewCode);
		}

		AfxMessageBox("Update complete.");	

	}NxCatchAllCall("Error applying Insurance Type.",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

// (j.jones 2008-09-09 08:47) - PLID 18695 - this function will add a new row to m_pInsuranceTypeList
void CAdvPayerIDDlg::AddNewRowToInsuranceTypeList(InsuranceTypeCode eCode, BOOL bColorize /*= FALSE*/)
{
	try {

		//call global functions for consistency
		CString strName = GetNameFromInsuranceType(eCode);
		// (j.jones 2010-10-15 14:47) - PLID 40953 - show the 5010 code
		CString strANSI = GetANSI5010_SBR09CodeFromInsuranceType(eCode);
		CString strNSF = GetNSFCodeFromInsuranceType(eCode);

		IRowSettingsPtr pRow = m_pInsTypeList->GetRow(-1);
		pRow->PutValue(itcID, (long)eCode);
		pRow->PutValue(itcName, _bstr_t(strName));
		pRow->PutValue(itcANSI, _bstr_t(strANSI));
		pRow->PutValue(itcNSF, _bstr_t(strNSF));

		if(bColorize) {
			pRow->PutForeColor(RGB(0,0,255));
		}

		m_pInsTypeList->AddRow(pRow);

	}NxCatchAll("Error in CAdvPayerIDDlg::AddNewRowToInsuranceTypeList");
}

// (j.jones 2008-09-18 10:01) - PLID 31138 - added OnApplyEligibility
void CAdvPayerIDDlg::OnApplyEligibility() 
{
	long nAuditTransactionID = -1;

	try {

		if(m_SelectedInsCoList->GetRowCount() == 0) {
			AfxMessageBox("Please select at least one insurance company.");
			return;
		}

		if(m_pEligibilityList->GetCurSel() == -1) {
			AfxMessageBox("Please select an Eligibility Payer ID.");
			return;
		}

		if(IDNO==MessageBox("This action will apply the selected Eligibility Payer ID to all selected Insurance companies.\n"
			"Are you sure you wish to do this?","Practice",MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		// (j.jones 2012-08-08 14:21) - PLID 51915 - this now saves as an ID field, not a string
		long nNewPayerID = VarLong(m_pEligibilityList->GetValue(m_pEligibilityList->CurSel, 0), -1);
		CString strNewPayerID = VarString(m_pEligibilityList->GetValue(m_pEligibilityList->CurSel, 1), "");

		CWaitCursor pWait;

		CString strSqlBatch;

		CString strInsIDs;

		int i=0;
		for(i=0;i<m_SelectedInsCoList->GetRowCount();i++) {
				
			long nInsID = VarLong(m_SelectedInsCoList->GetValue(i,ilcID));			
			
			// (j.jones 2012-08-08 14:13) - PLID 51915 - we now save the ID, not the code
			AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceCoT SET EligPayerID = %li WHERE PersonID = %li", nNewPayerID, nInsID);

			// (j.jones 2009-08-05 14:51) - PLID 31417 - audit this
			CString strOldPayerID = VarString(m_SelectedInsCoList->GetValue(i,ilcEligibilityPayerIDCode), "");
			if(strNewPayerID != strOldPayerID) {
				CString strInsuranceCoName = VarString(m_SelectedInsCoList->GetValue(i,ilcName));

				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(-1, strInsuranceCoName, nAuditTransactionID, aeiInsCoEligibility, nInsID, strOldPayerID, strNewPayerID, aepMedium, aetChanged);
			}

			// (j.jones 2009-08-05 13:53) - PLID 35109 - track a list of IDs of the insurance companies
			if(!strInsIDs.IsEmpty()) {
				strInsIDs += ",";
			}
			strInsIDs += AsString(nInsID);
		}

		// (j.jones 2009-08-05 13:58) - PLID 25109 - see if any of these insurance companies have a claim payer ID in use for any location
		if(!strInsIDs.IsEmpty()) {
			_RecordsetPtr rsLocCheck = CreateRecordset("SELECT EbillingInsCoIDs.EbillingID, "
				"InsuranceCoT.PersonID AS InsCoID, InsuranceCoT.Name AS InsCoName, LocationsT.Name AS LocName "
				"FROM InsuranceLocationPayerIDsT "
				"INNER JOIN InsuranceCoT ON InsuranceLocationPayerIDsT.InsuranceCoID = InsuranceCoT.PersonID "
				"INNER JOIN LocationsT ON InsuranceLocationPayerIDsT.LocationID = LocationsT.ID "
				"INNER JOIN EbillingInsCoIDs ON InsuranceLocationPayerIDsT.EligibilityPayerID = EbillingInsCoIDs.ID "
				"WHERE InsuranceLocationPayerIDsT.EligibilityPayerID Is Not Null "
				"AND InsuranceCoID IN (%s)", strInsIDs);
			if(!rsLocCheck->eof) {
				//if we have any results, prompt to clear
				int nRet = MessageBox("At least one of the insurance companies you are updating has an Eligibility payer ID in use for a specific location.\n\n"
					"Do you wish to clear this information so these insurance companies will use the new Eligibility payer ID for all locations?\n"
					"If not, the existing Eligibility payer ID for the insurance company and location will remain unchanged.", "Practice", MB_ICONQUESTION|MB_YESNOCANCEL);

				if(nRet == IDCANCEL) {
					//abort saving
					strSqlBatch = "";

					// (j.jones 2009-08-05 14:21) - PLID 31417 - rollback our auditing
					if(nAuditTransactionID != -1) {
						RollbackAuditTransaction(nAuditTransactionID);
					}
					return;
				}
				else if(nRet == IDYES) {
					//update the data - remove rows that will now be blank, update the rest
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM InsuranceLocationPayerIDsT WHERE ClaimPayerID Is Null AND UBPayerID Is Null AND InsuranceCoID IN (%s)", strInsIDs);
					AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceLocationPayerIDsT SET EligibilityPayerID = NULL WHERE InsuranceCoID IN (%s)", strInsIDs);

					// (j.jones 2009-08-05 15:50) - PLID 31417 - audit this
					while(!rsLocCheck->eof) {
						CString strOldPayerID = AdoFldString(rsLocCheck, "EbillingID", "");
						if(strOldPayerID != "") {
							long nInsCoID = AdoFldLong(rsLocCheck, "InsCoID");
							CString strInsuranceCoName = AdoFldString(rsLocCheck, "InsCoName", "");
							CString strLocationName = AdoFldString(rsLocCheck, "LocName", "");
							
							CString strOldValue, strNewValue;

							strOldValue.Format("%s: %s", strLocationName, strOldPayerID);
							strNewValue = "<Use Default>";

							if(nAuditTransactionID == -1) {
								nAuditTransactionID = BeginAuditTransaction();
							}

							AuditEvent(-1, strInsuranceCoName, nAuditTransactionID, aeiInsCoLocationEligibilityPayerID, nInsCoID, strOldValue, strNewValue, aepMedium, aetChanged);
						}

						rsLocCheck->MoveNext();
					}
				}
			}
			rsLocCheck->Close();
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}

		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
			nAuditTransactionID = -1;
		}

		// (j.jones 2009-08-05 15:11) - PLID 31417 - now update the "old" values in the list
		for(i=0;i<m_SelectedInsCoList->GetRowCount();i++) {
			
			m_SelectedInsCoList->PutValue(i, ilcEligibilityPayerIDCode, _bstr_t(strNewPayerID));
		}

		AfxMessageBox("Update complete.");

	}NxCatchAllCall("Error in CAdvPayerIDDlg::OnApplyEligibility.",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

// (j.jones 2009-08-05 12:20) - PLID 35109 - added abilities to apply per location
void CAdvPayerIDDlg::OnApplyPayerLoc()
{
	long nAuditTransactionID = -1;

	try {

		if(m_SelectedInsCoList->GetRowCount() == 0) {
			AfxMessageBox("Please select at least one insurance company.");
			return;
		}

		if(m_pEnvoyList->GetCurSel() == -1) {
			AfxMessageBox("Please select a Payer ID.");
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pLocRow = m_pLocationCombo->GetCurSel();
		if(pLocRow == NULL) {
			AfxMessageBox("Please select a location.");
			return;
		}

		if(IDNO==MessageBox("This action will apply the selected HCFA Payer ID to all selected Insurance companies for the selected location only.\n"
			"Are you sure you wish to do this?","Practice",MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		long nPayerID = VarLong(m_pEnvoyList->GetValue(m_pEnvoyList->CurSel, 0));
		CString strPayerCode = VarString(m_pEnvoyList->GetValue(m_pEnvoyList->CurSel, 1), "");
		long nLocationID = VarLong(pLocRow->GetValue(0));
		CString strLocationName = VarString(pLocRow->GetValue(1), "");

		CWaitCursor pWait;

		CString strSqlBatch;

		int i=0;
		for(i=0;i<m_SelectedInsCoList->GetRowCount();i++) {
				
			long nInsID = VarLong(m_SelectedInsCoList->GetValue(i,ilcID));
			CString strInsCoName = VarString(m_SelectedInsCoList->GetValue(i,ilcName));
			
			//do we already have a record?
			_RecordsetPtr rs = CreateParamRecordset("SELECT EbillingInsCoIDs.EbillingID "
				"FROM InsuranceLocationPayerIDsT "
				"LEFT JOIN EbillingInsCoIDs ON InsuranceLocationPayerIDsT.ClaimPayerID = EbillingInsCoIDs.ID "
				"WHERE InsuranceCoID = {INT} AND LocationID = {INT} ", nInsID, nLocationID);
			if(!rs->eof) {
				//we have a record, so update it
				AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceLocationPayerIDsT SET ClaimPayerID = %li "
					"WHERE InsuranceCoID = %li AND LocationID = %li ", nPayerID, nInsID, nLocationID);
			}
			else {
				//we don't have a record, and we need one, so create it
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO InsuranceLocationPayerIDsT (InsuranceCoID, LocationID, ClaimPayerID, EligibilityPayerID) "
					"VALUES (%li, %li, %li, NULL)", nInsID, nLocationID, nPayerID);
			}

			// (j.jones 2009-08-05 15:50) - PLID 31417 - audit this
			CString strOldPayerID = "";
			if(!rs->eof) {
				strOldPayerID = AdoFldString(rs, "EbillingID", "");
			}

			if(strOldPayerID != strPayerCode) {
				
				CString strOldValue;

				if(strOldPayerID.IsEmpty()) {
					strOldPayerID = "<Use Default>";
				}

				strOldValue.Format("%s: %s", strLocationName, strOldPayerID);

				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				AuditEvent(-1, strInsCoName, nAuditTransactionID, aeiInsCoLocationClaimPayerID, nInsID, strOldValue, strPayerCode, aepMedium, aetChanged);
			}

			rs->Close();
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}

		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
			nAuditTransactionID = -1;
		}

		AfxMessageBox("Update complete.");
	
	}NxCatchAllCall("Error in CAdvPayerIDDlg::OnApplyPayerLoc",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

// (j.jones 2009-08-05 12:20) - PLID 35109 - added abilities to apply per location
void CAdvPayerIDDlg::OnApplyEligibilityLoc()
{
	long nAuditTransactionID = -1;

	try {

		if(m_SelectedInsCoList->GetRowCount() == 0) {
			AfxMessageBox("Please select at least one insurance company.");
			return;
		}

		if(m_pEligibilityList->GetCurSel() == -1) {
			AfxMessageBox("Please select an Eligibility Payer ID.");
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pLocRow = m_pLocationCombo->GetCurSel();
		if(pLocRow == NULL) {
			AfxMessageBox("Please select a location.");
			return;
		}

		if(IDNO==MessageBox("This action will apply the selected Eligibility Payer ID to all selected Insurance companies for the selected location only.\n"
			"Are you sure you wish to do this?","Practice",MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		long nPayerID = VarLong(m_pEligibilityList->GetValue(m_pEligibilityList->CurSel, 0));
		CString strPayerCode = VarString(m_pEligibilityList->GetValue(m_pEligibilityList->CurSel, 1), "");
		long nLocationID = VarLong(pLocRow->GetValue(0));
		CString strLocationName = VarString(pLocRow->GetValue(1), "");

		CWaitCursor pWait;

		CString strSqlBatch;

		int i=0;
		for(i=0;i<m_SelectedInsCoList->GetRowCount();i++) {
				
			long nInsID = VarLong(m_SelectedInsCoList->GetValue(i,ilcID));
			CString strInsCoName = VarString(m_SelectedInsCoList->GetValue(i,ilcName));
			
			//do we already have a record?
			_RecordsetPtr rs = CreateParamRecordset("SELECT EbillingInsCoIDs.EbillingID "
				"FROM InsuranceLocationPayerIDsT "
				"LEFT JOIN EbillingInsCoIDs ON InsuranceLocationPayerIDsT.EligibilityPayerID = EbillingInsCoIDs.ID "
				"WHERE InsuranceCoID = {INT} AND LocationID = {INT} ", nInsID, nLocationID);
			if(!rs->eof) {
				//we have a record, so update it
				AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceLocationPayerIDsT SET EligibilityPayerID = %li "
					"WHERE InsuranceCoID = %li AND LocationID = %li ", nPayerID, nInsID, nLocationID);
			}
			else {
				//we don't have a record, and we need one, so create it
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO InsuranceLocationPayerIDsT (InsuranceCoID, LocationID, ClaimPayerID, EligibilityPayerID) "
					"VALUES (%li, %li, NULL, %li)", nInsID, nLocationID, nPayerID);
			}

			// (j.jones 2009-08-05 15:50) - PLID 31417 - audit this
			CString strOldPayerID = "";
			if(!rs->eof) {
				strOldPayerID = AdoFldString(rs, "EbillingID", "");
			}

			if(strOldPayerID != strPayerCode) {
				
				CString strOldValue;

				if(strOldPayerID.IsEmpty()) {
					strOldPayerID = "<Use Default>";
				}

				strOldValue.Format("%s: %s", strLocationName, strOldPayerID);

				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				AuditEvent(-1, strInsCoName, nAuditTransactionID, aeiInsCoLocationEligibilityPayerID, nInsID, strOldValue, strPayerCode, aepMedium, aetChanged);
			}

			rs->Close();
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}
		
		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
			nAuditTransactionID = -1;
		}

		AfxMessageBox("Update complete.");

	}NxCatchAllCall("Error in CAdvPayerIDDlg::OnApplyEligibilityLoc",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

// (j.jones 2009-12-16 16:40) - PLID 36621 - added UB payer ID
void CAdvPayerIDDlg::OnBnClickedApplyUbPayer()
{
	long nAuditTransactionID = -1;

	try {

		if(m_SelectedInsCoList->GetRowCount() == 0) {
			AfxMessageBox("Please select at least one insurance company.");
			return;
		}

		if(m_pUBPayerList->GetCurSel() == -1) {
			AfxMessageBox("Please select a UB Payer ID.");
			return;
		}

		if(IDNO==MessageBox("This action will apply the selected UB Payer ID to all selected Insurance companies.\n"
			"Are you sure you wish to do this?","Practice",MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		//this saves as an ID field, not a string
		long nNewPayerID = VarLong(m_pUBPayerList->GetValue(m_pUBPayerList->CurSel, 0), -1);
		CString strNewPayerCode = VarString(m_pUBPayerList->GetValue(m_pUBPayerList->CurSel, 1), "");

		CWaitCursor pWait;

		CString strSqlBatch;

		CString strInsIDs;

		int i=0;
		for(i=0;i<m_SelectedInsCoList->GetRowCount();i++) {
				
			long nInsID = VarLong(m_SelectedInsCoList->GetValue(i,ilcID));
			
			AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceCoT SET UBPayerID = %li WHERE PersonID = %li", nNewPayerID, nInsID);

			//audit this
			CString strOldPayerCode = VarString(m_SelectedInsCoList->GetValue(i,ilcUBPayerIDCode), "");
			if(strOldPayerCode != strNewPayerCode) {
				CString strInsuranceCoName = VarString(m_SelectedInsCoList->GetValue(i,ilcName));

				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(-1, strInsuranceCoName, nAuditTransactionID, aeiInsCoUBPayerID, nInsID, strOldPayerCode, strNewPayerCode, aepMedium, aetChanged);
			}

			//track a list of IDs of the insurance companies
			if(!strInsIDs.IsEmpty()) {
				strInsIDs += ",";
			}
			strInsIDs += AsString(nInsID);
		}

		//see if any of these insurance companies have a claim payer ID in use for any location
		if(!strInsIDs.IsEmpty()) {
			_RecordsetPtr rsLocCheck = CreateRecordset("SELECT EbillingInsCoIDs.EbillingID, "
				"InsuranceCoT.PersonID AS InsCoID, InsuranceCoT.Name AS InsCoName, LocationsT.Name AS LocName "
				"FROM InsuranceLocationPayerIDsT "
				"INNER JOIN InsuranceCoT ON InsuranceLocationPayerIDsT.InsuranceCoID = InsuranceCoT.PersonID "
				"INNER JOIN LocationsT ON InsuranceLocationPayerIDsT.LocationID = LocationsT.ID "
				"INNER JOIN EbillingInsCoIDs ON InsuranceLocationPayerIDsT.UBPayerID = EbillingInsCoIDs.ID "
				"WHERE InsuranceLocationPayerIDsT.UBPayerID Is Not Null "
				"AND InsuranceCoID IN (%s)", strInsIDs);
			if(!rsLocCheck->eof) {
				//if we have any results, prompt to clear
				int nRet = MessageBox("At least one of the insurance companies you are updating has a UB payer ID in use for a specific location.\n\n"
					"Do you wish to clear this information so these insurance companies will use the new UB payer ID for all locations?\n"
					"If not, the existing UB payer ID for the insurance company and location will remain unchanged.", "Practice", MB_ICONQUESTION|MB_YESNOCANCEL);
				
				if(nRet == IDCANCEL) {
					//abort saving
					strSqlBatch = "";

					// (j.jones 2009-08-05 14:21) - PLID 31417 - rollback our auditing
					if(nAuditTransactionID != -1) {
						RollbackAuditTransaction(nAuditTransactionID);
					}
					return;
				}
				else if(nRet == IDYES) {
					//update the data - remove rows that will now be blank, update the rest
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM InsuranceLocationPayerIDsT WHERE ClaimPayerID Is Null AND EligibilityPayerID Is Null AND InsuranceCoID IN (%s)", strInsIDs);
					AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceLocationPayerIDsT SET UBPayerID = NULL WHERE InsuranceCoID IN (%s)", strInsIDs);

					// (j.jones 2009-08-05 15:50) - PLID 31417 - audit this
					while(!rsLocCheck->eof) {
						CString strOldPayerID = AdoFldString(rsLocCheck, "EbillingID", "");
						if(strOldPayerID != "") {
							long nInsCoID = AdoFldLong(rsLocCheck, "InsCoID");
							CString strInsuranceCoName = AdoFldString(rsLocCheck, "InsCoName", "");
							CString strLocationName = AdoFldString(rsLocCheck, "LocName", "");
							
							CString strOldValue, strNewValue;

							strOldValue.Format("%s: %s", strLocationName, strOldPayerID);
							strNewValue = "<Use Default>";

							if(nAuditTransactionID == -1) {
								nAuditTransactionID = BeginAuditTransaction();
							}

							AuditEvent(-1, strInsuranceCoName, nAuditTransactionID, aeiInsCoLocationUBPayerID, nInsCoID, strOldValue, strNewValue, aepMedium, aetChanged);
						}

						rsLocCheck->MoveNext();
					}
				}
			}
			rsLocCheck->Close();
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}

		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
			nAuditTransactionID = -1;
		}

		// (j.jones 2009-08-05 15:11) - PLID 31417 - now update the "old" values in the list
		for(i=0;i<m_SelectedInsCoList->GetRowCount();i++) {
			
			m_SelectedInsCoList->PutValue(i, ilcUBPayerIDCode, _bstr_t(strNewPayerCode));
		}

		AfxMessageBox("Update complete.");

	}NxCatchAllCall("Error applying UB Payer ID.",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

// (j.jones 2009-12-16 16:40) - PLID 36621 - added UB payer ID
void CAdvPayerIDDlg::OnBnClickedApplyUbPayerLoc()
{
	long nAuditTransactionID = -1;

	try {

		if(m_SelectedInsCoList->GetRowCount() == 0) {
			AfxMessageBox("Please select at least one insurance company.");
			return;
		}

		if(m_pUBPayerList->GetCurSel() == -1) {
			AfxMessageBox("Please select a UB Payer ID.");
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pLocRow = m_pLocationCombo->GetCurSel();
		if(pLocRow == NULL) {
			AfxMessageBox("Please select a location.");
			return;
		}

		if(IDNO==MessageBox("This action will apply the selected UB Payer ID to all selected Insurance companies for the selected location only.\n"
			"Are you sure you wish to do this?","Practice",MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		long nPayerID = VarLong(m_pUBPayerList->GetValue(m_pUBPayerList->CurSel, 0));
		CString strPayerCode = VarString(m_pUBPayerList->GetValue(m_pUBPayerList->CurSel, 1), "");
		long nLocationID = VarLong(pLocRow->GetValue(0));
		CString strLocationName = VarString(pLocRow->GetValue(1), "");

		CWaitCursor pWait;

		CString strSqlBatch;

		int i=0;
		for(i=0;i<m_SelectedInsCoList->GetRowCount();i++) {
				
			long nInsID = VarLong(m_SelectedInsCoList->GetValue(i,ilcID));
			CString strInsCoName = VarString(m_SelectedInsCoList->GetValue(i,ilcName));
			
			//do we already have a record?
			_RecordsetPtr rs = CreateParamRecordset("SELECT EbillingInsCoIDs.EbillingID "
				"FROM InsuranceLocationPayerIDsT "
				"LEFT JOIN EbillingInsCoIDs ON InsuranceLocationPayerIDsT.UBPayerID = EbillingInsCoIDs.ID "
				"WHERE InsuranceCoID = {INT} AND LocationID = {INT} ", nInsID, nLocationID);
			if(!rs->eof) {
				//we have a record, so update it
				AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceLocationPayerIDsT SET UBPayerID = %li "
					"WHERE InsuranceCoID = %li AND LocationID = %li ", nPayerID, nInsID, nLocationID);
			}
			else {
				//we don't have a record, and we need one, so create it
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO InsuranceLocationPayerIDsT (InsuranceCoID, LocationID, UBPayerID) "
					"VALUES (%li, %li, %li)", nInsID, nLocationID, nPayerID);
			}

			//audit this
			CString strOldPayerID = "";
			if(!rs->eof) {
				strOldPayerID = AdoFldString(rs, "EbillingID", "");
			}

			if(strOldPayerID != strPayerCode) {
				
				CString strOldValue;

				if(strOldPayerID.IsEmpty()) {
					strOldPayerID = "<Use Default>";
				}

				strOldValue.Format("%s: %s", strLocationName, strOldPayerID);

				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				AuditEvent(-1, strInsCoName, nAuditTransactionID, aeiInsCoLocationUBPayerID, nInsID, strOldValue, strPayerCode, aepMedium, aetChanged);
			}

			rs->Close();
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}
		
		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
			nAuditTransactionID = -1;
		}

		AfxMessageBox("Update complete.");

	}NxCatchAllCall("Error in CAdvPayerIDDlg::OnBnClickedApplyUbPayerLoc",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}
