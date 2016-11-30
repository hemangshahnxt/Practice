// HL7BatchDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PracticeRc.h"
#include "HL7BatchDlg.h"
#include "HL7Utils.h"
#include "FileUtils.h"
#include "HL7LinkMultiplePatientsDlg.h"
//TES 4/17/2008 - PLID 29595 - Some declarations were moved to (sharable) HL7ParseUtils.
#include "HL7ParseUtils.h"
#include "HL7SettingsDlg.h"
#include "AuditTrail.h"
#include "RegUtils.h"
#include "HL7ExportDlg.h"
#include "MsgBox.h"
#include "FinancialRc.h"
#include "ExternalForm.h"
#include "HL7Client_Practice.h" // (z.manning 2013-05-20 11:07) - PLID 56777 - Renamed
#include <NxHL7Lib/HL7MessageFactory.h>
#include <NxHL7Lib/HL7CommonUtils.h>
#include "DecisionRuleUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (j.jones 2008-04-08 15:28) - PLID 29587 - created

/////////////////////////////////////////////////////////////////////////////
// CHL7BatchDlg dialog

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum ImportListColumns {

	ilcID = 0,
	ilcGroupID,
	ilcGroupName,
	ilcPatientName,
	ilcDescription,
	ilcAction,
	ilcMessageDate,		// (j.jones 2008-04-18 11:45) - PLID 21675 - added message date column
	ilcMessageType,
	ilcEventType,
	// (z.manning 2011-06-15 14:43) - PLID 40903 - Removed the message column
	//ilcMessage,
	ilcActionTaken,		//TES 5/12/2008 - PLID 13339 - Added columns describing what action was taken (for previously processed messages).
	ilcActionID,
	ilcInputDate,		//TES 10/16/2008 - PLID 31712 - Added a column for HL7MessageQueueT.InputDate
	ilcProvider,        // (s.tullis 2015-06-05 08:57) - PLID 66209 - Added Provider Column
	ilcLocation,		// (s.tullis 2015-06-19 09:23) - PLID 66210 - Added a Location Column 
};

enum ImportTypeFilterColumns {

	itfcMessageType = 0,
	itfcEventType,
	itfcDescription,
};

enum ExportListColumns {

	elcMessageID = 0,
	elcGroupID,
	elcPracticeRecordType,
	elcPracticeRecordID,
	// (z.manning 2011-06-15 14:43) - PLID 40903 - Removed the message column
	//elcMessage,
	elcMessageType,
	elcMessageTypeID, // (r.gonet 12/03/2012) - PLID 54114 - Added a column containing the message's HL7ExportMessageType
	elcDescription,
	elcCreateDate,		
	elcFlag, // (c.haag 2010-08-11 09:43) - PLID 39799
};

enum ExportGroupFilterColumns {
	
	egfcID = 0,
	egfcName,
};

CHL7BatchDlg::CHL7BatchDlg(CWnd* pParent)
	: CNxDialog(CHL7BatchDlg::IDD, pParent),
	m_HL7SettingsChecker(NetUtils::HL7SettingsT),
	m_HL7MessageQueueTChecker(NetUtils::HL7MessageQueueT),
	m_HL7MessageLogTChecker(NetUtils::HL7MessageLogT)
{
	//{{AFX_DATA_INIT(CHL7BatchDlg)
	m_bAutoProcessHL7Files = TRUE;
	m_bIsModal = false;
	//}}AFX_DATA_INIT
	// (c.haag 2010-08-11 09:43) - PLID 39799
	m_hIconRedX = (HICON)LoadImage(AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(IDI_X), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

	m_pdlLastRightClickedHL7List = NULL;
	m_bLastRightClickListWasImport = TRUE;

	// (r.gonet 12/03/2012) - PLID 54114 - Create the client that will be managing the sending of messages to NxServer.
	//  Tell it to ignore ACKs since that is legacy behavior for this dialog.
	m_pHL7Client = new CHL7Client_Practice();
	m_pHL7Client->SetIgnoreAcks(true);
	// (s.tullis 2015-06-02 16:09) - PLID 66211 - track if Columns Lengths have been changed
	m_bEditedDataListLength = FALSE;
}

CHL7BatchDlg::~CHL7BatchDlg()
{
	if (m_hIconRedX) DestroyIcon((HICON)m_hIconRedX);

	// (r.gonet 12/03/2012) - PLID 54114 - Now free up the memory reserved for the client.
	if(m_pHL7Client) {
		delete m_pHL7Client;
		m_pHL7Client = NULL;
	}
}


void CHL7BatchDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHL7BatchDlg)
	DDX_Control(pDX, IDC_BTN_EXPORT_PATIENTS, m_btnExportPatients);
	DDX_Control(pDX, IDC_SHOW_PROCESSED, m_nxbShowProcessed);
	DDX_Control(pDX, IDC_BTN_HL7_SETTINGS, m_btnConfigSettings);
	DDX_Control(pDX, IDC_BTN_CHECK_FOR_MESSAGES, m_btnCheckForNewMessages);
	DDX_Control(pDX, IDC_UNSELECT_ONE_HL7_IMPORT, m_btnUnselectOneImport);
	DDX_Control(pDX, IDC_UNSELECT_ONE_HL7_EXPORT, m_btnUnselectOneExport);
	DDX_Control(pDX, IDC_SELECT_ONE_HL7_IMPORT, m_btnSelectOneImport);
	DDX_Control(pDX, IDC_SELECT_ONE_HL7_EXPORT, m_btnSelectOneExport);
	DDX_Control(pDX, IDC_UNSELECT_ALL_HL7_IMPORT, m_btnUnselectAllImport);
	DDX_Control(pDX, IDC_UNSELECT_ALL_HL7_EXPORT, m_btnUnselectAllExport);
	DDX_Control(pDX, IDC_SELECT_ALL_HL7_IMPORT, m_btnSelectAllImport);
	DDX_Control(pDX, IDC_SELECT_ALL_HL7_EXPORT, m_btnSelectAllExport);
	DDX_Control(pDX, IDC_BTN_HL7_IMPORT, m_btnImport);
	DDX_Control(pDX, IDC_BTN_HL7_EXPORT, m_btnExport);
	DDX_Control(pDX, IDC_BTN_EXPORT_APPTS, m_btnExportAppts);
	DDX_Control(pDX, IDC_BTN_EXPORT_EMN_BILLS, m_btnExportEmnBills);
	DDX_Control(pDX, IDC_BTN_EXPORT_SYNDROMIC, m_btnExportSyndromic); // (a.walling 2010-02-22 10:45) - PLID 37154
	DDX_Control(pDX, IDC_BTN_EXPORT_EMNS, m_btnExportEMNs);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHL7BatchDlg, CNxDialog)
	//{{AFX_MSG_MAP(CHL7BatchDlg)
	ON_BN_CLICKED(IDC_SELECT_ONE_HL7_EXPORT, OnSelectOneHl7Export)
	ON_BN_CLICKED(IDC_SELECT_ALL_HL7_EXPORT, OnSelectAllHl7Export)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_HL7_EXPORT, OnUnselectOneHl7Export)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_HL7_EXPORT, OnUnselectAllHl7Export)
	ON_BN_CLICKED(IDC_SELECT_ONE_HL7_IMPORT, OnSelectOneHl7Import)
	ON_BN_CLICKED(IDC_SELECT_ALL_HL7_IMPORT, OnSelectAllHl7Import)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_HL7_IMPORT, OnUnselectOneHl7Import)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_HL7_IMPORT, OnUnselectAllHl7Import)
	ON_BN_CLICKED(IDC_BTN_HL7_EXPORT, OnBtnExportHL7Click)// (a.vengrofski 2010-05-12 12:12) - PLID <38547>
	ON_BN_CLICKED(IDC_BTN_HL7_IMPORT, OnBtnHl7Import)
	ON_BN_CLICKED(IDC_BTN_CHECK_FOR_MESSAGES, OnBtnCheckForMessages)
	ON_COMMAND(ID_IMPORT_COMMIT, OnImportCommit)
	ON_COMMAND(ID_IMPORT_DISMISS, OnImportDismiss)
	ON_COMMAND(ID_EXPORT_DISMISS, OnExportDismiss) // (r.gonet 02/26/2013) - PLID 47534
	ON_COMMAND(ID_VIEW_HL7_MESSAGE, OnViewHL7Message)
	ON_BN_CLICKED(IDC_BTN_HL7_SETTINGS, OnBtnHl7Settings)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_BN_CLICKED(IDC_SHOW_PROCESSED, OnShowProcessed)
	ON_BN_CLICKED(IDC_BTN_EXPORT_PATIENTS, OnBtnExportPatients)
	ON_BN_CLICKED(IDC_BTN_EXPORT_REFPHYS, OnBtnExportRefPhys)
	ON_BN_CLICKED(IDC_BTN_EXPORT_APPTS, OnBtnExportAppts)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_EXPORT_EMN_BILLS, &CHL7BatchDlg::OnBtnExportEmnBills)
	ON_BN_CLICKED(IDC_BTN_EXPORT_SYNDROMIC, &CHL7BatchDlg::OnBtnExportSyndromicData) // (a.walling 2010-02-22 10:46) - PLID 37154
	ON_BN_CLICKED(IDC_BTN_EXPORT_EMNS, &CHL7BatchDlg::OnBnClickedBtnExportEMNs)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CHL7BatchDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CHL7BatchDlg)
	ON_EVENT(CHL7BatchDlg, IDC_HL7_IMPORT_UNSELECTED_LIST, 3 /* DblClickCell */, OnDblClickCellHl7ImportUnselectedList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CHL7BatchDlg, IDC_HL7_IMPORT_SELECTED_LIST, 3 /* DblClickCell */, OnDblClickCellHl7ImportSelectedList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CHL7BatchDlg, IDC_HL7_EXPORT_UNSELECTED_LIST, 3 /* DblClickCell */, OnDblClickCellHl7ExportUnselectedList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CHL7BatchDlg, IDC_HL7_EXPORT_SELECTED_LIST, 3 /* DblClickCell */, OnDblClickCellHl7ExportSelectedList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CHL7BatchDlg, IDC_IMPORT_TYPE_FILTER, 16 /* SelChosen */, OnSelChosenImportTypeFilter, VTS_DISPATCH)
	ON_EVENT(CHL7BatchDlg, IDC_HL7_IMPORT_UNSELECTED_LIST, 6 /* RButtonDown */, OnRButtonDownHl7ImportUnselectedList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CHL7BatchDlg, IDC_HL7_IMPORT_SELECTED_LIST, 6 /* RButtonDown */, OnRButtonDownHl7ImportSelectedList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CHL7BatchDlg, IDC_HL7_EXPORT_GROUP_FILTER, 16 /* SelChosen */, OnSelChosenHl7ExportGroupFilter, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CHL7BatchDlg, IDC_HL7_EXPORT_UNSELECTED_LIST, 19, CHL7BatchDlg::LeftClickHl7ExportUnselectedList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CHL7BatchDlg, IDC_HL7_EXPORT_SELECTED_LIST, 19, CHL7BatchDlg::LeftClickHl7ExportSelectedList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CHL7BatchDlg, IDC_HL7_EXPORT_UNSELECTED_LIST, 6, CHL7BatchDlg::RButtonDownHl7ExportUnselectedList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CHL7BatchDlg, IDC_HL7_EXPORT_SELECTED_LIST, 6, CHL7BatchDlg::RButtonDownHl7ExportSelectedList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CHL7BatchDlg, IDC_HL7_EXPORT_UNSELECTED_LIST, 22, CHL7BatchDlg::ColumnSizingFinishedHl7List, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CHL7BatchDlg, IDC_HL7_EXPORT_SELECTED_LIST, 22, CHL7BatchDlg::ColumnSizingFinishedHl7List, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CHL7BatchDlg, IDC_HL7_IMPORT_UNSELECTED_LIST, 22, CHL7BatchDlg::ColumnSizingFinishedHl7List, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CHL7BatchDlg, IDC_HL7_IMPORT_SELECTED_LIST, 22, CHL7BatchDlg::ColumnSizingFinishedHl7List, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CHL7BatchDlg, IDC_HL7_EXPORT_UNSELECTED_LIST, 24, CHL7BatchDlg::FocusLostHl7List, VTS_NONE)
	ON_EVENT(CHL7BatchDlg, IDC_HL7_EXPORT_SELECTED_LIST, 24, CHL7BatchDlg::FocusLostHl7List, VTS_NONE)
	ON_EVENT(CHL7BatchDlg, IDC_HL7_IMPORT_SELECTED_LIST, 24, CHL7BatchDlg::FocusLostHl7List, VTS_NONE)
	ON_EVENT(CHL7BatchDlg, IDC_HL7_IMPORT_UNSELECTED_LIST, 24, CHL7BatchDlg::FocusLostHl7List, VTS_NONE)
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHL7BatchDlg message handlers

BOOL CHL7BatchDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {

		// (j.jones 2010-01-14 16:36) - PLID 31927 - added NewPatientsDefaultTextMessagePrivacy,
		// which is used when importing patients through HL7, might as well cache it here
		g_propManager.CachePropertiesInBulk("HL7BatchDlg-1", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'NewPatientsDefaultTextMessagePrivacy' "
			"	OR Name = 'HL7ImportNotifyMe_Patients' "
			"	OR Name = 'HL7ImportNotifyMe_Labs' "
			"	OR Name = 'HL7ImportNotifyMe_Bills' "
			"	OR Name = 'HL7ImportNotifyMe_Appointments' " // (z.manning 2010-06-29 17:30) - PLID 34572
			"	OR Name = 'HL7Import_FailedACKCheckDelaySeconds' " // (c.haag 2010-08-11 11:47) - PLID 39799
			"	OR Name = 'HL7ExportLockedEmns' " // (d.singleton 2012-12-13 14:29) - PLID 54047
			"	OR Name = 'HL7ImportNotifyMe_PatientDocuments' " //TES 3/14/2013 - PLID 55226
			"	OR Name = 'AssignNewPatientSecurityCode' " // (r.gonet 04/22/2014) - PLID 53170
			"   OR Name = 'DefaultLinksHL7ColumnSizes' "// (s.tullis 2015-06-02 16:09) - PLID 66211
			"	OR Name = 'HL7ImportNotifyMe_OpticalPrescriptions' " //TES 11/5/2015 - PLID 66371
			")",
			_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("HL7BatchDlg-2", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			// (j.jones 2011-06-24 12:24) - PLID 29885 - added DefaultMedicareSecondaryReasonCode
			"Name = 'DefaultMedicareSecondaryReasonCode' "
			")", _Q(GetCurrentUserName()));

		//auto-set our buttons
		{
			m_btnSelectOneExport.AutoSet(NXB_RIGHT);
			m_btnSelectAllExport.AutoSet(NXB_RRIGHT);
			m_btnUnselectOneExport.AutoSet(NXB_LEFT);
			m_btnUnselectAllExport.AutoSet(NXB_LLEFT);

			m_btnSelectOneImport.AutoSet(NXB_RIGHT);
			m_btnSelectAllImport.AutoSet(NXB_RRIGHT);
			m_btnUnselectOneImport.AutoSet(NXB_LEFT);
			m_btnUnselectAllImport.AutoSet(NXB_LLEFT);

			m_btnExport.AutoSet(NXB_EXPORT);
			m_btnImport.AutoSet(NXB_MODIFY);

			// (j.jones 2008-05-08 09:31) - PLID 29953 - changed more styles for modernization
			m_btnConfigSettings.AutoSet(NXB_MODIFY);
			m_btnCheckForNewMessages.AutoSet(NXB_MODIFY); //modify, because this actually imports data

			//TES 5/18/2009 - PLID 34282 - Check their permissions
			BOOL bCanExportPats = (GetCurrentUserPermissions(bioHL7Patients) & SPT______0_____ANDPASS);
			BOOL bCanExportAppts = (GetCurrentUserPermissions(bioHL7Appointments) & SPT______0_____ANDPASS);
			// (r.farnworth 2014-12-22 16:26) - PLID 64473 
			BOOL bCanExportRefPhys = (GetCurrentUserPermissions(bioHL7RefPhys) & SPT______0_____ANDPASS);
			//TES 7/10/2009 - PLID 34845 - Added EMN Bills
			BOOL bCanExportEmnBills = (g_pLicense->HasEMR(CLicense::cflrSilent) == 2) && ((GetCurrentUserPermissions(bioHL7EmnBills) & SPT______0_____ANDPASS));
			if(!bCanExportPats) {
				GetDlgItem(IDC_BTN_EXPORT_PATIENTS)->EnableWindow(FALSE);
			}
			if(!bCanExportAppts) {
				GetDlgItem(IDC_BTN_EXPORT_APPTS)->EnableWindow(FALSE);
			}
			if(!bCanExportEmnBills) {
				GetDlgItem(IDC_BTN_EXPORT_EMN_BILLS)->EnableWindow(FALSE);
			}
			if(!bCanExportPats && !bCanExportAppts && !bCanExportEmnBills) {
				GetDlgItem(IDC_BTN_HL7_EXPORT)->EnableWindow(FALSE);
			}
			if (!bCanExportRefPhys){
				GetDlgItem(IDC_BTN_EXPORT_REFPHYS)->EnableWindow(FALSE);
			}
			BOOL bCanImportPatients = (GetCurrentUserPermissions(bioHL7Patients) & SPT_______1____ANDPASS);
			BOOL bCanImportBills = (GetCurrentUserPermissions(bioHL7Bills) & SPT______0_____ANDPASS);
			BOOL bCanImportLabs = (GetCurrentUserPermissions(bioHL7Labs) & SPT______0_____ANDPASS);
			if(!bCanImportPatients && !bCanImportBills && !bCanImportLabs) {
				GetDlgItem(IDC_BTN_HL7_IMPORT)->EnableWindow(FALSE);
			}

			if(!(GetCurrentUserPermissions(bioHL7Settings) & SPT_V__________ANDPASS)) {
				m_btnConfigSettings.EnableWindow(FALSE);
			}

			// (d.singleton 2012-12-13 14:39) - PLID 54047 hide buttons for exporting locked emns by default
			BOOL bCanExportLockedEmns = GetRemotePropertyInt("HL7ExportLockedEmns", 0, 0, "<none>", true);
			if(!bCanExportLockedEmns) {
				m_btnExportEMNs.ShowWindow(SW_HIDE);
				m_btnExportEMNs.EnableWindow(FALSE);
			}
		}

		//set our datalists
		{
			//(e.lally 2008-10-08) PLID 31618 - Datalist2.0 controls need to use BindNxDataList2Ctrl
			m_ExportUnselectedList = BindNxDataList2Ctrl(IDC_HL7_EXPORT_UNSELECTED_LIST, false);
			m_ExportSelectedList = BindNxDataList2Ctrl(IDC_HL7_EXPORT_SELECTED_LIST, false);
			m_ImportUnselectedList = BindNxDataList2Ctrl(IDC_HL7_IMPORT_UNSELECTED_LIST, false);
			m_ImportSelectedList = BindNxDataList2Ctrl(IDC_HL7_IMPORT_SELECTED_LIST, false);

			m_ImportTypeFilterCombo = BindNxDataList2Ctrl(IDC_IMPORT_TYPE_FILTER, false);

			m_ExportGroupFilterCombo = BindNxDataList2Ctrl(IDC_HL7_EXPORT_GROUP_FILTER, true);
		}

		//try to select the first group, if we have a row yet
		IRowSettingsPtr pRow = m_ExportGroupFilterCombo->GetFirstRow();
		m_ExportGroupFilterCombo->PutCurSel(pRow);

		// (s.tullis 2015-06-02 16:09) - PLID 66211
		// Load Saved Datalist Columns Lengths 
		LoadDatalistColumnLength();

		//TES 6/24/2009 - PLID 34283 - If this is being popped up, UpdateView() won't be called by the framework, 
		// so we have to call it ourselves.
		if(m_bIsModal) {
			UpdateView();
		}
		}NxCatchAll("Error in CHL7BatchDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHL7BatchDlg::OnSelectOneHl7Export() 
{
	try {

		// (b.cardillo 2008-07-15 13:08) - PLID 30737 - Changed to use TakeCurrentRowAddSorted() for efficiency
		m_ExportSelectedList->TakeCurrentRowAddSorted(m_ExportUnselectedList, NULL);

	}NxCatchAll("Error in CHL7BatchDlg::OnSelectOneHl7Export");
}

void CHL7BatchDlg::OnSelectAllHl7Export() 
{
	try {

		m_ExportSelectedList->TakeAllRows(m_ExportUnselectedList);

	}NxCatchAll("Error in CHL7BatchDlg::OnSelectAllHl7Export");
}

void CHL7BatchDlg::OnUnselectOneHl7Export() 
{
	try {

		// (b.cardillo 2008-07-15 13:08) - PLID 30737 - Changed to use TakeCurrentRowAddSorted() for efficiency
		m_ExportUnselectedList->TakeCurrentRowAddSorted(m_ExportSelectedList, NULL);

	}NxCatchAll("Error in CHL7BatchDlg::OnUnselectOneHl7Export");
}

void CHL7BatchDlg::OnUnselectAllHl7Export() 
{
	try {

		m_ExportUnselectedList->TakeAllRows(m_ExportSelectedList);

	}NxCatchAll("Error in CHL7BatchDlg::OnUnselectAllHl7Export");
}

void CHL7BatchDlg::OnSelectOneHl7Import() 
{
	try {

		// (b.cardillo 2008-07-15 13:08) - PLID 30737 - Changed to use TakeCurrentRowAddSorted() for efficiency
		m_ImportSelectedList->TakeCurrentRowAddSorted(m_ImportUnselectedList, NULL);

	}NxCatchAll("Error in CHL7BatchDlg::OnSelectOneHl7Import");
}

void CHL7BatchDlg::OnSelectAllHl7Import() 
{
	try {

		m_ImportSelectedList->TakeAllRows(m_ImportUnselectedList);

	}NxCatchAll("Error in CHL7BatchDlg::OnSelectAllHl7Import");
}

void CHL7BatchDlg::OnUnselectOneHl7Import() 
{
	try {

		//we can't blindly move from selected to unselcted, incase the
		//selected list has types that are not part of the current "unselected"
		//filter

		CString strFilterMessageType = "";
		CString strFilterEventType = "";
		{
			IRowSettingsPtr pRow = m_ImportTypeFilterCombo->GetCurSel();
			if(pRow) {
				strFilterMessageType = VarString(pRow->GetValue(itfcMessageType),"");
				strFilterEventType = VarString(pRow->GetValue(itfcEventType),"");
			}
		}

		// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
		IRowSettingsPtr pCurSelRow = m_ImportSelectedList->GetFirstSelRow();
		while (pCurSelRow != NULL) {
			IRowSettingsPtr pRow = pCurSelRow;
			pCurSelRow = pCurSelRow->GetNextSelRow();

			//if it matches the import filter, move the row,
			//otherwise, remove the row

			CString strMessageType = VarString(pRow->GetValue(ilcMessageType),"");
			CString strEventType = VarString(pRow->GetValue(ilcEventType),"");
			if((strFilterMessageType.IsEmpty() || strFilterEventType.IsEmpty())
				|| (strFilterMessageType == strMessageType && strFilterEventType == strEventType)) {

				//it is in our filter (or we have no filter),
				//so move it to the unselected list				

				IRowSettingsPtr pRowToRemove = pRow;
				pRow = pRow->GetNextRow();

				// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method
				m_ImportUnselectedList->TakeRowAddSorted(pRowToRemove);
			}
			else {
				//it isn't in our filter, so just remove the row
				m_ImportSelectedList->RemoveRow(pRow);
			}
		}

	}NxCatchAll("Error in CHL7BatchDlg::OnUnselectOneHl7Import");
}

void CHL7BatchDlg::OnUnselectAllHl7Import() 
{
	try {

		//we can't blindly move from selected to unselcted, incase the
		//selected list has types that are not part of the current "unselected"
		//filter

		CString strFilterMessageType = "";
		CString strFilterEventType = "";
		{
			IRowSettingsPtr pRow = m_ImportTypeFilterCombo->GetCurSel();
			if(pRow) {
				strFilterMessageType = VarString(pRow->GetValue(itfcMessageType),"");
				strFilterEventType = VarString(pRow->GetValue(itfcEventType),"");
			}
		}

		IRowSettingsPtr pRow = m_ImportSelectedList->GetFirstRow();
		while(pRow) {

			//if it matches the import filter, move the row,
			//otherwise, remove the row

			CString strMessageType = VarString(pRow->GetValue(ilcMessageType),"");
			CString strEventType = VarString(pRow->GetValue(ilcEventType),"");
			if((strFilterMessageType.IsEmpty() || strFilterEventType.IsEmpty())
				|| (strFilterMessageType == strMessageType && strFilterEventType == strEventType)) {

				//it is in our filter (or we have no filter),
				//so move it to the unselected list

				IRowSettingsPtr pRowToRemove = pRow;
				pRow = pRow->GetNextRow();

				// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method
				m_ImportUnselectedList->TakeRowAddSorted(pRowToRemove);
			}
			else {
				//it isn't in our filter, so just remove the row
				IRowSettingsPtr pRowToRemove = pRow;
				pRow = pRow->GetNextRow();

				m_ImportSelectedList->RemoveRow(pRowToRemove);
			}
		}

	}NxCatchAll("Error in CHL7BatchDlg::OnUnselectAllHl7Import");
}

void CHL7BatchDlg::OnBtnHl7Export() 
{
	try {

		//TES 5/18/2009 - PLID 34282 - Check that they have permissions.
		//TES 7/10/2009 - PLID 34845 - Added EMN Bills
		bool bPatientsFound = false, bAppointmentsFound = false, bEmnBillsFound = false;
		IRowSettingsPtr pRow = m_ExportSelectedList->GetFirstRow();
		while(pRow && (!bPatientsFound || !bAppointmentsFound)) {
			HL7PracticeRecordType eRecordType = (HL7PracticeRecordType)VarLong(pRow->GetValue(elcPracticeRecordType));
			if(eRecordType == hprtPatient) bPatientsFound = true;
			else if(eRecordType == hprtAppt) bAppointmentsFound = true;
			else if(eRecordType == hprtEmnBill) bEmnBillsFound = true;
			pRow = pRow->GetNextRow();
		}
		if(bPatientsFound && !CheckCurrentUserPermissions(bioHL7Patients, sptDynamic0)) {
			return;
		}
		if(bAppointmentsFound && !CheckCurrentUserPermissions(bioHL7Appointments, sptDynamic0)) {
			return;
		}
		if(bEmnBillsFound && (g_pLicense->HasEMR(CLicense::cflrUse) != 2 || !CheckCurrentUserPermissions(bioHL7EmnBills, sptDynamic0))) {
			return;
		}

		// (j.jones 2008-05-07 08:52) - PLID 29598 - loop through all the selected
		// export messages, export them using the correct group settings, and
		// mark the messages as sent

		if(m_ExportSelectedList->GetRowCount() == 0) {
			AfxMessageBox("There are no messages in the Export Selected list. Please select some messages before exporting.");
			return;
		}

		if(IDNO == MessageBox("This will export all the pending HL7 messages in the Export Selected Records list. This action cannot be undone. "
			"Are you sure you wish to continue?",NULL, MB_YESNO)) {
			return;
		}

		// (z.manning 2011-06-15 15:09) - PLID 40903 - We no longer have the messages in the datalist, so let's load them now.
		CArray<long,long> arynMessageIDs;
		for(pRow = m_ExportSelectedList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
			long nMessageID = VarLong(pRow->GetValue(elcMessageID));
			arynMessageIDs.Add(nMessageID);
		}
		CMap<long,long,CString,LPCTSTR> mapMessageIDToMessage;
		GetHL7MessageLogMap(arynMessageIDs, mapMessageIDToMessage);

		bool bExportOneSucceeded = false;
		bool bExportOneFailed = false;
		// (r.gonet 12/03/2012) - PLID 54114 - In order to prevent really long exports that just error out at the end, 
		//  allow the user to abort early after the first failure.
		bool bPromptedForEarlyTermination = false;

		// (r.gonet 12/03/2012) - PLID 54114 - Iterate through the datalist and try to send each row's message.
		//  Leave the rows that are failures.
		pRow = m_ExportSelectedList->GetFirstRow();
		while(pRow) {
			long nMessageID = VarLong(pRow->GetValue(elcMessageID));
			long nGroupID = VarLong(pRow->GetValue(elcGroupID));
			HL7PracticeRecordType eRecordType = (HL7PracticeRecordType)VarLong(pRow->GetValue(elcPracticeRecordType));
			// (r.gonet 12/03/2012) - PLID 54114 - Get the message type because the HL7 Client uses that in reporting responses.
			HL7ExportMessageType eMessageType = (HL7ExportMessageType)VarLong(pRow->GetValue(elcMessageTypeID));
			NXDATALIST2Lib::IRowSettingsPtr pRemoveRow = NULL;

			// (z.manning 2011-06-15 15:12) - PLID 40903 - Get the message from the map we just made
			CString strMessage;
			if(!mapMessageIDToMessage.Lookup(nMessageID, strMessage)) {
				// (z.manning 2011-06-15 15:24) - PLID 40903 - Must have been deleted
				pRemoveRow = pRow;
			} else {
				
				// (r.gonet 12/03/2012) - PLID 54114 - Updated to use refactored function.
				// (r.gonet 12/09/2012) - PLID 54114 - Send the raw message to the client which
				//  will route it to NxServer and eventually send it to the third party.
				HL7ResponsePtr pResponse = m_pHL7Client->SendMessageToHL7(nMessageID, eRecordType, eMessageType, nGroupID);
				// (r.gonet 12/03/2012) - PLID 54114 - The response we get back contains the status
				if(pResponse->hmssSendStatus == hmssBatched || pResponse->hmssSendStatus == hmssSent || 
					pResponse->hmssSendStatus == hmssSent_NeedsAck || pResponse->hmssSendStatus == hmssSent_AckFailure ||pResponse->hmssSendStatus == hmssSent_AckReceived) 
				{
					// (r.gonet 12/03/2012) - PLID 54114 - We succeeded in sending. This dialog has never cared about the acknowledgement at the time of sending, just the 
					//  message's send status. The dialog will update to correctly show unacked messages later.
					bExportOneSucceeded = true;
					pRemoveRow = pRow;
				} else {
					bExportOneFailed = true;
					// (r.gonet 12/03/2012) - PLID 54114 - Okay, so we failed. If this is the first failure we encountered
					//  then ask the user if they want to continue. If there are 200 records to send here and the NxServer connection
					//  has been lost, then there is no point in waiting around for timeouts.
					if(!bPromptedForEarlyTermination) {
						int nMsgBoxResult = MessageBox(FormatString(
							"One of the messages has failed to export for the following reason: \r\n"
							"\r\n"
							"%s\r\n"
							"\r\n"
							"Do you want to continue this batch? ",
							pResponse->strErrorMessage), NULL,
							MB_YESNO|MB_ICONERROR);
						// (r.gonet 12/03/2012) - PLID 54114 - They were asked, don't ask again this session.
						bPromptedForEarlyTermination = true;
						if(nMsgBoxResult != IDYES) {
							// (r.gonet 12/03/2012) - PLID 54114 - They don't want to continue the batch, so break out of the loop early.
							break;
						}
					}
				}
			}

			pRow = pRow->GetNextRow();

			// (r.gonet 12/03/2012) - PLID 54114 - Remove the rows that have succeeded (or if the record was deleted)
			if(pRemoveRow != NULL) {
				m_ExportSelectedList->RemoveRow(pRemoveRow);
			}
		}

		//refresh the screen
		UpdateView();

		// (j.jones 2008-05-19 16:11) - PLID 30110 - send a tablechecker ourselves
		if(bExportOneSucceeded) {
			//update the entire table			
			m_HL7MessageLogTChecker.Refresh();
		}

		if(!bExportOneFailed) {
			AfxMessageBox("All messages were successfully exported.");
		}
		else {
			AfxMessageBox("Practice failed to export at least one message.");
		}
	}NxCatchAll("Error in CHL7BatchDlg::OnBtnHl7Export");
}

void CHL7BatchDlg::OnBtnHl7Import() 
{
	try {

		// (j.jones 2008-04-11 16:47) - PLID 29596 - this code was lifted and modified
		// from the old CHL7ImportDlg::OnCommitAllPending() function

		if(m_ImportSelectedList->GetRowCount() == 0) {
			AfxMessageBox("There are no messages in the Import Selected list. Please select some messages before importing.");
			return;
		}

		//TES 5/18/2009 - PLID 34282 - Make sure we have permission for all record types being imported
		IRowSettingsPtr pRow = m_ImportSelectedList->GetFirstRow();
		bool bPatientsFound = false, bBillsFound = false, bLabsFound = false, bApptsFound = false;
		while(pRow != NULL && (!bPatientsFound || !bBillsFound || !bLabsFound)) {
			CString strMessageType = VarString(pRow->GetValue(ilcMessageType),"");
			CString strEventType = VarString(pRow->GetValue(ilcEventType),"");
			HL7ImportRecordType hirt = GetRecordType(strMessageType, strEventType);
			if(hirt == hirtPatient) bPatientsFound = true;
			else if(hirt == hirtAppointment) bApptsFound = true; // (z.manning 2010-06-29 15:44) - PLID 34572
			else if(hirt == hirtBill) bBillsFound = true;
			else if(hirt == hirtLab) bLabsFound = true;
			pRow = pRow->GetNextRow();
		}
		if(bPatientsFound && !CheckCurrentUserPermissions(bioHL7Patients, sptDynamic1)) {
			return;
		}
		// (z.manning 2010-06-29 15:43) - PLID 34572
		if(bApptsFound && !CheckCurrentUserPermissions(bioHL7Appointments, sptDynamic1)) {
			return;
		}
		if(bBillsFound && !CheckCurrentUserPermissions(bioHL7Bills, sptDynamic0)) {
			return;
		}
		if(bLabsFound && !CheckCurrentUserPermissions(bioHL7Labs, sptDynamic0)) {
			return;
		}
		
		

		//(e.lally 2007-06-14) PLID 26326 - Decided to give a warning prior to running this code.
		if(IDNO == MessageBox("This will commit all the pending HL7 messages in the Import Selected Records list. This action cannot be undone. "
			"Are you sure you wish to continue?",NULL, MB_YESNO)) {
			return;
		}

		//TES 5/12/2008 - PLID 13339 - We need to give them an extra warning if there are rows in the list that have previously
		// been committed.
		BOOL bWarned = FALSE;
		pRow = m_ImportSelectedList->GetFirstRow();
		while(pRow != NULL && !bWarned) {
			long nActionID = VarLong(pRow->GetValue(ilcActionID),-1);
			if(nActionID == mqaCommit) {
				if(IDYES != MsgBox(MB_YESNO, "At least one selected message has previously been committed.  Re-committing selected messages may lead to duplicated records, "
					"or may overwrite changes to existing records.  Are you SURE you wish to do this?")) {
					return;
				}
				bWarned = TRUE;
			}
			pRow = pRow->GetNextRow();
		}

		// (z.manning 2011-06-15 15:09) - PLID 40903 - We no longer have the messages in the datalist, so let's load them now.
		CArray<long,long> arynMessageIDs;
		for(pRow = m_ImportSelectedList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
			long nMessageID = VarLong(pRow->GetValue(elcMessageID));
			arynMessageIDs.Add(nMessageID);
		}
		CMap<long,long,CString,LPCTSTR> mapMessageIDToMessage;
		GetHL7MessageQueueMap(arynMessageIDs, mapMessageIDToMessage);

		BOOL bCommitFailed = FALSE;
		BOOL bOneImported = FALSE;

		long nCountSkipped_NoBatchAllowed = 0;

		//TES 8/8/2007 - PLID 26892 - In the case of Update Patient messages, we need some special handling.  
		// Here's what we'll do: we go through each row once, and either successfully import the message, in which 
		// case we simply remove the row, fail to automatically link the patient, in which case we store the 
		// ThirdPartyID in the row and move on, or fail to commit the message entirely, in which case we leave the 
		// row as is.  Then we will prompt the user to link all the patients we couldn't auto-link, and then go back
		// through all the rows that have a ThirdPartyID stored and commit them based on the user's input, and remove
		// them from the list.  Then if there are any rows left in the list, we'll tell them that some failed, otherwise,
		// we'll report success.

		//TES 8/8/2007 - PLID 26892 - We need a separate dialog for each HL7 group.
		CArray<CHL7LinkMultiplePatientsDlg*,CHL7LinkMultiplePatientsDlg*> arLinkMultiples;

		//TES 10/31/2013 - PLID 59251 - Track if any CDS Interventions are triggered
		CDWordArray arNewCDSInterventions;

		//(e.lally 2007-06-14) PLID 26326 - Add ability to commit all pending messages.
		{
			IRowSettingsPtr pRow = m_ImportSelectedList->GetFirstRow();
			while(pRow) {
				CString strEventType = VarString(pRow->GetValue(ilcEventType), "");

				// (j.jones 2008-08-21 13:41) - PLID 29596 - The old import disallowed committing multiple
				// 'new bill' messages at once, but did so by disabling the Import button. Instead of doing
				// that,  just skip messages we cannot process in a batch, and warn at the end of the import
				// that this occurred.
				if(!IsCommitAllEventSupported(strEventType)) {

					nCountSkipped_NoBatchAllowed++;

					bCommitFailed = TRUE;

					pRow = pRow->GetNextRow();
					continue;
				}

				const long nMessageID = VarLong(pRow->GetValue(ilcID));

				// (z.manning 2011-06-15 15:12) - PLID 40903 - Get the message from the map we just made
				CString strMessage;
				if(!mapMessageIDToMessage.Lookup(nMessageID, strMessage)) {
					// (z.manning 2011-06-15 15:24) - PLID 40903 - Must have been deleted
					pRow = pRow->GetNextRow();
					continue;
				}

				//TES 5/8/2008 - PLID 29685 - Use the new HL7Message structure.
				HL7Message Message;
				Message.nMessageID = nMessageID;
				Message.strMessage = strMessage;
				Message.nHL7GroupID = VarLong(pRow->GetValue(ilcGroupID), -1);
				Message.strHL7GroupName = VarString(pRow->GetValue(ilcGroupName), "");
				Message.strPatientName = VarString(pRow->GetValue(ilcPatientName), "");
				//TES 10/16/2008 - PLID 31712 - Added a dtInputDate member
				Message.dtInputDate = VarDateTime(pRow->GetValue(ilcInputDate));

				// (r.gonet 05/01/2014) - PLID 61843 - We're beginning a new HL7 message transaction
				BeginNewHL7Transaction(GetRemoteData(), GetHL7SettingInt(Message.nHL7GroupID, "CurrentLoggingLevel"));

				BOOL bUpdatingPatients = FALSE;
				if(strEventType == "A08" || strEventType == "A31") {
					bUpdatingPatients = TRUE;
				}

				HL7_PIDFields PID;
				// (b.spivey, July 09, 2012) - PLID 49170 - Pass GetRemoteData() so the libs will have a connection pointer.
				ParsePIDSegment(Message.strMessage, Message.nHL7GroupID, PID, GetRemoteData());
				ENotFoundResult nfr = nfrFailure;
				//TES 9/18/2008 - PLID 31414 - Renamed
				long nPersonID = GetPatientFromHL7Message(PID, Message.nHL7GroupID, this, nfbSkip, &nfr);
				if(bUpdatingPatients && nPersonID == -1 && nfr == nfrSkipped) {
					//TES 8/8/2007 - PLID 26892 - Find the dialog for this group (creating it if necessary), and add
					// this message to it.
					CString strGroupName = VarString(pRow->GetValue(ilcGroupName), "");
					bool bFound = false;
					for(int nDlg = 0; nDlg < arLinkMultiples.GetSize() && !bFound; nDlg++) {
						if(arLinkMultiples[nDlg]->m_strHL7GroupName == strGroupName) {
							bFound = true;
							arLinkMultiples[nDlg]->AddPID(PID);
						}
					}
					if(!bFound) {
						CHL7LinkMultiplePatientsDlg* pDlg = new CHL7LinkMultiplePatientsDlg(this);
						pDlg->m_strHL7GroupName = strGroupName;
						pDlg->AddPID(PID);
						arLinkMultiples.Add(pDlg);
					}
				}
				else {
					//It would be more efficient to commit the HL7 event all at once, not one at a time, but that is
					//not the primary concern here
					//TES 4/21/2008 - PLID 29721 - Added parameters for auditing.
					// (j.jones 2008-05-19 16:11) - PLID 30110 - don't auto-send an HL7 tablechecker, we will do it ourselves
					//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
					if(CommitHL7Event(Message, this, FALSE, arNewCDSInterventions)) {
						//it succeeded!  Now we need to update the data to have the correct action
						//TES 8/8/2007 - PLID 27020 - Go ahead and commit this data now, it's not much slower
						// that way, and doing that ensures that if there's a crash partway through, we'll still
						// know which messages were processed already.
						/*strTempSql.Format("UPDATE HL7MessageQueueT SET ActionID = %li WHERE ID = %li; \r\n", mqaCommit, VarLong(pRow->GetValue(mqcID)));
						strSql += strTempSql;*/
						//TES 4/18/2008 - PLID 29657 - CommitHL7Event updates HL7MessageQueueT now.
						//ExecuteParamSql("UPDATE HL7MessageQueueT SET ActionID = {INT} WHERE ID = {INT}; \r\n", mqaCommit, nID);

						bOneImported = TRUE;

						//now remove the row
						//TES 5/12/2008 - PLID 13339 - Don't remove it if we're showing previously processed messages.
						if(IsDlgButtonChecked(IDC_SHOW_PROCESSED)) {
							pRow->PutValue(ilcActionID, (long)mqaCommit);
							pRow->PutValue(ilcActionTaken, _bstr_t("Committed"));
						}
						else {
							IRowSettingsPtr pRowToRemove = pRow;
							pRow = pRow->GetNextRow();

							m_ImportSelectedList->RemoveRow(pRowToRemove);
							continue;
						}

					}
					else {
						bCommitFailed = TRUE;
					}
				}

				pRow = pRow->GetNextRow();
			}
		}

		//TES 8/8/2007 - PLID 26892 - Go through each of our dialogs and display them for the user, then go through
		// our list and either create new records if they asked us to, or filling in the HL7IDLinkT with the mapped ID
		// they chose, and then committing the original Update event.
		for(int nDlg = 0; nDlg < arLinkMultiples.GetSize(); nDlg++)
		{
			CHL7LinkMultiplePatientsDlg *pDlg = arLinkMultiples[nDlg];
			if(pDlg->GetPatientCount() > 0)
			{
				if(IDOK == pDlg->DoModal())
				{
					//OK, they linked all the patients.  Let's go through our list and commit everything.
					IRowSettingsPtr pRow = m_ImportSelectedList->GetFirstRow();
					while(pRow)
					{
						const long nMessageID = VarLong(pRow->GetValue(ilcID));

						// (z.manning 2011-06-15 15:12) - PLID 40903 - Get the message from the map we just made
						CString strMessage;
						if(!mapMessageIDToMessage.Lookup(nMessageID, strMessage)) {
							// (z.manning 2011-06-15 15:24) - PLID 40903 - Must have been deleted
							pRow = pRow->GetNextRow();
							continue;
						}

						//TES 5/8/2008 - PLID 29685 - Use the new HL7Message structure.
						HL7Message Message;
						Message.nMessageID = nMessageID;
						Message.strMessage = strMessage;
						Message.nHL7GroupID = VarLong(pRow->GetValue(ilcGroupID), -1);
						Message.strHL7GroupName = VarString(pRow->GetValue(ilcGroupName), "");
						Message.strPatientName = VarString(pRow->GetValue(ilcPatientName), "");
						//TES 10/16/2008 - PLID 31712 - Added a dtInputDate member.
						Message.dtInputDate = VarDateTime(pRow->GetValue(ilcInputDate));
						CString strEventType = VarString(pRow->GetValue(ilcEventType), "");

						// (r.gonet 05/01/2014) - PLID 61843 - We're starting a new HL7 message transaction
						BeginNewHL7Transaction(GetRemoteData(), GetHL7SettingInt(Message.nHL7GroupID, "CurrentLoggingLevel"));
						
						if(Message.strHL7GroupName == pDlg->m_strHL7GroupName) {
							//This message is one of the ones that just got linked. Pull its HL7 ID, which should
							// be in the dialog's map.
							HL7_PIDFields PID;
							// (b.spivey, July 09, 2012) - PLID 49170 - Pass GetRemoteData() so the libs will have a connection pointer.
							ParsePIDSegment(Message.strMessage, Message.nHL7GroupID, PID, GetRemoteData());
							LPVOID pPersonID = NULL;
							BOOL bSuccess = pDlg->m_mapHL7IDToNextechID.Lookup(PID.strHL7ID, pPersonID);
							long nPersonID = (long)pPersonID;
							//It shouldn't be possible for this HL7 ID to not be in the map.
							ASSERT(bSuccess);
							if(!bSuccess) {
								bCommitFailed = TRUE;
							}
							else {
								//OK, we've got the person ID that they wanted to link to.
								if(nPersonID == -1) {
									//They wanted us to create new, so let's do that.
									long nNewPersonID = -1;
									// (j.jones 2008-05-19 16:11) - PLID 30110 - don't send an HL7 tablechecker,
									// because we will refresh the table when the import completes
									if(!AddHL7PatientToData(Message, FALSE, &nNewPersonID)) {
										bCommitFailed = TRUE;
									}
									else {

										//it succeeded!  Now we need to update the data to have the correct action
										//TES 8/9/2007 - PLID 27020 - Go ahead and commit this data now, it's not much slower
										// that way, and doing that ensures that if there's a crash partway through, we'll still
										// know which messages were processed already.
										/*strTempSql.Format("UPDATE HL7MessageQueueT SET ActionID = %li WHERE ID = %li; \r\n", mqaCommit, VarLong(pRow->GetValue(mqcID)));
										strSql += strTempSql;*/
										/*ExecuteParamSql("UPDATE HL7MessageQueueT SET ActionID = {INT} WHERE ID = {INT}; \r\n", mqaCommit, Message.nMessageID);

										//TES 4/21/2008 - PLID 29721 - Audit that we've processed this message.
										AuditEvent(PID.strFirst + (PID.strMiddle.IsEmpty() ? "" : " ") + PID.strMiddle + " " + PID.strLast,
											BeginNewAuditEvent(), aeiHL7MessageProcessed, nID, "Pending (" + strGroupName + " - " + GetActionDescriptionForHL7Event(strMessage, nGroupID) + ")",
											"Committed", aepMedium, aetChanged);*/

										bOneImported = TRUE;

										//TES 11/1/2007 - PLID 26892 - Also, we need to update the map, so that any
										// ensuing messages for this patient don't create more new records.
										pDlg->m_mapHL7IDToNextechID.SetAt(PID.strHL7ID, (LPVOID)nNewPersonID);

										//now remove the row
										//TES 5/12/2008 - PLID 13339 - Don't remove it if we're showing previously processed messages.
										if(IsDlgButtonChecked(IDC_SHOW_PROCESSED)) {
											pRow->PutValue(ilcActionID, (long)mqaCommit);
											pRow->PutValue(ilcActionTaken, _bstr_t("Committed"));
										}
										else {
											IRowSettingsPtr pRowToRemove = pRow;
											pRow = pRow->GetNextRow();

											m_ImportSelectedList->RemoveRow(pRowToRemove);
											continue;
										}
									}
								}
								else {
									//OK, fill the link in our table, then commit the event (which will now succeed, 
									// because it will find the ID in this table).

									// (j.jones 2008-04-17 11:30) - PLID 27244 - TryCreateHL7IDLinkRecord will create
									// a HL7IDLinkT record only if there isn't an exact match already
									//TES 9/18/2008 - PLID 31414 - This now needs to know what type of record we're linking.
									// (j.jones 2010-05-12 16:48) - PLID 36527 - now returns TRUE if a new HL7IDLinkT record was created,
									// so we can audit ourselves (this function does not audit)
									//TES 1/3/2011 - PLID 40744 - Pass in first and last name
									// (d.thompson 2013-01-21) - PLID 54732 - Start tracking assigning authority
									if(TryCreateHL7IDLinkRecord(GetRemoteData(), Message.nHL7GroupID, PID.strHL7ID, nPersonID, hilrtPatient, PID.strFirst, PID.strLast, PID.strAssigningAuthority)) {

										// (j.jones 2010-05-12 16:51) - PLID 36527 - audit the HL7IDLinkT creation
										CString strOld, strNew, strPatientName;
										strPatientName = GetExistingPatientName(nPersonID);
										strOld.Format("Patient Code '%s' (HL7 Group '%s')", PID.strHL7ID, Message.strHL7GroupName);
										strNew.Format("Linked to Patient '%s'", strPatientName);

										long nAuditID = BeginNewAuditEvent();
										AuditEvent(nPersonID, strPatientName, nAuditID, aeiHL7PatientLink, nPersonID, strOld, strNew, aepLow, aetCreated);
									}

									//It would be more efficient to commit the HL7 event all at once, not one at a time, but that is
										//not the primary concern here
									//TES 4/21/2008 - PLID 29721 - Added parameters for auditing.
									// (j.jones 2008-05-19 16:11) - PLID 30110 - don't send an HL7 tablechecker,
									// because we will refresh the table when the import completes
									//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
									if(CommitHL7Event(Message, this, FALSE, arNewCDSInterventions)) {
										//it succeeded!  Now we need to update the data to have the correct action
										//TES 8/9/2007 - PLID 27020 - We will commit the message right away.  We're already committing each
										// event individually, and this is less of an impact than that.  Plus, at this point, the message is
										// in fact committed, so the data should reflect that, even if something (like a crash) happens between
										// now and the end of this loop.
										/*strTempSql.Format("UPDATE HL7MessageQueueT SET ActionID = %li WHERE ID = %li; \r\n", mqaCommit, VarLong(pRow->GetValue(mqcID)));
										strSql += strTempSql;*/
										//TES 4/18/2008 - PLID 29657 - CommitHL7Event updates HL7MessageQueueT now.
										//ExecuteParamSql("UPDATE HL7MessageQueueT SET ActionID = {INT} WHERE ID = {INT}; \r\n", mqaCommit, nID);

										bOneImported = TRUE;

										//now remove the row
										//TES 5/12/2008 - PLID 13339 - Don't remove it if we're showing previously processed messages.
										if(IsDlgButtonChecked(IDC_SHOW_PROCESSED)) {
											pRow->PutValue(ilcActionID, (long)mqaCommit);
											pRow->PutValue(ilcActionTaken, _bstr_t("Committed"));
										}
										else {
											IRowSettingsPtr pRowToRemove = pRow;
											pRow = pRow->GetNextRow();

											m_ImportSelectedList->RemoveRow(pRowToRemove);
											continue;
										}
										
									}
									else {
										bCommitFailed = TRUE;
									}
								}
							}
						}
						pRow = pRow->GetNextRow();
					}
				}
				else {
					bCommitFailed = TRUE;
				}
			}
			//TES 8/9/2007 - PLID 26892 - Cleanup.
			delete pDlg;
		}
		//TES 8/9/2007 - PLID 26892 - Remove all the (now-dangling) pointers from this array.
		arLinkMultiples.RemoveAll();

		//refresh the screen to remove any committed rows
		UpdateView();

		// (j.jones 2008-05-19 16:11) - PLID 30110 - send a tablechecker ourselves
		//refresh all messages
		m_HL7MessageQueueTChecker.Refresh();

		if(bCommitFailed == FALSE) {
			AfxMessageBox("All messages were successfully committed.");
		}
		else {

			// (j.jones 2008-08-21 13:41) - PLID 29596 - warn if any were skipped because
			// they could not be committed in a batch
			if(nCountSkipped_NoBatchAllowed > 0) {

				CString strWarn;
				strWarn.Format("Practice skipped %li messages because 'Create Bill' messages cannot be created in a batch.\n"
					"You must right click on each 'Create Bill' row to create the bills one at a time.", nCountSkipped_NoBatchAllowed);
				AfxMessageBox(strWarn);
			}
			else {
				//give the standard message
				AfxMessageBox("Practice failed to commit at least one message.");
			}
		}

		//TES 10/31/2013 - PLID 59251 - If any interventions were triggered, notify the user
		GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);

	}NxCatchAll("Error in CHL7BatchDlg::OnBtnHl7Import");
}

void CHL7BatchDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {

		//see if the group list changed, as we do not respond to those changes real-time
		if(m_HL7SettingsChecker.Changed()) {

			long nID = -1;
			{
				IRowSettingsPtr pRow = m_ExportGroupFilterCombo->GetCurSel();

				if(pRow) {
					nID = VarLong(pRow->GetValue(egfcID), -1);
				}
			}
			
			m_ExportGroupFilterCombo->Requery();
			
			IRowSettingsPtr pRow = NULL;
			if(nID != -1) {
				pRow = m_ExportGroupFilterCombo->SetSelByColumn(egfcID, nID);
			}

			if(pRow == NULL) {
				//try to select the first row, if we have one
				pRow = m_ExportGroupFilterCombo->GetFirstRow();
				if(pRow == NULL) {
					//no row available, so ensure we have all groups loaded
					//(there will never be more than just a couple)
					m_ExportGroupFilterCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
					pRow = m_ExportGroupFilterCombo->GetFirstRow();
				}
				m_ExportGroupFilterCombo->PutCurSel(pRow);
			}
		}

		// (j.jones 2008-04-11 15:53) - PLID 29596 - first process any pending HL7 files,
		// if our boolean is set to do so
		if(m_bAutoProcessHL7Files) {
			//turn bSilent on, so we won't bother the user
			ProcessHL7Files(TRUE);

			//ProcessHL7Files will requery the import filter and lists upon completion
		}
		else {
			RequeryImportFilterTypes();
			RequeryImportLists();
		}

		RequeryExportLists();

		// (j.jones 2008-04-23 14:24) - PLID 29597 - remove any notifications we may have
		// regarding HL7, since they are at the batch tab now
		if(GetMainFrame()) {
			GetMainFrame()->UnNotifyUser(NT_HL7);
		}

	}NxCatchAll("Error in CHL7BatchDlg::UpdateView");
}

void CHL7BatchDlg::RequeryImportFilterTypes()
{
	//we only really need to see the ones that are available, so pull from the data
	try {

		//find the current selection

		CString strCurMsg = "";
		CString strCurEvent = "";

		{
			IRowSettingsPtr pRow = m_ImportTypeFilterCombo->GetCurSel();
			if(pRow) {
				strCurMsg = VarString(pRow->GetValue(itfcMessageType),"");
				strCurEvent = VarString(pRow->GetValue(itfcEventType),"");
			}
		}

		m_ImportTypeFilterCombo->Clear();

		//TES 5/20/2008 - PLID 13339 - Now that we can show previously processed messages, include their types.
		// (a.vengrofski 2010-07-28 11:23) - PLID <38919> - Removed ORU R01 labs from showing up in this list
		// (d.singleton 2013-01-28 15:50) - PLID 54895 do not filter out ORU^R01 IF its from a history documents link
		_RecordsetPtr rs = CreateParamRecordset("SELECT MessageType, EventType FROM HL7MessageQueueT "
												"WHERE ( MessageType <> 'ORU' AND EventType <> 'R01') OR "
												"GroupID IN ( SELECT HL7GroupID FROM HL7GenericSettingsT WHERE Name = 'UsePatientHistoryImportMSH_3' AND BitParam = 1 ) "
												"GROUP BY MessageType, EventType");

		while(!rs->eof) {
			CString strMsg, strEvent, strDesc;

			strMsg = AdoFldString(rs, "MessageType");
			strEvent = AdoFldString(rs, "EventType");

			// (d.singleton 2013-01-28 15:51) - PLID 54895 if we have an ORU^R01 message and we get this far its deff a history doc link
			strDesc = GetActionDescriptionForHL7Event(strMsg, strEvent, TRUE);

			//now put it all in the datalist
			IRowSettingsPtr pRow = m_ImportTypeFilterCombo->GetNewRow();
			pRow->PutValue(itfcMessageType, _bstr_t(strMsg));
			pRow->PutValue(itfcEventType, _bstr_t(strEvent));
			pRow->PutValue(itfcDescription, _bstr_t(strDesc));
			m_ImportTypeFilterCombo->AddRowSorted(pRow, NULL);

			rs->MoveNext();
		}
		rs->Close();

		//add an "all actions" option
		IRowSettingsPtr pAllRow = m_ImportTypeFilterCombo->GetNewRow();
		pAllRow->PutValue(itfcMessageType, _bstr_t(""));
		pAllRow->PutValue(itfcEventType, _bstr_t(""));
		pAllRow->PutValue(itfcDescription, _bstr_t("<Show All Actions>"));
		m_ImportTypeFilterCombo->AddRowSorted(pAllRow, NULL);

		//if we had a previous selection, re-select it
		BOOL bFound = FALSE;
		if(!strCurMsg.IsEmpty() && !strCurEvent.IsEmpty()) {
			IRowSettingsPtr pRow = m_ImportTypeFilterCombo->GetFirstRow();
			while(pRow != NULL && !bFound) {

				CString strMsg = VarString(pRow->GetValue(itfcMessageType),"");
				CString strEvent = VarString(pRow->GetValue(itfcEventType),"");

				if(strMsg == strCurMsg && strEvent == strCurEvent) {
					//found the matching row
					bFound = TRUE;
					m_ImportTypeFilterCombo->PutCurSel(pRow);
				}

				pRow = pRow->GetNextRow();
			}
		}
		
		//if no previous selection was found, select the "all actions" row
		if(!bFound) {
			m_ImportTypeFilterCombo->PutCurSel(pAllRow);
		}

	} NxCatchAll("Error in RequeryImportTypes()");
}


void CHL7BatchDlg::RequeryImportLists()
{
	try {
		
		//first cache an array of IDs in our selected list
		CArray<long, long> arySelectedIDs;
		//and a CString, comma-delimited
		CString strSelectedIDs;

		//populate the array and string
		{
			IRowSettingsPtr pRow = m_ImportSelectedList->GetFirstRow();
			while(pRow) {

				long nMessageID = VarLong(pRow->GetValue(ilcID));
				arySelectedIDs.Add(nMessageID);

				if(!strSelectedIDs.IsEmpty()) {
					strSelectedIDs += ",";
				}
				strSelectedIDs += AsString(nMessageID);

				pRow = pRow->GetNextRow();
			}
		}

		//now clear both lists
		m_ImportUnselectedList->Clear();
		m_ImportSelectedList->Clear();

		//TES 5/12/2008 - PLID 13339 - Check whether we're showing messages that we've already processed.
		CSqlFragment sqlShowProcessed;
		if(!IsDlgButtonChecked(IDC_SHOW_PROCESSED)) {
			sqlShowProcessed.Create(" ActionID Is Null ");
		}
		else {
			sqlShowProcessed.Create(" 1=1 ");
		}
		
		//find our type filter
		CSqlFragment sqlTypeFilter;
		IRowSettingsPtr pRow = m_ImportTypeFilterCombo->GetCurSel();
		if(pRow) {
			CString strMessageType = VarString(pRow->GetValue(itfcMessageType),"");
			CString strEventType = VarString(pRow->GetValue(itfcEventType),"");
			if(!strMessageType.IsEmpty() && !strEventType.IsEmpty()) {

				CSqlFragment sqlSelectedIDsWhere;
				if(!strSelectedIDs.IsEmpty()) {
					//do not filter the "selected" values
					sqlSelectedIDsWhere = CSqlFragment(" OR HL7MessageQueueT.ID IN ({INTSTRING}) ", strSelectedIDs);
				}

				sqlTypeFilter.Create(" AND ((MessageType = {STRING} AND EventType = {STRING}) {SQL})", strMessageType, strEventType, sqlSelectedIDsWhere);
			}
		}


		//pull all available messages from the data that match our filter
		//can't parameterize the where clause here
		// (a.vengrofski 2010-07-28 11:29) - PLID <38919> - Removed Lab Results from showing up here
		// (z.manning 2011-06-15 12:50) - PLID 40903 - Parameterized
		// (d.singleton 2013-01-28 15:48) - PLID 54895 check the group setting and if enabled do now filter out the ORU^R01 messages ( as they are history documents and not lab results )
		// (r.goldschmidt 2015-11-02 12:01) - PLID 66437 - We need to know if the message was purged
		// use a server-side cursor so it doesn't try to load up massive amounts of memory for the messages
		_RecordsetPtr rs = ::CreateParamRecordset(GetRemoteDataSnapshot(), ADODB::adUseServer, ADODB::adOpenForwardOnly, ADODB::adLockReadOnly, ADODB::adCmdText,
			"SELECT HL7MessageQueueT.ID, GroupID, HL7SettingsT.Name AS GroupName, PatientName, Description, \r\n"
			"	MessageType, EventType, ActionID, InputDate,HL7MessageQueueT.Message, \r\n"
			"	{SQL} AS MessageDateString, \r\n"
			"	CASE WHEN HL7MessageQueueT.PurgeDate IS NULL THEN CONVERT(BIT, 0) ELSE CONVERT(BIT, 1) END AS WasPurged \r\n"
			"FROM HL7MessageQueueT \r\n"
			"INNER JOIN HL7SettingsT ON HL7MessageQueueT.GroupID = HL7SettingsT.ID \r\n"
			"WHERE {SQL} {SQL} AND ((MessageType <> 'ORU' AND EventType <> 'R01') OR GroupID IN ( SELECT HL7GroupID FROM HL7GenericSettingsT WHERE Name = 'UsePatientHistoryImportMSH_3' AND BitParam = 1 )) \r\n"
			, GetMessageDateStringSql(), sqlShowProcessed, sqlTypeFilter);
		while(!rs->eof) {
			
			long nID = AdoFldLong(rs, "ID", -1);
			long nGroupID = AdoFldLong(rs, "GroupID", -1);
			CString strGroupName = AdoFldString(rs, "GroupName", "");
			CString strPatientName = AdoFldString(rs, "PatientName", "");
			CString strDescription = AdoFldString(rs, "Description", "");
			CString strMsgType = AdoFldString(rs, "MessageType", "");
			CString strEvent = AdoFldString(rs, "EventType", "");
			long nActionID = AdoFldLong(rs, "ActionID", -1);
			//TES 10/16/2008 - PLID 31712 - We'll also need the input date
			COleDateTime dtInput = AdoFldDateTime(rs, "InputDate");
			CString strMessageDateString = AdoFldString(rs, "MessageDateString", "");
			// (s.tullis 2015-06-05 08:57) - PLID 66209 - Parse Provider from the HL7 Message
			CString strProvider = GetProviderFromHL7Message(AdoFldString(rs, "Message", ""), AdoFldLong(rs, "GroupID", -1));
			// (s.tullis 2015-06-19 09:23) - PLID 66210 - Parse the Location  
			// (j.gruber 2016-01-29 14:54) - PLID 68000 - added because moved to HL7ParseUtils
			CString strLocation = GetLocationFromHL7Message(AdoFldString(rs, "Message", ""), AdoFldLong(rs, "GroupID", -1),
				GetHL7SettingBit(nGroupID, "EnableIntelleChart"), GetHL7SettingBit(nGroupID, "UseImportedLocation"));
			//TES 5/12/2008 - PLID 13339 - We need to fill in the "Action Taken" column.
			// (d.singleton 2013-01-28 15:49) - PLID 54895 if we have an ORU^R01 message and we got this far its deff a history document link
			CString strAction = GetActionDescriptionForHL7Event(strMsgType, strEvent, TRUE);
			CString strActionTaken = GetMessageActionDescription(nActionID);

			// (j.jones 2008-04-18 11:45) - PLID 21675 - added message date column
			// (z.manning 2011-06-15 16:02) - PLID 40903 - We no longer need the whole message to get the message date
			// (r.goldschmidt 2015-11-02 12:01) - PLID 66437 - If the message was purged, set the message date to the input date.
			//								Although this isn't the true message date, it is close enough for the purposes of info/sorting
			COleDateTime dtMessageDate;
			if (!!AdoFldBool(rs, "WasPurged")) {
				dtMessageDate = AdoFldDateTime(rs, "InputDate");
			}
			else {
				dtMessageDate = GetHL7DateFromStringField(strMessageDateString, nGroupID);
				if (dtMessageDate.m_status == COleDateTime::invalid) {
					//GetHL7MessageDate should never have sent back an invalid date
					ASSERT(FALSE);
					dtMessageDate = COleDateTime::GetCurrentTime();
				}
			}

			//add to the datalist - but which one?

			//was this previously in the selected list?
			BOOL bFound = FALSE;
			for(int i=arySelectedIDs.GetSize()-1;i>=0 && !bFound;i--) {
				if(arySelectedIDs.GetAt(i) == nID) {

					bFound = TRUE;

					//it needs to go back into the selected list
					IRowSettingsPtr pRow = m_ImportSelectedList->GetNewRow();
					pRow->PutValue(ilcID, nID);
					pRow->PutValue(ilcGroupID, nGroupID);
					pRow->PutValue(ilcGroupName, _bstr_t(strGroupName));
					pRow->PutValue(ilcPatientName, _bstr_t(strPatientName));
					pRow->PutValue(ilcDescription, _bstr_t(strDescription));
					pRow->PutValue(ilcAction, _bstr_t(strAction));
					// (j.jones 2008-04-18 11:45) - PLID 21675 - added message date column
					pRow->PutValue(ilcMessageDate, _variant_t(dtMessageDate, VT_DATE));
					pRow->PutValue(ilcMessageType, _bstr_t(strMsgType));					
					pRow->PutValue(ilcEventType, _bstr_t(strEvent));
					//TES 5/12/2008 - PLID 13339 - Added two new columns for information about previously processed messages.
					pRow->PutValue(ilcActionTaken, _bstr_t(strActionTaken));
					pRow->PutValue(ilcActionID, nActionID);
					//TES 10/16/2008 - PLID 31712 - Added a column for HL7MessageQueueT.InputDate
					pRow->PutValue(ilcInputDate, COleVariant(dtInput));
					// (s.tullis 2015-06-05 08:57) - PLID 66209 - Added Provider Column 
					pRow->PutValue(ilcProvider, _bstr_t(strProvider));
					// (s.tullis 2015-06-19 09:23) - PLID 66210 - Added a Location Column 
					pRow->PutValue(ilcLocation, _bstr_t(strLocation));
					m_ImportSelectedList->AddRowSorted(pRow, NULL);

					//remove this entry to make the search faster next time
					arySelectedIDs.RemoveAt(i);
				}
			}

			//if it wasn't in the selected list, add to the unselected list
			if(!bFound) {
				IRowSettingsPtr pRow = m_ImportUnselectedList->GetNewRow();
				pRow->PutValue(ilcID, nID);
				pRow->PutValue(ilcGroupID, nGroupID);
				pRow->PutValue(ilcGroupName, _bstr_t(strGroupName));
				pRow->PutValue(ilcPatientName, _bstr_t(strPatientName));
				pRow->PutValue(ilcDescription, _bstr_t(strDescription));
				pRow->PutValue(ilcAction, _bstr_t(strAction));
				// (j.jones 2008-04-18 11:45) - PLID 21675 - added message date column
				pRow->PutValue(ilcMessageDate, _variant_t(dtMessageDate, VT_DATE));
				pRow->PutValue(ilcMessageType, _bstr_t(strMsgType));
				pRow->PutValue(ilcEventType, _bstr_t(strEvent));
				//TES 5/12/2008 - PLID 13339 - Added two new columns for information about previously processed messages.
				pRow->PutValue(ilcActionTaken, _bstr_t(strActionTaken));
				pRow->PutValue(ilcActionID, nActionID);
				//TES 10/16/2008 - PLID 31712 - Added a column for HL7MessageQueueT.InputDate
				pRow->PutValue(ilcInputDate, COleVariant(dtInput));
				// (s.tullis 2015-06-05 08:57) - PLID 66209 - added provider column
				pRow->PutValue(ilcProvider, _bstr_t(strProvider));
				// (s.tullis 2015-06-19 09:23) - PLID 66210 - Added a Location Column 
				pRow->PutValue(ilcLocation, _bstr_t(strLocation));
				m_ImportUnselectedList->AddRowSorted(pRow, NULL);
			}

			rs->MoveNext();
		}
		rs->Close();

		//clear the array of selected IDs - if this array is non-empty, it
		//means that an item was previously in the selected list but is no
		//no longer available, it may have been imported by another user
		arySelectedIDs.RemoveAll();

	}NxCatchAll("Error in CHL7BatchDlg::RequeryImportLists");
}

void CHL7BatchDlg::RequeryExportLists()
{
	try {

		//find all records with a SentDate that is NULL

		//first cache an array of IDs in our selected list
		CArray<long, long> arySelectedIDs;
		//and a CString, comma-delimited
		CString strSelectedIDs;

		//populate the array and string
		{
			IRowSettingsPtr pRow = m_ExportSelectedList->GetFirstRow();
			while(pRow) {

				long nMessageID = VarLong(pRow->GetValue(elcMessageID));
				arySelectedIDs.Add(nMessageID);

				if(!strSelectedIDs.IsEmpty()) {
					strSelectedIDs += ",";
				}
				strSelectedIDs += AsString(nMessageID);

				pRow = pRow->GetNextRow();
			}
		}

		//now clear both lists
		m_ExportUnselectedList->Clear();
		m_ExportSelectedList->Clear();

		//if we have no current group selection, ensure requerying has finished, and select the first row
		long nHL7GroupID = -1;
		IRowSettingsPtr pRow = m_ExportGroupFilterCombo->GetCurSel();
		if(pRow == NULL) {
			//get the first row
			pRow = m_ExportGroupFilterCombo->GetFirstRow();
			if(pRow == NULL) {
				//no row available, so ensure we have all groups loaded
				//(there will never be more than just a couple, and it is unlikely for the first row to not exist yet)
				m_ExportGroupFilterCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
				pRow = m_ExportGroupFilterCombo->GetFirstRow();
			}
			m_ExportGroupFilterCombo->PutCurSel(pRow);
		}
		if(pRow != NULL) {
			nHL7GroupID = VarLong(pRow->GetValue(egfcID),-1);	
		}

		if(nHL7GroupID == -1) {
			//no group selected, therefore we cannot display anything, so leave now
			return;
		}

		// (z.manning 2008-07-16 17:51) - PLID 30490 - Changed this query to also load schedule messages.
		// (a.vengrofski 2010-07-28 10:50) - PLID <38919> - Removed the ability for Labs to show up in the export lists.
		// (c.haag 2010-08-11 09:31) - PLID 39799 - We now include records that do have sent dates but no ACK dates
		// for groups that are expecting ACK's for a certain amount of time after being dispatched. The ACK usually comes
		// almost immediately.
		//TES 6/22/2011 - PLID 44261 - The ExportType and ExpectACK settings are stored differently now.  Let's pull all the groups that
		// have both settings, and use that array in the query
		CArray<long,long> arHL7GroupsExportTcp;
		GetHL7SettingsGroupsBySetting("ExportType", (long)1, arHL7GroupsExportTcp);
		CArray<long,long> arHL7GroupsExpectACK;
		GetHL7SettingsGroupsBySetting("ExpectACK", TRUE, arHL7GroupsExpectACK);
		CArray<long,long> arHL7GroupsRequireACK;
		for(int i = 0; i < arHL7GroupsExportTcp.GetSize(); i++) {
			bool bMatched = false;
			for(int j = 0; j < arHL7GroupsExpectACK.GetSize() && !bMatched; j++) {
				if(arHL7GroupsExpectACK[j] == arHL7GroupsExportTcp[i]) {
					arHL7GroupsRequireACK.Add(arHL7GroupsExportTcp[i]);
					bMatched = true;
				}
			}
		}	

		// (d.singleton 2012-12-19 14:19) - PLID 53041 added exporting lab results and locked emns
		// (a.wilson 2013-02-26 13:25) - PLID 51811 - added patient name to description of emn bills.
		long nFailedACKCheckDelay = GetRemotePropertyInt("HL7Import_FailedACKCheckDelaySeconds", 300);
		// (r.gonet 02/26/2013) - PLID 47534 - Filtered out dismissed messages
		_RecordsetPtr rs = CreateParamRecordset("SELECT MessageID, GroupID, CreateDate, "
				"CASE WHEN PracticeRecordType = {INT} THEN PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle "
				"	  WHEN PracticeRecordType = {INT} THEN (EMNBillPerson.Last + ', ' +	EMNBillPerson.First + ' ' + EMNBillPerson.Middle + ' - Bill from ' + EMNBillMaster.Description + ' on ' + CONVERT(nvarchar(50), EMNBillMaster.Date, 101)) "
				"	  WHEN PracticeRecordType = {INT} THEN COALESCE(ApptPerson.Last, '<No Patient>') + ', ' + COALESCE(ApptPerson.First, '') + ' - ' + CAST(StartTime AS nvarchar(50)) "
				"	  WHEN PracticeRecordType = {INT} THEN LabPerson.Last + ', ' + LabPerson.First + ' ' + LabPerson.Middle + ' - ' + LabsT.FormNumberTextID "
				"	  WHEN PracticeRecordType = {INT} THEN EMRPerson.Last + ', ' + EMRPerson.First + ' ' + EMRPerson.Middle + ' - ' + EMRMasterT.Description "
				"	  WHEN PracticeRecordType = {INT} THEN RefPhysPerson.Last + ', ' + RefPhysPerson.First + ' ' + RefPhysPerson.Middle " // (v.maida 2014-12-23 12:19) - PLID 64472 - Added referring physician description.
				"	  ELSE '' END AS RecordDescription, "
				"MessageType, PracticeRecordType, PracticeRecordID, "
				"CONVERT(BIT, CASE WHEN (SentDate IS NOT NULL AND DATEDIFF(second, SentDate, GetDate()) > {INT} AND AcknowledgeDate IS NULL AND GroupID IN ({INTARRAY})) THEN 1 ELSE 0 END) AS AckMissing "
				"FROM HL7MessageLogT "
				"LEFT JOIN PersonT ON HL7MessageLogT.PracticeRecordID = PersonT.ID AND PracticeRecordType = {INT} "
				"LEFT JOIN (EMRMasterT EMNBillMaster LEFT JOIN PersonT EMNBillPerson ON EMNBillMaster.PatientID = EMNBillPerson.ID) ON HL7MessageLogT.PracticeRecordID = EMNBillMaster.ID AND PracticeRecordType = {INT} "
				"LEFT JOIN (AppointmentsT LEFT JOIN PersonT ApptPerson ON AppointmentsT.PatientID = ApptPerson.ID) ON HL7MessageLogT.PracticeRecordID = AppointmentsT.ID AND PracticeRecordType = {INT} "
				"LEFT JOIN (LabsT LEFT JOIN PersonT LabPerson ON LabsT.PatientID = LabPerson.ID) ON HL7MessageLogT.PracticeRecordID = LabsT.ID AND PracticeRecordType = {INT} "
				"LEFT JOIN (EMRMasterT LEFT JOIN PersonT EMRPerson ON EMRMasterT.PatientID = EMRPerson.ID) ON HL7MessageLogT.PracticeRecordID = EMRMasterT.ID AND PracticeRecordType = {INT} "
				"LEFT JOIN (ReferringPhysT LEFT JOIN PersonT RefPhysPerson ON ReferringPhysT.PersonID = RefPhysPerson.ID) ON HL7MessageLogT.PracticeRecordID = ReferringPhysT.PersonID AND PracticeRecordType = {INT} " // (v.maida 2014-12-23 12:19) - PLID 64472 - Added referring physician description.
				"WHERE (SentDate Is Null OR (SentDate IS NOT NULL AND DATEDIFF(second, SentDate, GetDate()) > {INT} AND AcknowledgeDate IS NULL AND GroupID IN ({INTARRAY}))) "
				"AND Dismissed = 0 AND GroupID = {INT} AND MessageType <> {INT}", hprtPatient, hprtEmnBill, hprtAppt, hprtLabResult, hprtLockedEmn, hprtReferringPhysician, nFailedACKCheckDelay, arHL7GroupsRequireACK,
				hprtPatient, hprtEmnBill, hprtAppt, hprtLabResult, hprtLockedEmn, hprtReferringPhysician, nFailedACKCheckDelay, arHL7GroupsRequireACK, nHL7GroupID, hemtNewLab);

		while(!rs->eof) {
			
			long nID = AdoFldLong(rs, "MessageID", -1);
			long nGroupID = AdoFldLong(rs, "GroupID", -1);
			COleDateTime dtCreateDate = AdoFldDateTime(rs, "CreateDate", COleDateTime::GetCurrentTime());
			HL7PracticeRecordType hl7RecordType = (HL7PracticeRecordType)AdoFldLong(rs, "PracticeRecordType", (long)hprtPatient);
			long nRecordID = AdoFldLong(rs, "PracticeRecordID", -1);
			CString strDescription = AdoFldString(rs, "RecordDescription", "");
			BOOL bAckMissing = AdoFldBool(rs, "AckMissing");

			HL7ExportMessageType hemtMessageType = (HL7ExportMessageType)AdoFldLong(rs, "MessageType");
			CString strMessageType = "";
			switch(hemtMessageType)
			{
				case hemtAddNewPatient:
					strMessageType = "Add New Patient";
					break;

				case hemtUpdatePatient:
					strMessageType = "Update Existing Patient";
					break;

				case hemtNewAppt:
					strMessageType = "Add New Appointment";
					break;

				case hemtUpdateAppt:
					strMessageType = "Update Existing Appointment";
					break;

				case hemtCancelAppt:
					strMessageType = "Cancel Appointment";
					break;

				//TES 7/10/2009 - PLID 25154 - Added Bills (from EMR)
				case hemtNewEmnBill:
					strMessageType = "Add New Bill";
					break;

				//TES 5/26/2010 - PLID 38541 - Added Lab Orders
				case hemtNewLab:
					strMessageType = "New Lab Order";
					break;

				// (d.singleton 2012-12-14 13:50) - PLID 53041 added exporting locked emns
				case hemtLockedEMN:
					strMessageType = "Locked EMN";
					break;

				// (d.singleton 2012-12-14 13:51) - PLID 53041 added exporting lab results
				case hemtNewLabResult:
					strMessageType = "Lab Result";
					break;

				// (v.maida 2014-12-31 09:04) - PLID 64472 - added exporting referring physicians
				case hemtReferringPhysician:
					strMessageType = "Add/Update Referring Physician";
					break;
				// (s.tullis 2015-09-29 16:04) - PLID 66197 - Added Patient Reminder
				case hemtPatientReminder:
					strMessageType = "Patient Reminder";
					break;
				default:
				//	ASSERT(FALSE);
					break;
			}

			//add to the datalist - but which one?

			//was this previously in the selected list?
			BOOL bFound = FALSE;
			for(int i=arySelectedIDs.GetSize()-1;i>=0 && !bFound;i--) {
				if(arySelectedIDs.GetAt(i) == nID) {

					bFound = TRUE;

					//it needs to go back into the selected list
					IRowSettingsPtr pRow = m_ExportSelectedList->GetNewRow();
					pRow->PutValue(elcMessageID, (long)nID);
					pRow->PutValue(elcGroupID, (long)nGroupID);
					pRow->PutValue(elcPracticeRecordType, (long)hl7RecordType);
					pRow->PutValue(elcPracticeRecordID, (long)nRecordID);
					pRow->PutValue(elcMessageType, _bstr_t(strMessageType));
					// (r.gonet 12/03/2012) - PLID 54114 - Add in the HL7ExportMessageType id
					pRow->PutValue(elcMessageTypeID, (long)hemtMessageType);
					pRow->PutValue(elcDescription, _bstr_t(strDescription));
					pRow->PutValue(elcCreateDate, _variant_t(dtCreateDate, VT_DATE));
					// (c.haag 2010-08-11 09:46) - PLID 39799 - If this record was sent
					// but we never got an ACK back that we expected to get back, then
					// assign a warning icon to the record.
					if (bAckMissing) {
						pRow->PutValue(elcFlag,_variant_t((long)m_hIconRedX));
					}
					m_ExportSelectedList->AddRowSorted(pRow, NULL);

					//remove this entry to make the search faster next time
					arySelectedIDs.RemoveAt(i);
				}
			}

			//if it wasn't in the selected list, add to the unselected list
			if(!bFound) {
				IRowSettingsPtr pRow = m_ExportUnselectedList->GetNewRow();
				pRow->PutValue(elcMessageID, (long)nID);
				pRow->PutValue(elcGroupID, (long)nGroupID);
				pRow->PutValue(elcPracticeRecordType, (long)hl7RecordType);
				pRow->PutValue(elcPracticeRecordID, (long)nRecordID);			
				pRow->PutValue(elcMessageType, _bstr_t(strMessageType));
				// (r.gonet 12/03/2012) - PLID 54114 - Add in the message type id
				pRow->PutValue(elcMessageTypeID, (long)hemtMessageType);
				pRow->PutValue(elcDescription, _bstr_t(strDescription));
				pRow->PutValue(elcCreateDate, _variant_t(dtCreateDate, VT_DATE));
				// (c.haag 2010-08-11 09:46) - PLID 39799 - If this record was sent
				// but we never got an ACK back that we expected to get back, then
				// assign a warning icon to the record.
				if (bAckMissing) {
					pRow->PutValue(elcFlag,_variant_t((long)m_hIconRedX));
				}
				m_ExportUnselectedList->AddRowSorted(pRow, NULL);
			}

			rs->MoveNext();
		}
		rs->Close();

		//clear the array of selected IDs - if this array is non-empty, it
		//means that an item was previously in the selected list but is no
		//no longer available, it may have been exported by another user
		arySelectedIDs.RemoveAll();

	}NxCatchAll("Error in CHL7BatchDlg::RequeryExportLists");
}

void CHL7BatchDlg::OnDblClickCellHl7ImportUnselectedList(LPDISPATCH lpRow, short nColIndex) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		m_ImportUnselectedList->PutCurSel(pRow);
		
		if(pRow) {
			OnSelectOneHl7Import();
		}

	}NxCatchAll("Error in CHL7BatchDlg::OnDblClickCellHl7ImportUnselectedList");
}

void CHL7BatchDlg::OnDblClickCellHl7ImportSelectedList(LPDISPATCH lpRow, short nColIndex) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		m_ImportSelectedList->PutCurSel(pRow);
		
		if(pRow) {
			OnUnselectOneHl7Import();
		}

	}NxCatchAll("Error in CHL7BatchDlg::OnDblClickCellHl7ImportSelectedList");
}

void CHL7BatchDlg::OnDblClickCellHl7ExportUnselectedList(LPDISPATCH lpRow, short nColIndex) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		m_ExportUnselectedList->PutCurSel(pRow);
		
		if(pRow) {
			OnSelectOneHl7Export();
		}

	}NxCatchAll("Error in CHL7BatchDlg::OnDblClickCellHl7ExportUnselectedList");
}

void CHL7BatchDlg::OnDblClickCellHl7ExportSelectedList(LPDISPATCH lpRow, short nColIndex) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		m_ExportSelectedList->PutCurSel(pRow);
		
		if(pRow) {
			OnUnselectOneHl7Export();
		}

	}NxCatchAll("Error in CHL7BatchDlg::OnDblClickCellHl7ExportSelectedList");
}

// (j.jones 2008-04-11 14:53) - PLID 29596 - when clicked, check for new HL7 import files
void CHL7BatchDlg::OnBtnCheckForMessages() 
{
	try {

		//process any pending HL7 files, with bSilent turned off
		ProcessHL7Files(FALSE);

		//ProcessHL7Files will requery the import filter and lists upon completion

	}NxCatchAll("Error in CHL7BatchDlg::OnBtnCheckForMessages");
}

void CHL7BatchDlg::OnSelChosenImportTypeFilter(LPDISPATCH lpRow) 
{
	try {
		
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			pRow = m_ImportTypeFilterCombo->GetFirstRow();
			if(pRow == NULL) {
				return;
			}
			m_ImportTypeFilterCombo->PutCurSel(pRow);
		}

		//this will requery the unselected import list, and leave the selected list alone
		RequeryImportLists();

	}NxCatchAll("Error in CHL7BatchDlg::OnSelChosenImportTypeFilter");
}

// (j.jones 2008-04-11 14:55) - PLID 29596 - will process all files for all HL7 groups
// bSilent will suppress all messages if TRUE
// (z.manning 2010-05-27 09:16) - PLID 36976 - There is also an equivilent version of this
// function in NxServer (HL7Support::ProcessHL7Files). Please keep them in sync as necessary.
void CHL7BatchDlg::ProcessHL7Files(BOOL bSilent)
{
	try {

		//this code was lifted and modified from the old CHL7ImportDlg::OnImportFromFile() function

		BOOL bOneImported = FALSE;
		BOOL bOneFailed = FALSE;
		//TES 6/25/2009 - PLID 34355 - Track if we failed to delete any files.
		BOOL bOneDeleteFailed = FALSE;
		// (a.wilson 2013-05-16 11:27) - PLID 41682 - user for making an error message more descriptive.
		bool bOneUnsupportedMessage = false;
		//TES 3/16/2011 - PLID 41912 - Used for making a more helpful error message
		BOOL bPossibleFacilityIDMismatch = FALSE;

		//TES 4/7/2008 - PLID 27364 - Track if there any files we can't access.
		int nLockedFiles = 0;

		//set m_bAutoProcessHL7Files to TRUE, such that we assume it will work,
		//and if anything says otherwise during the process, then it will be
		//set to FALSE at that time
		m_bAutoProcessHL7Files = TRUE;

		//get the information for each group
		// (j.jones 2008-05-05 10:08) - PLID 29600 - pull the BatchImports setting
		//TES 3/16/2011 - PLID 41912 - Pull the ForceFacilityIDMatch and LabFacilityID settings
		//TES 6/22/2011 - PLID 44261 - Get HL7 Settings from the cache, not the database.
		CArray<long,long> arHL7GroupIDs;
		GetAllHL7SettingsGroups(arHL7GroupIDs);

		//TES 10/31/2013 - PLID 59251 - Track if any new interventions are triggered
		CDWordArray arNewCDSInterventions;
		for(int i = 0; i < arHL7GroupIDs.GetSize(); i++) {
			long nGroupID = arHL7GroupIDs[i];
			CString strName = GetHL7GroupName(nGroupID);
			long nType = GetHL7SettingInt(nGroupID, "ImportType");
			BOOL bBatchImports = GetHL7SettingBit(nGroupID, "BatchImports");
			//only process groups with an import type of 'file'
			if (nType == 0) {		
				CString strFolder = GetHL7SettingText(nGroupID, "ImportFile");
				if ((! FileUtils::DoesFileOrDirExist(strFolder)) || (strFolder.IsEmpty())) {
					CString strMsg;
					strMsg.Format("The folder you have specified for HL7 Imports for group '%s' is not valid.  Please edit your HL7 Settings for this group, and enter a valid path in the Filename field in the Import section.", strName);
					if(!bSilent) {
						AfxMessageBox(strMsg);
					}
					//log that this failed
					Log("CHL7BatchDlg::ProcessHL7Files message 1: " + strMsg);

					//since this failed, set m_bAutoProcessHL7Files to false so we don't auto-attempt this again upon refresh
					m_bAutoProcessHL7Files = FALSE;
					continue;
				}
				CString strExtension = GetHL7SettingText(nGroupID, "ImportExtension");

				//loop through the files in the folder and import each one
				CFileFind finder;
				CString strFile;
				int state = 2;

				if (finder.FindFile(strFolder ^ "*."+strExtension))
				{
					while (state)
					{	if (!finder.FindNextFile())
							state = 0;

						if(!finder.IsDots() && !finder.IsDirectory()) {
							strFile = finder.GetFileName();
							if(!strExtension.IsEmpty() || strFile.Find(".") == -1) {
								CString strMessage;
								try
								{
									
									//we found one so import it
									CFile InFile;
									if(!InFile.Open(strFolder ^ strFile,  CFile::modeRead | CFile::shareCompat)) {
										//TES 4/7/2008 - PLID 27364 - We can't access it.  Most likely it's just still being
										// written to by the third-party app.  Just skip this file.
										nLockedFiles++;
									}
									else {
										const int LEN_16_KB = 16384;
										CString strIn;	//input string
										long iFileSize = InFile.Read(strIn.GetBuffer(LEN_16_KB), LEN_16_KB);
										strIn.ReleaseBuffer(iFileSize);

										while (iFileSize == LEN_16_KB) {

											strMessage += strIn;
											iFileSize = InFile.Read(strIn.GetBuffer(LEN_16_KB), LEN_16_KB);
											//TES 9/11/2008 - PLID 31415 - Need to pass in the file size!
											strIn.ReleaseBuffer(iFileSize);
										}

										strMessage += strIn;

										//TES 6/24/2009 - PLID 34355 - Now, before going any farther, attempt to delete
										// the file.  If we can't delete it, we can't import it (because otherwise we'll
										// just keep re-importing it).
										InFile.Close();

										// (r.gonet 05/01/2014) - PLID 61843 - We're starting a new HL7 message transaction
										BeginNewHL7Transaction(GetRemoteData(), GetHL7SettingInt(nGroupID, "CurrentLoggingLevel"));

										//TES 4/21/2008 - PLID 29721 - Added strHL7GroupName parameter for auditing.
										// (j.jones 2008-05-05 10:09) - PLID 29600 - added bBatchImports parameter
										// (j.jones 2008-05-19 16:11) - PLID 30110 - don't send a tablechecker, we will do it ourselves
										//TES 6/25/2009 - PLID 34355 - Put this in its own try...catch, so that if it
										// errors out we will still attempt to re-create the original message.
										// (z.manning 2010-06-28 16:19) - PLID 38896 - I reorganized this code so that we add
										// the message first, delete the file and if the delete fails we then delete the message.
										// I also put this code in an ADO transaction for added safety in case something happens to
										// the SQL connection between adding the message to data and attempting to delete it.
										HL7Message message;
										BOOL bSuccess = FALSE;
										{
											CSqlTransaction sqlTran("EnqueueHL7Message");
											sqlTran.Begin();
											// (a.wilson 2013-05-08 17:43) - PLID 41682 - support result enum.
											HL7AddMessageToQueueResult amtqResult = AddMessageToQueue(strMessage, nGroupID, strName, FALSE, message);
											if (amtqResult.bSuccess == true) {
												if(DeleteFile(strFolder ^ strFile)) {
													bOneImported = TRUE;
													bSuccess = TRUE;
												}
												else {
													Log("Failed to delete file '%s' and will now attempt to delete newly created HL7MessageQueueT record (ID = %li)", strFolder ^ strFile, message.nMessageID);
													if(message.nMessageID != -1) {
														// (r.gonet 05/01/2014) - PLID 61843 - Ensure that any HL7 transaction that was for this message no longer references the message.
														// Don't delete the HL7Transaction though because we want to see the deletion failure in the logs.
														// (r.gonet 2016-05-24 9:11) - PLID-66576 - Removed a stray comma between the statements.
														ExecuteParamSql(
															"UPDATE HL7TransactionT SET ImportMessageID = NULL WHERE ImportMessageID = {INT}; "
															"DELETE FROM HL7MessageQueueT WHERE ID = {INT}"
															, message.nMessageID, message.nMessageID);
													}
													bOneDeleteFailed = TRUE;
													bOneFailed = TRUE;
												}
											}
											else {
												// (a.wilson 2013-05-08 17:44) - PLID 41682 - if the event type was not supported
												//	and everyting else worked fine then we need to move the message into a folder
												//	named "Unsupported".
												if (amtqResult.eccErrorCode == eccEventNotSupported) {
													//attempt to move the file to a seperate folder in the same path. don't worry if it fails.
													if (!MoveUnsupportedHL7Message(strFolder, strFile)) {
														Log("Failed to move file %s to the unsupported HL7 message folder.", strFolder ^ strFile);
													}
													bOneUnsupportedMessage = true;
												}
												//TES 3/16/2011 - PLID 41912 - If they're configured to force matching Facility IDs, then there's a high
												// probability that that's why this failed.
												else if(amtqResult.eccErrorCode == eccUnknownKeyIdentifier 
													&& GetHL7SettingBit(nGroupID, "ForceFacilityIDMatch") 
													&& !GetHL7SettingText(nGroupID, "LabFacilityID").IsEmpty()) {
													bPossibleFacilityIDMismatch = TRUE;
												}
												bOneFailed = TRUE;
											}

											sqlTran.Commit();
										}

										CString strMessageType;
										GetHL7MessageComponent(message.strMessage, message.nHL7GroupID, "MSH", 1, 9, 1, 1, strMessageType);

										// (j.jones 2008-05-05 10:00) - PLID 29600 - only auto-commit if the group setting says to do so
										// (z.manning 2010-06-29 09:54) - PLID 38896 - I moved this code out of AddMessageToQueue
										// (r.farnworth 2014-12-09 15:42) - PLID 64325 - Currently, if your HL7 link setting is not set to "batch import messages," 
										// the billing dialog opops up with whatever message it brought in. DFT import messages should always batch and never automatically attempt to be processed.
										if (strMessageType != "DFT") {
											if (bSuccess && !bBatchImports && message.nMessageID != -1) {
												//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
												CommitHL7Event(message, this, FALSE, arNewCDSInterventions);
											}
										}

										// (z.manning 2010-06-28 16:25) - PLID 38896 - The rearrangment of the above code makes this
										// all unnecessary.
										/*else {
											//TES 6/25/2009 - PLID 34355 - It failed, re-create the original file.
											CFile OutFile;
											bool bRecreated = false;
											if(!DoesExist(strFolder ^ strFile)) {
												if(OutFile.Open(strFolder ^ strFile, CFile::modeWrite|CFile::modeCreate | CFile::shareCompat)) {
													OutFile.Write(strMessage.GetBuffer(strMessage.GetLength()), strMessage.GetLength());
													strMessage.ReleaseBuffer();
													OutFile.Close();
													bRecreated = true;
												}
											}
											if(!bRecreated) {
												//TES 6/25/2009 - PLID 34355 - Uh-oh!  We failed to re-create it, but
												// we haven't saved it to data.  We need to make sure it's not lost,
												// so let's create it as a temporary file and have them save it somewhere.
												CString strTmpFileName;
												HANDLE hFile = CreateNxTempFile("RecoveredHL7Message", strExtension, &strTmpFileName);
												if(hFile == INVALID_HANDLE_VALUE) {
													//TES 6/25/2009 - PLID 34355 - We can't even create a temp file!
													// We're just going to have to put it on screen.
													CMsgBox dlg;
													dlg.msg = "An HL7 message failed to import, and then could not be restored to its original location.  "
														"Please copy the text below, paste it into a text editor (such as Notepad or Microsoft Word), and save it.  "
														"Additionally, please contact NexTech Technical Support to notify them of this problem.\r\n"
														"\r\n"
														"WARNING! If you click OK on this message before copying the text to another location, the message will be PERMANENTLY LOST!\r\n"
														"\r\n" + strMessage;
													dlg.DoModal();
												}
												else {
													MsgBox("An HL7 Message could not be imported, and additionally could not be restored to its original location.  "
														"It will now be opened on screen in Notepad.  Please save the file to a location where you will be able to "
														"find it again, and call NexTech Technical Support.");
													// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
													ShellExecute((HWND)this, NULL, "notepad.exe", strTmpFileName, NULL, SW_SHOW);
												}
											}
										}*/
									}
								
								} NxCatchAll("Error in CHL7BatchDlg::ProcessHL7Files : ProcessFile");
							}
						}
					}
				}
			}
		}
		
		// (j.jones 2008-05-19 16:11) - PLID 30110 - send a tablechecker ourselves
		//refresh the table
		if(bOneImported) {
			m_HL7MessageQueueTChecker.Refresh();
		}

		//now requery our lists to re-display results
		RequeryImportFilterTypes();
		RequeryImportLists();
		
		if(bOneFailed) {
			CString strMsg;
			//TES 6/25/2009 - PLID 34355 - Give a different message if the failure was due to lack of delete permissions.
			if(bOneDeleteFailed) {
				strMsg = "Some messages were not imported because they could not be deleted out of the import path.  Please ensure that you have permission to delete files in your HL7 Import folder.";
			}
			else {
				// (a.wilson 2013-05-16 11:34) - PLID 41682 - add error message to explain unsupported events.
				if (bPossibleFacilityIDMismatch || bOneUnsupportedMessage) {
					//TES 3/16/2011 - PLID 41912 - If we think the problem might have been the facility ID, then say as much.
					strMsg.Format("Some messages could not be successfully imported for the following reasons:\r\n%s%s",
						(bOneUnsupportedMessage ? "\r\n- One or more messages were unsupported HL7 events." : ""), 
						(bPossibleFacilityIDMismatch ? "\r\n- The messages may have been incorrectly formatted, "
						"or their Facility ID may not match the Facility ID specified in your HL7 Settings." : ""));
				}
				else {
					strMsg = "Some messages could not be successfully imported.";
				}
			}
			if(!bSilent) {					
				AfxMessageBox(strMsg);
			}
			//log that this failed
			Log("CHL7BatchDlg::ProcessHL7Files message 2: " + strMsg);
		}
		else {
			if(bOneImported) {
				if(!bSilent) {
					AfxMessageBox("All messages imported successfully!");
				}
			}
			else {
				if(!bSilent) {
					AfxMessageBox("No messages were found in your Import path(s).");
				}
			}
		}

		if(nLockedFiles > 10) {
			//TES 4/7/2008 - PLID 27364 - Hmm, there may be something else going on here.  Let's give the user a heads up.
			CString strMsg;
			strMsg.Format("The HL7 Link has detected an unusually high number of inaccessible files (%i) in your HL7 Import folder(s).  "
				"These files may be in use by another program, or you may not have sufficient permissions to access them.\r\n\r\n"
				"There may not be anything wrong with your system; however, if you continue to get this message, please contact NexTech Technical Support to investigate this issue.",
				nLockedFiles);
			if(!bSilent) {
				MsgBox(strMsg);
			}
			//log that this failed
			Log("CHL7BatchDlg::ProcessHL7Files message 3: " + strMsg);
		}

		//TES 10/31/2013 - PLID 59251 - If any new interventions were triggered, notify the user, even if bSilent is true (that's only to avoid matching prompts
		// and the like)
		//if(!bSilent) {
			GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);
		//}

	}NxCatchAll("Error in CHL7BatchDlg::ProcessHL7Files");
}

void CHL7BatchDlg::OnRButtonDownHl7ImportUnselectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		m_ImportUnselectedList->PutCurSel(pRow);
		if(pRow == NULL) {
			return;
		}

		//indicate that we clicked the unselected list
		SetLastRightClickList(m_ImportUnselectedList, TRUE);

		//build + popup the menu
		CMenu mnu;
		mnu.LoadMenu(IDR_HL7_MENU);
		// (r.gonet 02/26/2013) - PLID 47534 - We have two dismiss menu items, one for import and one for export. Remove the export one.
		mnu.RemoveMenu(ID_EXPORT_DISMISS, MF_BYCOMMAND);
		CMenu *pmnuSub = mnu.GetSubMenu(0);		//we just want the first submenu
		if(!pmnuSub)
			return;

		//set 'commit' as default
		pmnuSub->SetDefaultItem(ID_IMPORT_COMMIT);

		//TES 5/18/2009 - PLID 34282 - Check whether they have permission to import this type of record, and disable
		// the menu item if not.
		bool bCanCommit = true;
		CString strMessageType = VarString(pRow->GetValue(ilcMessageType),"");
		CString strEventType = VarString(pRow->GetValue(ilcEventType),"");
		HL7ImportRecordType hirt = GetRecordType(strMessageType, strEventType);
		if(hirt == hirtPatient) {
			if(!(GetCurrentUserPermissions(bioHL7Patients) & SPT_______1____ANDPASS)) {
				bCanCommit = false;
			}
		}
		// (z.manning 2010-06-29 15:51) - PLID 34572
		if(hirt == hirtAppointment) {
			if(!(GetCurrentUserPermissions(bioHL7Appointments) & SPT_______1____ANDPASS)) {
				bCanCommit = false;
			}
		}
		else if(hirt == hirtBill) {
			if(!(GetCurrentUserPermissions(bioHL7Bills) & SPT______0_____ANDPASS)) {
				bCanCommit = false;
			}
		}
		else if(hirt == hirtLab) {
			if(!(GetCurrentUserPermissions(bioHL7Labs) & SPT______0_____ANDPASS)) {
				bCanCommit = false;
			}
		}
		if(!bCanCommit) {
			pmnuSub->EnableMenuItem(ID_IMPORT_COMMIT, MF_DISABLED|MF_GRAYED);
		}

		// (d.singleton 2013-05-08 16:50) - PLID 55241 - need to be able to dismiss new image hl7 messages, so see if this is an image message
		long nGroupID = VarLong(pRow->GetValue(ilcGroupID), -1);
		BOOL bIsImageMessage = GetHL7SettingBit(nGroupID, "UsePatientHistoryImportMSH_3");

		//TES 5/12/2008 - PLID 13339 - They can't dismiss it if it's already been processed.
		//TES 10/14/2008 - PLID 31666 - They also can't dismiss it if it's a lab result.
		if(VarLong(pRow->GetValue(ilcActionID),-1) != -1 ||
			(strMessageType == "ORU" && strEventType == "R01" && !bIsImageMessage) ) {
			pmnuSub->EnableMenuItem(ID_IMPORT_DISMISS, MF_DISABLED|MF_GRAYED);
		}

		//and show the popup
		CPoint pt;
		GetCursorPos(&pt);
		pmnuSub->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);

	}NxCatchAll("Error in CHL7BatchDlg::OnRButtonDownHl7ImportUnselectedList");
}

void CHL7BatchDlg::OnRButtonDownHl7ImportSelectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		m_ImportSelectedList->PutCurSel(pRow);
		if(pRow == NULL) {
			return;
		}

		//indicate that we clicked the selected list
		SetLastRightClickList(m_ImportSelectedList, TRUE);

		//build + popup the menu
		CMenu mnu;
		mnu.LoadMenu(IDR_HL7_MENU);
		// (r.gonet 02/26/2013) - PLID 47534 - We have two commands for dismiss. One for import and one for export. Remove the export one.
		mnu.RemoveMenu(ID_EXPORT_DISMISS, MF_BYCOMMAND);
		CMenu *pmnuSub = mnu.GetSubMenu(0);		//we just want the first submenu
		if(!pmnuSub)
			return;

		//set 'commit' as default
		pmnuSub->SetDefaultItem(ID_IMPORT_COMMIT);

		//TES 5/18/2009 - PLID 34282 - Check whether they have permission to import this type of record, and disable
		// the menu item if not.
		bool bCanCommit = true;
		CString strMessageType = VarString(pRow->GetValue(ilcMessageType),"");
		CString strEventType = VarString(pRow->GetValue(ilcEventType),"");
		HL7ImportRecordType hirt = GetRecordType(strMessageType, strEventType);
		if(hirt == hirtPatient) {
			if(!(GetCurrentUserPermissions(bioHL7Patients) & SPT_______1____ANDPASS)) {
				bCanCommit = false;
			}
		}
		// (z.manning 2010-06-29 15:52) - PLID 34572
		if(hirt == hirtAppointment) {
			if(!(GetCurrentUserPermissions(bioHL7Appointments) & SPT_______1____ANDPASS)) {
				bCanCommit = false;
			}
		}
		else if(hirt == hirtBill) {
			if(!(GetCurrentUserPermissions(bioHL7Bills) & SPT______0_____ANDPASS)) {
				bCanCommit = false;
			}
		}
		else if(hirt == hirtLab) {
			if(!(GetCurrentUserPermissions(bioHL7Labs) & SPT______0_____ANDPASS)) {
				bCanCommit = false;
			}
		}
		if(!bCanCommit) {
			pmnuSub->EnableMenuItem(ID_IMPORT_COMMIT, MF_DISABLED|MF_GRAYED);
		}
		
		// (d.singleton 2013-05-08 16:50) - PLID 55241 - need to be able to dismiss new image hl7 messages, so see if this is an image message
		long nGroupID = VarLong(pRow->GetValue(ilcGroupID), -1);
		BOOL bIsImageMessage = GetHL7SettingBit(nGroupID, "UsePatientHistoryImportMSH_3");

		//TES 5/12/2008 - PLID 13339 - They can't dismiss it if it's already been processed.
		//TES 10/14/2008 - PLID 31666 - They also can't dismiss it if it's a lab result.
		if(VarLong(pRow->GetValue(ilcActionID),-1) != -1 ||
			(strMessageType == "ORU" && strEventType == "R01" && !bIsImageMessage) ) {
			pmnuSub->EnableMenuItem(ID_IMPORT_DISMISS, MF_DISABLED|MF_GRAYED);
		}

		//and show the popup
		CPoint pt;
		GetCursorPos(&pt);
		pmnuSub->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);

	}NxCatchAll("Error in CHL7BatchDlg::OnRButtonDownHl7ImportSelectedList");
}

void CHL7BatchDlg::OnImportCommit() 
{
	try {

		if(m_pdlLastRightClickedHL7List == NULL || !m_bLastRightClickListWasImport) {
			return;
		}

		IRowSettingsPtr pRow = NULL;
		
		//detect which list we right-clicked
		pRow = m_pdlLastRightClickedHL7List->GetCurSel();

		if(pRow == NULL) {
			return;
		}

		//TES 5/18/2009 - PLID 34282 - Check their permissions.
		CString strMessageType = VarString(pRow->GetValue(ilcMessageType),"");
		CString strEventType = VarString(pRow->GetValue(ilcEventType),"");
		HL7ImportRecordType hirt = GetRecordType(strMessageType, strEventType);
		if(hirt == hirtPatient) {
			if(!CheckCurrentUserPermissions(bioHL7Patients, sptDynamic1)) {
				return;
			}
		}
		// (z.manning 2010-06-29 15:52) - PLID 34572
		if(hirt == hirtAppointment) {
			if(!CheckCurrentUserPermissions(bioHL7Appointments, sptDynamic1)) {
				return;
			}
		}
		else if(hirt == hirtBill) {
			if(!CheckCurrentUserPermissions(bioHL7Bills, sptDynamic0)) {
				return;
			}
		}
		else if(hirt == hirtLab) {
			if(!CheckCurrentUserPermissions(bioHL7Labs, sptDynamic0)) {
				return;
			}
		}

		//TES 5/12/2008 - PLID 13339 - Need an extra warning if this message has previously been committed.
		long nActionID = VarLong(pRow->GetValue(ilcActionID),-1);
		if(nActionID == mqaCommit) {
			if(IDYES != MsgBox(MB_YESNO, "This message has previously been committed.  Re-committing it may lead to duplicated records, "
				"or may overwrite changes to this record.  Are you SURE you wish to do this?")) {				
				return;
			}
		}

		const long nMessageID = VarLong(pRow->GetValue(ilcID), -1);
		// (z.manning 2011-06-15 16:16) - PLID 40903 - The message is no longer in the datalist
		CString strMessage = VarString(GetTableField("HL7MessageQueueT", "Message", "ID", nMessageID), "");

		//TES 5/8/2008 - PLID 29685 - Use the new HL7Message structure.
		HL7Message Message;
		Message.nMessageID = nMessageID;
		Message.strMessage = strMessage;
		Message.nHL7GroupID = VarLong(pRow->GetValue(ilcGroupID), -1);
		Message.strHL7GroupName  = VarString(pRow->GetValue(ilcGroupName), "");
		Message.strPatientName = VarString(pRow->GetValue(ilcPatientName), "");
		//TES 10/16/2008 - PLID 31712 - Added a dtInputDate member
		Message.dtInputDate = VarDateTime(pRow->GetValue(ilcInputDate));

		// (r.gonet 05/01/2014) - PLID 61843 - We're beginning a new HL7 message transaction
		BeginNewHL7Transaction(GetRemoteData(), GetHL7SettingInt(Message.nHL7GroupID, "CurrentLoggingLevel"));

		//TES 10/31/2013 - PLID 59251 - Track if any new interventions get triggered
		CDWordArray arNewCDSInterventions;
		//TES 4/21/2008 - PLID 29721 - Added parameters for auditing.
		// (j.jones 2008-05-19 16:11) - PLID 30110 - don't auto-send an HL7 tablechecker, we will do it ourselves
		if(CommitHL7Event(Message, this, FALSE, arNewCDSInterventions)) {
			//it succeeded!  Now we need to update the data to have the correct action
			//TES 4/18/2008 - PLID 29657 - CommitHL7Event updates HL7MessageQueueT now.
			//ExecuteParamSql("UPDATE HL7MessageQueueT SET ActionID = {INT} WHERE ID = {INT}", mqaCommit, nID);

			// (j.jones 2008-05-19 16:11) - PLID 30110 - send a tablechecker ourselves
			//refresh the tablechecker
			m_HL7MessageQueueTChecker.Refresh(Message.nMessageID);

			//now remove the row
			//TES 5/12/2008 - PLID 13339 - Don't remove it if we're showing previously processed messages.
			if(IsDlgButtonChecked(IDC_SHOW_PROCESSED)) {
				pRow->PutValue(ilcActionID, (long)mqaCommit);
				pRow->PutValue(ilcActionTaken, _bstr_t("Committed"));
			}
			else {
				m_pdlLastRightClickedHL7List->RemoveRow(pRow);
			}
		}
		else {
			AfxMessageBox("Practice failed to commit the message.");
		}

		//TES 10/31/2013 - PLID 59251 - If any interventions were triggered, notify the user
		GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);

	} NxCatchAll("Error in CHL7BatchDlg::OnImportCommit");
}

void CHL7BatchDlg::OnImportDismiss() 
{
	try {

		if(m_pdlLastRightClickedHL7List == NULL || !m_bLastRightClickListWasImport) {
			return;
		}

		IRowSettingsPtr pRow = NULL;
		
		//detect which list we right-clicked
		pRow = m_pdlLastRightClickedHL7List->GetCurSel();

		if(pRow == NULL) {
			return;
		}

		//TES 10/14/2008 - PLID 31666 - If this is a Lab Result message, they're not allowed to dismiss it.
		CString strMessageType = VarString(pRow->GetValue(ilcMessageType),"");
		CString strEventType = VarString(pRow->GetValue(ilcEventType),"");
		// (d.singleton 2013-05-08 17:16) - PLID 55241 - need to be able to dismiss new image hl7 messages
		long nGroupID = VarLong(pRow->GetValue(ilcGroupID), -1);
		BOOL bIsImageMessage = GetHL7SettingBit(nGroupID, "UsePatientHistoryImportMSH_3");

		if(strMessageType == "ORU" && strEventType == "R01" && !bIsImageMessage) {
			//TES 10/14/2008 - PLID 31666 - It shouldn't have even given them the option.
			ASSERT(FALSE);
			return;
		}
					
		long nID = VarLong(pRow->GetValue(ilcID), -1);

		// (r.gonet 02/26/2013) - PLID 48419 - Refactored to use shared function
		if(DismissImportedHL7Message(nID)) {
			//remove the row
			//TES 5/12/2008 - PLID 13339 - Don't remove it if we're showing previously processed messages.
			if(IsDlgButtonChecked(IDC_SHOW_PROCESSED)) {
				pRow->PutValue(ilcActionID, (long)mqaDismiss);
				pRow->PutValue(ilcActionTaken, _bstr_t("Dismissed"));
			}
			else {
				m_pdlLastRightClickedHL7List->RemoveRow(pRow);
			}
		} else {
			// (r.gonet 02/26/2013) - PLID 48419 - Can't see this happening but just in case.
			MessageBox("Practice failed to dismiss the message.", "Dismissal Error", MB_OK|MB_ICONERROR);
		}

	} NxCatchAll("Error in CHL7BatchDlg::OnImportDismiss");
}

// (r.gonet 02/26/2013) - PLID 47534 - Handles the menu item to dismiss an message that is pending export.
void CHL7BatchDlg::OnExportDismiss() 
{
	try {

		// (r.gonet 02/26/2013) - PLID 47534 - Don't proceed if they haven't right clicked or if the right click was for an import list.
		if(m_pdlLastRightClickedHL7List == NULL || m_bLastRightClickListWasImport) {
			return;
		}

		IRowSettingsPtr pRow = NULL;
		pRow = m_pdlLastRightClickedHL7List->GetCurSel();
		if(pRow == NULL) {
			return;
		}
					
		long nID = VarLong(pRow->GetValue(elcMessageID), -1);
		if(DismissExportedHL7Message(nID)) {
			//remove the row
			m_pdlLastRightClickedHL7List->RemoveRow(pRow);
		} else {
			// (r.gonet 02/26/2013) - PLID 47534 - Don't see this happening but this is just in case.
			MessageBox("Practice failed to dismiss the message.", "Dismissal Error", MB_OK|MB_ICONERROR);
		}

	} NxCatchAll("Error in CHL7BatchDlg::OnExportDismiss");
}

void CHL7BatchDlg::OnBtnHl7Settings() 
{
	try {

		//TES 5/27/2009 - PLID 34282 - Check their permission
		if(!CheckCurrentUserPermissions(bioHL7Settings, sptView)) {
			return;
		}
		CHL7SettingsDlg dlg(this);
		dlg.DoModal();

		//refresh the screen to reflect any changes,
		//including attempting to auto-process files
		m_bAutoProcessHL7Files = TRUE;
		UpdateView();		

	} NxCatchAll("Error in CHL7BatchDlg::OnBtnHl7Settings");
}

// (j.jones 2008-04-17 15:41) - PLID 29701 - added OnTableChanged, so we can
// catch any incoming HL7 messages
LRESULT CHL7BatchDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {

		switch(wParam) {
		// (j.jones 2008-04-17 15:59) - PLID 29701 - a HL7MessageQueueT tablechecker
		// means something changed in that table, either an addition or commit,
		// so we need to update accordingly		
		case NetUtils::HL7MessageQueueT:
			{
				try {

					// (j.jones 2008-05-19 16:52) - PLID 30110 - we'll ignore the message
					// if we sent it from this tab
					if(m_HL7MessageQueueTChecker.Changed()) {

						long nID = (long)lParam;
						if(nID == -1) {
							//we were not given a specific ID, so just refresh all the import data
							RequeryImportFilterTypes();
							RequeryImportLists();
						}
						else {

							//if we were given a specific ID, load up its information
							ProcessChangedHL7MessageQueueID(nID);					
						}
					}

				} NxCatchAll("Error in CHL7BatchDlg::OnTableChanged:HL7MessageQueueT");
			}
			break;
		// (j.jones 2008-04-29 14:45) - PLID 29813 - added HL7MessageLogT tablechecker,
		// which also means something changed in the table, not necessarily an addition
		case NetUtils::HL7MessageLogT:
			{
				try {

					// (j.jones 2008-05-19 16:52) - PLID 30110 - we'll ignore the message
					// if we sent it from this tab
					if(m_HL7MessageLogTChecker.Changed()) {

						long nID = (long)lParam;
						if(nID == -1) {
							//we were not given a specific ID, so just refresh all the export data
							RequeryExportLists();
						}
						else {

							//if we were given a specific ID, load up its information
							ProcessChangedHL7MessageLogID(nID);				
						}
					}
					
				} NxCatchAll("Error in CHL7BatchDlg::OnTableChanged:HL7MessageLogT");
			}
			break;
		}

	} NxCatchAll("Error in CHL7BatchDlg::OnTableChanged");

	return 0;
}

// (j.jones 2008-04-17 16:11) - PLID 29701 - this function will update the given imported message ID on the screen
void CHL7BatchDlg::ProcessChangedHL7MessageQueueID(long nID)
{
	try {

		if(nID == -1) {
			//we were not given a specific ID, so do nothing, the caller shouldn't have
			//called this function at all
			ASSERT(FALSE);
			return;
		}

		//if we were given a specific ID, load up its information
		// (a.vengrofski 2010-08-25 17:26) - PLID <38919> - Need to not let the table checker add a lab.
		_RecordsetPtr rs = CreateParamRecordset("SELECT ActionID, GroupID, HL7SettingsT.Name AS GroupName, PatientName, Description, "
			"MessageType, EventType, InputDate, {SQL} AS MessageDateString, \r\n"
			"CASE WHEN HL7MessageQueueT.PurgeDate IS NULL THEN CONVERT(BIT, 0) ELSE CONVERT(BIT, 1) END AS WasPurged \r\n"
			"FROM HL7MessageQueueT "
			"INNER JOIN HL7SettingsT ON HL7MessageQueueT.GroupID = HL7SettingsT.ID "
			"WHERE HL7MessageQueueT.ID = {INT} "
			"AND MessageType <> 'ORU' AND EventType <> 'R01'"
			, GetMessageDateStringSql(), nID);
		if(!rs->eof) {
			
			//start by only retrieving the fields we need
			long nActionID = AdoFldLong(rs, "ActionID", -1);					
			CString strMsgType = AdoFldString(rs, "MessageType", "");
			CString strEvent = AdoFldString(rs, "EventType", "");

			//if the ActionID is not -1, it means this action was either committed or dismissed
			if(nActionID != -1) {
				//if it is in either list, remove the row
				IRowSettingsPtr pRow = m_ImportUnselectedList->FindByColumn(ilcID, (long)nID, m_ImportUnselectedList->GetFirstRow(), FALSE);
				if(pRow) {
					//found it in the unselected list, remove it
					//TES 5/12/2008 - PLID 13339 - Don't remove it if we're showing previously processed messages.
					if(IsDlgButtonChecked(IDC_SHOW_PROCESSED)) {
						pRow->PutValue(ilcActionID, nActionID);
						pRow->PutValue(ilcActionTaken, _bstr_t(GetMessageActionDescription(nActionID)));
					}
					else {
						m_ImportUnselectedList->RemoveRow(pRow);

						//see if we can potentially remove a now-meaningless import filter
						TryRemoveImportFilter(strMsgType, strEvent);
					}
				}
				else {
					//see if it is in the selected list
					pRow = m_ImportSelectedList->FindByColumn(ilcID, (long)nID, m_ImportSelectedList->GetFirstRow(), FALSE);
					if(pRow) {
						//found it in the selected list, remove it
						//TES 5/12/2008 - PLID 13339 - Don't remove it if we're showing previously processed messages.
						if(IsDlgButtonChecked(IDC_SHOW_PROCESSED)) {
							pRow->PutValue(ilcActionID, nActionID);
							pRow->PutValue(ilcActionTaken, _bstr_t(GetMessageActionDescription(nActionID)));
						}
						else {
							m_ImportSelectedList->RemoveRow(pRow);

							//see if we can potentially remove a now-meaningless import filter
							TryRemoveImportFilter(strMsgType, strEvent);
						}
					}
				}

				//we are done, so get out of here
				return;
			}

			//if we get here, the action ID is -1, meaning that this is an uncommitted action and
			//does need to display in our list, so first see if it is not already in either list
			{
				IRowSettingsPtr pRow = m_ImportUnselectedList->FindByColumn(ilcID, (long)nID, m_ImportUnselectedList->GetFirstRow(), FALSE);
				if(pRow) {
					//found it in the unselected list, which means we don't need to update anything, so leave
					return;
				}
				else {
					//see if it is in the selected list
					pRow = m_ImportSelectedList->FindByColumn(ilcID, (long)nID, m_ImportSelectedList->GetFirstRow(), FALSE);
					if(pRow) {
						//found it in the selected list, which means we don't need to update anything, so leave
						return;
					}
				}
			}

			//if we get here, it means we need to add the message, so get the rest of the information we need

			long nGroupID = AdoFldLong(rs, "GroupID", -1);
			CString strGroupName = AdoFldString(rs, "GroupName", "");
			CString strPatientName = AdoFldString(rs, "PatientName", "");
			CString strDescription = AdoFldString(rs, "Description", "");
			//TES 10/16/2008 - PLID 31712 - We'll also need the input date.
			COleDateTime dtInput = AdoFldDateTime(rs, "InputDate");

			CString strAction = GetActionDescriptionForHL7Event(strMsgType, strEvent);

			// (j.jones 2008-04-18 11:45) - PLID 21675 - added message date column
			// (z.manning 2011-06-15 16:33) - PLID 40903 - We no longer need the full message to get the date
			// (r.goldschmidt 2015-11-02 12:01) - PLID 66437 - If the message was purged, set the message date to the input date.
			//								Although this isn't the true message date, it is close enough for the purposes of info/sorting
			COleDateTime dtMessageDate;
			if (!!AdoFldBool(rs, "WasPurged")) {
				dtMessageDate = AdoFldDateTime(rs, "InputDate");
			}
			else {
				CString strMessageDateString = AdoFldString(rs, "MessageDateString", "");
				dtMessageDate = GetHL7DateFromStringField(strMessageDateString, nGroupID);
				if (dtMessageDate.m_status == COleDateTime::invalid) {
					//GetHL7MessageDate should never have sent back an invalid date
					ASSERT(FALSE);
					dtMessageDate = COleDateTime::GetCurrentTime();
				}
			}

			//add to the filter if we need to
			TryAddImportFilter(strMsgType, strEvent);

			//before we try to add the message, is it applicable in our current filter?
			{
				IRowSettingsPtr pRow = m_ImportTypeFilterCombo->GetCurSel();
				if(pRow) {
					CString strFilterMessageType = VarString(pRow->GetValue(itfcMessageType),"");
					CString strFilterEventType = VarString(pRow->GetValue(itfcEventType),"");
					if(!strFilterMessageType.IsEmpty() && !strFilterEventType.IsEmpty()
						&& (strFilterMessageType != strMsgType || strFilterEventType != strEvent)) {

						//it does not match our filter, so we can't add the message
						return;
					}
				}
			}

			//now we can add the message to the unselected list
			{
				IRowSettingsPtr pRow = m_ImportUnselectedList->GetNewRow();
				pRow->PutValue(ilcID, nID);
				pRow->PutValue(ilcGroupID, nGroupID);
				pRow->PutValue(ilcGroupName, _bstr_t(strGroupName));
				pRow->PutValue(ilcPatientName, _bstr_t(strPatientName));
				pRow->PutValue(ilcDescription, _bstr_t(strDescription));
				pRow->PutValue(ilcAction, _bstr_t(strAction));
				// (j.jones 2008-04-18 11:45) - PLID 21675 - added message date column
				pRow->PutValue(ilcMessageDate, _variant_t(dtMessageDate, VT_DATE));
				pRow->PutValue(ilcMessageType, _bstr_t(strMsgType));
				pRow->PutValue(ilcEventType, _bstr_t(strEvent));
				//TES 8/18/2008 - PLID 13339 - Need to fill the ActionID and ActionTaken columns.
				pRow->PutValue(ilcActionID, nActionID);
				pRow->PutValue(ilcActionTaken, _bstr_t(GetMessageActionDescription(nActionID)));
				//TES 10/16/2008 - PLID 31712 - Added a column for HL7MessageQueueT.InputDate
				pRow->PutValue(ilcInputDate, COleVariant(dtInput));
				m_ImportUnselectedList->AddRowSorted(pRow, NULL);
			}
		}
		rs->Close();

	} NxCatchAll("Error in CHL7BatchDlg::ProcessChangedHL7MessageQueueID");
}

// (j.jones 2008-04-17 16:20) - PLID 29701 - this function add a new row to the filter list if the given
// message type and event type are not in the filter already
void CHL7BatchDlg::TryAddImportFilter(CString strMessageType, CString strEventType)
{
	try {

		//see if the passed-in messagetype and eventtype are in our filter, and if not, add that filter

		IRowSettingsPtr pRow = m_ImportTypeFilterCombo->GetFirstRow();
		while(pRow) {

			//if the row matches our message type and event, then we don't need to add a filter

			CString strFilterMessageType = VarString(pRow->GetValue(itfcMessageType),"");
			CString strFilterEventType = VarString(pRow->GetValue(itfcEventType),"");

			if(!strFilterMessageType.IsEmpty() && !strFilterEventType.IsEmpty()
				&& strFilterMessageType == strMessageType && strFilterEventType == strEventType) {

				//there is a filter for this combination, so we don't need to do anything
				return;
			}

			pRow = pRow->GetNextRow();
		}

		//if we are still here, we didn't find a match, so add a new filter row

		CString strDescription = GetActionDescriptionForHL7Event(strMessageType, strEventType);

		IRowSettingsPtr pNewRow = m_ImportTypeFilterCombo->GetNewRow();
		pNewRow->PutValue(itfcMessageType, _bstr_t(strMessageType));
		pNewRow->PutValue(itfcEventType, _bstr_t(strEventType));		
		pNewRow->PutValue(itfcDescription, _bstr_t(strDescription));
		m_ImportTypeFilterCombo->AddRowSorted(pNewRow, NULL);

	} NxCatchAll("Error in CHL7BatchDlg::TryAddImportFilter");
}

// (j.jones 2008-04-17 16:20) - PLID 29701 - this function remove the matching row from the filter list if 
// no current messages are using it, but ONLY if the current selection is the filter in question, or "all" messages
void CHL7BatchDlg::TryRemoveImportFilter(CString strMessageType, CString strEventType)
{
	try {

		//before we can attempt to do anything here, we cannot know (without accessing the data)
		//if there are messages matching a filter unless we are currently using that filter, or showing all messages
		{
			IRowSettingsPtr pRow = m_ImportTypeFilterCombo->GetCurSel();
			if(pRow) {
				CString strMessageTypeToCheck = VarString(pRow->GetValue(itfcMessageType),"");
				CString strEventTypeToCheck = VarString(pRow->GetValue(itfcEventType),"");

				if(!strMessageTypeToCheck.IsEmpty() && !strEventTypeToCheck.IsEmpty()
					&& (strMessageTypeToCheck != strMessageType || strEventTypeToCheck != strEventType)) {

					//the selection doesn't match our filter, so we can't know whether we can safely remove
					//it without requerying, and we don't need to requery, it's not that important
					return;
				}
			}
		}

		//if there is no matching message in either the selected or unselected list,
		//remove the filter row

		//find the filter row in question
		IRowSettingsPtr pFilterRow = NULL;
		{
			IRowSettingsPtr pRow = m_ImportTypeFilterCombo->GetFirstRow();
			while(pRow != NULL && pFilterRow == NULL) {

				//if the row matches our message type and event, 

				CString strFilterMessageType = VarString(pRow->GetValue(itfcMessageType),"");
				CString strFilterEventType = VarString(pRow->GetValue(itfcEventType),"");

				if(!strFilterMessageType.IsEmpty() && !strFilterEventType.IsEmpty()
					&& strFilterMessageType == strMessageType && strFilterEventType == strEventType) {

					//there is a filter for this combination, so we're done
					pFilterRow = pRow;
				}

				pRow = pRow->GetNextRow();
			}
		}

		if(pFilterRow == NULL) {
			//this filter doesn't even exist
			return;
		}

		//now see if we are keeping the filter - if any row in either list
		//matches, when we won't remove it

		//first the unselected list
		{
			IRowSettingsPtr pRow = m_ImportUnselectedList->GetFirstRow();
			while(pRow) {

				//if the row matches our message type and event, we should not remove the filter row

				CString strMessageTypeToCheck = VarString(pRow->GetValue(ilcMessageType),"");
				CString strEventTypeToCheck = VarString(pRow->GetValue(ilcEventType),"");

				if((strMessageTypeToCheck.IsEmpty() || strEventTypeToCheck.IsEmpty())
					|| (strMessageTypeToCheck == strMessageType && strEventTypeToCheck == strEventType)) {

					//there is a message for this filter, so do not remove it
					return;
				}

				pRow = pRow->GetNextRow();
			}
		}

		//now the selected list
		{
			IRowSettingsPtr pRow = m_ImportSelectedList->GetFirstRow();
			while(pRow) {

				//if the row matches our message type and event, we should not remove the filter row

				CString strMessageTypeToCheck = VarString(pRow->GetValue(ilcMessageType),"");
				CString strEventTypeToCheck = VarString(pRow->GetValue(ilcEventType),"");

				if((strMessageTypeToCheck.IsEmpty() || strEventTypeToCheck.IsEmpty())
					|| (strMessageTypeToCheck == strMessageType && strEventTypeToCheck == strEventType)) {

					//there is a message for this filter, so do not remove it
					return;
				}

				pRow = pRow->GetNextRow();
			}
		}

		//if we get here, nothing satisfies this filter, so this row is doomed
		m_ImportTypeFilterCombo->RemoveRow(pFilterRow);

		//calling OnSelChosen will re-filter the list properly, even if the cur sel is NULL
		OnSelChosenImportTypeFilter(m_ImportTypeFilterCombo->GetCurSel());

	} NxCatchAll("Error in CHL7BatchDlg::TryRemoveImportFilter");
}

// (j.jones 2008-04-29 14:49) - PLID 29813 - this function will update the given message log ID on the screen
void CHL7BatchDlg::ProcessChangedHL7MessageLogID(long nID)
{
	try {

		if(nID == -1) {
			//we were not given a specific ID, so do nothing, the caller shouldn't have
			//called this function at all
			ASSERT(FALSE);
			return;
		}
		
		long nHL7GroupID = -1;
		IRowSettingsPtr pRow = m_ExportGroupFilterCombo->GetCurSel();
		if(pRow) {
			nHL7GroupID = VarLong(pRow->GetValue(egfcID),-1);
		}
		else {
			//no filter, which means we should not be displaying anything at all, so leave
			return;
		}

		//if we were given a specific ID, load up its information, but go ahead
		//and filter on the current group ID, since we won't display the message if
		//it doesn't match our filter
		// (z.manning 2008-07-17 08:21) - PLID 30490 - This query now handles appointment messages too.
		// (a.vengrofski 2010-07-28 11:37) - PLID <38919> - Removed Labs from showing up here
		// (c.haag 2010-08-11 09:31) - PLID 39799 - It's not enough that the sent date is not null; we also
		// need to verify that we got an ACKnowledgement for the message.
		//TES 6/22/2011 - PLID 44261 - The ExportType and ExpectACK settings are stored differently now.  Let's pull all the groups that
		// have both settings, and use that array in the query
		CArray<long,long> arHL7GroupsExportTcp;
		GetHL7SettingsGroupsBySetting("ExportType", (long)1, arHL7GroupsExportTcp);
		CArray<long,long> arHL7GroupsExpectACK;
		GetHL7SettingsGroupsBySetting("ExpectACK", TRUE, arHL7GroupsExpectACK);
		CArray<long,long> arHL7GroupsRequireACK;
		for(int i = 0; i < arHL7GroupsExportTcp.GetSize(); i++) {
			bool bMatched = false;
			for(int j = 0; j < arHL7GroupsExpectACK.GetSize() && !bMatched; j++) {
				if(arHL7GroupsExpectACK[j] == arHL7GroupsExportTcp[i]) {
					arHL7GroupsRequireACK.Add(arHL7GroupsExportTcp[i]);
					bMatched = true;
				}
			}
		}
	
		long nFailedACKCheckDelay = GetRemotePropertyInt("HL7Import_FailedACKCheckDelaySeconds", 300);
		// (r.gonet 02/26/2013) - PLID 47534 - Filtered out dismissed messages.
		_RecordsetPtr rs = CreateParamRecordset("SELECT MessageID, GroupID, CreateDate, SentDate, "
				"CASE WHEN PracticeRecordType = {INT} THEN PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle "
				"	  WHEN PracticeRecordType = {INT} THEN COALESCE(ApptPerson.Last, '<No Patient>') + ', ' + COALESCE(ApptPerson.First, '') + ' - ' + CAST(StartTime AS nvarchar(50)) "
				"	  ELSE '' END AS RecordDescription, "
				"MessageType, PracticeRecordType, PracticeRecordID, "
				"CONVERT(BIT, CASE WHEN (SentDate IS NOT NULL AND DATEDIFF(second, SentDate, GetDate()) > {INT} AND AcknowledgeDate IS NULL AND GroupID IN ({INTARRAY})) THEN 1 ELSE 0 END) AS AckMissing "
				"FROM HL7MessageLogT "
				"LEFT JOIN PersonT ON HL7MessageLogT.PracticeRecordID = PersonT.ID AND PracticeRecordType = {INT} "
				"LEFT JOIN (AppointmentsT LEFT JOIN PersonT ApptPerson ON AppointmentsT.PatientID = ApptPerson.ID) ON HL7MessageLogT.PracticeRecordID = AppointmentsT.ID AND PracticeRecordType = {INT} "
				"WHERE HL7MessageLogT.MessageID = {INT} AND HL7MessageLogT.GroupID = {INT} AND MessageType <> {INT} AND Dismissed = 0"
				, hprtPatient, hprtAppt, nFailedACKCheckDelay, arHL7GroupsRequireACK, hprtPatient, hprtAppt, nID, nHL7GroupID, hemtNewLab);
		if(!rs->eof) {
			
			//start by only retrieving the fields we need
			_variant_t vtSentDate = rs->Fields->Item["SentDate"]->Value;		
			BOOL bAckMissing = AdoFldBool(rs, "AckMissing");

			//if the SentDate is filled in, it means the record has been exported
			// (c.haag 2010-08-11 09:31) - PLID 39799 - It's only exported if the ACK is not missing
			if(vtSentDate.vt != VT_NULL && !bAckMissing) {
				//if it is in either list, remove the row
				IRowSettingsPtr pRow = m_ExportUnselectedList->FindByColumn(elcMessageID, (long)nID, m_ExportUnselectedList->GetFirstRow(), FALSE);
				if(pRow) {
					//found it in the unselected list, remove it
					m_ExportUnselectedList->RemoveRow(pRow);
				}
				else {
					//see if it is in the selected list
					pRow = m_ExportSelectedList->FindByColumn(elcMessageID, (long)nID, m_ExportSelectedList->GetFirstRow(), FALSE);
					if(pRow) {
						//found it in the selected list, remove it
						m_ExportSelectedList->RemoveRow(pRow);
					}
				}

				//we are done, so get out of here
				return;
			}

			//if we get here, the SentDate is NULL (or the ACK is missing), meaning that this message has not been exported and
			//does need to display in our list, so first see if it is not already in either list
			{
				IRowSettingsPtr pRow = m_ExportUnselectedList->FindByColumn(elcMessageID, (long)nID, m_ExportUnselectedList->GetFirstRow(), FALSE);
				if(pRow) {
					//found it in the unselected list, which means we don't need to update anything, so leave
					return;
				}
				else {
					//see if it is in the selected list
					pRow = m_ExportSelectedList->FindByColumn(elcMessageID, (long)nID, m_ExportSelectedList->GetFirstRow(), FALSE);
					if(pRow) {
						//found it in the selected list, which means we don't need to update anything, so leave
						return;
					}
				}
			}

			//if we get here, it means we need to add the message, so get the rest of the information we need
			
			long nGroupID = AdoFldLong(rs, "GroupID", -1);
			COleDateTime dtCreateDate = AdoFldDateTime(rs, "CreateDate", COleDateTime::GetCurrentTime());
			HL7PracticeRecordType hl7RecordType = (HL7PracticeRecordType)AdoFldLong(rs, "PracticeRecordType", (long)hprtPatient);
			long nRecordID = AdoFldLong(rs, "PracticeRecordID", -1);
			CString strDescription = AdoFldString(rs, "RecordDescription", "");

			HL7ExportMessageType hemtMessageType = (HL7ExportMessageType)AdoFldLong(rs, "MessageType");
			CString strMessageType = "";
			switch(hemtMessageType)
			{
				case hemtAddNewPatient:
					strMessageType = "Add New Patient";
					break;

				case hemtUpdatePatient:
					strMessageType = "Update Existing Patient";
					break;

				case hemtNewAppt:
					strMessageType = "Add New Appointment";
					break;

				case hemtUpdateAppt:
					strMessageType = "Update Existing Appointment";
					break;

				case hemtCancelAppt:
					strMessageType = "Cancel Appointment";
					break;

				//TES 7/10/2009 - PLID 25154 - Added Bills (from EMR)
				case hemtNewEmnBill:
					strMessageType = "Add New Bill";
					break;

				//TES 5/26/2010 - PLID 38541 - Added Lab Orders
				case hemtNewLab:
					strMessageType = "New Lab Order";
					break;

				default:
					ASSERT(FALSE);
					break;
			}

			//add to the unselected list
			{
				IRowSettingsPtr pRow = m_ExportUnselectedList->GetNewRow();
				pRow->PutValue(elcMessageID, (long)nID);
				pRow->PutValue(elcGroupID, (long)nGroupID);
				pRow->PutValue(elcPracticeRecordType, (long)hl7RecordType);
				pRow->PutValue(elcPracticeRecordID, (long)nRecordID);
				pRow->PutValue(elcMessageType, _bstr_t(strMessageType));
				// (r.gonet 12/03/2012) - PLID 54114 - Add in the message type id
				pRow->PutValue(elcMessageTypeID, (long)hemtMessageType);
				pRow->PutValue(elcDescription, _bstr_t(strDescription));
				pRow->PutValue(elcCreateDate, _variant_t(dtCreateDate, VT_DATE));
				m_ExportUnselectedList->AddRowSorted(pRow, NULL);
			}
		}
		rs->Close();

	} NxCatchAll("Error in CHL7BatchDlg::ProcessChangedHL7MessageLogID");
}

void CHL7BatchDlg::OnShowProcessed() 
{
	try {
		//TES 5/12/2008 - PLID 13339 - Show/Hide the "Action Taken" column appropriately.
		if(IsDlgButtonChecked(IDC_SHOW_PROCESSED)) {
			m_ImportSelectedList->GetColumn(ilcActionTaken)->PutStoredWidth(70);
			m_ImportUnselectedList->GetColumn(ilcActionTaken)->PutStoredWidth(70);
		}
		else {
			m_ImportSelectedList->GetColumn(ilcActionTaken)->PutStoredWidth(0);
			m_ImportUnselectedList->GetColumn(ilcActionTaken)->PutStoredWidth(0);
		}
		
		//TES 5/12/2008 - PLID 13339 - Now requery the lists to take the new filter into account.
		RequeryImportLists();
	}NxCatchAll("Error in CHL7BatchDlg::OnShowProcessed()");
}

void CHL7BatchDlg::OnSelChosenHl7ExportGroupFilter(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL) {
			//select the first option
			pRow = m_ExportGroupFilterCombo->GetFirstRow();
			if(pRow) {
				m_ExportGroupFilterCombo->PutCurSel(pRow);
			}
			else {
				//there are no groups, clear both lists and leave
				m_ExportUnselectedList->Clear();
				m_ExportSelectedList->Clear();
				return;
			}
		}

		RequeryExportLists();

	}NxCatchAll("Error in CHL7BatchDlg::OnSelChosenHl7ExportGroupFilter");
}

void CHL7BatchDlg::OnBtnExportPatients() 
{
	try {
		//TES 5/18/2009 - PLID 34282 - Check that they have permission to export patients.
		if(CheckCurrentUserPermissions(bioHL7Patients, sptDynamic0)) {
			CHL7ExportDlg dlg;
			// (z.manning 2008-07-18 14:01) - PLID 30782 - Need to specify the type of export
			dlg.DoModal(hprtPatient);

			//if they exported any patients, we'll receive a tablechecker if they
			//were batched and the export lists need reloaded
		}

	}NxCatchAll("Error in CHL7BatchDlg::OnBtnExportPatients");
}

// (r.farnworth 2014-12-22 11:51) - PLID 64473 - Add support to export MFN HL7 messages - Add an Export Referring Physicians button and functionality on the Links->HL7 tab
void CHL7BatchDlg::OnBtnExportRefPhys()
{
	try {
		if (CheckCurrentUserPermissions(bioHL7RefPhys, sptDynamic0)) {
			CHL7ExportDlg dlg;
			dlg.DoModal(hprtReferringPhysician);
		}

	}NxCatchAll("Error in CHL7BatchDlg::OnBtnExportRefPhys");
}

// (z.manning 2008-07-18 14:38) - PLID 30782 - Added a button to export appointments
void CHL7BatchDlg::OnBtnExportAppts() 
{
	try {

		//TES 5/18/2009 - PLID 34282 - Check that they have permission to export appointments.
		if(CheckCurrentUserPermissions(bioHL7Appointments, sptDynamic0)) {
			CHL7ExportDlg dlg;
			dlg.DoModal(hprtAppt);

			//if they exported any appts, we'll receive a tablechecker if they
			//were batched and the export lists need reloaded
		}

	}NxCatchAll("CHL7BatchDlg::OnBtnExportAppts");
}

//TES 7/10/2009 - PLID 34845 - Added a button to export EMN bills
void CHL7BatchDlg::OnBtnExportEmnBills()
{
	try {

		//TES 5/18/2009 - PLID 34282 - Check that they have permission to export bills.
		if(g_pLicense->HasEMR(CLicense::cflrUse) == 2 && CheckCurrentUserPermissions(bioHL7EmnBills, sptDynamic0)) {
			CHL7ExportDlg dlg;
			dlg.DoModal(hprtEmnBill);

			//if they exported any bills, we'll receive a tablechecker if they
			//were batched and the export lists need reloaded
		}

	}NxCatchAll("CHL7BatchDlg::OnBtnExportEmnBills");
}

// (a.walling 2010-02-22 10:46) - PLID 37154 - Export syndromic surveillance data
void CHL7BatchDlg::OnBtnExportSyndromicData()
{
	try {
		// (a.walling 2010-02-23 17:42) - PLID 37483 - Get a filter
		CString strFilter;
		{
			CExternalForm Form(this);
			Form.m_bRequireSelection = true;
			Form.m_bDefaultRemember = true;

			Form.m_Caption = "Syndromic Surveillance Reporting";
			// (r.gonet 03/18/2014) - PLID 60782 - Added an additional hidden column for DiagCodes.ID so we can get
			// those IDs back rather than potentially non-unique CodeNumbers.
			Form.m_ColFormat = "DiagCodeID|CodeNumber|CodeDesc";
			Form.m_ColWidths = "0|25|-1";
			Form.m_BoundCol = 2;
			Form.m_SortBy = "2";
			Form.m_SQL = "SELECT ID, CodeNumber, CodeDesc FROM DiagCodes";
			Form.m_Filter = &strFilter;
			Form.m_RepID = "-100";

			if (IDCANCEL == Form.DoModal()) {
				return;
			}
		}

		if (strFilter.IsEmpty()) {
			MessageBox("You must select a subset of diagnosis codes to run this report.", NULL, MB_ICONSTOP);
			return;
		}

		// (b.spivey, June 25, 2013) - PLID 57316 - Each message is its own element in this array.
		CArray<CString, LPCSTR> aryMessages; 
		{
			CWaitCursor cws;
			// (a.walling 2010-02-23 17:42) - PLID 37483 - We use a configurable filter now
			// (r.gonet 12/03/2012) - PLID 54112 - Create a session with certain state variables
			CHL7Session session(GetCurrentLocationID(), GetCurrentLocationName(), GetCurrentUserID(), GetCurrentUserName());
			// (r.gonet 12/03/2012) - PLID 54112 - Create the syndromic batch message from the HL7 factory.
			CHL7MessageFactory factory(&session, GetRemoteData(), GetHL7SettingsCache());
			
			// (j.armen 2013-01-28 13:47) - PLID 54224 - Parse the codes from the filter and place them in a string array
			// (r.gonet 03/18/2014) - PLID 60782 - Changed the array to be an array of diagnosis code IDs rather than diagnosis code numbers due to ICD-9/ICD-10 overlap.
			CArray<long, long> aryDiagCodeIDs;
			StringAsArray(strFilter.Trim().Mid(1, strFilter.GetLength() - 2), aryDiagCodeIDs);

			// (b.spivey, November 1, 2013) - PLID 59267 - bit of a hack, had to use this dialog to get a list of personIDs
			CHL7ExportDlg dlg;

			CArray<long, long> aryPersonIDs;
			// (r.gonet 03/18/2014) - PLID 60782 - Changed the array to be an array of diagnosis code IDs rather than diagnosis code numbers due to ICD-9/ICD-10 overlap.
			dlg.SetDiagnosisCodeArray(aryDiagCodeIDs); 
			// (b.spivey, November 1, 2013) - PLID 59267 - sadly OnHl7Close returns IDOK instead of IDCLOSE, so we had to do a hack around this. 
			if (dlg.DoModal(hprtSyndromicSurveillance) == IDCANCEL) {
				dlg.GetSyndromicPersonIDs(aryPersonIDs); 
			}
			else {
				return;
			}

			// (b.spivey, June 25, 2013) - PLID 57316 - Get the individual messages for saving.
			// (r.gonet 03/18/2014) - PLID 60782 - Changed the array to be an array of diagnosis code IDs rather than diagnosis code numbers due to ICD-9/ICD-10 overlap.
			factory.CreateSyndromicBatch(aryDiagCodeIDs, aryPersonIDs, aryMessages);
		}

		if (aryMessages.GetSize() <= 0) {
			MessageBox("There were no results!");
			return;
		}

		// (b.spivey, June 26, 2013) - PLID 57316 - moved this because selecting where to save the document first just didn't flow right.
		CString strFileName = FormatString("Syndromic Surveillance Data %s.hl7", COleDateTime::GetCurrentTime().Format("%Y%m%d%H%M%S"));

		CFileDialog SaveAs(FALSE, NULL, strFileName, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "HL7 Files (*.hl7)|*.hl7|All Files (*.*)|*.*||");

		if (SaveAs.DoModal() == IDCANCEL) {
			return;
		}

		CString strPathName = SaveAs.GetFolderPath();
		CString strExtName = SaveAs.GetFileExt(); 
		strFileName =  SaveAs.GetFileTitle();


		// (b.spivey, June 26, 2013) - PLID 57316 - We have to export each file individually-- batching is noncompliant for MU2. 
		for (int i = 0; i < aryMessages.GetSize(); i++) {
			CString strFileNameFormat = "";
			strFileNameFormat.Format("_%li.", i); 

			strFileNameFormat = strPathName ^ (strFileName + strFileNameFormat + strExtName);

			CFile f(strFileNameFormat, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite);
			f.Write(aryMessages.GetAt(i), aryMessages.GetAt(i).GetLength());
		}

		MessageBox(FormatString("The exported HL7 batch has been saved to %s.", strPathName));
	}NxCatchAll("CHL7BatchDlg::OnBtnExportSyndromicData");
}

// (a.vengrofski 2010-05-11 12:16) - PLID <38547> 
void CHL7BatchDlg::OnBtnExportHL7Click()
{
	try {
			OnBtnHl7Export();
	}NxCatchAll("CHL7BatchDlg::OnBtnExportHL7Click");
}

// (c.haag 2010-08-11 09:43) - PLID 39799 - If the user clicked on the icon column, explain what it means
void CHL7BatchDlg::LeftClickHl7ExportUnselectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		if (elcFlag == nCol) {
			IRowSettingsPtr pRow(lpRow);
			if (NULL != pRow && pRow->GetValue(elcFlag).vt != VT_EMPTY) {
				AfxMessageBox("This record was previously exported, but the receiving server never responded with an acknowledgement. "
					"This could be due to a communication problem or the server not responding in a timely manner.\n\n"
					"It is recommended that you attempt to export this record again in case the server never received it originally.",
					MB_OK | MB_ICONERROR);
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-08-11 09:43) - PLID 39799 - If the user clicked on the icon column, explain what it means
void CHL7BatchDlg::LeftClickHl7ExportSelectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		if (elcFlag == nCol) {
			IRowSettingsPtr pRow(lpRow);
			if (NULL != pRow && pRow->GetValue(elcFlag).vt != VT_EMPTY) {
				AfxMessageBox("This record was previously exported, but the receiving server never responded with an acknowledgement. "
					"This could be due to a communication problem or the server not responding in a timely manner.\n\n"
					"It is recommended that you attempt to export this record again in case the server never received it originally.",
					MB_OK | MB_ICONERROR);
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CHL7BatchDlg::SetLastRightClickList(_DNxDataListPtr pdl, BOOL bImportList)
{
	m_pdlLastRightClickedHL7List = pdl;
	m_bLastRightClickListWasImport = bImportList;
}

// (z.manning 2011-07-08 16:15) - PLID 38753
void CHL7BatchDlg::OnViewHL7Message()
{
	try
	{
		if(m_pdlLastRightClickedHL7List == NULL) {
			return;
		}

		IRowSettingsPtr pRow = m_pdlLastRightClickedHL7List->GetCurSel();
		if(pRow == NULL) {
			return;
		}

		if(m_bLastRightClickListWasImport) {
			ViewHL7ImportMessage(VarLong(pRow->GetValue(ilcID)));
		}
		else {
			ViewHL7ExportMessage(VarLong(pRow->GetValue(elcMessageID)));
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-07-08 16:36) - PLID 38753
void CHL7BatchDlg::RButtonDownHl7ExportUnselectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		SetLastRightClickList(m_ExportUnselectedList, FALSE);
		m_ExportUnselectedList->PutCurSel(IRowSettingsPtr(lpRow));
		ShowExportListMenu();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-07-08 16:36) - PLID 38753
void CHL7BatchDlg::RButtonDownHl7ExportSelectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		SetLastRightClickList(m_ExportSelectedList, FALSE);
		m_ExportSelectedList->PutCurSel(IRowSettingsPtr(lpRow));
		ShowExportListMenu();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-07-08 16:36) - PLID 38753
void CHL7BatchDlg::ShowExportListMenu()
{
	if(m_pdlLastRightClickedHL7List == NULL) {
		return;
	}

	if(m_pdlLastRightClickedHL7List->GetCurSel() == NULL) {
		return;
	}

	CMenu mnu;
	mnu.CreatePopupMenu();

	// (r.gonet 02/26/2013) - PLID 47534 - Add the dismiss option to the right click menu for exported messages.
	mnu.AppendMenu(MF_ENABLED|MF_BYCOMMAND, ID_EXPORT_DISMISS, "&Dismiss");
	mnu.AppendMenu(MF_ENABLED|MF_BYCOMMAND, ID_VIEW_HL7_MESSAGE, "&View HL7 Message");

	CPoint pt;
	GetCursorPos(&pt);
	mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
}

// (d.singleton 2012-10-04 12:42) - PLID 53041 dialog to export locked emns
void CHL7BatchDlg::OnBnClickedBtnExportEMNs()
{
	try {
		CHL7ExportDlg dlg;
		dlg.DoModal(hprtLockedEmn);
	}NxCatchAll(__FUNCTION__);
}

// (s.tullis 2015-06-02 16:09) - PLID 66211
// Load datalist lengths
// default lengths are defined in the resource
void CHL7BatchDlg::LoadDatalistColumnLength()
{
	try{
		CString strColWidths = GetRemotePropertyText("DefaultLinksHL7ColumnSizes", "", 0, GetCurrentUserName(), true);

		if (strColWidths.IsEmpty())
		{
			return;
		}
		
		std::vector<CString> arrDataListsLengths = GetDatalistColumnLengths(strColWidths);

		PutDataListColumnWidth(m_ExportUnselectedList, GetDatalistColumnIntLengths(arrDataListsLengths.at(0)));
	
		PutDataListColumnWidth(m_ExportSelectedList, GetDatalistColumnIntLengths(arrDataListsLengths.at(1)));

		PutDataListColumnWidth(m_ImportUnselectedList, GetDatalistColumnIntLengths(arrDataListsLengths.at(2)));

		PutDataListColumnWidth(m_ImportSelectedList, GetDatalistColumnIntLengths(arrDataListsLengths.at(3)));

	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2015-06-02 16:09) - PLID 66211
// Save Datalist Width Value to Remote Property
void CHL7BatchDlg::SaveDatalistColumnLength()
{
	try{
		CString strWidths = FormatString("%s;%s;%s;%s",
		GetstrDataColumnWidths(m_ExportUnselectedList),
		GetstrDataColumnWidths(m_ExportSelectedList),
		GetstrDataColumnWidths(m_ImportUnselectedList),
		GetstrDataColumnWidths(m_ImportSelectedList));

		SetRemotePropertyText("DefaultLinksHL7ColumnSizes", strWidths, 0, GetCurrentUserName());
		
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2015-06-02 16:09) - PLID 66211
// Takes in a string of comma delimited string
// returns vector of seperated widths
std::vector<long> CHL7BatchDlg::GetDatalistColumnIntLengths(CString strDatalistColumnWidths)
{
	std::vector<long> arrColumnLengths;
	int nCurIndex = 0;
		try{
			while (nCurIndex != -1)
			{
				arrColumnLengths.push_back(atoi(strDatalistColumnWidths.Tokenize(_T(","), nCurIndex)));
			}

			arrColumnLengths.pop_back();
		}NxCatchAll(__FUNCTION__)
		
	return arrColumnLengths;
}
// (s.tullis 2015-06-02 16:09) - PLID 66211
// Takes in a string of colon seperated strings
// returns vector of seperated datalist comma delimited string widths
std::vector<CString> CHL7BatchDlg::GetDatalistColumnLengths(CString strDatalistColumnWidths)
{
	std::vector<CString> arrColumnLengths;
	int nCurIndex = 0;
	try{
		while (nCurIndex != -1)
		{
			arrColumnLengths.push_back((strDatalistColumnWidths.Tokenize(_T(";"), nCurIndex)));
		}
		arrColumnLengths.pop_back();
	}NxCatchAll(__FUNCTION__)

	return arrColumnLengths;
}
// (s.tullis 2015-06-02 16:09) - PLID 66211
// Put the saved datalist width values
void CHL7BatchDlg::PutDataListColumnWidth(NXDATALIST2Lib::_DNxDataListPtr pdl, std::vector<long> arrWidths)
{
	try{
		//if for some reason the columns changes just use the default in the resource
		// IE someone added a column 
		if (pdl->ColumnCount != arrWidths.size())
		{
			return;
		}

		for (short i = 0; i < pdl->ColumnCount; i++)
		{
			IColumnSettingsPtr pCol = pdl->GetColumn(i);
			long nStyle = pCol->ColumnStyle;
			nStyle = (nStyle&(~csWidthAuto)&(~csWidthData));
			pCol->PutColumnStyle(nStyle);
			pCol->StoredWidth = arrWidths.at(i);
		}
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2015-06-02 16:09) - PLID 66211
// Assign the editing flag (to save) if the datalists have had their columns resized
void CHL7BatchDlg::ColumnSizingFinishedHl7List(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try {

		m_bEditedDataListLength = TRUE;

	}NxCatchAll(__FUNCTION__);
}
// (s.tullis 2015-06-02 16:09) - PLID 66211
// save the datalist when they stop editing if they resized
void CHL7BatchDlg::FocusLostHl7List()
{
	try {

		if (m_bEditedDataListLength){
			SaveDatalistColumnLength();
			m_bEditedDataListLength = FALSE;
		}

	}NxCatchAll(__FUNCTION__);
}
// (s.tullis 2015-06-02 16:09) - PLID 66211
// take in a datalist point
// returns comma delimited string of the datalist column widths
CString CHL7BatchDlg::GetstrDataColumnWidths(NXDATALIST2Lib::_DNxDataListPtr pdl){
	CString strWidths = "";
	try{
		for (short i = 0; i < pdl->ColumnCount; i++)
		{
			IColumnSettingsPtr pCol = pdl->GetColumn(i);
			strWidths += (i == pdl->ColumnCount-1) ? FormatString("%li", pCol->StoredWidth) : FormatString("%li,", pCol->StoredWidth);
		}
	}NxCatchAll(__FUNCTION__)
	return strWidths;
}


