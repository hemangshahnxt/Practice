
#include "stdafx.h"
#include "CPTCodes.h"
#include "GlobalDataUtils.h"
#include "InternationalUtils.h"
#include "ClaimSetupDLG.h"
#include "NDCDefSetupDlg.h"
#include "AuditTrail.h"
#include "Auditing.h"
#include "ConfigureChargeLevelProviderDlg.h"
#include "AdministratorRc.h"



// (s.tullis 2014-05-20 17:48) - PLID 62018 - Remove the Claim Setup infomation from the Admin Billing dialog to a new dialog that launchs from the Additional Service Code Setup Menu
using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


using namespace NXDATALIST2Lib;


enum NOCColumns {
	noccID = 0,
	noccName,
};

void CClaimSetup::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NOC_LABEL, m_nxlNOC);
	DDX_Control(pDX, IDC_CPT_REQUIRE_REF_PHYS, m_checkRequireRefPhys);
	DDX_Control(pDX, IDC_CHECK_BATCH_WHEN_ZERO, m_checkBatchWhenZero);
	DDX_Control(pDX, IDC_CLAIM_SETUP_DEFAULT_ON_HOLD_CHECK, m_checkDefaultAsOnHold);
	DDX_Control(pDX, IDC_CLAIM_SETUP_IS_LAB_CHARGE_CHECK, m_checkIsLabCharge);
	DDX_Control(pDX, IDOK, m_btnOk);
}

CClaimSetup::CClaimSetup(CWnd* pParent)
: CNxDialog(CClaimSetup::IDD, pParent)
{
	// (r.gonet 07/08/2014) - PLID 62572 - Initialize member variables
	m_nServiceID = -1;
	m_bReqRefPhy = FALSE;
	m_bBatchIfZero = FALSE;
	m_bDefaultAsOnHold = FALSE;
	// (r.gonet 08/06/2014) - PLID 63096 - Initialize the IsLabCharge flag.
	m_bIsLabCharge = FALSE;
	eOldNocType = (long)noctDefault;
	NocWarnedOnce = FALSE;
}

BEGIN_MESSAGE_MAP(CClaimSetup, CNxDialog)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_BN_CLICKED(IDC_CHECK_BATCH_WHEN_ZERO, OnCheckBatchWhenZero)
	ON_BN_CLICKED(IDOK, OnCloseClick)
	ON_BN_CLICKED(IDC_NDC_DEFAULTS, OnClickNDCdefault)
	ON_BN_CLICKED(IDC_CHARGE_LEVEL_PROVIDERS, OnConfigureProviders)
END_MESSAGE_MAP()



// (s.tullis 2014-05-20 17:48) - PLID 62018 - Query and set values in the Dialog
BOOL CClaimSetup::OnInitDialog(){

	CNxDialog::OnInitDialog();

	try {
		_RecordsetPtr	tmpRS;
		FieldsPtr fields;
		NocWarnedOnce = FALSE;

		m_nxlNOC.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_nxlNOC.SetType(dtsHyperlink);
		m_nxlNOC.SetSingleLine(true);
		m_nxlNOC.SetText("NOC Code");
		m_btnOk.AutoSet(NXB_CLOSE);


		m_NOCCombo = BindNxDataList2Ctrl(IDC_NOC_COMBO, false);

		//add NOC options for <Default>, No, and Yes
		NXDATALIST2Lib::IRowSettingsPtr pNOCRow = m_NOCCombo->GetNewRow();
		pNOCRow->PutValue(noccID, (long)noctDefault);
		pNOCRow->PutValue(noccName, "<Default>");
		m_NOCCombo->AddRowAtEnd(pNOCRow, NULL);
		pNOCRow = m_NOCCombo->GetNewRow();
		pNOCRow->PutValue(noccID, (long)noctNo);
		pNOCRow->PutValue(noccName, "No");
		m_NOCCombo->AddRowAtEnd(pNOCRow, NULL);
		pNOCRow = m_NOCCombo->GetNewRow();
		pNOCRow->PutValue(noccID, (long)noctYes);
		pNOCRow->PutValue(noccName, "Yes");
		m_NOCCombo->AddRowAtEnd(pNOCRow, NULL);

		// (r.gonet 07/08/2014) - PLID 62572 - Added CPTCodeT.DefaultAsOnHold
		// (r.gonet 08/06/2014) - PLID 63096 - Added ServiceT.LabCharge.
		tmpRS = CreateParamRecordset(
			"SELECT ClaimNote, BatchIfZero, NOCType, RequireRefPhys, ClaimNote, "
			"	CPTCodeT.DefaultAsOnHold, ServiceT.LabCharge "
			"FROM ServiceT "
			"INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"WHERE ServiceT.ID = {INT} "
			, m_nServiceID);

		if (!tmpRS->eof){
			fields = tmpRS->Fields;

			// (r.gonet 07/08/2014) - PLID 62572 - Changed to BOOL variables instead of longs.
			m_bReqRefPhy = VarBool(fields->GetItem("RequireRefPhys")->Value, FALSE);
			m_bBatchIfZero = VarBool(fields->GetItem("BatchIfZero")->Value, FALSE);
			// (r.gonet 07/08/2014) - PLID 62572 - Get the DefaultAsOnHold value
			m_bDefaultAsOnHold = AdoFldBool(fields, "DefaultAsOnHold", FALSE);
			// (r.gonet 08/06/2014) - PLID 63096 - Load the IsLabCharge flag.
			m_bIsLabCharge = AdoFldBool(fields, "LabCharge", FALSE);
			m_strClaimNote = VarString(fields->GetItem("ClaimNote")->Value, "");

			_variant_t varNOCType = fields->Item["NOCType"]->Value;
			NOCTypes eType = noctDefault;
			if (varNOCType.vt == VT_BOOL) {
				BOOL bNOCType = VarBool(varNOCType);
				eType = bNOCType ? noctYes : noctNo;
			}

			eOldNocType = eType;
			m_NOCCombo->SetSelByColumn(noccID, (long)eType);
			// (r.gonet 07/08/2014) - PLID 62572 - Changed member variables to booleans
			m_checkRequireRefPhys.SetCheck(m_bReqRefPhy ? BST_CHECKED : BST_UNCHECKED);
			m_checkBatchWhenZero.SetCheck(m_bBatchIfZero ? BST_CHECKED : BST_UNCHECKED);
			// (r.gonet 07/08/2014) - PLID 62572 - Set the checkbox to use the value we pulled out of the database for Default As On Hold
			m_checkDefaultAsOnHold.SetCheck(m_bDefaultAsOnHold ? BST_CHECKED : BST_UNCHECKED);
			// (r.gonet 08/06/2014) - PLID 63096 - Set the Is Lab Charge checkbox in the UI.
			m_checkIsLabCharge.SetCheck(m_bIsLabCharge ? BST_CHECKED : BST_UNCHECKED);
			SetDlgItemText(IDC_CLAIM_NOTE, m_strClaimNote);

			tmpRS->Close();
		}	
	}
	NxCatchAll("Error in ClaimSetupDLG OnInitDialog")

	return TRUE;
}
// (s.tullis 2014-05-20 17:48) - PLID 62018 - Pop up noc description when noc code link is clicked
LRESULT CClaimSetup::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try{
		UINT nIdc = (UINT)wParam;
		switch (nIdc) {
		
		case IDC_NOC_LABEL:
			PopupNOCDescription();
			break;
		default:
			//What?  Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll("Error in OnLabelClick");
	return 0;
}
// (s.tullis 2014-05-20 17:48) - PLID 62018 - Noc Description pop up
void CClaimSetup::PopupNOCDescription()
{
	try {

		if (m_nServiceID == -1) {
			return;
		}

		CString strCodeName, strSubCodeName;
		GetDlgItemText(IDC_CPTCODE, strCodeName);
		GetDlgItemText(IDC_CPTSUBCODE, strSubCodeName);
		strSubCodeName.TrimLeft(); strSubCodeName.TrimRight();
		if (!strSubCodeName.IsEmpty()) {
			strCodeName += " - " + strSubCodeName;
		}

		//run a recordset to use the exact same SQL logic the ebilling export will use
		// (j.jones 2013-07-15 16:17) - PLID 57566 - NOCType is now in ServiceT
		BOOL bIsNOCCode = ReturnsRecordsParam("SELECT ServiceT.ID FROM CPTCodeT "
			"INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID "
			"WHERE CPTCodeT.ID = {INT} "
			"AND ("
			" Name LIKE '%Not Otherwise Classified%' "
			" OR Name LIKE '%Not Otherwise%' "
			" OR Name LIKE '%Unlisted%' "
			" OR Name LIKE '%Not listed%' "
			" OR Name LIKE '%Unspecified%' "
			" OR Name LIKE '%Unclassified%' "
			" OR Name LIKE '%Not otherwise specified%' "
			" OR Name LIKE '%Non-specified%' "
			" OR Name LIKE '%Not elsewhere specified%' "
			" OR Name LIKE '%Not elsewhere%' "
			" OR Name LIKE '%nos' "
			" OR Name LIKE '%nos %' "
			" OR Name LIKE '%nos;%' "
			" OR Name LIKE '%nos,%' "
			" OR Name LIKE '%noc' "
			" OR Name LIKE '%noc %' "
			" OR Name LIKE '%noc;%' "
			" OR Name LIKE '%noc,%' "
			")", m_nServiceID);

		// (j.jones 2012-04-11 17:06) - PLID 47313 - updated this comment to mention the default claim note feature
		CString strMessage;
		strMessage.Format("A \"Not Otherwise Classified\" (NOC) code is any service code with the following words or phrases in the description:\n\n"
			"- Not Otherwise Classified \n"
			"- Not Otherwise \n"
			"- Unlisted \n"
			"- Not Listed \n"
			"- Unspecified \n"
			"- Unclassified \n"
			"- Not Otherwise Specified \n"
			"- Non-specified \n"
			"- Not Elsewhere Specified \n"
			"- Not Elsewhere \n"
			"- Nos (Note: Includes \"nos\", \"nos;\", \"nos,\") \n"
			"- Noc (Note: Includes \"noc\", \"noc;\", \"noc,\") \n\n"
			"Practice will treat all codes with these descriptions as NOC codes, unless specifically noted in this setting.\n\n"
			"This service code, %s, defaults to %sbeing reported as an NOC code. "
			"If this default is not correct, change the NOC code setting to Yes or No to force NOC on or off for this service code.\n\n"
			"NOC codes also require a description to be sent in ANSI claims in Loop 2400 SV101-7. "
			"Charges for NOC codes will need a billing note entered with the \"Claim\" box checked in order to enter this description. "
			"Practice will automatically report this note in the correct field for NOC codes.\n\n"
			"A default claim note can be entered for this service code by clicking the \"...\" button next to the Description field, "
			"and selecting \"Edit Claim Note\". This note will automatically add to bills with the \"Claim\" box checked.",
			strCodeName, bIsNOCCode ? "" : "not ");

		MsgBox(strMessage);

	} NxCatchAll(__FUNCTION__);
}

void CClaimSetup::OnCheckBatchWhenZero()
{
	try {

		DontShowMeAgain(this, "Changing the setting to 'include on claims even with a zero charge amount' will only "
			"take effect on new charges for this service code.\n"
			"Existing bills containing this service code may need the charge's batch status to be "
			"manually changed in order to submit on claims.", "ServiceCodeBatchWhenZero");

		

	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 07/08/2014) - PLID 62572 - Split from saving code. Validates the changes before saving. Returns true if validation successful.
// Returns false if validation failed
bool CClaimSetup::Validate()
{
	CString strNewClaimNote;
	GetDlgItemText(IDC_CLAIM_NOTE, strNewClaimNote);
	strNewClaimNote = CheckClaimNote(strNewClaimNote);

	NOCTypes eNewNocType = noctDefault;
	IRowSettingsPtr pNocRow = m_NOCCombo->GetCurSel();
	if (pNocRow) {
		eNewNocType = (NOCTypes)VarLong(pNocRow->GetValue(noccID), (long)noctDefault);
	}

	// warn if claim note is not filled out and noc code is set to yes
	if (strNewClaimNote.IsEmpty()) {
		if (eNewNocType == noctYes && !NocWarnedOnce &&   IDYES == MessageBox("A 'Not Otherwise Classified' (NOC) code requires a billing note with the \"Claim\" box checked.\n\n"
			"A default claim note can be entered for this service code by clicking the \"...\" button next to the Description field, "
			"and selecting \"Edit Claim Note\". This note will automatically add to bills with the \"Claim\" box checked.\n\n"
			"Would you like to enter a default claim note now?", "Practice", MB_ICONQUESTION | MB_YESNO)) {

			GetDlgItem(IDC_CLAIM_NOTE)->SetFocus();

			NocWarnedOnce = TRUE;

			return false;
		}
	}

	return true;
}

// (s.tullis 2014-05-20 17:48) - PLID 62018 - Get Values, Save changes, and Audit
// (r.gonet 07/08/2014) - PLID 62572 - Refactored saving code. Added saving of Always default charge as 'On Hold'.
void CClaimSetup::OnCloseClick()
{
	long nAuditTransactionID = -1;
	try {
		// (r.gonet 07/08/2014) - PLID 62572 - Validate the changes
		if (!Validate()) {
			return;
		}

		nAuditTransactionID = BeginAuditTransaction();
		CParamSqlBatch sqlUpdateBatch;
		bool bUpdateNeeded = false;

		///////////////////////
		// Claim Note
		CString strNewClaimNote;
		GetDlgItemText(IDC_CLAIM_NOTE, strNewClaimNote);
		strNewClaimNote = CheckClaimNote(strNewClaimNote);
		if (strNewClaimNote != m_strClaimNote) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(-1, strCPTCodeDesc, nAuditTransactionID, aeiDefClaimNote, m_nServiceID, strCodeName + ": " + m_strClaimNote, strNewClaimNote, aepLow, aetChanged);
			bUpdateNeeded = true;
		}

		///////////////////////
		// NOC Code
		NOCTypes eNewNocType = noctDefault;
		_variant_t varNewNocType = g_cvarNull;
		IRowSettingsPtr pNocRow = m_NOCCombo->GetCurSel();
		if (pNocRow) {
			eNewNocType = (NOCTypes)VarLong(pNocRow->GetValue(noccID), (long)noctDefault);
			if (eNewNocType == noctYes) {
				varNewNocType = g_cvarTrue;
			}
			else if (eNewNocType == noctNo) {
				varNewNocType = g_cvarFalse;
			}
		}

		if (eNewNocType != eOldNocType) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			CString strOldValue, strNewValue;
			switch (eOldNocType) {
			case noctYes:
				strOldValue = "Yes";
				break;
			case noctNo:
				strOldValue = "No";
				break;
			case noctDefault:
			default:
				strOldValue = "<Default>";
				break;
			}
			switch (eNewNocType) {
			case noctYes:
				strNewValue = "Yes";
				break;
			case noctNo:
				strNewValue = "No";
				break;
			case noctDefault:
			default:
				strNewValue = "<Default>";
				break;
			}

			AuditEvent(-1, strCPTCodeDesc, nAuditTransactionID, aeiCPTNOCType, m_nServiceID, strCodeName + ": " + strOldValue, strNewValue, aepLow, aetChanged);
			bUpdateNeeded = true;
		}

		///////////////////////
		// Is Lab Charge
		// (r.gonet 08/06/2014) - PLID 63096 - Audit that the IsLabCharge flag changed if it did.
		BOOL bNewIsLabCharge = (m_checkIsLabCharge.GetCheck() == BST_CHECKED ? TRUE : FALSE);
		if (bNewIsLabCharge != m_bIsLabCharge) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			CString strOldValue = m_bIsLabCharge != FALSE ? "Yes" : "No";
			CString strNewValue = bNewIsLabCharge != FALSE ? "Yes" : "No";

			AuditEvent(-1, strCPTCodeDesc, nAuditTransactionID, aeiCPTIsLabCharge, m_nServiceID, strCodeName + ": " + strOldValue, strNewValue, aepLow, aetChanged);
			bUpdateNeeded = true;
		}

		// (r.gonet 08/06/2014) - PLID 63096 - Added ServiceT.LabCharge
		sqlUpdateBatch.Add(
			"UPDATE ServiceT SET "
			"	ClaimNote = {STRING}, "
			"	NOCType = {VT_BOOL}, "
			"	LabCharge = {BIT} "
			"WHERE ID = {INT}; "
			, strNewClaimNote, varNewNocType, bNewIsLabCharge
			, m_nServiceID);

		// Get the full CPT code name for auditing
		CString strFullCodeName = strCodeName;
		CString strTempSubCodeName = strSubCodeName;
		strTempSubCodeName.TrimLeft();
		strTempSubCodeName.TrimRight();
		if (!strTempSubCodeName.IsEmpty()) {
			strFullCodeName += " - " + strTempSubCodeName;
		}

		///////////////////////
		// Batch if Zero
		BOOL bNewBatchIfZero = (m_checkBatchWhenZero.GetCheck() == BST_CHECKED ? TRUE : FALSE);
		if (m_bBatchIfZero != bNewBatchIfZero) {
			bUpdateNeeded = true;
		}

		///////////////////////
		// Require Referring Physician
		BOOL bNewRequireRefPhys = (m_checkRequireRefPhys.GetCheck() == BST_CHECKED ? TRUE : FALSE);
		if (bNewRequireRefPhys != m_bReqRefPhy) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			CString strOldValue = m_bReqRefPhy != FALSE ? "Yes" : "No";
			CString strNewValue = bNewRequireRefPhys != FALSE ? "Yes" : "No";

			AuditEvent(-1, strCPTCodeDesc, nAuditTransactionID, aeiCPTReqRefPhys, m_nServiceID, strFullCodeName + ": " + strOldValue, strNewValue, aepLow, aetChanged);
			bUpdateNeeded = true;
		}

		// (r.gonet 07/08/2014) - PLID 62572 - Get the default as on hold value and audit if necessary
		///////////////////////
		// Default As On Hold
		BOOL bNewDefaultAsOnHold = (m_checkDefaultAsOnHold.GetCheck() == BST_CHECKED ? TRUE : FALSE);
		if (bNewDefaultAsOnHold != m_bDefaultAsOnHold) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			CString strOldValue = m_bDefaultAsOnHold != FALSE ? "Yes" : "No";
			CString strNewValue = bNewDefaultAsOnHold != FALSE ? "Yes" : "No";

			AuditEvent(-1, strCPTCodeDesc, nAuditTransactionID, aeiCPTDefaultAsOnHold, m_nServiceID, strFullCodeName + ": " + strOldValue, strNewValue, aepLow, aetChanged);
			bUpdateNeeded = true;
		}

		sqlUpdateBatch.Add(
			"UPDATE CPTCodeT SET "
			"	BatchIfZero = {BIT}, "
			"	RequireRefPhys = {BIT}, "
			"	DefaultAsOnHold = {BIT} "
			"WHERE ID = {INT}; "
			, bNewBatchIfZero, bNewRequireRefPhys, bNewDefaultAsOnHold
			, m_nServiceID);

		if (bUpdateNeeded) {
			sqlUpdateBatch.Execute(GetRemoteData());
			//let everyone else know fields have changed
			CClient::RefreshTable(NetUtils::CPTCodeT, m_nServiceID);
		}

		// Audit
		CommitAuditTransaction(nAuditTransactionID);
	} NxCatchAllCall(__FUNCTION__,
		// rollback the audit transaction if we have one, and had an exception
		if (nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);

	CNxDialog::OnOK();
}

// (r.gonet 05/22/2014) - PLID 61832 - Open the charge level claim providers config dialog
void CClaimSetup::OnConfigureProviders()
{
	try {
		if (m_nServiceID == -1) {
			// No service, no service.
			return;
		}

		CConfigureChargeLevelProviderDlg dlg(this, m_nServiceID, CConfigureChargeLevelProviderDlg::EServiceType::CPTCode);

		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}
// (s.tullis 2014-05-20 17:48) - PLID 62018 - Make sure claim not is not
CString CClaimSetup::CheckClaimNote(CString strclaimnote){

	try{

		strclaimnote.TrimRight();
		strclaimnote.TrimLeft();


		if (strclaimnote.GetLength() > 80){
			strclaimnote = strclaimnote.Left(80);
		}

	}NxCatchAll(__FUNCTION__)

	return strclaimnote;

}


BOOL CClaimSetup::PreTranslateMessage(MSG* pMsg){

	try{
		if (pMsg->message == WM_KEYDOWN)
		{
			if ((pMsg->wParam == VK_RETURN) )
				pMsg->wParam = VK_TAB;
		}



	}NxCatchAll(__FUNCTION__)
	return CDialog::PreTranslateMessage(pMsg);


}


// (s.tullis 2014-05-20 17:48) - PLID 62018 - launch NDC defaults dialog
void CClaimSetup::OnClickNDCdefault()
{
	try{
		CNDCDefSetupDlg dlg(m_nServiceID, false, this);
		if (dlg.DoModal() == IDOK){
			//CCPTCodes::m_CPTChecker.Refresh(m_nServiceID);
		}

	}NxCatchAll(__FUNCTION__)


}