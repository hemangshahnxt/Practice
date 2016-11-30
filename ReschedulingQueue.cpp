// ReschedulingQueue.cpp : implementation file
//

// (a.walling 2014-12-22 14:24) - PLID 64370 - CReschedulingQueue dialog

#include "stdafx.h"
#include "Practice.h"
#include "ReschedulingQueue.h"
#include "RescheduleAppointmentsDlg.h"
#include "SchedulerRc.h"
#include "PracticeRc.h"
#include "MultiSelectDlg.h"
#include "ReschedulingUtils.h"

#include <NxUILib/DatalistUtils.h>
#include <NxSystemUtilitiesLib/NxOleDataSource.h>

#include "NotesDlg.h"			// (j.gruber 2014-12-15 11:57) - PLID 64419 - Rescheduling Queue - add notes icon
#include "NxModalParentDlg.h"  // (j.gruber 2014-12-15 11:57) - PLID 64419 - Rescheduling Queue - add notes icon
#include "GlobalSchedUtils.h"

#include "pugixml/pugixml.hpp"
#include "FirstAvailableAppt.h"


#define ID_REMOVE_FROM_QUEUE 64395	// (b.spivey, January 12, 2015) PLID 64395 - 
#define ID_GO_TO_APPOINTMENT	64396	// (b.spivey, January 15, 2015) PLID 64396
#define ID_RESTORE_ORIGINAL_APPOINTMENT	64394 // (b.spivey, January 16, 2015) PLID 64394
#define	ID_GO_TO_PATIENT 64608	// (b.spivey, January 16, 2015) PLID  64608
#define ID_OPEN_RESOURCE_IN_DAY_VIEW 64687 // (a.walling 2015-01-26 14:46) - PLID 64687
#define ID_REFRESH_EVENTUALLY 42
#define ID_GO_TO_FINDFIRSTAVAILABEL 64689 //(s.dhole 6/4/2015 2:21 PM ) - PLID 65638

using namespace NXDATALIST2Lib;
using namespace ADODB;

// CReschedulingQueue dialog

namespace {
	enum EAppointmentListColumns {
		ealcPatientID = 0,
		ealcAppointmentID, 
		ealcCancelReasonID, 
		ealcIsCancelled,
		ealcRescheduleNotes, 
		ealcNoShow,
		ealcApptTime,
		ealcPatientName,
		ealcResource,
		ealcPurpose,
		ealcLocation, 
		ealcAppointmentNotes,
		ealcHomePhone,
		ealcCellPhone,
		ealcWorkPhone, 
		ealcCancellationReason, 
	};

	enum EFilterDatalistColumns {
		efdcID = 0,
		efdcName,
	};

	enum EFilterDatalistSentinels {
		efdsNoShow = -3,
		efdsCancel = -2,
		efdsMultiple = -2,
		efdsAll = -1,

	};
}

IMPLEMENT_DYNAMIC(CReschedulingQueue, CNxDialog)

CReschedulingQueue::CReschedulingQueue(CWnd* pParent /*=NULL*/)
	: CNxDialog(CReschedulingQueue::IDD, pParent, "CReschedulingQueue")
{
	// (b.spivey, January 8th, 2015) PLID 64390 
	m_hIconNotes = (HICON)LoadImage(AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(IDI_TOPIC_OPEN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	m_hIconHasNotes = (HICON)LoadImage(AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(IDI_BILL_NOTES), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
}

// (b.spivey, February 6th 2015) - PLID 64390 - prevent a resource leak. 
CReschedulingQueue::~CReschedulingQueue()
{
	DestroyIcon((HICON)m_hIconNotes);
	DestroyIcon((HICON)m_hIconHasNotes);
}

void CReschedulingQueue::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_REMEMBER_COL_RESCHED_LIST, m_checkRememberColumnWidths);
	DDX_Control(pDX, IDC_RESOURCE_LIST_STATIC, m_nxstaticMultiResourceLabel);
	DDX_Control(pDX, IDC_LOCATION_LIST_STATIC, m_nxstaticMultiLocationLabel);
	DDX_Control(pDX, IDC_APPOINTMENTS_SHOWN_STATIC, m_nxstaticAppointmentsShowLabel); 
}

static bool g_bReschedulingQueueShouldDock = true;

bool CReschedulingQueue::ShouldDock()
{
	// (a.walling 2015-01-12 16:42) - PLID 64570 - Dock or pop out
	return g_bReschedulingQueueShouldDock;
}

bool CReschedulingQueue::IsDocked() const
{
	if (auto* pParent = GetParent()) {		
		if (pParent->GetStyle() & WS_CHILD) {
			return true;
		}
	}

	return false;
}

BEGIN_MESSAGE_MAP(CReschedulingQueue, CNxDialog)
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDCANCEL, &CReschedulingQueue::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_RESCHEDULE_APPOINTMENTS, &CReschedulingQueue::OnBnClickedRescheduleAppointments)
	ON_BN_CLICKED(IDC_CHECK_REMEMBER_COL_RESCHED_LIST, &CReschedulingQueue::OnBnClickedRememberColumnWidths)
	ON_BN_CLICKED(IDC_TOGGLE_DOCK, &CReschedulingQueue::OnBnClickedToggleDock)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_MESSAGE(WM_TABLE_CHANGED_EX, OnTableChangedEx)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CReschedulingQueue message handlers

BOOL CReschedulingQueue::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		SetIcon(AfxGetApp()->LoadIcon(MAKEINTRESOURCE(IDI_RESCHEDULINGQ)), FALSE);

		// (b.spivey, January 6th, 2015) PLID 64389 - caching
		g_propManager.CachePropertiesInBulk("RescheduleQueueDlg1", propText,
			"("
			"("
			"Username = '<None>' OR Username = '%s') AND ("
			"Name = 'ReschedulingQueueAppointmentListColumnWidths' "
			" OR Name = 'ReschedulingQueueResourceList' " 
			")"
			")",
			_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("RescheduleQueueDlg2", propNumber,
			"("
			"("
			"Username = '<None>' OR Username = '%s') AND ("
			"Name = 'ReschedulingQueueRememberColumnWidths' "
			")"
			")",
			_Q(GetCurrentUserName()));

		// (b.spivey, January 28, 2015) PLID 64389 - remember per user. 
		m_bRememberColumns = !!GetRemotePropertyInt("ReschedulingQueueRememberColumnWidths", 1, 0, GetCurrentUserName());
		m_checkRememberColumnWidths.SetCheck(m_bRememberColumns); 

		// (b.spivey, January 7th, 2015) PLID 64397 - Label for listing multiple resources.
		m_nxstaticMultiResourceLabel.SetType(dtsHyperlink);
		m_nxstaticMultiResourceLabel.SetSingleLine();
		// (b.spivey, January 8th, 2015) PLID 64398 - Locations label. 
		m_nxstaticMultiLocationLabel.SetType(dtsHyperlink);
		m_nxstaticMultiLocationLabel.SetSingleLine();

		m_pListResource = BindNxDataList2Ctrl(IDC_LIST_RESOURCE, false);

		m_pListLocation = BindNxDataList2Ctrl(IDC_LIST_LOCATIONS, false);
		m_pListReason = BindNxDataList2Ctrl(IDC_LIST_REASON, false);
		m_pListType = BindNxDataList2Ctrl(IDC_LIST_TYPE, false);
		m_pListPurpose = BindNxDataList2Ctrl(IDC_LIST_PURPOSE, false);

		m_pList = BindNxDataList2Ctrl(IDC_LIST_APPOINTMENTS, false);


		{
			using namespace NXDATALIST2Lib;

			// (b.spivey, January 7th, 2015) PLID 64397 - Resource list
			AppendColumns(m_pListResource, {
				{ "ID", "ID", 0, csFixedWidth },
				{ "Name", "Name", 0, csWidthAuto, cftTextSingleLine, 0 },
			});

			m_pListResource->FromClause = R"(	
				(
					SELECT ID, Item AS Name FROM ResourceT WHERE Inactive = 0 				
					UNION ALL 				
					SELECT -1, '< All Resources >' 
					UNION ALL 
					SELECT -2, '< Multiple Resources >'
				) SubQ		
			)";

			m_pListResource->PutDisplayColumn(_bstr_t("[1]")); 
			m_pListResource->Requery(); 


			///
			// (b.spivey, January 8th, 2015) PLID 64398 - Locations data list
			AppendColumns(m_pListLocation, {
				{ "ID", "ID", 0, csFixedWidth },
				{ "Name", "Name", 0, csWidthAuto, cftTextSingleLine, 0 },
			});
			m_pListLocation->FromClause = R"(
			(
				SELECT ID, Name 
				FROM LocationsT
				WHERE TypeID = 1 AND Active = 1
				UNION ALL 
				SELECT -1, '< All Locations >'
				UNION ALL 
				SELECT -2, '< Multiple Locations >'
			) SubQ
			)";
			m_pListLocation->PutDisplayColumn(_bstr_t("[1]"));
			m_pListLocation->Requery();
			IRowSettingsPtr pLocationRow = m_pListLocation->SetSelByColumn(efdcID, GetCurrentLocationID()); 
			if (!pLocationRow) {
				m_pListLocation->SetSelByColumn(efdcID, efdsAll);
			}
			else {
				m_naryLocationIDs.Add(pLocationRow->GetValue(efdcID)); 
			}

			// (b.spivey, January 8th, 2015) PLID 64401 - Added support for cancel reasons. 
			AppendColumns(m_pListReason, {
				{ "ID", "ID", 0, csFixedWidth },
				{ "Name", "Name", 0, csWidthAuto, cftTextSingleLine, 0 },
			});
			m_pListReason->FromClause = R"(
				( SELECT ID, Description AS Name 
				FROM AptCancelReasonT 
				UNION ALL
				SELECT -1, '< All >'
				UNION ALL 
				SELECT -2, '< Cancelled >'
				UNION ALL
				SELECT -3, '< No Shows >'  ) SubQ
			)";
			m_pListReason->Requery(); 
			m_pListReason->PutDisplayColumn(_bstr_t("[1]"));
			m_pListReason->SetSelByColumn(efdcID, efdsAll);


			// (b.spivey, January 6th, 2015) PLID 64399 - List for types. 
			AppendColumns(m_pListType, {
				{ "ID", "ID", 0, csFixedWidth },
				{ "Name", "Name", 0, csWidthAuto, cftTextSingleLine, 0 },
			});
			m_pListType->FromClause = R"(	
				(
					SELECT ID, Name FROM AptTypeT WHERE Inactive = 0 				
					UNION ALL 				
					SELECT -1, '< All Types >' 				
				) SubQ	
			)";

			m_pListType->Requery();
			m_pListType->PutDisplayColumn("[1]");
			m_pListType->SetSelByColumn(efdcID, efdsAll);


			// (b.spivey, January 7th, 2015) PLID 64400 - 
			AppendColumns(m_pListPurpose, {
				{ "ID", "ID", 0, csFixedWidth },
				{ "Name", "Name", 0, csWidthAuto, cftTextSingleLine, 0 },
			});
			m_pListPurpose->FromClause = R"(
				(
					SELECT ID, Name FROM AptPurposeT
					UNION ALL 				
					SELECT -1, '< All Purposes >' 
				) SubQ	
			)";

			CString str;
			// (b.spivey, January 28th, 2015) - PLID 64400 
			str.Format("(ID = -1)");
			m_pListPurpose->WhereClause = _bstr_t(str);

			m_pListPurpose->Requery(); 
			m_pListPurpose->PutDisplayColumn("[1]"); 
			m_pListPurpose->SetSelByColumn(efdcID, efdsAll);
			
			///
			// (b.spivey, December 31, 2014) PLID 64387 - Added the queue list. 
			// (b.spivey, January 28, 2015) PLID 64387 - Changed column names to match MRD. 
			AppendColumns(m_pList, {
				{ "PatientID", "", 0, csFixedWidth },
				{ "AppointmentID", "", 0, csFixedWidth },
				{ "CancelReasonID", "", 0, csFixedWidth },
				{ "IsCancelled", "", 0, csFixedWidth, cftBoolTrueFalse },
				{ "RescheduleNotes", "", 24, csFixedWidth, cftBitmapBuiltIn},
				{ "NoShow", "", 28, csFixedWidth, cftTextSingleLine},
				{ "StartTime", "Date", 128, csWidthData, cftDateAndTime, 0 },
				{ "PatientName", "Patient Name", 128, csWidthData, cftTextSingleLine},
				{ "Resource", "Resource", 128, csVisible, cftTextSingleLine },
				{ "Purpose", "Purpose", 128, csVisible, cftTextSingleLine },
				{ "LocationName", "Location", 128, csVisible, cftTextSingleLine },
				{ "AppointmentNotes", "Notes", 90, csVisible, cftTextSingleLine },
				{ "HomePhone", "Home Phone", 95, csVisible, cftTextSingleLine },
				{ "CellPhone", "Mobile Phone", 95, csVisible, cftTextSingleLine },
				{ "WorkPhone", "Work Phone", 95, csVisible, cftTextSingleLine },
				{ "CancelledReason", "Cancellation Reason", 90, csVisible, cftTextSingleLine },
			});

			UpdateVisibleAppointmentColumns(m_pList, "ReschedulingQueueAppointmentListColumnWidths"); 

		}

		// (b.spivey, January 7th, 2015) PLID 64397 - Get the stored resources for this user. 
		CString strResources = GetRemotePropertyText("ReschedulingQueueResourceList", "", 0, GetCurrentUserName(), true); 
		ParseCommaDeliminatedText(m_naryResourceIDs, strResources);
		
		// (r.goldschmidt 2015-01-06 10:36) - PLID 64372 - Secure controls based on relevant permissions
		SecureControls();
		ToggleLabels();

		if (IsDocked()) {
			SetDlgItemText(IDC_TOGGLE_DOCK, "&Pop-out");
		}
		else {			
			SetDlgItemText(IDC_TOGGLE_DOCK, "&Dock to Scheduler");
		}

		// (a.walling 2015-02-09 16:35) - PLID 64570 - Set NXB_CLOSE style for the close button
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDCANCEL))->AutoSet(NXB_CLOSE);
		
		GetMainFrame()->RequestTableCheckerMessages(GetSafeHwnd());
	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// (a.walling 2015-01-22 16:08) - PLID 64662 - Rescheduling Queue - selecting specific appointments
void CReschedulingQueue::ResetFilters()
{
	m_pListResource->SetSelByColumn(efdcID, -1L);
	m_naryResourceIDs.RemoveAll();
	m_pListLocation->SetSelByColumn(efdcID, -1L);
	m_naryLocationIDs.RemoveAll();
		
	m_pListReason->SetSelByColumn(efdcID, -1L);
	m_pListType->SetSelByColumn(efdcID, -1L);
	m_pListPurpose->SetSelByColumn(efdcID, -1L);

	ToggleLabels();
}

// (a.walling 2015-01-22 16:08) - PLID 64662 - Rescheduling Queue - selecting specific appointments
void CReschedulingQueue::Refresh(long nApptID)
{
	try {
		m_nPendingApptID = -1;
		if (nApptID != -1) {
			if (m_pList->FindByColumn(ealcAppointmentID, nApptID, nullptr, VARIANT_TRUE)) {
				return;
			}
			else {
				m_nPendingApptID = nApptID;
			}
		}

		UpdateAppointmentList(); 
	} NxCatchAll(__FUNCTION__);
}

// (b.spivey, January 7th, 2015) PLID 64397 - Initially implemented for resources. 
LRESULT CReschedulingQueue::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try{
		switch (wParam) {
			// (b.spivey, May 28, 2013) - PLID 56872 - If they clicked the race label, then we 
			//		change and audit the patient's races based on the user's interactions. 
			case (IDC_RESOURCE_LIST_STATIC) :
			{
				MultiSelectResource();

				if (m_naryResourceIDs.GetCount() == 0) {
					m_pListResource->SetSelByColumn(efdcID, efdsAll);
				}
				else if (m_naryResourceIDs.GetCount() == 1) {
					IRowSettingsPtr pRow = m_pListResource->SetSelByColumn(efdcID, m_naryResourceIDs.GetAt(0));
					if (!pRow) {
						m_pListResource->Requery();
						m_pListResource->WaitForRequery(EPatienceLevel::dlPatienceLevelWaitIndefinitely);
						m_pListResource->SetSelByColumn(efdcID, m_naryResourceIDs.GetAt(0));
					}
				}
				SetRemotePropertyText("ReschedulingQueueResourceList", GetCommaDeliminatedText(m_naryResourceIDs), 0, GetCurrentUserName());
				break;
			}
			// (b.spivey, January 8th, 2015) PLID 64398 -
			case (IDC_LOCATION_LIST_STATIC) :
			{
				MultiSelectLocations();

				if (m_naryLocationIDs.GetCount() == 0) {
					m_pListLocation->SetSelByColumn(efdcID, efdsAll);
				}
				else if (m_naryLocationIDs.GetCount() == 1) {
					IRowSettingsPtr pRow = m_pListLocation->SetSelByColumn(efdcID, m_naryLocationIDs.GetAt(0));
					if (!pRow) {
						m_pListLocation->Requery();
						m_pListLocation->WaitForRequery(EPatienceLevel::dlPatienceLevelWaitIndefinitely);
						m_pListLocation->SetSelByColumn(efdcID, m_naryLocationIDs.GetAt(0));
					}
				}
				break;
			}

			default:
			{
				break;
			}
		}
		ToggleLabels();
	}NxCatchAll(__FUNCTION__);
	return 0;
}

void CReschedulingQueue::OnSize(UINT nType, int cx, int cy)
{
	CNxDialog::OnSize(nType, cx, cy);
}


void CReschedulingQueue::OnDestroy()
{
	CNxDialog::OnDestroy();

	try {
		GetMainFrame()->UnrequestTableCheckerMessages(GetSafeHwnd());

		m_naryResourceIDs.RemoveAll();
		m_naryLocationIDs.RemoveAll();
		m_nPendingApptID = -1;

		// why not
		m_pListResource = nullptr;
		m_pListLocation = nullptr;
		m_pListReason = nullptr;
		m_pListType = nullptr;
		m_pListPurpose = nullptr;
		m_pList = nullptr;
	} NxCatchAll(__FUNCTION__);
}

// (b.spivey, January 7th, 2015) PLID 64397 - Change the cursor so they know it's a link. 
BOOL CReschedulingQueue::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	try {
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		
		if ((m_nxstaticMultiResourceLabel.IsWindowVisible() && m_nxstaticMultiResourceLabel.IsWindowEnabled()))
		{
			m_nxstaticMultiResourceLabel.GetWindowRect(rc);
			ScreenToClient(&rc);
			//Only if we're over the label does the cursor change. 
			if (rc.PtInRect(pt))
			{
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
		// (b.spivey, January 8th, 2015) PLID 64398 -
		if (m_nxstaticMultiLocationLabel.IsWindowVisible() && m_nxstaticMultiLocationLabel.IsWindowEnabled())
		{
			m_nxstaticMultiLocationLabel.GetWindowRect(rc);
			ScreenToClient(&rc);
			//Only if we're over the label does the cursor change. 
			if (rc.PtInRect(pt))
			{
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	}NxCatchAll(__FUNCTION__);

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CReschedulingQueue::OnBnClickedCancel()
{
	if (IsDocked()) {
		ShowWindow(SW_HIDE);
		auto* pParent = dynamic_cast<CNxDialog*>(GetParent());

		::DestroyWindow(*this);

		if (pParent) {
			pParent->SetControlPositions();	
		}
	}
	else {		
		// parent is top level, eg CNxModelessParentDlg, just fwd the click
		GetParent()->PostMessage(WM_COMMAND, IDCANCEL);
	}
}


void CReschedulingQueue::OnBnClickedRescheduleAppointments()
{
	try {
		// (a.walling 2015-01-05 16:24) - PLID 64373 - Launch reschedule appointments dialog
		CRescheduleAppointmentsDlg dlg(this);
		dlg.DoModal();
		Refresh();
	} NxCatchAll(__FUNCTION__);
}

///RequireDataListSel

BEGIN_EVENTSINK_MAP(CReschedulingQueue, CNxDialog)
	ON_EVENT(CReschedulingQueue, IDC_LIST_RESOURCE, 16, CReschedulingQueue::SelChosenListResource, VTS_DISPATCH)
	ON_EVENT(CReschedulingQueue, IDC_LIST_LOCATIONS, 16, CReschedulingQueue::SelChosenListLocation, VTS_DISPATCH)
	ON_EVENT(CReschedulingQueue, IDC_LIST_REASON, 16, CReschedulingQueue::SelChosenListReason, VTS_DISPATCH)
	ON_EVENT(CReschedulingQueue, IDC_LIST_TYPE, 16, CReschedulingQueue::SelChosenListType, VTS_DISPATCH)
	ON_EVENT(CReschedulingQueue, IDC_LIST_PURPOSE, 16, CReschedulingQueue::SelChosenListPurpose, VTS_DISPATCH)

	ON_EVENT(CReschedulingQueue, IDC_LIST_RESOURCE, 1, CReschedulingQueue::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CReschedulingQueue, IDC_LIST_LOCATIONS, 1, CReschedulingQueue::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CReschedulingQueue, IDC_LIST_REASON, 1, CReschedulingQueue::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CReschedulingQueue, IDC_LIST_TYPE, 1, CReschedulingQueue::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CReschedulingQueue, IDC_LIST_PURPOSE, 1, CReschedulingQueue::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CReschedulingQueue, IDC_LIST_APPOINTMENTS, 33, CReschedulingQueue::DragInitListAppointments, VTS_PBOOL VTS_DISPATCH VTS_I2 VTS_I4)

	ON_EVENT(CReschedulingQueue, IDC_LIST_APPOINTMENTS, 22, OnColumnSizingFinishedAppointmentList, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CReschedulingQueue, IDC_LIST_APPOINTMENTS, 19, OnLeftClickAppointmentsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CReschedulingQueue, IDC_LIST_APPOINTMENTS, 3, OnDblClickCellAppointmentsList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CReschedulingQueue, IDC_LIST_APPOINTMENTS, 32, OnShowContextMenuAppointmentsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4 VTS_PBOOL)
END_EVENTSINK_MAP()


void CReschedulingQueue::SelChosenListResource(LPDISPATCH lpRow)
{
	try {
		
		//void CGeneral2Dlg::ToggleLabels() 
		IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			return; 
		}
		
		if (VarLong(pRow->GetValue(efdcID)) == -2) {
			MultiSelectResource(); 
			ToggleLabels(); 
		}
		else if (VarLong(pRow->GetValue(efdcID)) == -1) {
			m_naryResourceIDs.RemoveAll(); 
		}
		else {
			long nResourceID = VarLong(pRow->GetValue(efdcID));
			m_naryResourceIDs.RemoveAll(); 
			m_naryResourceIDs.Add(nResourceID);
		}
		UpdateAppointmentList(); 
		SetRemotePropertyText("ReschedulingQueueResourceList", GetCommaDeliminatedText(m_naryResourceIDs), 0, GetCurrentUserName());
	} NxCatchAll(__FUNCTION__);
}

// (b.spivey, January 7th, 2015) PLID 64397
void CReschedulingQueue::MultiSelectResource()
{
	CMultiSelectDlg dlg(this, "RescheduleQueueMultiResourceSelectDlg");
	CArray<long, long> naryUnselected;
	dlg.PreSelect(m_naryResourceIDs);
	if (dlg.Open("ResourceT", "", "ID", "Item", "Please select the resource(s) you want to filter on.") == IDOK) {
		m_naryResourceIDs.RemoveAll();
		dlg.FillArrayWithIDs(m_naryResourceIDs);
		dlg.FillArrayWithUnselectedIDs(&naryUnselected);
		if (naryUnselected.GetCount() <= 0) {
			m_naryResourceIDs.RemoveAll();
		}
		UpdateAppointmentList();
	}
}

// (b.spivey, January 7th, 2015) PLID 64397 - 
void CReschedulingQueue::ToggleLabels()
{
	//Resources - Label for listing multiple. 
	if (m_naryResourceIDs.GetCount() > 1) {

		CString strResourceLabel = "";
		CString strResourceToolTip = "";
		int nCount = 0, nSize = 0;
		nSize = m_naryResourceIDs.GetSize();

		//Create the label using the loaded data list
		while (nCount < nSize) {
			IRowSettingsPtr pRow = m_pListResource->FindByColumn(0, _variant_t(m_naryResourceIDs.GetAt(nCount)), 0, FALSE);
			nCount++;
			if (!pRow) {
				// (b.spivey, February 11, 2015) PLID 64397 - Update list to include new resource. 
				m_pListResource->Requery();
				m_pListResource->WaitForRequery(EPatienceLevel::dlPatienceLevelWaitIndefinitely);
				pRow = m_pListResource->FindByColumn(0, _variant_t(m_naryResourceIDs.GetAt((nCount-1))), 0, FALSE);
				if (!pRow) continue;
			}

			//Label is one line, ToolTip is multiline. 
			strResourceLabel += VarString(pRow->GetValue(efdcName)) + ", "; 
			strResourceToolTip += VarString(pRow->GetValue(efdcName)) + "\r\n";
		}
		strResourceLabel.Trim(", ");
		strResourceToolTip.Trim("\r\n");

		m_nxstaticMultiResourceLabel.SetText(strResourceLabel);
		m_nxstaticMultiResourceLabel.SetToolTip(strResourceToolTip);

		//Hide the datalist, show the label. 
		m_nxstaticMultiResourceLabel.EnableWindow(TRUE);
		m_nxstaticMultiResourceLabel.ShowWindow(SW_SHOWNA);
		m_pListResource->Enabled = FALSE;
		GetDlgItem(IDC_LIST_RESOURCE)->ShowWindow(SW_HIDE);

	}
	else {
		CString strResourceLabel = "";
		CString strResourceToolTip = "";

		m_nxstaticMultiResourceLabel.SetText(strResourceLabel);
		m_nxstaticMultiResourceLabel.SetToolTip(strResourceToolTip);

		//hide the label, show the datalist. 
		m_nxstaticMultiResourceLabel.EnableWindow(FALSE);
		m_nxstaticMultiResourceLabel.ShowWindow(SW_HIDE);
		m_pListResource->Enabled = TRUE;
		GetDlgItem(IDC_LIST_RESOURCE)->ShowWindow(SW_SHOW);

		//Special case, if they selected one from the multi-select dialog then I want to select it.
		//		If they selected none, I want nothing selected.
		if (m_naryResourceIDs.GetCount() == 0) {
			m_pListResource->SetSelByColumn(efdcID, efdsAll); 
		}
		else if (m_naryResourceIDs.GetCount() == 1) {
			IRowSettingsPtr pRow = m_pListResource->SetSelByColumn(efdcID, m_naryResourceIDs.GetAt(0));
			if (!pRow) {
				m_pListResource->Requery();
				m_pListResource->WaitForRequery(EPatienceLevel::dlPatienceLevelWaitIndefinitely);
				m_pListResource->SetSelByColumn(efdcID, m_naryResourceIDs.GetAt(0));
			}
		}
	}

	// (b.spivey, January 8th, 2015) PLID 64398 -
	//Locations - Label for listing multiple. 
	if (m_naryLocationIDs.GetCount() > 1) {

		CString strLocationLabel = "";
		CString strLocationToolTip = "";
		int nCount = 0, nSize = 0;
		nSize = m_naryLocationIDs.GetSize();

		//Create the label using the loaded data list
		while (nCount < nSize) {
			IRowSettingsPtr pRow = m_pListLocation->FindByColumn(0, _variant_t(m_naryLocationIDs.GetAt(nCount)), 0, FALSE);
			nCount++;
			if (!pRow) {
				// (b.spivey, February 11, 2015) PLID 64398 - Update the locations datalist to include this new selection.
				m_pListLocation->Requery();
				m_pListLocation->WaitForRequery(EPatienceLevel::dlPatienceLevelWaitIndefinitely);
				pRow = m_pListLocation->FindByColumn(0, _variant_t(m_naryLocationIDs.GetAt((nCount - 1))), 0, FALSE);
				if (!pRow) continue;
			}

			//Label is one line, ToolTip is multiline. 
			strLocationLabel += VarString(pRow->GetValue(efdcName)) + ", ";
			strLocationToolTip += VarString(pRow->GetValue(efdcName)) + "\r\n";
		}
		strLocationLabel.Trim(", ");
		strLocationToolTip.Trim("\r\n");

		m_nxstaticMultiLocationLabel.SetText(strLocationLabel);
		m_nxstaticMultiLocationLabel.SetToolTip(strLocationToolTip);

		//Hide the datalist, show the label. 
		m_nxstaticMultiLocationLabel.EnableWindow(TRUE);
		m_nxstaticMultiLocationLabel.ShowWindow(SW_SHOWNA);
		m_pListLocation->Enabled = FALSE;
		GetDlgItem(IDC_LIST_LOCATIONS)->ShowWindow(SW_HIDE);

	}
	else {
		CString strLocationLabel = "";
		CString strLocationToolTip = "";

		m_nxstaticMultiLocationLabel.SetText(strLocationLabel);
		m_nxstaticMultiLocationLabel.SetToolTip(strLocationToolTip);

		//hide the label, show the datalist. 
		m_nxstaticMultiLocationLabel.EnableWindow(FALSE);
		m_nxstaticMultiLocationLabel.ShowWindow(SW_HIDE);
		m_pListLocation->Enabled = TRUE;
		GetDlgItem(IDC_LIST_LOCATIONS)->ShowWindow(SW_SHOW);

		//Special case, if they selected one from the multi-select dialog then I want to select it.
		//		If they selected none, I want nothing selected.
		if (m_naryLocationIDs.GetCount() == 0) {
			m_pListLocation->SetSelByColumn(efdcID, efdsAll);
		}
		else if (m_naryLocationIDs.GetCount() == 1) {
			IRowSettingsPtr pRow = m_pListLocation->SetSelByColumn(efdcID, m_naryLocationIDs.GetAt(0));
			if (!pRow) {
				m_pListLocation->Requery();
				m_pListLocation->WaitForRequery(EPatienceLevel::dlPatienceLevelWaitIndefinitely);
				m_pListLocation->SetSelByColumn(efdcID, m_naryLocationIDs.GetAt(0));
			}
		}
	}
}
// (b.spivey, January 9th, 2015) PLID 64390 - need to be able to open up the notes dialog and add more notes
void CReschedulingQueue::OnLeftClickAppointmentsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			return;
		}

		switch (nCol) {
		case ealcRescheduleNotes:

		{

			long nApptID = VarLong(pRow->GetValue(ealcAppointmentID), -1);
			if (-1 == nApptID) {
				// We should never get into this state, because it means a appointment item has a notes icon but no ID in the
				// appointments table! That means the note cannot possibly be tied to the appointment. We should throw an exception.
				ThrowNxException("Attempted to add a note to an appointment that is not in the appointments table!");
			}

			// (b.spivey, February 6, 2015) - PLID 64395 - This has the potential to fail, lets handle that and tell the calling code that it failed.
			if (!ReturnsRecordsParam("SELECT TOP 1 ID FROM AppointmentsT WHERE ID = {INT}", nApptID)) {
				AfxMessageBox("The appointment could not be found. The list will now update.");
				UpdateAppointmentList();
				UpdateAppointmentsShownLabel();
				break; 
			}

			// Let the user add extra notes to this history item
			CNotesDlg dlgNotes(this);
			//// (a.walling 2010-09-20 11:41) - PLID 40589 - Set the patient ID
			dlgNotes.SetPersonID(VarLong(pRow->GetValue(ealcPatientID)));
			COleDateTime dtAppt = VarDateTime(pRow->GetValue(ealcApptTime));
			dlgNotes.SetApptInformation(nApptID, dtAppt, VarString(pRow->GetValue(ealcPurpose), ""), VarString(pRow->GetValue(ealcResource), ""));
			CNxModalParentDlg dlg(this, &dlgNotes, CString("Appointment Notes"));
			dlg.DoModal();

			//// Now update the icon				
			_RecordsetPtr prs = CreateParamRecordset(
				"SELECT CAST(CASE WHEN NoteInfoT.AppointmentID IS NULL THEN 0 ELSE 1 END AS BIT) AS HasExtraNotes "
				"FROM AppointmentsT LEFT JOIN NoteInfoT ON AppointmentsT.ID = NoteInfoT.AppointmentID WHERE AppointmentsT.ID = {INT}", nApptID);
			if (prs->eof) {
				//Don't do anything. 
			}
			else {
				BOOL bHasNotes = AdoFldBool(prs, "HasExtraNotes");
				if (bHasNotes) {
					pRow->PutValue(ealcRescheduleNotes, (long)m_hIconHasNotes);
				}
				else {
					pRow->PutValue(ealcRescheduleNotes, (long)m_hIconNotes);
				}
			}
		}
			break;
		default:
			break;
		}


	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2015-01-26 14:06) - PLID 64686
void CReschedulingQueue::OnDblClickCellAppointmentsList(LPDISPATCH lpRow, short nColIndex)
{
	try {		

		IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			return;
		}
		
		long nApptID = VarLong(pRow->GetValue(ealcAppointmentID));

		// (a.walling 2015-01-26 14:05) - PLID 64686 - If the queue is undocked and the appointment is double-clicked, 
		// automatically open the scheduler in day view with the appointment resource selected and select the current date. 
		// If there are multiple resources on an appointment, select the first alphabetical resource listed on the appointment in the scheduler.
		
		// (a.walling 2015-01-26 14:46) - PLID 64687
		long nResourceID = Nx::Scheduler::GetFirstAlphabeticalResource(nApptID);
				
		Nx::Scheduler::OpenResourceInDayViewForToday(nResourceID);

	}NxCatchAll(__FUNCTION__);
}

// (b.spivey, January 12, 2015) PLID 64395 - For menu options. 
void CReschedulingQueue::OnShowContextMenuAppointmentsList(LPDISPATCH lpRow, short nCol, long x, long y, long hwndFrom, VARIANT_BOOL* pbContinue)
{
	try {
		IRowSettingsPtr pRow(lpRow);

		if (!pRow) {
			return;
		}

		m_pList->PutCurSel(pRow);

		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;


		long nPatientID = VarLong(pRow->GetValue(ealcPatientID), 0);
		if (nPatientID > 0) {
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_GO_TO_PATIENT, "Go To Patient");
		}
		else {
			mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_DISABLED, ID_GO_TO_PATIENT, "Go To Patient");
		}
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_GO_TO_APPOINTMENT, "Go To Appointment");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_OPEN_RESOURCE_IN_DAY_VIEW, "Open Resource in Day View"); // (a.walling 2015-01-26 14:46) - PLID 64687
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_GO_TO_FINDFIRSTAVAILABEL, "Find Next Available Appointment"); //(s.dhole 6/4/2015 2:21 PM ) - PLID 65638
		
		mnu.InsertMenu(nIndex++, MF_SEPARATOR | MF_BYPOSITION);
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_REMOVE_FROM_QUEUE, "Remove From Queue");

		if (!!VarLong(pRow->GetValue(ealcIsCancelled), 0)) {
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_RESTORE_ORIGINAL_APPOINTMENT, "Restore Original Appointment");
		}

		CPoint pt;
		GetCursorPos(&pt);
		mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
	}NxCatchAll(__FUNCTION__); 
}

BOOL CReschedulingQueue::OnCommand(WPARAM wParam, LPARAM lParam)
{
	try {
		switch (wParam) {
			// (b.spivey, January 12, 2015) PLID 64395 - in this case the user wants to remove from the queue. So we make a 
			//	note on the appointment and remove it from the queue, then we remove it from the list. 
			case ID_REMOVE_FROM_QUEUE:
			{
				IRowSettingsPtr pRow = m_pList->GetCurSel();
				if (!pRow) {
					break;
				}

				CString strUserName = CString(GetCurrentUserName()); 
				long nApptID = VarLong(pRow->GetValue(ealcAppointmentID), -1);
				long nPersonID = VarLong(pRow->GetValue(ealcPatientID), -1);
				long nCurrentUserID = GetCurrentUserID();
				COleDateTime dtApptDate = VarDateTime(pRow->GetValue(ealcApptTime));
				CString strPurpose = VarString(pRow->GetValue(ealcPurpose), "");
				CString strResource = VarString(pRow->GetValue(ealcResource), "");

				// (b.spivey, February 6, 2015) - PLID 64395 - This has the potential to fail, lets handle that and tell the calling code that it failed.
				if (!Nx::Scheduler::RemoveFromReschedulingQueueWithNote(nApptID, nPersonID, nCurrentUserID, strPurpose, strResource, strUserName, dtApptDate)) {
					AfxMessageBox("The appointment could not be removed from the Rescheduling Queue. The list will now update.");
					UpdateAppointmentList();
					UpdateAppointmentsShownLabel();
					break; 
				}

				m_pList->RemoveRow(pRow); 

				UpdateAppointmentsShownLabel();
				break;
			}

			// (b.spivey, January 15, 2015) PLID 64396
			case ID_GO_TO_APPOINTMENT:
			{
				IRowSettingsPtr pRow = m_pList->GetCurSel();
				if (!pRow) {
					break;
				}
				long nAppID = VarLong(pRow->GetValue(ealcAppointmentID));
				OpenAppointment(nAppID); 

				break;
			}

			// (b.spivey, January 16, 2015) PLID 64394
			case ID_RESTORE_ORIGINAL_APPOINTMENT:
			{
				IRowSettingsPtr pRow = m_pList->GetCurSel();
				if (!pRow) {
					break;
				}
				long nApptID = VarLong(pRow->GetValue(ealcAppointmentID));
				if (AppointmentUncancel(this, nApptID)) {
					m_pList->RemoveRow(pRow);
					Nx::Scheduler::RemoveFromReschedulingQueue(nApptID);
					UpdateAppointmentsShownLabel();
				}
				break; 
			}

			// (b.spivey, January 16, 2015) PLID 64608
			case ID_GO_TO_PATIENT:
			{
				IRowSettingsPtr pRow = m_pList->GetCurSel();
				if (!pRow) {
					break;
				}
				long nPatientID = VarLong(pRow->GetValue(ealcPatientID));
				GoToPatient(nPatientID);
				break;
			}

			// (a.walling 2015-01-26 14:46) - PLID 64687
			case ID_OPEN_RESOURCE_IN_DAY_VIEW:
			{
				IRowSettingsPtr pRow = m_pList->GetCurSel();
				if (!pRow) {
					break;
				}
				long nApptID = VarLong(pRow->GetValue(ealcAppointmentID));
				long nResourceID = Nx::Scheduler::GetFirstAlphabeticalResource(nApptID);
			
				// (a.walling 2015-01-26 14:45) - PLID 64687 - add the ability to right-click an appointment in the queue and select “Open Resource in Day View”. 
				// If the queue is docked, it will automatically open the resource in the Day View with the current date selected. 
				// In the queue is not docked, this selection will automatically open the scheduler in the day view with the 
				// appointment resource selected and select the current date. If there are multiple resources on an appointment, 
				// select the first alphabetical resource listed on the appointment in the scheduler.
				Nx::Scheduler::OpenResourceInDayViewForToday(nResourceID);
				break;
			}
			case ID_GO_TO_FINDFIRSTAVAILABEL:
			{
				//(s.dhole 6/4/2015 2:21 PM ) - PLID 65638
				IRowSettingsPtr pRow = m_pList->GetCurSel();
				if (!pRow) {
					break;
				}
				
				long nApptID = VarLong(pRow->GetValue(ealcAppointmentID));
				if (LoadFindFirstAvailableAppt(nApptID))
				{
					if (IDNO == AfxMessageBox("Would you like to retain the original as a cancelled appointment?",
						MB_ICONQUESTION | MB_YESNO))
					{
						AppointmentDeleteNoHistory(nApptID, true);
					}

					m_pList->RemoveRow(pRow);
					Nx::Scheduler::RemoveFromReschedulingQueue(nApptID);
					UpdateAppointmentsShownLabel();
				}
				break;
			}
			default:
				break;
		}
	}NxCatchAll(__FUNCTION__); 

	return CDialog::OnCommand(wParam, lParam);
}

// (b.spivey, January 16, 2015) PLID 64608
void CReschedulingQueue::GoToPatient(long nPatientID)
{
	try {

		if (nPatientID == -1) {
			//do nothing
			return;
		}

		//Set the active patient
		CMainFrame *pMainFrame;
		pMainFrame = GetMainFrame();
		if (pMainFrame != NULL) {

			if (!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
				if (IDNO == MessageBox("This patient is not in the current lookup. \n"
					"Do you wish to reset the lookup to include all patients?", "Practice", MB_ICONQUESTION | MB_YESNO)) {
					return;
				}
			}

			if (pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {

				//Now just flip to the patient's module and set the active Patient
				pMainFrame->FlipToModule(PATIENT_MODULE_NAME);

				CNxTabView *pView = pMainFrame->GetActiveView();
				if (pView) {
					pView->UpdateView();
				}
			}
		}//end if MainFrame
		else {
			MsgBox(MB_ICONSTOP | MB_OK, "ReschedulingQueue.cpp: Cannot Open Mainframe");
		}//end else pMainFrame	

	}NxCatchAll(__FUNCTION__);
}

// (b.spivey, January 13, 2015) PLID 64402 - When the filters change, we'll change our label. 
void CReschedulingQueue::UpdateAppointmentList()
{
	try {

		CString strFilter = "";

		if (!m_naryResourceIDs.IsEmpty()) {
			strFilter += " AT.ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID IN ("; 
			foreach(long i, m_naryResourceIDs) {
				CString str;
				str.Format("%li", i); 
				strFilter += str + ", "; 
			}
			strFilter.TrimRight(", "); 
			strFilter += ")) "; 
		}

		if (!m_naryLocationIDs.IsEmpty()) {
			strFilter += " AND AT.LocationID IN (";
			foreach(long i, m_naryLocationIDs) {
				CString str;
				str.Format("%li", i);
				strFilter += str + ", ";
			}
			strFilter.TrimRight(", ");
			strFilter += ") ";
		}

		IRowSettingsPtr pRow = m_pListType->GetCurSel();
		if (pRow) {
			long nID = pRow->GetValue(efdcID);
			if (nID != efdsAll && nID > 0) {
				CString str;
				str.Format(" AND AT.AptTypeID = %li", nID); 
				strFilter += str; 
			}
		}

		pRow = m_pListPurpose->GetCurSel();
		if (pRow) {
			long nID = pRow->GetValue(efdcID);
			if (nID != efdsAll && nID > 0) {
				CString str;
				str.Format(" AND AT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID = %li) ", nID);
				strFilter += str;
			}
		}

		pRow = m_pListReason->GetCurSel();
		if (pRow) {
			long nID = pRow->GetValue(efdcID);
			CString str; 
			if (nID == efdsCancel) {
				str.Format(" AND AT.Status = 4 ");
			}
			else if (nID == efdsNoShow) {
				str.Format(" AND AT.ShowState = 3 "); 
			}
			else if (nID > 0) {
				str.Format(" AND CancelReasonID = %li", nID); 
			}
			strFilter += str; 
		}

		strFilter.TrimLeft(); 
		if (strFilter.Left(3).CompareNoCase("AND") == 0) {
			strFilter.Delete(0, 3);
		}
		
		if (strFilter.GetLength() > 0) {
			strFilter = "WHERE " + strFilter;
		}
		
		// (b.spivey, January 22, 2015) PLID 64388 
		CString strPhones = "";
		if (!(GetCurrentUserPermissions(bioPatientPhoneNumbers) & (SPT__R________))) {
			strPhones = R"( 
				'< Private >' AS CellPhone, 
				'< Private >' AS HomePhone, 
				'< Private >' AS WorkPhone,
			)";
		}
		else {
			strPhones = R"( 
				CASE WHEN PT.PrivCell = 1 THEN '< Private >' ELSE PT.CellPhone END AS CellPhone, 
				CASE WHEN PT.PrivHome = 1 THEN '< Private >' ELSE PT.HomePhone END AS HomePhone, 
				CASE WHEN PT.PrivWork = 1 THEN '< Private >' ELSE PT.WorkPhone END AS WorkPhone, 
				)";
		}

		// (b.spivey, January 8th, 2015) PLID 64390  
		CString strAppointmentList;
		// (b.spivey, January 23, 2015) PLID 64388 
		// (b.spivey, January 28, 2015) PLID 64388 - data formatting. 
		// (b.spivey, January 28, 2015) PLID 64388 - Consider NULLs. 
		strAppointmentList.Format(
			R"(

				(	
					SELECT 
						PT.ID AS PatientID, 
						CASE WHEN NotesQ.AppointmentID IS NOT NULL THEN %li ELSE %li END AS RescheduleNotes,
						RQT.AppointmentID AS AppointmentID,
						RQT.QueueDate,
						CASE WHEN AT.ShowState = 3 THEN '(n)' ELSE '' END AS NoShow,
						CASE WHEN PT.ID > 0 THEN Last + ', ' + First + ' ' + Middle ELSE '' END AS PatientName,
						LT.Name AS LocationName,
						AT.Notes AS AppointmentNotes,
						%s 
						CASE WHEN AT.Status = 4 THEN 1 ELSE 0 END AS IsCancelled,
						CASE WHEN AT.Status = 4 AND AT.CancelReasonID IS NOT NULL THEN ACRT.Description
							WHEN AT.Status = 4 AND Len(CancelledReason) > 0 THEN CancelledReason
							WHEN AT.ShowState = 3 THEN '< No Show >' 
							ELSE '' END AS CancelledReason,
						AT.StartTime,
						LEFT(ResourceSubQ.ResourceList, LEN(ResourceSubQ.ResourceList) - 1) AS Resource,
						ATT.Name + COALESCE((' - ' + LEFT(PurposeSubQ.PurposeList, LEN(PurposeSubQ.PurposeList) - 1)), '') AS Purpose,
						COALESCE(AT.CancelReasonID, -1) AS CancelReasonID  
					FROM ReschedulingQueueT RQT
					INNER JOIN AppointmentsT AT ON RQT.AppointmentID = AT.ID
					INNER JOIN PersonT PT ON AT.PatientID = PT.ID
					LEFT JOIN LocationsT LT ON AT.LocationID = LT.ID
					LEFT JOIN AptCancelReasonT ACRT ON AT.CancelReasonID = ACRT.ID 
					LEFT JOIN AptTypeT ATT ON AT.AptTypeID = ATT.ID 
					LEFT JOIN (
						SELECT DISTINCT AppointmentID 
						FROM NoteInfoT 
					) NotesQ ON AT.ID = NotesQ.AppointmentID 
					CROSS APPLY(
						SELECT(
							SELECT RT.Item + ', '
							FROM  AppointmentResourceT ART
							INNER JOIN  ResourceT RT ON ART.ResourceID = RT.ID
							WHERE ART.AppointmentID = AT.ID
							ORDER BY RT.Item
							FOR XML PATH(''), TYPE
						).value('/', 'nvarchar(max)')
					) ResourceSubQ(ResourceList)
					CROSS APPLY(
						SELECT(
							SELECT AptPurposeName.Name + ', '
							FROM  AppointmentPurposeT AptPurposeLink
							INNER JOIN  AptPurposeT AptPurposeName ON AptPurposeLink.PurposeID = AptPurposeName.ID
							WHERE AptPurposeLink.AppointmentID = AT.ID
							ORDER BY AptPurposeName.Name
							FOR XML PATH(''), TYPE
						).value('/', 'nvarchar(max)')
					) PurposeSubQ(PurposeList) 

					%s 

				) SubQ
			)", (long)m_hIconHasNotes, (long)m_hIconNotes, strPhones, strFilter);
		m_pList->FromClause = _bstr_t(strAppointmentList);

		// (a.walling 2015-01-29 13:06) - PLID 64586 - Rescheduling Queue - Refresh, table checkers
		long nCurID = -1;
		{
			if (IRowSettingsPtr pCurRow = m_pList->GetCurSel()) {
				nCurID = pCurRow->Value[ealcAppointmentID];
			}
		}

		KillTimer(ID_REFRESH_EVENTUALLY);
		m_bRefreshOnShow = false;

		m_pList->Requery(); 
		m_pList->WaitForRequery(EPatienceLevel::dlPatienceLevelWaitIndefinitely); 

		if (m_nPendingApptID != -1) {
			auto found = m_pList->FindByColumn(ealcAppointmentID, m_nPendingApptID, nullptr, VARIANT_TRUE);
			if (!found && !strFilter.IsEmpty()) {
				// (a.walling 2015-01-22 16:08) - PLID 64662 - could not find it and filter was not empty, so reset filter and try again
				ResetFilters();
				UpdateAppointmentList();
				return;
			}
			m_nPendingApptID = -1;
		}
		else if (nCurID != -1) {
			m_pList->FindByColumn(ealcAppointmentID, m_nPendingApptID, nullptr, VARIANT_TRUE);
		}

		UpdateAppointmentsShownLabel();
	} NxCatchAll(__FUNCTION__);
}

// (b.spivey, January 13, 2015) PLID 64402 - update the label. 
void CReschedulingQueue::UpdateAppointmentsShownLabel()
{
	long nFilteredQueue = m_pList->GetRowCount(); 
	long nTotalQueue = 0;
	_RecordsetPtr prs = CreateParamRecordset("SELECT COUNT(*) AS CountOfQueue FROM ReschedulingQueueT "); 
	if (!prs->eof) {
		nTotalQueue = AdoFldLong(prs->Fields, "CountOfQueue", -1);
	}
	CString str;
	str.Format("%li shown out of %li", nFilteredQueue, nTotalQueue); 
	m_nxstaticAppointmentsShowLabel.SetText(str); 
	
}

// (b.spivey, January 8th, 2015) PLID 64398 -
void CReschedulingQueue::SelChosenListLocation(LPDISPATCH lpRow)
{
	try {
		
		IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			return;
		}

		if (VarLong(pRow->GetValue(efdcID)) == efdsMultiple) {
			MultiSelectLocations();
			ToggleLabels();
		}
		else if (VarLong(pRow->GetValue(efdcID)) == efdsAll) {
			m_naryLocationIDs.RemoveAll();
		}
		else {
			long nLocationID = VarLong(pRow->GetValue(efdcID));
			m_naryLocationIDs.RemoveAll();
			m_naryLocationIDs.Add(nLocationID);
		}

		UpdateAppointmentList(); 

	} NxCatchAll(__FUNCTION__);
}


// (b.spivey, January 7th, 2015) PLID 64397
void CReschedulingQueue::MultiSelectLocations()
{
	CMultiSelectDlg dlg(this, "RescheduleQueueMultiLocationSelectDlg");
	CArray<long, long> naryUnselected; 
	dlg.PreSelect(m_naryLocationIDs);
	if (dlg.Open("LocationsT", "TypeID = 1 AND Active = 1", "ID", "Name", "Please select the location(s) you want to filter on.") == IDOK) {
		m_naryLocationIDs.RemoveAll();
		dlg.FillArrayWithIDs(m_naryLocationIDs);
		dlg.FillArrayWithUnselectedIDs(&naryUnselected);
		if (naryUnselected.GetCount() <= 0) {
			m_naryLocationIDs.RemoveAll(); 
		}
		UpdateAppointmentList();
	}
}



void CReschedulingQueue::SelChosenListReason(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			return; 
		}

		UpdateAppointmentList();
	} NxCatchAll(__FUNCTION__);
}


void CReschedulingQueue::SelChosenListType(LPDISPATCH lpRow)
{
	try {
		// (b.spivey, January 8th, 2015) PLID 64400 - 
		IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			return; 
		}
		
		long nTypeID = pRow->GetValue(efdcID);
		CString str, strFilter; 

		// (b.spivey, January 28th, 2015) - PLID 64400 
		if (nTypeID > 0) {
			str.Format("ID IN (SELECT AptPurposeID FROM AptPurposeTypeT WHERE AptTypeID = %li) OR", nTypeID); 
		}

		strFilter.Format("(%s ID = -1)", str);

		if (VarString(m_pListPurpose->GetWhereClause(), "").CompareNoCase(strFilter) != 0) {
			m_pListPurpose->WhereClause = _bstr_t(strFilter);
			m_pListPurpose->Requery();
			m_pListPurpose->SetSelByColumn(efdcID, efdsAll);
		}
		UpdateAppointmentList();
	} NxCatchAll(__FUNCTION__);
}


void CReschedulingQueue::SelChosenListPurpose(LPDISPATCH lpRow)
{
	try {
		UpdateAppointmentList();
	} NxCatchAll(__FUNCTION__);
}

///

struct CReschedulingQueueDropSource
	: public CNxOleDropSource
{
	
	virtual SCODE GiveFeedback(DROPEFFECT dropEffect) override
	{
		auto sc = COleDropSource::GiveFeedback(dropEffect);

		if (sc == DRAGDROP_S_USEDEFAULTCURSORS) {
			if (dropEffect != DROPEFFECT_NONE) {
				SetCursor(AfxGetApp()->LoadCursor(IDC_POINTER_COPY)); // ew
				sc = S_OK;
			}
		}
		
		return sc;
	}
};


// (a.walling 2015-01-05 14:17) - PLID 64410 - Rescheduling Queue - DragBegin for appointment row
void CReschedulingQueue::DragInitListAppointments(BOOL* pbCancel, LPDISPATCH lpRow, short nCol, long nFlags)
{
	try {
		if (!lpRow) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;

		long id = pRow->Value[ealcAppointmentID];

		CSharedFile data;
		data.Write(&id, sizeof(id));

		if (!m_pDataSource) {
			m_pDataSource.reset(new CNxOleDataSource(), false);
		}
		m_pDataSource->Empty();

		boost::intrusive_ptr<CNxOleDataSource> pDs = m_pDataSource;

		pDs->CacheGlobalData(Nx::Scheduler::cfRes, data.Detach());

		CReschedulingQueueDropSource dropSource;
		DROPEFFECT de = pDs->DoDragDrop(DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK, nullptr, &dropSource);

		if (de != DROPEFFECT_NONE) {
			*pbCancel = TRUE;
			if (!ReturnsRecordsParam("SELECT NULL FROM ReschedulingQueueT WHERE AppointmentID = {INT}", id)) {
				m_pList->RemoveRow(pRow);
				UpdateAppointmentsShownLabel();
			}
		}

		pDs.reset();

	} NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2015-01-06 10:36) - PLID 64372 - Secure controls based on relevant permissions
void CReschedulingQueue::SecureControls()
{

	if (!(GetCurrentUserPermissions(bioMassRescheduleAppointments) & sptWrite)){
		GetDlgItem(IDC_RESCHEDULE_APPOINTMENTS)->EnableWindow(FALSE);
	}

}

// (b.spivey, January 6th, 2015) PLID 64389 - Remember column widths
void CReschedulingQueue::SaveColumnWidths(NXDATALIST2Lib::_DNxDataListPtr pList, CString strRememberedWidthsConfigRTName)
{
	//Don't save in this case. 
	if (!m_bRememberColumns) {
		return;
	}

	CString strColumnWidths;
	for (int i = 0; i < pList->ColumnCount; i++)
	{
		NXDATALIST2Lib::IColumnSettingsPtr pCol = pList->GetColumn(i);
		CString str;

		str.Format("%d", pCol->StoredWidth);

		if (i > 0)
			strColumnWidths += ",";

		strColumnWidths += str;
	}

	SetRemotePropertyText(strRememberedWidthsConfigRTName, strColumnWidths, 0, GetCurrentUserName());
}

void CReschedulingQueue::OnColumnSizingFinishedAppointmentList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try {
		SaveColumnWidths(m_pList, "ReschedulingQueueAppointmentListColumnWidths");
	} NxCatchAll(__FUNCTION__); 
}

void CReschedulingQueue::UpdateVisibleAppointmentColumns(NXDATALIST2Lib::_DNxDataListPtr pList, CString strRememberedWidthsConfigRTName)
{
	// datalist, you so crazy. unless i set the stored width here it will scrunch everything up into remaining space 
	// to avoid a horiz scoll bar. since the auto columns were never resized (by app code or by user). so we are
	// 'resizing' them meow! even if they are not 'remembering' columns, we want it to 'remember' between requeries.
	for (int i = 0; i < pList->ColumnCount; i++)
	{
		auto pCol = pList->GetColumn(i);
		pCol->PutStoredWidth(pCol->GetStoredWidth());
	}

	// (b.spivey, January 28, 2015) PLID 64389 - only load if they remembered. 
	if (m_bRememberColumns) { 
		CString strRememberedWidths = GetRemotePropertyText(strRememberedWidthsConfigRTName, "", 0, GetCurrentUserName(), true);
	
		//convert the remembered widths, if any, to an array
		CArray<int, int> aryRememberedWidths;
		if (!strRememberedWidths.IsEmpty()) {
			ParseCommaDeliminatedText(aryRememberedWidths, strRememberedWidths);

			//if the remembered width array is not equal to our column count,
			//we cannot possibly apply it, and therefore have to reset their widths
			if (aryRememberedWidths.GetSize() != pList->ColumnCount) {
				strRememberedWidths = "";
				SetRemotePropertyText(strRememberedWidthsConfigRTName, "", 0, GetCurrentUserName());
				aryRememberedWidths.RemoveAll();
				return; 
			}

			for (int i = 0; i < pList->ColumnCount; i++)
			{
				// (b.spivey, February 2, 2015) - PLID 64389 - I don't know exactly how this works, but it 
				//	 seems that setting the style csVisible overrides any auto-setting magic. 
				NXDATALIST2Lib::IColumnSettingsPtr pCol = pList->GetColumn(i);
				// (b.spivey, February 4, 2015) - PLID 64389 - Use iterator, since that actually changes. 
				if (pList == m_pList && i <= ealcIsCancelled) {
					pCol->PutStoredWidth(0);
					pCol->PutColumnStyle(NXDATALIST2Lib::csFixedWidth | NXDATALIST2Lib::csVisible); 
					continue; 
				}
				else if (pList == m_pList && i >= ealcRescheduleNotes && i <= ealcNoShow) {
					pCol->PutStoredWidth(aryRememberedWidths.GetAt(i));
					//Still don't know how this works but it needs to be "visible" as well as having a fixed width. 
					pCol->PutColumnStyle(NXDATALIST2Lib::csFixedWidth | NXDATALIST2Lib::csVisible); 
					continue; 
				}
				pCol->PutStoredWidth(aryRememberedWidths.GetAt(i));
				pCol->PutColumnStyle(NXDATALIST2Lib::csVisible); 
			}
		}
	}
}

void CReschedulingQueue::OnBnClickedRememberColumnWidths()
{
	try {
		m_bRememberColumns = !!m_checkRememberColumnWidths.GetCheck();

		SetRemotePropertyInt("ReschedulingQueueRememberColumnWidths", (m_bRememberColumns ? 1 : 0), 0, GetCurrentUserName()); 

		if (m_bRememberColumns) {
			SaveColumnWidths(m_pList, "ReschedulingQueueAppointmentListColumnWidths");
		} 
		else {
			
		}

	} NxCatchAll(__FUNCTION__); 
}

// (a.walling 2015-01-12 16:42) - PLID 64570 - Toggle dock
void CReschedulingQueue::OnBnClickedToggleDock()
{
	try {
		g_bReschedulingQueueShouldDock = !g_bReschedulingQueueShouldDock;
		GetMainFrame()->ShowReschedulingQueue();
	} NxCatchAll(__FUNCTION__);
}

LRESULT CReschedulingQueue::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {

	} NxCatchAllIgnore();
	return 0;
}

// (a.walling 2015-01-29 13:06) - PLID 64586 - Rescheduling Queue - Refresh, table checkers
LRESULT CReschedulingQueue::OnTableChangedEx(WPARAM wParam, LPARAM lParam)
{
	try {
		switch (wParam) {
		case NetUtils::ReschedulingQueueT:
		{
			if (!m_pList) {
				break;
			}
			if (m_pList->IsRequerying()) {
				break;
			}
			CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
			if (!pDetails) {
				break;
			}
			CString xml = pDetails->GetXml();

			using namespace pugi;
			xml_document doc;
			auto result = doc.load(xml);

			if (!result) {
				break;
			}
			
			bool updated = false;

			for (auto removed : doc.select_nodes("/tc/del/@id")) {
				long id = removed.attribute().as_int();
				if (auto pRow = m_pList->FindByColumn(ealcAppointmentID, id, nullptr, VARIANT_FALSE)) {
					m_pList->RemoveRow(pRow);
					updated = true;
				}
			}
			
			if (doc.select_node("/tc/add/@id")) {
				RefreshEventually(1000);
			}

			if (updated) {
				UpdateAppointmentsShownLabel();
			}

			break;
		}
		}
	} NxCatchAllIgnore();
	return 0;
}

void CReschedulingQueue::RefreshEventually(UINT nTimeout)
{
	m_bRefreshOnShow = true;
	KillTimer(ID_REFRESH_EVENTUALLY);
	SetTimer(ID_REFRESH_EVENTUALLY, nTimeout, nullptr);	
}

void CReschedulingQueue::OnTimer(UINT_PTR nIDEvent)
{
	try {
		if (nIDEvent != ID_REFRESH_EVENTUALLY) {
			return CWnd::OnTimer(nIDEvent);
		}

		KillTimer(ID_REFRESH_EVENTUALLY);

		if (!m_pList || m_pList->IsRequerying()) {
			return;
		}

		if (!IsWindowVisible()) {
			m_bRefreshOnShow = true;
			return;
		}
		
		m_bRefreshOnShow = false;
		Refresh();
	} NxCatchAllIgnore();
}


void CReschedulingQueue::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CWnd::OnShowWindow(bShow, nStatus);

	try {
		if (bShow && m_bRefreshOnShow) {
			m_bRefreshOnShow = false;
			RefreshEventually(0);
		}
	} NxCatchAllIgnore();
}

void CReschedulingQueue::LoadResourceIDStringIntoArray(CString strResourceIDs, CDWordArray& adwIDs)
{
	while (strResourceIDs.GetLength()) {
		long nID;
		if (-1 != strResourceIDs.Find(" ")) {
			nID = atoi(strResourceIDs.Left(strResourceIDs.Find(" ")));
			strResourceIDs = strResourceIDs.Right(strResourceIDs.GetLength() - strResourceIDs.Find(" ") - 1);
		}
		else  {
			nID = atoi(strResourceIDs);
			strResourceIDs.Empty();
		}
		adwIDs.Add(nID);
	}
}

//(s.dhole 6/4/2015 2:21 PM ) - PLID 65638
bool CReschedulingQueue::LoadFindFirstAvailableAppt(long nAppointmentID)
{

	_RecordsetPtr rs = AppointmentGrab(nAppointmentID, TRUE, TRUE);
	if (rs == NULL || rs->eof) {
		//how did this happen?
		ASSERT(FALSE);
		return false;
	}
	else {
		long nPatientID = AdoFldLong(rs, "PatientID");
		COleDateTime dtApptDate = VarDateTime(rs->Fields->Item["Date"]->Value);
		long nLocationID = VarLong(AdoFldLong(rs, "LocationID"));
		long nPrimaryInsuredPartyID = AdoFldLong(rs, "PrimaryInsuredPartyID", -1);
		long nAptTypeID = AdoFldLong(rs, "AptTypeID", -1);
		CDWordArray adwResourceIDs;
		LoadResourceIDStringIntoArray(AdoFldString(rs, "ResourceIDs", ""), adwResourceIDs);
		CDWordArray adwPurposeIDs;
		LoadResourceIDStringIntoArray(AdoFldString(rs, "PurposeIDs", ""), adwPurposeIDs);
		// (r.farnworth 2016-01-25 17:19) - PLID 65638 - Need time fields for duration
		COleDateTime dtStart = AdoFldDateTime(rs, "StartTime");
		COleDateTime dtEnd = AdoFldDateTime(rs, "EndTime");
		
		GetMainFrame()->m_FirstAvailAppt.m_bDefaultOverrides.bPatient = true;
		GetMainFrame()->m_FirstAvailAppt.m_nPreselectPatientID = nPatientID;
		GetMainFrame()->m_FirstAvailAppt.m_bDefaultOverrides.bAppt = true;
		GetMainFrame()->m_FirstAvailAppt.m_nPreselectApptTypeID = nAptTypeID;

		// (r.farnworth 2016-01-25 15:15) - PLID 65638 - Add Location to the preselect
		GetMainFrame()->m_FirstAvailAppt.m_bDefaultOverrides.bLocation = true;
		CArray<long, long> naryLocationIDs;
		naryLocationIDs.Add(nLocationID);
		GetMainFrame()->m_FirstAvailAppt.m_aryPreselectLocations.Copy(naryLocationIDs);

		// (r.farnworth 2016-01-25 15:15) - PLID 65638 - Add Duration to the preselect
		GetMainFrame()->m_FirstAvailAppt.m_bDefaultOverrides.bDurations = true;
		COleDateTimeSpan dtDuration = dtEnd - dtStart;
		GetMainFrame()->m_FirstAvailAppt.m_nPreselectDurHours = dtDuration.GetHours();
		GetMainFrame()->m_FirstAvailAppt.m_nPreselectDurMins = dtDuration.GetMinutes();
		GetMainFrame()->m_FirstAvailAppt.m_nPreselectIntervalMins = (long)dtDuration.GetTotalMinutes();
		
		GetMainFrame()->m_FirstAvailAppt.m_bRunImmediately = false;
		// warn user
		if (adwPurposeIDs.GetSize() > 1) {
			MsgBox("This appointment has multiple purposes. Please select the default purpose from the Find First Available screen and add multiple purposes upon creating your appointment.");
		}
		else if (adwPurposeIDs.GetSize() == 1) {
			GetMainFrame()->m_FirstAvailAppt.m_nPreselectApptPurposeID = adwPurposeIDs[0];
		}
		GetMainFrame()->m_FirstAvailAppt.m_bDefaultOverrides.bResources = true;
		GetMainFrame()->m_FirstAvailAppt.m_aryPreselectResources.RemoveAll();
		foreach(DWORD dwResID, adwResourceIDs) {
			GetMainFrame()->m_FirstAvailAppt.m_aryPreselectResources.Add(dwResID);
		}
		if (adwResourceIDs.GetSize() > 1) {
			GetMainFrame()->m_FirstAvailAppt.m_nPreselectResourceTypeSelection = -202; //FFA_MULTIPLE_SCHEDULER_RESOURCE
		}
		else if (adwResourceIDs.GetSize() == 1) {
			GetMainFrame()->m_FirstAvailAppt.m_nPreselectResourceTypeSelection = adwResourceIDs[0];
		}
		GetMainFrame()->m_FirstAvailAppt.m_bDefaultOverrides.bInsurance = true;
		GetMainFrame()->m_FirstAvailAppt.m_nPreselectInsuredPartyID = nPrimaryInsuredPartyID;

		GetMainFrame()->m_FirstAvailAppt.m_bReSchedule = true;
		GetMainFrame()->m_FirstAvailAppt.m_bReturnSlot = true;

		if (IDCANCEL == GetMainFrame()->m_FirstAvailAppt.DoModal()) {
			return false;
		}

		CClient::RefreshAppointmentTable(GetMainFrame()->m_FirstAvailAppt.m_nNewResID);
		return true;
	}
}
