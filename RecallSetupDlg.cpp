// RecallSetupDlg.cpp : implementation file
//

// (j.armen 2012-03-06 16:45) - PLID 48304 - Created

#include "stdafx.h"
#include "AdministratorRc.h"
#include "RecallSetupDlg.h"
#include "GlobalDataUtils.h"
#include "GetNewIDName.h"
#include "MultiSelectDlg.h"
#include "AuditTrail.h"
#include "DiagSearchUtils.h"
#include "GlobalAuditUtils.h" 

// CRecallSetupDlg

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (j.armen 2012-03-23 12:21) - PLID 49008 - Auditing Macros
#define BEGIN_AUDIT	{ CAuditTransaction auditTran;
#define AUDIT(nItem, nRecordID, strOldValue, strNewValue, aet)	AuditEvent(-1, "", auditTran, nItem, nRecordID, strOldValue, strNewValue, aepMedium, aet);
#define END_AUDIT	auditTran.Commit(); };

IMPLEMENT_DYNAMIC(CRecallSetupDlg, CNxDialog)

CRecallSetupDlg::CRecallSetupDlg(CWnd* pParent /*=NULL*/) : CNxDialog(CRecallSetupDlg::IDD, pParent), m_nRecallTemplateID(-1), m_nRecallStepID(-1)
{
}

CRecallSetupDlg::~CRecallSetupDlg()
{
}


BEGIN_MESSAGE_MAP(CRecallSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_RECALL_NEW_TEMPLATE, OnBnClickedNewRecallTemplate)
	ON_BN_CLICKED(IDC_RECALL_NEW_COPY_TEMPLATE, OnBnClickedNewCopyRecallTemplate)
	ON_BN_CLICKED(IDC_RECALL_DELETE_TEMPLATE, OnBnClickedDeleteRecallTemplate)
	ON_BN_CLICKED(IDC_RECALL_RENAME_TEMPLATE, OnBnClickedRenameRecallTemplate)
	ON_BN_CLICKED(IDC_RECALL_NEW_STEP, OnBnClickedNewRecallStep)
	ON_BN_CLICKED(IDC_RECALL_DELETE_STEP, OnBnClickedDeleteRecallStep)
	ON_BN_CLICKED(IDC_RECALL_STEP_UP, OnBnClickedRecallStepUp)
	ON_BN_CLICKED(IDC_RECALL_STEP_DOWN, OnBnClickedRecallStepDown)
	ON_BN_CLICKED(IDC_RECALL_REPEAT_LAST_STEP, OnBnClickedRecallRepeatStep)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CRecallSetupDlg, CNxDialog)
	ON_EVENT(CRecallSetupDlg, IDC_NXDL_RECALL_TEMPLATE_SETUP, 16 /* SelChosen */, OnSelChosenRecallTemplate, VTS_DISPATCH)
	ON_EVENT(CRecallSetupDlg, IDC_NXDL_RECALL_TEMPLATE_SETUP, 18 /* RequeryFinished */, OnRequeryFinishedRecallTemplate, VTS_I2)
	ON_EVENT(CRecallSetupDlg, IDC_NXDL_RECALL_STEPS, 29 /* SelChosen */, OnSelChosenRecallStep, VTS_DISPATCH)
	ON_EVENT(CRecallSetupDlg, IDC_NXDL_RECALL_STEPS, 18 /* RequeryFinished */, OnRequeryFinishedRecallStep, VTS_I2)
	ON_EVENT(CRecallSetupDlg, IDC_NXDL_RECALL_STEPS, 9, EditingFinishingNxdlRecallSteps, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CRecallSetupDlg, IDC_RECALL_SETUP_DIAGNOSIS_SEARCH, 16, CRecallSetupDlg::SelChosenRecallSetupDiagnosisSearch, VTS_DISPATCH)
	ON_EVENT(CRecallSetupDlg, IDC_NXDL_RECALL_DIAGNOSIS_SELECTED, 6, CRecallSetupDlg::RButtonDownNxdlRecallDiagCodeSelected, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CRecallSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RECALL_NXCOLORCTRL1, m_nxcolor1);
	DDX_Control(pDX, IDC_RECALL_NXCOLORCTRL2, m_nxcolor2);
	DDX_Control(pDX, IDC_RECALL_NXCOLORCTRL3, m_nxcolor3);
	DDX_Control(pDX, IDC_RECALL_NXCOLORCTRL4, m_nxcolor4);
	DDX_Control(pDX, IDC_RECALL_NEW_TEMPLATE, m_btnNewRecallTemplate);
	DDX_Control(pDX, IDC_RECALL_NEW_COPY_TEMPLATE, m_btnNewCopyRecallTemplate);
	DDX_Control(pDX, IDC_RECALL_DELETE_TEMPLATE, m_btnDeleteRecallTemplate);
	DDX_Control(pDX, IDC_RECALL_NXL_TEMPLATE, m_nxlRecallTemplate);
	DDX_Control(pDX, IDC_RECALL_RENAME_TEMPLATE, m_btnRenameRecallTemplate);
	DDX_Control(pDX, IDC_RECALL_NEW_STEP, m_btnNewRecallStep);
	DDX_Control(pDX, IDC_RECALL_DELETE_STEP, m_btnDeleteRecallStep);
	DDX_Control(pDX, IDC_RECALL_STEP_UP, m_btnRecallStepUp);
	DDX_Control(pDX, IDC_RECALL_STEP_DOWN, m_btnRecallStepDown);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_RECALL_REPEAT_LAST_STEP, m_btnRepeatLastStep);
}

BOOL CRecallSetupDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try
	{
		HICON hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_RECALL));
		SetIcon(hIcon, TRUE);		// Set big icon
		SetIcon(hIcon, FALSE);		// Set small icon

		m_nxcolor1.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_nxcolor2.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_btnNewRecallTemplate.AutoSet(NXB_NEW);
		m_btnNewCopyRecallTemplate.AutoSet(NXB_NEW);
		m_btnDeleteRecallTemplate.AutoSet(NXB_DELETE);
		m_nxlRecallTemplate.SetText("Recall Template");
		m_nxlRecallTemplate.SetHzAlign(DT_RIGHT);
		m_nxlRecallTemplate.SetSingleLine();
		m_pdlRecallTemplate = BindNxDataList2Ctrl(IDC_NXDL_RECALL_TEMPLATE_SETUP);
		m_btnRenameRecallTemplate.AutoSet(NXB_MODIFY);
		m_btnNewRecallStep.AutoSet(NXB_NEW);
		m_btnDeleteRecallStep.AutoSet(NXB_DELETE);
		m_btnRecallStepUp.AutoSet(NXB_UP);
		m_btnRecallStepDown.AutoSet(NXB_DOWN);
		m_btnClose.AutoSet(NXB_CLOSE);
		m_pdlRecallSteps = BindNxDataList2Ctrl(IDC_NXDL_RECALL_STEPS, false);
		m_pdlDiagCode = BindNxDataList2Ctrl(IDC_NXDL_RECALL_DIAGNOSIS_SELECTED, false);
		// (a.wilson 2014-02-17 12:04) - PLID 60775 - New Diagnosis Search
		m_pdlRecallDiagnosisSearch = DiagSearchUtils::BindDiagDualSearchListCtrl(this, IDC_RECALL_SETUP_DIAGNOSIS_SEARCH, GetRemoteData());
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (j.armen 2012-03-23 12:22) - PLID 49008 - Added Auditing
void CRecallSetupDlg::OnBnClickedNewRecallTemplate()
{
	try
	{
		CGetNewIDName dlg(this);
		CString strRecallTemplateName;
		
		dlg.m_pNewName = &strRecallTemplateName;
		dlg.m_nMaxLength = 50;
		dlg.m_strCaption = "Enter a name for the new recall template:";

		if(dlg.DoModal() == IDOK)
		{
			strRecallTemplateName.Trim();
			if(strRecallTemplateName.IsEmpty())
			{
				MessageBox("You must enter a name for this recall template.","Practice",MB_OK|MB_ICONEXCLAMATION);
				return;
			}

			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON\r\n"
				"DECLARE @Name NVARCHAR(50)\r\n"
				"DECLARE @OutRecallTemplateT TABLE (ID INT, TemplateName NVARCHAR(50))\r\n"
				"DECLARE @OutRecallStepT TABLE (ID INT, RecallTemplateID INT, StepOrder INT, StepName NVARCHAR(50))\r\n"
				"SET @Name = {STRING}\r\n"
				"BEGIN TRAN\r\n"
				"IF NOT EXISTS (SELECT ID FROM RecallTemplateT WHERE Name = @Name AND Active = 1)\r\n"
				"BEGIN\r\n"
				"	INSERT\r\n"
				"		INTO RecallTemplateT (Name)\r\n"
				"		OUTPUT inserted.ID, inserted.Name INTO @OutRecallTemplateT\r\n"
				"		VALUES (@Name)\r\n"
				"	INSERT\r\n"
				"		INTO RecallStepT (RecallTemplateID, Name, StepOrder)\r\n"
				"		OUTPUT inserted.ID, inserted.RecallTemplateID, inserted.StepOrder, inserted.Name INTO @OutRecallStepT\r\n"
				"		SELECT ID, 'Step 1', 1 FROM @OutRecallTemplateT\r\n"
				"END\r\n"
				"COMMIT TRAN\r\n"
				"SET NOCOUNT OFF\r\n"
				"SELECT ID, TemplateName AS NewValue FROM @OutRecallTemplateT\r\n"
				"SELECT RecallStepT.ID, TemplateName + ': (' + CAST(StepOrder AS NVARCHAR) + ') ' + StepName AS NewValue FROM @OutRecallStepT RecallStepT INNER JOIN @OutRecallTemplateT RecallTemplateT ON RecallStepT.RecallTemplateID = RecallTemplateT.ID\r\n",
				strRecallTemplateName);

			if(prs->eof)
			{
				MessageBox("There is already a recall template with this name.\r\nPlease choose a new name.","Practice",MB_OK|MB_ICONEXCLAMATION);
			}
			else
			{
				SetActiveRecallTemplateID(AdoFldLong(prs, "ID"));
				m_pdlRecallTemplate->Requery();
				
				BEGIN_AUDIT
				{
					for(; !prs->eof; prs->MoveNext()) {
						AUDIT(aeiRecallTemplateCreated, AdoFldLong(prs, "ID"), "", AdoFldString(prs, "NewValue"), aetCreated)
					}

					prs = prs->NextRecordset(NULL);
					for(; !prs->eof; prs->MoveNext()) {
						AUDIT(aeiRecallStepCreated, AdoFldLong(prs, "ID"), "", AdoFldString(prs, "NewValue"), aetCreated)	
					}
				}
				END_AUDIT
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-03-23 12:22) - PLID 49008 - Added Auditing
void CRecallSetupDlg::OnBnClickedNewCopyRecallTemplate()
{
	try
	{
		IRowSettingsPtr pRow = GetActiveRecallTemplateRow();
		
		CGetNewIDName dlg(this);
		CString strRecallTemplateName;
		
		dlg.m_pNewName = &strRecallTemplateName;
		dlg.m_nMaxLength = 50;
		dlg.m_strCaption = "Enter a name for the new recall template:";

		if(dlg.DoModal() == IDOK)
		{
			strRecallTemplateName.Trim();
			if(strRecallTemplateName.IsEmpty())
			{
				MessageBox("You must enter a name for this recall template.","Practice",MB_OK|MB_ICONEXCLAMATION);
				return;
			}

			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON\r\n"
				"DECLARE @Name NVARCHAR(50)\r\n"
				"DECLARE @RecallTemplateID INT\r\n"
				"DECLARE @OutRecallTemplateT TABLE (ID INT, TemplateName NVARCHAR(50))\r\n"
				"DECLARE @OutRecallStepT TABLE (ID INT, RecallTemplateID INT, StepOrder INT, StepName NVARCHAR(50))\r\n"
				"DECLARE @NewRecallTemplateDiagLinkT TABLE (RecallTemplateID INT, DiagCodesID INT)\r\n"
				"SET @Name = {STRING}\r\n"
				"SET @RecallTemplateID = {INT}\r\n"
				"BEGIN TRAN\r\n"
				"IF NOT EXISTS (SELECT ID FROM RecallTemplateT WHERE Name = @Name AND Active = 1)\r\n"
				"BEGIN\r\n"
				"	INSERT\r\n"
				"		INTO RecallTemplateT (Name, RepeatLastStep)\r\n"
				"		OUTPUT inserted.ID, inserted.Name INTO @OutRecallTemplateT\r\n"
				"		SELECT @Name, RepeatLastStep FROM RecallTemplateT WHERE ID = @RecallTemplateID\r\n"
				"	INSERT\r\n"
				"		INTO RecallStepT (RecallTemplateID, Name, StepOrder, Days, Weeks, Months, Years)\r\n"
				"		OUTPUT inserted.ID, inserted.RecallTemplateID, inserted.StepOrder, inserted.Name INTO @OutRecallStepT\r\n"
				"		SELECT (SELECT ID FROM @OutRecallTemplateT), Name, StepOrder, Days, Weeks, Months, Years FROM RecallStepT WHERE RecallTemplateID = @RecallTemplateID AND Active = 1\r\n"
				"	INSERT\r\n"
				"		INTO RecallTemplateDiagLinkT (RecallTemplateID, DiagCodesID)\r\n"
				"		OUTPUT inserted.RecallTemplateID, inserted.DiagCodesID INTO @NewRecallTemplateDiagLinkT\r\n"
				"		SELECT (SELECT ID FROM @OutRecallTemplateT), DiagCodesID FROM RecallTemplateDiagLinkT WHERE RecallTemplateID = @RecallTemplateID\r\n"
				"END\r\n"
				"COMMIT TRAN\r\n"
				"SET NOCOUNT OFF\r\n"
				"SELECT ID, TemplateName AS NewValue FROM @OutRecallTemplateT\r\n"
				"SELECT RecallStepT.ID, TemplateName + ': (' + CAST(StepOrder AS NVARCHAR) + ') ' + StepName AS NewValue FROM @OutRecallStepT RecallStepT INNER JOIN @OutRecallTemplateT RecallTemplateT ON RecallStepT.RecallTemplateID = RecallTemplateT.ID\r\n"
				"SELECT ID, TemplateName + ': ' + NewSubQ.DiagCodes AS NewValue FROM @OutRecallTemplateT RecallTemplateT\r\n"
				"INNER JOIN\r\n"
				"(\r\n"
				"	SELECT SubQ.RecallTemplateID, LEFT(DiagCodes, LEN(DiagCodes)-1) AS DiagCodes\r\n"
				"	FROM(\r\n"
				"		SELECT DISTINCT B.RecallTemplateID,\r\n"
				"			(\r\n"
				"				SELECT CodeNumber + ', '\r\n"
				"				FROM @NewRecallTemplateDiagLinkT A\r\n"
				"				INNER JOIN DiagCodes ON A.DiagCodesID = DiagCodes.ID\r\n"
				"				WHERE A.RecallTemplateID = B.RecallTemplateID\r\n"
				"				ORDER BY A.RecallTemplateID\r\n"
				"				FOR XML PATH ('')\r\n"
				"			) DiagCodes\r\n"
				"		FROM @NewRecallTemplateDiagLinkT B\r\n"
				"	) SubQ\r\n"
				") NewSubQ ON RecallTemplateT.ID = NewSubQ.RecallTemplateID\r\n",
				strRecallTemplateName, VarLong(pRow->GetValue(eRecallTemplateID)));

			if(prs->eof)
			{
				MessageBox("There is already a recall template with this name.\r\nPlease choose a new name.","Practice",MB_OK|MB_ICONEXCLAMATION);
			}
			else
			{
				SetActiveRecallTemplateID(AdoFldLong(prs, "ID"));
				m_pdlRecallTemplate->Requery();

				BEGIN_AUDIT
				{
					for(; !prs->eof; prs->MoveNext()) {
						AUDIT(aeiRecallTemplateCreated, AdoFldLong(prs, "ID"), "", AdoFldString(prs, "NewValue"), aetCreated)
					}

					prs = prs->NextRecordset(NULL);
					for(; !prs->eof; prs->MoveNext()) {
						AUDIT(aeiRecallStepCreated, AdoFldLong(prs, "ID"), "", AdoFldString(prs, "NewValue"), aetCreated)	
					}

					prs = prs->NextRecordset(NULL);
					for(; !prs->eof; prs->MoveNext()) {
						AUDIT(aeiRecallDiagLinkModified, AdoFldLong(prs, "ID"), "", AdoFldString(prs, "NewValue"), aetCreated)	
					}
				}
				END_AUDIT
			}			
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-03-14 12:31) - PLID 48798 - Ability to delete/inactive a recall template
// (j.armen 2012-03-23 12:22) - PLID 49008 - Added Auditing
void CRecallSetupDlg::OnBnClickedDeleteRecallTemplate()
{
	try
	{
		IRowSettingsPtr pRow = GetActiveRecallTemplateRow();

		_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON\r\n"
			"DECLARE @RecallTemplateID INT\r\n"
			"DECLARE @OutRecallTemplateT TABLE (ID INT, TemplateName NVARCHAR(50))\r\n"
			"DECLARE @OutRecallStepT TABLE (ID INT, RecallTemplateID INT, StepOrder INT, StepName NVARCHAR(50))\r\n"
			"DECLARE @OldRecallTemplateDiagLinkT TABLE (RecallTemplateID INT, DiagCodesID INT)\r\n"
			"SET @RecallTemplateID = {INT}\r\n"
			"BEGIN TRAN\r\n"
			"IF NOT EXISTS (SELECT ID FROM RecallT WHERE RecallTemplateID = @RecallTemplateID)\r\n"
			"BEGIN\r\n"
			"	DELETE FROM RecallTemplateDiagLinkT\r\n"
			"		OUTPUT deleted.RecallTemplateID, deleted.DiagCodesID INTO @OldRecallTemplateDiagLinkT\r\n"
			"		WHERE RecallTemplateID = @RecallTemplateID\r\n"
			"	DELETE FROM RecallStepT\r\n"
			"		OUTPUT deleted.ID, deleted.RecallTemplateID, deleted.StepOrder, deleted.Name INTO @OutRecallStepT\r\n"
			"		WHERE RecallTemplateID = @RecallTemplateID\r\n"
			"	DELETE FROM RecallTemplateT\r\n"
			"		OUTPUT deleted.ID, deleted.Name INTO @OutRecallTemplateT\r\n"
			"		WHERE ID = @RecallTemplateID\r\n"
			"END\r\n"
			"COMMIT TRAN\r\n"
			"SET NOCOUNT OFF\r\n"
			"SELECT ID, TemplateName + ': ' + OldSubQ.DiagCodes AS OldValue FROM @OutRecallTemplateT RecallTemplateT\r\n"
			"INNER JOIN\r\n"
			"(\r\n"
			"	SELECT SubQ.RecallTemplateID, LEFT(DiagCodes, LEN(DiagCodes)-1) AS DiagCodes\r\n"
			"	FROM(\r\n"
			"		SELECT DISTINCT B.RecallTemplateID,\r\n"
			"			(\r\n"
			"				SELECT CodeNumber + ', '\r\n"
			"				FROM @OldRecallTemplateDiagLinkT A\r\n"
			"				INNER JOIN DiagCodes ON A.DiagCodesID = DiagCodes.ID\r\n"
			"				WHERE A.RecallTemplateID = B.RecallTemplateID\r\n"
			"				ORDER BY A.RecallTemplateID\r\n"
			"				FOR XML PATH ('')\r\n"
			"			) DiagCodes\r\n"
			"		FROM @OldRecallTemplateDiagLinkT B\r\n"
			"	) SubQ\r\n"
			") OldSubQ ON RecallTemplateT.ID = OldSubQ.RecallTemplateID\r\n"
			"SELECT RecallStepT.ID, TemplateName + ': (' + CAST(StepOrder AS NVARCHAR) + ') ' + StepName AS OldValue FROM @OutRecallStepT RecallStepT INNER JOIN @OutRecallTemplateT RecallTemplateT ON RecallStepT.RecallTemplateID = RecallTemplateT.ID\r\n"
			"SELECT ID, TemplateName AS OldValue FROM @OutRecallTemplateT\r\n",
			VarLong(pRow->GetValue(eRecallTemplateID)));

		_RecordsetPtr prsDiag = prs;
		_RecordsetPtr prsStep = prsDiag->NextRecordset(NULL);
		_RecordsetPtr prsTemplate = prsStep->NextRecordset(NULL);

		if(prsTemplate->eof)
		{
			if(IDYES == MessageBox("This recall template is already in use by patients and cannot be deleted.  Would you like to inactivate instead?","Practice",MB_YESNO|MB_ICONQUESTION))
			{
				_RecordsetPtr prs = CreateParamRecordset(
					"SET NOCOUNT ON\r\n"
					"DECLARE @OutRecallTemplateT TABLE (ID INT, TemplateName NVARCHAR(50))\r\n"
					"UPDATE RecallTemplateT\r\n"
					"	SET Active = 0\r\n"
					"	OUTPUT inserted.ID, inserted.Name INTO @OutRecallTemplateT\r\n"
					"	WHERE Active = 1 AND ID = {INT}\r\n"
					"SET NOCOUNT OFF\r\n"
					"SELECT ID, TemplateName + ': Active' AS OldValue, TemplateName + ': Inactive' AS NewValue FROM @OutRecallTemplateT\r\n",
					VarLong(pRow->GetValue(eRecallTemplateID)));

				SetActiveRecallTemplateID(-1);
				m_pdlRecallTemplate->Requery();

				BEGIN_AUDIT
				{
					for(; !prs->eof; prs->MoveNext()) {
						AUDIT(aeiRecallTemplateInactivated, AdoFldLong(prs, "ID"), AdoFldString(prs, "OldValue"), AdoFldString(prs, "NewValue"), aetChanged)
					}
				}
				END_AUDIT
			}
		}
		else
		{
			SetActiveRecallTemplateID(-1);
			m_pdlRecallTemplate->Requery();
			BEGIN_AUDIT
			{
				for(; !prsDiag->eof; prsDiag->MoveNext()) {
					AUDIT(aeiRecallDiagLinkModified, AdoFldLong(prsDiag, "ID"), AdoFldString(prsDiag, "OldValue", ""), "", aetDeleted);
				}

				for(; !prsStep->eof; prsStep->MoveNext()) {
					AUDIT(aeiRecallStepDeleted, AdoFldLong(prsStep, "ID"), AdoFldString(prsStep, "OldValue"), "", aetDeleted)
				}

				for(; !prsTemplate->eof; prsTemplate->MoveNext()) {
					AUDIT(aeiRecallTemplateDeleted, AdoFldLong(prsTemplate, "ID"), AdoFldString(prsTemplate, "OldValue"), "", aetDeleted)	
				}
			}
			END_AUDIT
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-03-23 12:22) - PLID 49008 - Added Auditing
void CRecallSetupDlg::OnBnClickedRenameRecallTemplate()
{
	try
	{
		IRowSettingsPtr pRow = GetActiveRecallTemplateRow();

		CGetNewIDName dlg(this);
		CString strRecallTemplateName;
		
		dlg.m_pNewName = &strRecallTemplateName;
		dlg.m_nMaxLength = 50;
		dlg.m_strCaption = "Enter a new name for this recall template:";

		if(dlg.DoModal() == IDOK)
		{
			strRecallTemplateName.Trim();
			if(strRecallTemplateName.IsEmpty())
			{
				MessageBox("You must enter a name for this recall template.","Practice",MB_OK|MB_ICONEXCLAMATION);
				return;
			}

			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON\r\n"
				"DECLARE @OutRecallTemplateT TABLE (ID INT, OldValue NVARCHAR(50), NewValue NVARCHAR(50))\r\n"
				"DECLARE @Name NVARCHAR(50)\r\n"
				"SET @Name = {STRING}\r\n"
				"BEGIN TRAN\r\n"
				"IF NOT EXISTS (SELECT ID FROM RecallTemplateT WHERE Name = @Name AND Active = 1)\r\n"
				"BEGIN\r\n"
				"	UPDATE RecallTemplateT\r\n"
				"		SET Name = @Name\r\n"
				"		OUTPUT inserted.ID, deleted.Name, inserted.Name INTO @OutRecallTemplateT\r\n"
				"		WHERE ID = {INT}\r\n"
				"END\r\n"
				"COMMIT TRAN\r\n"
				"SET NOCOUNT OFF\r\n"
				"SELECT ID, OldValue, NewValue FROM @OutRecallTemplateT",
				strRecallTemplateName, VarLong(pRow->GetValue(eRecallTemplateID)));

			if(prs->eof)
			{
				MessageBox("There is already a recall template with this name.\r\nPlease choose a new name.","Practice",MB_OK|MB_ICONEXCLAMATION);
			}
			else
			{
				m_pdlRecallTemplate->Requery();

				BEGIN_AUDIT
				{
					for(; !prs->eof; prs->MoveNext()) {
						AUDIT(aeiRecallTemplateRename, AdoFldLong(prs, "ID"), AdoFldString(prs, "OldValue"), AdoFldString(prs, "NewValue"), aetChanged)	
					}
				}
				END_AUDIT
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-03-23 12:22) - PLID 49008 - Added Auditing
void CRecallSetupDlg::OnBnClickedNewRecallStep()
{
	try
	{
		IRowSettingsPtr pRow = GetActiveRecallStepRow();

		CGetNewIDName dlg(this);
		CString strRecallStepName;
		
		dlg.m_pNewName = &strRecallStepName;
		dlg.m_nMaxLength = 50;
		dlg.m_strCaption = "Enter a name for the new recall step:";

		if(dlg.DoModal() == IDOK)
		{
			strRecallStepName.Trim();
			if(strRecallStepName.IsEmpty())
			{
				MessageBox("You must enter a name for this recall step.","Practice",MB_OK|MB_ICONEXCLAMATION);
				return;
			}

			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON\r\n"
				"DECLARE @Name NVARCHAR(50)\r\n"
				"DECLARE @Message NVARCHAR(100)\r\n"
				"DECLARE @RecallStepID INT\r\n"
				"DECLARE @InsertStepOrder INT\r\n"
				"DECLARE @RecallTemplateID INT\r\n"
				"DECLARE @OutRecallStepT TABLE (ID INT, RecallTemplateID INT, StepOrder INT, OldStepOrder INT, StepName NVARCHAR(50))\r\n"
				"SET @Name = {STRING}\r\n"
				"SET @RecallStepID = {INT}\r\n"
				"BEGIN TRAN\r\n"
				"SELECT @InsertStepOrder = (StepOrder + 1), @RecallTemplateID = RecallTemplateID FROM RecallStepT WHERE ID = @RecallStepID\r\n"
				"IF NOT EXISTS (SELECT ID FROM RecallT WHERE RecallTemplateID = @RecallTemplateID)\r\n"
				"BEGIN\r\n"
				"	IF NOT EXISTS (SELECT ID FROM RecallStepT WHERE Name = @Name AND RecallTemplateID = @RecallTemplateID AND Active = 1)\r\n"
				"	BEGIN\r\n"
				"		UPDATE RecallStepT\r\n"
				"			SET StepOrder = (StepOrder + 1)\r\n"
				"			OUTPUT inserted.ID, inserted.RecallTemplateID, inserted.StepOrder, deleted.StepOrder, inserted.Name INTO @OutRecallStepT\r\n"
				"			WHERE RecallTemplateID = @RecallTemplateID AND StepOrder >= @InsertStepOrder\r\n"
				"		INSERT\r\n"
				"			INTO RecallStepT (Name, RecallTemplateID, StepOrder)\r\n"
				"			OUTPUT inserted.ID, inserted.RecallTemplateID, inserted.StepOrder, NULL, inserted.Name INTO @OutRecallStepT\r\n"
				"			VALUES (@Name, @RecallTemplateID, @InsertStepOrder)\r\n"
				"	END\r\n"
				"	ELSE\r\n"
				"	BEGIN\r\n"
				"		SET @Message = 'There is already a recall step with this name.\r\nPlease choose a new name.'\r\n"
				"	END\r\n"
				"END\r\n"
				"ELSE\r\n"
				"BEGIN\r\n"
				"	SET @Message = 'This recall template has already been used.\r\nThe steps cannot be modified.'\r\n"
				"END\r\n"
				"COMMIT TRAN\r\n"
				"SET NOCOUNT OFF\r\n"
				"SELECT @Message AS Message WHERE @Message IS NOT NULL\r\n"
				"SELECT RecallStepT.ID, CASE WHEN OldStepOrder IS NULL THEN '' ELSE RecallTemplateT.Name + ': (' + CAST(OldStepOrder AS NVARCHAR) + ') ' + StepName END AS OldValue,	CASE WHEN StepOrder IS NULL THEN '' ELSE RecallTemplateT.Name + ': (' + CAST(StepOrder AS NVARCHAR) + ') ' + StepName END AS NewValue FROM @OutRecallStepT RecallStepT INNER JOIN RecallTemplateT ON RecallStepT.RecallTemplateID = RecallTemplateT.ID ORDER BY StepOrder",
				strRecallStepName, VarLong(pRow->GetValue(eRecallStepID)));

			if(!prs->eof)
			{
				MessageBox(AdoFldString(prs, "Message"),"Practice",MB_OK|MB_ICONEXCLAMATION);
			}
			else
			{
				prs = prs->NextRecordset(NULL);

				SetActiveRecallStepID(AdoFldLong(prs, "ID"));
				m_pdlRecallSteps->Requery();

				BEGIN_AUDIT
				{
					for(; !prs->eof; prs->MoveNext()) {
						AUDIT(AdoFldString(prs, "OldValue").IsEmpty() ? aeiRecallStepCreated : aeiRecallStepOrderChanged, AdoFldLong(prs, "ID"), AdoFldString(prs, "OldValue"), AdoFldString(prs, "NewValue"), AdoFldString(prs, "OldValue").IsEmpty() ? aetCreated : aetChanged)	
					}
				}
				END_AUDIT
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-03-23 12:22) - PLID 49008 - Added Auditing
void CRecallSetupDlg::OnBnClickedDeleteRecallStep()
{
	try
	{
		IRowSettingsPtr pRow = GetActiveRecallStepRow();

		_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON\r\n"
			"DECLARE @RecallTemplateID INT\r\n"
			"DECLARE @RecallStepID INT\r\n"
			"DECLARE @StepOrder INT\r\n"
			"DECLARE @Message NVARCHAR(100)\r\n"
			"DECLARE @OutRecallStepT TABLE (ID INT, RecallTemplateID INT, StepOrder INT, OldStepOrder INT, StepName NVARCHAR(50))\r\n"
			"SET @RecallStepID = {INT}\r\n"
			"BEGIN TRAN\r\n"
			"SELECT @RecallTemplateID = RecallTemplateID, @StepOrder = StepOrder FROM RecallStepT WHERE ID = @RecallStepID\r\n"
			"IF NOT EXISTS (SELECT ID FROM RecallT WHERE RecallTemplateID = @RecallTemplateID)\r\n"
			"BEGIN\r\n"
			"	IF (SELECT COUNT(ID) FROM RecallStepT WHERE RecallTemplateID = @RecallTemplateID AND Active = 1) > 1\r\n"
			"	BEGIN\r\n"
			"		UPDATE RecallStepT\r\n"
			"			SET StepOrder = (StepOrder - 1)\r\n"
			"			OUTPUT inserted.ID, inserted.RecallTemplateID, inserted.StepOrder, deleted.StepOrder, inserted.Name INTO @OutRecallStepT\r\n"
			"			WHERE RecallTemplateID = @RecallTemplateID AND StepOrder > @StepOrder AND Active = 1\r\n"
			"		DELETE FROM RecallStepT\r\n"
			"			OUTPUT deleted.ID, deleted.RecallTemplateID, NULL, deleted.StepOrder, deleted.Name INTO @OutRecallStepT\r\n"
			"			WHERE ID = @RecallStepID\r\n"
			"	END\r\n"
			"END\r\n"
			"ELSE\r\n"
			"BEGIN\r\n"
			"	SET @Message = 'This recall template has already been used.\r\nThe steps cannot be modified.'\r\n"
			"END\r\n"
			"COMMIT TRAN\r\n"
			"SET NOCOUNT OFF\r\n"
			"SELECT @Message AS Message WHERE @Message IS NOT NULL\r\n"
			"SELECT RecallStepT.ID, CASE WHEN OldStepOrder IS NULL THEN '' ELSE RecallTemplateT.Name + ': (' + CAST(OldStepOrder AS NVARCHAR) + ') ' + StepName END AS OldValue,	CASE WHEN StepOrder IS NULL THEN '' ELSE RecallTemplateT.Name + ': (' + CAST(StepOrder AS NVARCHAR) + ') ' + StepName END AS NewValue FROM @OutRecallStepT RecallStepT INNER JOIN RecallTemplateT ON RecallStepT.RecallTemplateID = RecallTemplateT.ID ORDER BY StepOrder", 
			VarLong(pRow->GetValue(eRecallStepID)));

		if(!prs->eof)
		{
			MessageBox(AdoFldString(prs, "Message"),"Practice",MB_OK|MB_ICONEXCLAMATION);
		}
		else
		{
			prs = prs->NextRecordset(NULL);

			SetActiveRecallStepID(prs->eof ? -1 : AdoFldLong(prs, "ID"));
			m_pdlRecallSteps->Requery();

			BEGIN_AUDIT
			{
				for(; !prs->eof; prs->MoveNext()) {
					AUDIT(AdoFldString(prs, "NewValue").IsEmpty() ? aeiRecallStepDeleted : aeiRecallStepOrderChanged, AdoFldLong(prs, "ID"), AdoFldString(prs, "OldValue"), AdoFldString(prs, "NewValue"), AdoFldString(prs, "NewValue").IsEmpty() ? aetDeleted : aetChanged)
				}
			}
			END_AUDIT
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-03-23 12:22) - PLID 49008 - Added Auditing
void CRecallSetupDlg::SwapSteps(bool bMoveDown)
{
	IRowSettingsPtr pRow = GetActiveRecallStepRow();

		_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON\r\n"
			"DECLARE @RecallTemplateID INT\r\n"
			"DECLARE @SwapRecallID INT\r\n"
			"DECLARE @Direction BIT\r\n"
			"DECLARE @SwapRecallOrder INT\r\n"
			"DECLARE @Message NVARCHAR(100)\r\n"
			"DECLARE @OutRecallStepT TABLE (ID INT, RecallTemplateID INT, StepOrder INT, OldStepOrder INT, StepName NVARCHAR(50))\r\n"
			"SET @SwapRecallID = {INT}\r\n"
			"SET @Direction = {BIT}\r\n"
			"BEGIN TRAN\r\n"
			"SELECT @RecallTemplateID = RecallTemplateID, @SwapRecallOrder = StepOrder FROM RecallStepT WHERE ID = @SwapRecallID AND Active = 1\r\n"
			"IF NOT EXISTS (SELECT ID FROM RecallT WHERE RecallTemplateID = @RecallTemplateID)\r\n"
			"BEGIN\r\n"
			"	IF (@Direction = 0 AND @SwapRecallOrder > 1) OR (@Direction = 1 AND @SwapRecallOrder < (SELECT MAX(StepOrder) FROM RecallStepT WHERE RecallTemplateID = @RecallTemplateID AND Active = 1))\r\n"
			"	BEGIN\r\n"
			"		UPDATE RecallStepT\r\n"
			"			SET StepOrder = (CASE WHEN @Direction = 0 THEN (StepOrder + 1) ELSE (StepOrder - 1) END)\r\n"
			"			OUTPUT inserted.ID, inserted.RecallTemplateID, inserted.StepOrder, deleted.StepOrder, inserted.Name INTO @OutRecallStepT\r\n"
			"			WHERE RecallTemplateID = @RecallTemplateID AND StepOrder = CASE WHEN @Direction = 0 THEN (@SwapRecallOrder - 1) ELSE (@SwapRecallOrder + 1) END\r\n"
			"		UPDATE RecallStepT\r\n"
			"			SET StepOrder = (CASE WHEN @Direction = 0 THEN (StepOrder - 1) ELSE (StepOrder + 1) END)\r\n"
			"			OUTPUT inserted.ID, inserted.RecallTemplateID, inserted.StepOrder, deleted.StepOrder, inserted.Name INTO @OutRecallStepT\r\n"
			"			WHERE ID = @SwapRecallID\r\n"
			"	END\r\n"
			"END\r\n"
			"ELSE\r\n"
			"BEGIN\r\n"
			"	SET @Message = 'This recall template has already been used.\r\nThe steps cannot be modified.'\r\n"
			"END\r\n"
			"COMMIT TRAN\r\n"
			"SET NOCOUNT OFF\r\n"
			"SELECT @Message AS Message WHERE @Message IS NOT NULL\r\n"
			"SELECT RecallStepT.ID, CASE WHEN OldStepOrder IS NULL THEN '' ELSE RecallTemplateT.Name + ': (' + CAST(OldStepOrder AS NVARCHAR) + ') ' + StepName END AS OldValue,	CASE WHEN StepOrder IS NULL THEN '' ELSE RecallTemplateT.Name + ': (' + CAST(StepOrder AS NVARCHAR) + ') ' + StepName END AS NewValue FROM @OutRecallStepT RecallStepT INNER JOIN RecallTemplateT ON RecallStepT.RecallTemplateID = RecallTemplateT.ID ORDER BY StepOrder", 
			VarLong(pRow->GetValue(eRecallStepID)), bMoveDown);

		if(!prs->eof)
		{
			MessageBox(AdoFldString(prs, "Message"),"Practice",MB_OK|MB_ICONEXCLAMATION);
		}
		else
		{
			m_pdlRecallSteps->Requery();

			prs = prs->NextRecordset(NULL);
			BEGIN_AUDIT
			{
				for(; !prs->eof; prs->MoveNext()) {
					AUDIT(aeiRecallStepOrderChanged, AdoFldLong(prs, "ID"), AdoFldString(prs, "OldValue"), AdoFldString(prs, "NewValue"), aetChanged)
				}
			}
			END_AUDIT
		}
}

void CRecallSetupDlg::OnBnClickedRecallStepUp()
{
	try
	{
		SwapSteps(false);
	}NxCatchAll(__FUNCTION__);
}

void CRecallSetupDlg::OnBnClickedRecallStepDown()
{
	try
	{
		SwapSteps(true);
	}NxCatchAll(__FUNCTION__);
}

void CRecallSetupDlg::OnSelChosenRecallTemplate(LPDISPATCH lpRow) 
{
	try {
		IRowSettingsPtr pRow = GetActiveRecallTemplateRow(lpRow);

		m_btnRepeatLastStep.SetCheck(VarBool(pRow->GetValue(eRecallTemplateRepeatLastStep)) ? BST_CHECKED : BST_UNCHECKED);

		m_pdlRecallSteps->PutWhereClause(_bstr_t(CSqlFragment("RecallTemplateID = {INT} AND Active = 1", VarLong(pRow->GetValue(eRecallTemplateID))).Flatten()));
		m_pdlRecallSteps->Requery();
		m_pdlDiagCode->PutWhereClause(_bstr_t(CSqlFragment("RecallTemplateID = {INT}", VarLong(pRow->GetValue(eRecallTemplateID))).Flatten()));
		m_pdlDiagCode->Requery();
	}NxCatchAll(__FUNCTION__);
}

void CRecallSetupDlg::OnRequeryFinishedRecallTemplate(short nFlags) 
{
	try
	{
		IRowSettingsPtr pRow = GetActiveRecallTemplateRow();

		GetDlgItem(IDC_RECALL_NEW_COPY_TEMPLATE)->EnableWindow(pRow ? TRUE : FALSE);
		GetDlgItem(IDC_RECALL_DELETE_TEMPLATE)->EnableWindow(pRow ? TRUE : FALSE);
		GetDlgItem(IDC_NXDL_RECALL_TEMPLATE_SETUP)->EnableWindow(pRow ? TRUE : FALSE);
		GetDlgItem(IDC_RECALL_RENAME_TEMPLATE)->EnableWindow(pRow ? TRUE : FALSE);
		GetDlgItem(IDC_RECALL_NEW_STEP)->EnableWindow(pRow ? TRUE : FALSE);
		GetDlgItem(IDC_RECALL_DELETE_STEP)->EnableWindow(pRow ? TRUE : FALSE);
		GetDlgItem(IDC_RECALL_STEP_UP)->EnableWindow(pRow ? TRUE : FALSE);
		GetDlgItem(IDC_RECALL_STEP_DOWN)->EnableWindow(pRow ? TRUE : FALSE);
		GetDlgItem(IDC_NXDL_RECALL_STEPS)->EnableWindow(pRow ? TRUE : FALSE);
		GetDlgItem(IDC_NXDL_RECALL_DIAGNOSIS_SELECTED)->EnableWindow(pRow ? TRUE : FALSE);
		GetDlgItem(IDC_RECALL_SETUP_DIAGNOSIS_SEARCH)->EnableWindow(pRow ? TRUE : FALSE); // (a.wilson 2014-02-17 15:03) - PLID 60775
		GetDlgItem(IDC_RECALL_REPEAT_LAST_STEP)->EnableWindow(pRow ? TRUE : FALSE);

		m_btnRepeatLastStep.SetCheck(pRow && VarBool(pRow->GetValue(eRecallTemplateRepeatLastStep)) ? BST_CHECKED : BST_UNCHECKED);

		m_pdlRecallSteps->PutWhereClause(_bstr_t(CSqlFragment("RecallTemplateID = {INT} AND Active = 1", pRow ? VarLong(pRow->GetValue(eRecallTemplateID)) : - 1).Flatten()));
		m_pdlRecallSteps->Requery();
		m_pdlDiagCode->PutWhereClause(_bstr_t(CSqlFragment("RecallTemplateID = {INT}", pRow ? VarLong(pRow->GetValue(eRecallTemplateID)) : - 1).Flatten()));
		m_pdlDiagCode->Requery();
	}NxCatchAll(__FUNCTION__);
}

void CRecallSetupDlg::OnSelChosenRecallStep(LPDISPATCH lpRow) 
{
	try {
		IRowSettingsPtr pRow = GetActiveRecallStepRow(lpRow);

		GetDlgItem(IDC_NXDL_RECALL_STEPS)->EnableWindow(pRow ? TRUE : FALSE);
		GetDlgItem(IDC_RECALL_DELETE_STEP)->EnableWindow(pRow && (m_pdlRecallSteps->GetRowCount() > 1) ? TRUE : FALSE);
		GetDlgItem(IDC_RECALL_STEP_UP)->EnableWindow(pRow && (VarLong(pRow->GetValue(eRecallStepOrder)) != 1) ? TRUE : FALSE);
		GetDlgItem(IDC_RECALL_STEP_DOWN)->EnableWindow(pRow && (VarLong(pRow->GetValue(eRecallStepOrder)) != GetMaxStepOrder()) ? TRUE : FALSE);

	}NxCatchAll(__FUNCTION__);
}

void CRecallSetupDlg::OnRequeryFinishedRecallStep(short nFlags) 
{
	try
	{
		IRowSettingsPtr pRow = GetActiveRecallStepRow();

		GetDlgItem(IDC_NXDL_RECALL_STEPS)->EnableWindow(pRow ? TRUE : FALSE);
		GetDlgItem(IDC_RECALL_DELETE_STEP)->EnableWindow(pRow && (m_pdlRecallSteps->GetRowCount() > 1) ? TRUE : FALSE);
		GetDlgItem(IDC_RECALL_STEP_UP)->EnableWindow(pRow && (VarLong(pRow->GetValue(eRecallStepOrder)) != 1) ? TRUE : FALSE);
		GetDlgItem(IDC_RECALL_STEP_DOWN)->EnableWindow(pRow && (VarLong(pRow->GetValue(eRecallStepOrder)) != GetMaxStepOrder()) ? TRUE : FALSE);

	}NxCatchAll(__FUNCTION__);
}

namespace {
	bool CompareArrays(const CArray<long,long> &ar1, const CArray<long,long> &ar2)
	{
		if(ar1.GetSize() != ar2.GetSize()) return false;
		
		for(int i = 0; i < ar1.GetSize(); i++) {
			bool bMatched = false;
			for(int j = 0; j < ar2.GetSize() && !bMatched; j++) {
				if(ar1[i] == ar2[j]) bMatched = true;
			}
			if(!bMatched) return false;
		}
		return true;
	}
}

void CRecallSetupDlg::EditingFinishingNxdlRecallSteps(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		switch(nCol)
		{
			case eRecallStepName:
				*pbCommit = UpdateRecallStepName(VarLong(pRow->GetValue(eRecallStepID)), VarString(pvarNewValue), VarString(varOldValue));
				break;
			case eRecallStepDay:
				pvarNewValue->lVal = labs(VarLong(pvarNewValue));
				*pbCommit = UpdateRecallStepTime(VarLong(pRow->GetValue(eRecallStepID)), VarLong(pvarNewValue), VarLong(varOldValue), "Days");
				break;
			case eRecallStepWeek:
				pvarNewValue->lVal = labs(VarLong(pvarNewValue));
				*pbCommit = UpdateRecallStepTime(VarLong(pRow->GetValue(eRecallStepID)), VarLong(pvarNewValue), VarLong(varOldValue), "Weeks");
				break;
			case eRecallStepMonth:
				pvarNewValue->lVal = labs(VarLong(pvarNewValue));
				*pbCommit = UpdateRecallStepTime(VarLong(pRow->GetValue(eRecallStepID)), VarLong(pvarNewValue), VarLong(varOldValue), "Months");
				break;
			case eRecallStepYear:
				pvarNewValue->lVal = labs(VarLong(pvarNewValue));
				*pbCommit = UpdateRecallStepTime(VarLong(pRow->GetValue(eRecallStepID)), VarLong(pvarNewValue), VarLong(varOldValue), "Years");
				break;
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-03-23 12:22) - PLID 49008 - Added Auditing
bool CRecallSetupDlg::UpdateRecallStepName(const long& nRecallStepID, CString& strNewValue, CString& strOldValue)
{
	try
	{
		strNewValue.Trim();
		strOldValue.Trim();

		if(strNewValue.Compare(strOldValue) != 0)
		{
			if(strNewValue.IsEmpty())
			{
				MessageBox("You must enter a name for this recall step.","Practice",MB_OK|MB_ICONEXCLAMATION);
				return false;
			}

			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON\r\n"
				"DECLARE @OutRecallStepT TABLE (ID INT, RecallTemplateID INT, StepOrder INT, OldValue NVARCHAR(50), NewValue NVARCHAR(50))\r\n"
				"DECLARE @Name NVARCHAR(50)\r\n"
				"DECLARE @RecallStepID INT\r\n"
				"DECLARE @RecallTemplateID INT\r\n"
				"SET @Name = {STRING}\r\n"
				"SET @RecallStepID = {INT}\r\n"
				"BEGIN TRAN\r\n"
				"SELECT @RecallTemplateID = RecallTemplateID FROM RecallStepT WHERE ID = @RecallStepID\r\n"
				"IF NOT EXISTS (SELECT ID FROM RecallStepT WHERE Name = @Name AND Active = 1 AND RecallTemplateID = @RecallTemplateID)\r\n"
				"BEGIN\r\n"
				"	UPDATE RecallStepT\r\n"
				"		SET Name = @Name\r\n"
				"		OUTPUT inserted.ID, inserted.RecallTemplateID, inserted.StepOrder, deleted.Name, inserted.Name INTO @OutRecallStepT\r\n"
				"		WHERE ID = @RecallStepID\r\n"
				"END\r\n"
				"COMMIT TRAN\r\n"
				"SET NOCOUNT OFF\r\n"
				"SELECT RecallStepT.ID, RecallTemplateT.Name + ': (' + CAST(StepOrder AS NVARCHAR) + ') ' + OldValue AS OldValue, RecallTemplateT.Name + ': (' + CAST(StepOrder AS NVARCHAR) + ') ' + NewValue AS NewValue FROM @OutRecallStepT RecallStepT INNER JOIN RecallTemplateT ON RecallStepT.RecallTemplateID = RecallTemplateT.ID",
				strNewValue, nRecallStepID);

			if(prs->eof)
			{
				MessageBox("There is already a recall step with this name.\r\nPlease choose a new name.","Practice",MB_OK|MB_ICONEXCLAMATION);
				return false;
			}
			else
			{
				BEGIN_AUDIT
				{
					for(; !prs->eof; prs->MoveNext()) {
						AUDIT(aeiRecallStepRename, AdoFldLong(prs, "ID"), AdoFldString(prs, "OldValue"), AdoFldString(prs, "NewValue"), aetChanged)	
					}
				}
				END_AUDIT
				return true;
			}
		}
	}NxCatchAll(__FUNCTION__);
	return false;
}

// (j.armen 2012-03-23 12:22) - PLID 49008 - Added Auditing
bool CRecallSetupDlg::UpdateRecallStepTime(const long& nRecallStepID, const long& nNewValue, const long& nOldValue, const CString& strField)
{
	try
	{
		if(nNewValue != nOldValue)
		{
			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON\r\n"
				"DECLARE @OutRecallStepT TABLE (ID INT, RecallTemplateID INT, StepOrder INT, StepName NVARCHAR(50), OldDays INT, OldWeeks INT, OldMonths INT, OldYears INT, NewDays INT, NewWeeks INT, NewMonths INT, NewYears INT)\r\n"
				"BEGIN TRAN\r\n"
				"UPDATE RecallStepT\r\n"
				"	SET {CONST_STRING} = {INT}\r\n"
				"	OUTPUT\r\n"
				"		inserted.ID, inserted.RecallTemplateID, inserted.StepOrder, inserted.Name,\r\n"
				"		deleted.Days, deleted.Weeks, deleted.Months, deleted.Years,\r\n"
				"		inserted.Days, inserted.Weeks, inserted.Months, inserted.Years\r\n"
				"		INTO @OutRecallStepT\r\n"
				"	WHERE ID = {INT}\r\n"
				"COMMIT TRAN\r\n"
				"SET NOCOUNT OFF\r\n"
				"SELECT\r\n"
				"	RecallStepT.ID,\r\n"
				"	RecallTemplateT.Name + ': (' + CAST(StepOrder AS NVARCHAR) + ') ' + StepName + ' -' +\r\n"
				"		CASE WHEN NewDays <> OldDays THEN ' Days: ' + CAST(OldDays AS NVARCHAR) ELSE '' END +\r\n"
				"		CASE WHEN NewWeeks <> OldWeeks THEN ' Weeks: ' + CAST(OldWeeks AS NVARCHAR) ELSE '' END +\r\n"
				"		CASE WHEN NewMonths <> OldMonths THEN ' Months: ' + CAST(OldMonths AS NVARCHAR) ELSE '' END +\r\n"
				"		CASE WHEN NewYears <> OldYears THEN ' Years: ' + CAST(OldYears AS NVARCHAR) ELSE '' END\r\n"
				"		AS OldValue,\r\n"
				"	RecallTemplateT.Name + ': (' + CAST(StepOrder AS NVARCHAR) + ') ' + StepName + ' -' +\r\n"
				"		CASE WHEN NewDays <> OldDays THEN ' Days: ' + CAST(NewDays AS NVARCHAR) ELSE '' END +\r\n"
				"		CASE WHEN NewWeeks <> OldWeeks THEN ' Weeks: ' + CAST(NewWeeks AS NVARCHAR) ELSE '' END +\r\n"
				"		CASE WHEN NewMonths <> OldMonths THEN ' Months: ' + CAST(NewMonths AS NVARCHAR) ELSE '' END +\r\n"
				"		CASE WHEN NewYears <> OldYears THEN ' Years: ' + CAST(NewYears AS NVARCHAR) ELSE '' END\r\n"
				"		AS NewValue\r\n"
				"FROM @OutRecallStepT RecallStepT\r\n"
				"INNER JOIN RecallTemplateT ON RecallStepT.RecallTemplateID = RecallTemplateT.ID",
				strField, nNewValue, nRecallStepID);

			BEGIN_AUDIT
			{
				for(; !prs->eof; prs->MoveNext()) {
					AUDIT(aeiRecallStepDateModified, AdoFldLong(prs, "ID"), AdoFldString(prs, "OldValue"), AdoFldString(prs, "NewValue"), aetChanged)	
				}
			}
			END_AUDIT
			return true;
		}
	}NxCatchAll(__FUNCTION__);
	return false;
}

// (j.armen 2012-03-23 12:22) - PLID 49008 - Added Auditing
void CRecallSetupDlg::OnBnClickedRecallRepeatStep()
{
	try
	{
		IRowSettingsPtr pRow = GetActiveRecallTemplateRow();

		_RecordsetPtr prs = CreateParamRecordset(
			"DECLARE @RepeatLastStep BIT\r\n"
			"SET @RepeatLastStep = {BOOL}\r\n"
			"UPDATE RecallTemplateT\r\n"
			"SET RepeatLastStep = @RepeatLastStep\r\n"
			"OUTPUT inserted.ID,\r\n"
			"	deleted.Name + ': ' + CASE WHEN deleted.RepeatLastStep = 1 THEN 'Repeat Last Step' ELSE 'Do Not Repeat Last Step' END AS OldValue,\r\n"
			"	inserted.Name + ': ' + CASE WHEN inserted.RepeatLastStep = 1 THEN 'Repeat Last Step' ELSE 'Do Not Repeat Last Step' END AS NewValue\r\n"
			"WHERE ID = {INT} AND RepeatLastStep <> @RepeatLastStep",

			m_btnRepeatLastStep.GetCheck() == BST_CHECKED ? true : false, VarLong(pRow->GetValue(eRecallTemplateID)));

		pRow->PutValue(eRecallTemplateRepeatLastStep, m_btnRepeatLastStep.GetCheck() == BST_CHECKED ? g_cvarTrue : g_cvarFalse);

		BEGIN_AUDIT
		{
			for(; !prs->eof; prs->MoveNext()) {
				AUDIT(aeiRecallTemplateRepeatLastStepModified, AdoFldLong(prs, "ID"), AdoFldString(prs, "OldValue"), AdoFldString(prs, "NewValue"), aetChanged)	
			}
		}
		END_AUDIT

	}NxCatchAll(__FUNCTION__);
}

IRowSettingsPtr CRecallSetupDlg::GetActiveRecallTemplateRow(LPDISPATCH lpRow)
{
	IRowSettingsPtr pRow(lpRow);

	if(pRow)
		m_pdlRecallTemplate->PutCurSel(pRow);
	else
		pRow = m_pdlRecallTemplate->SetSelByColumn(eRecallTemplateID, m_nRecallTemplateID);

	if(!pRow)
	{
		pRow = m_pdlRecallTemplate->GetCurSel();
		m_pdlRecallTemplate->PutCurSel(pRow);
	}

	if(!pRow)
	{
		pRow = m_pdlRecallTemplate->GetTopRow();
		m_pdlRecallTemplate->PutCurSel(pRow);
	}

	m_nRecallTemplateID = pRow ? VarLong(pRow->GetValue(eRecallTemplateID)) : - 1;
	
	return pRow;
}

IRowSettingsPtr CRecallSetupDlg::GetActiveRecallStepRow(LPDISPATCH lpRow)
{
	IRowSettingsPtr pRow(lpRow);

	if(pRow)
		m_pdlRecallSteps->PutCurSel(pRow);
	else
		pRow = m_pdlRecallSteps->SetSelByColumn(eRecallStepID, m_nRecallStepID);

	if(!pRow)
	{
		pRow = m_pdlRecallSteps->GetCurSel();
		m_pdlRecallSteps->PutCurSel(pRow);
	}

	if(!pRow)
	{
		pRow = m_pdlRecallSteps->GetLastRow();
		m_pdlRecallSteps->PutCurSel(pRow);
	}

	m_nRecallStepID = pRow ? VarLong(pRow->GetValue(eRecallStepID)) : - 1;
	
	return pRow;
}

void CRecallSetupDlg::SetActiveRecallTemplateID(const long& nRecallTemplateID)
{
	m_nRecallTemplateID = nRecallTemplateID;
}

void CRecallSetupDlg::SetActiveRecallStepID(const long& nRecallStepID)
{
	m_nRecallStepID = nRecallStepID;
}

long CRecallSetupDlg::GetMaxStepOrder()
{
	long nMaxStepOrder = -1;
	for(IRowSettingsPtr pRow = m_pdlRecallSteps->GetFirstRow(); pRow; pRow = pRow->GetNextRow())
	{
		long nStepOrder = VarLong(pRow->GetValue(eRecallStepOrder));
		if(nStepOrder > nMaxStepOrder)
			nMaxStepOrder = nStepOrder;
	}
	return nMaxStepOrder;
}

// (a.wilson 2014-02-17 15:03) - PLID 60775 - add a diagnosis code.
void CRecallSetupDlg::SelChosenRecallSetupDiagnosisSearch(LPDISPATCH lpRow)
{
	long nAuditTransactionID = -1;
	try {
		if (lpRow) {
			long nDiagID = -1;
			CString strDiagCode, strDiagDescription;
			CDiagSearchResults results = DiagSearchUtils::ConvertDualSearchResults(lpRow);
			//Get the code that was selected, it could be either a 9 or 10 code not both.
			if (results.m_ICD9.m_nDiagCodesID != -1) {
				nDiagID = results.m_ICD9.m_nDiagCodesID;
				strDiagCode = results.m_ICD9.m_strCode;
				strDiagDescription = results.m_ICD9.m_strDescription;
			} else if (results.m_ICD10.m_nDiagCodesID != -1) {
				nDiagID = results.m_ICD10.m_nDiagCodesID;
				strDiagCode = results.m_ICD10.m_strCode;
				strDiagDescription = results.m_ICD10.m_strDescription;
			} else {
				//no code was selected, just return.
				return;
			}

			//check if the code is already on the template. if it is then do not continue;
			for(IRowSettingsPtr pDiagRow = m_pdlDiagCode->GetTopRow(); pDiagRow; pDiagRow = pDiagRow->GetNextRow()) {
				if (VarLong(pDiagRow->GetValue(eDiagCodeID)) == nDiagID) {
					return;
				}
			}
			//get template id for insert.
			IRowSettingsPtr pTemplateRow = GetActiveRecallTemplateRow();
			long nTemplateID = VarLong(pTemplateRow->GetValue(eRecallTemplateID));
			CString strTemplateName = VarString(pTemplateRow->GetValue(eRecallTemplateName));
			//Insert record into database.
			_RecordsetPtr prs = CreateParamRecordset(
				"INSERT INTO RecallTemplateDiagLinkT (RecallTemplateID, DiagCodesID) \r\n"
				"VALUES ({INT}, {INT}) ", nTemplateID, nDiagID);
			//Audit Event
			nAuditTransactionID = BeginAuditTransaction();
			AuditEvent(-1, "", nAuditTransactionID, aeiRecallDiagLinkAdded, nTemplateID, "", 
				FormatString("%s: %s", strTemplateName, strDiagCode), aepMedium, aetCreated);
			CommitAuditTransaction(nAuditTransactionID);
			//add record to diagnosis list.
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pdlDiagCode->GetNewRow();
			if (pNewRow) {
				pNewRow->PutValue(eDiagCodeID, nDiagID);
				pNewRow->PutValue(eDiagCode, _bstr_t(strDiagCode));
				pNewRow->PutValue(eDiagCodeDescription, _bstr_t(strDiagDescription));
				m_pdlDiagCode->AddRowSorted(pNewRow, NULL);
			} else {
				m_pdlDiagCode->Requery();
			}
		}
	}NxCatchAllCall(__FUNCTION__,
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	)
}

// (a.wilson 2014-02-17 15:03) - PLID 60869 - attempt to remove diag code.
void CRecallSetupDlg::RButtonDownNxdlRecallDiagCodeSelected(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	long nAuditTransactionID = -1;
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			m_pdlDiagCode->PutCurSel(pRow);

			int nCmdID = 0;
			CNxMenu mnu;
			mnu.m_hMenu = CreatePopupMenu();
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, 1, "Remove Diagnosis Code");

			CPoint pt;
			GetCursorPos(&pt);
			nCmdID = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);

			//Remove Selection.
			if (nCmdID == 1) {
				//get template id for removal.
				IRowSettingsPtr pTemplateRow = GetActiveRecallTemplateRow();
				long nTemplateID = VarLong(pTemplateRow->GetValue(eRecallTemplateID));
				CString strTemplateName = VarString(pTemplateRow->GetValue(eRecallTemplateName));
				long nDiagCodeID = VarLong(pRow->GetValue(eDiagCodeID));
				CString strDiagCode = VarString(pRow->GetValue(eDiagCode));

				//remove from database
				_RecordsetPtr prs = CreateParamRecordset(
					"DELETE FROM RecallTemplateDiagLinkT \r\n"
					"WHERE RecallTemplateID = {INT} AND DiagCodesID = {INT} ", 
					nTemplateID, nDiagCodeID);
				//Audit Event
				nAuditTransactionID = BeginAuditTransaction();
				AuditEvent(-1, "", nAuditTransactionID, aeiRecallDiagLinkRemoved, nTemplateID, 
					FormatString("%s: %s", strTemplateName, strDiagCode), "", aepMedium, aetDeleted);
				CommitAuditTransaction(nAuditTransactionID);
				//remove from list
				if (m_pdlDiagCode->RemoveRow(pRow) == VARIANT_FALSE) {
					m_pdlDiagCode->Requery();
				}
			}
		}
	}NxCatchAllCall(__FUNCTION__,
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	)
}
