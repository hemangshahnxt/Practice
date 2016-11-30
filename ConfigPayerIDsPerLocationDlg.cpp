// ConfigPayerIDsPerLocationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigPayerIDsPerLocationDlg.h"
#include "EditInsPayersDlg.h"
#include "AuditTrail.h"

// CConfigPayerIDsPerLocationDlg dialog

// (j.jones 2009-08-04 13:15) - PLID 34467 - created

IMPLEMENT_DYNAMIC(CConfigPayerIDsPerLocationDlg, CNxDialog)

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



enum PayerComboColumns {

	pccID = 0,
	pccCode,
	pccName,
};

enum LocationComboColumns {

	lccID = 0,
	lccName,
};

CConfigPayerIDsPerLocationDlg::CConfigPayerIDsPerLocationDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConfigPayerIDsPerLocationDlg::IDD, pParent)
{
	m_nCurLocationID = -1;
	m_nInsuranceCoID = -1;
	m_nOldClaimPayerID = -1;
	m_nOldEligibilityPayerID = -1;
	m_nOldUBPayerID = -1;
	m_nColor = RGB(153, 204, 255);	
}

void CConfigPayerIDsPerLocationDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_CLOSE_PAYER_ID_LOC, m_btnClose);
	DDX_Control(pDX, IDC_EDIT_PAYER_LIST, m_btnEditPayerList);
	DDX_Control(pDX, IDC_LABEL_INSCO, m_nxstaticInsCoLabel);
	DDX_Control(pDX, IDC_PAYERID_LOC_CLR, m_bkg);
}

BEGIN_MESSAGE_MAP(CConfigPayerIDsPerLocationDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_CLOSE_PAYER_ID_LOC, OnBtnClose)
	ON_BN_CLICKED(IDC_EDIT_PAYER_LIST, OnEditPayerList)
END_MESSAGE_MAP()

// CConfigPayerIDsPerLocationDlg message handlers

BOOL CConfigPayerIDsPerLocationDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);

		//update our background color
		m_bkg.SetColor(m_nColor);

		m_nxstaticInsCoLabel.SetWindowText(m_strInsuranceCoName);

		m_LocationCombo = BindNxDataList2Ctrl(IDC_LOCATIONS_PAYER_ID_COMBO, true);
		m_ClaimPayerIDCombo = BindNxDataList2Ctrl(IDC_CLAIM_PAYER_ID_COMBO, true);
		m_EligibilityPayerIDCombo = BindNxDataList2Ctrl(IDC_ELIGIBILITY_PAYER_ID_COMBO, true);
		// (j.jones 2009-12-16 16:03) - PLID 36237 - added UB Payer IDs
		m_UBPayerIDCombo = BindNxDataList2Ctrl(IDC_UB_PAYER_ID_COMBO, true);

		{
			IRowSettingsPtr pRow = m_ClaimPayerIDCombo->GetNewRow();
			pRow->PutValue(pccID, (long)-1);
			pRow->PutValue(pccCode, " <Use Default>");
			pRow->PutValue(pccName, " <Use Default>");
			m_ClaimPayerIDCombo->AddRowSorted(pRow, NULL);
		}

		{
			IRowSettingsPtr pRow = m_EligibilityPayerIDCombo->GetNewRow();
			pRow->PutValue(pccID, (long)-1);
			pRow->PutValue(pccCode, " <Use Default>");
			pRow->PutValue(pccName, " <Use Default>");
			m_EligibilityPayerIDCombo->AddRowSorted(pRow, NULL);
		}

		// (j.jones 2009-12-16 16:03) - PLID 36237 - added UB Payer IDs
		{
			IRowSettingsPtr pRow = m_UBPayerIDCombo->GetNewRow();
			pRow->PutValue(pccID, (long)-1);
			pRow->PutValue(pccCode, " <Use Default>");
			pRow->PutValue(pccName, " <Use Default>");
			m_UBPayerIDCombo->AddRowSorted(pRow, NULL);
		}

		//load the current location
		m_nCurLocationID = GetCurrentLocationID();
		m_LocationCombo->SetSelByColumn(0, m_nCurLocationID);
		Load();

	}NxCatchAll("Error in CConfigPayerIDsPerLocationDlg::OnInitDialog");

	return TRUE;
}

void CConfigPayerIDsPerLocationDlg::OnBtnClose()
{
	try {

		if(!Save()) {
			return;
		}

		CNxDialog::OnOK();

	}NxCatchAll("Error in CConfigPayerIDsPerLocationDlg::OnBtnClose");
}

void CConfigPayerIDsPerLocationDlg::OnEditPayerList()
{
	try {

		//first save our selections
		if(!Save()) {
			return;
		}
		
		CEditInsPayersDlg dlg(this);
		dlg.DoModal();

		//requery both lists
		m_ClaimPayerIDCombo->Requery();
		m_EligibilityPayerIDCombo->Requery();
		// (j.jones 2009-12-16 16:03) - PLID 36237 - added UB Payer IDs
		m_UBPayerIDCombo->Requery();

		{
			IRowSettingsPtr pRow = m_ClaimPayerIDCombo->GetNewRow();
			pRow->PutValue(pccID, (long)-1);
			pRow->PutValue(pccCode, " <Use Default>");
			pRow->PutValue(pccName, " <Use Default>");
			m_ClaimPayerIDCombo->AddRowSorted(pRow, NULL);
		}

		{
			IRowSettingsPtr pRow = m_EligibilityPayerIDCombo->GetNewRow();
			pRow->PutValue(pccID, (long)-1);
			pRow->PutValue(pccCode, " <Use Default>");
			pRow->PutValue(pccName, " <Use Default>");
			m_EligibilityPayerIDCombo->AddRowSorted(pRow, NULL);
		}

		// (j.jones 2009-12-16 16:03) - PLID 36237 - added UB Payer IDs
		{
			IRowSettingsPtr pRow = m_UBPayerIDCombo->GetNewRow();
			pRow->PutValue(pccID, (long)-1);
			pRow->PutValue(pccCode, " <Use Default>");
			pRow->PutValue(pccName, " <Use Default>");
			m_UBPayerIDCombo->AddRowSorted(pRow, NULL);
		}

		//reload the location information
		Load();

	}NxCatchAll("Error in CConfigPayerIDsPerLocationDlg::OnEditPayerList");
}

void CConfigPayerIDsPerLocationDlg::Load()
{
	try {

		m_nOldClaimPayerID = -1;
		m_nOldEligibilityPayerID = -1;
		m_strOldClaimPayerCode = "";
		m_strOldEligibilityPayerCode = "";		
		m_nOldUBPayerID = -1;
		m_strOldUBPayerCode = "";

		// (j.jones 2009-12-16 16:03) - PLID 36237 - added UB Payer IDs
		_RecordsetPtr rs = CreateParamRecordset("SELECT ClaimPayerID, EligibilityPayerID, UBPayerID, "
			"ClaimPayerIDsT.EbillingID AS ClaimPayerCode, "
			"EligibilityPayerIDsT.EbillingID AS EligibilityPayerCode, "
			"UBPayerIDsT.EbillingID AS UBPayerCode "
			"FROM InsuranceLocationPayerIDsT "
			"LEFT JOIN EbillingInsCoIDs AS ClaimPayerIDsT ON InsuranceLocationPayerIDsT.ClaimPayerID = ClaimPayerIDsT.ID "
			"LEFT JOIN EbillingInsCoIDs AS EligibilityPayerIDsT ON InsuranceLocationPayerIDsT.EligibilityPayerID = EligibilityPayerIDsT.ID "
			"LEFT JOIN EbillingInsCoIDs AS UBPayerIDsT ON InsuranceLocationPayerIDsT.UBPayerID = UBPayerIDsT.ID "
			"WHERE InsuranceCoID = {INT} AND LocationID = {INT}", m_nInsuranceCoID, m_nCurLocationID);

		if(!rs->eof) {
			m_nOldClaimPayerID = AdoFldLong(rs, "ClaimPayerID", -1);
			m_nOldEligibilityPayerID = AdoFldLong(rs, "EligibilityPayerID", -1);
			m_strOldClaimPayerCode = AdoFldString(rs, "ClaimPayerCode", "");
			m_strOldEligibilityPayerCode = AdoFldString(rs, "EligibilityPayerCode", "");
			m_nOldUBPayerID = AdoFldLong(rs, "UBPayerID", -1);
			m_strOldUBPayerCode = AdoFldString(rs, "UBPayerCode", "");
		}
		rs->Close();

		m_ClaimPayerIDCombo->SetSelByColumn(pccID, m_nOldClaimPayerID);
		m_EligibilityPayerIDCombo->SetSelByColumn(pccID, m_nOldEligibilityPayerID);
		m_UBPayerIDCombo->SetSelByColumn(pccID, m_nOldUBPayerID);

	}NxCatchAll("Error in CConfigPayerIDsPerLocationDlg::Load");
}

BOOL CConfigPayerIDsPerLocationDlg::Save()
{
	try {

		IRowSettingsPtr pLocRow = m_LocationCombo->GetCurSel();
		if(pLocRow == NULL) {
			AfxMessageBox("The payer IDs cannot be saved without a location selected.");
			return FALSE;
		}

		CString strLocationName = VarString(pLocRow->GetValue(lccName));

		_variant_t varClaimID = g_cvarNull;
		_variant_t varEligID = g_cvarNull;
		_variant_t varUBID = g_cvarNull;
		CString strClaimPayerCode = "", strEligibilityPayerCode = "", strUBPayerCode = "";

		IRowSettingsPtr pClaimRow = m_ClaimPayerIDCombo->GetCurSel();
		if(pClaimRow) {
			varClaimID = pClaimRow->GetValue(pccID);

			if(VarLong(varClaimID, -1) == -1) {
				//reset -1 to NULL
				varClaimID = g_cvarNull;
			}
			else {
				strClaimPayerCode = VarString(pClaimRow->GetValue(pccCode), "");
			}
		}
		
		IRowSettingsPtr pEligibilityRow = m_EligibilityPayerIDCombo->GetCurSel();
		if(pEligibilityRow) {
			varEligID = pEligibilityRow->GetValue(pccID);

			if(VarLong(varEligID, -1) == -1) {
				//reset -1 to NULL
				varEligID = g_cvarNull;
			}
			else {
				strEligibilityPayerCode = VarString(pEligibilityRow->GetValue(pccCode), "");
			}
		}

		// (j.jones 2009-12-16 16:03) - PLID 36237 - added UB Payer IDs
		IRowSettingsPtr pUBRow = m_UBPayerIDCombo->GetCurSel();
		if(pUBRow) {
			varUBID = pUBRow->GetValue(pccID);

			if(VarLong(varUBID, -1) == -1) {
				//reset -1 to NULL
				varUBID = g_cvarNull;
			}
			else {
				strUBPayerCode = VarString(pUBRow->GetValue(pccCode), "");
			}
		}

		//do we already have a record?
		_RecordsetPtr rs = CreateParamRecordset("SELECT ClaimPayerID, EligibilityPayerID, UBPayerID "
			"FROM InsuranceLocationPayerIDsT "
			"WHERE InsuranceCoID = {INT} AND LocationID = {INT} ", m_nInsuranceCoID, m_nCurLocationID);
		if(!rs->eof) {
			//we have a record, so update it if we have data, delete if not
			if(varClaimID == g_cvarNull && varEligID == g_cvarNull && varUBID == g_cvarNull) {
				ExecuteParamSql("DELETE FROM InsuranceLocationPayerIDsT WHERE InsuranceCoID = {INT} AND LocationID = {INT} ", m_nInsuranceCoID, m_nCurLocationID);
			}
			else {
				// (j.jones 2009-12-16 16:03) - PLID 36237 - added UB Payer IDs
				ExecuteParamSql("UPDATE InsuranceLocationPayerIDsT SET ClaimPayerID = {VT_I4}, EligibilityPayerID = {VT_I4}, UBPayerID = {VT_I4} "
					"WHERE InsuranceCoID = {INT} AND LocationID = {INT} ", varClaimID, varEligID, varUBID, m_nInsuranceCoID, m_nCurLocationID);
			}
		}
		else if(varClaimID != g_cvarNull || varEligID != g_cvarNull || varUBID != g_cvarNull) {
			//we don't have a record, and we need one, so create it
			// (j.jones 2009-12-16 16:03) - PLID 36237 - added UB Payer IDs
			ExecuteParamSql("INSERT INTO InsuranceLocationPayerIDsT (InsuranceCoID, LocationID, ClaimPayerID, EligibilityPayerID, UBPayerID) "
				"VALUES ({INT}, {INT}, {VT_I4}, {VT_I4}, {VT_I4})", m_nInsuranceCoID, m_nCurLocationID, varClaimID, varEligID, varUBID);
		}
		rs->Close();

		//audit accordingly
		long nAuditID = -1;

		if(VarLong(varClaimID, -1) != m_nOldClaimPayerID) {

			CString strOldValue, strNewValue;

			strOldValue.Format("%s: %s", strLocationName, m_strOldClaimPayerCode);
			if(VarLong(varClaimID, -1) == -1) {
				strNewValue = "<Use Default>";
			}
			else {
				strNewValue = strClaimPayerCode;
			}

			if(nAuditID == -1) {
				nAuditID = BeginNewAuditEvent();
			}
			AuditEvent(-1, m_strInsuranceCoName, nAuditID, aeiInsCoLocationClaimPayerID, m_nInsuranceCoID, strOldValue, strNewValue, aepMedium, aetChanged);
		}

		m_nOldClaimPayerID = VarLong(varClaimID, -1);

		if(VarLong(varEligID, -1) != m_nOldEligibilityPayerID) {

			CString strOldValue, strNewValue;

			strOldValue.Format("%s: %s", strLocationName, m_strOldEligibilityPayerCode);
			if(VarLong(varEligID, -1) == -1) {
				strNewValue = "<Use Default>";
			}
			else {
				strNewValue = strEligibilityPayerCode;
			}

			if(nAuditID == -1) {
				nAuditID = BeginNewAuditEvent();
			}
			AuditEvent(-1, m_strInsuranceCoName, nAuditID, aeiInsCoLocationEligibilityPayerID, m_nInsuranceCoID, strOldValue, strNewValue, aepMedium, aetChanged);
		}

		m_nOldEligibilityPayerID = VarLong(varEligID, -1);

		// (j.jones 2009-12-16 16:03) - PLID 36237 - added UB Payer IDs
		if(VarLong(varUBID, -1) != m_nOldUBPayerID) {

			CString strOldValue, strNewValue;

			strOldValue.Format("%s: %s", strLocationName, m_strOldUBPayerCode);
			if(VarLong(varUBID, -1) == -1) {
				strNewValue = "<Use Default>";
			}
			else {
				strNewValue = strUBPayerCode;
			}

			if(nAuditID == -1) {
				nAuditID = BeginNewAuditEvent();
			}
			AuditEvent(-1, m_strInsuranceCoName, nAuditID, aeiInsCoLocationUBPayerID, m_nInsuranceCoID, strOldValue, strNewValue, aepMedium, aetChanged);
		}

		m_nOldUBPayerID = VarLong(varUBID, -1);

		return TRUE;

	}NxCatchAll("Error in CConfigPayerIDsPerLocationDlg::Save");

	return FALSE;
}


BEGIN_EVENTSINK_MAP(CConfigPayerIDsPerLocationDlg, CNxDialog)
ON_EVENT(CConfigPayerIDsPerLocationDlg, IDC_LOCATIONS_PAYER_ID_COMBO, 16, OnSelChosenLocationCombo, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CConfigPayerIDsPerLocationDlg::OnSelChosenLocationCombo(LPDISPATCH lpRow)
{
	try {

		long nNewLocationID = -1;
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			nNewLocationID = GetCurrentLocationID();
			m_LocationCombo->SetSelByColumn(0, nNewLocationID);
		}
		else {
			nNewLocationID = VarLong(pRow->GetValue(0));
		}

		if(m_nCurLocationID != nNewLocationID) {

			//save the previous location
			if(!Save()) {
				//if it failed, revert to the old selection
				m_LocationCombo->SetSelByColumn(0, m_nCurLocationID);
			}
		}

		m_nCurLocationID = nNewLocationID;

		Load();

	}NxCatchAll("Error in CConfigPayerIDsPerLocationDlg::OnSelChosenLocationCombo");
}
