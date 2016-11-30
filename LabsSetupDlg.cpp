// LabsSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LabsSetupDlg.h"
#include "GetNewIDName.h"
#include "AdministratorRc.h"
#include "EditComboBox.h"
#include "LabEditStepDlg.h"
#include "SignatureDlg.h"
#include "LabEditDiagnosisDlg.h"
#include "GlobalLabUtils.h"
#include "LabsToBeOrderedSetupDlg.h"
#include <GlobalLabUtils.h>
#include <PrintUtils.h>
#include "HL7Utils.h"
#include "LabsSelCustomFieldsDlg.h"
#include "LabBarcodeSetupDlg.h"
#include "LabProcedureGroupsSetupDlg.h"
#include "GlobalAuditUtils.h"
#include "AuditTrail.h"
#include "ImportLoincDlg.h"
#include "LabUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALIST2Lib;

/////////////////////////////////////////////////////////////////////////////
// CLabsSetupDlg dialog

// (m.hancock 2006-07-05 15:14) - PLID 21187 - Create an Administrator section to create 
//lab procedures and assign steps to procedures.
enum LabProceduresListColumns {
	lplLabProcedureID = 0,
	lplLabProcedureName,
	lplLabProcedureType, // (r.galicki 2008-10-16 12:14) - PLID 31552
};

// (m.hancock 2006-07-05 15:14) - PLID 21187 - Create an Administrator section to create 
//lab procedures and assign steps to procedures.
enum LabStepsListColumns {
	lsStepID = 0,
	lsLabProcedureID,
	lsStepOrder,
	lsName,
	lsCreateLadder,	//TES 1/5/2011 - PLID 37877
	lsCompletedByHL7, //TES 1/5/2011 - PLID 37877
};

// (r.galicki 2008-10-16 11:22) - PLID 31552 - Lab type list columns
enum LabTypeListColumns {
	ltlcType = 0,
	ltlcName,
};

CLabsSetupDlg::CLabsSetupDlg(CWnd* pParent)
	: CNxDialog(CLabsSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLabsSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLabsSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLabsSetupDlg)
	DDX_Control(pDX, IDC_RENAME_LAB_PROCEDURE, m_btnRenameLabProcedure);
	DDX_Control(pDX, IDC_DELETE_LAB_PROCEDURE, m_btnDeleteLabProcedure);
	DDX_Control(pDX, IDC_NEW_LAB_PROCEDURE, m_btnNewLabProcedure);
	DDX_Control(pDX, IDC_MODIFY_STEP, m_btnModifyStep);
	DDX_Control(pDX, IDC_DELETE_STEP, m_btnDeleteStep);
	DDX_Control(pDX, IDC_NEW_STEP, m_btnNewStep);
	DDX_Control(pDX, IDC_DOWN, m_btnDown);
	DDX_Control(pDX, IDC_UP, m_btnUp);
	DDX_Control(pDX, IDC_EDIT_LAB_DESC, m_btnEditLabDesc);
	DDX_Control(pDX, IDC_EDIT_CLINICAL_DIAGS, m_btnEditClinDiag);
	DDX_Control(pDX, IDC_LAB_PROCEDURES_STATIC, m_nxstaticLabProceduresStatic);
	DDX_Control(pDX, IDC_LAB_EDIT_SIGNATURE, m_btnEditSignature);
	DDX_Control(pDX, IDC_COPY_LAB_PROCEDURE, m_btnCreateCopy);
	DDX_Control(pDX, IDC_LAB_EDIT_LABEL_PRINT_SETTINGS, m_btnLabelPrintSetup);
	DDX_Control(pDX, IDC_SEND_SPECIMEN_LABEL, m_btnSendSpecimenLabel);
	DDX_Control(pDX, IDC_SEND_REQUEST_FORM, m_btnSendRequestForm);
	DDX_Control(pDX, IDC_BTN_CONFIG_LAB_TO_BE_ORDERED_LOC, m_btnConfigLabToBeOrderedLoc);
	DDX_Control(pDX, IDC_EDIT_CUSTOM_FIELDS, m_btnEditCustomFields);
	DDX_Control(pDX, IDC_LAB_BARCODE_SETUP_BTN, m_btnBarcodeSetup);
	DDX_Control(pDX, IDC_LAB_PROCEDURE_GROUP_SETUP_BTN, m_btnLabProcedureGroupsSetup);
	DDX_Control(pDX, IDC_IMPORT_LOINC, m_btnImportLOINC);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLabsSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CLabsSetupDlg)
	ON_BN_CLICKED(IDC_NEW_LAB_PROCEDURE, OnNewLabProcedure)
	ON_BN_CLICKED(IDC_DELETE_LAB_PROCEDURE, OnDeleteLabProcedure)
	ON_BN_CLICKED(IDC_RENAME_LAB_PROCEDURE, OnRenameLabProcedure)
	ON_BN_CLICKED(IDC_NEW_STEP, OnNewStep)
	ON_BN_CLICKED(IDC_DELETE_STEP, OnDeleteStep)
	ON_BN_CLICKED(IDC_MODIFY_STEP, OnModifyStep)
	ON_BN_CLICKED(IDC_UP, OnUp)
	ON_BN_CLICKED(IDC_DOWN, OnDown)
	ON_BN_CLICKED(IDC_EDIT_LAB_DESC, OnEditLabDesc)
	ON_BN_CLICKED(IDC_EDIT_CLINICAL_DIAGS, OnEditClinicalDiags)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_LAB_EDIT_SIGNATURE, &CLabsSetupDlg::OnBnClickedLabEditSignature)
	ON_BN_CLICKED(IDC_COPY_LAB_PROCEDURE, &CLabsSetupDlg::OnBnClickedCopyLabProcedure)
	ON_BN_CLICKED(IDC_LAB_EDIT_LABEL_PRINT_SETTINGS, &CLabsSetupDlg::OnBnClickedLabEditLabelPrintSettings)
	ON_BN_CLICKED(IDC_SEND_SPECIMEN_LABEL, &CLabsSetupDlg::OnBnClickedSendSpecimenLabel)
	ON_BN_CLICKED(IDC_SEND_REQUEST_FORM, &CLabsSetupDlg::OnBnClickedSendRequestForm)
	ON_BN_CLICKED(IDC_BTN_CONFIG_LAB_TO_BE_ORDERED_LOC, OnBtnConfigLabToBeOrderedLoc)
	ON_BN_CLICKED(IDC_EDIT_CUSTOM_FIELDS, &CLabsSetupDlg::OnBnClickedEditCustomFields)
	ON_BN_CLICKED(IDC_LAB_BARCODE_SETUP_BTN, &CLabsSetupDlg::OnBnClickedLabBarcodeSetupBtn)
	ON_BN_CLICKED(IDC_LAB_PROCEDURE_GROUP_SETUP_BTN, &CLabsSetupDlg::OnBnClickedLabProcedureGroupSetupBtn)
	ON_BN_CLICKED(IDC_IMPORT_LOINC, &CLabsSetupDlg::OnBnClickedImportLoinc)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLabsSetupDlg message handlers

BOOL CLabsSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		// (m.hancock 2006-07-05 15:01) - PLID 21187 - Create an Administrator section to create 
		//lab procedures and assign steps to procedures.
		
		//Set the icons on the NxIconButtons
		m_btnNewLabProcedure.AutoSet(NXB_NEW);
		m_btnDeleteLabProcedure.AutoSet(NXB_DELETE);
		m_btnRenameLabProcedure.AutoSet(NXB_MODIFY);
		m_btnNewStep.AutoSet(NXB_NEW);
		m_btnDeleteStep.AutoSet(NXB_DELETE);
		m_btnModifyStep.AutoSet(NXB_MODIFY);
		m_btnUp.AutoSet(NXB_UP);
		m_btnDown.AutoSet(NXB_DOWN);
		// (z.manning, 05/14/2008) - PLID 29566
		m_btnEditLabDesc.AutoSet(NXB_MODIFY);
		m_btnEditClinDiag.AutoSet(NXB_MODIFY);
		m_btnEditSignature.AutoSet(NXB_MODIFY);
		m_btnCreateCopy.AutoSet(NXB_NEW); // (z.manning 2010-05-07 10:30) - PLID 35537
		m_btnLabelPrintSetup.AutoSet(NXB_MODIFY); // (z.manning 2010-05-13 16:49) - PLID 37876
		// (r.gonet 09/21/2011) - PLID 45584
		m_btnEditCustomFields.AutoSet(NXB_MODIFY);
		// (r.gonet 03/29/2012) - PLID 49299
		m_btnLabProcedureGroupsSetup.AutoSet(NXB_MODIFY);

		// (j.jones 2010-06-25 15:52) - PLID 39185 - added ability to link locations and 'to be ordered' entries
		m_btnConfigLabToBeOrderedLoc.AutoSet(NXB_MODIFY);
		// (r.gonet 10/11/2011) - PLID 45924
		m_btnBarcodeSetup.AutoSet(NXB_MODIFY);
		// (s.tullis 2015-11-13 16:41) - PLID 54285 - Only show LOINCT import if this is Tech Support
		m_btnImportLOINC.AutoSet(NXB_MODIFY);
		if (GetCurrentUserID() != -26)
		{
			m_btnImportLOINC.ShowWindow(SW_HIDE);
		}

		//(e.lally 2007-04-20) PLID 25738 - Only notified the user once when a change to a lab procedure is made (per time loaded).
		//We have not told them anything yet.
		m_bUserNotifiedOfChange = FALSE;

		//Set up the datalists
		// (a.walling 2007-11-14 14:19) - PLID 28059 - VS2008 - Bad binds; should be BindNxDataList2Ctrl.
		m_LabProcedureList = BindNxDataList2Ctrl(IDC_LAB_PROCEDURES_LIST, true);
		m_StepsList = BindNxDataList2Ctrl(IDC_STEPS_LIST, false);

		//Select the first lab procedure
		m_LabProcedureList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		m_LabProcedureList->PutCurSel(m_LabProcedureList->GetFirstRow());
		OnSelChosenLabProceduresList(m_LabProcedureList->GetFirstRow());

		// (r.galicki 2008-10-16 11:16) - PLID 31552 - Build 'Lab Type' dropdown
		m_LabTypeList = BindNxDataList2Ctrl(IDC_NXDL_LABTYPE, false);
		IRowSettingsPtr pRow = m_LabTypeList->GetNewRow();
		pRow->PutValue(ltlcType, _variant_t((BYTE)ltBiopsy));
		pRow->PutValue(ltlcName, "Biopsy");
		m_LabTypeList->AddRowAtEnd(pRow, NULL);
		pRow = m_LabTypeList->GetNewRow();
		pRow->PutValue(ltlcType, _variant_t((BYTE)ltLabWork));
		pRow->PutValue(ltlcName, "Lab Work");
		m_LabTypeList->AddRowAtEnd(pRow, NULL);
		pRow = m_LabTypeList->GetNewRow();
		pRow->PutValue(ltlcType, _variant_t((BYTE)ltCultures));
		pRow->PutValue(ltlcName, "Cultures");
		m_LabTypeList->AddRowAtEnd(pRow, NULL);
		pRow = m_LabTypeList->GetNewRow();
		pRow->PutValue(ltlcType, _variant_t((BYTE)ltDiagnostics));
		pRow->PutValue(ltlcName, "Diagnostics");
		m_LabTypeList->AddRowAtEnd(pRow, NULL);

		//set list to type value of selected procedure
		pRow = m_LabProcedureList->CurSel;
		if(pRow) {
			BYTE nType = VarByte(pRow->GetValue(lplLabProcedureType));
			m_LabTypeList->SetSelByColumn(ltlcType, nType);
		}


	} NxCatchAll("Error in CLabsSetupDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CLabsSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CLabsSetupDlg)
	ON_EVENT(CLabsSetupDlg, IDC_LAB_PROCEDURES_LIST, 16 /* SelChosen */, OnSelChosenLabProceduresList, VTS_DISPATCH)
	ON_EVENT(CLabsSetupDlg, IDC_STEPS_LIST, 10 /* EditingFinished */, OnEditingFinishedStepsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CLabsSetupDlg, IDC_STEPS_LIST, 18 /* RequeryFinished */, OnRequeryFinishedStepsList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CLabsSetupDlg, IDC_STEPS_LIST, 3, CLabsSetupDlg::OnDblClickCellStepsList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CLabsSetupDlg, IDC_NXDL_LABTYPE, 16, CLabsSetupDlg::SelChosenNxdlLabtype, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CLabsSetupDlg::OnSelChosenLabProceduresList(LPDISPATCH lpRow) 
{
	try {
		
		// (m.hancock 2006-07-05 15:01) - PLID 21187 - Create an Administrator section to create 
		//lab procedures and assign steps to procedures.
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if(pRow != NULL) {

			RequeryLabSteps();

			// (r.galicki 2008-10-17 11:24) - PLID 31552 - Update type drop down
			BYTE type = VarByte(pRow->GetValue(lplLabProcedureType));
			if(m_LabTypeList) {
				m_LabTypeList->SetSelByColumn(ltlcType, type);
			}

			// (d.thompson 2010-06-14) - PLID 36791 - Load options for Send Specimen Label and Send Request Form
			long nLabID = VarLong(pRow->GetValue(lplLabProcedureID));
			_RecordsetPtr prs = CreateParamRecordset("SELECT SendSpecimenLabel, SendRequestForm FROM LabProceduresT WHERE ID = {INT};", nLabID);
			CheckDlgButton(IDC_SEND_SPECIMEN_LABEL, AdoFldBool(prs, "SendSpecimenLabel"));
			CheckDlgButton(IDC_SEND_REQUEST_FORM, AdoFldBool(prs, "SendRequestForm"));

			//(e.lally 2007-04-20) PLID 25738 - Only notified the user once when a change to a lab procedure is made (per time loaded).
			//Reset our flag to notify them again for the selected lab procedure (even if they already have been notified during this session).
			m_bUserNotifiedOfChange = FALSE;
		}
		else {
			// (d.thompson 2010-06-14) - PLID 36791 - Make sure the 'send' options are unchecked
			CheckDlgButton(IDC_SEND_SPECIMEN_LABEL, FALSE);
			CheckDlgButton(IDC_SEND_REQUEST_FORM, FALSE);
		}
	} NxCatchAll("Error in CLabsSetupDlg::OnSelChosenLabProceduresList");
}

void CLabsSetupDlg::OnNewLabProcedure() 
{
	try {

		// (m.hancock 2006-07-05 16:06) - PLID 21187 - Create a new lab procedure
		CGetNewIDName dlg(this);
		CString strName;
		dlg.m_pNewName = &strName;
		dlg.m_nMaxLength = 50;

		if (dlg.DoModal() == IDOK) 
		{
			strName.TrimRight();

			//Check if the lab procedure already exists
			if(!IsRecordsetEmpty("SELECT ID FROM LabProceduresT WHERE Name = '%s'",_Q(strName))) {
				MessageBox("There is already a lab procedure with this name or one may have previously existed. Please choose a new name.","Practice",MB_OK|MB_ICONEXCLAMATION);
				return;
			}

			//Check if the lab procedure is blank
			if(strName == "") {
				MessageBox("You cannot enter a blank name for a ladder.");
				return;
			}

			//Add the lab procedure
			// (d.thompson 2010-06-14) - PLID 36791 - Defaults for sending are to 'On'
			// (r.goldschmidt 2014-08-20 15:46) - PLID 53701 - get ID for auditing purposes; set default for type to 1
			_RecordsetPtr prsNewLab = CreateParamRecordset(GetRemoteData(),
				"SET NOCOUNT ON \r\n"
				"INSERT INTO LabProceduresT(Inactive, Name, Type, SendSpecimenLabel, SendRequestForm) VALUES (0, {STRING}, 1, 1, 1) \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT ID, Type FROM LabProceduresT WHERE ID = (SELECT CONVERT(int, SCOPE_IDENTITY())) \r\n",
				strName);

			// (r.goldschmidt 2014-08-18 15:36) - PLID 53701 - need auditing for Adminstrator>Labs tab, new lab procedure
			long nRecordID = AdoFldLong(prsNewLab, "ID");
			AuditEvent(-1, "", BeginNewAuditEvent(), aeiAdminLabProcedureCreated, nRecordID, "", strName, aepMedium, aetCreated);

			//Refresh the list of lab procedures.  We cannot simply add the row because we need the ID for the lab procedure,
			//so if we retrieve the ID in the future, we can just add a new row to the list here instead of requery.
			// (r.goldschmidt 2014-08-20 15:46) - PLID 53701 - have the ID now, so we can build the row without a requery
			IRowSettingsPtr pNewRow = m_LabProcedureList->GetNewRow();
			pNewRow->PutValue(lplLabProcedureID, prsNewLab->GetFields()->GetItem("ID")->GetValue());
			pNewRow->PutValue(lplLabProcedureName, _bstr_t(strName));
			pNewRow->PutValue(lplLabProcedureType, prsNewLab->GetFields()->GetItem("Type")->GetValue());
			m_LabProcedureList->PutCurSel(m_LabProcedureList->AddRowSorted(pNewRow, NULL));
			
			//Clear the step list
			m_StepsList->Clear();

			// (d.thompson 2010-06-14) - PLID 36791 - By default, the 2 'send' options are always enabled on new lab procedures
			CheckDlgButton(IDC_SEND_SPECIMEN_LABEL, TRUE);
			CheckDlgButton(IDC_SEND_REQUEST_FORM, TRUE);

			//(e.lally 2007-04-26) PLID 25738 - Reset our flag for new lab procedures.
			m_bUserNotifiedOfChange = FALSE;

			// (r.galicki 2008-11-06 17:06) - PLID 31552 - Default lab type dropdown to default in data
			IRowSettingsPtr pRow = m_LabProcedureList->CurSel;
			if(pRow) {
				BYTE nType = VarByte(pRow->GetValue(lplLabProcedureType));
				m_LabTypeList->SetSelByColumn(ltlcType, nType);
			}
		}
	} NxCatchAll("Error in CLabsSetupDlg::OnNewLabProcedure: Could not add lab procedure");
}

void CLabsSetupDlg::OnDeleteLabProcedure() 
{
	try {

		// (m.hancock 2006-07-05 16:06) - PLID 21187 - "Delete" a lab procedure by marking it inactive.
		// We need to mark inactive because existing lab records will refer back to the LabProcedures table,
		// and they can't do that if the lab procedure is deleted, can they?

		//Get the currently selected row
		NXDATALIST2Lib::IRowSettingsPtr pRow(m_LabProcedureList->CurSel);

		//Make sure we actually have a row selected
		if(pRow != NULL) {

			//Get the currently selected lab procedure
			long nID = VarLong(pRow->GetValue(lplLabProcedureID));
			CString strName = VarString(pRow->GetValue(lplLabProcedureName));

			// (z.manning 2010-01-08 14:45) - PLID 36566 - See if this lab procedure is used in HL7
			//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 Settings
			CArray<long,long> arHL7GroupIDs;
			GetHL7SettingsGroupsBySetting("DefaultLabProcedureID", nID, arHL7GroupIDs);
			if(arHL7GroupIDs.GetSize() > 0) {
				int nResult = MessageBox("This lab procedure is used in one or more HL7 settings groups. "
					"Are you sure you want to delete it?", NULL, MB_YESNO|MB_ICONQUESTION);
				if(nResult != IDYES) {
					return;
				}
			}

			//Prompt the user
			if(IDYES == MessageBox("Are you sure you wish to delete this lab procedure?\nThis change will not affect existing lab records.", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				//(e.lally 2009-05-11) PLID 28553 - Check if it appears on any order set templates and give one last chance to cancel
				_RecordsetPtr rsOST = CreateParamRecordset(GetRemoteData(), "SELECT COUNT(Distinct OrderSetTemplateID) as TemplateCount "
					"FROM OrderSetTemplateLabsT WHERE LabProcedureID = {INT} ", nID);
				if(!rsOST->eof){
					long nOSTemplateCount = AdoFldLong(rsOST, "TemplateCount", 0);
					if(nOSTemplateCount > 0){
						CString strMessage;
						strMessage.Format("This lab procedure will be removed from %li Order Set Template(s).\n"
							"Are you sure you wish to continue?", nOSTemplateCount);
						if(IDYES != MessageBox(strMessage, "Practice", MB_ICONQUESTION|MB_YESNO)){
							return;
						}
					}
				}

				//Mark the lab procedure as inactive
				// (z.manning 2008-10-01 16:02) - PLID 31556 - Delete any actions associated with this lab ladder
				// (z.manning 2010-01-08 14:56) - PLID 36566 - Handle HL7SettingsT
				//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
				CArray<long,long> arHL7GroupsToUpdate;
				GetHL7SettingsGroupsBySetting("DefaultLabProcedureID", nID, arHL7GroupsToUpdate);
				for(int i = 0; i < arHL7GroupsToUpdate.GetSize(); i++) {
					SetHL7SettingInt(arHL7GroupsToUpdate[i], "DefaultLabProcedureID", -1);
				}
				ExecuteParamSql(
					//(e.lally 2009-05-08) PLID 28553 - Delete this lab procedure from any and all order set templates.
					//TES 7/22/2011 - PLID 42492 - Also deactivate the steps.
					"DELETE FROM OrderSetTemplateLabsT WHERE LabProcedureID = {INT}; "
					"UPDATE EmrActionsT SET Deleted = 1 WHERE (SourceType = {INT} AND SourceID = {INT}) "
					"	OR (DestType = {INT} AND DestID = {INT}); "
					"UPDATE LabProceduresT SET Inactive = 1 WHERE ID = {INT}; "
					"UPDATE LabProcedureStepsT SET Inactive = 1 WHERE LabProcedureID = {INT}; "
					, nID, eaoLab, nID, eaoLab, nID, nID, nID);

				// (r.goldschmidt 2014-08-18 15:36) - PLID 53701 - need auditing for Adminstrator>Labs tab.
				AuditEvent(-1, "", BeginNewAuditEvent(), aeiAdminLabProcedureDeleted, nID, strName,"", aepMedium, aetDeleted);

				//Remove the row from the lab procedure list
				m_LabProcedureList->RemoveRow(pRow);

				//Set the selection to the first lab procedure and display its steps
				m_LabProcedureList->PutCurSel(m_LabProcedureList->GetFirstRow());
				OnSelChosenLabProceduresList(m_LabProcedureList->GetFirstRow());
			}
		}
		else
			AfxMessageBox("Please select a lab procedure before trying to delete a lab procedure.");
	} NxCatchAll("Error in CLabsSetupDlg::OnDeleteLabProcedure");
}

void CLabsSetupDlg::OnRenameLabProcedure() 
{
	try {

		// (m.hancock 2006-07-05 16:06) - PLID 21187 - Rename an existing lab procedure

		//Get the currently selected row
		NXDATALIST2Lib::IRowSettingsPtr pRow(m_LabProcedureList->CurSel);

		//Make sure we actually have a row selected
		if(pRow != NULL) {

			//Get the currently selected lab procedure
			long nID = VarLong(pRow->GetValue(lplLabProcedureID));
			CString strOldName = VarString(pRow->GetValue(lplLabProcedureName));

			//Ask the user for the new name
			CGetNewIDName dlg(this);
			CString strName;
			dlg.m_pNewName = &strName;
			dlg.m_nMaxLength = 50;

			if (dlg.DoModal() == IDOK) 
			{
				strName.TrimRight();

				//Check if the lab procedure already exists
				if(!IsRecordsetEmpty("SELECT ID FROM LabProceduresT WHERE Name = '%s'",_Q(strName))) {
					MessageBox("There is already a lab procedure with this name. Please choose a new name.","Practice",MB_OK|MB_ICONEXCLAMATION);
					return;
				}

				//Check if the lab procedure is blank
				if(strName == "") {
					MessageBox("You cannot enter a blank name for a ladder.");
					return;
				}

				//Update the name on the lab procedure
				ExecuteSql("UPDATE LabProceduresT SET Name = '%s' WHERE ID = %li", _Q(strName), nID);

				// (r.goldschmidt 2014-08-18 15:36) - PLID 53701 - need auditing for Adminstrator>Labs tab.
				AuditEvent(-1, "", BeginNewAuditEvent(), aeiAdminLabProcedureRenamed, nID, strOldName, strName, aepMedium);

				//Update the row in the lab procedure list
				pRow->PutValue(lplLabProcedureName, _bstr_t(strName));

				//Resort the lab procedure list
				m_LabProcedureList->Sort();
			}
		}
		else
			AfxMessageBox("Please select a lab procedure before trying to rename a lab procedure.");
	} NxCatchAll("Error in CLabsSetupDlg::OnRenameLabProcedure");
}

void CLabsSetupDlg::OnEditingFinishedStepsList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		//Set the selection to the row that was clicked
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		switch(nCol) {
		
		case lsName: 
			{
				// (m.hancock 2006-07-05 17:19) - PLID 21187 - Edit the name of the step.  In the future we should
				// support editing other values relating to the steps, such as actions.

				//If the new value is "New step", then the user just added the step and chose not to change its name.
				//In that case, display warning that this will not take effect on existing lab records
				if(VarString(varNewValue) == "New step")
					AfxMessageBox("Existing lab records associated with patients will not be updated by this change.\nThe change will only take effect in future labs.");

				//Check for bCommit because the user may have pressed Escape and we don't want to do this if they did
				if((bCommit == TRUE) && (VarString(varOldValue) != VarString(varNewValue))) {

					//Get the data required for the update
					long nStepID = VarLong(pRow->GetValue(lsStepID));
					long nStepOrder = VarLong(pRow->GetValue(lsStepOrder));
					long nLabProcedureID = VarLong(pRow->GetValue(lsLabProcedureID));
					CString strNewName = VarString(varNewValue, "");

					//Get Procedure Name
					CString strLabProcedureName;
					IRowSettingsPtr pProcedureRow(m_LabProcedureList->CurSel);
					if (pProcedureRow){
						strLabProcedureName = VarString(pProcedureRow->GetValue(lplLabProcedureName));
					}

					//Prevent applying step names that are empty
					if(strNewName.IsEmpty()) {
						MessageBox("You cannot enter a blank name for a step.");
						bCommit = FALSE;
						pRow->PutValue(lsName, _bstr_t(VarString(varOldValue)));
						return;
					}

					//Update the LabProcedureStepsT record
					ExecuteSql("UPDATE LabProcedureStepsT SET Name = '%s' WHERE StepID = %li AND LabProcedureID = %li",
						_Q(strNewName), nStepID, nLabProcedureID);

					// (r.goldschmidt 2014-08-19 09:19) - PLID 53701 - need auditing for Adminstrator>Labs tab, lab procedure step renamed
					CString strOldValue, strNewValue;
					strOldValue.Format("%s: (Step %li) %s", strLabProcedureName, nStepOrder, VarString(varOldValue));
					strNewValue.Format("%s: (Step %li) %s", strLabProcedureName, nStepOrder, strNewName);
					AuditEvent(-1, "", BeginNewAuditEvent(), aeiAdminLabProcedureStepRenamed, nStepID, strOldValue, strNewValue, aepMedium, aetChanged);

					//Display warning that this will not take effect on existing lab records
					//(e.lally 2007-04-20) PLID 25738 - Only notified the user once when a change to a lab procedure is made (per time loaded).
					if(!m_bUserNotifiedOfChange){
						m_bUserNotifiedOfChange = TRUE;
						AfxMessageBox("Existing lab records associated with patients will not be updated by any changes to the setup.\nThe changes will only take effect in future labs.");
					}
				}
			}
		}
	} NxCatchAll("Error in CLabsSetupDlg::OnEditingFinishedStepsList");
}

void CLabsSetupDlg::OnNewStep() 
{
	try {

		// (m.hancock 2006-07-05 17:48) - PLID 21187 - For now, insert a new step and allow the user to edit its name.
		// In the future we should support actions for steps, similar to how tracking steps can be setup.

		//Get the currently selected lab procedure
		NXDATALIST2Lib::IRowSettingsPtr pRow(m_LabProcedureList->CurSel);

		//Make sure we actually have a row selected
		if(pRow != NULL) {

			//Get the lab procedure ID and name
			long nLabProcedureID = VarLong(pRow->GetValue(lplLabProcedureID));
			CString strLabProcedureName = VarString(pRow->GetValue(lplLabProcedureName));

			// (z.manning 2008-10-10 12:49) - PLID 21108 - We now have a dialog to create/edit steps
			CLabEditStepDlg dlg(nLabProcedureID, strLabProcedureName, this);
			//TES 1/5/2011 - PLID 37877 - Need to pass in whether the previous row (which would be the last row in this ladder) is flagged
			// to be completed by HL7.
			IRowSettingsPtr pRow = m_StepsList->GetLastRow();
			if(pRow != NULL) {
				if(AsBool(pRow->GetValue(lsCompletedByHL7))) {
					dlg.SetPrevStepIsCompletedByHL7();
				}
			}
			if(dlg.DoModal() == IDOK) {
				RequeryLabSteps();
			}
		}
	} NxCatchAll("Error in CLabsSetupDlg::OnNewStep");
}

void CLabsSetupDlg::OnDeleteStep() 
{
	try {
		// (m.hancock 2006-07-06 15:11) - PLID 21187 - We don't want to actually delete the step, so just set Inactive = 0,
		// that way existing steps stay intact.

		//Get the currently selected lab procedure
		NXDATALIST2Lib::IRowSettingsPtr pRow(m_StepsList->CurSel);

		//Make sure we actually have a row selected
		if(pRow != NULL) {

			//Prompt the user
			if(IDYES == MessageBox("Are you sure you wish to delete this step?\nExisting lab procedures will keep this step active, but it will not be included in future lab procedures.", "Practice", MB_ICONQUESTION|MB_YESNO)) {

				//TES 1/5/2011 - PLID 37877 - See whether this will result in a CompletedByHL7-CreateLadder combo, and if so, warn the user.
				IRowSettingsPtr pPrevRow = pRow->GetPreviousRow();
				IRowSettingsPtr pNextRow = pRow->GetNextRow();
				if(pPrevRow != NULL && pNextRow != NULL) {
					if(AsBool(pPrevRow->GetValue(lsCompletedByHL7)) && AsBool(pNextRow->GetValue(lsCreateLadder))) {
						CString strPrevName = VarString(pPrevRow->GetValue(lsName),"");
						CString strNextName = VarString(pNextRow->GetValue(lsName),"");
						if(IDYES != MsgBox(MB_YESNO, "This action will result in a step (%s) that is flagged to be automatically completed "
							"when HL7 results are received, immediately followed by a step (%s) that is flagged to prompt to create a ladder "
							"when the step is activated.  Because it is not possible to prompt to create a ladder during the HL7 import "
							"process, you will NOT be prompted to create a ladder if the first step (%s) is automatically completed.\r\n\r\n"
							"Are you sure you wish to continue?", strPrevName, strNextName, strPrevName)) {
								return;
						}
					}
				}

				//Get the step ID and name
				long nStepID = VarLong(pRow->GetValue(lsStepID));
				CString strStepName = VarString(pRow->GetValue(lsName));

				//Get the procedure ID
				long nLabProcedureID = VarLong(pRow->GetValue(lsLabProcedureID));

				//Get the procedure Name
				CString strProcedureName;
				IRowSettingsPtr pProcedureRow(m_LabProcedureList->CurSel);
				if (pProcedureRow){
					strProcedureName = VarString(pProcedureRow->GetValue(lplLabProcedureName));
				}

				//Get the current step order
				long nStepOrder = VarLong(pRow->GetValue(lsStepOrder));

				//Mark the step inactive
				//(e.lally 2007-04-20) PLID 25720 - Update the rest of the step orders to that the maximum step order
					//value is the same as the count of the number of active steps.
				ExecuteSql(
					"UPDATE LabProcedureStepsT SET Inactive = 1 WHERE StepID = %li; \r\n"
					"UPDATE LabProcedureStepsT SET StepOrder = StepOrder - 1 "
					"	WHERE LabProcedureID = %li AND Inactive = 0 AND StepOrder > %li", nStepID, nLabProcedureID, nStepOrder);

				// (r.goldschmidt 2014-08-18 15:36) - PLID 53701 - need auditing for Adminstrator>Labs tab, delete step.
				CString strAuditValue;
				strAuditValue.Format("%s: (Step %li) %s", strProcedureName, nStepOrder, strStepName);
				CAuditTransaction audit;
				AuditEvent(-1, "", audit, aeiAdminLabProcedureStepDeleted, nStepID, strAuditValue, "", aepMedium, aetDeleted);

				IRowSettingsPtr pCurRow = pRow->GetNextRow();

				//save the step order from the row we are removing
				long nInactiveStepOrder = VarLong(pRow->GetValue(lsStepOrder));

				//Remove the row from the list
				m_StepsList->RemoveRow(pRow);

				long nTempStepOrder = -1;
				//(e.lally 2007-04-20) PLID 25720 - Update the step order value for all the steps after the deleted one
				while(pCurRow)
				{
					nTempStepOrder = VarLong(pCurRow->GetValue(lsStepOrder));
					//Make sure we are not decrimenting if the step order was duplicated from bad data.
					if (nTempStepOrder != nInactiveStepOrder){
						nTempStepOrder--;
						pCurRow->PutValue(lsStepOrder, (long)nTempStepOrder);
						// (r.goldschmidt 2014-09-30 16:31) - PLID 53701 - audit step order change
						long nShiftedStepID = VarLong(pCurRow->GetValue(lsStepID));
						CString strStepName = VarString(pCurRow->GetValue(lsName));
						CString strOldValue, strNewValue;
						strOldValue.Format("%s: (Step %li) %s", strProcedureName, nTempStepOrder + 1, strStepName);
						strNewValue.Format("%s: (Step %li) %s", strProcedureName, nTempStepOrder, strStepName);
						AuditEvent(-1, "", audit, aeiAdminLabProcedureStepOrderChanged, nShiftedStepID, strOldValue, strNewValue, aepMedium);
					}

					pCurRow = pCurRow->GetNextRow();
				}
				audit.Commit();
			}			
		}
		else
			AfxMessageBox("Please select a step before trying to delete a step.");
	} NxCatchAll("Error in CLabsSetupDlg::OnDeleteStep");
}

void CLabsSetupDlg::OnModifyStep() 
{
	try {

		// (m.hancock 2006-07-05 17:37) - PLID 21187 - For now, we're just allowing editing the name of the step.
		// In the future we should support editing other values relating to the steps, such as actions.

		//Get the currently selected lab procedure
		NXDATALIST2Lib::IRowSettingsPtr pLabProcedureRow(m_LabProcedureList->CurSel);

		// this should always pass
		if (pLabProcedureRow != NULL)
		{
			// (z.manning 2008-10-10 10:59) - PLID 21108 - We now have a dialog to edit steps
			long nLabProcedureID = VarLong(pLabProcedureRow->GetValue(lplLabProcedureID));
			CString strLabProcedureName = VarString(pLabProcedureRow->GetValue(lplLabProcedureName));

			//Get the currently selected row from the step list
			NXDATALIST2Lib::IRowSettingsPtr pRow(m_StepsList->CurSel);

			// Make sure we actually have a row selected
			if (pRow != NULL)
			{
				// (r.goldschmidt 2014-08-20 12:16) - PLID 53701 - Prep for auditing
				CLabEditStepDlg dlg(nLabProcedureID, strLabProcedureName, this);
				dlg.SetStepID(VarLong(pRow->GetValue(lsStepID)));
				//TES 1/5/2011 - PLID 37877 - Need to pass in whether the previous step is completed by HL7, or the next step prompts to create a ladder.
				IRowSettingsPtr pPrevRow = pRow->GetPreviousRow();
				if (pPrevRow != NULL) {
					if (AsBool(pPrevRow->GetValue(lsCompletedByHL7))) {
						dlg.SetPrevStepIsCompletedByHL7();
					}
				}
				IRowSettingsPtr pNextRow = pRow->GetNextRow();
				if (pNextRow != NULL) {
					if (AsBool(pNextRow->GetValue(lsCreateLadder))) {
						dlg.SetNextStepIsCreateLadder();
					}
				}
				if (dlg.DoModal() == IDOK) {
					pRow->PutValue(lsName, _bstr_t(dlg.GetStepName()));
					//TES 1/5/2011 - PLID 37877 - These values may also have changed.
					pRow->PutValue(lsCreateLadder, dlg.GetCreateLadder() ? g_cvarTrue : g_cvarFalse);
					pRow->PutValue(lsCompletedByHL7, dlg.GetCompletedByHL7() ? g_cvarTrue : g_cvarFalse);
				}
			}
			else
				AfxMessageBox("Please select a step before trying to modify a step.");
		}
	} NxCatchAll("Error in CLabsSetupDlg::OnModifyStep");
}

void CLabsSetupDlg::OnUp() 
{
	try {
		// (m.hancock 2006-07-06 16:29) - PLID 21187 - Move a step up in it's processing order.

		//Get the currently selected row
		NXDATALIST2Lib::IRowSettingsPtr pRow(m_StepsList->CurSel);

		//Make sure we actually have a row selected
		if (pRow != NULL) {

			// get procedure name for auditing later
			CString strProcedureName;
			IRowSettingsPtr pProcedureRow(m_LabProcedureList->CurSel);
			if (pProcedureRow){
				strProcedureName = VarString(pProcedureRow->GetValue(lplLabProcedureName));
			}

			//Get some information about the step
			long nStepID = VarLong(pRow->GetValue(lsStepID));
			long nCurOrder = VarLong(pRow->GetValue(lsStepOrder));
			long nCurLabProcedureID = VarLong(pRow->GetValue(lsLabProcedureID));
			CString strCurStepName = VarString(pRow->GetValue(lsName), "");
			_variant_t varCurCreateLadder = pRow->GetValue(lsCreateLadder);
			_variant_t varCurCompletedByHL7 = pRow->GetValue(lsCompletedByHL7);
			
			//If this is the top record, we cannot go higher
			if(nCurOrder <= 1)
				return;

			//Get the previous row
			NXDATALIST2Lib::IRowSettingsPtr pPrevRow = pRow->GetPreviousRow();
			if(pPrevRow != NULL) {
				
				//Get some information about the previous step
				long nPrevStepID = VarLong(pPrevRow->GetValue(lsStepID));
				long nPrevStepOrder = VarLong(pPrevRow->GetValue(lsStepOrder));
				long nPrevLabProcedureID = VarLong(pPrevRow->GetValue(lsLabProcedureID));
				CString strPrevStepName = VarString(pPrevRow->GetValue(lsName), "");
				_variant_t varPrevCreateLadder = pPrevRow->GetValue(lsCreateLadder);
				_variant_t varPrevCompletedByHL7 = pPrevRow->GetValue(lsCompletedByHL7);

				//(e.lally 2007-04-20) PLID 25720 - Since there was a bug that caused duplicate step orders
					//to occur in the data, we have to fix them as we come across them.
				if(nCurOrder == nPrevStepOrder){
					//Darn, we have bad data. Fix it!
					//Send in the previous StepOrder and StepID as we want that incremented
					FixDuplicateStepOrdering(nPrevLabProcedureID, nPrevStepOrder, nPrevStepID);
					//All the values for this procedure should be fixed now, let's just requery the list manually to get the new ordering
					RequeryLabSteps();
				}
				else {
					//TES 1/5/2011 - PLID 37877 - If we're going to be in a state where we have a Completed By HL7 step followed by
					// a Create Ladder step, warn the user.
					if(AsBool(varPrevCreateLadder) && AsBool(varCurCompletedByHL7)) {
						if(IDYES != MsgBox(MB_YESNO, "This action will result in a step (%s) that is flagged to be automatically completed "
							"when HL7 results are received, immediately followed by a step (%s) that is flagged to prompt to create a ladder "
							"when the step is activated.  Because it is not possible to prompt to create a ladder during the HL7 import "
							"process, you will NOT be prompted to create a ladder if the first step (%s) is automatically completed.\r\n\r\n"
							"Are you sure you wish to continue?", strCurStepName, strPrevStepName, strCurStepName)) {
								return;
						}
					}
					else if(AsBool(varCurCreateLadder)) {
						IRowSettingsPtr pPrevPrevRow = pPrevRow->GetPreviousRow();
						if(pPrevPrevRow != NULL) {
							if(AsBool(pPrevPrevRow->GetValue(lsCompletedByHL7))) {
								CString strPrevPrevStepName = VarString(pPrevPrevRow->GetValue(lsName),"");
								if(IDYES != MsgBox(MB_YESNO, "This action will result in a step (%s) that is flagged to be automatically completed "
									"when HL7 results are received, immediately followed by a step (%s) that is flagged to prompt to create a ladder "
									"when the step is activated.  Because it is not possible to prompt to create a ladder during the HL7 import "
									"process, you will NOT be prompted to create a ladder if the first step (%s) is automatically completed.\r\n\r\n"
									"Are you sure you wish to continue?", strPrevPrevStepName, strCurStepName, strPrevPrevStepName)) {
										return;
								}
							}
						}
					}
					//The data is as we expected.
					//Swap the values in the data for the the previous row and this row
					ExecuteSql(
						"UPDATE LabProcedureStepsT SET StepOrder = %li WHERE StepID = %li; "
						"UPDATE LabProcedureStepsT SET StepOrder = %li WHERE StepID = %li; ",
						nPrevStepOrder, nStepID, nCurOrder, nPrevStepID);

					// (r.goldschmidt 2014-08-18 15:36) - PLID 53701 - need auditing for Adminstrator>Labs tab, step order changed
					long nAuditID = BeginNewAuditEvent();
					CString strProcedureNamePlusLow, strProcedureNamePlusHigh;
					strProcedureNamePlusLow.Format("%s: (Step %li) ", strProcedureName, nPrevStepOrder);
					strProcedureNamePlusHigh.Format("%s: (Step %li) ", strProcedureName, nCurOrder);
					AuditEvent(-1, "", nAuditID, aeiAdminLabProcedureStepOrderChanged, nStepID, strProcedureNamePlusHigh + strCurStepName, strProcedureNamePlusLow + strCurStepName, aepMedium);
					AuditEvent(-1, "", nAuditID, aeiAdminLabProcedureStepOrderChanged, nPrevStepID, strProcedureNamePlusLow + strPrevStepName, strProcedureNamePlusHigh + strPrevStepName, aepMedium);

					//Swap the values in the previous row and this row
					pPrevRow->PutValue(lsStepID, (long)nStepID);
					pPrevRow->PutValue(lsLabProcedureID, (long)nCurLabProcedureID);
					pPrevRow->PutValue(lsStepOrder, (long)nPrevStepOrder);
					pPrevRow->PutValue(lsName, _bstr_t(strCurStepName));
					pPrevRow->PutValue(lsCreateLadder, varCurCreateLadder);
					pPrevRow->PutValue(lsCompletedByHL7, varCurCompletedByHL7);
					pRow->PutValue(lsStepID, (long)nPrevStepID);
					pRow->PutValue(lsLabProcedureID, (long)nPrevLabProcedureID);
					pRow->PutValue(lsStepOrder, (long)nCurOrder);
					pRow->PutValue(lsName, _bstr_t(strPrevStepName));
					pRow->PutValue(lsCreateLadder, varPrevCreateLadder);
					pRow->PutValue(lsCompletedByHL7, varPrevCompletedByHL7);

					//Ensure the selection is on the original row we moved up
					m_StepsList->PutCurSel(pPrevRow);
				}

				//Display warning that this will not take effect on existing lab records
				//(e.lally 2007-04-20) PLID 25738 - Only notified the user once when a change to a lab procedure is made (per time loaded).
				if(!m_bUserNotifiedOfChange){
					m_bUserNotifiedOfChange = TRUE;
					AfxMessageBox("Existing lab records associated with patients will not be updated by any changes to the setup.\nThe changes will only take effect in future labs.");
				}
			}
		}
		else
			AfxMessageBox("Please select a step before trying to change the ordering.");
	} NxCatchAll("Error in CLabsSetupDlg::OnUp");
}

void CLabsSetupDlg::OnDown() 
{
	try {
		// (m.hancock 2006-07-06 16:29) - PLID 21187 - Move a step down in it's processing order.

		//Get the currently selected row
		NXDATALIST2Lib::IRowSettingsPtr pRow(m_StepsList->CurSel);

		//Make sure we actually have a row selected
		if(pRow != NULL) {

			// get procedure name for auditing later
			CString strProcedureName;
			IRowSettingsPtr pProcedureRow(m_LabProcedureList->CurSel);
			if (pProcedureRow){
				strProcedureName = VarString(pProcedureRow->GetValue(lplLabProcedureName));
			}

			//Get some information about the step
			long nStepID = VarLong(pRow->GetValue(lsStepID));
			long nCurOrder = VarLong(pRow->GetValue(lsStepOrder));
			long nCurLabProcedureID = VarLong(pRow->GetValue(lsLabProcedureID));
			CString strCurStepName = VarString(pRow->GetValue(lsName), "");
			_variant_t varCurCreateLadder = pRow->GetValue(lsCreateLadder);
			_variant_t varCurCompletedByHL7 = pRow->GetValue(lsCompletedByHL7);
			
			//If this is the bottom record, we cannot go higher
			if(nCurOrder >= m_StepsList->GetRowCount())
				return;

			//Get the next row
			NXDATALIST2Lib::IRowSettingsPtr pNextRow = pRow->GetNextRow();
			if(pNextRow != NULL) {
				
				//Get some information about the next step
				long nNextStepID = VarLong(pNextRow->GetValue(lsStepID));
				long nNextStepOrder = VarLong(pNextRow->GetValue(lsStepOrder));
				long nNextLabProcedureID = VarLong(pNextRow->GetValue(lsLabProcedureID));
				CString strNextStepName = VarString(pNextRow->GetValue(lsName), "");
				_variant_t varNextCreateLadder = pNextRow->GetValue(lsCreateLadder);
				_variant_t varNextCompletedByHL7 = pNextRow->GetValue(lsCompletedByHL7);

				//(e.lally 2007-04-20) PLID 25720 - Since there was a bug that caused duplicate step orders
					//to occur in the data, we have to fix them as we come across them.
				if(nCurOrder == nNextStepOrder){
					//Darn, we have bad data. Fix it!
					//Send in the current StepOrder and StepID as we want that incremented
					FixDuplicateStepOrdering(nCurLabProcedureID, nCurOrder, nStepID);
					//All the values for this procedure should be fixed now, let's just requery the list manually to get the new ordering
					RequeryLabSteps();
				}
				else{
					//TES 1/5/2011 - PLID 37877 - If we're going to be in a state where we have a Completed By HL7 step followed by
					// a Create Ladder step, warn the user.
					if(AsBool(varCurCreateLadder) && AsBool(varNextCompletedByHL7)) {
						if(IDYES != MsgBox(MB_YESNO, "This action will result in a step (%s) that is flagged to be automatically completed "
							"when HL7 results are received, immediately followed by a step (%s) that is flagged to prompt to create a ladder "
							"when the step is activated.  Because it is not possible to prompt to create a ladder during the HL7 import "
							"process, you will NOT be prompted to create a ladder if the first step (%s) is automatically completed.\r\n\r\n"
							"Are you sure you wish to continue?", strNextStepName, strCurStepName, strNextStepName)) {
								return;
						}
					}
					else if(AsBool(varCurCompletedByHL7)) {
						IRowSettingsPtr pNextNextRow = pNextRow->GetNextRow();
						if(pNextNextRow != NULL) {
							if(AsBool(pNextNextRow->GetValue(lsCreateLadder))) {
								CString strNextNextStepName = VarString(pNextNextRow->GetValue(lsName),"");
								if(IDYES != MsgBox(MB_YESNO, "This action will result in a step (%s) that is flagged to be automatically completed "
									"when HL7 results are received, immediately followed by a step (%s) that is flagged to prompt to create a ladder "
									"when the step is activated.  Because it is not possible to prompt to create a ladder during the HL7 import "
									"process, you will NOT be prompted to create a ladder if the first step (%s) is automatically completed.\r\n\r\n"
									"Are you sure you wish to continue?", strCurStepName, strNextNextStepName, strCurStepName)) {
										return;
								}
							}
						}
					}

					//The data is as we expected.

					//Swap the values in the data for the the next row and this row
					ExecuteSql(
						"UPDATE LabProcedureStepsT SET StepOrder = %li WHERE StepID = %li; "
						"UPDATE LabProcedureStepsT SET StepOrder = %li WHERE StepID = %li; ",
						nNextStepOrder, nStepID, nCurOrder, nNextStepID);

					// (r.goldschmidt 2014-08-18 15:36) - PLID 53701 - need auditing for Adminstrator>Labs tab, step order changed
					long nAuditID = BeginNewAuditEvent();
					CString strProcedureNamePlusLow, strProcedureNamePlusHigh;
					strProcedureNamePlusLow.Format("%s: (Step %li) ", strProcedureName, nCurOrder);
					strProcedureNamePlusHigh.Format("%s: (Step %li) ", strProcedureName, nNextStepOrder);
					AuditEvent(-1, "", nAuditID, aeiAdminLabProcedureStepOrderChanged, nStepID, strProcedureNamePlusLow + strCurStepName, strProcedureNamePlusHigh + strCurStepName, aepMedium);
					AuditEvent(-1, "", nAuditID, aeiAdminLabProcedureStepOrderChanged, nNextStepID, strProcedureNamePlusHigh + strNextStepName, strProcedureNamePlusLow + strNextStepName, aepMedium);

					//Swap the values in the next row and this row
					pNextRow->PutValue(lsStepID, (long)nStepID);
					pNextRow->PutValue(lsLabProcedureID, (long)nCurLabProcedureID);
					pNextRow->PutValue(lsStepOrder, (long)nNextStepOrder);
					pNextRow->PutValue(lsName, _bstr_t(strCurStepName));
					pNextRow->PutValue(lsCreateLadder, varCurCreateLadder);
					pNextRow->PutValue(lsCompletedByHL7, varCurCompletedByHL7);
					pRow->PutValue(lsStepID, (long)nNextStepID);
					pRow->PutValue(lsLabProcedureID, (long)nNextLabProcedureID);
					pRow->PutValue(lsStepOrder, (long)nCurOrder);
					pRow->PutValue(lsName, _bstr_t(strNextStepName));
					pRow->PutValue(lsCreateLadder, varNextCreateLadder);
					pRow->PutValue(lsCompletedByHL7, varNextCompletedByHL7);

					//Ensure the selection is on the original row we moved down
					m_StepsList->PutCurSel(pNextRow);
				}

				//Display warning that this will not take effect on existing lab records
				//(e.lally 2007-04-20) PLID 25738 - Only notified the user once when a change to a lab procedure is made (per time loaded).
				if(!m_bUserNotifiedOfChange){
					m_bUserNotifiedOfChange = TRUE;
					AfxMessageBox("Existing lab records associated with patients will not be updated by any changes to the setup.\nThe changes will only take effect in future labs.");
				}
			}
		}
		else
			AfxMessageBox("Please select a step before trying to change the ordering.");
	} NxCatchAll("Error in CLabsSetupDlg::OnDown");
}

void CLabsSetupDlg::OnEditLabDesc() 
{
	try{
		
		// (z.manning 2008-10-27 10:23) - PLID 24630 - We have a new dialog to edit lab diagnoses
		CLabEditDiagnosisDlg dlg(this);
		dlg.DoModal();
		
	}NxCatchAll("Error in CLabsSetupDlg::OnEditDescriptionList");
	
}

void CLabsSetupDlg::OnEditClinicalDiags() 
{
	try{
		//(e.lally 2009-08-10) PLID 31811 - Renamed Dissection to Description
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 15, "Edit Microscopic Description List").DoModal();

	}NxCatchAll("Error in CLabsSetupDlg::OnEditClinicalDiagnosisList");
	
}

void CLabsSetupDlg::OnRequeryFinishedStepsList(short nFlags) 
{
	try{
		IRowSettingsPtr pRow = m_StepsList->GetLastRow();
		if(pRow == NULL)
			return;
		long nMaxStepOrder = VarLong(pRow->GetValue(lsStepOrder));
		long nRowCount = m_StepsList->GetRowCount();
		if(nMaxStepOrder != nRowCount){
			//(e.lally 2007-04-20) PLID 25720 - We have bad data, recalculate the step ordering and commit to disk.
			ResetCurrentStepsListOrdering();
		}
	}NxCatchAll("Error in CLabsSetupDlg::OnRequeryFinishedStepsList");
	
}

//(e.lally 2007-04-20) PLID 25720 - This function needs to take the current list of steps and
	//force a save of the step order in the database. This should only need to be called when
	//a list of steps is loaded that do not match incrementally from 1 to N. No new bad data should
	//be created anymore, but this can serve as a failsafe in the event that it ever happens again.
void CLabsSetupDlg::ResetCurrentStepsListOrdering()
{
	try{
		//unselect everything in the steps list
		m_StepsList->PutCurSel(NULL);

		//disable the datalist and arrow buttons to prevent user interaction while we do this.
		m_StepsList->Enabled = FALSE;
		GetDlgItem(IDC_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_DOWN)->EnableWindow(FALSE);

		//update the database to reflect current ordering
		CString strSqlBatch = BeginSqlBatch();
		IRowSettingsPtr pRow = m_StepsList->GetFirstRow();
		long nNewOrder = 0, nStepID=-1;
		while(pRow){
			nNewOrder++;
			nStepID = VarLong(pRow->GetValue(lsStepID));
			pRow->PutValue(lsStepOrder, (long)nNewOrder);
			AddStatementToSqlBatch(strSqlBatch, "UPDATE LabProcedureStepsT SET StepOrder = %li WHERE StepID = %li; ",
				nNewOrder, nStepID);
			pRow = pRow->GetNextRow();
		}
		if(nNewOrder == m_StepsList->GetRowCount() && !strSqlBatch.IsEmpty()){
			//Good, we didn't goof up anwhere and there are steps to update. Commit to disk.
			ExecuteSqlBatch(strSqlBatch);
		}

		//re-enable datalist and arrow buttons to allow user interaction again.
		m_StepsList->Enabled = TRUE;
		GetDlgItem(IDC_UP)->EnableWindow(TRUE);
		GetDlgItem(IDC_DOWN)->EnableWindow(TRUE);
		return;
	}NxCatchAll("Error resetting the step order");
	//If we failed somewhere in here, we should re-enable our controls so the user can continue using them again
	m_StepsList->Enabled = TRUE;
	GetDlgItem(IDC_UP)->EnableWindow(TRUE);
	GetDlgItem(IDC_DOWN)->EnableWindow(TRUE);
}

void CLabsSetupDlg::FixDuplicateStepOrdering(long nProcedureID, long nDuplicateStepOrder, long nStepIDToIncrement)
{
	try{
		ExecuteSql("DECLARE @nProcedureID INT \r\n"
			//We will need the ProcedureID throughout
			"SET @nProcedureID = %li \r\n"

			//First, fix every step order after nDuplicateStepOrder for just THIS procedure ID
			"UPDATE LabProcedureStepsT SET StepOrder = StepOrder + 1 WHERE StepOrder >= %li +1 AND "
			"	LabProcedureID = @nProcedureID AND Inactive = 0; \r\n"

			//Next, manually fix the current StepOrder duplicate.
			"UPDATE LabProcedureStepsT SET StepOrder = %li + 1 WHERE StepID = %li; "
			
			//It is also possible for our procedure to be missing steps in the ordering (i.e. 1,3,3,4,5 become 1,3,4,5,6)
				//let's fix that too.
			"DECLARE @nStepCounter INT \r\n"
			//Start looking at the value 1
			"SET @nStepCounter = 1 \r\n"
			//Loop while the max step order is higher than the count of steps (i.e. max value of 6 is more than the count of 5 records)
			"WHILE((SELECT COUNT(*) FROM LabProcedureStepsT WHERE LabProcedureID = @nProcedureID AND Inactive = 0) < "
			"      (SELECT COALESCE(Max(StepOrder), 0) FROM LabProcedureStepsT WHERE LabProcedureID = @nProcedureID AND Inactive = 0) ) \r\n"
			"BEGIN \r\n"
			//Does value of stepCounter exist?
			"	IF(NOT EXISTS(SELECT StepID FROM LabProcedureStepsT WHERE LabProcedureID = @nProcedureID AND StepOrder = @nStepCounter AND Inactive = 0)) "
			"	BEGIN \r\n"
					//No, Move every step order down one where it is more than our counter 
						//for just THIS procedure ID
			"		UPDATE LabProcedureStepsT SET StepOrder = StepOrder - 1 WHERE StepOrder > @nStepCounter AND "
			"			LabProcedureID = @nProcedureID AND Inactive = 0; \r\n"
			"	END \r\n"
			"	ELSE BEGIN \r\n"
					//Yes, the value exists, increment our counter to look at the next one
			"		SET @nStepCounter = @nStepCounter + 1\r\n"
			"	END \r\n"
			"END \r\n"
			, nProcedureID, nDuplicateStepOrder, nDuplicateStepOrder, nStepIDToIncrement);

	}NxCatchAll("Error updating duplicate step ordering")

}

// (z.manning 2008-10-10 12:10) - PLID 21108
void CLabsSetupDlg::OnDblClickCellStepsList(LPDISPATCH lpRow, short nColIndex)
{
	try
	{
		OnModifyStep();

	}NxCatchAll("CLabsSetupDlg::OnDblClickCellStepsList");
}

// (r.galicki 2008-10-21 11:33) - PLID 31552
void CLabsSetupDlg::SelChosenNxdlLabtype(LPDISPATCH lpRow)
{
	try{
		IRowSettingsPtr pProcedureRow = m_LabProcedureList->CurSel;
		IRowSettingsPtr pTypeRow(lpRow);
		
		if(pProcedureRow && pTypeRow) {
			BYTE nType = VarByte(pTypeRow->GetValue(ltlcType));
			long nID = VarLong(pProcedureRow->GetValue(lplLabProcedureID));
			ExecuteParamSql("UPDATE LabProceduresT SET Type = {INT} WHERE ID = {INT}", nType, nID);

			// (r.goldschmidt 2014-08-18 15:36) - PLID 53701 - need auditing for Adminstrator>Labs tab, procedure type
			CString strProcedureName = VarString(pProcedureRow->GetValue(lplLabProcedureName));
			BYTE nOldType = VarByte(pProcedureRow->GetValue(lplLabProcedureType));
			CString strOldTypeName;
			IRowSettingsPtr pOldTypeRow = m_LabTypeList->FindByColumn(ltlcType, nOldType, NULL, FALSE);
			if (pOldTypeRow){
				strOldTypeName = VarString(pOldTypeRow->GetValue(ltlcName));
			}
			CString strNewTypeName = VarString(pTypeRow->GetValue(ltlcName));
			AuditEvent(-1, "", BeginNewAuditEvent(), aeiAdminLabProcedureProcedureType, nID, strProcedureName + ": " + strOldTypeName, strProcedureName + ": " + strNewTypeName, aepMedium, aetChanged);
			
			//update procedure list 'Type' column
			pProcedureRow->PutValue(lplLabProcedureType, nType);
		}
	}NxCatchAll("Error in CLabsSetupDlg::SelChosenNxdlLabtype")
}

// (z.manning 2008-10-22 15:40) - PLID 21082 - Added ability to edit signatures from labs tab
void CLabsSetupDlg::OnBnClickedLabEditSignature()
{
	try
	{
		CSignatureDlg dlgSignature(this);
		dlgSignature.m_bSetupOnly = TRUE;
		// (z.manning 2008-12-09 09:07) - PLID 32260 - Added a preference for checking for password when loading signature.
		dlgSignature.m_bCheckPasswordOnLoad = (GetRemotePropertyInt("SignatureCheckPasswordLab", 1, 0, GetCurrentUserName()) == 1);
		dlgSignature.DoModal();

	}NxCatchAll("CLabsSetupDlg::OnBnClickedLabEditSignature");
}

// (z.manning 2008-11-04 15:06) - PLID 21108
void CLabsSetupDlg::RequeryLabSteps()
{
	IRowSettingsPtr pRow = m_LabProcedureList->GetCurSel();
	if(pRow == NULL) {
		m_StepsList->Clear();
	}
	else {
		long nLabProcedureID = VarLong(pRow->GetValue(lplLabProcedureID));
		CString strWhere = FormatString("LabProcedureID = %li AND Inactive = 0", nLabProcedureID);
		m_StepsList->PutWhereClause(_bstr_t(strWhere));
		m_StepsList->Requery();
	}
}

// (z.manning 2010-05-07 10:33) - PLID 35537
void CLabsSetupDlg::OnBnClickedCopyLabProcedure()
{
	try
	{
		// (z.manning 2010-05-07 10:38) - PLID 35537 - First get the lab ID of the lab
		// we're copying.
		IRowSettingsPtr pCurrentLabRow = m_LabProcedureList->GetCurSel();
		if(pCurrentLabRow == NULL) {
			MessageBox("You must select a lab first.");
			return;
		}
		long nCurrentLabID = VarLong(pCurrentLabRow->GetValue(lplLabProcedureID));

		CGetNewIDName dlg(this);
		dlg.m_nMaxLength = 50;
		CString strNewLabName;
		dlg.m_pNewName = &strNewLabName;

		if (dlg.DoModal() == IDOK) 
		{
			strNewLabName.TrimRight();

			//Check if the lab procedure is blank
			if(strNewLabName == "") {
				MessageBox("You cannot enter a blank name for a ladder.");
				return;
			}

			//Check if the lab procedure already exists
			if(ReturnsRecords("SELECT ID FROM LabProceduresT WHERE Name = '%s'", _Q(strNewLabName))) {
				MessageBox("There is already a lab procedure with this name or one may have previously existed. Please choose a new name.", "Practice", MB_OK|MB_ICONEXCLAMATION);
				return;
			}

			// (z.manning 2010-05-07 11:29) - PLID 35537 - Copy the new lab
			// (d.thompson 2010-06-14) - PLID 36791 - Added the SpecimenLabel and RequestForm options to the copy
			// (r.gonet 09/21/2011) - PLID 45584 - Copy by Reference the old lab procedure's custom fields template
			// (r.goldschmidt 2014-08-21 16:27) - PLID 53701 - need auditing for Adminstrator>Labs tab. add second recordset for steps
			_RecordsetPtr prsNewLab = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"SET XACT_ABORT ON \r\n"
				"BEGIN TRAN \r\n"
				"DECLARE @nCopyFromLabID int \r\n"
				"DECLARE @nNewLabID int \r\n"
				"SET @nCopyFromLabID = {INT} \r\n"
				"--LabProceduresT-- \r\n"
				"INSERT INTO LabProceduresT (Name, Inactive, Type, SendSpecimenLabel, SendRequestForm, LabProcCFTemplateID) \r\n"
				"SELECT {STRING}, 0, Type, SendSpecimenLabel, SendRequestForm, LabProcCFTemplateID FROM LabProceduresT WHERE ID = @nCopyFromLabID \r\n"
				"SET @nNewLabID = (SELECT CONVERT(int, SCOPE_IDENTITY())) \r\n"
				"--LabProcedureStepsT-- \r\n"
				"INSERT INTO LabProcedureStepsT (LabProcedureID, StepOrder, Name, CreateLadder, CreateTodo, TodoCategoryID, TodoPriority, Inactive) \r\n"
				"SELECT @nNewLabID, StepOrder, Name, CreateLadder, CreateTodo, TodoCategoryID, TodoPriority, Inactive \r\n"
				"FROM LabProcedureStepsT \r\n"
				"WHERE LabProcedureID = @nCopyFromLabID AND Inactive = 0 \r\n"
				"--LabProcedureStepTodoAssignToT-- \r\n"
				"INSERT INTO LabProcedureStepTodoAssignToT (LabProcedureStepID, UserID) \r\n"
				"SELECT DestSteps.StepID, LabProcedureStepTodoAssignToT.UserID \r\n"
				"FROM LabProcedureStepsT DestSteps \r\n"
				"INNER JOIN LabProcedureStepsT SourceSteps ON DestSteps.StepOrder = SourceSteps.StepOrder \r\n"
				"INNER JOIN LabProcedureStepTodoAssignToT ON SourceSteps.StepID = LabProcedureStepTodoAssignToT.LabProcedureStepID \r\n"
				"WHERE SourceSteps.LabProcedureID = @nCopyFromLabID AND DestSteps.LabProcedureID = @nNewLabID \r\n"
				"COMMIT TRAN \r\n"
				"\r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT ID, Type, SendSpecimenLabel, SendRequestForm FROM LabProceduresT WHERE ID = @nNewLabID \r\n"
				"SELECT StepID, StepOrder, Name FROM LabProcedureStepsT WHERE LabProcedureID = @nNewLabID \r\n"
				, nCurrentLabID, strNewLabName);

			// (z.manning 2010-05-07 12:39) - PLID 35537 - Add the new row to the lab procedure list
			IRowSettingsPtr pNewRow = m_LabProcedureList->GetNewRow();
			pNewRow->PutValue(lplLabProcedureID, prsNewLab->GetFields()->GetItem("ID")->GetValue());
			pNewRow->PutValue(lplLabProcedureName, _bstr_t(strNewLabName));
			pNewRow->PutValue(lplLabProcedureType, prsNewLab->GetFields()->GetItem("Type")->GetValue());
			m_LabProcedureList->PutCurSel(m_LabProcedureList->AddRowSorted(pNewRow, NULL));

			// (d.thompson 2010-06-14) - PLID 36791 - By default, the 2 'send' options are always enabled on new lab procedures
			CheckDlgButton(IDC_SEND_SPECIMEN_LABEL, AdoFldBool(prsNewLab, "SendSpecimenLabel"));
			CheckDlgButton(IDC_SEND_REQUEST_FORM, AdoFldBool(prsNewLab, "SendRequestForm"));

			// (r.goldschmidt 2014-08-22 10:36) - PLID 53701 - It appears that setting the procedure type isn't necessary since it remains the same
			//    (similarly for checkboxes, but they were already in code, so I didn't change it.)

			// (r.goldschmidt 2014-08-18 15:36) - PLID 53701 - need auditing for Adminstrator>Labs tab, copy lab procedure
			long nLabRecordID = AdoFldLong(prsNewLab, "ID");
			CAuditTransaction auditTran;
			AuditEvent(-1, "", auditTran, aeiAdminLabProcedureCreated, nLabRecordID, "", strNewLabName, aepMedium, aetCreated);

			BYTE nType = AdoFldByte(prsNewLab, "Type");
			CString strTypeName;
			IRowSettingsPtr pTypeRow = m_LabTypeList->FindByColumn(ltlcType, nType, NULL, FALSE);
			if (pTypeRow){
				strTypeName = VarString(pTypeRow->GetValue(ltlcName));
			}
			CString strNewTypeName = strNewLabName + ": " + strTypeName;
			AuditEvent(-1, "", auditTran, aeiAdminLabProcedureProcedureType, nLabRecordID, "", strNewTypeName, aepMedium, aetCreated);

			CString strNewSpecimen = strNewLabName + ": " + (AdoFldBool(prsNewLab, "SendSpecimenLabel") ? "Checked" : "Unchecked");
			AuditEvent(-1, "", auditTran, aeiAdminLabProcedureSendSpecimenLabel, nLabRecordID, "", strNewSpecimen, aepMedium, aetCreated);

			CString strNewRequest = strNewLabName + ": " + (AdoFldBool(prsNewLab, "SendRequestForm") ? "Checked" : "Unchecked");
			AuditEvent(-1, "", auditTran, aeiAdminLabProcedureSendRequestForm, nLabRecordID, "", strNewRequest, aepMedium, aetCreated);

			// (r.goldschmidt 2014-08-22 10:36) - PLID 53701 - now audit the creation of each step
			prsNewLab = prsNewLab->NextRecordset(NULL);
			while (!prsNewLab->eof){
				CString strNewStepName;
				strNewStepName.Format("%s: (Step %li) %s", strNewLabName, AdoFldLong(prsNewLab, "StepOrder"), AdoFldString(prsNewLab, "Name"));
				AuditEvent(-1, "", auditTran, aeiAdminLabProcedureStepCreated, AdoFldLong(prsNewLab, "StepID"), "", strNewStepName, aepMedium, aetCreated);
				prsNewLab->MoveNext();
			}
			auditTran.Commit();
			
			// (z.manning 2010-05-07 12:43) - PLID 35537 - Reload the steps
			RequeryLabSteps();

			//(e.lally 2007-04-26) PLID 25738 - Reset our flag for new lab procedures.
			m_bUserNotifiedOfChange = FALSE;
		}

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-05-13 16:27) - PLID 37876
void CLabsSetupDlg::OnBnClickedLabEditLabelPrintSettings()
{
	try
	{
		// Loads the lab label print settings and displays the print dialog
		LabUtils::ShowLabLabelPrintSettings();
	}NxCatchAll(__FUNCTION__);
}

// (d.thompson 2010-06-14) - PLID 36791
void CLabsSetupDlg::OnBnClickedSendSpecimenLabel()
{
	try {
		//Get the lab procedure row
		NXDATALIST2Lib::IRowSettingsPtr pRow(m_LabProcedureList->CurSel);
		if(pRow != NULL) {
			//Get the currently selected lab procedure ID and name
			long nID = VarLong(pRow->GetValue(lplLabProcedureID));
			CString strLabProcedureName = VarString(pRow->GetValue(lplLabProcedureName));

			BOOL bChecked = IsDlgButtonChecked(IDC_SEND_SPECIMEN_LABEL);

			ExecuteParamSql("UPDATE LabProceduresT SET SendSpecimenLabel = {INT} WHERE ID = {INT};", bChecked == FALSE ? 0 : 1, nID);

			// (r.goldschmidt 2014-08-21 17:20) - PLID 53701 - need auditing for Adminstrator>Labs tab, send specimen label checkbox
			AuditEvent(-1, "", BeginNewAuditEvent(), aeiAdminLabProcedureSendSpecimenLabel, nID, strLabProcedureName + ": " + (!!bChecked ? "Unchecked" : "Checked"), strLabProcedureName + ": " + (!!bChecked ? "Checked" : "Unchecked"), aepMedium);
		}
		else {
			//Shouldn't be possible, but no worries if it is, just ignore.
		}
	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2010-06-14) - PLID 36791
void CLabsSetupDlg::OnBnClickedSendRequestForm()
{
	try {
		//Get the lab procedure row
		NXDATALIST2Lib::IRowSettingsPtr pRow(m_LabProcedureList->CurSel);
		if(pRow != NULL) {
			//Get the currently selected lab procedure ID and name
			long nID = VarLong(pRow->GetValue(lplLabProcedureID));
			CString strLabProcedureName = VarString(pRow->GetValue(lplLabProcedureName));

			BOOL bChecked = IsDlgButtonChecked(IDC_SEND_REQUEST_FORM);

			ExecuteParamSql("UPDATE LabProceduresT SET SendRequestForm = {INT} WHERE ID = {INT};", bChecked == FALSE ? 0 : 1, nID);

			// (r.goldschmidt 2014-08-21 17:20) - PLID 53701 - need auditing for Adminstrator>Labs tab, send request form checkbox
			AuditEvent(-1, "", BeginNewAuditEvent(), aeiAdminLabProcedureSendRequestForm, nID, strLabProcedureName + ": " + (!!bChecked ? "Unchecked" : "Checked"), strLabProcedureName + ": " + (!!bChecked ? "Checked" : "Unchecked"), aepMedium);
		}
		else {
			//Shouldn't be possible, but no worries if it is, just ignore.
		}
	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-06-25 15:52) - PLID 39185 - added ability to link locations and 'to be ordered' entries
void CLabsSetupDlg::OnBtnConfigLabToBeOrderedLoc()
{
	try {

		CLabsToBeOrderedSetupDlg dlg(this);
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

void CLabsSetupDlg::OnBnClickedEditCustomFields()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(m_LabProcedureList->CurSel);
		if(pRow != NULL) {
			//Get the currently selected lab procedure ID
			long nID = VarLong(pRow->GetValue(lplLabProcedureID));
			CLabsSelCustomFieldsDlg dlg(nID, this);
			dlg.DoModal();
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 10/11/2011) - PLID 45924 - Open the barcode setup dlg.
void CLabsSetupDlg::OnBnClickedLabBarcodeSetupBtn()
{
	try {
		CLabBarcodeSetupDlg dlg(this);
		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 03/29/2012) - PLID 49299 - Open the lab procedure group setup dialog.
void CLabsSetupDlg::OnBnClickedLabProcedureGroupSetupBtn()
{
	try {
		CLabProcedureGroupsSetupDlg dlg(this);
		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}

// (s.tullis 2015-11-16 10:50) - PLID 54285 - Create a way to import the labs' compendiums of orderable test codes, which are usually in CSV or Excel files.
void CLabsSetupDlg::OnBnClickedImportLoinc()
{
	try {
		CImportLoincDlg dlg(this);
		dlg.DoModal();
	}NxCatchAll(__FUNCTION__)
}
