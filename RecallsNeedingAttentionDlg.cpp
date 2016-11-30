// RecallsNeedingAttentionDlg.cpp : implementation file
//

// (j.armen 2012-02-24 16:19) - PLID 48303 - Created

#include "stdafx.h"
#include "PatientsRc.h"
#include "RecallsNeedingAttentionDlg.h"
#include "RecallUtils.h"
#include "GlobalDataUtils.h"
#include "schedulerView.h"
#include "MultiSelectDlg.h"
#include "CreatePatientRecall.h"
// (b.savon 2012-03-01 12:31) - PLID 48486
#include "LinkRecallToExistingAppointmentDlg.h" 
// (b.savon 2012-03-02 16:41) - PLID 48474
#include "MergeEngine.h"
#include "LetterWriting.h"
// (b.savon 2012-03-09 12:09) - PLID 48763
#include "AuditTrail.h"
#include "NotesDlg.h"			// (j.armen 2012-03-19 09:03) - PLID 48780
#include "NxModalParentDlg.h"	// (j.armen 2012-03-19 09:03) - PLID 48780
#include "PatientReminderSenthistoryUtils.h"
#include "NxSchedulerDlg.h"
#include "FirstAvailableAppt.h" // (r.gonet 2014-11-19) - PLID 64173 - Sorry, the elimination of this header from mainfrm.h necessitated this
								// header's inclusion.
#include "NxWordProcessorLib\GenericWordProcessorManager.h"

// CRecallsNeedingAttentionDlg

using namespace NXDATALIST2Lib;
using namespace ADODB;

IMPLEMENT_DYNAMIC(CRecallsNeedingAttentionDlg, CNxDialog)

// (j.armen 2012-03-09 17:56) - PLID 48462 - Template List Column Enum
namespace TemplateList {
	enum {
		ID,
		Name,
	};
}

// (j.armen 2012-03-12 09:35) - PLID 48460 - Provider List Column Enum
namespace ProviderList {
	enum {
		ID,
		Name,
	};
}

// (j.armen 2012-03-12 09:31) - PLID 48483 - Location List Column Enum
namespace LocationList {
	enum {
		ID,
		Name,
	};
}

// (j.armen 2012-03-09 16:45) - PLID 48546 - Status List Column Enum
namespace StatusList {
	enum {
		ID,
		Name,
	};
}

// (j.armen 2012-03-20 13:39) - PLID 48913 - Added MasterGroupID, ParentGroupID, SetOrder
namespace RecallList {
	enum {
		MasterGroupID,
		ParentGroupID,
		RecallID,
		ProviderName,
		PatientID,
		PatientName,
		PatientPrefContact,
		Date,
		LocationID,
		LocationName,
		TemplateName,
		StepName,
		SetOrder,
		AppointmentID,
		AppointmentDate,
		AppointmentType,
		AppointmentPurpose,
		StatusColor,
		StatusID,
		StatusName,
		Discontinued,
		NotesIcon,
	};
}

// (j.armen 2012-06-05 16:04 ) - PLID 50805 - Pass in a ConfigRT entry so that this dlg will save it's size
CRecallsNeedingAttentionDlg::CRecallsNeedingAttentionDlg(CWnd* pParent) 
	: CNxDialog(CRecallsNeedingAttentionDlg::IDD, pParent, "CRecallsNeedingAttentionDlg")
{
	m_bUsePatientID = false;					// (j.armen 2012-02-28 14:57) - PLID 48452
	m_dtStart.SetStatus(COleDateTime::null);	// (j.armen 2012-02-28 16:30) - PLID 48463
	COleDateTime dtNow = COleDateTime::GetCurrentTime();
	// (j.armen 2012-03-20 13:40) - PLID 48303 - Default the End Date to be 30 days into the future.
	m_dtEnd.SetDate(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay());	// (j.armen 2012-02-28 16:31) - PLID 48463
	m_dtEnd += COleDateTimeSpan(30, 0, 0, 0);
	m_bNeedsRefresh = FALSE;
}

CRecallsNeedingAttentionDlg::~CRecallsNeedingAttentionDlg()
{
	DestroyIcon(m_hNotes);
}

BEGIN_MESSAGE_MAP(CRecallsNeedingAttentionDlg, CNxDialog)
	ON_WM_CONTEXTMENU()
	ON_WM_SETCURSOR()
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_RECALL_DATERANGE_END, OnDtnDatetimechangeRecallDaterange)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_RECALL_DATERANGE_START, OnDtnDatetimechangeRecallDaterange)
	ON_COMMAND(ID_RECALL_SCHEDULER, OnScheduler)
	ON_COMMAND(ID_RECALL_FFA, OnFFA)
	ON_COMMAND(ID_RECALL_DISCONTINUE, OnSwapDiscontinueStatus)
	ON_COMMAND(ID_RECALL_RESUME, OnSwapDiscontinueStatus)
	ON_COMMAND(ID_LINK_TO_EXIST_APPT, OnLinkExistingAppointment)
	ON_COMMAND(ID_RECALLLISTPOPUP_UNLINKEXISTINGAPPOINTMENT, OnUnlinkExistingAppointment)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_BN_CLICKED(IDC_BTN_MERGE_TO_WORD, &CRecallsNeedingAttentionDlg::OnBnClickedBtnMergeToWord)
	ON_COMMAND(ID_RECALL_APPOINTMENT, OnAppointment)
	ON_BN_CLICKED(IDOK, &CRecallsNeedingAttentionDlg::OnBnClickedOk)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BTN_CREATEMERGEGROUP, &CRecallsNeedingAttentionDlg::OnBnClickedBtnCreatemergegroup)
	ON_BN_CLICKED(IDC_ADD_PT_REMINDER_RECALL, &CRecallsNeedingAttentionDlg::OnBnClickedAddPtReminderRecall)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CRecallsNeedingAttentionDlg, CNxDialog)
	ON_EVENT(CRecallsNeedingAttentionDlg, IDC_NXDL_RECALL_PROVIDERS, 16, SelChosenRecallProviders, VTS_DISPATCH)	// (j.armen 2012-03-12 09:29) - PLID 48460
	ON_EVENT(CRecallsNeedingAttentionDlg, IDC_NXDL_RECALL_TEMPLATES, 16, SelChosenRecallTemplates, VTS_DISPATCH)	// (j.armen 2012-03-09 17:52) - PLID 48462
	ON_EVENT(CRecallsNeedingAttentionDlg, IDC_NXDL_RECALL_LOCATIONS, 16, SelChosenRecallLocations, VTS_DISPATCH)	// (j.armen 2012-03-12 09:29) - PLID 48483
	ON_EVENT(CRecallsNeedingAttentionDlg, IDC_NXDL_RECALL_STATUS, 16, SelChosenRecallStatus, VTS_DISPATCH)			// (j.armen 2012-03-09 16:47) - PLID 48546
	ON_EVENT(CRecallsNeedingAttentionDlg, IDC_NXDL_RECALL_DISPLAY, 19 , OnLeftClickRecallList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CRecallsNeedingAttentionDlg, IDC_NXDL_RECALL_DISPLAY, 6, OnRightClickRecallList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CRecallsNeedingAttentionDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NXCOLORCTRL1, m_nxcRecallBkg);
	DDX_Control(pDX, IDC_RECALL_CURRENT_PAT, m_btnCurrentPat);
	DDX_Check(pDX, IDC_RECALL_CURRENT_PAT, m_bUsePatientID);
	DDX_Control(pDX, IDC_RECALL_DATERANGE_START, m_cdtStart);
	DDX_DateTimeCtrl(pDX, IDC_RECALL_DATERANGE_START, m_dtStart);
	DDX_Control(pDX, IDC_RECALL_DATERANGE_END, m_cdtEnd);
	DDX_DateTimeCtrl(pDX, IDC_RECALL_DATERANGE_END, m_dtEnd);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_NXL_RECALL_PROVIDERS, m_nxlProviders);
	DDX_Control(pDX, IDC_NXL_RECALL_TEMPLATES, m_nxlTemplates);
	DDX_Control(pDX, IDC_NXL_RECALL_LOCATIONS, m_nxlLocations);
	DDX_Control(pDX, IDC_NXL_RECALL_STATUS, m_nxlStatus);
	DDX_Control(pDX, IDC_BTN_MERGE_TO_WORD, m_btnMergeToWord);
	DDX_Control(pDX, IDC_RECALL_CREATE, m_btnCreateRecall);
	DDX_Control(pDX, IDC_BTN_CREATEMERGEGROUP, m_btnCreateMergeGroup);
	DDX_Control(pDX, IDC_CHK_REMEMBER_COLUMNS, m_btnRememberColumns);
	DDX_Control(pDX, IDC_ADD_PT_REMINDER_RECALL, m_btnAddPatientReminder);
}

BOOL CRecallsNeedingAttentionDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try
	{
		HICON hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_RECALL));
		SetIcon(hIcon, TRUE);		// Set big icon
		SetIcon(hIcon, FALSE);		// Set small icon

		m_nxcRecallBkg.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_pdlRecallList = BindNxDataList2Ctrl(IDC_NXDL_RECALL_DISPLAY, false);
		m_pdlProviderList = BindNxDataList2Ctrl(IDC_NXDL_RECALL_PROVIDERS, true);
		m_pdlTemplateList = BindNxDataList2Ctrl(IDC_NXDL_RECALL_TEMPLATES, true);
		m_pdlLocationList = BindNxDataList2Ctrl(IDC_NXDL_RECALL_LOCATIONS, true);
		m_pdlStatusList = BindNxDataList2Ctrl(IDC_NXDL_RECALL_STATUS, false);

		{
			// (j.armen 2012-02-28 15:57) - PLID 48460 - Add All and Multiple options
			IRowSettingsPtr pRow = m_pdlProviderList->GetNewRow();
			pRow->PutValue(ProviderList::ID, eIDAll);
			pRow->PutValue(ProviderList::Name, _bstr_t(" {All Providers} "));
			m_pdlProviderList->AddRowSorted(pRow, NULL);
			pRow = m_pdlProviderList->GetNewRow();
			pRow->PutValue(ProviderList::ID, eIDMultiple);
			pRow->PutValue(ProviderList::Name, _bstr_t(" {Multiple Providers} "));
			m_pdlProviderList->AddRowSorted(pRow, NULL);
		}

		{
			// (j.armen 2012-02-28 16:28) - PLID 48462 - Add All and Multiple options
			IRowSettingsPtr pRow = m_pdlTemplateList->GetNewRow();
			pRow->PutValue(TemplateList::ID, eIDAll);
			pRow->PutValue(TemplateList::Name, _bstr_t(" {All Recall Templates} "));
			m_pdlTemplateList->AddRowSorted(pRow, NULL);
			pRow = m_pdlTemplateList->GetNewRow();
			pRow->PutValue(TemplateList::ID, eIDMultiple);
			pRow->PutValue(TemplateList::Name, _bstr_t(" {Multiple Recall Templates} "));
			m_pdlTemplateList->AddRowSorted(pRow, NULL);
		}

		{
			// (j.armen 2012-03-01 09:47) - PLID 48483 - Add All and Multiple options
			IRowSettingsPtr pRow = m_pdlLocationList->GetNewRow();
			pRow->PutValue(LocationList::ID, eIDAll);
			pRow->PutValue(LocationList::Name, _bstr_t(" {All Locations} "));
			m_pdlLocationList->AddRowSorted(pRow, NULL);
			pRow = m_pdlLocationList->GetNewRow();
			pRow->PutValue(LocationList::ID, eIDMultiple);
			pRow->PutValue(LocationList::Name, _bstr_t(" {Multiple Locations} "));
			m_pdlLocationList->AddRowSorted(pRow, NULL);
		}

		{
			// (j.armen 2012-03-01 09:47) - PLID 48546 - Add statuses to the list manually
			IRowSettingsPtr pRow = m_pdlStatusList->GetNewRow();
			pRow->PutValue(StatusList::ID, eIDAll);
			pRow->PutValue(StatusList::Name, _bstr_t(" {All Statuses} "));
			m_pdlStatusList->AddRowSorted(pRow, NULL);
			pRow = m_pdlStatusList->GetNewRow();
			pRow->PutValue(StatusList::ID, eIDMultiple);
			pRow->PutValue(StatusList::Name, _bstr_t(" {Multiple Statuses} "));
			m_pdlStatusList->AddRowSorted(pRow, NULL);
			pRow = m_pdlStatusList->GetNewRow();
			pRow->PutValue(StatusList::ID, RecallUtils::eDiscontinued);
			pRow->PutValue(StatusList::Name, _bstr_t("Discontinued"));
			pRow->PutBackColor(RecallUtils::eDiscontinuedColor);
			m_pdlStatusList->AddRowSorted(pRow, NULL);
			pRow = m_pdlStatusList->GetNewRow();
			pRow->PutValue(StatusList::ID, RecallUtils::eComplete);
			pRow->PutValue(StatusList::Name, _bstr_t("Complete"));
			pRow->PutBackColor(RecallUtils::eCompleteColor);
			m_pdlStatusList->AddRowSorted(pRow, NULL);
			pRow = m_pdlStatusList->GetNewRow();
			pRow->PutValue(StatusList::ID, RecallUtils::eScheduled);
			pRow->PutValue(StatusList::Name, _bstr_t("Scheduled"));
			pRow->PutBackColor(RecallUtils::eScheduledColor);
			m_pdlStatusList->AddRowSorted(pRow, NULL);
			pRow = m_pdlStatusList->GetNewRow();
			pRow->PutValue(StatusList::ID, RecallUtils::eNeedToSchedule);
			pRow->PutValue(StatusList::Name, _bstr_t("Need to Schedule"));
			pRow->PutBackColor(RecallUtils::eNeedToScheduleColor);
			m_pdlStatusList->AddRowSorted(pRow, NULL);
			pRow = m_pdlStatusList->GetNewRow();
			pRow->PutValue(StatusList::ID, RecallUtils::ePastDue);
			pRow->PutValue(StatusList::Name, _bstr_t("Past Due"));
			pRow->PutBackColor(RecallUtils::ePastDueColor);
			m_pdlStatusList->AddRowSorted(pRow, NULL);

			// (j.armen 2012-03-20 13:41) - PLID 48303 - By default, filter on scheduled, need to schedule, and past due.
			// Excludes Complete/Discontinued
			m_aryStatusIDs.Add(RecallUtils::eScheduled);
			m_aryStatusIDs.Add(RecallUtils::eNeedToSchedule);
			m_aryStatusIDs.Add(RecallUtils::ePastDue);
		}

		// (j.armen 2012-02-28 16:28) - PLID 48460 - Set Provider Filter label settings
		m_nxlProviders.SetType(dtsHyperlink);
		m_nxlProviders.SetSingleLine();

		// (j.armen 2012-02-28 16:28) - PLID 48462 - Set Template Filter label settings
		m_nxlTemplates.SetType(dtsHyperlink);
		m_nxlTemplates.SetSingleLine();

		// (j.armen 2012-03-01 09:47) - PLID 48483 - Set Location Filter label settings
		m_nxlLocations.SetType(dtsHyperlink);
		m_nxlLocations.SetSingleLine();

		// (j.armen 2012-03-01 09:47) - PLID 48546 - Set Status Filter label settings
		m_nxlStatus.SetType(dtsHyperlink);
		m_nxlStatus.SetSingleLine();

		m_btnClose.AutoSet(NXB_CLOSE);
		// (b.savon 2012-03-02 16:41) - PLID 48474 - Add ability to Merge To Word from Recall Dlg
		m_btnMergeToWord.AutoSet(NXB_MERGE);
		m_btnCreateRecall.AutoSet(NXB_RECALL);
		// (b.savon 2012-03-09 12:09) - PLID 48763 - Create merge group
		m_btnCreateMergeGroup.AutoSet(NXB_NEW);


		g_propManager.CachePropertiesInBulk("CRecallsNeedingAttentionDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'RecallNotesDefaultCategory' " // (j.armen 2012-03-22 09:12) - PLID 48919
			"OR Name = 'dontshow RecallPatientReminder' " // (b.savon 2014-09-02 13:18) - PLID 62791
			")",
			_Q(GetCurrentUserName()));

		// (b.savon 2012-03-08 14:34) - PLID 48716 - Remember column widths
		g_propManager.CachePropertiesInBulk("CRecallsNeedingAttentionDlg_Columns", propText,
		"(Username = '<None>' OR Username = '%s') AND \r\n"
		"(Name = 'RecallsNeedingAttentionColumnWidths' \r\n"
		")"
		, _Q(GetCurrentUserName()));

		CString strColumns = GetRemotePropertyText("RecallsNeedingAttentionColumnWidths", "", 0, GetCurrentUserName(), true);
		if( strColumns != "" ){
			m_btnRememberColumns.SetCheck(TRUE);
			RestoreColumnWidths();
		}else{
			m_btnRememberColumns.SetCheck(FALSE);
		}

		// (j.armen 2012-03-19 09:04) - PLID 48780 - Load our Note Icon
		m_hNotes = (HICON) LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_BILL_NOTES), IMAGE_ICON, 16,16, 0);

		// (j.armen 2012-03-20 13:41) - PLID 48303 - More effecient way of setting the note column - do it async!
		m_pdlRecallList->GetColumn(RecallList::NotesIcon)->FieldName = _bstr_t(CSqlFragment(
			"CASE "
			"	WHEN HasNote = 1 THEN CONVERT(SQL_VARIANT, {CONST_INT}) "
			"	ELSE CONVERT(SQL_VARIANT, {CONST_STRING}) "
			"END", m_hNotes, "'BITMAP:FILE'").Flatten());

		//(a.wilson 2012-3-23) PLID 48472 - check if current user has permission to create recalls.
		if ((GetCurrentUserPermissions(bioRecallSystem) & (sptCreate | sptCreateWithPass))) {
			GetDlgItem(IDC_RECALL_CREATE)->EnableWindow(TRUE);
		} else {
			GetDlgItem(IDC_RECALL_CREATE)->EnableWindow(FALSE);
		}

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CRecallsNeedingAttentionDlg::UpdateFilter()
{
	UpdateData(TRUE);
	DoFilter();
}

void CRecallsNeedingAttentionDlg::UpdateDatalistSelectionUI(NXDATALIST2Lib::_DNxDataListPtr pList, short colID, short colName, CNxLabel& label, UINT labelID, const CArray<long, long>& arIDs)
{
	IRowSettingsPtr pRow = pList->GetCurSel();
	switch(arIDs.GetSize())
	{
		case 0:
			DlgCtrlMultiSelEnable(pList, labelID, label, false);
			if(!pRow || VarLong(pRow->GetValue(colID)) != eIDAll)
				pList->SetSelByColumn(colID, eIDAll);
			break;
		case 1:
			DlgCtrlMultiSelEnable(pList, labelID, label, false);
			if(!pRow || VarLong(pRow->GetValue(colID)) != arIDs[0])
				pList->SetSelByColumn(colID, arIDs[0]);
			break;
		default:
			DlgCtrlMultiSelEnable(pList, labelID, label, true);
			{
				bool bMore = false;
				CString strText;
				CString strTip;

				for(int i = 0; i < arIDs.GetCount(); i++)
				{
					pRow = pList->FindByColumn(colID, arIDs[i], NULL, VARIANT_FALSE);
					if(pRow)
					{
						CString name = AsString(pRow->GetValue(colName));
						strText += name;
						strTip += name;

						strText += ", ";
						strTip += "\r\n";
					}
					else
					{
						bMore = true;
					}
				}

				// (a.walling 2014-01-27 14:29) - PLID 59993 - Instead of throwing an exception, just show "(more)..."
				if (bMore) {
					strText += "(more)...";
					strTip += "(more)...";
				}

				strText.TrimRight(", ");
				strTip.TrimRight("\r\n");

				label.SetText(strText);
				label.SetToolTip(strTip);
			}
			break;
	}
}

void CRecallsNeedingAttentionDlg::DoFilter()
{
	//Dlg Components
	UpdateData(FALSE);

	// (a.walling 2014-01-27 14:29) - PLID 59993 - Instead of throwing an exception, just show "(more)..."
	// this can occur if something was added, inactivated, deleted, etc and does not display in the dialog.
	// Although we could spend more time making this more fool-proof, requerying etc, it is a relatively rare
	// occurrence, though one that is constantly throwing exceptions when hit.
	// Since it is only used for the display, I think we are safe to just handle it like this for now.
	UpdateDatalistSelectionUI(m_pdlProviderList, ProviderList::ID, ProviderList::Name, m_nxlProviders, IDC_NXDL_RECALL_PROVIDERS, m_aryProviderIDs);
	UpdateDatalistSelectionUI(m_pdlTemplateList, TemplateList::ID, TemplateList::Name, m_nxlTemplates, IDC_NXDL_RECALL_TEMPLATES, m_aryTemplateIDs);
	UpdateDatalistSelectionUI(m_pdlLocationList, LocationList::ID, LocationList::Name, m_nxlLocations, IDC_NXDL_RECALL_LOCATIONS, m_aryLocationIDs);
	UpdateDatalistSelectionUI(m_pdlStatusList, StatusList::ID, StatusList::Name, m_nxlStatus, IDC_NXDL_RECALL_STATUS, m_aryStatusIDs);

	// (a.walling 2013-12-12 16:51) - PLID 60007 - Update recalls now
	RecallUtils::UpdateRecalls(m_bUsePatientID ? GetActivePatientID() : -1);

	//Data Components
	CSqlFragment sqlFrom(/*SELECT * FROM */"({SQL}) SubQ", RecallUtils::SelectRecalls());

	CSqlFragment sqlWhere("1=1 ");
	
	// (j.armen 2012-02-28 15:01) - PLID 48452
	if(m_bUsePatientID)
	{
		sqlWhere += CSqlFragment("AND PatientID = {INT} ", GetActivePatientID());
		SetWindowText("Recalls Needing Attention - " + GetActivePatientName());
	}
	else
	{
		SetWindowText("Recalls Needing Attention");
	}

	// (j.armen 2012-02-28 15:58) - PLID 48460 - If we have selected locations to filter on, then filter
	if(m_aryProviderIDs.GetSize() > 0)
	{
		sqlWhere += CSqlFragment("AND ParentProviderID IN ({INTARRAY}) ", m_aryProviderIDs); 
	}

	// (j.armen 2012-02-28 16:29) - PLID 48462 - If we have selected recall templates to filter on, then check for them
	if(m_aryTemplateIDs.GetSize() > 0)
	{
		sqlWhere += CSqlFragment("AND ParentRecallTemplateID IN ({INTARRAY}) ", m_aryTemplateIDs);
	}

	// (j.armen 2012-03-01 10:18) - PLID 48483 - If we have selected locations to filter on, then filter
	if(m_aryLocationIDs.GetSize() > 0)
	{
		sqlWhere += CSqlFragment("AND ParentLocationID IN ({INTARRAY}) ", m_aryLocationIDs);
	}

	// (j.armen 2012-03-01 10:19) - PLID 48546 - If we have any statuses that we are filtering on, then check for them
	if(m_aryStatusIDs.GetSize() > 0)
	{
		sqlWhere += CSqlFragment("AND ParentRecallStatusID IN ({INTARRAY}) ", m_aryStatusIDs);
	}

	// (j.armen 2012-02-28 16:34) - PLID 48463 - If we have a valid start date, search for recall dates >= the date selected
	if(m_dtStart.GetStatus() == COleDateTime::valid)
	{
		sqlWhere += CSqlFragment("AND ParentRecallDate >= dbo.AsDateNoTime({OLEDATETIME}) ", m_dtStart);
	}

	// (j.armen 2012-02-28 16:34) - PLID 48463 - If we have a valid end date, search for recall dates <= the date selected
	if(m_dtEnd.GetStatus() == COleDateTime::valid)
	{
		sqlWhere += CSqlFragment("AND ParentRecallDate <= dbo.AsDateNoTime({OLEDATETIME}) ", m_dtEnd);
	}

	// (j.armen 2012-05-29 15:12) - PLID 48546 - If the filter has not changed, then no need to requery
	if(VarString(m_pdlRecallList->FromClause).CompareNoCase(sqlFrom.Flatten()) != 0
		|| VarString(m_pdlRecallList->WhereClause).CompareNoCase(sqlWhere.Flatten()) != 0)
	{
		m_pdlRecallList->PutFromClause(_bstr_t(sqlFrom.Flatten()));
		m_pdlRecallList->PutWhereClause(_bstr_t(sqlWhere.Flatten()));
		RequeryRecallList();
	}
}

void CRecallsNeedingAttentionDlg::RequeryRecallList()
{
	m_pdlRecallList->Requery();
	m_bNeedsRefresh = FALSE;
}

BOOL CRecallsNeedingAttentionDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	try
	{
		switch(wParam)
		{
			case IDC_RECALL_CURRENT_PAT:	// (j.armen 2012-02-28 15:00) - PLID 48452
				UpdateFilter();
				break;
			case IDC_RECALL_CREATE:
				{
					//(a.wilson 2012-3-26) PLID 48472 - check permissions before creating recall dialog.
					BOOL bCreatePerm = (GetCurrentUserPermissions(bioRecallSystem) & (sptCreate));
					BOOL bCreatePermWithPass = (GetCurrentUserPermissions(bioRecallSystem) & (sptCreateWithPass));

					if (!bCreatePerm && !bCreatePermWithPass) {
						PermissionsFailedMessageBox();
						break;
					} else if (!bCreatePerm && bCreatePermWithPass) {
						if (!CheckCurrentUserPassword()) {
							break;
						}
					}

					CCreatePatientRecall::PatientRecall Recall;
					Recall.nPatientID = GetActivePatientID();
					CCreatePatientRecall dlg(Recall, this);
					dlg.DoModal();
					// (a.walling 2013-12-12 16:51) - PLID 60007 - Update recalls now
					RecallUtils::UpdateRecalls(m_bUsePatientID ? GetActivePatientID() : -1);
					RequeryRecallList();
				}
				break;
		}
	}NxCatchAll(__FUNCTION__);
	return CNxDialog::OnCommand(wParam, lParam);
}

// (j.armen 2012-02-28 16:35) - PLID 48463 - Update the filter any time the date ranges are modified
void CRecallsNeedingAttentionDlg::OnDtnDatetimechangeRecallDaterange(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	try
	{
		UpdateFilter();
	}NxCatchAll(__FUNCTION__);
	*pResult = 0;
}

// (j.armen 2012-02-28 15:59) - PLID 48460 - when the user selects a provider, process the selection
void CRecallsNeedingAttentionDlg::SelChosenRecallProviders(LPDISPATCH lpRow)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);

		// If the user did not select a row, default to all
		if(!pRow)
			pRow = m_pdlProviderList->FindByColumn(ProviderList::ID, eIDAll, NULL, VARIANT_TRUE);

		// If we still don't have a row, then something went horribly wrong
		if(!pRow)
			AfxThrowNxException("Unable to select provider");

		switch(AsLong(pRow->GetValue(ProviderList::ID)))
		{
			case eIDAll:
				m_aryProviderIDs.RemoveAll();
				break;
			case eIDMultiple:
				ShowSelectMultiProvider();
				break;
			default:
				m_aryProviderIDs.RemoveAll();
				m_aryProviderIDs.Add(pRow->GetValue(ProviderList::ID));
				break;
		}

		DoFilter();

	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-02-28 16:01) - PLID 48462 - when the user selects a template, process the selection
void CRecallsNeedingAttentionDlg::SelChosenRecallTemplates(LPDISPATCH lpRow)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);

		// If the user did not select a row, default to all
		if(!pRow)
			pRow = m_pdlTemplateList->FindByColumn(TemplateList::ID, eIDAll, NULL, VARIANT_TRUE);

		// If we still don't have a row, then something went horribly wrong
		if(!pRow)
			AfxThrowNxException("Unable to select recall template");

		switch(VarLong(pRow->GetValue(TemplateList::ID), eIDAll))
		{
			case eIDAll:
				m_aryTemplateIDs.RemoveAll();
				break;
			case eIDMultiple:
				ShowSelectMultiTemplate();
				break;
			default:
				m_aryTemplateIDs.RemoveAll();
				m_aryTemplateIDs.Add(pRow->GetValue(TemplateList::ID));
				break;
		}

		DoFilter();

	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-03-01 10:20) - PLID 48483 - when the user selects a location, process the selection
void CRecallsNeedingAttentionDlg::SelChosenRecallLocations(LPDISPATCH lpRow)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);

		// If the user did not select a row, default to all
		if(!pRow)
			pRow = m_pdlLocationList->FindByColumn(LocationList::ID, eIDAll, NULL, VARIANT_TRUE);

		// If we still don't have a row, then something went horribly wrong
		if(!pRow)
			AfxThrowNxException("Unable to select location");

		switch(AsLong(pRow->GetValue(LocationList::ID)))
		{
			case eIDAll:
				m_aryLocationIDs.RemoveAll();
				break;
			case eIDMultiple:
				ShowSelectMultiLocation();
				break;
			default:
				m_aryLocationIDs.RemoveAll();
				m_aryLocationIDs.Add(pRow->GetValue(LocationList::ID));
				break;
		}

		DoFilter();

	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-03-01 10:20) - PLID 48546 - When the user selects a recall status, process the message
void CRecallsNeedingAttentionDlg::SelChosenRecallStatus(LPDISPATCH lpRow)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);

		// If the user did not select a row, default to all
		if(!pRow)
			pRow = m_pdlStatusList->FindByColumn(StatusList::ID, eIDAll, NULL, VARIANT_TRUE);

		// If we still don't have a row, then something went horribly wrong
		if(!pRow)
			AfxThrowNxException("Unable to select recall status");

		// Now that we have selected status, update the status array
		switch(VarLong(pRow->GetValue(StatusList::ID), eIDAll))
		{
			case eIDAll:
				m_aryStatusIDs.RemoveAll();
				break;
			case eIDMultiple:
				ShowSelectMultiStatus();
				break;
			default:
				m_aryStatusIDs.RemoveAll();
				m_aryStatusIDs.Add(pRow->GetValue(StatusList::ID));
				break;
		}

		DoFilter();

	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-02-28 15:59) - PLID 48460 - Show the multi select
void CRecallsNeedingAttentionDlg::ShowSelectMultiProvider()
{
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "ProvidersT");
	dlg.PreSelect(m_aryProviderIDs);
	if(IDOK == dlg.Open("(SELECT ID, [Last] + ', ' + [First] + ' ' + [Middle] AS Name, Archived FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID) SubQ", "Archived = 0", "ID", "Name", "Select Providers"))
	{
		dlg.FillArrayWithIDs(m_aryProviderIDs);
	}
}

// (j.armen 2012-02-28 16:29) - PLID 48462 - Show the multi select
void CRecallsNeedingAttentionDlg::ShowSelectMultiTemplate()
{
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "RecallTemplateT");
	dlg.PreSelect(m_aryTemplateIDs);
	if(IDOK == dlg.Open("RecallTemplateT", "Active = 1", "ID", "Name", "Select Recall Templates"))
	{
		dlg.FillArrayWithIDs(m_aryTemplateIDs);
	}
}

// (j.armen 2012-03-12 09:40) - PLID 48483 - Show the multi select
void CRecallsNeedingAttentionDlg::ShowSelectMultiLocation()
{
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "LocationsT");
	dlg.PreSelect(m_aryLocationIDs);
	if(IDOK == dlg.Open("LocationsT", "Active = 1 AND TypeID = 1", "ID", "Name", "Select Locations"))
	{
		dlg.FillArrayWithIDs(m_aryLocationIDs);
	}
}

// (j.armen 2012-03-09 17:54) - PLID 48546 - Show the multi select
void CRecallsNeedingAttentionDlg::ShowSelectMultiStatus()
{
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "RecallStatus");
	dlg.PreSelect(m_aryStatusIDs);
	CSqlFragment sql(
		"(SELECT {CONST_INT} AS ID, {CONST_STRING} AS Name\r\n"
		"UNION SELECT {CONST_INT} AS ID, {CONST_STRING} AS Name\r\n"
		"UNION SELECT {CONST_INT} AS ID, {CONST_STRING} AS Name\r\n"
		"UNION SELECT {CONST_INT} AS ID, {CONST_STRING} AS Name\r\n"
		"UNION SELECT {CONST_INT} AS ID, {CONST_STRING} AS Name)SubQ",
		RecallUtils::ePastDue, "'Past Due'",
		RecallUtils::eNeedToSchedule, "'Need to Schedule'",
		RecallUtils::eScheduled, "'Scheduled'",
		RecallUtils::eComplete, "'Complete'",
		RecallUtils::eDiscontinued, "'Discontinued'");

	if(IDOK == dlg.Open(sql.Flatten(), "", "ID", "Name", "Select Recall Statuses"))
	{
		dlg.FillArrayWithIDs(m_aryStatusIDs);
	}
}

LRESULT CRecallsNeedingAttentionDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try
	{
		switch(wParam) 
		{
			case IDC_NXL_RECALL_PROVIDERS:	// (j.armen 2012-02-28 16:00) - PLID 48460 - Handle the provider label click
				ShowSelectMultiProvider();
				break;
			case IDC_NXL_RECALL_TEMPLATES:	// (j.armen 2012-02-28 16:29) - PLID 48462 - Handle the template label click
				ShowSelectMultiTemplate();
				break;
			case IDC_NXL_RECALL_LOCATIONS:	// (j.armen 2012-03-01 10:51) - PLID 48483 - Handle the location label click
				ShowSelectMultiLocation();
				break;
			case IDC_NXL_RECALL_STATUS:		// (j.armen 2012-03-01 10:51) - PLID 48546 - Handle the status label click
				ShowSelectMultiStatus();
				break;
			default:
				ASSERT(FALSE);
				break;
		}

		DoFilter();
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}

void CRecallsNeedingAttentionDlg::DlgCtrlMultiSelEnable(NXDATALIST2Lib::_DNxDataListPtr& pdl2, UINT dl2ID, CNxLabel& nxl, bool bEnable)
{
	if(bEnable)
	{
		if(!nxl.IsWindowEnabled())
			nxl.EnableWindow(TRUE);
		if(!nxl.IsWindowVisible())
			nxl.ShowWindow(SW_SHOWNA);
		if(pdl2->Enabled) {
			pdl2->Enabled = VARIANT_FALSE;
			GetDlgItem(dl2ID)->ShowWindow(SW_HIDE);
		}
	}
	else
	{
		if(nxl.IsWindowEnabled())
			nxl.EnableWindow(FALSE);
		if(nxl.IsWindowVisible())
			nxl.ShowWindow(SW_HIDE);
		if(!pdl2->Enabled) {
			pdl2->Enabled = VARIANT_TRUE;
			GetDlgItem(dl2ID)->ShowWindow(SW_SHOW);
		}
	}
}

BOOL CRecallsNeedingAttentionDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		// (j.armen 2012-02-28 16:00) - PLID 48460 - Handle mouseover label changing cursor
		if(m_nxlProviders.IsWindowVisible() && m_nxlProviders.IsWindowEnabled())
		{
			m_nxlProviders.GetWindowRect(rc);
			ScreenToClient(&rc);
			if(rc.PtInRect(pt))
			{
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
		// (j.armen 2012-02-28 16:29) - PLID 48462 - Handle mouseover label changing cursor
		if(m_nxlTemplates.IsWindowVisible() && m_nxlTemplates.IsWindowEnabled())
		{
			m_nxlTemplates.GetWindowRect(rc);
			ScreenToClient(&rc);
			if(rc.PtInRect(pt))
			{
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
		// (j.armen 2012-02-28 16:29) - PLID 48483 - Handle mouseover label changing cursor
		if(m_nxlLocations.IsWindowVisible() && m_nxlLocations.IsWindowEnabled())
		{
			m_nxlLocations.GetWindowRect(rc);
			ScreenToClient(&rc);
			if(rc.PtInRect(pt))
			{
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
		// (j.armen 2012-02-28 16:29) - PLID 48546 - Handle mouseover label changing cursor
		if(m_nxlStatus.IsWindowVisible() && m_nxlStatus.IsWindowEnabled())
		{
			m_nxlStatus.GetWindowRect(rc);
			ScreenToClient(&rc);
			if(rc.PtInRect(pt))
			{
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	}NxCatchAll(__FUNCTION__);
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (j.armen 2012-02-28 15:28) - PLID 48457 - This will update the discontinued status
void CRecallsNeedingAttentionDlg::OnSwapDiscontinueStatus()
{
	try
	{
		//(a.wilson 2012-3-23) PLID 48472 - checking whether the current user has unlink permission.
		BOOL bDisResPerm = (GetCurrentUserPermissions(bioRecallSystem) & (sptDynamic0));
		BOOL bDisResPermWithPass = (GetCurrentUserPermissions(bioRecallSystem) & (sptDynamic0WithPass));

		if (!bDisResPerm && !bDisResPermWithPass) {
			PermissionsFailedMessageBox();
			return;
		} else if (!bDisResPerm && bDisResPermWithPass) {
			if (!CheckCurrentUserPassword()) {
				return;
			}
		}

		IRowSettingsPtr pRow = m_pdlRecallList->GetCurSel();
		if(pRow)
		{
			// (b.savon 2012-03-20 09:35) - PLID 48471 - Get the discontinued status and prepare auditing
			BOOL bDiscontinued = AsBool(pRow->GetValue(RecallList::Discontinued));
			CString strRecallTemplate = AsString(pRow->GetValue(RecallList::TemplateName));
			CString strOldValue = bDiscontinued ? "Recall: " + strRecallTemplate + " - Status: Discontinued" : "Recall: " + strRecallTemplate + " - Status: Active";
			CString strNewValue = bDiscontinued ? "Recall: " + strRecallTemplate + " - Status: Active" : "Recall: " + strRecallTemplate + " - Status: Discontinued";
			
			long nRecallID = pRow->GetValue(RecallList::RecallID);
			ExecuteParamSql("UPDATE RecallT SET Discontinued = (CASE WHEN Discontinued = 0 THEN 1 ELSE 0 END) WHERE ID = {INT}", nRecallID);

			// (a.walling 2013-12-12 16:51) - PLID 60007 - Update recalls now
			RecallUtils::UpdateRecalls(m_bUsePatientID ? GetActivePatientID() : -1);

			// (b.savon 2012-03-20 09:35) - PLID 48471 - Audit the status change
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(	AsLong(pRow->GetValue(RecallList::PatientID)),		//Patient ID
						AsString(pRow->GetValue(RecallList::PatientName)),	//Patient Name
						nAuditID,											//Audit ID
						aeiPatientRecallDiscontinued,						//GlobalAuditUtils Enum
						nRecallID,											//Recall ID
						strOldValue,										//If cur status is discontinued, reflect that
						strNewValue,										//New status is !curr status
						aepMedium,											//Priority
						aetChanged											//Changed Value
					  );

			RequeryRecallList();
		}
		else
		{
			// This should not be possible
			AfxThrowNxException("Unable to select recall to discontinue");
		}
	} NxCatchAll(__FUNCTION__);
}

void CRecallsNeedingAttentionDlg::OnLeftClickRecallList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{ 
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL)
			return;

		switch(nCol){
			case RecallList::PatientName:
				GotoPatient(pRow->GetValue(RecallList::PatientID));	// follow the hyperlink!
				break;
			case RecallList::NotesIcon:	
				// (j.armen 2012-03-19 09:05) - PLID 48780 - Handle the user clicking the NotesIcon
				CNotesDlg dlgRecallNotes(this);

				dlgRecallNotes.SetPersonID(VarLong(pRow->GetValue(RecallList::PatientID)));
				dlgRecallNotes.m_vtRecallID = pRow->GetValue(RecallList::RecallID);
				dlgRecallNotes.m_clrRecallNote = m_nxcRecallBkg.GetColor();

				dlgRecallNotes.m_strPrependText = FormatString("(%s - %s - Recall: %s)", 
					VarString(pRow->GetValue(RecallList::TemplateName)),
					VarString(pRow->GetValue(RecallList::StepName)),
					AsString(pRow->GetValue(RecallList::Date)));

				// (j.armen 2012-03-19 09:17) - PLID 48919 - Default Category for recall notes
				dlgRecallNotes.m_nCategoryIDOverride = GetRemotePropertyInt("RecallNotesDefaultCategory",-1,0,"<None>",true);

				CNxModalParentDlg dlg(this, &dlgRecallNotes, CString("Recall Notes"));
				dlg.DoModal();
	
				SetRowNoteIcon(pRow, ReturnsRecordsParam("SELECT 1 FROM Notes WHERE RecallID = {VT_I4}", pRow->GetValue(RecallList::RecallID)));
				break;
		}
	}NxCatchAll(__FUNCTION__);
}

//(a.wilson 2012-2-28) PLID 48418
void CRecallsNeedingAttentionDlg::OnRightClickRecallList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		if (m_pdlRecallList->GetReadOnly() == VARIANT_FALSE) {
			GetDlgItem(IDC_NXDL_RECALL_DISPLAY)->SetFocus();
			m_pdlRecallList->PutCurSel(NXDATALIST2Lib::IRowSettingsPtr(lpRow));
		}
	} NxCatchAll(__FUNCTION__);
}

//(a.wilson 2012-2-28) PLID 48418 - generate the context menu
void CRecallsNeedingAttentionDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	try {
		CWnd *pRecallWnd = GetDlgItem(IDC_NXDL_RECALL_DISPLAY);
		IRowSettingsPtr pRow = m_pdlRecallList->GetCurSel();

		if (pWnd->GetSafeHwnd() == pRecallWnd->GetSafeHwnd() 
				&& m_pdlRecallList->GetReadOnly() == VARIANT_FALSE && pRow != NULL) {
			
			CMenu mnu;
			mnu.LoadMenu(IDR_RECALL_POPUP);
			CMenu *pmnuSub= mnu.GetSubMenu(0);

			if (pmnuSub) {
				if (point.x == -1) {
					CRect rc;
					pWnd->GetWindowRect(&rc);
					GetCursorPos(&point);

					if (!rc.PtInRect(point)) {
						point.x = rc.left+5;
						point.y = rc.top+5;
					}
				}

				COleDateTime dtRecallAppointmentDate = AsDateTime(pRow->GetValue(RecallList::AppointmentDate));
				// (b.savon 2012-03-06 10:37) - PLID 48635 - Unlink Existing appointment from recall
				if( dtRecallAppointmentDate.GetStatus() == COleDateTime::invalid ||
					dtRecallAppointmentDate.GetStatus() == COleDateTime::null || 
					//(a.wilson 2012-3-23) PLID 48472 - permission to unlink
					(!(GetCurrentUserPermissions(bioRecallSystem) & (sptDynamic1 | sptDynamic1WithPass)))) {
					pmnuSub->RemoveMenu(ID_RECALLLISTPOPUP_UNLINKEXISTINGAPPOINTMENT, MF_BYCOMMAND);
				}

				// (j.armen 2012-02-28 15:52) - PLID 48457 - Show either the Discontinue or Resume menu item
				//(a.wilson 2012-3-23) PLID 48472 - permission to discontinue or resume.
				if (AsBool(pRow->GetValue(RecallList::Discontinued))) {
					pmnuSub->RemoveMenu(ID_RECALL_DISCONTINUE, MF_BYCOMMAND);
					pmnuSub->EnableMenuItem(ID_RECALL_SCHEDULER, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					pmnuSub->EnableMenuItem(ID_RECALL_APPOINTMENT, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					pmnuSub->EnableMenuItem(ID_RECALL_FFA, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					pmnuSub->EnableMenuItem(ID_LINK_TO_EXIST_APPT, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					if (!(GetCurrentUserPermissions(bioRecallSystem) & (sptDynamic0 | sptDynamic0WithPass))) {
						pmnuSub->EnableMenuItem(ID_RECALL_RESUME, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					}
				} else {
					pmnuSub->RemoveMenu(ID_RECALL_RESUME, MF_BYCOMMAND);
					if (!(GetCurrentUserPermissions(bioRecallSystem) & (sptDynamic0 | sptDynamic0WithPass))) {
						pmnuSub->EnableMenuItem(ID_RECALL_DISCONTINUE, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					}
				}
				//(a.wilson 2012-3-6) PLID 48418 - check for attached appointment which will 
				//show either go to scheduler or go to appointment
				if (VarLong(pRow->GetValue(RecallList::AppointmentID), -1) != -1) {
					pmnuSub->RemoveMenu(ID_RECALL_SCHEDULER, MF_BYCOMMAND);
					pmnuSub->EnableMenuItem(ID_RECALL_FFA, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
				} else {
					pmnuSub->RemoveMenu(ID_RECALL_APPOINTMENT, MF_BYCOMMAND);
				}

				pmnuSub->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, this, NULL);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-03-01 10:10) - PLID 48486 - Link existing recall to existing appointment
void CRecallsNeedingAttentionDlg::OnLinkExistingAppointment()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlRecallList->GetCurSel();

		if(pRow){
			long nRecallID = AsLong(pRow->GetValue(RecallList::RecallID));
			CString strPatientName = AsString(pRow->GetValue(RecallList::PatientName));
			long nPatientID = AsLong(pRow->GetValue(RecallList::PatientID));
			CString strTemplateName = AsString(pRow->GetValue(RecallList::TemplateName));

			CLinkRecallToExistingAppointmentDlg linkDlg(nPatientID, strPatientName, nRecallID, strTemplateName, this);
			if( linkDlg.DoModal() == IDOK ){
				// (a.walling 2013-12-12 16:51) - PLID 60007 - Update recalls now
				RecallUtils::UpdateRecalls(m_bUsePatientID ? GetActivePatientID() : -1);
				RequeryRecallList();
			}
		}

	}NxCatchAll(__FUNCTION__);
}

//(a.wilson 2012-2-28) PLID 48418 - this will send them to the scheduler view for the specific recall date or
//if no recall was right clicked then just to the scheduler.
void CRecallsNeedingAttentionDlg::OnScheduler()
{
	try {
		COleDateTime dtRecallDate;
		IRowSettingsPtr pRow = m_pdlRecallList->GetCurSel();
		//check to see if we have an item for a date.
		if (pRow != NULL) {
			_variant_t vtRecallDate = pRow->GetValue(RecallList::Date);

			if (vtRecallDate.vt == VT_DATE) {
				dtRecallDate = VarDateTime(vtRecallDate);
			} else {
				ThrowNxException("CRecallsNeedingAttentionDlg::OnScheduler() - Invalid value for the Recall Date.");
			}

			if (GoToMonthTab(dtRecallDate)) {
				Minimize();
			}
		}
	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-2-28) PLID 48418 - this will generate a FFA dialog filled with any information we can provide
//or will just generate the FFA is no item was right clicked.
void CRecallsNeedingAttentionDlg::OnFFA()
{
	try {
		CMainFrame  *pMainFrame = GetMainFrame();
		IRowSettingsPtr pRow = m_pdlRecallList->GetCurSel();

		if (pMainFrame != NULL) {
			COleDateTime dtRecallDate;
			long nRecallID = -1;

			if (pRow != NULL) {
				_variant_t vtRecallDate = pRow->GetValue(RecallList::Date);

				if (vtRecallDate.vt == VT_DATE) {
					dtRecallDate = VarDateTime(vtRecallDate);
				} else {
					ThrowNxException("CRecallsNeedingAttentionDlg::OnFFA() - Invalid value for the Recall Date.");
				}
				// (v.maida 2016-01-25 16:22) - PLID 65645 - When opening up FFA, don't jump to the scheduler any longer.
				//if (GoToMonthTab(dtRecallDate)) {
				Minimize();
				long nPatientID = VarLong(pRow->GetValue(RecallList::PatientID));

				if (nPatientID > 0) {
					//need to set preselects to be the current rows information to alleviate any extra work on the user.
					//(a.wilson 2012-3-28) PLID 49245 - we only want to override the patient and the startdate
					//otherwise keep the rest of the defaults.
					pMainFrame->m_FirstAvailAppt.m_bDefaultOverrides.SetAllTo(false);
					pMainFrame->m_FirstAvailAppt.m_bDefaultOverrides.bPatient = true;
					pMainFrame->m_FirstAvailAppt.m_bDefaultOverrides.bStartDate = true;
					pMainFrame->m_FirstAvailAppt.m_nPreselectPatientID = nPatientID;
					pMainFrame->m_FirstAvailAppt.m_bPreselectStartDateRadio = TRUE;
					if (dtRecallDate >= COleDateTime::GetCurrentTime()) { 												
						pMainFrame->m_FirstAvailAppt.m_dtPreselectStart = dtRecallDate;
					}
				}
				pMainFrame->m_FirstAvailAppt.DoModal();
				//}
			}
		}
	} NxCatchAll(__FUNCTION__);
}

void CRecallsNeedingAttentionDlg::GotoPatient(long nPatID)
{
	try {
		if (nPatID != -1) {
			CMainFrame *pMainFrame = GetMainFrame();
			if (pMainFrame != NULL) {
				if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatID)) {
					if(IDNO == MessageBox("This patient is not in the current lookup. \n"
						"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
						return;
					}
				}

				if(!pMainFrame->m_patToolBar.TrySetActivePatientID(nPatID)) {
					return;
				}

				pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
				CNxTabView *pView = pMainFrame->GetActiveView();
				if(pView)
					pView->UpdateView();
				Minimize();
			}
			else {
				MsgBox(MB_ICONSTOP|MB_OK, "ERROR - Cannot Open Mainframe");
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CRecallsNeedingAttentionDlg::Minimize() {
	try {
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		if (GetWindowPlacement(&wp)) {
			if (!IsIconic()) {
				wp.showCmd = SW_MINIMIZE;
				SetWindowPlacement(&wp);
				GetMainFrame()->SetForegroundWindow();
			}
		}
	}NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-2-28) PLID 48418 - this takes makes the active sheet the month tab in the scheduler module, 
//otherwise if the month is not already constructed then the FFA will throw errors.
bool CRecallsNeedingAttentionDlg::GoToMonthTab(const COleDateTime& dtDate)
{
	CMainFrame *pMainFrame = GetMainFrame();
	if (pMainFrame != NULL) {
		if (pMainFrame->FlipToModule(SCHEDULER_MODULE_NAME)) {
			CNxTabView *pView = pMainFrame->GetActiveView();
			if (pView && pView->IsKindOf(RUNTIME_CLASS(CSchedulerView))) {
				((CSchedulerView *)pView)->SetActiveTab(SchedulerModule::MonthTab);
				((CNxSchedulerDlg *)pView->GetActiveSheet())->SetActiveDate(dtDate);
				((CNxSchedulerDlg *)pView->GetActiveSheet())->OnSetDateSelCombo();
				return true;
			}
		}
	}
	return false;
}

// (b.savon 2012-03-02 16:41) - PLID 48474 - Add ability to Merge To Word from Recall Dlg
void CRecallsNeedingAttentionDlg::OnBnClickedBtnMergeToWord()
{
	try{
		Merge();

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-03-02 16:41) - PLID 48474 - Add ability to Merge To Word from Recall Dlg
void CRecallsNeedingAttentionDlg::Merge()
{
	try{
		
		//Run the gauntlet
		if( m_pdlRecallList->GetRowCount() <= 0 ){
			MessageBox("Please adjust your filters so that there are recalls in the list.", "Practice", MB_ICONINFORMATION);
			return;
		}

		if( !GetWPManager()->CheckWordProcessorInstalled() ){
			MessageBox("Please make sure Microsoft Word is installed.", "Practice", MB_ICONINFORMATION);
			return;
		}

		if( IDNO == MessageBox("A new Word document will be generated for each recall.  This will cause the merge to take a long time if you are "
			"merging a lot of recalls.\n\nAre you sure you wish to merge these recalls?", "Practice", MB_ICONEXCLAMATION | MB_YESNO )){

			return;
		}

		BOOL bPauseEvery10 = TRUE;

		if(m_pdlRecallList->GetRowCount() > 10) {
			if(IDNO == MessageBox("You are merging a large number of Word documents - it is recommended that the merge pauses every 10 records\n"
								  "so you will have the option to print and close the documents before continuing.\n\n"
								  "Would you like to pause the merge every 10 records? (Clicking 'No' will merge all the records without stopping.)",
								  "Practice",MB_ICONQUESTION|MB_YESNO)) {
				bPauseEvery10 = FALSE;
			}
		}

		//We made it through the gauntlet, now let's get down to business
		CString strFilter;
		strFilter = "Microsoft Word Templates|*.dot;*.dotx;*.dotm||";
		
		// Let's ask them to pick a document.
		// Later, we may tie merge documents to recall templates
		// (a.walling 2012-04-27 15:23) - PLID 46648 - Dialogs must set a parent!
		CFileDialog fDlg(TRUE, "dot", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter, this);
		CString strInitialDir = GetTemplatePath();
		fDlg.m_ofn.lpstrInitialDir = strInitialDir;

		if( fDlg.DoModal() == IDOK ){
			CWaitCursor wc;
			CString strTemplateName = fDlg.GetPathName();
			CMap<long, long, bool, bool> mapPatIDs;  // (r.farnworth 2014-09-12 12:40) - PLID 62792

			//Go through out filtered list and merge the letters.
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlRecallList->GetFirstRow();
			for(int i = 0; i < m_pdlRecallList->GetRowCount(); i++){
				if( pRow == NULL ){
					return;
				}

				if( bPauseEvery10 && (i%10 == 0) && i != 0 ){
					CString str;
					long nNumLeft = 10;
					if( m_pdlRecallList->GetRowCount() - i <= 10 ){
						nNumLeft = m_pdlRecallList->GetRowCount() - i;
						if( nNumLeft == 1){
							str.Format("Press OK to merge the last record.");
						}else{
							str.Format("Press OK to merge the last %li records.", nNumLeft);
						}
					}else{
						str.Format("Press OK to merge the next %li records.", nNumLeft);
					}//END if( m_pdlRecallList->GetRowCount() - i <= 10 ){

					MessageBox(str);
				}//END if( bPauseEvery10 && (i%10 == 0) && i != 0 ){


				CSqlFragment sqlTemp = CSqlFragment("SELECT ID FROM PersonT WHERE ID = {INT}", AsLong(pRow->GetValue(RecallList::PatientID)));
				CString strMergeT = CreateTempIDTable(sqlTemp.Flatten(), "ID");

				CMergeEngine mi;

				if( g_bMergeAllFields ){
					mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;
				}

				mi.m_nFlags |= BMS_SAVE_FILE_AND_HISTORY;

				mi.LoadSenderInfo(FALSE);

				// (b.savon 2012-03-19 10:12) - PLID 48966 - Merge directly to printer
				if( IsDlgButtonChecked(IDC_CHK_MERGE_TO_PRINTER) ){
					mi.m_nFlags = (mi.m_nFlags | BMS_MERGETO_PRINTER) & ~BMS_MERGETO_SCREEN;
				}

				// (z.manning 2016-06-03 8:41) - NX-100806 - Check if the merge was successful
				if (!mi.MergeToWord(strTemplateName, std::vector<CString>(), strMergeT)) {
					// (z.manning 2016-06-13 8:25) - NX-100806 - Break out of the loop so that we can still process
					// any patients that have already merged successfully but do not continue trying to merge.
					break;
				}

				// (r.farnworth 2014-09-12 12:40) - PLID 62792 - When user merge documnet on Letterwriting or Recall, Based on "Sent Reminder" option, insert record to PatientRemindersSetT
				if (m_btnAddPatientReminder.GetCheck() == BST_CHECKED){
					bool matchFound = false;
					if (!mapPatIDs.Lookup(AsLong(pRow->GetValue(RecallList::PatientID)), matchFound))
					{
						mapPatIDs[AsLong(pRow->GetValue(RecallList::PatientID))] = true;
					}
				}

				//Tracer? TODO
				/*if( tracer ){
					INSERT INTO RECALLHISTORYT
					INSERT INTO RECALLHISTORYDETAILST
				}*/

				pRow = pRow->GetNextRow();
			}//END ALL ROWS

			// (r.farnworth 2014-09-15 10:19) - PLID 62792 - When user merge documnet on Letterwriting or Recall, Based on "Sent Reminder" option, insert record to PatientRemindersSetT
			CString sqlToExecute;

			if (!mapPatIDs.IsEmpty())
			{
				POSITION pos = mapPatIDs.GetStartPosition();
				while (pos != NULL)
				{
					long nPatID;
					bool bFound;
					mapPatIDs.GetNextAssoc(pos, nPatID, bFound);

					CString currentSQL;
					// (s.tullis 2015-10-01 09:23) - PLID 66442 - Added Default Delete flag
					currentSQL.Format(" INSERT INTO PatientRemindersSentT (PatientID, UserID, ReminderMethod, ReminderDate) "
						" VALUES (%li, %li, %li, GETDATE());\r\n ", nPatID, GetCurrentUserID(), srhRecalls);
					sqlToExecute += currentSQL;
				}

				ExecuteSql(sqlToExecute);
			}

		}//END DoModal
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-03-06 10:40) - PLID 48635 - Unlink existing appointment
void CRecallsNeedingAttentionDlg::OnUnlinkExistingAppointment()
{
	try {
		//(a.wilson 2012-3-23) PLID 48472 - checking whether the current user has unlink permission.
		BOOL bUnlinkPerm = (GetCurrentUserPermissions(bioRecallSystem) & (sptDynamic1));
		BOOL bUnlinkPermWithPass = (GetCurrentUserPermissions(bioRecallSystem) & (sptDynamic1WithPass));

		if (!bUnlinkPerm && !bUnlinkPermWithPass) {
			PermissionsFailedMessageBox();
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlRecallList->GetCurSel();

		if(pRow){
			long nRecallID = AsLong(pRow->GetValue(RecallList::RecallID));
			CString strPatientName = AsString(pRow->GetValue(RecallList::PatientName));
			long nPatientID = AsLong(pRow->GetValue(RecallList::PatientID));
			CString strDate = FormatDateTimeForInterface(AsDateTime(pRow->GetValue(RecallList::AppointmentDate)));
			CString strPurpose = AsString(pRow->GetValue(RecallList::AppointmentPurpose));
			CString strTemplate = AsString(pRow->GetValue(RecallList::TemplateName));

			CString strMessage;
			strMessage.Format("Are you sure you would like to unlink the following appointment:\n\n"
							  "%s - %s\n\n"
							  "for %s (%li) that is linked to the '%s' recall template?", 
							  strDate, strPurpose, 
							  strPatientName, ::GetExistingPatientUserDefinedID(nPatientID), strTemplate);

			if( IDYES == MessageBox(strMessage, "Practice - Unlink Recall Appointment", MB_ICONQUESTION|MB_YESNO) ){
				//(a.wilson 2012-3-23) PLID 48472 - checking whether they need to enter password or not.
				if (!bUnlinkPerm && bUnlinkPermWithPass) {
					if (!CheckCurrentUserPassword()) {
						return;
					}
				}
				CSqlFragment sqlUpdate = CSqlFragment("UPDATE RecallT SET RecallAppointmentID = {VT_I4} WHERE ID = {INT} \r\n", g_cvarNull, nRecallID);
				ExecuteParamSql(sqlUpdate);

				// (a.walling 2013-12-12 16:51) - PLID 60007 - Update recalls now
				RecallUtils::UpdateRecalls(m_bUsePatientID ? GetActivePatientID() : -1);

				// (b.savon 2012-03-20 11:40) - PLID 48471 - Audit Unlinked Appointments
				CString strOldValue = "Recall: " + strTemplate + " - Appointment: " + strDate + " - Purpose: " + strPurpose;
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(nPatientID, strPatientName, nAuditID, aeiPatientRecallLinkedAppointment, nRecallID, strOldValue, "", aepMedium, aetChanged);

				RequeryRecallList();
			}
		}
	}NxCatchAll(__FUNCTION__);
}

//(a.wilson 2012-3-6) PLID 48418 - when a recall has an attached appointment we want the ability to jump to it.
void CRecallsNeedingAttentionDlg::OnAppointment() 
{
	try {
		CMainFrame  *pMainFrame = GetMainFrame();
		IRowSettingsPtr pRow = m_pdlRecallList->GetCurSel();

		if (pMainFrame != NULL && pRow != NULL) {
			if (pMainFrame->FlipToModule(SCHEDULER_MODULE_NAME)) {
				CNxTabView *pView = pMainFrame->GetActiveView();
				if (pView && pView->IsKindOf(RUNTIME_CLASS(CSchedulerView))) {
					Minimize();
					((CSchedulerView *)pView)->OpenAppointment(VarLong(pRow->GetValue(RecallList::AppointmentID)), false);
				}
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-03-08 14:31) - PLID 48716 - Remember column widths
// Code based off of CEMRSearch::RestoreColumns()
void CRecallsNeedingAttentionDlg::RestoreColumnWidths()
{

	CString strColumnWidths = GetRemotePropertyText("RecallsNeedingAttentionColumnWidths", "", 0, GetCurrentUserName(), false);

	CArray<int, int> arWidths;

	int tokIndex = strColumnWidths.Find(',');

	// It is empty or invalid, so rebuild
	if (tokIndex == -1) {
		SaveColumnWidths();
		return;
	}

	int nColIndex = 0;
	while(tokIndex != -1) {
		CString str = strColumnWidths.Left(tokIndex);
		arWidths.Add(atoi(str));
		strColumnWidths = strColumnWidths.Right(strColumnWidths.GetLength() - (tokIndex + 1));
		tokIndex = strColumnWidths.Find(',');

		nColIndex++;
	}
	arWidths.Add(atoi(strColumnWidths));

	if (arWidths.GetSize() != m_pdlRecallList->ColumnCount) {
		SaveColumnWidths();
		return;
	}

	int nDataListWidth = 0;
	for (int i = 0; i < m_pdlRecallList->ColumnCount; i++)
	{
		// Now go through the columns, clear any that are set as Auto or Data, and set their saved width.
		IColumnSettingsPtr pCol = m_pdlRecallList->GetColumn(i);
		long nStyle = pCol->ColumnStyle;
		nStyle = (nStyle&(~csWidthAuto)&(~csWidthData));
		pCol->ColumnStyle = nStyle;
		pCol->StoredWidth = arWidths[i];
		nDataListWidth += arWidths[i];
	}
}

// (b.savon 2012-03-08 14:32) - PLID 48716 - Remember column widths
// Code based off of CEMRSearch::SaveColumns()
void CRecallsNeedingAttentionDlg::SaveColumnWidths()
{
	CString strColumnWidths;

	if (m_btnRememberColumns.GetCheck() != BST_CHECKED){ 
		SetRemotePropertyText("RecallsNeedingAttentionColumnWidths", "", 0, GetCurrentUserName());
		return;
	}

	try{
		// Store the columns in a xx,xx,xx,xx format
		for (int i = 0; i < m_pdlRecallList->ColumnCount; i++)
		{
			IColumnSettingsPtr pCol = m_pdlRecallList->GetColumn(i);
			CString str;		
			
			str.Format("%d", pCol->StoredWidth);
			
			if (i > 0)
				strColumnWidths += ",";

			strColumnWidths += str;
		}

		SetRemotePropertyText("RecallsNeedingAttentionColumnWidths", strColumnWidths, 0, GetCurrentUserName());
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-03-19 09:46) - PLID 48716 - Remember column widths
void CRecallsNeedingAttentionDlg::OnBnClickedOk()
{
	try{
		SaveColumnWidths();
	}NxCatchAll(__FUNCTION__);

	CNxDialog::OnOK();
}

// (b.savon 2012-03-19 09:46) - PLID 48716 - Remember column widths
void CRecallsNeedingAttentionDlg::OnClose()
{
	try{
		SaveColumnWidths();
	}NxCatchAll(__FUNCTION__);

	CNxDialog::OnClose();
}

// (b.savon 2012-03-09 12:09) - PLID 48763 - Create Merge Group
//Code Based on LetterWriting Create Merge Group
void CRecallsNeedingAttentionDlg::OnBnClickedBtnCreatemergegroup()
{
	//Run the Gauntlet
	// Check permission
	if (!CheckCurrentUserPermissions(bioLWGroup, sptWrite)) {
		MessageBox("You do not have permissions to create merge groups.  Please see your office manager for assistance.", "Practice", MB_ICONINFORMATION);
		return;
	}

	if(m_pdlRecallList->GetRowCount() == 0) {
		MessageBox("Please adjust your filters so that there are recalls in the list.", "Practice", MB_ICONINFORMATION);
		return;
	}

	// This is a new group
	CString strGroupName = "Untitled Group";
	if (Prompt("Enter the name for this group:", strGroupName, 50) == IDOK) {

		strGroupName.TrimRight();

		if(strGroupName == "") {
			AfxMessageBox("Please enter a non-blank name for this group.");
			return;
		}

		if(strGroupName == "{Current Patient}" || strGroupName == "{Current Filter}" || strGroupName == "New Group...") {
			AfxMessageBox("You cannot make a group with the same name as a system group.\n"
				"Please enter a different name.");
			return;
		}

		try {
			//SqlTransaction
			CSqlTransaction trans("CRecallsNeedingAttentionDlg::DoSaveCurrentGroup");
			trans.Begin();

			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ID FROM GroupsT WHERE Name = {STRING}", strGroupName);
			if(!rs->eof) {
				rs->Close();
				//Rollback is implicit. But we must rollback before messagebox!
				trans.Rollback();
				AfxMessageBox("That group name already exists, please choose a new name.");
				return;
			}else{
				rs->Close();
			}

			//We made it through the gauntlet, let's get down to business
			// Create the new group
			_RecordsetPtr prs = CreateParamRecordset(GetRemoteData(),
								"SET NOCOUNT ON \r\n"
								"DECLARE @NewNumber INT "
								"SET @NewNumber = (SELECT COALESCE(MAX(ID), 0) + 1 FROM GroupsT) \r\n"
								"INSERT INTO GroupsT ( ID, Name ) VALUES (@NewNumber, {STRING})"
								"SET NOCOUNT OFF \r\n"
								"SELECT @NewNumber AS GroupID \r\n"
								, strGroupName);

			long nNewGroupId = -1;
			if( !prs->eof ){
				nNewGroupId = AdoFldLong(prs->Fields, "GroupID", -1);
			}
			prs->Close();

			//We need a valid group id
			if( nNewGroupId == -1 ){
				trans.Rollback();
				MessageBox("Unable to create new merge group.  Please try again.", "Practice", MB_ICONERROR);
				return;
			}

			//Run through our list of recalls, selecting only distinct patients for the group.
			//We do this by keeping a map of our patients and only creating sql fragments for
			//those not already in the list.
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlRecallList->GetFirstRow();
			CMap<long, long, BOOL, BOOL> mapMergeGroup;
			BOOL bInGroup;
			CSqlFragment sqlQuery;
			while( pRow ){
				//Get patient ID
				long nPatientID = AsLong(pRow->GetValue(RecallList::PatientID));
				//Lookup
				if( !mapMergeGroup.Lookup(nPatientID,bInGroup) ){ 
					mapMergeGroup.SetAt(nPatientID, TRUE);

					sqlQuery += CSqlFragment("INSERT INTO GroupDetailsT (GroupID, PersonID) VALUES({INT}, {INT})", 
												nNewGroupId, nPatientID);
				}
				//Next please
				pRow = pRow->GetNextRow();
			}//While rows in datalist

			//Only execute if we have a query.
			if( !sqlQuery.IsEmpty() ){
				ExecuteParamSql(sqlQuery);
			}

			//auditing
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiGroupCreated, -1, "", strGroupName, aepMedium, aetChanged);

			trans.Commit();

			//Let the user know we were successful.
			MessageBox("Group: " + strGroupName + " created successfully!", "Practice", MB_ICONINFORMATION);

		} NxCatchAll("CRecallsNeedingAttentionDlg::DoSaveCurrentGroup");
	}//END if( entered a new merge group name ) 
}//END CRecallsNeedingAttentionDlg::OnBnClickedBtnCreatemergegroup()

// (j.armen 2012-03-19 09:06) - PLID 48780 - Function to set a row's note value
void CRecallsNeedingAttentionDlg::SetRowNoteIcon(IRowSettingsPtr& pRow, BOOL bHasNotes)
{
	if(bHasNotes)
		pRow->PutValue(RecallList::NotesIcon, (long)m_hNotes);
	else
		pRow->PutValue(RecallList::NotesIcon, (LPCTSTR)"BITMAP:FILE");
}

// (b.savon 2014-09-02 13:17) - PLID 62791 - Add patient reminder
void CRecallsNeedingAttentionDlg::OnBnClickedAddPtReminderRecall()
{
	try{
		if (m_btnAddPatientReminder.GetCheck() == BST_CHECKED){
			DontShowMe(this, "RecallPatientReminder");
		}
	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2015-11-05 13:44) - PLID 57109
void CRecallsNeedingAttentionDlg::SetNeedsRefresh()
{
	if (IsWindowVisible()) {
		// (z.manning 2015-11-05 13:45) - PLID 57109 - This window is visible so refresh right away
		RequeryRecallList();
	}
	else {
		// (z.manning 2015-11-05 13:45) - PLID 57109 - The window is not currently visible so flag
		// it to refresh next time it is shown.
		m_bNeedsRefresh = TRUE;
	}
}

// (z.manning 2015-11-05 13:47) - PLID 57109
void CRecallsNeedingAttentionDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	try
	{
		__super::OnShowWindow(bShow, nStatus);

		// (z.manning 2015-11-05 13:48) - PLID 57109 - If showing the window, check and see if we 
		// need to refresh.
		if (bShow)
		{
			if (m_bNeedsRefresh) {
				RequeryRecallList();
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}
