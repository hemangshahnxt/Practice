// (r.gonet 05/19/2014) - PLID 40426 - Dialog that lets you move labs orders and results to a different patient or a different lab on the same patient.
// MovePatientLabsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "MultiSelectDlg.h"
#include "MovePatientLabsDlg.h"
#include "PatientsRc.h"
#include "NxPracticeSharedLib\HistoryUtils.h"
#include "RenameFileDlg.h"
#include "AuditTrail.h"
#include <NxHL7Lib\HL7CommonTypes.h>

using namespace ADODB;

// CMovePatientLabsDlg dialog

IMPLEMENT_DYNAMIC(CMovePatientLabsDlg, CNxDialog)

// (r.gonet 06/02/2014) - PLID 40426 - Added.
/// <summary>
/// Constructs a new CMovePatientLabsDlg
/// </summary>
/// <param name="pParent">The parent window.</param>
/// <param name="nPatientID">The ID of the patient to move lab results from.</param>
/// <param name="nLabID">The ID of the lab order to move lab results from.</param>
/// <param name="nLabResultID">The ID of the lab result to select by default to move. May be -1.</param>
CMovePatientLabsDlg::CMovePatientLabsDlg(CWnd* pParent, long nPatientID, long nLabID, long nLabResultID)
	: CNxDialog(CMovePatientLabsDlg::IDD, pParent)
{
	m_nFromPatientID = nPatientID;
	m_nFromLabID = nLabID;
	if (nLabResultID != -1) {
		m_arySelectedLabResultIDs.Add(nLabResultID);
	}
}

// (r.gonet 06/02/2014) - PLID 40426 - Added.
/// <summary>
/// Standard destructor.
/// </summary>
CMovePatientLabsDlg::~CMovePatientLabsDlg()
{
}

void CMovePatientLabsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MPL_HEADER_STATIC, m_nxstaticHeader);
	DDX_Control(pDX, IDC_MPL_RESULTS_STATIC, m_nxstaticResults);
	DDX_Control(pDX, IDC_MPL_MULTI_RESULTS_STATIC, m_nxlMultiResults);
	DDX_Control(pDX, IDC_MPL_NXCOLOR, m_nxcolorBackground);
	DDX_Control(pDX, IDC_MPL_TO_PATIENT_STATIC, m_nxstaticToPatient);
	DDX_Control(pDX, IDC_MPL_TO_ORDER_STATIC, m_nxstaticToOrder);
	DDX_Control(pDX, IDC_MPL_MOVE_BUTTON, m_btnMove);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}

BEGIN_MESSAGE_MAP(CMovePatientLabsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_MPL_MOVE_BUTTON, &CMovePatientLabsDlg::OnBnClickedMplMoveButton)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, &CMovePatientLabsDlg::OnLabelClick)
END_MESSAGE_MAP()


// CMovePatientLabsDlg message handlers

// (r.gonet 06/02/2014) - PLID 40426 - Added.
/// <summary>
/// Initializes the CMovePatientLabsDlg.
/// </summary>
/// <returns>Specifies whether the application has set the input focus to one of the controls in the dialog box. If OnInitDialog returns 
/// nonzero, Windows sets the input focus to the default location, the first control in the dialog box. The application can return 0 only 
/// if it has explicitly set the input focus to one of the controls in the dialog box.</returns>
BOOL CMovePatientLabsDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		m_nxcolorBackground.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_btnMove.AutoSet(NXB_MODIFY);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_nxlMultiResults.SetType(dtsDisabledHyperlink);
		m_nxlMultiResults.ShowWindow(SW_HIDE);

		// Set up the lab results combo and have it only pull results for a certain lab.
		m_pResultCombo = BindNxDataList2Ctrl(IDC_MPL_RESULT_COMBO, false);
		m_pResultCombo->WhereClause = _bstr_t(FormatString("LabResultsQ.LabID = %li", m_nFromLabID));
		m_pResultCombo->Requery();

		m_pToPatientCombo = BindNxDataList2Ctrl(IDC_MPL_TO_PATIENT_COMBO, true);
		AddSpecialRowsToPatientCombo();
		m_pToPatientCombo->SetSelByColumn((short)EToPatientComboColumns::PersonID, _variant_t(m_nFromPatientID, VT_I4));

		m_pToOrderCombo = BindNxDataList2Ctrl(IDC_MPL_TO_ORDER_COMBO, false);
		ReloadToOrderCombo(m_nFromPatientID);
		
	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CMovePatientLabsDlg, CNxDialog)
	ON_EVENT(CMovePatientLabsDlg, IDC_MPL_RESULT_COMBO, 18, CMovePatientLabsDlg::RequeryFinishedMplResultCombo, VTS_I2)
	ON_EVENT(CMovePatientLabsDlg, IDC_MPL_RESULT_COMBO, 1, CNxDialog::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CMovePatientLabsDlg, IDC_MPL_TO_PATIENT_COMBO, 1, CNxDialog::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CMovePatientLabsDlg, IDC_MPL_TO_ORDER_COMBO, 1, CNxDialog::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CMovePatientLabsDlg, IDC_MPL_RESULT_COMBO, 16, CMovePatientLabsDlg::SelChosenMplResultCombo, VTS_DISPATCH)
	ON_EVENT(CMovePatientLabsDlg, IDC_MPL_TO_PATIENT_COMBO, 16, CMovePatientLabsDlg::SelChosenMplToPatientCombo, VTS_DISPATCH)
END_EVENTSINK_MAP()

// (r.gonet 06/02/2014) - PLID 40426 - Added.
/// <summary>
/// Adds the special rows to the order combo. Selects the default row.
/// </summary>
/// <param name="nFlags">The reason why the requerying finished.</param>
void CMovePatientLabsDlg::RequeryFinishedMplResultCombo(short nFlags)
{
	NXDATALIST2Lib::IRowSettingsPtr pMultiRow = m_pResultCombo->GetNewRow();
	pMultiRow->PutValue((short)EResultComboColumns::ID, _variant_t(-2L, VT_I4));
	pMultiRow->PutValue((short)EResultComboColumns::Name, _bstr_t(" <Multiple Lab Results>"));
	m_pResultCombo->AddRowBefore(pMultiRow, m_pResultCombo->GetFirstRow());

	NXDATALIST2Lib::IRowSettingsPtr pNoResultRow = m_pResultCombo->GetNewRow();
	pNoResultRow->PutValue((short)EResultComboColumns::ID, _variant_t(-1L, VT_I4));
	pNoResultRow->PutValue((short)EResultComboColumns::Name, _bstr_t(" <Select a Lab Result>"));
	m_pResultCombo->AddRowBefore(pNoResultRow, m_pResultCombo->GetFirstRow());

	if (m_arySelectedLabResultIDs.GetSize() > 0) {
		// We have an initial lab result, so select that.
		m_pResultCombo->FindByColumn((short)EResultComboColumns::ID, _variant_t(m_arySelectedLabResultIDs.GetAt(0), VT_I4), m_pResultCombo->GetFirstRow(), VARIANT_TRUE);
	} else {
		// Select the <Select a Lab Result> row
		m_pResultCombo->CurSel = pNoResultRow;
	}
}

// (r.gonet 06/02/2014) - PLID 40426 - Added.
/// <summary>
/// Adds the special rows to the patient combo
/// </summary>
void CMovePatientLabsDlg::AddSpecialRowsToPatientCombo()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pNoPatientRow = m_pToPatientCombo->GetNewRow();
		pNoPatientRow->PutValue((short)EToPatientComboColumns::PersonID, _variant_t(-1L, VT_I4));
		pNoPatientRow->PutValue((short)EToPatientComboColumns::Name, _bstr_t(" <Select a Patient>"));
		m_pToPatientCombo->AddRowBefore(pNoPatientRow, m_pToPatientCombo->GetFirstRow());
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 06/02/2014) - PLID 40426 - Added.
/// <summary>
/// Reloads the To Lab Order combo. Loads all the labs for a given patient.
/// </summary>
/// <param name="nToPatientID">The ID of the patient to load LabsT records for.</param>
void CMovePatientLabsDlg::ReloadToOrderCombo(long nToPatientID)
{
	// Remove the old rows.
	m_pToOrderCombo->Clear();
	if (nToPatientID != -1) {
		// We have a patient, so select their labs. This query mimics the one that loads the labs datalist in the Patients->Labs tab.
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT LabsT.ID, LabsT.FormNumberTextID, LabsT.Specimen, LabsT.LOINC_Code, LabsT.LOINC_Description, "
			"	LabsT.Type, LabsT.AnatomySide, LabAnatomyT.Description AS AnatomicLocation, AnatomyQualifiersT.Name AS LocationQualifier, "
			"	LabsT.ToBeOrdered, "
			"	LabsT.InputDate "
			"FROM ("
			"	SELECT LabsT.ID, "
			"	LabsT.FormNumberTextID, "
			"	LabsT.Specimen, "
			"	LabsT.InputDate, "
			"	LabsT.Type, "
			"	LabsT.AnatomySide, "
			"	LabsT.LOINC_Code, "
			"	LabsT.LOINC_Description, "
			"	LabsT.AnatomyID, "
			"	LabsT.AnatomyQualifierID, "
			"	LabsT.ToBeOrdered "
			"	FROM LabsT "
			// Don't show deleted labs and don't let them move results into the same lab.
			"	WHERE LabsT.Deleted = 0 AND LabsT.PatientID = {INT} AND LabsT.ID <> {INT} "
			") LabsT \r\n"
			// sorts by specimen alphabetically
			"LEFT JOIN ( "
			"	SELECT LabsT.FormNumberTextID, MAX(LabsT.InputDate) AS InputDate FROM LabsT GROUP BY LabsT.FormNumberTextID "
			") LabsInputQ ON LabsT.FormNumberTextID = LabsInputQ.FormNumberTextID "
			"LEFT JOIN LabAnatomyT ON LabAnatomyT.ID = LabsT.AnatomyID "
			"LEFT JOIN AnatomyQualifiersT ON AnatomyQualifiersT.ID = LabsT.AnatomyQualifierID "
			"ORDER BY LabsInputQ.InputDate DESC, LabsT.FormNumberTextID, LabsT.Specimen; "
			, nToPatientID, m_nFromLabID
			);
		while (!prs->eof) {
			long nLabID = AdoFldLong(prs->Fields, "ID");
			CString strFormNumberTextID = AdoFldString(prs->Fields, "FormNumberTextID", "");
			CString strSpecimen = AdoFldString(prs->Fields, "Specimen", "");
			COleDateTime dtInputDate = AdoFldDateTime(prs->Fields, "InputDate", g_cdtNull);

			// The description of the lab is a bit complex in its construction.
			CString strDescription = GetLabDescription(prs->Fields);

			// Add the lab order to the To Orders combo.
			NXDATALIST2Lib::IRowSettingsPtr pNewOrderRow = m_pToOrderCombo->GetNewRow();
			pNewOrderRow->PutValue((short)EToOrderComboColumns::ID, _variant_t(nLabID));
			pNewOrderRow->PutValue((short)EToOrderComboColumns::FormNumberTextID, _bstr_t(strFormNumberTextID));
			pNewOrderRow->PutValue((short)EToOrderComboColumns::Specimen, _bstr_t(strSpecimen));
			pNewOrderRow->PutValue((short)EToOrderComboColumns::Description, _bstr_t(strDescription));
			pNewOrderRow->PutValue((short)EToOrderComboColumns::InputDate, dtInputDate != g_cdtNull ? _variant_t(dtInputDate, VT_DATE) : g_cvarNull);
			m_pToOrderCombo->AddRowAtEnd(pNewOrderRow, NULL);
			prs->MoveNext();
		}
		prs->Close();
	}

	// Add a special default row and select it.
	NXDATALIST2Lib::IRowSettingsPtr pNoOrderRow = m_pToOrderCombo->GetNewRow();
	pNoOrderRow->PutValue((short)EToOrderComboColumns::ID, _variant_t(-1L, VT_I4));
	pNoOrderRow->PutValue((short)EToOrderComboColumns::Description, _bstr_t(" <Select a Lab Order>"));
	m_pToOrderCombo->AddRowBefore(pNoOrderRow, m_pToOrderCombo->GetFirstRow());
	m_pToOrderCombo->CurSel = pNoOrderRow;
}

// (r.gonet 06/02/2014) - PLID 40426 - Added.
/// <summary>
/// Gets the lab description. Tightly coupled to the query in ReloadToOrderCombo.
/// </summary>
/// <param name="pFields">Recordset fields containing information about the lab order.</param>
/// <returns>A description of the lab.</returns>
CString CMovePatientLabsDlg::GetLabDescription(FieldsPtr pFields)
{
	if (!pFields) {
		ThrowNxException("%s : pFields is NULL", __FUNCTION__);
	}

	CString strFormNumberTextID = AdoFldString(pFields, "FormNumberTextID", "");
	CString strSpecimen = AdoFldString(pFields, "Specimen", "");

	// Use to be ordered instead of anatomic location
	// for non-biopsy labs
	LabType eType = (LabType)AdoFldByte(pFields, "Type");
	CString strTypeDesc;
	if (eType == ltBiopsy) {
		//Assemble the anatomic location(s)
		CString strAnatomicLocation = AdoFldString(pFields, "AnatomicLocation", "");
		AnatomySide as = (AnatomySide)AdoFldLong(pFields, "AnatomySide", 0);
		CString strQualifier = AdoFldString(pFields, "LocationQualifier", "");
		strTypeDesc = ::FormatAnatomicLocation(strAnatomicLocation, strQualifier, as);
	} else {
		strTypeDesc = AdoFldString(pFields, "ToBeOrdered", "");
	}

	// Output the LOINC code and description as well.
	CString strTestCode = AdoFldString(pFields, "LOINC_Code", "");
	CString strTestDescription = AdoFldString(pFields, "LOINC_Description", "");
	CString strTestCodeAndDescription;
	strTestCodeAndDescription += strTestCode;
	if (!strTestDescription.IsEmpty()) {
		if (!strTestCodeAndDescription.IsEmpty()) {
			strTestCodeAndDescription += " - ";
		}
		strTestCodeAndDescription += strTestDescription;
	}
	if (!strTestCodeAndDescription.IsEmpty()) {
		strTestCodeAndDescription = FormatString("(%s)", strTestCodeAndDescription);
		if (!strTypeDesc.IsEmpty()) {
			strTypeDesc += " ";
		}
		strTypeDesc += strTestCodeAndDescription;
	}

	// Assemble the description for the lab
	CString strDescription = strFormNumberTextID;
	if (!strSpecimen.IsEmpty()) {
		strDescription += " - " + strSpecimen;
	}
	if (!strTypeDesc.IsEmpty()) {
		strDescription += " - " + strTypeDesc;
	}

	return strDescription;
}

// (r.gonet 06/02/2014) - PLID 40426 - Added.
/// <summary>
/// Handler for the SelChosen event for the Result combo.
/// </summary>
/// <param name="lpRow">The row that was selected by the user.</param>
void CMovePatientLabsDlg::SelChosenMplResultCombo(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			ThrowNxException("%s : pRow is NULL", __FUNCTION__);
		}

		long nID = VarLong(pRow->GetValue((short)EResultComboColumns::ID));
		if (nID == -1) { 
			// They chose <Select a Lab Result>, which means nothing was selected.
			m_arySelectedLabResultIDs.RemoveAll();
		} else if (nID == -2) {
			// They chose to select multiple results, so pop up the multi selection dialog.
			SelectMultipleResults();
		} else {
			// They chose a single result, so reflect that in our array of selected result IDs.
			m_arySelectedLabResultIDs.RemoveAll();
			m_arySelectedLabResultIDs.Add(nID);
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 06/02/2014) - PLID 40426 - Added.
/// <summary>
/// Handler for the SelChosen event for the To Patient combo.
/// </summary>
/// <param name="lpRow">The row that was selected by the user.</param>
void CMovePatientLabsDlg::SelChosenMplToPatientCombo(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			ThrowNxException("%s : pRow is NULL", __FUNCTION__);
		}
		long nToPatientID = VarLong(pRow->GetValue((short)EToPatientComboColumns::PersonID));
		// The To Order combo is tied with the the To Patient combo, so reload that.
		ReloadToOrderCombo(nToPatientID);
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 06/02/2014) - PLID 40426 - Added.
/// <summary>
/// Handler for when the user clicks the Move button. Performs the move of the results to the new lab order. Closes the dialog if successful.
/// </summary>
void CMovePatientLabsDlg::OnBnClickedMplMoveButton()
{
	long nAuditTransID = -1;

	try {
		// We can't move a lab result record without permission to write
		if (!CheckCurrentUserPermissions(bioPatientLabs, sptDynamic4)) {
			return;
		}

		// Make sure they made selections.
		if (m_arySelectedLabResultIDs.GetSize() == 0) {
			MessageBox("Please select a lab result to move.", NULL, MB_ICONERROR | MB_OK);
			return;
		}
		long nToPatientID = -1;
		NXDATALIST2Lib::IRowSettingsPtr pToPatientRow = m_pToPatientCombo->CurSel;
		if (pToPatientRow == NULL || (nToPatientID = VarLong(pToPatientRow->GetValue((short)EToPatientComboColumns::PersonID), -1)) == -1) {
			MessageBox("Please select a patient.", NULL, MB_ICONERROR | MB_OK);
			return;
		}
		long nToLabID = -1;
		NXDATALIST2Lib::IRowSettingsPtr pToOrderRow = m_pToOrderCombo->CurSel;
		if (pToOrderRow == NULL || (nToLabID = VarLong(pToOrderRow->GetValue((short)EToOrderComboColumns::ID), -1)) == -1) {
			MessageBox("Please select a lab order.", NULL, MB_ICONERROR | MB_OK);
			return;
		}

		// Run through various usage checks for dependencies.

		// Check if this lab result has been corrected 
		CParamSqlBatch sqlUsageChecks;
		sqlUsageChecks.Add(
			"SELECT "
			"CONVERT(BIT, CASE WHEN LinkedLabsQ.LinkedLabID IS NULL THEN 0 ELSE 1 END) AS LabWasReplaced, "
			"CONVERT(BIT, CASE WHEN LinkedResultsQ.LinkedResultID IS NULL THEN CASE WHEN LabResultsT.HL7MessageID NOT IN (SELECT HL7MessageID FROM LabResultsT WHERE LinkedLabID IS NOT NULL) AND LinkedLabsQ.LinkedLabID IS NOT NULL THEN 1 ELSE 0 END ELSE 1 END) AS ResultWasReplaced "
			"FROM LabsT "
			"INNER JOIN LabResultsT ON LabsT.ID = LabResultsT.LabID "
			"LEFT JOIN (SELECT LinkedLabID FROM LabResultsT WHERE Deleted = 0 GROUP BY LinkedLabID) LinkedLabsQ ON LabsT.ID = LinkedLabsQ.LinkedLabID "
			"LEFT JOIN (SELECT LinkedResultID FROM LabResultsT WHERE Deleted = 0 GROUP BY LinkedResultID) LinkedResultsQ ON LabResultsT.ResultID = LinkedResultsQ.LinkedResultID "
			"WHERE LabResultsT.ResultID IN ({INTARRAY}); "
			, m_arySelectedLabResultIDs);

		// Check if this lab result is a correction 
		sqlUsageChecks.Add(
			"SELECT LabResultsT.LinkedLabID, LabResultsT.LinkedResultID " 
			"FROM LabResultsT "
			"WHERE LabResultsT.ResultID IN ({INTARRAY}); "
			, m_arySelectedLabResultIDs);
		
		// Check for todos (they won't be moved because we don't know what they are actually for)
		sqlUsageChecks.Add(
			"SELECT NULL "
			"FROM TodoList "
			"INNER JOIN LabsT ON TodoList.RegardingID = LabsT.ID AND TodoList.RegardingType = {INT} "
			"INNER JOIN LabResultsT ON LabsT.ID = LabResultsT.LabID "
			"WHERE LabResultsT.ResultID IN ({INTARRAY}) AND TodoList.Done IS NULL; "
			, (long)ttLab, m_arySelectedLabResultIDs);
		
		if (m_nFromPatientID != nToPatientID) {
			// Check for HL7 patient links
			sqlUsageChecks.Add(
				"SELECT DISTINCT HL7IDLinkT.ID, HL7IDLinkT.GroupID, HL7IDLinkT.ThirdPartyID, PersonT.FullName, HL7SettingsT.Name AS GroupName "
				"FROM LabResultsT "
				"INNER JOIN LabsT ON LabsT.ID = LabResultsT.LabID "
				"INNER JOIN HL7MessageQueueT ON LabResultsT.HL7MessageID = HL7MessageQueueT.ID "
				"INNER JOIN HL7SettingsT ON HL7MessageQueueT.GroupID = HL7SettingsT.ID "
				"INNER JOIN HL7IDLinkT ON HL7MessageQueueT.GroupID = HL7IDLinkT.GroupID "
				"INNER JOIN PersonT ON HL7IDLinkT.PersonID = PersonT.ID AND HL7IDLinkT.PersonID = LabsT.PatientID "
				"WHERE HL7IDLinkT.RecordType = {INT} AND HL7IDLinkT.PersonID = {INT} AND LabResultsT.ResultID IN({INTARRAY}); "
				, (long)hilrtPatient, m_nFromPatientID, m_arySelectedLabResultIDs);

			// Check related mailsent entries if we are moving to a different patient
			sqlUsageChecks.Add(
				"SELECT DISTINCT MailSent.MailID, MailSent.PersonID, MailSent.Selection, MailSent.PathName, MailSent.Subject, MailSent.Sender, MailSent.Date, MailSent.Location, "
				"	MailSent.MailBatchID, MailSent.InternalRefID, MailSent.InternalTblName, MailSent.MergedPacketID, MailSent.ServiceDate, MailSent.CategoryID, "
				"	MailSent.Checksum, MailSent.PicID, MailSent.LabStepID, MailSent.EMNID, MailSent.IsPhoto, MailSent.TemplateID, MailSent.PictureOrigID, MailSent.CCDAtypeField, "
				"	MailSentNotesT.Note "
				"FROM MailSent "
				"LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
				"INNER JOIN LabResultsT ON MailSent.MailID = LabResultsT.MailID "
				"WHERE LabResultsT.ResultID IN ({INTARRAY}); "
				, m_arySelectedLabResultIDs);
		}

		// If the lab or lab result was corrected, then should we really be moving it? Considering the lab company thought the results were wrong to begin with?
		_RecordsetPtr prsUsageChecks = sqlUsageChecks.CreateRecordset(GetRemoteDataSnapshot());
		if (!prsUsageChecks->eof) {
			BOOL bLabWasCorrected = AdoFldBool(prsUsageChecks->Fields, "LabWasReplaced", FALSE);
			if (bLabWasCorrected) {
				if (IDYES != MessageBox("One of the results being moved is part of a lab order that was amended or replaced with a correction. Are you sure you want to continue the move?", "Amended/Corrected Lab", MB_ICONWARNING | MB_YESNO)) {
					return;
				}
			}
			BOOL bResultWasCorrected = AdoFldBool(prsUsageChecks->Fields, "ResultWasReplaced", FALSE);
			if (bLabWasCorrected) {
				if (IDYES != MessageBox("One of the results being moved was amended or replaced with a correction. Are you sure you want to continue the move?", "Amended/Corrected Lab Result", MB_ICONWARNING | MB_YESNO)) {
					return;
				}
			}
		}

		// If the lab result was a correction of a previous result, then should we really be moving it? Considering that it will leave the previous results uncorrected.
		prsUsageChecks = prsUsageChecks->NextRecordset(NULL);
		if (!prsUsageChecks->eof) {
			bool bLabIsCorrection = (AdoFldLong(prsUsageChecks->Fields, "LinkedLabID", -1) != -1);
			if (bLabIsCorrection) {
				if (IDYES != MessageBox("One of the results being moved is part of a lab order that is an amendment or correction of a previous lab order. Are you sure you want to continue the move?", "Lab Correction/Amendment", MB_ICONWARNING | MB_YESNO)) {
					return;
				}
			}
			bool bResultIsCorrection = (AdoFldLong(prsUsageChecks->Fields, "LinkedResultID", -1) != -1);
			if (bResultIsCorrection) {
				if (IDYES != MessageBox("One of the results being moved is an amendment correction of a previous result. Are you sure you want to continue the move?", "Lab Result Correction/Amendment", MB_ICONWARNING | MB_YESNO)) {
					return;
				}
			}
		}

		// Check for todos linked to the lab the results are being moved from. They may now be out of date.
		prsUsageChecks = prsUsageChecks->NextRecordset(NULL);
		if (!prsUsageChecks->eof) {
			MessageBox("This lab is linked to a to-do alarm. To-do alarms will not be moved. After the results are moved, please verify the to-dos are still relevant to the lab the results were moved from.", "To-Dos Exist", MB_ICONWARNING | MB_OK);
		}

		// Gather any mailsent records associated with the results and check if they are in use elsewhere besides on the results we are moving.

		// Array of all the HL7IDLinks to delete
		CArray<long, long> aryHL7IDLinksToDelete;
		// Array of all the mailsent entries to move to the new patient
		CArray<CMailSentEntry, CMailSentEntry&> aryMailSentEntriesToMove;
		// 1 to 1 with the aryMailsentEntriesToMove array. Whether or not we should copy rather than move the mailsent entry due to existing dependencies.
		CArray<bool, bool> aryMailSentDoCopy;
		if (m_nFromPatientID != nToPatientID) {
			if (nAuditTransID == -1) {
				nAuditTransID = BeginAuditTransaction();
			}

			prsUsageChecks = prsUsageChecks->NextRecordset(NULL);
			while (!prsUsageChecks->eof) {
				long nHL7IDLinkID = AdoFldLong(prsUsageChecks->Fields, "ID");
				long nGroupID = AdoFldLong(prsUsageChecks->Fields, "GroupID");
				CString strThirdPartyID = AdoFldString(prsUsageChecks->Fields, "ThirdPartyID");
				CString strFullName = AdoFldString(prsUsageChecks->Fields, "FullName");
				CString strGroupName = AdoFldString(prsUsageChecks->Fields, "GroupName");
				aryHL7IDLinksToDelete.Add(nHL7IDLinkID);

				// We need to audit in advance while we still have the record's values. We'll rollback if we fail.
				CString strOld, strNew;
				strOld.Format("Patient ID '%s', Linked to %s (HL7 Group '%s')", strThirdPartyID, strFullName, strGroupName);
				strNew.Format("<Code Removed>");
				AuditEvent(-1, "", nAuditTransID, aeiHL7PatientLink, nGroupID, strOld, strNew, aepLow, aetDeleted);

				prsUsageChecks->MoveNext();
			}

			prsUsageChecks = prsUsageChecks->NextRecordset(NULL);
			while (!prsUsageChecks->eof) {
				// Just need a container for all the fields 
				CMailSentEntry mailSentEntry;
				FillMailSentEntry(prsUsageChecks->Fields, mailSentEntry);
				aryMailSentEntriesToMove.Add(mailSentEntry);
				prsUsageChecks->MoveNext();
			}
			prsUsageChecks->Close();

			// Check if the mailsent entries are in use by other labs or EMNs, etc. If they are, then we have the option of just copying them rather than moving them.
			for (long i = 0; i < aryMailSentEntriesToMove.GetSize(); i++) {
				CMailSentEntry &mailSentEntry = aryMailSentEntriesToMove.GetAt(i);
				if (IsDocumentInUse(aryMailSentEntriesToMove.GetAt(i))) {
					if (IDYES != MessageBox(FormatString("The lab result-associated document '%s' is in use on this patient and so cannot be moved. Do you want to create a copy of the document instead? Selecting No will abort the entire move.",
						mailSentEntry.m_strPathName), "Document Is In Use", MB_ICONQUESTION | MB_YESNO)) {
						
						if (nAuditTransID > -1) {
							RollbackAuditTransaction(nAuditTransID);
						}
						return;
					} else {
						aryMailSentDoCopy.Add(true);
					}
				} else {
					aryMailSentDoCopy.Add(false);
				}
			}
		} else {
			// No need to move any documents. The patient is the same.
			prsUsageChecks->Close();
		}

		// Prompt them if they are sure about the move before going to the point of no return.
		CString strLabOrderDescription = VarString(pToOrderRow->GetValue((short)EToOrderComboColumns::Description), "");
		if (IDYES != MessageBox(
			FormatString("Are you sure you want to move %s to the lab order '%s'? Selecting No will abort the entire move."
			, (m_arySelectedLabResultIDs.GetSize() > 1 ? FormatString("these %li lab results", m_arySelectedLabResultIDs.GetSize()) : "this lab result"), 
			strLabOrderDescription), NULL, MB_ICONQUESTION | MB_YESNO))
		{
			if (nAuditTransID > -1) {
				RollbackAuditTransaction(nAuditTransID);
			}
			return;
		}

		// POINT OF NO RETURN

		// Move (or copy) the mailsent entries first.
		CString strMailSentMoveErrors;
		bool bOneMailSentFailed = false;
		for (long i = 0; i < aryMailSentEntriesToMove.GetSize(); i++) {
			bool bCopy = aryMailSentDoCopy.GetAt(i);
			CMailSentEntry &mailSentEntry = aryMailSentEntriesToMove.GetAt(i);
			if (!MoveMailSent(mailSentEntry, nToPatientID, bCopy)) {
				bOneMailSentFailed = true;
				strMailSentMoveErrors += "\r\n" + mailSentEntry.m_strPathName;
			}
		}

		// Now do the move of the results. Remove the dependencies (the user said they were OK with the potentially unsafe ones).
		CParamSqlBatch sqlMoveBatch;
		sqlMoveBatch.Add("SET NOCOUNT ON; ");
		if (m_nFromPatientID != nToPatientID) {
			// Notes are also associated with a patient, so move them if we are switching patients.
			sqlMoveBatch.Add("UPDATE Notes SET PersonID = {INT} WHERE LabResultID IN ({INTARRAY}); ", m_nFromPatientID, m_arySelectedLabResultIDs);
		}
		// Moved results can no longer be reflex sources
		sqlMoveBatch.Add("UPDATE LabsT SET ParentResultID = NULL WHERE ParentResultID IN ({INTARRAY});", m_arySelectedLabResultIDs);
		// Since the lab is being changed, it does not make sense to keep the linked lab ID (especially if it is on a different patient)
		sqlMoveBatch.Add("UPDATE LabResultsT SET LinkedLabID = NULL WHERE ResultID IN ({INTARRAY});", m_arySelectedLabResultIDs);
		// Remove the association between corrections (unless we are moving the correction and the corrected result together)
		sqlMoveBatch.Add("UPDATE LabResultsT SET LinkedResultID = NULL WHERE ResultID IN ({INTARRAY}) AND LinkedResultID NOT IN ({INTARRAY});", m_arySelectedLabResultIDs, m_arySelectedLabResultIDs);
		// Conversely, moved results can no longer be corrected (unless we are moving the correction and the corrected result together)
		sqlMoveBatch.Add("UPDATE LabResultsT SET LinkedResultID = NULL WHERE LinkedResultID IN ({INTARRAY}) AND ResultID NOT IN ({INTARRAY});", m_arySelectedLabResultIDs, m_arySelectedLabResultIDs);
		// Now move the lab result
		sqlMoveBatch.Add("UPDATE LabResultsT SET LabID = {INT} WHERE LabResultsT.ResultID IN ({INTARRAY}); ", nToLabID, m_arySelectedLabResultIDs);
		// Delete the associated HL7IDLinkT records. Because if they imported a result to the wrong patient, we'll have a bad HL7 ID link, and the mismatch is just
		// going to happen again.
		sqlMoveBatch.Add("DELETE FROM HL7IDLinkT WHERE ID IN ({INTARRAY}); ", aryHL7IDLinksToDelete);
		sqlMoveBatch.Add("SET NOCOUNT OFF; ");
		// Grab information needed to audit
		sqlMoveBatch.Add(
			"SELECT LabsT.ID, LabsT.FormNumberTextID, LabsT.Specimen, LabsT.LOINC_Code, LabsT.LOINC_Description, "
			"	LabsT.Type, LabsT.AnatomySide, LabAnatomyT.Description AS AnatomicLocation, AnatomyQualifiersT.Name AS LocationQualifier, "
			"	LabsT.ToBeOrdered "
			"FROM LabsT "
			"LEFT JOIN LabAnatomyT ON LabAnatomyT.ID = LabsT.AnatomyID "
			"LEFT JOIN AnatomyQualifiersT ON AnatomyQualifiersT.ID = LabsT.AnatomyQualifierID "
			"WHERE LabsT.ID IN ({INT}, {INT}); "
			, m_nFromLabID, nToLabID);
		sqlMoveBatch.Add("SELECT LabResultsT.ResultID, (CASE WHEN LabResultsT.Name <> '' THEN LabResultsT.Name WHEN COALESCE(LabResultsT.LOINC, '') <> '' THEN LabResultsT.LOINC ELSE ' { Unnamed Result }' END) AS ResultName FROM LabResultsT WHERE LabResultsT.ResultID IN ({INTARRAY}); ", m_arySelectedLabResultIDs);
		_RecordsetPtr prsMoveResults = sqlMoveBatch.CreateRecordset(GetRemoteData());


		CString strOldPatientName = GetExistingPatientName(m_nFromPatientID);
		CString strNewPatientName = GetExistingPatientName(nToPatientID);

		CString strOldLab, strNewLab;
		while (!prsMoveResults->eof) {
			long nLabID = AdoFldLong(prsMoveResults->Fields, "ID");
			CString strLabDescription = GetLabDescription(prsMoveResults->Fields);
			if (nLabID == m_nFromLabID) {
				strOldLab = strLabDescription;
			} else if (nLabID == nToLabID) {
				strNewLab = strLabDescription;
			}
			prsMoveResults->MoveNext();
		}
		prsMoveResults = prsMoveResults->NextRecordset(NULL);

		// Now audit
		if (nAuditTransID == -1) {
			nAuditTransID = BeginAuditTransaction();
		}

		// Audit each result that was moved.
		while (!prsMoveResults->eof) {
			long nResultID = AdoFldLong(prsMoveResults->Fields, "ResultID");
			CString strResultName = AdoFldString(prsMoveResults->Fields, "ResultName");
			CString strOldValue = FormatString("Patient: %s; Lab: %s (Result: %s)", strOldPatientName, strOldLab, strResultName);
			CString strNewValue = FormatString("Patient: %s; Lab: %s", strNewPatientName, strNewLab);
			AuditEvent(m_nFromPatientID, strOldPatientName, nAuditTransID, aeiLabResultMoved, nResultID, strOldValue, strNewValue, aepMedium, aetChanged);
			prsMoveResults->MoveNext();
		}
		prsMoveResults->Close();

		if (nAuditTransID != -1) {
			CommitAuditTransaction(nAuditTransID);
		}

		// (r.gonet 08/25/2014) - PLID 63221 - Send a table checker for each individual lab.
		if (m_nFromPatientID != nToPatientID) {
			CClient::RefreshLabsTable(m_nFromPatientID, m_nFromLabID);
			CClient::RefreshLabsTable(nToPatientID, nToLabID);
		} else {
			// Just send one.
			CClient::RefreshLabsTable(m_nFromPatientID, -1);
		}

		if (bOneMailSentFailed) {
			MessageBox(FormatString("Nextech failed to move at least one lab result-associated document to the new patient. Please move the below document(s) manually.\r\n%s"
				, strMailSentMoveErrors), "Error", MB_ICONERROR | MB_OK);
		}

		// Close the dialog.
		CNxDialog::OnOK();
	} NxCatchAllCall(__FUNCTION__,
		if (nAuditTransID > -1) {
			RollbackAuditTransaction(nAuditTransID);
		}
	);
}

// (r.gonet 06/02/2014) - PLID 40426 - Added.
/// <summary>
/// Opens the multi-select dialog and lets the user select one or more lab results, then reflects this in the interface.
/// </summary>
void CMovePatientLabsDlg::SelectMultipleResults()
{
	CMultiSelectDlg dlg(this, "MoveLabResults_Results");
	dlg.PreSelect(m_arySelectedLabResultIDs);

	// Ignore the special rows.
	CVariantArray varyIdsToSkip;
	varyIdsToSkip.Add(_variant_t(-1L, VT_I4));
	varyIdsToSkip.Add(_variant_t(-2L, VT_I4));

	// Show the other columns from the dropdown in the multi-select
	CArray<short, short> aryExtraColumns;
	aryExtraColumns.Add((short)EResultComboColumns::ReceivedDate);
	aryExtraColumns.Add((short)EResultComboColumns::Value);
	aryExtraColumns.Add((short)EResultComboColumns::FinalDiagnosis);
	aryExtraColumns.Add((short)EResultComboColumns::MicroscopicDescription);
	aryExtraColumns.Add((short)EResultComboColumns::Comments);

	// Open the multi-select
	if (IDOK == dlg.OpenWithDataList2(m_pResultCombo, varyIdsToSkip, "Select one or more lab results to move:", 1, 0xFFFFFFFF, (short)EResultComboColumns::ID, (short)EResultComboColumns::Name, &aryExtraColumns)) {
		// They chose some results. Great.
		m_arySelectedLabResultIDs.RemoveAll();
		dlg.FillArrayWithIDs(m_arySelectedLabResultIDs);

		if (m_arySelectedLabResultIDs.GetSize() > 1) {
			// They selected multiple results. Show the multi-select label.
			CString strMultiSelectString = dlg.GetMultiSelectString();
			m_nxlMultiResults.SetText(strMultiSelectString);
			m_nxlMultiResults.SetType(dtsHyperlink);
			m_nxlMultiResults.ShowWindow(SW_SHOW);
			GetDlgItem(IDC_MPL_RESULT_COMBO)->ShowWindow(SW_HIDE);
		} else {
			// They just chose one result. Show the combo and select that result.
			m_nxlMultiResults.SetText("");
			m_nxlMultiResults.SetType(dtsDisabledHyperlink);
			m_nxlMultiResults.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MPL_RESULT_COMBO)->ShowWindow(SW_SHOW);
			
			long nNewID = m_arySelectedLabResultIDs.GetAt(0);
			m_pResultCombo->FindByColumn((short)EResultComboColumns::ID, _variant_t(nNewID, VT_I4), m_pResultCombo->GetFirstRow(), VARIANT_TRUE);
		}
	} else if (m_arySelectedLabResultIDs.GetSize() == 1) {
		// Revert to last selected
		m_pResultCombo->FindByColumn((short)EResultComboColumns::ID, _variant_t(m_arySelectedLabResultIDs.GetAt(0), VT_I4), m_pResultCombo->GetFirstRow(), VARIANT_TRUE);
	} else if (m_arySelectedLabResultIDs.GetSize() == 0) {
		// Revert to default
		m_pResultCombo->FindByColumn((short)EResultComboColumns::ID, _variant_t(-1L, VT_I4), m_pResultCombo->GetFirstRow(), VARIANT_TRUE);
	} else {
		// No need to revert since we are showing the label and didn't change it.
	}
}

// (r.gonet 06/02/2014) - PLID 40426 - Added.
/// <summary>
/// Gets the document path for patient and caches the result.
/// </summary>
/// <param name="nPatientID">The PersonID of the patient to get the document path of.</param>
/// <returns>The patient's document folder path.</returns>
CString CMovePatientLabsDlg::GetPatientDocumentPath(long nPatientID)
{
	// We may already have the result cached.
	static CMap<long, long, CString, CString&> mapPatientToDocumentPath;
	CString strDocumentPath;
	if (mapPatientToDocumentPath.Lookup(nPatientID, strDocumentPath)) {
		// It is cached.
		return strDocumentPath;
	} else {
		// Get it and cache it.
		strDocumentPath = ::GetPatientDocumentPath(nPatientID);
		mapPatientToDocumentPath.SetAt(nPatientID, strDocumentPath);
	}
	return strDocumentPath;
}

// (r.gonet 06/02/2014) - PLID 40426 - Added.
/// <summary>
/// Moves a mail sent entry and the underlying document file to a new patient.
/// </summary>
/// <param name="mailSentEntry">The mail sent entry to move.</param>
/// <param name="nToPatientID">The ID of the patient to move the document to.</param>
/// <param name="bCopy">If true, then copy the document to the new patient's document folder and create a new mailsent entry rather than moving it.</param>
/// <returns>True if the move/copy succeeded and false if it failed.</returns>
bool CMovePatientLabsDlg::MoveMailSent(CMailSentEntry &mailSentEntry, long nToPatientID, bool bCopy)
{
	long nAuditTransID = -1;

	try {
		// We can't move a mailsent record without permission.
		if (!CheckCurrentUserPermissions(bioPatientHistory, sptWrite)) {
			return false;
		}

		CString strFileName, strPath;
		GetPathComponents(mailSentEntry, strFileName, strPath);
		
		CString strOriginalFileName = strFileName;
		CString strOriginalFilePath;
		CString strNewFilePath;
		CString strNewFileName = strFileName;

		BOOL bMoveFile = TRUE;
		BOOL bIsPatientPath = FALSE;
		if (strPath != strFileName) {
			//don't move the file if its attached somewhere other then the current patient's folder
			bMoveFile = FALSE;
			//we need to change where this is currently, just in case they rename it below
			strOriginalFilePath = GetFilePath(strPath);
			strNewFilePath = GetFilePath(strPath);
			bIsPatientPath = FALSE;
		} else {
			//they are using the patient's folder

			// (a.walling 2011-02-11 15:13) - PLID 42441 - Always use GetCurPatientDocumentPath instead of GetPatientDocumentPath
			strOriginalFilePath = this->GetPatientDocumentPath(mailSentEntry.m_nPersonID);
			strNewFilePath = this->GetPatientDocumentPath(nToPatientID);
			bIsPatientPath = TRUE;
		}

		//pop up an input box to have them change the name if necessary
		CString strChange, strTemp;
		strChange = strFileName;
		strTemp = strChange;
		bool bPromptForFileNameAgain;
		do {
			bPromptForFileNameAgain = false;
			if (InputBox(this, "This file may contain patient information, such as patient name, please update the filename if necessary", strChange, "")) {
				if (strChange != strTemp) {
					if (strChange.IsEmpty()) {
						MsgBox("The filename cannot be blank.");
						bPromptForFileNameAgain = true;
					} else {
						bMoveFile = TRUE;
						strNewFileName = MakeValidFolderName(strChange);
					}
				}
			}
			// Don't accept nothing for an answer.
		} while (bPromptForFileNameAgain);

		CParamSqlBatch sqlMoveBatch;

		CString strNewPathName = mailSentEntry.m_strPathName;
		if (bMoveFile) {
			//try to copy the file
			BOOL bFileMoved = FALSE;
			CString strNewFileNameChanged;
			CString strDstPath = strNewFileName;
			DWORD dwLastError = -1;

			BOOL bResult;
			if (bCopy) {
				bResult = CopyFile(strOriginalFilePath ^ strOriginalFileName, strNewFilePath ^ strNewFileName, TRUE);
			} else {
				bResult = MoveFile(strOriginalFilePath ^ strOriginalFileName, strNewFilePath ^ strNewFileName);
			}

			DWORD dwPreviousLastError = -1;
			while (!bResult) {
				dwPreviousLastError = dwLastError;
				dwLastError = GetLastError();
				//does the filename already exist?
				if (dwLastError == ERROR_FILE_EXISTS || dwLastError == ERROR_ALREADY_EXISTS) {
					// A file already exists by that name in the destination. Tell the user to rename our file and then let's try again.
					CRenameFileDlg dlgRename(strNewFilePath ^ strNewFileName, strNewFilePath, this);
					if (dlgRename.DoModal() == IDOK) {
						strDstPath = dlgRename.m_strDstFullPath;
						strNewFileName = GetFileName(strDstPath);

						if (bCopy) {
							bResult = CopyFile(strOriginalFilePath ^ strOriginalFileName, strNewFilePath ^ strNewFileName, TRUE);
						} else {
							bResult = MoveFile(strOriginalFilePath ^ strOriginalFileName, strNewFilePath ^ strNewFileName);
						}
					} else {
						bFileMoved = FALSE;
						// Don't retry
						break;
					}
				// If we are trying to move the file, and we haven't just tried copying and doing a delayed delete, then try that.
				// The check for the dwPreviousLastError is to prevent infinite looping.
				} else if (dwLastError == ERROR_SHARING_VIOLATION && dwPreviousLastError != dwLastError  && !bCopy) {
					// Most likely our parent window has it open. 
					bResult = CopyFile(strOriginalFilePath ^ strOriginalFileName, strNewFilePath ^ strNewFileName, TRUE);
					if (bResult) {
						DeleteFileWhenPossible(strOriginalFilePath ^ strOriginalFileName);
					}
				} else {
					// Don't retry
					break;
				}
			}

			if(bResult) {
				bFileMoved = TRUE;
			} else {
				bFileMoved = FALSE;
			}

			if (!bFileMoved) {
				// We failed to move/copy the file!
				CString strError;
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwLastError, 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
				strError.ReleaseBuffer();
				MessageBox(FormatString("Could not move the document from '%s' to '%s'. \n\nError: %s",
					strOriginalFilePath ^ strOriginalFileName, strDstPath, strError), "Error", MB_ICONERROR | MB_OK);
				return false;
			}

			//if we got here, we succeeded in moving the file, now update the data						
			if ((strOriginalFileName != strNewFileName)) {
				if (bIsPatientPath) {
					strNewPathName = strNewFileName;
				} else {
					strNewPathName = strNewFilePath ^ strNewFileName;
				}
			}

		}

		long nOldMailID = mailSentEntry.m_nMailID;
		long nNewMailID = -1;
		if (bCopy) {
			// We have found there to be other records depending on this one. So we must create a new MailSent record.
			nNewMailID = CreateNewMailSentEntry(nToPatientID, mailSentEntry.m_strNote, mailSentEntry.m_strSelection, strNewPathName,
				mailSentEntry.m_strSender, mailSentEntry.m_strSubject, mailSentEntry.m_nLocation, mailSentEntry.m_dtServiceDate, 
				mailSentEntry.m_nChecksum, mailSentEntry.m_nCategoryID, -1, -1, mailSentEntry.m_bIsPhoto, -1, "", ctNone);
			// Put the moved results on the new mailsent record
			sqlMoveBatch.Add("UPDATE LabResultsT SET MailID = {INT} WHERE MailID = {INT} AND ResultID IN ({INTARRAY}); ", nNewMailID, nOldMailID, m_arySelectedLabResultIDs);
		} else {
			nNewMailID = nOldMailID;
			if (strOriginalFileName != strNewFileName) {
				//the file name got changed along the way
				sqlMoveBatch.Add("UPDATE MailSent set PathName = {STRING} WHERE MailID = {INT};", strNewPathName, mailSentEntry.m_nMailID);
			}
			sqlMoveBatch.Add("UPDATE MailSent SET PersonID = {INT} WHERE MailID = {INT};", nToPatientID, mailSentEntry.m_nMailID);
			// We also need to move notes that are assigned to the MailSent record
			sqlMoveBatch.Add("UPDATE Notes SET PersonID = {INT} WHERE MailID = {INT};", nToPatientID, mailSentEntry.m_nMailID);
			// Also move the todos associated with the mailsent record.
			sqlMoveBatch.Add("UPDATE TodoList SET PersonID = {INT} WHERE RegardingID = {INT} AND RegardingType = {INT};", nToPatientID, mailSentEntry.m_nMailID, (long)ttMailSent);
		}
		sqlMoveBatch.Execute(GetRemoteData());

		// Now audit
		if (nAuditTransID == -1) {
			nAuditTransID = BeginAuditTransaction();
		}

		if (!bCopy) {
			// Only audit the detach if we actually moved the file. If we copied, we didn't detach.
			AuditEvent(mailSentEntry.m_nPersonID, GetExistingPatientName(mailSentEntry.m_nPersonID), nAuditTransID, aeiPatientDocDetach, nOldMailID, strOriginalFileName, "", aepMedium, aetDeleted);
		}
		AuditEvent(nToPatientID, GetExistingPatientName(nToPatientID), nAuditTransID, aeiPatientDocumentAttach, nNewMailID, "", strNewFileName, aepMedium, aetCreated);

		if (nAuditTransID != -1) {
			CommitAuditTransaction(nAuditTransID);
		}

		//send a messsage
		// (j.jones 2011-07-22 16:04) - PLID 21784 - this never used the available nMailID before
		// (j.jones 2014-08-04 13:31) - PLID 63141 - this now up to two Ex tablecheckers, one for each patient
		if (!bCopy) {
			//we detached the old file
			CClient::RefreshMailSentTable(mailSentEntry.m_nPersonID, nOldMailID);
		}
		CClient::RefreshMailSentTable(nToPatientID, nNewMailID);

		return true;
	} NxCatchAllCall(__FUNCTION__,
		if (nAuditTransID > -1) {
			RollbackAuditTransaction(nAuditTransID);
		}
	);

	return false;
}

// (r.gonet 06/02/2014) - PLID 40426 - Added.
/// <summary>Handles the WM_SETCURSOR message. Sets the mouse cursor to a hand when hovering over multi-select label hyperlinks.</summary>
/// <param name="pWnd">Specifies a pointer to the window that contains the cursor. The pointer may be temporary and should not be used for later use.</param>
/// <param name="nHitTest">Specifies the hit-test area code. The hit test determines the cursor's location.</param>
/// <param name="message">Specifies the mouse message number.</param>
/// <returns>Nonzero to halt further processing, or 0 to continue.</return>
BOOL CMovePatientLabsDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	try {
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		// If our multi select label is visible and enabled, and the mouse is over it,
		// set the link cursor.
		if (m_nxlMultiResults.IsWindowVisible() && m_nxlMultiResults.IsWindowEnabled()) {
			m_nxlMultiResults.GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	}NxCatchAllCallIgnore({
		if (m_bNotifyOnce) {
			m_bNotifyOnce = false;
			try { throw; }NxCatchAll(__FUNCTION__);
		}
	});
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (r.gonet 06/02/2014) - PLID 40426 - Added.
/// <summary>Handles when the user left clicks a CNxLabel control.</summary>
/// <param name="wParam">The nFlags parameter of the WM_LBUTTONDOWN message.</param>
/// <param name="lParam">IDC number of the CNxLabel control.</param>
/// <returns>Nonzero to halt further processing, or 0 to continue.</return>
LRESULT CMovePatientLabsDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		UINT nIdc = (UINT)wParam;
		switch (nIdc) {
		case IDC_MPL_MULTI_RESULTS_STATIC:
			SelectMultipleResults();
			break;

		default:
			//Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	} NxCatchAll(__FUNCTION__);
	return 0;
}

/// <summary>
/// Constructs a new empty instance of CMailSentEntry.
/// </summary>
CMovePatientLabsDlg::CMailSentEntry::CMailSentEntry()
{
	m_nMailID = -1;
	m_nPersonID = -1;
	m_dtDate = g_cdtInvalid;
	m_nLocation = -1;
	m_nMailBatchID = -1;
	m_nInternalRefID = -1;
	m_nMergedPacketID = -1;
	m_dtServiceDate = g_cdtInvalid;
	m_nCategoryID = -1;
	m_nChecksum = -1;
	m_nPicID = -1;
	m_nLabStepID = -1;
	m_nEMNID = -1;
	m_bIsPhoto = FALSE;
	m_nTemplateID = -1;
	m_nPictureOrigID = -1;
	// 0 is None.
	m_nCCDAtypeField = 0;
}

/// <summary>
/// Constructs a new instance of CMailSentEntry
/// </summary>
CMovePatientLabsDlg::CMailSentEntry::CMailSentEntry(long nMailID, long nPersonID, CString strSelection, CString strPathName, CString strSubject, CString strSender,
	COleDateTime dtDate, long nLocation, long nMailBatchID, long nInternalRefID, CString strInternalTblName, long nMergedPacketID, COleDateTime dtServiceDate,
	long nCategoryID, long nChecksum, long nPicID, long nLabStepID, long nEMNID, BOOL bIsPhoto, long nTemplateID, long nPictureOrigID, long nCCDAtypeField, CString strNote)
{
	m_nMailID = nMailID;
	m_nPersonID = nPersonID;
	m_strSelection = strSelection;
	m_strPathName = strPathName;
	m_strSubject = strSubject;
	m_strSender = strSender;
	m_dtDate = dtDate;
	m_nLocation = nLocation;
	m_nMailBatchID = nMailBatchID;
	m_nInternalRefID = nInternalRefID;
	m_strInternalTblName = strInternalTblName;
	m_nMergedPacketID = nMergedPacketID;
	m_dtServiceDate = dtServiceDate;
	m_nCategoryID = nCategoryID;
	m_nChecksum = nChecksum;
	m_nPicID = nPicID;
	m_nLabStepID = nLabStepID;
	m_nEMNID = nEMNID;
	m_bIsPhoto = bIsPhoto;
	m_nTemplateID = nTemplateID;
	m_nPictureOrigID = nPictureOrigID;
	m_nCCDAtypeField = nCCDAtypeField;
	m_strNote = strNote;
}

/// <summary>
/// Fills a CMailSentEntry from a recordset's fields object.
/// </summary>
/// <param name="pMailSentFields">A recordset's fields object containing information about the MailSent record.</param>
/// <param name="mailSentEntry">The mail sent entry to fill.</param>
void CMovePatientLabsDlg::FillMailSentEntry(FieldsPtr pMailSentFields, CMailSentEntry &mailSentEntry)
{
	mailSentEntry.m_nMailID = AdoFldLong(pMailSentFields, "MailID");
	mailSentEntry.m_nPersonID = AdoFldLong(pMailSentFields, "PersonID");
	mailSentEntry.m_strSelection = AdoFldString(pMailSentFields, "Selection");
	mailSentEntry.m_strPathName = AdoFldString(pMailSentFields, "PathName");
	mailSentEntry.m_strSubject = AdoFldString(pMailSentFields, "Subject");
	mailSentEntry.m_strSender = AdoFldString(pMailSentFields, "Sender");
	mailSentEntry.m_dtDate = AdoFldDateTime(pMailSentFields, "Date", g_cdtNull);
	mailSentEntry.m_nLocation = AdoFldLong(pMailSentFields, "Location", -1);
	mailSentEntry.m_nMailBatchID = AdoFldLong(pMailSentFields, "MailBatchID");
	mailSentEntry.m_nInternalRefID = AdoFldLong(pMailSentFields, "InternalRefID", -1);
	mailSentEntry.m_strInternalTblName = AdoFldString(pMailSentFields, "InternalTblName", "");
	mailSentEntry.m_nMergedPacketID = AdoFldLong(pMailSentFields, "MergedPacketID", -1);
	mailSentEntry.m_dtServiceDate = AdoFldDateTime(pMailSentFields, "ServiceDate", g_cdtNull);
	mailSentEntry.m_nCategoryID = AdoFldLong(pMailSentFields, "CategoryID", -1);
	mailSentEntry.m_nChecksum = AdoFldLong(pMailSentFields, "Checksum", -1);
	mailSentEntry.m_nPicID = AdoFldLong(pMailSentFields, "PicID", -1);
	mailSentEntry.m_nLabStepID = AdoFldLong(pMailSentFields, "LabStepID", -1);
	mailSentEntry.m_nEMNID = AdoFldLong(pMailSentFields, "EMNID", -1);
	mailSentEntry.m_bIsPhoto = AdoFldBool(pMailSentFields, "IsPhoto", FALSE);
	mailSentEntry.m_nTemplateID = AdoFldLong(pMailSentFields, "TemplateID", -1);
	mailSentEntry.m_nPictureOrigID = AdoFldLong(pMailSentFields, "PictureOrigID", -1);
	mailSentEntry.m_nCCDAtypeField = AdoFldLong(pMailSentFields, "CCDAtypeField", -1);
	mailSentEntry.m_strNote = AdoFldString(pMailSentFields, "Note", "");
}

/// <summary>
/// Copy constructor for the CMailSentEntry class.
/// </summary>
/// <param name="other">The other CMailSentEntry object to copy from.</param>
CMovePatientLabsDlg::CMailSentEntry::CMailSentEntry(CMailSentEntry &other)
{
	m_nMailID = other.m_nMailID;
	m_nPersonID = other.m_nPersonID;
	m_strSelection = other.m_strSelection;
	m_strPathName = other.m_strPathName;
	m_strSubject = other.m_strSubject;
	m_strSender = other.m_strSender;
	m_dtDate = other.m_dtDate;
	m_nLocation = other.m_nLocation;
	m_nMailBatchID = other.m_nMailBatchID;
	m_nInternalRefID = other.m_nInternalRefID;
	m_strInternalTblName = other.m_strInternalTblName;
	m_nMergedPacketID = other.m_nMergedPacketID;
	m_dtServiceDate = other.m_dtServiceDate;
	m_nCategoryID = other.m_nCategoryID;
	m_nChecksum = other.m_nChecksum;
	m_nPicID = other.m_nPicID;
	m_nLabStepID = other.m_nLabStepID;
	m_nEMNID = other.m_nEMNID;
	m_bIsPhoto = other.m_bIsPhoto;
	m_nTemplateID = other.m_nTemplateID;
	m_nPictureOrigID = other.m_nPictureOrigID;
	m_nCCDAtypeField = other.m_nCCDAtypeField;
	m_strNote = other.m_strNote;
}

/// <summary>
/// Gets the file name and path of a mailsent entry.
/// </summary>
/// <param name="mailSentEntry">The mail sent entry.</param>
/// <param name="strFileNameFromList">The file name without the path of the mailsent entry.</param>
/// <param name="strPathFromList">The full path of the mailsent entry</param>
void CMovePatientLabsDlg::GetPathComponents(CMailSentEntry &mailSentEntry, CString &strFileName, CString &strPath)
{
	int loc = mailSentEntry.m_strPathName.ReverseFind('\\');
	//files w/o a full path
	if (loc == -1) {		//no backslash, must just be a file
		strFileName= mailSentEntry.m_strPathName;
	}
	//directories
	else if (mailSentEntry.m_strSelection == "BITMAP:FOLDER") {
		strFileName = "";
	}
	//everything else
	else {
		strFileName = mailSentEntry.m_strPathName.Right(mailSentEntry.m_strPathName.GetLength() - loc - 1);
	}
	strPath = mailSentEntry.m_strPathName;
}

/// <summary>
/// Determines whether a mailsent entry or the document it references is used elsewhere. Ignores the selected lab results.
/// </summary>
/// <param name="mailSentEntry">The mail sent entry to check.</param>
/// <returns>True if the mailsent entry is used elsewhere. False if it isn't.</returns>
bool CMovePatientLabsDlg::IsDocumentInUse(CMailSentEntry &mailSentEntry)
{
	long nFromPatientID = mailSentEntry.m_nPersonID;

	CString strFileNameFromList, strPathFromList;
	GetPathComponents(mailSentEntry, strFileNameFromList, strPathFromList);

	//check to see if this file is attached to an EMN or a lab that we are not moving
	if (mailSentEntry.m_nEMNID != -1) {
		// copy the document instead
		return true;
	}

	// the image might be used in an EMN still
	bool bFileNameIsPerPatient = true;
	if (strPathFromList.Find('\\') != -1 || strPathFromList.Find("MultiPatDoc") != -1) {
		//the file name is a full path that is not specific to this patient
		bFileNameIsPerPatient = false;
	}
	if ((bFileNameIsPerPatient && ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"INNER JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID "
		"WHERE EMRMasterT.PatientID = {INT} "
		"AND (EMRDetailsT.InkImagePathOverride = {STRING} OR EMRDetailsT.InkImagePathOverride = '\\' + {STRING})",
		nFromPatientID, strPathFromList, strPathFromList))
		||
		(!bFileNameIsPerPatient && ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"WHERE EMRDetailsT.InkImagePathOverride = {STRING}", strPathFromList))
		) {

		return true;
	}

	// lab steps can have documents attached
	if (mailSentEntry.m_nLabStepID != -1) {
		return true;
	}

	//now look for lab results, ignoring the ones the user selected.
	_RecordsetPtr rsCheck = CreateParamRecordset("SELECT ResultID FROM LabResultsT WHERE MailID = {INT} AND LabResultsT.ResultID NOT IN ({INTARRAY}); "
		, mailSentEntry.m_nMailID, m_arySelectedLabResultIDs);
	if (!rsCheck->eof) {
		return true;
	}

	return false;
}