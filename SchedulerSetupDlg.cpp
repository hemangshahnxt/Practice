// SchedulerSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SchedulerSetupDlg.h"
#include "ResourceOrderDlg.h"
#include "TemplatesDlg.h"
#include "GetNewIDName.h"
#include "client.h"
#include "globalDataUtils.h"
#include "phaseTracking.h"
#include "color.h"
#include "marketUtils.h"
#include "DontShowDlg.h"
#include "ProcedureReplaceDlg.h"
#include "AdministratorRc.h"
#include "AuditTrail.h"
#include "globalutils.h"
#include "BlockTimeWarningDlg.h"
#include "SchedulerView.h"
#include "SchedulerSetupDlg.h"
#include "SchedulerStatusDlg.h"
#include "SchedulerDurationDlg.h"
#include "InactiveTypesDlg.h"
#include "PreferenceUtils.h"
#include "GlobalDrawingUtils.h"
#include "GlobalSchedUtils.h"
#include "TemplateItemEntryGraphicalDlg.h"
#include "SchedCountSetupDlg.h"
#include "ChildFrm.h"
#include "EditResourceSetDlg.h"
#include "MultiSelectDlg.h"
#include "ApptTypeCodeLinkDlg.h" // (j.gruber 2010-07-20 14:05) - PLID 30481
#include "RecallSetupDlg.h"	// (j.armen 2012-02-24 15:54) - PLID 48304
#include "ListMergeDlg.h"
#include "SchedulingMixRulesDLG.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace PhaseTracking;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



typedef enum
{
	PC_ID = 0,
	PC_NAME,
	PC_COLOR,
	PC_CATEGORY,
	PC_DEFAULTDURATION,
	PC_DEFAULTARRIVALMINS
} P_COLUMN;
/////////////////////////////////////////////////////////////////////////////
// CSchedulerSetupDlg dialog


CSchedulerSetupDlg::CSchedulerSetupDlg(CWnd* pParent)
	: CNxDialog(CSchedulerSetupDlg::IDD, pParent),
	m_purposeChecker(NetUtils::AptPurposeT),
	m_typeChecker(NetUtils::AptTypeT), 
	m_purposeTypeChecker(NetUtils::AptPurposeTypeT)
{
	//{{AFX_DATA_INIT(CSchedulerSetupDlg)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "System_Setup/Scheduler_Setup/Setup_Appointment_Purposes.htm";

	m_bNeedToSaveDuration = FALSE;
	m_bNeedToSaveArrivalMins = FALSE;
}


void CSchedulerSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSchedulerSetupDlg)
	DDX_Control(pDX, IDC_INACTIVATE_TYPE, m_btnInactivateType);
	DDX_Control(pDX, IDC_INACTIVE_TYPES, m_btnInactiveTypes);
	DDX_Control(pDX, IDC_SCHEDULER_DURATION, m_btnDuration);
	DDX_Control(pDX, IDC_APPTBOOKINGALARMS, m_btnBookingAlarms);
	DDX_Control(pDX, IDC_TYPE_COLOR, m_typeColor);
	DDX_Control(pDX, IDC_COLOR_PICKER_CTRL, m_ctrlColorPicker);
	DDX_Control(pDX, IDC_NEW_TYPE, m_btnNewType);
	DDX_Control(pDX, IDC_DELETE_TYPE, m_btnDeleteType);
	DDX_Control(pDX, IDC_RENAME_TYPE, m_btnRenameType);
	DDX_Control(pDX, IDC_NEW_PURPOSE, m_btnNewPurpose);
	DDX_Control(pDX, IDC_DELETE_PURPOSE, m_btnDeletePurpose);
	DDX_Control(pDX, IDC_RENAME_PURPOSE, m_btnRenamePurpose);
	DDX_Control(pDX, IDC_SELECT_ALL, m_btnSelectAll);
	DDX_Control(pDX, IDC_DESELECT_ALL, m_btnDeselectAll);
	DDX_Control(pDX, IDC_RESOURCE, m_btnResource);
	DDX_Control(pDX, IDC_RESOURCE, m_btnResource);
	DDX_Control(pDX, IDC_TEMPLATES, m_btnTemplates);
	DDX_Control(pDX, IDC_ADMIN_TEMPLATE_COLLECTIONS, m_btnTemplateCollections);
	DDX_Control(pDX, IDC_REPLACE, m_btnReplace);
	DDX_Control(pDX, IDC_APPTSTATUSES, m_btnStatus);
	DDX_Control(pDX, IDC_BTN_SCHEDPREF, m_btnPreferences);
	DDX_Control(pDX, IDC_EDIT_DEFAULT_ARRIVAL_MINS, m_nxeditEditDefaultArrivalMins);
	DDX_Control(pDX, IDC_EDIT_DEFAULT_TYPE_DURATION, m_nxeditEditDefaultTypeDuration);
	DDX_Control(pDX, IDC_DEFAULT_ARRIVAL_TIME_LABEL, m_nxstaticDefaultArrivalTimeLabel);
	DDX_Control(pDX, IDC_DEFAULT_ARRIVAL_TIME_LABEL2, m_nxstaticDefaultArrivalTimeLabel2);
	DDX_Control(pDX, IDC_DEFAULT_DURATION_LABEL, m_nxstaticDefaultDurationLabel);
	DDX_Control(pDX, IDC_COPY_TO, m_btnCopyTo);
	DDX_Control(pDX, IDC_MERGE_INTO, m_btnMergeInto);
	DDX_Control(pDX, IDC_CONF_TYPE_CODE_LINK, m_btnCodeLink);
	DDX_Control(pDX, IDC_RECALL_SETUP_BUTTON, m_btnRecallSetup);	// (j.armen 2012-02-24 15:54) - PLID 48304
	DDX_Control(pDX, IDC_BTN_MERGE_RESOURCES, m_btnMergeResources);
	DDX_Control(pDX, IDC_BTN_SCHEDULE_MIX_RULES, m_btnScheduleMixRules);// (s.tullis 2014-12-08 15:49) - PLID 64125 
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSchedulerSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSchedulerSetupDlg)
	ON_BN_CLICKED(IDC_RESOURCE, OnResource)
	ON_BN_CLICKED(IDC_TEMPLATES, OnTemplates)
	ON_BN_CLICKED(IDC_ADMIN_TEMPLATE_COLLECTIONS, OnTemplateCollections)
	ON_BN_CLICKED(IDC_NEW_PURPOSE, OnNewPurpose)
	ON_BN_CLICKED(IDC_NEW_TYPE, OnNewType)
	ON_BN_CLICKED(IDC_DELETE_PURPOSE, OnDeletePurpose)
	ON_BN_CLICKED(IDC_DELETE_TYPE, OnDeleteType)
	ON_BN_CLICKED(IDC_REPLACE, OnReplace)
	ON_BN_CLICKED(IDC_RENAME_TYPE, OnRenameType)
	ON_BN_CLICKED(IDC_RENAME_PURPOSE, OnRenamePurpose)
	ON_BN_CLICKED(IDC_SELECT_ALL, OnSelectAll)
	ON_BN_CLICKED(IDC_DESELECT_ALL, OnDeselectAll)
	ON_BN_CLICKED(IDC_BTN_SCHEDPREF, OnPreferences)
	ON_BN_CLICKED(IDC_COPY_TO, OnCopyTo)
	ON_BN_CLICKED(IDC_MERGE_INTO, OnMergeInto)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_SCHEDULER_DURATION, OnSchedulerDuration)
	ON_BN_CLICKED(IDC_APPTBOOKINGALARMS, OnBookingAlarms)
	ON_BN_CLICKED(IDC_APPTSTATUSES, OnEditStatuses)
	ON_EN_CHANGE(IDC_EDIT_DEFAULT_TYPE_DURATION, OnChangeEditDefaultTypeDuration)
	ON_EN_KILLFOCUS(IDC_EDIT_DEFAULT_TYPE_DURATION, OnKillfocusEditDefaultTypeDuration)
	ON_EN_CHANGE(IDC_EDIT_DEFAULT_ARRIVAL_MINS, OnChangeEditDefaultArrivalMins)
	ON_EN_KILLFOCUS(IDC_EDIT_DEFAULT_ARRIVAL_MINS, OnKillfocusEditDefaultArrivalMins)
	ON_BN_CLICKED(IDC_INACTIVATE_TYPE, OnInactivateType)
	ON_BN_CLICKED(IDC_INACTIVE_TYPES, OnInactiveTypes)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	//}}AFX_MSG_MAP
	ON_EN_SETFOCUS(IDC_EDIT_DEFAULT_TYPE_DURATION, &CSchedulerSetupDlg::OnSetfocusEditDefaultTypeDuration)
	ON_BN_CLICKED(IDC_CONF_TYPE_CODE_LINK, &CSchedulerSetupDlg::OnBnClickedConfTypeCodeLink)
	ON_BN_CLICKED(IDC_RECALL_SETUP_BUTTON, OnBnClickedRecallSetup) // (j.armen 2012-02-24 15:54) - PLID 48304
	ON_BN_CLICKED(IDC_BTN_MERGE_RESOURCES, OnBtnMergeResources)
	ON_BN_CLICKED(IDC_BTN_SCHEDULE_MIX_RULES, OnBtnScheduleMixRules )
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CSchedulerSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSchedulerSetupDlg)
	ON_EVENT(CSchedulerSetupDlg, IDC_TYPE_COLOR, -600 /* Click */, OnClickTypeColor, VTS_NONE)
	ON_EVENT(CSchedulerSetupDlg, IDC_CATEGORY_COMBO, 16 /* SelChosen */, OnSelChosenCategoryCombo, VTS_I4)
	ON_EVENT(CSchedulerSetupDlg, IDC_PURPOSE_COMBO, 16 /* SelChosen */, OnSelChosenPurposeCombo, VTS_I4)
	ON_EVENT(CSchedulerSetupDlg, IDC_TYPE_COMBO, 16 /* SelChosen */, OnSelChosenTypeCombo, VTS_I4)
	ON_EVENT(CSchedulerSetupDlg, IDC_PROCEDURE_LIST, 10 /* EditingFinished */, OnEditingFinishedProcedureList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CSchedulerSetupDlg, IDC_PROCEDURE_LIST, 5 /* LButtonUp */, OnLButtonUpProcedureList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CSchedulerSetupDlg, IDC_TYPE_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedTypeCombo, VTS_I2)
	ON_EVENT(CSchedulerSetupDlg, IDC_PROCEDURE_LIST, 18 /* RequeryFinished */, OnRequeryFinishedProcedureList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CSchedulerSetupDlg, IDC_PROCEDURE_LIST, 6, CSchedulerSetupDlg::RButtonDownProcedureList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CSchedulerSetupDlg, IDC_PROCEDURE_LIST, 7, CSchedulerSetupDlg::RButtonUpProcedureList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()


void CSchedulerSetupDlg::AddCategory(short id, LPCSTR str)
{
	//This does NOT add a AptType category to the data
	//AptType categories MUST stay hard coded

	//this just adds the hard coded row to the combo box
	IRowSettingsPtr pRow = m_categoryCombo->GetRow(-1);
	pRow->Value[0] = id;
	pRow->Value[1] = _bstr_t(str);
	m_categoryCombo->AddRow(pRow);
}

BOOL CSchedulerSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_procedureList	= BindNxDataListCtrl(IDC_PROCEDURE_LIST, false);
	m_typeCombo		= BindNxDataListCtrl(IDC_TYPE_COMBO);
	m_purposeCombo	= BindNxDataListCtrl(IDC_PURPOSE_COMBO);

	m_categoryCombo = BindNxDataListCtrl(IDC_CATEGORY_COMBO, false);
	AddCategory(AC_NON_PROCEDURAL,	"Non Procedural");
	AddCategory(AC_CONSULT,			"Consult");
	AddCategory(AC_PREOP,			"PreOp");
	AddCategory(AC_MINOR,			"Minor Procedure");
	AddCategory(AC_SURGERY,			"Surgery");
	AddCategory(AC_FOLLOW_UP,		"Follow-Up");
	AddCategory(AC_OTHER,			"Other Procedural");
	AddCategory(AC_BLOCK_TIME,		"Block Time");

	DWORD color = GetNxColor(GNC_ADMIN, -1);

	m_typeColor.SetBackColor(color);

	m_btnNewType.AutoSet(NXB_NEW);
	m_btnDeleteType.AutoSet(NXB_DELETE);
	m_btnRenameType.AutoSet(NXB_MODIFY);
	m_btnNewPurpose.AutoSet(NXB_NEW);
	m_btnDeletePurpose.AutoSet(NXB_DELETE);
	m_btnRenamePurpose.AutoSet(NXB_MODIFY);
	// (z.manning, 04/25/2008) - PLID 29566 - Added style for inactivate button
	m_btnInactivateType.AutoSet(NXB_MODIFY);
	m_btnSelectAll.AutoSet(NXB_MODIFY);
	m_btnDeselectAll.AutoSet(NXB_MODIFY);
	m_btnResource.AutoSet(NXB_MODIFY);
	m_btnTemplates.AutoSet(NXB_MODIFY);
	m_btnTemplateCollections.AutoSet(NXB_MODIFY); // (z.manning 2014-12-03 09:52) - PLID 64205
	m_btnReplace.AutoSet(NXB_MODIFY);
	m_btnStatus.AutoSet(NXB_MODIFY);
	m_btnPreferences.AutoSet(NXB_MODIFY);
	m_btnInactiveTypes.AutoSet(NXB_MODIFY);
	m_btnBookingAlarms.AutoSet(NXB_MODIFY);
	m_btnDuration.AutoSet(NXB_MODIFY);
	// (j.gruber 2010-07-20 14:08) - PLID 30481 - Code link button
	m_btnCodeLink.AutoSet(NXB_MODIFY);

	m_btnCopyTo.AutoSet(NXB_MODIFY);
	m_btnMergeInto.AutoSet(NXB_MODIFY);

	m_btnRecallSetup.AutoSet(NXB_MODIFY); // (j.armen 2012-02-24 15:54) - PLID 48304

	// (j.armen 2012-03-28 09:12) - PLID 48480 - disable the button if the user doesn't have the license
	m_btnRecallSetup.EnableWindow((g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent)) ? TRUE : FALSE);

	// (j.jones 2012-04-10 15:15) - PLID 44174 - added ability to merge resources
	m_btnMergeResources.AutoSet(NXB_MODIFY);

	// (s.tullis 2014-12-02 10:51) - PLID 64125 - added scheduler mix rules
	m_btnScheduleMixRules.AutoSet(NXB_MODIFY);

	m_typeCombo->CurSel = 0;
	m_purposeCombo->CurSel = 0;

	m_brush.CreateSolidBrush(PaletteColor(GetNxColor(GNC_ADMIN, 0)));


	//JMJ - removed 11/10/2003
	///if (!IsSurgeryCenter())
	//{
	//	GetDlgItem(IDC_SCHEDULER_DURATION)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_DEFAULT_DURATION_LABEL)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_EDIT_DEFAULT_TYPE_DURATION)->ShowWindow(SW_HIDE);
	//}
	return TRUE;
}

void CSchedulerSetupDlg::OnResource() 
{
	//DRT 4/30/03 - Requires sptRead permission to look at the current view
	if (!CheckCurrentUserPermissions(bioSchedResourceOrder, sptRead)) 
		return;

	CResourceOrderDlg dlg(this);
	dlg.m_nCurResourceViewID = CSchedulerView::srvCurrentUserDefaultView;
	dlg.DoModal();
	CClient::RefreshTable(NetUtils::Resources);

	// Now if the scheduler is open, ask it to reload whatever its current view is, just in case
	CSchedulerView *pExistingSchedView = (CSchedulerView *)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);
	if (pExistingSchedView) {
		pExistingSchedView->OnReloadCurrentResourceList();
	}
}

void CSchedulerSetupDlg::OnTemplates() 
{
	if(!CheckCurrentUserPermissions(bioSchedTemplating, sptRead)) {
		return;
	}

	// (z.manning, 12/05/2006) - PLID 7555 - If the schedule is open, set the resource that will
	// be selected when opening the template editor.
	// (z.manning 2014-12-01 16:34) - PLID 64205 - Pass in the dialog type
	CTemplateItemEntryGraphicalDlg dlgTemplateEditor(stetNormal, this);
	try {
		CChildFrame *frmSched = GetMainFrame()->GetOpenViewFrame(SCHEDULER_MODULE_NAME);
		if(frmSched) {
			CSchedulerView* viewSched = (CSchedulerView*)(frmSched->GetActiveView());
			// (j.jones 2012-08-08 10:11) - PLID 51063 - added check that the active view is non-null
			if(viewSched) {
				dlgTemplateEditor.SetDefaultResourceID(viewSched->GetActiveResourceID());
			}
		}
	}NxCatchAllIgnore();

	dlgTemplateEditor.DoModal();
}

// (z.manning 2014-12-03 10:11) - PLID 64205
void CSchedulerSetupDlg::OnTemplateCollections() 
{
	try
	{
		if (GetMainFrame() != NULL) {
			GetMainFrame()->OpenSchedulerTemplateCollectionEditor();
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CSchedulerSetupDlg::OnNewPurpose() 
{
	CString name;
	CGetNewIDName dlg(this);
	dlg.m_pNewName = &name;
	dlg.m_strCaption = "Enter a new non-procedural purpose name";
	dlg.m_nMaxLength = 100;

	if (IDOK != dlg.DoModal())
		return;

	name.TrimRight();

	while(name == ""){		//can't enter an empty name!
		AfxMessageBox("You must enter a name for this appointment purpose.");
		if (IDOK != dlg.DoModal())
			return;
		name.TrimRight();
	}

	try
	{
		//first check for this apt. purpose already
		// (c.haag 2008-12-17 17:21) - PLID 32376 - The purpose may have been inactivated
		_RecordsetPtr prs = CreateParamRecordset("SELECT Inactive FROM AptPurposeT "
			"LEFT JOIN ProcedureT ON ProcedureT.ID = AptPurposeT.ID "
			"WHERE AptPurposeT.Name = {STRING} ", name);
		if (!prs->eof) {
			if (AdoFldBool(prs, "Inactive", FALSE)) {
				// The appointment purpose name is used by an inactive procedure
				MessageBox("This name is already in use by an inactive procedure. Please choose a new name.","Practice",MB_OK|MB_ICONEXCLAMATION);
			} else {
				MessageBox("This appointment purpose already exists. Please choose a new name.","Practice",MB_OK|MB_ICONEXCLAMATION);
			}
			return;
		}

		long id = NewNumber("AptPurposeT", "ID");
		ExecuteSql("INSERT INTO AptPurposeT (ID, Name) SELECT %i, '%s'", id, _Q(name));

		//auditing
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, "", nAuditID, aeiSchedPurposeCreated, id, "", name, aepMedium, aetCreated);

		m_purposeCombo->Requery();
		m_purposeCombo->SetSelByColumn(0, id);
		m_purposeChecker.Refresh(id);
		UpdateView();
	}
	NxCatchAll("Could not create purpose");
}

void CSchedulerSetupDlg::OnNewType() 
{
	CString name;
	CGetNewIDName dlg(this);
	dlg.m_pNewName = &name;
	dlg.m_strCaption = "Enter a new appointment type name";
	dlg.m_nMaxLength = 50;

	if (IDOK != dlg.DoModal())
		return;

	name.TrimRight();

	while(name == ""){		//can't enter an empty name!
		AfxMessageBox("You must enter a name for this appointment type.");
		if (IDOK != dlg.DoModal())
			return;
		name.TrimRight();
	}
	
	try
	{
		//first check for this apt. type already
		if(!IsRecordsetEmpty("SELECT ID FROM AptTypeT WHERE Name = '%s'",_Q(name))) {
			MessageBox("This appointment type already exists. Please choose a new name.","Practice",MB_OK|MB_ICONEXCLAMATION);
			return;
		}

		long id = NewNumber("AptTypeT", "ID");
		ExecuteSql("INSERT INTO AptTypeT (ID, Name) SELECT %i, '%s'", id, _Q(name));

		//auditing
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, "", nAuditID, aeiSchedTypeCreated, id, "", name, aepMedium, aetCreated);

		m_typeChecker.Refresh(id);
		m_typeCombo->Requery();
		m_typeCombo->SetSelByColumn(0, id);
		UpdateView();
	}
	NxCatchAll("Could not create type");
}

bool CSchedulerSetupDlg::GetPurposeID(long &id)
{
	long curSel = m_purposeCombo->CurSel;
	if (curSel == -1)
		return false;
	id = VarLong(m_purposeCombo->Value[curSel][0]);
	return true;
}

bool CSchedulerSetupDlg::GetTypeID(long &id)
{
	long curSel = m_typeCombo->CurSel;
	if (curSel == -1)
		return false;
	id = VarLong(m_typeCombo->Value[curSel][0]);
	return true;
}

void CSchedulerSetupDlg::OnDeletePurpose() 
{
	long id;

	if (!GetPurposeID(id))
		return;

	if(AfxMessageBox("Are you sure you wish to delete this purpose?", MB_YESNO) == IDNO)
		return;

	try
	{
		_RecordsetPtr rs = CreateRecordset("SELECT TOP 1 PurposeID "
			"FROM AppointmentPurposeT WHERE PurposeID = %i", id);
		if (!rs->eof)
		{	AfxMessageBox("You may not delete a non-procedural purpose that has been scheduled");
			return;
		}

		// (d.moore 2007-10-24) - PLID 4013 - Prevent deleting the purpose if it is in use in the waiting list. Otherwise and error will occur.
		rs = CreateRecordset("SELECT TOP 1 ID FROM WaitingListPurposeT WHERE PurposeID = %li", id);
		if (!rs->eof)
		{	AfxMessageBox("You may not delete a purpose that has been used in the waiting list.");
			return;
		}

		// (b.cardillo 2011-02-26 11:01) - This should have been closed or just put in its own code block, but wasn't.
		rs->Close();

		// (b.cardillo 2011-02-26 10:15) - PLID 40419 - Check for appointment prototypes that reference this aptpurpose
		{
			// Get the list of prototypes, if any, that depend on this aptpurpose
			CString strReferencedPrototypes = GenerateDelimitedListFromRecordsetColumn(CreateParamRecordset(
					_T("SELECT Name FROM ApptPrototypeT WHERE ID IN (\r\n")
					_T(" SELECT PS.ApptPrototypeID \r\n")
					_T(" FROM ApptPrototypePropertySetT PS \r\n")
					_T(" INNER JOIN ApptPrototypePropertySetAptPurposeSetT PSAPS ON PS.ID = PSAPS.ApptPrototypePropertySetID \r\n")
					_T(" INNER JOIN ApptPrototypePropertySetAptPurposeSetDetailT PSAPSD ON PSAPS.ID = PSAPSD.ApptPrototypePropertySetAptPurposeSetID \r\n")
					_T(" WHERE PSAPSD.AptPurposeID = {INT} \r\n")
					_T(") \r\n")
					_T("ORDER BY Name \r\n")
					, id)
				, AsVariant("Name"), "", "\r\n");
			if (!strReferencedPrototypes.IsEmpty()) {
				AfxMessageBox(FormatString(
					_T("You may not delete this purpose because it is referenced by the following Appointment Prototypes:\r\n\r\n%s")
					, strReferencedPrototypes));
				return;
			}
		}

		// (b.spivey - February 4th, 2014) - PLID 60563 - If we have default durations, warn before deleting. This 
		//	 case should be impossible but better safe than sorry. 
		if (ReturnsRecordsParam("SELECT * FROM ProviderSchedDefDurationDetailT WHERE AptPurposeID = {INT} ", id)
			&& AfxMessageBox("Practice has detected default duration sets associated with this appointment purpose. "
			"If you delete this appointment purpose then any default durations associated with it will be removed as well. Do you still wish to delete this data?", MB_YESNO|MB_ICONWARNING) != IDYES) {
			return;
		}
		// (s.tullis 2014-12-12 08:41) - PLID 64440 
		if (ReturnsRecordsParam("Select ID FROM ScheduleMixRuleDetailsT WHERE AptPurposeID = { INT }", id))
		{
			AfxMessageBox("You may not delete this purpose because it is associated with a Scheduling Mix Rule template ");
			return;
		}
		// (s.tullis 2015-07-08 11:17) - PLID 63851 - Need to check and warn Template Rules
		if (!CheckWarnTemplateRuleDetails(FALSE, id))
		{
			return;
		}
		
		CString strOld = CString(m_purposeCombo->GetValue(m_purposeCombo->CurSel, 1).bstrVal);
		// (s.tullis 2015-07-08 11:17) - PLID 63851 - Remove the Detail from the rule
		ExecuteParamSql("DELETE FROM TemplateRuleDetailsT WHERE (ObjectType =2 OR ObjectType = 102 ) AND ObjectID = {INT}", id);
		ExecuteSql("DELETE FROM AptPurposeTypeT WHERE AptPurposeID = %i", id);
		ExecuteSql("DELETE FROM ResourcePurposeTypeT WHERE AptPurposeID = %i", id);
		// (b.spivey - February 4th, 2014) - PLID 60563 - If we have default durations, delete the whole set. Not just parts of it.
		ExecuteParamSql(
					"DECLARE @DelTable TABLE "
					"( "
					"	DefaultDurationSetID INT NOT NULL "
					") "
					"	"
					"INSERT INTO @DelTable "
					"SELECT ProviderSchedDefDurationID FROM ProviderSchedDefDurationDetailT WHERE AptPurposeID = {INT} "
					"	"
					"DELETE Del "
					"FROM ProviderSchedDefDurationDetailT Del "
					"INNER JOIN @DelTable DelT ON Del.ProviderSchedDefDurationID = DelT.DefaultDurationSetID "
					"	"
					"DELETE Del "
					"FROM ProviderSchedDefDurationT Del "
					"INNER JOIN @DelTable DelT ON Del.ID = DelT.DefaultDurationSetID ", id);
		ExecuteSql("DELETE FROM AptBookAlarmDetailsT WHERE AptPurposeID = %li", id);
		//TES 6/12/2008 - PLID 28078 - Since we're deleting this purpose, we're not going to require allocations for it any more.
		ExecuteSql("DELETE FROM ApptsRequiringAllocationsDetailT WHERE AptPurposeID = %d", id);
		// (a.walling 2010-06-15 15:49) - PLID 39184 - Clear our any resource set links using this purpose
		ExecuteSql("DELETE FROM AptResourceSetLinksT WHERE AptPurposeID = %d", id);
		ExecuteSql("DELETE FROM AptPurposeT WHERE ID = %i", id);
	
		//auditing
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, "", nAuditID, aeiSchedPurposeDeleted, id, strOld, "<Deleted>", aepMedium, aetDeleted);

		m_purposeChecker.Refresh(id);

		m_purposeCombo->Requery();
		m_purposeCombo->CurSel = 0;
		UpdateView();
	}
	NxCatchAll("Could not delete purpose");
}

void CSchedulerSetupDlg::OnClickTypeColor() 
{
	long id;

	if (!GetTypeID(id))
		return;

	CColor color = VarLong(m_typeCombo->Value[m_typeCombo->CurSel][PC_COLOR]);

//	CColorDialog dlg(color, CC_FULLOPEN | CC_RGBINIT);
//	if (IDCANCEL == dlg.DoModal())
//		return;
//	color = dlg.GetColor();

	m_ctrlColorPicker.SetColor(color);
	m_ctrlColorPicker.ShowColor();
	color = m_ctrlColorPicker.GetColor();

	/*
	if (m_suggested.GetCheck())
	{
		color.SetV(192);
	}
	*/

	try
	{	
		ExecuteSql("UPDATE AptTypeT SET Color = %i WHERE ID = %i", color, id);

		m_typeChecker.Refresh(id);

		m_typeColor.SetColor(color);
		m_typeCombo->Value[m_typeCombo->CurSel][PC_COLOR] = color;
		OnRequeryFinishedTypeCombo(-1);
		return;
	}
	NxCatchAll("Could not change color");
	Refresh();
}

void CSchedulerSetupDlg::Refresh()
{
	try {

		// (j.jones 2014-08-07 16:40) - PLID 63232 - these TCs needed to be checked in Refresh()
		long id, curSel;

		if (m_purposeChecker.Changed())
		{
			curSel = m_purposeCombo->CurSel;

			if (curSel != -1)
			{
				id = VarLong(m_purposeCombo->Value[curSel][0]);
				m_purposeCombo->Requery();
				m_purposeCombo->SetSelByColumn(0, id);
			}
			else m_purposeCombo->Requery();

			OnSelChosenTypeCombo(m_typeCombo->CurSel);
		}

		if (m_typeChecker.Changed())
		{
			curSel = m_typeCombo->CurSel;

			if (curSel != -1)
			{
				id = VarLong(m_typeCombo->Value[curSel][0]);
				m_typeCombo->Requery();
				m_typeCombo->SetSelByColumn(0, id);
			}
			else m_typeCombo->Requery();
		}

		OnSelChosenPurposeCombo(m_purposeCombo->CurSel);
		OnSelChosenTypeCombo(m_typeCombo->CurSel);

		// (j.jones 2012-04-11 09:41) - PLID 44174 - disable the resource merge button if no access exists
		if(!(GetCurrentUserPermissions(bioMergeSchedulerResources) & (sptDynamic0 | sptDynamic0WithPass))) {
			m_btnMergeResources.EnableWindow(FALSE);
		}
		else {
			m_btnMergeResources.EnableWindow(TRUE);
		}

	}NxCatchAll(__FUNCTION__);
}

void CSchedulerSetupDlg::OnSelChosenCategoryCombo(long nRow) 
{
	try
	{
		long type;
		short id, oldID;

		if(nRow == -1) {
			//make sure there are rows
			if(m_categoryCombo->GetRowCount() > 0) {
				//no selection - set it to the first row
				m_categoryCombo->PutCurSel(0);
				nRow = 0;	//change what we got, this is used lower down
				//and let it continue from there
			}
		}

		if (!GetTypeID(type))
			return;

		id = VarShort(m_categoryCombo->Value[nRow][0]);
		oldID = VarShort(m_typeCombo->Value[m_typeCombo->CurSel][PC_CATEGORY]);

		if(id == AC_BLOCK_TIME) {
			//Are there any appointments with the current type and an associated patient?
			_RecordsetPtr rsCount = CreateRecordset("SELECT Count(ID) AS ApptCount FROM AppointmentsT WHERE AptTypeID = %li AND PatientID <> -25", type);
			long nCount = AdoFldLong(rsCount, "ApptCount");
			if(nCount > 0) {
				CBlockTimeWarningDlg dlg(this);
				dlg.m_nAffectedCount = nCount;
				dlg.m_nTypeID = type;
				dlg.m_strTypeName = VarString(m_typeCombo->Value[m_typeCombo->CurSel][PC_NAME]);
				dlg.DoModal();
				m_categoryCombo->SetSelByColumn(0, oldID);
				return;
			}
		}


		ExecuteSql("UPDATE AptTypeT SET Category = %i WHERE ID = %i", id, type);

		
		m_typeCombo->Value[m_typeCombo->CurSel][PC_CATEGORY] = id;

		if (oldID == AC_NON_PROCEDURAL || id == AC_NON_PROCEDURAL ||
			oldID == AC_BLOCK_TIME || id == AC_BLOCK_TIME)
		{	ExecuteSql("DELETE FROM ResourcePurposeTypeT WHERE AptTypeID = %i", type);
			ExecuteSql("DELETE FROM AptPurposeTypeT WHERE AptTypeID = %i", type);
			m_purposeTypeChecker.Refresh();
			UpdateView();
		}

		// (j.gruber 2010-07-20 14:19) - PLID 30481 - grey out button if selected surgery or minor procedure		
		// (j.jones 2011-07-22 13:10) - PLID 42059 - Other Procedure appts. now use the procedure's codes, not the appt type codes
		if (id == AC_SURGERY || id == AC_MINOR || id == AC_OTHER) {
			m_btnCodeLink.EnableWindow(FALSE);
		}
		else {
			m_btnCodeLink.EnableWindow(TRUE);
		}

		m_typeChecker.Refresh(type);
		return;
	}
	NxCatchAll("could not change category");
	UpdateView();
}

void CSchedulerSetupDlg::OnSelChosenPurposeCombo(long nRow) 
{
	long id;

	if (!GetPurposeID(id)){
		GetDlgItem(IDC_DELETE_PURPOSE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RENAME_PURPOSE)->EnableWindow(FALSE);
		return;
	}
	else{
		GetDlgItem(IDC_DELETE_PURPOSE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RENAME_PURPOSE)->EnableWindow(TRUE);
	}
	
}

void CSchedulerSetupDlg::OnSelChosenTypeCombo(long nRow) 
{
	long id;
	short category;
	CString where, from, field;

	if(nRow == -1) {
		//make sure there are rows
		if(m_typeCombo->GetRowCount() > 0) {
			//no selection - set it to the first row
			m_typeCombo->PutCurSel(0);
			nRow = 0;	//change what we got, this is used lower down
			//and let it continue from there
		}
	}

	//get variables
	if (!GetTypeID(id)){
		GetDlgItem(IDC_DELETE_TYPE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RENAME_TYPE)->EnableWindow(FALSE);
		GetDlgItem(IDC_INACTIVATE_TYPE)->EnableWindow(FALSE);
		return;
	}
	else{
		GetDlgItem(IDC_DELETE_TYPE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RENAME_TYPE)->EnableWindow(TRUE);
		GetDlgItem(IDC_INACTIVATE_TYPE)->EnableWindow(TRUE);
	}


	try
	{
		// Appointment type color
		m_typeColor.SetColor(VarLong(m_typeCombo->Value[nRow][PC_COLOR]));

		// Appointment type category
		category = VarShort(m_typeCombo->Value[nRow][PC_CATEGORY]);
		m_categoryCombo->SetSelByColumn(0, category);

		// Appointment type default duration
		CString strDuration;
		strDuration.Format("%d", VarLong(m_typeCombo->Value[nRow][PC_DEFAULTDURATION]));
		SetDlgItemText(IDC_EDIT_DEFAULT_TYPE_DURATION,strDuration);

		// Appointment type default arrival minutes
		CString strArrivalMins;
		strArrivalMins.Format("%d", VarLong(m_typeCombo->Value[nRow][PC_DEFAULTARRIVALMINS]));
		SetDlgItemText(IDC_EDIT_DEFAULT_ARRIVAL_MINS, strArrivalMins);

		//get from
		from.Format("AptPurposeT "
			"LEFT JOIN AptPurposeTypeT ON AptPurposeT.ID = AptPurposeTypeT.AptPurposeID "
				"AND AptPurposeTypeT.AptTypeID = %i "
			"LEFT JOIN ProcedureT ON AptPurposeT.ID = ProcedureT.ID", id);

		//get where
		if (category == AC_NON_PROCEDURAL || category == AC_BLOCK_TIME)
			where = "ProcedureT.ID IS NULL";
		// (c.haag 2008-11-26 16:00) - PLID 32264 - Factor in inactive flag
		else where = "ProcedureT.ID IS NOT NULL AND AptPurposeT.ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1)";

		// (j.gruber 2010-07-20 14:06) - PLID 30481 - grey out the configure button if we are on a procedure or minor procedure
		// (j.jones 2011-07-22 13:10) - PLID 42059 - Other Procedure appts. now use the procedure's codes, not the appt type codes
		if (category == AC_SURGERY || category == AC_MINOR || category == AC_OTHER) {
			m_btnCodeLink.EnableWindow(FALSE);
		}
		else {
			m_btnCodeLink.EnableWindow(TRUE);
		}


		//get field
		IColumnSettingsPtr pCol = m_procedureList->GetColumn(1);
		field.Format("CONVERT(BIT, CASE WHEN (AptPurposeTypeT.AptTypeID = %i) THEN 1 ELSE 0 END)", id);

		//set everything
		m_procedureList->FromClause = _bstr_t(from);
		m_procedureList->WhereClause = _bstr_t(where);
		pCol->FieldName = _bstr_t(field);
		m_procedureList->Requery();
		UpdateData(FALSE);
	}NxCatchAll("Could not load apt type");
}

void CSchedulerSetupDlg::OnEditingFinishedProcedureList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if (!bCommit) {
		return;
	}
	
	long type, purpose;
	
	if (!GetTypeID(type))
		return;

	purpose = VarLong(m_procedureList->Value[nRow][0]);

	try
	{
		BOOL bOldVal = VarBool(varOldValue);
		BOOL bNewVal = VarBool(varNewValue);
		if (bOldVal != bNewVal) {
			if (bNewVal)
			{
				ExecuteSql("INSERT INTO AptPurposeTypeT(AptTypeID, AptPurposeID) SELECT %i, %i", 
					type, purpose);
				ExecuteSql("INSERT INTO ResourcePurposeTypeT(ResourceID, AptTypeID, AptPurposeID) SELECT ID, %i, %i FROM ResourceT", 
					type, purpose);
			}
			else
			{
				ExecuteSql("DELETE FROM ResourcePurposeTypeT WHERE AptTypeID = %i AND AptPurposeID = %i", 
					type, purpose);
				ExecuteSql("DELETE FROM AptPurposeTypeT WHERE AptTypeID = %i AND AptPurposeID = %i", 
					type, purpose);
			}
			m_purposeTypeChecker.Refresh();
		}
		return;
	}NxCatchAll("Could not save purpose type");
	UpdateView();
}

void CSchedulerSetupDlg::OnDeleteType() 
{
	long id;

	if (!GetTypeID(id))
		return;

	if(AfxMessageBox("Are you sure you wish to delete this type?", MB_YESNO) == IDNO)
		return;

	try
	{
		_RecordsetPtr rs = CreateRecordset("SELECT TOP 1 AptPurposeID "
			"FROM AppointmentsT WHERE AptTypeID = %i", id);

		CString strOld = CString(m_typeCombo->GetValue(m_typeCombo->CurSel, 1).bstrVal);
		if (!rs->eof)
		{	AfxMessageBox("You may not delete a type that has been scheduled");
			return;
		}

		// c.haag 8/26/05
		// PLID 16820 - Don't delete the type if it's in use by the NexPDA link
		CString strOutlookUsers;
		_RecordsetPtr prsOutlook = CreateRecordset("select last + ', ' + first + ' ' + middle as username from outlookprofilet "
			"left join persont on persont.id = outlookprofilet.userid "
			"where outlookprofilet.userid in (select userid from outlookcategoryt where id in "
			"(select categoryid from outlookapttypet where apttypeid = %d))", id);
		FieldsPtr fOutlook = prsOutlook->Fields;
		while (!prsOutlook->eof)
		{	strOutlookUsers += AdoFldString(fOutlook, "Username", "") + "\n";
			prsOutlook->MoveNext();
		}
		if (strOutlookUsers.GetLength()) {
			// (z.manning 2009-11-12 15:36) - PLID 31879 - May also be NexSync
			MsgBox(MB_OK|MB_ICONINFORMATION,
				"You may not delete this type because it is in use by the following NexPDA or NexSync profiles:\n\n" +
				strOutlookUsers);
			return;
		}

		// (d.moore 2007-10-24) - PLID 4013 - Check to see if the appointment type is in use in the waiting list.
		rs = CreateRecordset("SELECT TOP 1 ID FROM WaitingListT WHERE TypeID = %li", id);
		if (!rs->eof)
		{	AfxMessageBox("You may not delete a type that has been used in the waiting list");
			return;
		}

		//DRT 6/11/2008 - PLID 30306 - Do not let them delete the type if it is joined to a superbill template default config.
		rs = CreateParamRecordset("SELECT GroupID FROM SuperbillTemplateTypeT WHERE TypeID = {INT}", id);
		if(!rs->eof) {
			//This type has default templates applied to it.
			AfxMessageBox("You may not delete a type which has default superbill templates applied to it.");
			return;
		}
		// (s.tullis 2014-12-12 08:41) - PLID 64440 - need to check if this Type is Associated with a Scheduling Mix Rules
		rs = CreateParamRecordset("Select ID FROM ScheduleMixRuleDetailsT WHERE AptTypeID = {INT}", id);
		if (!rs->eof) {
			AfxMessageBox("You may not delete a type that is used in a Scheduling Mix Rule template.");
			return;
		}

		// (b.cardillo 2011-02-26 11:01) - These should have been closed or just put in their own code blocks, but weren't.
		rs->Close();
		prsOutlook->Close();

		// (b.cardillo 2011-02-26 10:15) - PLID 40419 - Check for appointment prototypes that reference this apttype
		{
			// Get the list of prototypes, if any, that depend on this apttype
			CString strReferencedPrototypes = GenerateDelimitedListFromRecordsetColumn(CreateParamRecordset(
					_T("SELECT Name FROM ApptPrototypeT WHERE ID IN (\r\n")
					_T(" SELECT PS.ApptPrototypeID \r\n")
					_T(" FROM ApptPrototypePropertySetT PS \r\n")
					_T(" INNER JOIN ApptPrototypePropertySetAptTypeT PSAT ON PS.ID = PSAT.ApptPrototypePropertySetID \r\n")
					_T(" WHERE PSAT.AptTypeID = {INT} \r\n")
					_T(") \r\n")
					_T("ORDER BY Name \r\n")
					, id)
				, AsVariant("Name"), "", "\r\n");
			if (!strReferencedPrototypes.IsEmpty()) {
				AfxMessageBox(FormatString(
					_T("You may not delete this type because it is referenced by the following Appointment Prototypes:\r\n\r\n%s")
					, strReferencedPrototypes));
				return;
			}
		}
		// (s.tullis 2015-07-08 11:17) - PLID 63851 - Need to check and warn Template Rules
		if (!CheckWarnTemplateRuleDetails(TRUE, id))
		{
			return;
		}
		// (s.tullis 2015-07-08 11:17) - PLID 63851 - Remove the detail from the rule
		ExecuteParamSql("Delete FROM TemplateRuleDetailsT WHERE (ObjectType =1 OR ObjectType = 101)  AND ObjectID = {INT} ", id);
		ExecuteSql("DELETE FROM PalmSettingsAptTypeT WHERE AptTypeID = %li ", id);
		ExecuteSql("DELETE FROM AptPurposeTypeT WHERE AptTypeID = %li", id);
		ExecuteSql("DELETE FROM ResourcePurposeTypeT WHERE AptTypeID = %i", id);
		ExecuteSql("DELETE FROM ProviderSchedDefDurationDetailT WHERE ProviderSchedDefDurationID IN (SELECT ID FROM ProviderSchedDefDurationT WHERE AptTypeID = %d)", id);
		ExecuteSql("DELETE FROM ProviderSchedDefDurationT WHERE AptTypeID = %li", id);
		ExecuteSql("DELETE FROM AptBookAlarmDetailsT WHERE AptTypeID = %li", id);
		// (c.haag 2005-10-28 12:50 PM) - PLID 18032 - We no longer have this table
//		ExecuteSql("DELETE FROM OutlookSyncAptTypeT WHERE AptTypeID = %li", id);
		// (z.manning, 02/15/2008) - PLID 28909 - Clear any attendance related remote properties that use this ID.
		ExecuteSql("UPDATE ConfigRT SET IntParam = -1 WHERE Name LIKE 'Attendance%%TypeID' AND IntParam = %li", id);
		//TES 6/12/2008 - PLID 28078 - Since we're deleting this type, we're not going to require allocations for it any more.
		ExecuteSql("DELETE FROM ApptsRequiringAllocationsDetailT WHERE AptTypeID = %li", id);
		// (a.walling 2010-06-15 15:48) - PLID 39184 - Clear our any resource set links using this type
		ExecuteSql("DELETE FROM AptResourceSetLinksT WHERE AptTypeID = %i", id);
		// (j.gruber 2010-07-20 14:46) - PLID 30481 - delete a code link
		ExecuteSql("DELETE FROM ApptTypeServiceLinkT WHERE AptTypeID = %i", id);
		// (r.gonet 09-19-2010 3:53) - PLID 39282 - Delete any Appointment Reminder Scheduler Filters that have this type
		ExecuteSql("DELETE FROM ApptReminderFiltersT WHERE AppointmentType = %li", id);
		// (j.gruber 2011-05-06 16:37) - PLID 43550 - delete from Conversion Types
		ExecuteParamSql("DELETE FROM ApptServiceConvTypesT WHERE ApptTypeID = {INT}", id);
		ExecuteSql("DELETE FROM AptTypeT WHERE ID = %i", id);
		

		// (c.haag 2010-01-04 11:02) - PLID 28977 - Make sure this ID is no longer in the scheduler count property string.
		// If we don't do this, then id will be left dangling in the string; and if it happens to be reused when adding a new
		// type, it would automatically be hidden from the counts at the bottom of the scheduler.
		CSchedulerCountSettings s;
		s.Save();

		//auditing
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, "", nAuditID, aeiSchedTypeDeleted, id, strOld, "<Deleted>", aepMedium, aetDeleted);

		m_typeChecker.Refresh(id);
		m_typeCombo->Requery();
		m_typeCombo->CurSel = 0;
		UpdateView();
	}
	NxCatchAll("Could not delete type");
}

void CSchedulerSetupDlg::OnLButtonUpProcedureList(long nRow, short nCol, long x, long y, long nFlags) 
{

/*	Done by the Datalist automatically now
	long type, purpose;

	try
	{

		if (nCol != 1 || nRow == -1 || !GetTypeID(type))
			return;

		purpose = VarLong(m_procedureList->Value[nRow][0]);


		if (VarLong(m_procedureList->Value[nRow][1]))
		{
			ExecuteSql("DELETE FROM AptPurposeTypeT WHERE AptTypeID = %i AND AptPurposeID = %i", 
				type, purpose);
		}
		else 
		{	ExecuteSql("INSERT INTO AptPurposeTypeT(AptTypeID, AptPurposeID) SELECT %i, %i", 
				type, purpose);
		}
		m_procedureList->Requery();
	}NxCatchAll("Could not single click update");*/
}

void CSchedulerSetupDlg::OnRequeryFinishedTypeCombo(short nFlags) 
{
	long count = m_typeCombo->GetRowCount();
	IRowSettingsPtr pRow;

	for (long i = 0; i < count; i++)
	{	pRow = m_typeCombo->GetRow(i);
		pRow->ForeColor = VarLong(pRow->Value[PC_COLOR]);
	}
}

void CSchedulerSetupDlg::OnReplace() 
{
	DontShowMeAgain(this, "Be careful when using this utility.\n"
		"You can change large amounts of data at once\n"
		"And your changes cannot be undone", 
		"AptPurposeReplace");

	CProcedureReplaceDlg dlg(this);

	dlg.DoModal();
}

void CSchedulerSetupDlg::OnRenameType() 
{
	int nCurSel = m_typeCombo->CurSel;
	if(nCurSel != -1){
		CString strResult;
		int nResult = InputBoxLimited(this, "Please enter a new name for this type:", strResult, "",50,false,false,NULL);
		if(nResult == IDOK && strResult != "") {
			try {

				if(!IsRecordsetEmpty("SELECT Name FROM AptTypeT WHERE Name = '%s' AND ID <> %li",_Q(strResult), VarLong(m_typeCombo->GetValue(nCurSel, 0)))) {
					MessageBox("This appointment type already exists. Please choose a new name.","Practice",MB_OK|MB_ICONEXCLAMATION);
					return;
				}

				CString strOld = CString(m_typeCombo->GetValue(nCurSel, 1).bstrVal);

				// (z.manning, 08/07/2007) - PLID 26968 - Warn if this type is used on any appointments (including cancelled ones).
				_RecordsetPtr prsApptCount = CreateParamRecordset(
					"SELECT COUNT(*) AS Count FROM AppointmentsT WHERE AptTypeID = {INT}", VarLong(m_typeCombo->GetValue(nCurSel, 0)));
				long nApptCount = AdoFldLong(prsApptCount, "Count", 0);
				if(nApptCount > 0 && strResult != strOld)
				{
					int nResult = MessageBox(FormatString(
						"Renaming this type will change it everywhere in the program including on %li appointments.\r\n\r\n"
						"Are you sure you want to rename this appointment type?", nApptCount), NULL, MB_YESNO);
					if(nResult != IDYES) {
						return;
					}
				}

				ExecuteSql("UPDATE AptTypeT SET Name = '%s' WHERE ID = %li", _Q(strResult), VarLong(m_typeCombo->GetValue(nCurSel, 0)));
				m_typeCombo->PutValue(nCurSel, 1, _bstr_t(strResult));

				//auditing
				long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();
				if(nAuditID != -1)
					AuditEvent(-1, "", nAuditID, aeiSchedType, VarLong(m_typeCombo->GetValue(nCurSel, 0)), strOld, strResult, aepMedium, aetChanged);
			}NxCatchAll("Error in CSchedulerSetupDlg::OnRenameType");
		}
	}
}


void CSchedulerSetupDlg::OnRenamePurpose() 
{
	int nCurSel = m_purposeCombo->CurSel;
	if(nCurSel != -1){
		CString strResult;
		int nResult = InputBoxLimited(this, "Please enter a new name for this purpose:", strResult, "",100,false,false,NULL);
		if(nResult == IDOK && strResult != ""){
			try{

				// (c.haag 2008-12-17 17:21) - PLID 32376 - The purpose may have been inactivated
				_RecordsetPtr prs = CreateParamRecordset("SELECT Inactive FROM AptPurposeT "
					"LEFT JOIN ProcedureT ON ProcedureT.ID = AptPurposeT.ID "
					"WHERE AptPurposeT.Name = {STRING} AND AptPurposeT.ID <> {INT} ", strResult, VarLong(m_purposeCombo->GetValue(nCurSel, 0)));
				if (!prs->eof) {
					if (AdoFldBool(prs, "Inactive", FALSE)) {
						// The appointment purpose name is used by an inactive procedure
						MessageBox("This name is already in use by an inactive procedure. Please choose a new name.","Practice",MB_OK|MB_ICONEXCLAMATION);
					} else {
						MessageBox("This purpose already exists. Please choose a new name.","Practice",MB_OK|MB_ICONEXCLAMATION);
					}
					return;
				}

				long nID = VarLong(m_purposeCombo->GetValue(nCurSel, 0));

				CString strOld = CString(m_purposeCombo->GetValue(nCurSel, 1).bstrVal);
				ExecuteSql("UPDATE AptPurposeT SET Name = '%s' WHERE ID = %li", _Q(strResult), nID);
				m_purposeCombo->PutValue(nCurSel, 1, _bstr_t(strResult));
				m_purposeChecker.Refresh(nID);
				UpdateView();
			//auditing
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1)
				AuditEvent(-1, "", nAuditID, aeiSchedPurpose, VarLong(m_purposeCombo->GetValue(nCurSel, 0)), strOld, strResult, aepMedium, aetChanged);
			}NxCatchAll("Error in CSchedulerSetupDlg::OnRenamePurpose");
		}
	}
}

void CSchedulerSetupDlg::OnSelectAll() 
{
	long nTypeID;
	try
	{
		if (!GetTypeID(nTypeID))
			return;

		CSqlTransaction trans("SchedSetupSelAll");
		trans.Begin();

		//Is this procedural, or not?
		if( IsAptTypeCategoryNonProcedural(m_typeCombo->GetValue(m_typeCombo->CurSel, PC_CATEGORY)) ) {

			//This is non-procedural
			ExecuteSql("INSERT INTO AptPurposeTypeT (AptTypeID, AptPurposeID) "
				"SELECT %li, ID FROM AptPurposeT WHERE ID NOT IN (SELECT ID FROM ProcedureT) "
				"AND ID NOT IN (SELECT AptPurposeID FROM AptPurposeTypeT WHERE AptTypeID = %li) ", 
				nTypeID, nTypeID);

			ExecuteSql("INSERT INTO ResourcePurposeTypeT (ResourceID, AptTypeID, AptPurposeID) "
				"SELECT ResourceT.ID, %li, AptPurposeT.ID FROM ResourceT, AptPurposeT "
				"WHERE AptPurposeT.ID NOT IN (SELECT ID FROM ProcedureT) "
				"AND AptPurposeT.ID NOT IN (SELECT AptPurposeID FROM ResourcePurposeTypeT WHERE AptTypeID = %li) ",
				nTypeID, nTypeID);
		}
		else {
			//This is procedural
			// (c.haag 2008-11-26 16:02) - PLID 32264 - Only include active procedures
			ExecuteSql("INSERT INTO AptPurposeTypeT (AptTypeID, AptPurposeID) "
				"SELECT %li, ID FROM AptPurposeT WHERE ID IN (SELECT ID FROM ProcedureT WHERE Inactive = 0) "
				"AND ID NOT IN (SELECT AptPurposeID FROM AptPurposeTypeT WHERE AptTypeID = %li) ", 
				nTypeID, nTypeID);

			// (c.haag 2008-11-26 16:02) - PLID 32264 - Only include active procedures
			ExecuteSql("INSERT INTO ResourcePurposeTypeT (ResourceID, AptTypeID, AptPurposeID) "
				"SELECT ResourceT.ID, %li, AptPurposeT.ID FROM ResourceT, AptPurposeT "
				"WHERE AptPurposeT.ID IN (SELECT ID FROM ProcedureT WHERE Inactive = 0) "
				"AND AptPurposeT.ID NOT IN (SELECT AptPurposeID FROM ResourcePurposeTypeT WHERE AptTypeID = %li) ",
				nTypeID, nTypeID);
		}
		trans.Commit();

		// (z.manning, 5/4/2006, PLID 20418) - No need to requery here.
		//m_procedureList->Requery();
		long nRowCount = m_procedureList->GetRowCount();
		for(int i = 0; i < nRowCount; i++) {
			m_procedureList->PutValue( i, 1, COleVariant((short)TRUE, VT_BOOL) );
		}

		return;
	}NxCatchAll("Could not single click update");
}

void CSchedulerSetupDlg::OnDeselectAll() 
{
	try
	{
		long nTypeID;
		if (!GetTypeID(nTypeID))
			return;

		ExecuteSql("DELETE FROM AptPurposeTypeT WHERE AptTypeID = %i", nTypeID);
		ExecuteSql("DELETE FROM ResourcePurposeTypeT WHERE AptTypeID = %i", nTypeID);

		// (z.manning, 5/4/2006, PLID 20418) - No need to requery here.
		//m_procedureList->Requery();
		long nRowCount = m_procedureList->GetRowCount();
		for(int i = 0; i < nRowCount; i++) {
			m_procedureList->PutValue( i, 1, COleVariant((short)FALSE, VT_BOOL) );
		}
	}NxCatchAll("Could not single click update");}


void CSchedulerSetupDlg::OnPreferences()
{
	// (z.manning, 02/26/2007) - PLID 24921 - Check permission for preferences.
	//TES 6/23/2009 - PLID 34155 - Changed the View permission to Read
	if(CheckCurrentUserPermissions(bioPreferences,sptRead)) {
		ShowPreferencesDlg(GetRemoteData(), GetCurrentUserName(), GetRegistryBase(), piSchedulerModule);
	}

	// (c.haag 2005-01-26 14:12) - PLID 15415 - Flush the remote property cache because one or
	// more preferences may have changed, which means ConfigRT changed without intervention from
	// the cache.
	// (c.haag 2005-11-14 09:41) - PLID 16595 - This is no longer necessary
	//FlushRemotePropertyCache();
}

HBRUSH CSchedulerSetupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	//to avoid drawing problems with transparent text and disabled ites
	//override the NxDialog way of doing text with a non-grey background
	//NxDialog relies on the NxColor to draw the background, then draws text transparently
	//instead, we actually color the background of the STATIC text

	if(nCtlColor == CTLCOLOR_STATIC) {
		if (pWnd->GetDlgCtrlID() == IDC_TYPE_CAPTION
			|| pWnd->GetDlgCtrlID() == IDC_USER_CAPTION) {

			extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(0x008080FF));
			return m_brush;
		}
	}*/
	
	// (a.walling 2008-04-01 16:47) - PLID 29497 - Deprecated; use parent class' implementation
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CSchedulerSetupDlg::OnSchedulerDuration() 
{
	//TES 12/18/2008 - PLID 32513 - Scheduler Standard users aren't allowed to do this.
	if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Configuring default durations for different appointment type/purpose/provider combinations")) {
		return;
	}

	CSchedulerDurationDlg dlg(this);
	dlg.DoModal();
}

void CSchedulerSetupDlg::OnBookingAlarms()
{
	//TES 12/18/2008 - PLID 32513 - Scheduler Standard users aren't allowed to do this.
	if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Configure booking alarms for possible duplicate appointments")) {
		return;
	}

	if (CheckCurrentUserPermissions(bioAdminScheduler, sptView|sptRead|sptWrite))
	{
		CAptBookAlarmListDlg dlgAptBookAlarmList(this);
		dlgAptBookAlarmList.DoModal();
	}
}

void CSchedulerSetupDlg::OnEditStatuses()
{
	CSchedulerStatusDlg dlg(this);
	dlg.DoModal();
}

void CSchedulerSetupDlg::OnChangeEditDefaultTypeDuration() 
{
	m_bNeedToSaveDuration = TRUE;
}

void CSchedulerSetupDlg::OnKillfocusEditDefaultTypeDuration() 
{
	if (m_bNeedToSaveDuration)
	{
		try {
			long nID;
			UpdateData(TRUE);
			if (GetTypeID(nID))
			{
				CString strDuration;
				GetDlgItemText(IDC_EDIT_DEFAULT_TYPE_DURATION,strDuration);

				ExecuteSql("UPDATE AptTypeT SET DefaultDuration = %d WHERE ID = %d",
					atoi(strDuration), nID);
				long nRow = m_typeCombo->FindByColumn(PC_ID, nID, 0, FALSE);
				if (nRow != -1)
					m_typeCombo->Value[nRow][PC_DEFAULTDURATION] = (long)atoi(strDuration);
			}
			CClient::RefreshTable(NetUtils::DefaultDuration, -1);
			m_bNeedToSaveDuration = FALSE;
		}
		NxCatchAll("Error saving default type duration");
	}
}

void CSchedulerSetupDlg::OnChangeEditDefaultArrivalMins()
{
	m_bNeedToSaveArrivalMins = TRUE;
}

void CSchedulerSetupDlg::OnKillfocusEditDefaultArrivalMins()
{
	if(m_bNeedToSaveArrivalMins) {
		try {
			long nID;
			UpdateData(TRUE);
			if(GetTypeID(nID)) {
				CString strArrivalMins;
				GetDlgItemText(IDC_EDIT_DEFAULT_ARRIVAL_MINS, strArrivalMins);
				if(strArrivalMins.IsEmpty()) {
					strArrivalMins = "0";
				}

				ExecuteSql("UPDATE AptTypeT SET DefaultArrivalMins = %d WHERE ID = %d",
					atoi(strArrivalMins), nID);
				
				long nRow = m_typeCombo->FindByColumn(PC_ID, nID, 0, FALSE);
				if (nRow != -1) {
					m_typeCombo->Value[nRow][PC_DEFAULTARRIVALMINS] = (long)atoi(strArrivalMins);
				}
			}
			m_bNeedToSaveArrivalMins = FALSE;
		}NxCatchAll("Error saving default arrival minutes");
	}
}

void CSchedulerSetupDlg::OnInactivateType()
{
	try {
		if(m_typeCombo->CurSel != -1) {
			CString strType = VarString(m_typeCombo->GetValue(m_typeCombo->CurSel, 1));
			if(IDYES == MsgBox(MB_YESNO, "Are you sure you wish to make the appointment type %s inactive?\n"
				"You will no longer be able to schedule appointments for this type.", strType)) {
				long nTypeID = VarLong(m_typeCombo->GetValue(m_typeCombo->CurSel, 0));
				ExecuteSql("UPDATE AptTypeT SET Inactive = 1 WHERE ID = %li", nTypeID);
				// (z.manning, 02/15/2008) - PLID 28909 - Clear any attendance related remote properties that use this ID.
				ExecuteSql("UPDATE ConfigRT SET IntParam = -1 WHERE Name LIKE 'Attendance%%TypeID' AND IntParam = %li", nTypeID);
				m_typeCombo->RemoveRow(m_typeCombo->CurSel);
				m_typeCombo->CurSel = 0;
				m_typeChecker.Refresh(nTypeID);
				Refresh();
			}
		}
	}NxCatchAll("Error in CSchedulerDlg::OnInactivateType()");
}


void CSchedulerSetupDlg::OnInactiveTypes() 
{
	CInactiveTypesDlg dlg(this);
	dlg.DoModal();
	m_typeChecker.Refresh();
	long nSel = m_typeCombo->CurSel;
	m_typeCombo->Requery();
	m_typeCombo->CurSel = nSel;
	Refresh();
}


void CSchedulerSetupDlg::OnRequeryFinishedProcedureList(short nFlags)
{
	if(m_procedureList->GetRowCount() == 0){
		GetDlgItem(IDC_DESELECT_ALL)->EnableWindow(FALSE);
		GetDlgItem(IDC_SELECT_ALL)->EnableWindow(FALSE);
	}
	else{
		GetDlgItem(IDC_DESELECT_ALL)->EnableWindow(TRUE);
		GetDlgItem(IDC_SELECT_ALL)->EnableWindow(TRUE);
	}
}

LRESULT CSchedulerSetupDlg::OnTableChanged(WPARAM wParam, LPARAM lParam) {

	try {
		switch(wParam) {
			case NetUtils::AptPurposeT:
			case NetUtils::AptTypeT:
			case NetUtils::AptPurposeTypeT: {
				try {

					long id, curSel;

					if (m_purposeChecker.Changed())
					{	curSel = m_purposeCombo->CurSel;
						
						if (curSel != -1)
						{	id = VarLong(m_purposeCombo->Value[curSel][0]);
							m_purposeCombo->Requery();
							m_purposeCombo->SetSelByColumn(0, id);
						}
						else m_purposeCombo->Requery();

						OnSelChosenTypeCombo(m_typeCombo->CurSel);
					}

					if (m_typeChecker.Changed())
					{	curSel = m_typeCombo->CurSel;
						
						if (curSel != -1)
						{	id = VarLong(m_typeCombo->Value[curSel][0]);
							m_typeCombo->Requery();
							m_typeCombo->SetSelByColumn(0, id);
						}
						else m_typeCombo->Requery();
					}

					if (m_purposeTypeChecker.Changed()) {

						OnSelChosenPurposeCombo(m_purposeCombo->CurSel);
						OnSelChosenTypeCombo(m_typeCombo->CurSel);
					}
				} NxCatchAll("Error in CSchedulerSetupDlg::OnTableChanged:Generic");
				break;
			}
		}
	} NxCatchAll("Error in CSchedulerSetupDlg::OnTableChanged");

	return 0;
}

// (a.walling 2008-09-03 08:58) - PLID 23457 - Copy procedure info to another type
void CSchedulerSetupDlg::OnCopyTo()
{
	try {
		CopyProcedureInfo(TRUE);
	} NxCatchAll("CSchedulerSetupDlg: Error copying procedure information");
}

// (a.walling 2008-09-03 08:58) - PLID 23457 - Merge procedure info into another type
void CSchedulerSetupDlg::OnMergeInto()
{
	try {
		CopyProcedureInfo(FALSE);
	} NxCatchAll("CSchedulerSetupDlg: Error merging procedure information");
}

// (a.walling 2008-09-02 17:54) - PLID 23457 - Overwrite or merge procedure info into another appointment type
void CSchedulerSetupDlg::CopyProcedureInfo(BOOL bOverwrite) 
{
	long nSourceTypeID;
	
	if (!GetTypeID(nSourceTypeID))
		return;

	if (!bOverwrite) {
		m_procedureList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		long nRowCount = m_procedureList->GetRowCount();
		long nSelected = 0;
		for(int i = 0; i < nRowCount; i++) {
			BOOL bSelected = VarBool(m_procedureList->GetValue(i, 1), FALSE);
			if (bSelected) {
				nSelected++;
			}
		}

		if (nSelected == 0) {
			MessageBox("No procedures are selected!", NULL, MB_OK|MB_ICONEXCLAMATION);
			return;
		}
	}
	
	// GetTypeID guarantees we have a cursel
	long nSourceCategoryID = VarByte(m_typeCombo->Value[m_typeCombo->CurSel][PC_CATEGORY]);

	_RecordsetPtr prs;
	switch (nSourceCategoryID) {	
		case AC_BLOCK_TIME:
		case AC_NON_PROCEDURAL:
			prs = CreateRecordset("SELECT ID, Name FROM AptTypeT WHERE Inactive = 0 AND ID <> %li AND Category IN (%li, %li) ORDER BY Name", nSourceTypeID, AC_BLOCK_TIME, AC_NON_PROCEDURAL);
			break;
		default:
			prs = CreateRecordset("SELECT ID, Name FROM AptTypeT WHERE Inactive = 0 AND ID <> %li AND Category NOT IN (%li, %li) ORDER BY Name", nSourceTypeID, AC_BLOCK_TIME, AC_NON_PROCEDURAL);
			break;
	}

	CMap<long, long, long, long> mapIDs;
	long nTypeCommandID = 100;
	CMenu mnu;
	if (mnu.CreatePopupMenu()) {
		long nTotal = 0;
		while (!prs->eof) {
			long nTypeID = AdoFldLong(prs, "ID");
			CString strName = ConvertToControlText(AdoFldString(prs, "Name"));
			mapIDs.SetAt(nTypeCommandID, nTypeID);

			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, nTypeCommandID, strName);

			prs->MoveNext();

			nTypeCommandID++;
			nTotal++;
		}

		if (nTotal == 0) {
			mnu.AppendMenu(MF_DISABLED|MF_GRAYED|MF_BYPOSITION, 0, "(no applicable types!)");
		}

		CPoint pt;
		GetMessagePos(pt);
		long nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);
		if (nResult) {
			long nTargetTypeID;
			if (mapIDs.Lookup(nResult, nTargetTypeID)) {
				if (bOverwrite) {
					if (IDOK != MessageBox("The currently selected procedures will be copied to this appointment type, clearing out all other selected procedures for that type. If you want to add these procedures to this type, rather than overwrite them entirely, please choose 'Merge Into' instead.", NULL, MB_OKCANCEL|MB_ICONINFORMATION)) {
						return;
					}
					CString strBatch = BeginSqlBatch();
					// overwrite
					AddStatementToSqlBatch(strBatch, "DELETE FROM AptPurposeTypeT WHERE AptTypeID = %li", nTargetTypeID);
					AddStatementToSqlBatch(strBatch, "DELETE FROM ResourcePurposeTypeT WHERE AptTypeID = %li", nTargetTypeID);
					// (c.haag 2008-12-17 17:30) - PLID 32376 - Don't copy inactive procedures
					AddStatementToSqlBatch(strBatch, "INSERT INTO AptPurposeTypeT(AptTypeID, AptPurposeID) "
						"SELECT %li, AptPurposeID FROM AptPurposeTypeT WHERE AptTypeID = %li AND AptPurposeID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1)", nTargetTypeID, nSourceTypeID);
					AddStatementToSqlBatch(strBatch, "INSERT INTO ResourcePurposeTypeT(ResourceID, AptTypeID, AptPurposeID) "
						"SELECT ResourceID, %li, AptPurposeID FROM ResourcePurposeTypeT WHERE AptTypeID = %li AND AptPurposeID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) ", nTargetTypeID, nSourceTypeID);
					ExecuteSqlBatch(strBatch);
				} else {
					if (IDOK != MessageBox("The currently selected procedures will be merged into this appointment type, in addition to all other selected procedures for that type. If you want to overwrite them entirely, clearing out any other selected procedures, please choose 'Copy To' instead.", NULL, MB_OKCANCEL|MB_ICONINFORMATION)) {
						return;
					}
					CString strBatch = BeginSqlBatch();
					// merge
					AddStatementToSqlBatch(strBatch, "INSERT INTO AptPurposeTypeT(AptTypeID, AptPurposeID) "
						"SELECT %li, AptPurposeID FROM AptPurposeTypeT WHERE AptTypeID = %li AND AptPurposeID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) "
						"AND AptPurposeID NOT IN (SELECT AptPurposeID FROM AptPurposeTypeT WHERE AptTypeID = %li)", nTargetTypeID, nSourceTypeID, nTargetTypeID);
					AddStatementToSqlBatch(strBatch, "INSERT INTO ResourcePurposeTypeT(ResourceID, AptTypeID, AptPurposeID) "
						"SELECT ResourceID, %li, AptPurposeID FROM ResourcePurposeTypeT MasterResourcePurposeTypeT WHERE MasterResourcePurposeTypeT.AptTypeID = %li AND AptPurposeID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) "
						"AND MasterResourcePurposeTypeT.AptPurposeID NOT IN (SELECT AptPurposeID FROM ResourcePurposeTypeT WHERE AptTypeID = %li AND ResourceID = MasterResourcePurposeTypeT.ResourceID)", nTargetTypeID, nSourceTypeID, nTargetTypeID);
					ExecuteSqlBatch(strBatch);
				}
			} else {
				ThrowNxException("Invalid command!");
			}
		}
	}

	return;
}


void CSchedulerSetupDlg::OnSetfocusEditDefaultTypeDuration()
{
	try {
		//TES 12/18/2008 - PLID 32513 - Scheduler Standard users aren't allowed to edit default durations, so if
		// one of them tries to get into this box, set the focus to the next box in the tab order (which is the category
		// dropdown, as it happens).
		if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Configuring default durations for different appointment types")) {
			GetDlgItem(IDC_CATEGORY_COMBO)->SetFocus();
		}
	}NxCatchAll("Error in CSchedulerSetupDlg::OnSetfocusEditDefaultTypeDuration()");
}

// (a.walling 2010-06-15 14:14) - PLID 39184 - Set the cursel if right button is down on a non-highlighted row
void CSchedulerSetupDlg::RButtonDownProcedureList(long nRow, short nCol, long x, long y, long nFlags)
{
	try {
		IRowSettingsPtr pRow = m_procedureList->GetRow(nRow);
		if (pRow) {
			if (!pRow->IsHighlighted()) {
				m_procedureList->CurSel = nRow;				
			}
		} else {
			m_procedureList->CurSel = nRow;
		}
	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2010-06-15 14:14) - PLID 39184 - Display a popup menu to assign and edit resource sets to types and type-purposes
void CSchedulerSetupDlg::RButtonUpProcedureList(long nRow, short nCol, long x, long y, long nFlags)
{
	try {
		long nAptTypeID = -1;
		CString strAptTypeName;
		if (!GetTypeID(nAptTypeID)) return;
		strAptTypeName = VarString(m_typeCombo->Value[m_typeCombo->CurSel][PC_NAME]);
		
		CString strPurposeName;

		CDWordArray dwaPurposes;

		long p = m_procedureList->GetFirstSelEnum();
		LPDISPATCH pDisp = NULL;
		while(p){
			m_procedureList->GetNextSelEnum(&p, &pDisp);
			IRowSettingsPtr pRow(pDisp);
			pDisp->Release();
			
			int purpose = VarLong(pRow->GetValue(0));

			dwaPurposes.Add((DWORD)purpose);
		}

		if (dwaPurposes.IsEmpty()) {
			return;
		} else if (dwaPurposes.GetSize() == 1) {
			strPurposeName = m_procedureList->GetValue(m_procedureList->CurSel, 2);
		} else {
			strPurposeName = "(Multiple)";
		}

		// (a.walling 2010-06-17 08:35) - PLID 39184 - Modified to get more information on what is using these sets
		_RecordsetPtr prs = CreateParamRecordset(FormatString(
			"SELECT ResourceSetT.ID, ResourceSetT.Name, "
				"SUM(CASE WHEN AptResourceSetLinksNoProcedureT.ResourceSetID IS NOT NULL THEN 1 ELSE 0 END) AS TypeResourceSets, "
				"SUM(CASE WHEN AptResourceSetLinksT.ResourceSetID IS NOT NULL THEN 1 ELSE 0 END) AS ProcedureResourceSets "
			"FROM ResourceSetT "
			"LEFT JOIN AptResourceSetLinksT AptResourceSetLinksNoProcedureT "
				"ON ResourceSetT.ID = AptResourceSetLinksNoProcedureT.ResourceSetID "
				"AND AptResourceSetLinksNoProcedureT.AptPurposeID IS NULL "
				"AND AptResourceSetLinksNoProcedureT.AptTypeID = {INT} "
			"LEFT JOIN AptResourceSetLinksT "
				"ON ResourceSetT.ID = AptResourceSetLinksT.ResourceSetID "
				"AND AptResourceSetLinksT.AptPurposeID IN (%s) "
				"AND AptResourceSetLinksT.AptTypeID = {INT} "
			"GROUP BY ResourceSetT.ID, ResourceSetT.Name, AptResourceSetLinksNoProcedureT.ResourceSetID "
			"ORDER BY ResourceSetT.Name", ArrayAsString(dwaPurposes, true)),
			nAptTypeID, nAptTypeID);


		struct CMenuInfo {
			CMenuInfo() : m_nResourceSetID(-1), m_bIsPurpose(false), m_bSelected(false), m_bEdit(false), m_bMultiple(false) {};

			long m_nResourceSetID;
			bool m_bIsPurpose;
			bool m_bSelected;
			bool m_bEdit;
			bool m_bMultiple;
		};
		CMap<long, long, CMenuInfo*, CMenuInfo*> mapMenuInfo;

		CMenu menuPurpose;
		CMenu menuType;
		CMenu menuEdit;

		CMenu menu;

		menu.CreatePopupMenu();
		if (!dwaPurposes.IsEmpty()) {
			menuPurpose.CreatePopupMenu();
		}
		menuType.CreatePopupMenu();
		menuEdit.CreatePopupMenu();

		menu.AppendMenu(MF_BYPOSITION|MF_POPUP, (UINT)menuType.m_hMenu, FormatString("Resource sets for type '%s'...", strAptTypeName));
		if (!dwaPurposes.IsEmpty()) {
			if (dwaPurposes.GetSize() == 1) {
				menu.AppendMenu(MF_BYPOSITION|MF_POPUP, (UINT)menuPurpose.m_hMenu, FormatString("Resource sets for type '%s' with purpose '%s'...", strAptTypeName, strPurposeName));
			} else {
				menu.AppendMenu(MF_BYPOSITION|MF_POPUP, (UINT)menuPurpose.m_hMenu, FormatString("Resource sets for type '%s' with any of the selected purposes...", strAptTypeName));
			}
		}
		menu.AppendMenu(MF_BYPOSITION|MF_SEPARATOR);
		menu.AppendMenu(MF_BYPOSITION|MF_POPUP, (UINT)menuEdit.m_hMenu, "Edit...");

		long nMenuItem = 100;

		{
			nMenuItem++;
			{
				CMenuInfo* pMenuInfo = new CMenuInfo();
				CMenuInfo& menuInfo(*pMenuInfo);
				menuInfo.m_nResourceSetID = -1;
				menuInfo.m_bIsPurpose = false;

				menuType.AppendMenu(MF_BYPOSITION, nMenuItem, "(None)");
				mapMenuInfo.SetAt(nMenuItem, pMenuInfo);
			}

			if (!dwaPurposes.IsEmpty()) {
				nMenuItem++;
				{
					CMenuInfo* pMenuInfo = new CMenuInfo();
					CMenuInfo& menuInfo(*pMenuInfo);
					menuInfo.m_nResourceSetID = -1;
					menuInfo.m_bIsPurpose = true;

					menuPurpose.AppendMenu(MF_BYPOSITION, nMenuItem, "(None)");
					mapMenuInfo.SetAt(nMenuItem, pMenuInfo);
				}
			}
		}

		CDWordArray dwaExistingTypeResourceSetIDs;
		CDWordArray dwaExistingPurposeResourceSetIDs;
		int nItem = 0;
		while (!prs->eof) {
			nItem++;

			long nResourceSetID = AdoFldLong(prs, "ID");
			CString strName = AdoFldString(prs, "Name", "");
			long nSelectedForType = AdoFldLong(prs, "TypeResourceSets", 0);
			long nSelectedForProcedures = AdoFldLong(prs, "ProcedureResourceSets", 0);

			
			nMenuItem++;
			{
				if (nItem == 1) {
					menuType.AppendMenu(MF_BYPOSITION|MF_SEPARATOR);
				}

				CMenuInfo* pMenuInfo = new CMenuInfo();
				CMenuInfo& menuInfo(*pMenuInfo);
				menuInfo.m_nResourceSetID = nResourceSetID;
				menuInfo.m_bIsPurpose = false;

				menuInfo.m_bSelected = nSelectedForType > 0;

				if (menuInfo.m_bSelected) {
					dwaExistingTypeResourceSetIDs.Add(nResourceSetID);
				}

				menuType.AppendMenu(MF_BYPOSITION|(menuInfo.m_bSelected ? MF_CHECKED : MF_UNCHECKED), nMenuItem, strName);
				mapMenuInfo.SetAt(nMenuItem, pMenuInfo);
			}

			if (!dwaPurposes.IsEmpty()) {
				nMenuItem++;
				{
					if (nItem == 1) {
						menuPurpose.AppendMenu(MF_BYPOSITION|MF_SEPARATOR);
					}

					CMenuInfo* pMenuInfo = new CMenuInfo();
					CMenuInfo& menuInfo(*pMenuInfo);
					menuInfo.m_nResourceSetID = nResourceSetID;
					menuInfo.m_bIsPurpose = true;

					menuInfo.m_bSelected = nSelectedForProcedures > 0;

					if (menuInfo.m_bSelected) {
						dwaExistingPurposeResourceSetIDs.Add(nResourceSetID);
					}

					CString strModName = strName;

					// (a.walling 2010-06-17 09:02) - PLID 39184 - Identify which are on all purposes and which are on one
					if (dwaPurposes.GetSize() > 1 && nSelectedForProcedures == dwaPurposes.GetSize()) {
						strModName += " (all purposes)";
					} else if (nSelectedForProcedures > 0 && nSelectedForProcedures < dwaPurposes.GetSize()) {
						strModName += FormatString(" (%li purpose%s)", nSelectedForProcedures, nSelectedForProcedures > 1 ? "s" : "");
					}

					menuPurpose.AppendMenu(MF_BYPOSITION|(menuInfo.m_bSelected ? MF_CHECKED : MF_UNCHECKED), nMenuItem, strModName);
					mapMenuInfo.SetAt(nMenuItem, pMenuInfo);
				}
			}

			nMenuItem++;
			{
				CMenuInfo* pMenuInfo = new CMenuInfo();
				CMenuInfo& menuInfo(*pMenuInfo);
				menuInfo.m_nResourceSetID = nResourceSetID;
				menuInfo.m_bEdit = true;

				menuEdit.AppendMenu(MF_BYPOSITION, nMenuItem, strName);
				mapMenuInfo.SetAt(nMenuItem, pMenuInfo);
			}

			prs->MoveNext();
		}
		
		nMenuItem++;
		{
			CMenuInfo* pMenuInfo = new CMenuInfo();
			CMenuInfo& menuInfo(*pMenuInfo);
			menuInfo.m_nResourceSetID = -1;
			menuInfo.m_bEdit = true;

			if (nItem > 0) {
				menuEdit.AppendMenu(MF_BYPOSITION|MF_SEPARATOR);
			}
			menuEdit.AppendMenu(MF_BYPOSITION, nMenuItem, "(New)");
			mapMenuInfo.SetAt(nMenuItem, pMenuInfo);
		}
		
		// (a.walling 2010-06-17 09:07) - PLID 39184 - Multiple selection dialog
		nMenuItem++;
		{
			CMenuInfo* pMenuInfo = new CMenuInfo();
			CMenuInfo& menuInfo(*pMenuInfo);
			menuInfo.m_nResourceSetID = -1;
			menuInfo.m_bMultiple = true;
			menuInfo.m_bIsPurpose = false;

			menuType.AppendMenu(MF_BYPOSITION|MF_SEPARATOR);
			menuType.AppendMenu(MF_BYPOSITION, nMenuItem, "(Multiple)");
			mapMenuInfo.SetAt(nMenuItem, pMenuInfo);
		}
		
		nMenuItem++;
		{
			CMenuInfo* pMenuInfo = new CMenuInfo();
			CMenuInfo& menuInfo(*pMenuInfo);
			menuInfo.m_nResourceSetID = -1;
			menuInfo.m_bMultiple = true;
			menuInfo.m_bIsPurpose = true;

			menuPurpose.AppendMenu(MF_BYPOSITION|MF_SEPARATOR);
			menuPurpose.AppendMenu(MF_BYPOSITION, nMenuItem, "(Multiple)");
			mapMenuInfo.SetAt(nMenuItem, pMenuInfo);
		}


		CPoint pt;
		GetCursorPos(&pt);

		int nCmdId = menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);
		CMenuInfo* pMenuInfo = NULL;
		if (mapMenuInfo.Lookup(nCmdId, pMenuInfo) && pMenuInfo != NULL) {
			if (pMenuInfo->m_bEdit) {
				CEditResourceSetDlg dlg(this);
				// this will create a new one if it is -1.
				dlg.m_nResourceSetID = pMenuInfo->m_nResourceSetID;

				dlg.DoModal();
			} else {
				CDWordArray dwaNewResourceSetIDs;
				CDWordArray dwaExistingResourceSetIDs;

				// This will determine if we should mess with existing selections or just deal with new and removed selections.
				bool bApplyExisting = false;
				bool bCancel = false;

				if (pMenuInfo->m_bMultiple) {
					if (pMenuInfo->m_bIsPurpose) {
						dwaExistingResourceSetIDs.Copy(dwaExistingPurposeResourceSetIDs);
					} else {
						dwaExistingResourceSetIDs.Copy(dwaExistingTypeResourceSetIDs);
					}

					// Init the dialog and fill with existing selections
					// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
					CMultiSelectDlg dlg(this, "ResourceSetT");
					for (int i=0; i < dwaExistingResourceSetIDs.GetSize(); i++)
					{
						dlg.PreSelect(dwaExistingResourceSetIDs[i]);
					}

					dlg.m_strNameColTitle = "Resource Set";

					// Have the user select all the resources to associate this appointment with
					HRESULT hRes = dlg.Open("ResourceSetT", "", "ID", "Name", 
						FormatString("Please select the resource sets to associate with %s.", pMenuInfo->m_bIsPurpose ? dwaPurposes.GetSize() == 1 ? "this purpose" : "these purposes" : "this appointment type")
					);

					// If we said yes, update our array of resources with this information
					if (hRes == IDOK)
					{
						dlg.FillArrayWithIDs(dwaNewResourceSetIDs);

						if (pMenuInfo->m_bIsPurpose && dwaPurposes.GetSize() > 1 && !dwaNewResourceSetIDs.IsEmpty()) {
							if (IDYES == MessageBox("Some resource sets may have been previously linked with only some purposes. Do you want to maintain these existing selections, or apply these resource sets to all selected purposes?\r\n\r\n"
								"Choose 'Yes' to maintain existing selections by only applying new resource sets and removing unselected resource sets.\r\n\r\n"
								"Choose 'No' to apply these resource sets to all selected purposes.", NULL, MB_YESNO)) {
								bApplyExisting = false;
							} else {
								bApplyExisting = true;
							}
						}
					} else {
						bCancel = true;
					}
				} else {
					dwaNewResourceSetIDs.Add(pMenuInfo->m_nResourceSetID);
				}

				if (!bCancel) {
					CDWordArray dwaTotalResourceSetIDs;
					dwaTotalResourceSetIDs.Copy(dwaNewResourceSetIDs);
					for (int i = 0; i < dwaExistingResourceSetIDs.GetSize(); i++) {
						if (!IsIDInArray(dwaExistingResourceSetIDs.GetAt(i), &dwaTotalResourceSetIDs)) {
							dwaTotalResourceSetIDs.Add(dwaExistingResourceSetIDs.GetAt(i));
						}
					}

					CParamSqlBatch batch;
					int nStatements = 0;

					for (int nResourceSetIndex = 0; nResourceSetIndex < dwaTotalResourceSetIDs.GetSize(); nResourceSetIndex++) {
						long nResourceSetID = (long)dwaTotalResourceSetIDs.GetAt(nResourceSetIndex);

						bool bNeedRemove = false;
						bool bNeedRemoveAll = false;
						bool bNeedAdd = false;

						if (pMenuInfo->m_bMultiple) {
							if (!IsIDInArray((DWORD)nResourceSetID, &dwaNewResourceSetIDs)) {
								// not in new, must have been in existing, so remove
								bNeedRemove = true;
							} else if (!IsIDInArray((DWORD)nResourceSetID, &dwaExistingResourceSetIDs)) {
								// not in existing, must be in new
								bNeedAdd = true;
							} else {
								// in both new and existing. if bApplyExisting, then we'll want to add to all purposes
								if (bApplyExisting) {
									bNeedAdd = true;
								}
							}
						} else {
							if (pMenuInfo->m_bSelected) {
								bNeedRemove = true;
							} else if (pMenuInfo->m_nResourceSetID == -1) {
								bNeedRemoveAll = true;
							} else {
								bNeedAdd = true;
							}
						}

						// we'll toggle
						if (bNeedRemove || bNeedRemoveAll || bNeedAdd) {
							nStatements++;
							if (pMenuInfo->m_bIsPurpose) {

								for (int i = 0; i < dwaPurposes.GetSize(); i++) {
									if (bNeedRemove) {
										batch.Add("DELETE FROM AptResourceSetLinksT WHERE AptTypeID = {INT} AND AptPurposeID = {INT} AND ResourceSetID = {INT}", nAptTypeID, dwaPurposes.GetAt(i), nResourceSetID);
									} else if (bNeedRemoveAll) {
										batch.Add("DELETE FROM AptResourceSetLinksT WHERE AptTypeID = {INT} AND AptPurposeID = {INT}", nAptTypeID, dwaPurposes.GetAt(i));
									} else if (bNeedAdd) {
										// (a.walling 2010-06-17 08:20) - PLID 39184 - Ensure there are never any duplicate resource set links
										batch.Add("DELETE FROM AptResourceSetLinksT WHERE AptTypeID = {INT} AND AptPurposeID = {INT} AND ResourceSetID = {INT}", nAptTypeID, dwaPurposes.GetAt(i), nResourceSetID);
										batch.Add("INSERT INTO AptResourceSetLinksT(AptTypeID, AptPurposeID, ResourceSetID) VALUES({INT}, {INT}, {INT})", nAptTypeID, dwaPurposes.GetAt(i), nResourceSetID);
									}
								}
							} else {
								if (bNeedRemove) {
									batch.Add("DELETE FROM AptResourceSetLinksT WHERE AptTypeID = {INT} AND AptPurposeID IS NULL AND ResourceSetID = {INT}", nAptTypeID, nResourceSetID);
								} else if (bNeedRemoveAll) {
									batch.Add("DELETE FROM AptResourceSetLinksT WHERE AptTypeID = {INT} AND AptPurposeID IS NULL", nAptTypeID);
								} else if (bNeedAdd) {
									// (a.walling 2010-06-17 08:20) - PLID 39184 - Ensure there are never any duplicate resource set links
									batch.Add("DELETE FROM AptResourceSetLinksT WHERE AptTypeID = {INT} AND AptPurposeID IS NULL AND ResourceSetID = {INT}", nAptTypeID, nResourceSetID);
									batch.Add("INSERT INTO AptResourceSetLinksT(AptTypeID, AptPurposeID, ResourceSetID) VALUES({INT}, NULL, {INT})", nAptTypeID, nResourceSetID);
								}
							}
						}
					}

					if (nStatements > 0) {
						batch.Execute(GetRemoteData());
					}
				}
			}
		}

		POSITION pos = mapMenuInfo.GetStartPosition();
		while (pos) {
			CMenuInfo* pMenuInfo = NULL;
			long nKey = -1;
			mapMenuInfo.GetNextAssoc(pos, nKey, pMenuInfo);
			delete pMenuInfo;
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-07-20 14:04) - PLID 30481
void CSchedulerSetupDlg::OnBnClickedConfTypeCodeLink()
{
	try {
		long nCurSel = m_typeCombo->GetCurSel();
		long nApptTypeID = VarLong(m_typeCombo->GetValue(nCurSel, 0));
		CString strType = VarString(m_typeCombo->GetValue(nCurSel, 1));
		CApptTypeCodeLinkDlg dlg(nApptTypeID, strType, this);
		dlg.DoModal();
	}NxCatchAll(__FUNCTION__);

}

// (j.armen 2012-02-24 15:54) - PLID 48304
void CSchedulerSetupDlg::OnBnClickedRecallSetup()
{
	try
	{
		// (j.armen 2012-03-28 09:22) - PLID 48480 - The button should not be enabled, but just in case
		if(g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent))
		{
			CRecallSetupDlg dlg(this);
			dlg.DoModal();
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-04-10 15:15) - PLID 44174 - added ability to merge resources
void CSchedulerSetupDlg::OnBtnMergeResources()
{
	try {

		if(!CheckCurrentUserPermissions(bioMergeSchedulerResources, sptDynamic0)) {
			return;
		}

		CListMergeDlg dlg(this);
		dlg.m_eListType = mltSchedulerResources;
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (s.tullis 2014-12-02 10:51) - PLID 64125 - added scheduler mix rules
void CSchedulerSetupDlg::OnBtnScheduleMixRules()
{
	try{
		if (CheckCurrentUserPermissions(bioAdminScheduler, sptView | sptRead | sptWrite))
		{
			CSchedulingMixRulesDLG dlg(this);
			dlg.DoModal();
		}
	}NxCatchAll(__FUNCTION__)
}




