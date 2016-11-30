// EmrTreeWnd.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "EmrTreeWnd.h"
#include "EMN.h"
#include "EMNMoreInfoDlg.h"
#include "EmrTopicWnd.h"
#include "EmrItemAdvListDlg.h"
#include "EmrItemAdvTableDlg.h"
#include "SelectDlg.h"
#include "AuditTrail.h"
#include "EmrItemAdvImageDlg.h"
#include "MultiSelectDlg.h"
#include "EmrEditorDlg.h"
#include "PicContainerDlg.h"
#include "TaskEditDlg.h"
#include "InternationalUtils.h"
#include <mmsystem.h>
#include "ChooseEMNCategoryDlg.h"
#include "DontShowDlg.h"
#include "FileUtils.h"
#include "EMNLoader.h"
#include "NxCDO.h"
#include "EMNUnspawner.h"
#include "EmrDebugDlg.h"
#include "EMRAlertDlg.h"
#include "EMNWaitForAccessDlg.h"
#include "PatientSummaryDlg.h"
#include "EMRProblemListDlg.h"
#include "EMRProblemEditDlg.h"
#include "ChooseDateDlg.h"
#include "NewCropUtils.h"
#include "NewCropBrowserDlg.h"
#include "EMRProblemChooserDlg.h"
#include "EmrPositionSpawnedTopicsDlg.h"
#include "DeleteEMNConfirmDlg.h"
#include "EMRPreviewMultiPopupDlg.h"
#include "PicContainerDlg.h"
#include <NxDataUtilitiesLib/NxAlgorithm.h>
#include "EmrColors.h"
#include "FirstDataBankUtils.h"
#include "EMREMChecklistDlg.h"
#include "PrescriptionQueueDlg.h"
#include "NxModalParentDlg.h"
#include "EMNMedication.h"	// (j.jones 2012-11-30 14:03) - PLID 53966 - moved the EMNMedication class to its own file
#include "ReconcileMedicationsUtils.h"
#include "EMRTopic.h"
#include "EMR.h"
#include "EMNDetail.h"
#include "DecisionRuleUtils.h"
#include "EMNProvider.h"
#include "CCDAUtils.h" // (b.savon 2014-02-25 14:22) - PLID 61029
#include "DiagQuickListUtils.h"
#include "SelectCCDAInfoDlg.h"  // (b.savon 2014-05-01 16:57) - PLID 61909
#include "EmrItemAdvImageState.h"	// (j.armen 2014-07-21 16:32) - PLID 62836

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2012-07-11 09:39) - PLID 51476 - NXM_SET_SAVE_DOCS_IN_HISTORY, NXM_TREE_SEL_CHANGED is no more

// (a.walling 2011-10-20 21:22) - PLID 46077 - Facelift - EMR tree pane
// Use GetEmrPreviewCtrl() internally; use EmrTree for sensible access to the tree and objects.

using namespace ADODB;
using namespace NXDATALIST2Lib;

// (j.jones 2007-04-03 14:56) - PLID 25464 - set the splitter info here
// (a.walling 2008-06-11 09:46) - PLID 30351 - decrease this buffer
const long gc_nTopicWndRightDistFromSplitterRight = 5;
const long gc_nSplitterThickness = 5;

// (j.jones 2007-04-03 15:01) - PLID 25464 - set the splitter limits here
// (a.walling 2007-04-19 13:17) - PLID 25725 - Changed the splitter limits to allow stretching
// the preview pane across the entire emn. There is a snapping functionality adhering to the tree.
#define LEFT_SPLITTER_LIMIT			0.0
#define RIGHT_SPLITTER_LIMIT		1.0
#define SPLITTER_SNAP_WIDTH			30

#define EMR_PRESCRIPTION_QUEUE_WIDTH	900
#define EMR_PRESCRIPTION_QUEUE_HEIGHT	700

// (c.haag 2007-09-27 10:29) - PLID 27509 - General calls to pRow->Visible = ... have been replaced
// with SetTreeRowVisible(). General calls to m_pTree->CurSel = ... have been replaced with SetTreeSel()

// (c.haag 2007-10-04 16:38) - PLID 27509 - General calls to m_pTree->RemoveRow have been replaced with
// RemoveTreeRow()

// (j.jones 2013-05-08 09:31) - PLID 56596 - moved function defines to the .cpp
namespace EmrTree
{
	ChildRow::ChildRow()
	{
	}

	ChildRow::ChildRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
		: m_pRow(pRow)
	{
	}

	NXDATALIST2Lib::IRowSettingsPtr ChildRow::GetRow()
	{
		return m_pRow;
	}

	ChildRow::operator NXDATALIST2Lib::IRowSettings*()
	{
		return m_pRow;
	}

	NXDATALIST2Lib::IRowSettingsPtr& ChildRow::operator->()
	{
		return m_pRow;
	}

	EmrTreeRowType ChildRow::GetType()
	{
		if (!IsValid()) return etrtInvalid;
		return static_cast<EmrTreeRowType>(VarLong(m_pRow->GetValue(TREE_COLUMN_ROW_TYPE)));
	}

	long ChildRow::IsLoaded()
	{
		if (!IsValid()) return 0;
		return static_cast<long>(VarLong(m_pRow->GetValue(TREE_COLUMN_LOADED)));
	}

	bool ChildRow::IsEMN()
	{
		return GetType() == etrtEmn;
	}

	bool ChildRow::IsTopic()
	{
		return GetType() == etrtTopic;
	}

	bool ChildRow::IsValid()
	{
		return m_pRow ? true : false;
	}

	bool ChildRow::IsMoreInfo()
	{
		return GetType() == etrtMoreInfo;
	}

	bool ChildRow::IsPlaceholder()
	{
		return GetType() == etrtPlaceholder;
	}

	//TES 2/12/2014 - PLID 60748 - New row type
	bool ChildRow::IsCodes()
	{
		return GetType() == etrtCodes;
	}

	//TES 2/19/2014 - PLID 60750 - Several places treat Codes and More Info the same way
	bool ChildRow::IsMoreInfoOrCodes()
	{
		return GetType() == etrtMoreInfo || GetType() == etrtCodes;
	}

	CEMN* ChildRow::GetEMN()
	{	
		if (!IsValid()) return NULL;

		if (IsEMN()) {
			return reinterpret_cast<CEMN*>(VarLong(m_pRow->GetValue(TREE_COLUMN_OBJ_PTR)));
		} else if (GetParent()) {
			return GetParent().GetEMN();
		} else {
			return NULL;
		}
	}

	CEMRTopic* ChildRow::GetTopic()
	{
		if (!IsValid() || !IsTopic()) return NULL;

		return reinterpret_cast<CEMRTopic*>(VarLong(m_pRow->GetValue(TREE_COLUMN_OBJ_PTR)));
	}

	CEmrTopicWndPtr ChildRow::GetTopicWnd()
	{
		CEMRTopic* pTopic = GetTopic();
		if (!pTopic) return CEmrTopicWndPtr();
		return pTopic->GetTopicWnd();
	}

	////

	ChildRow ChildRow::GetNext()
	{
		if (!IsValid()) {
			return ChildRow();
		}
		
		return ChildRow(m_pRow->GetNextRow());
	}

	ChildRow ChildRow::GetPrev()
	{
		if (!IsValid()) {
			return ChildRow();
		}
		
		return ChildRow(m_pRow->GetPreviousRow());
	}

	ChildRow ChildRow::GetParent()
	{
		if (!IsValid()) {
			return ChildRow();
		}
		
		return ChildRow(m_pRow->GetParentRow());
	}

	TreeRow::TreeRow()
	{
	}

	TreeRow::TreeRow(NXDATALIST2Lib::IRowSettingsPtr pChildRow, NXDATALIST2Lib::IRowSettingsPtr pRootRow)
		: ChildRow(pChildRow)
		, m_pRootRow(pRootRow)
	{
	}

	TreeRow TreeRow::GetNext()
	{
		if (!IsValid()) {
			return TreeRow();
		}

		NXDATALIST2Lib::IRowSettingsPtr pNextRow;

		pNextRow = m_pRow->GetFirstChildRow();

		if (!pNextRow) {
			pNextRow = m_pRow->GetNextRow();
		}

		if (!pNextRow) {
			pNextRow = m_pRow;

			while (pNextRow = pNextRow->GetParentRow()) {
				if (pNextRow == m_pRootRow) {
					return TreeRow();
				}
				
				NXDATALIST2Lib::IRowSettingsPtr pNextSibling = pNextRow->GetNextRow();
				if (pNextSibling) {
					return TreeRow(pNextSibling, m_pRootRow);
				}
			}
		}

		if (pNextRow) {
			return TreeRow(pNextRow, m_pRootRow);
		} else {
			return TreeRow();
		}
	}

	TreeRow TreeRow::GetPrev()
	{
		if (!IsValid()) {
			return TreeRow();
		}

		NXDATALIST2Lib::IRowSettingsPtr pPrevRow;

		pPrevRow = m_pRow->GetPreviousRow();

		if (pPrevRow) {
			// get the absolute last child row here
			NXDATALIST2Lib::IRowSettingsPtr pLastChildRow;
			while (pLastChildRow = pPrevRow->GetLastChildRow()) {
				pPrevRow = pLastChildRow;
			}
		} else {
			pPrevRow = m_pRow->GetParentRow();

			if (pPrevRow == m_pRootRow) {
				pPrevRow = NULL;
			}
		}

		if (pPrevRow) {
			return TreeRow(pPrevRow, m_pRootRow);
		} else {
			return TreeRow();
		}
	}

	TreeRow TreeRow::GetParent()
	{
		if (!IsValid()) {
			return TreeRow();
		}

		NXDATALIST2Lib::IRowSettingsPtr pParentRow = m_pRow->GetParentRow();
		if (!pParentRow || pParentRow == m_pRootRow) {
			return TreeRow();
		}
		
		return TreeRow(pParentRow, m_pRootRow);
	}
};

/////////////////////////////////////////////////////////////////////////////
// CEmrTreeWnd

IMPLEMENT_DYNAMIC(CEmrTreeWnd, CWnd)

// (j.jones 2013-05-16 14:25) - PLID 56596 - m_EMR is now a reference, and needs to be filled here
CEmrTreeWnd::CEmrTreeWnd() :
	m_checkCategories(NetUtils::EMNTabCategoriesT), m_checkCharts(NetUtils::EMNTabChartsT),
	m_checkCategoryChartLink(NetUtils::EmnTabChartCategoryLinkT),
	// (j.jones 2007-07-23 11:09) - PLID 26742 - added medications/allergies info checkers
	m_CurrentMedicationsInfoChecker(NetUtils::CurrentMedicationsEMRInfoID),
	m_CurrentAllergiesInfoChecker(NetUtils::CurrentAllergiesEMRInfoID),
	// (j.jones 2010-06-21 15:21) - PLID 37981 - supported generic tables
	m_CurrentGenericTableInfoChecker(NetUtils::CurrentGenericTableEMRInfoID)
	, m_dlgEMChecklistSetup(this)
	, m_EMR(*(new CEMR()))
{
	m_lpDraggingRow = NULL;
	m_EMR.SetInterface(this);
	m_bInitialLoadComplete = FALSE;
	m_nEMRSavePref = 1;

	m_bDrugInteractionsChangedThisSession = FALSE;
	m_bHasAnsweredHasNoAllergiesPrompt = FALSE;
	m_bHasAnsweredHasNoCurrentMedsPrompt = FALSE;

	// (c.haag 2007-09-26 17:13) - PLID 27509 - This is now obselete (from PLID 21425)
	//m_bExpectingSelChange = FALSE;
	// (j.jones 2007-04-03 14:56) - PLID 25464 - added support for the splitter bar

	// (a.walling 2007-06-11 15:46) - PLID 26278 - Ensure the CSS file here
	CEMRPreviewCtrlDlg::EnsureCSSFile();

	m_bIsTemplate = FALSE; // (a.walling 2007-04-12 10:02) - PLID 25601 - this was garbage value when actually used on an EMN

	m_bReloadingEMN = FALSE;

	m_bDragging = FALSE;

	// (a.walling 2007-06-19 17:10) - PLID 25548
	m_bAutoScroll = FALSE;

	// (a.walling 2007-08-31 15:19) - PLID 24733
	m_bEMNCompleteInitialSave = TRUE;

	// (a.walling 2007-10-01 13:51) - PLID 25548 - Whether we are in ShowEMN or not
	// (will be either 0 or 1, but implemented as a counting variable in case
	// the function can be called recursively in the future)
	m_nInShowEMN = 0;
	
	// (a.walling 2008-04-29 09:20) - PLID 29815 - Load our icon size, and initialize our memory
	m_nIconSize = GetRemotePropertyInt("EMRTreeIconSize", 32, 0, GetCurrentUserName(), true);
	

	m_brBackground.CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
}

CEmrTreeWnd::~CEmrTreeWnd()
{
	try {

		// (j.jones 2013-05-16 14:25) - PLID 56596 - clear our m_EMR reference, it is never null,
		// it's always filled in the constructor
		delete &m_EMR;

	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2007-12-17 16:17) - PLID 28391 - Added NXM_REFRESH_TOPIC_HTML_VISIBILITY handler
// (a.walling 2008-06-02 12:17) - PLID 22049 - Added OnEmnLoadComplete
// (a.walling 2008-09-18 10:38) - PLID 26781 - Added NXM_EMR_LIMIT_RESOURCES handler
BEGIN_MESSAGE_MAP(CEmrTreeWnd, CWnd)
	//{{AFX_MSG_MAP(CEmrTreeWnd)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_MESSAGE(NXM_PRE_DELETE_EMR_ITEM, OnPreDeleteEmrItem)
	ON_MESSAGE(NXM_POST_ADD_EMR_ITEM, OnPostAddEmrItem)
	ON_WM_CREATE()
	ON_MESSAGE(NXM_MERGE_OVERRIDE_CHANGED, OnMergeOverrideChanged)
	ON_MESSAGE(NXM_EMN_DETAIL_DRAG_BEGIN, OnEmnDetailDragBegin)
	ON_MESSAGE(NXM_EMN_DETAIL_DRAG_END, OnEmnDetailDragEnd)
	ON_MESSAGE(NXM_EMN_DETAIL_UPDATE_PREVIEW_POSITION, OnEmnDetailUpdatePreviewPosition)
	ON_WM_TIMER()
	ON_COMMAND(IDM_OPEN_EMN, OnOpenEmn)
	ON_COMMAND(IDM_FINISH_EMN, OnFinishEmn)
	ON_COMMAND(IDM_LOCK_EMN, OnLockEmn)
	ON_COMMAND(IDM_SAVE_ROW, OnSaveRow)
	ON_COMMAND(IDM_CHANGE_EMN_DATE, OnChangeEmnDate)
	// (j.jones 2009-09-23 16:49) - PLID 29718 - added multiple options for changing dates
	ON_COMMAND(IDM_SET_EMN_DATE_TODAY, OnSetEmnDateToday)
	ON_COMMAND(IDM_SET_EMN_DATE_LAST_APPT, OnSetEmnDateLastAppt)
	ON_COMMAND(IDM_DELETE_EMN, OnDeleteEmn)
	// (j.jones 2007-09-11 15:36) - PLID 27352 - added unique function for AddTopic vs. AddSubtopic
	ON_COMMAND(IDM_ADD_NEW_TOPIC, OnAddNewTopic)
	ON_COMMAND(IDM_ADD_NEW_SUBTOPIC, OnAddNewSubtopic)
	ON_COMMAND(IDM_DELETE_TOPIC, OnDeleteTopic)
	ON_MESSAGE(NXM_EMN_CHARGE_ADDED, OnEmnChargeAdded)
	ON_MESSAGE(NXM_EMN_CHARGE_CHANGED, OnEmnChargeChanged)
	ON_MESSAGE(NXM_EMN_DIAG_ADDED, OnEmnDiagAdded)
	ON_MESSAGE(NXM_EMN_MEDICATION_ADDED, OnEmnMedicationAdded)
	ON_MESSAGE(NXM_EMR_TOPIC_ADDED, OnEmrTopicAdded)
	ON_MESSAGE(NXM_EMR_TOPIC_ADD_PREVIEW, OnEmrTopicAddPreview)
	ON_MESSAGE(NXM_EMR_SPAWNING_COMPLETE, OnEmrSpawningComplete)
	ON_MESSAGE(NXM_EMN_ADDED, OnEmnAdded)
	ON_MESSAGE(NXM_EMN_PROCEDURE_ADDED, OnEmrProcedureAdded)
	ON_MESSAGE(NXM_EMN_MULTI_PROCEDURES_ADDED, OnEmrMultiProceduresAdded)
	ON_MESSAGE(NXM_UPDATE_NAME_EMNPROCEDURE, OnUpdateNameOfEMNProcedure)
	ON_MESSAGE(NXM_PRE_DELETE_EMN_CHARGE, OnPreDeleteEmnCharge)
	ON_MESSAGE(NXM_PRE_DELETE_EMN_DIAG, OnPreDeleteEmnDiag)
	ON_MESSAGE(NXM_UPDATE_CODE_EMNDIAGCODE, OnUpdateCodeOfEMNDiagCode)
	ON_MESSAGE(NXM_PRE_DELETE_EMN_MEDICATION, OnPreDeleteEmnMedication)
	ON_MESSAGE(NXM_PRE_DELETE_EMN_PROCEDURE, OnPreDeleteEmnProcedure)
	ON_MESSAGE(NXM_POST_DELETE_EMN_PROCEDURE, OnPostDeleteEmnProcedure)
	ON_MESSAGE(NXM_PRE_DELETE_EMR_TOPIC, OnPreDeleteEmrTopic)
	ON_MESSAGE(NXM_PRE_DELETE_EMN, OnPreDeleteEmn)
	ON_MESSAGE(NXM_POST_DELETE_EMN, OnPostDeleteEmn)
	ON_MESSAGE(NXM_QUERY_UNSPAWN_EMNS, OnQueryUnspawnEmns)
	ON_MESSAGE(NXM_HIDE_EMR_TOPIC, OnHideEmrTopic)
	ON_MESSAGE(NXM_EMR_ITEM_EDITCONTENT, OnEmrItemEditContent)
	ON_MESSAGE(NXM_EMN_STATUS_CHANGING, OnEmnStatusChanging)
	ON_MESSAGE(NXM_EMN_MORE_INFO_CHANGED, OnEmnMoreInfoChanged)
	ON_MESSAGE(NXM_EMN_CODES_CHANGED, OnEmnCodesChanged)
	ON_MESSAGE(NXM_EMN_CHANGED, OnEmnChanged)
	ON_MESSAGE(NXM_EMR_ITEM_ADDED, OnEmrItemAdded)
	ON_COMMAND(IDM_TOGGLE_SHOW_IF_EMPTY, OnToggleShowIfEmpty)
	ON_MESSAGE(NXM_EMN_REFRESH_CHARGES, OnEmnRefreshCharges)
	ON_MESSAGE(NXM_EMN_REFRESH_PRESCRIPTIONS, OnEmnRefreshPrescriptions)
	ON_MESSAGE(NXM_EMN_REFRESH_DIAG_CODES, OnEmnRefreshDiagCodes)
	ON_COMMAND(ID_EMR_ADD_ITEM, OnAddItemToTopic)
	ON_MESSAGE(NXM_EMR_ITEM_STATECHANGED, OnEmrItemStateChanged)
	// (j.jones 2007-08-30 09:29) - PLID 27243 - Office Visit incrementing is no longer
	// used in the L2 EMR, it's in Custom Records only
	//ON_MESSAGE(NXM_PROCESS_EMR_OFFICE_VISITS, OnProcessEMROfficeVisits)
	ON_COMMAND(IDM_TOGGLE_HIDE_ON_EMN, OnToggleHideOnEmn)
	ON_COMMAND(IDM_COPY_EMN, OnCopyEmn)
	ON_MESSAGE(NXM_EMR_ITEM_CHANGED, OnEmrItemChanged)
	ON_MESSAGE(NXM_EMR_TREE_ENSURE_VISIBLE_TOPIC, OnEmrTreeEnsureVisibleTopic)
	ON_MESSAGE(NXM_TOPIC_LOAD_COMPLETE, OnTopicLoadComplete)
	ON_MESSAGE(NXM_TOPIC_LOAD_COMPLETE_BY_EMN_LOADER, OnTopicLoadCompleteByPreloader)
	ON_MESSAGE(NXM_TOPIC_MODIFIED_CHANGED, OnTopicModifiedChanged)
	ON_MESSAGE(NXM_EMN_PREVIEW_LOAD, OnLoadEmnPreview)
	ON_MESSAGE(NXM_REFRESH_TOPIC_HTML_VISIBILITY, OnRefreshTopicHTMLVisibility)
	ON_COMMAND(IDM_GOTO_PREVIEW, OnGotoPreview)
	ON_COMMAND(IDM_AUTOSCROLL, OnAutoScroll)
	ON_COMMAND(IDM_HIDETOPICPREVIEW, OnHideTopicPreview)
	ON_COMMAND(IDM_HIDETOPICTITLE, OnHideTopicTitle)
	ON_COMMAND(IDM_HIDEALLDETAILSPREVIEW, OnHideAllDetailsPreview)
	ON_COMMAND(IDM_HIDEALLDETAILSTITLE, OnHideAllDetailsTitle)
	ON_COMMAND(IDM_COLUMN_ONE, OnTopicColumnOne)
	ON_COMMAND(IDM_COLUMN_TWO, OnTopicColumnTwo)
	// (a.walling 2009-07-06 10:10) - PLID 34793 - Clearing is deprecated
	/*
	ON_COMMAND(IDM_CLEAR_LEFT, OnTopicClearLeft)
	ON_COMMAND(IDM_CLEAR_RIGHT, OnTopicClearRight)
	*/
	// (a.walling 2009-07-06 12:34) - PLID 34793 - Grouping for columns
	ON_COMMAND(IDM_GROUP_BEGIN, OnTopicGroupBegin)
	ON_COMMAND(IDM_GROUP_END, OnTopicGroupEnd)
	ON_COMMAND(IDM_TEXT_RIGHT, OnTopicTextRight)
	// (a.walling 2010-08-31 18:20) - PLID 36148 - Page breaks
	ON_COMMAND(IDM_PAGE_BREAK_BEFORE, OnTopicNewPageBefore)
	ON_COMMAND(IDM_PAGE_BREAK_AFTER, OnTopicNewPageAfter)
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_CAPTURECHANGED()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(NXM_EMN_LOADSAVE_COMPLETE, OnEmnLoadSaveComplete)
	ON_MESSAGE(NXM_EMN_WRITEACCESS, OnEmnWriteAccessChanged)
	ON_COMMAND(IDM_TOGGLEREADONLY, OnToggleReadOnly)
	ON_COMMAND(IDM_RELOAD_EMN, OnReloadEMN)
	ON_MESSAGE(NXM_WINDOW_CLOSING, OnWindowClosing)
	ON_MESSAGE(NXM_TRY_ACQUIRE_WRITE_ACCESS, OnTryAcquireWriteAccess)
	ON_MESSAGE(NXM_UPDATE_EMR_PREVIEW, OnUpdateEmrPreview)
	ON_MESSAGE(NXM_EMN_FORCE_ACCESS_REQUEST, OnForceAccessRequest)
	ON_MESSAGE(NXM_EMN_TODO_ADDED, OnEmnTodoAdded)
	ON_MESSAGE(NXM_EMN_TODO_DELETED, OnEmnTodoDeleted)
	ON_MESSAGE(NXM_EMN_TODO_REFRESH_LIST, OnEmnTodoRefreshList)	
	ON_COMMAND(IDM_ADD_EMN_PROBLEM, OnAddEmnProblem)
	ON_COMMAND(IDM_EDIT_EMN_PROBLEM, OnEditEmnProblem)
	ON_COMMAND(IDM_LINK_EMN_PROBLEMS, OnLinkEmnProblems)
	ON_COMMAND(IDM_ADD_TOPIC_PROBLEM, OnAddTopicProblem)
	ON_COMMAND(IDM_EDIT_TOPIC_PROBLEM, OnEditTopicProblem)
	ON_COMMAND(IDM_LINK_TOPIC_PROBLEMS, OnLinkTopicProblems)
	ON_MESSAGE(NXM_EMR_PROBLEM_CHANGED, OnEmrProblemChanged)
	ON_MESSAGE(NXM_EMR_TOPIC_CHANGED, OnEmrTopicChanged)
	ON_MESSAGE(NXM_EMR_MINIMIZE_PIC, OnEmrMinimizePic)
	ON_MESSAGE(NXM_EMR_SAVE_ALL, OnMessageEmrSaveAll)
	ON_MESSAGE(NXM_NEWCROP_BROWSER_DLG_CLOSED, OnNewCropBrowserClosed)
	ON_MESSAGE(NXM_INSERT_STOCK_EMR_ITEM, OnInsertStockEmrItem)
	// (a.walling 2010-01-12 08:38) - PLID 36840 - Changed to defined message with EMRPREVIEW_ rather than a registered message
	ON_MESSAGE(NXM_EMRPREVIEW_PRINT_MULTIPLE, OnPrintMultiple)
	ON_MESSAGE(NXM_ADD_IMAGE_TO_EMR, OnAddImageToEMR)
	// (a.walling 2010-04-13 13:20) - PLID 37150 - The EMR is entirely loaded
	ON_MESSAGE(NXM_EMR_LOADED, OnEMRLoaded)
	// (j.jones 2010-06-21 10:22) - PLID 39010 - added ability to add a generic table to the EMR
	ON_MESSAGE(NXM_ADD_GENERIC_TABLE_TO_EMR, OnAddGenericTableToEMR)	
	ON_COMMAND(IDM_PUBLISH_TO_NEXWEB_PORTAL, OnPublishToNexWebPortal)
	ON_WM_CONTEXTMENU()
	ON_MESSAGE(NXM_EMR_CHECK_SAVE_SHOW_DRUG_INTERACTIONS, CheckSaveEMNForDrugInteractions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CEmrTreeWnd, CWnd)
    //{{AFX_EVENTSINK_MAP(CEmrTreeWnd)
	ON_EVENT(CEmrTreeWnd, IDC_EMR_TREE_CTL, 2 /* Sel Changed */, OnSelChangedTree, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CEmrTreeWnd, IDC_EMR_TREE_CTL, 26 /* RowExpanded */, OnRowExpandedTree, VTS_DISPATCH)
	ON_EVENT(CEmrTreeWnd, IDC_EMR_TREE_CTL, 8 /* EditingStarting */, OnEditingStartingTree, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CEmrTreeWnd, IDC_EMR_TREE_CTL, 9 /* EditingFinishing */, OnEditingFinishingTree, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEmrTreeWnd, IDC_EMR_TREE_CTL, 10 /* EditingFinished */, OnEditingFinishedTree, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEmrTreeWnd, IDC_EMR_TREE_CTL, 12 /* DragBegin */, OnDragBeginTree, VTS_PBOOL VTS_DISPATCH VTS_I2 VTS_I4)
	ON_EVENT(CEmrTreeWnd, IDC_EMR_TREE_CTL, 13 /* DragOverCell */, OnDragOverCellTree, VTS_PBOOL VTS_DISPATCH VTS_I2 VTS_DISPATCH VTS_I2 VTS_I4)
	ON_EVENT(CEmrTreeWnd, IDC_EMR_TREE_CTL, 14 /* DragEnd */, OnDragEndTree, VTS_DISPATCH VTS_I2 VTS_DISPATCH VTS_I2 VTS_I4)
	ON_EVENT(CEmrTreeWnd, IDC_EMR_TREE_CTL, 6 /* RButtonDown */, OnRButtonDownTree, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrTreeWnd, IDC_EMR_TREE_CTL, 1 /* SelChanging */, OnSelChangingTree, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEmrTreeWnd, IDC_EMR_TREE_CTL, 27 /* RowCollapsed */, OnRowCollapsedTree, VTS_DISPATCH)
	ON_EVENT(CEmrTreeWnd, IDC_EMR_TREE_CTL, 28 /* CurSelWasSet */, OnCurSelWasSetTree, VTS_NONE)
	// (a.walling 2012-06-28 17:11) - PLID 51276 - More Info should be a clickable link
	ON_EVENT(CEmrTreeWnd, IDC_EMR_TREE_CTL, 19 /* LeftClick */, OnLeftClickTree, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

#define HIDDEN_COLOR	RGB(192,192,192)

/////////////////////////////////////////////////////////////////////////////
// CEmrTreeWnd message handlers
#define MORE_INFO_NODE_ID	-2
//TES 2/12/2014 - PLID 60748 - New row type
#define CODES_NODE_ID	-3

void CEmrTreeWnd::CreateControls()
{
	try {
		//cache preferences

		// (j.jones 2012-09-27 12:04) - ***IMPORTANT: if you add more properties to be cached here, you have to add
		// the ConfigRT name to the cache in PicContainerDlg, because this window is created before EmrEditorDlg is.

		m_nEMRSavePref = GetRemotePropertyInt("EMRSavePref", 1, 0, GetCurrentUserName(), true);
		m_nAutoCollapse = GetRemotePropertyInt("EmrAutoCollapse", 2, 0, GetCurrentUserName(), true);
		// (a.walling 2007-06-19 17:11) - PLID 25548 - Pref to autoscroll
		// (a.walling 2008-10-21 10:48) - PLID 31752 - Change autoscroll default to ON (TRUE)
		m_bAutoScroll = GetRemotePropertyInt("EMNPreviewAutoScroll", TRUE, 0, GetCurrentUserName(), true);
		// (a.walling 2007-08-31 15:12) - PLID 24733 - Whether to save the entire EMN on the first save action.
		m_bEMNCompleteInitialSave = GetRemotePropertyInt("EMNCompleteInitialSave", 1, 0, "<None>", true);
		// (j.jones 2012-09-27 09:23) - PLID 52820 - store the drug interaction preference
		//0 - do not prompt, 1 - save & prompt when changing meds, or any other content that affects interactions,
		//2 - only warn when saving, 3 - only warn when closing
		//This preference is only available when they have the FDB license. Otherwise, we do not use this feature.
		m_eEMRDrugInteractionChecksPref = edictNoPrompt;
		// (j.fouts 2013-05-30 09:47) - PLID 56807 - Drug Interactions is now tied to NexERx
		if(g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts) {
			m_eEMRDrugInteractionChecksPref = (EMRDrugInteractionCheckType)GetRemotePropertyInt("EMRDrugInteractionChecks", (long)edictSaveWarnWhenMedsChange, 0, GetCurrentUserName(), true);
		}

		// (j.jones 2012-09-28 10:50) - PLID 52820 - track if drug interactions changed during this EMR session
		m_bDrugInteractionsChangedThisSession = FALSE;

		// (j.jones 2012-10-25 16:24) - PLID 53322 - track whether we prompted for "has no allergies", such that
		// we never prompt twice in one EMR session
		m_bHasAnsweredHasNoAllergiesPrompt = FALSE;

		// (j.jones 2012-10-29 09:19) - PLID 53324 - track whether we prompted for "has no medications", such that
		// we never prompt twice in one EMR session
		m_bHasAnsweredHasNoCurrentMedsPrompt = FALSE;

		if(m_pTree == NULL) {
			//We haven't created our datalist yet, so let's do it.
			CRect rTree;
			GetClientRect(&rTree);
			m_wndTree.CreateControl(_T("NXDATALIST2.NxDataListCtrl.1"), "", WS_CHILD, rTree, this, IDC_EMR_TREE_CTL);
					
			m_pTree = m_wndTree.GetControlUnknown();

			ASSERT(m_pTree != NULL); // We need to be able to get the control unknown and set an NxDataListPtr to point to it

			m_wndTree.ShowWindow(SW_SHOW);

			m_pTree->IsComboBox = FALSE;
			m_pTree->AllowSort = FALSE;
			m_pTree->DragVisible = TRUE;
			m_pTree->HeadersVisible = FALSE;

			// (a.walling 2012-08-16 08:38) - PLID 52164 - Removed commented-out code which contained a font leak just in case someone emulated it

			m_pTree->InsertColumn(TREE_COLUMN_ID, _bstr_t("ID"), _bstr_t(""), 0, csFixedWidth|csVisible);
			m_pTree->InsertColumn(TREE_COLUMN_ROW_TYPE, _bstr_t("WindowType"), _bstr_t(""), 0, csFixedWidth|csVisible);
			m_pTree->InsertColumn(TREE_COLUMN_OBJ_PTR, _bstr_t("Object"), _bstr_t(""), 0, csFixedWidth|csVisible);
			m_pTree->InsertColumn(TREE_COLUMN_ICON, _bstr_t("Icon"), _bstr_t(""), GetRemotePropertyInt("EMRTreeIconSize", 32, 0, GetCurrentUserName(), true), csVisible|csFixedWidth);
			m_pTree->GetColumn(TREE_COLUMN_ICON)->FieldType = NXDATALIST2Lib::cftBitmapBuiltIn;
			m_pTree->InsertColumn(TREE_COLUMN_NAME, _bstr_t("Name"), _bstr_t(""), 0, csVisible|csWidthAuto|csEditable);
			m_pTree->GetColumn(TREE_COLUMN_NAME)->DataType = VT_BSTR;
			// (b.cardillo 2006-03-17 14:13) - PLID 19759 - Text is vertically centered.
			m_pTree->GetColumn(TREE_COLUMN_NAME)->AlignV = NXDATALIST2Lib::vaVCenter;
			//DRT 2/17/2006 - PLID 19351 - Text will wrap.
			m_pTree->GetColumn(TREE_COLUMN_NAME)->FieldType = cftTextWordWrap;
			m_pTree->InsertColumn(TREE_COLUMN_LOADED, _bstr_t("Loaded"), _bstr_t(""),0, csFixedWidth|csVisible);
		}
	} NxCatchAll("Error in CreateControls");
}

void CEmrTreeWnd::LoadSubtopics(NXDATALIST2Lib::IRowSettings *pParentRow)
{
	try {

		// (j.jones 2007-06-15 09:30) - PLID 26297 - added locked color preference
		// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
		COLORREF cLocked = EmrColors::Topic::Locked();

		NXDATALIST2Lib::IRowSettingsPtr pRow(pParentRow);

		if(!EmrTree::ChildRow(pRow).IsLoaded()) {
			CWaitCursor cuWait;
			EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
			if(etrt == etrtEmn) {
				CEMN *pEMN = EmrTree::ChildRow(pRow).GetEMN();
				BOOL bLocked = (pEMN->GetStatus() == 2);
				//TES 2/19/2014 - PLID 60748 - The first child will now be the Codes topic instead of the More Info topic
				NXDATALIST2Lib::IRowSettingsPtr pCodesRow = pRow->GetFirstChildRow();
				ASSERT(EmrTree::ChildRow(pCodesRow).IsCodes());
				//m.hancock - 2/27/2006 - PLID 19503 - The background of the <More Info> node is not colored grey when the EMN is locked
				if(bLocked == TRUE) {
					pCodesRow->PutBackColor(cLocked);
					NXDATALIST2Lib::IRowSettingsPtr pMoreInfoRow = pCodesRow->GetNextRow();
					ASSERT(EmrTree::ChildRow(pMoreInfoRow).IsMoreInfo());
					pMoreInfoRow->PutBackColor(cLocked);
				}
				for(int i = 0; i < pEMN->GetTopicCount(); i++) {
					//TES 3/8/2006 - Don't add invisible topics.
					if(pEMN->GetTopic(i)->GetVisible()) 
						AddTopicRow(pEMN->GetTopic(i), pParentRow, pCodesRow);
				}
			}
			else if(etrt == etrtTopic) {
				IRowSettingsPtr pPlaceholder = pParentRow->GetFirstChildRow();
				CEMRTopic *pParentTopic = EmrTree::ChildRow(pRow).GetTopic();
				for(int i = 0; i < pParentTopic->GetSubTopicCount(); i++) {
					CEMRTopic *pTopic = pParentTopic->GetSubTopic(i);
					//TES 3/8/2006 - Don't add invisible topics.
					if(pParentTopic->GetSubTopic(i)->GetVisible()) {
						IRowSettingsPtr pNewRow = AddTopicRow(pParentTopic->GetSubTopic(i), pParentRow);

						// (a.walling 2007-05-18 10:35) - PLID 25092 - Mark these topics if we cant drag to them
						if (m_bDragging && m_bIsTemplate) {
							BOOL bSpawnedTemplateTopic = (m_bIsTemplate && pTopic->GetSourceActionID() != -1);

							if (bSpawnedTemplateTopic) {
								// this topic cannot be dragged to. So save its color to our map.
								OLE_COLOR color = pNewRow->GetBackColor();
								IRowSettings* pDerefRow = pNewRow;
								pDerefRow->AddRef();
								m_mapTopicColors[pDerefRow] = color;
								pNewRow->PutBackColor(cLocked);
							}
						}
					}
				}
				//Delete our placeholder row.
				RemoveTreeRow(pPlaceholder);
			}
			else {
				// (a.walling 2014-05-14 08:31) - PLID 62056 - This is normal
				// usually called via EnsureTopicRowLoadedAllSubTopics when passed an EMN
				// since it will call LoadSubtopics on all child rows.
				ASSERT(etrt == etrtMoreInfo || etrt == etrtCodes);
			}
		}
		pRow->PutValue(TREE_COLUMN_LOADED, (long)1);
		EnsureModifiedState(pRow);
	}NxCatchAll("Error in CEmrTreeWnd::LoadSubtopics()");
}

void CEmrTreeWnd::SetTemplate(long nEmrTemplateID) {
	// (c.haag 2007-03-08 09:41) - PLID 25110 - Assign a -1 patient ID
	// since templates are not associated with patients
	//DRT 7/27/2007 - PLID 26836 - Added parameter for nEMNIDTBeDisplayed, which is ignored for templates
	long nEMNID = -1;
	//TES 11/22/2010 - PLID 41582 - This is a template, so the PIC ID is -1
	SetEMR(nEmrTemplateID, -1, TRUE, -1, nEMNID);
}

// (c.haag 2007-03-07 16:48) - PLID 25110 - Added support for a patient ID
//DRT 7/27/2007 - PLID 26836 - Added a parameter for Preloading to let us know
//	which will be displayed to the user.  If the value is -2, the loading will choose
//	which EMN is to be displayed, and will change the nEMNIDToBeDisplayed variable to be
//	that selection.
//TES 11/22/2010 - PLID 41582 - Added nPicID (only really needed for new EMRs)
void CEmrTreeWnd::SetPatientEMR(long nEmrID, long nPicID, long nPatientID, long &nEMNIDToBeDisplayed) {

	SetEMR(nEmrID, nPicID, FALSE, nPatientID, nEMNIDToBeDisplayed);
}

void CEmrTreeWnd::SetIsTemplate(BOOL bIsTemplate)
{
	// (c.haag 2007-08-30 15:59) - PLID 27256 - Flags whether this is a template. This
	// should only be called by CEmrTemplateEditorDlg::OnInitDialog() to indicate to us
	// whether this is a template before CreateControls is called.
	m_bIsTemplate = bIsTemplate;
}

//DRT 7/27/2007 - PLID 26836 - Added a parameter for Preloading to let us know
//	which will be displayed to the user.  This parameter is ignored when nEmrID is -1, or
//	when bIsTemplate is TRUE.  If the value is -2, the loading will choose
//	which EMN is to be displayed, and will change the nEMNIDToBeDisplayed variable to be
//	that selection.
//TES 11/22/2010 - PLID 41582 - Added nPicID (only really needed for new EMRs)
void CEmrTreeWnd::SetEMR(long nEmrID, long nPicID, BOOL bIsTemplate, long nPatientID, long &nEMNIDToBeDisplayed)
{	
	// (c.haag 2007-03-07 16:48) - PLID 25110 - Added support for a patient ID so that
	// we don't need to call GetActivePatientID
	try {
		m_bIsTemplate = bIsTemplate;

		if(nEmrID == -1)
		{
			// (a.walling 2007-11-28 12:45) - PLID 28044 - Don't create a new one if license expired
			if (g_pLicense->HasEMR(CLicense::cflrSilent) == 2) {
				//TES 11/22/2010 - PLID 41582 - Pass in nPicID
				m_EMR.CreateNew(bIsTemplate?-1:nPatientID, bIsTemplate, nPicID);
			} else {
			}
			// (a.walling 2007-11-28 12:45) - PLID 28044 - Don't create a new one if license expired
			if (g_pLicense->HasEMR(CLicense::cflrSilent) == 2) {
				//TES 11/22/2010 - PLID 41582 - Pass in nPicID
				m_EMR.CreateNew(bIsTemplate?-1:nPatientID, bIsTemplate, nPicID);
			}
		}
		else {
			//Load the EMR
			//DRT 7/27/2007 - PLID 26836 - Pass in the EMN that is to be displayed.  This allows the EMR to properly preload
			//	the non-displayed EMNs in the background.
			m_EMR.LoadFromID(nEmrID, bIsTemplate, nEMNIDToBeDisplayed);
		}
		
		if(m_pTree == NULL) {
			//We haven't created our datalist yet, so let's do it.
			CreateControls();
		}

		//Set our initial edit mode.
		CheckDlgButton(IDC_EDIT_MODE, bIsTemplate?BST_CHECKED:BST_UNCHECKED);
		OnEditMode();

		// (d.thompson 2013-11-26 15:44) - PLID 59353 - Audit opening EMRs, only if it's non-new, and not a template.
		long nOpenAuditEvent = -1;
		if(nEmrID != -1 && !bIsTemplate && m_EMR.GetEMNCount() > 0) {
			nOpenAuditEvent = BeginNewAuditEvent();
		}

		//Display each of the EMNs
		for(int i = 0; i < m_EMR.GetEMNCount(); i++) {
			//Add to the tree.
			CEMN *pEMN = m_EMR.GetEMN(i);

			// (d.thompson 2013-11-26 15:37) - PLID 59353 - Audit the opening of any EMN.  We consider opening as "loaded in the editor", whether they 
			//	actually look at it or not.
			if(nOpenAuditEvent > 0) {
				AuditEvent(GetPatientID(), pEMN->GetPatientName(), nOpenAuditEvent, aeiEMNOpened, pEMN->GetID(), "", "Viewed", aepMedium, aetOpened);
			}

			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTree->GetNewRow();
			pRow->PutValue(TREE_COLUMN_ID, pEMN->GetID());
			pRow->PutValue(TREE_COLUMN_OBJ_PTR, (long)pEMN);
			pRow->PutValue(TREE_COLUMN_ICON, (long)0);
			pRow->PutValue(TREE_COLUMN_NAME, _bstr_t(GetEmnRowDisplayText(pEMN)));
			pRow->PutValue(TREE_COLUMN_ROW_TYPE, (long)etrtEmn);
			pRow->PutValue(TREE_COLUMN_LOADED, (long)0);
			m_pTree->AddRowAtEnd(pRow, NULL);
			EnsureModifiedState(pRow);
			
			//TES 2/12/2014 - PLID 60748 - Add the Codes child node that we know is always there.
			NXDATALIST2Lib::IRowSettingsPtr pChildRow = m_pTree->GetNewRow();
			pChildRow->PutValue(TREE_COLUMN_ID, (long)CODES_NODE_ID);
			pChildRow->PutValue(TREE_COLUMN_OBJ_PTR, (long)0);
			pChildRow->PutValue(TREE_COLUMN_ICON, (long)0);
			pChildRow->PutValue(TREE_COLUMN_NAME, _bstr_t("<Codes>"));
			pChildRow->PutValue(TREE_COLUMN_ROW_TYPE, (long)etrtCodes);
			pChildRow->PutValue(TREE_COLUMN_LOADED, (long)1);
			pChildRow->PutCellLinkStyle(TREE_COLUMN_NAME, dlLinkStyleTrue);
			m_pTree->AddRowAtEnd(pChildRow, pRow);
			EnsureModifiedState(pChildRow);
			
			//Add the More Info child node that we know is always there.
			pChildRow = m_pTree->GetNewRow();
			pChildRow->PutValue(TREE_COLUMN_ID, (long)MORE_INFO_NODE_ID);
			pChildRow->PutValue(TREE_COLUMN_OBJ_PTR, (long)0);
			pChildRow->PutValue(TREE_COLUMN_ICON, (long)0);
			pChildRow->PutValue(TREE_COLUMN_NAME, _bstr_t("<More Info>"));
			pChildRow->PutValue(TREE_COLUMN_ROW_TYPE, (long)etrtMoreInfo);
			pChildRow->PutValue(TREE_COLUMN_LOADED, (long)1);
			// (a.walling 2012-06-28 17:11) - PLID 51276 - More Info should be a clickable link
			//pChildRow->PutCellLinkStyle(TREE_COLUMN_ICON, dlLinkStyleTrue); // icon cells don't handle this well
			pChildRow->PutCellLinkStyle(TREE_COLUMN_NAME, dlLinkStyleTrue);
			m_pTree->AddRowAtEnd(pChildRow, pRow);
			EnsureModifiedState(pChildRow);


			GetParent()->SendMessage(NXM_EMN_ADDED, (WPARAM)m_EMR.GetEMN(i));

			
		}

		//DRT 2/8/2006 - PLID 19178 - I removed the code here that was forcing a load of the first EMN.  In many
		//	cases that's not what should have been done, and I don't think the EmrTreeWnd should be making that
		//	decision anyways, the EMREditorDlg is the one doing the loading.  I fixed up its loading scheme
		//	so that the first EMN is selected if it is needed.  This also turns out to be a speed boost, because
		//	we're no longer loading the topics for an EMN that we don't immediately need.

		m_bInitialLoadComplete = TRUE;
	}NxCatchAll("Error in CEmrTreeWnd::SetEMR()");
}

// (z.manning 2009-03-04 15:41) - PLID 33338 - Use the new source action info class
CEMN* CEmrTreeWnd::AddEMNFromTemplate(long nTemplateID, SourceActionInfo &sai, long nAppointmentID)
{
	CWaitCursor pWait;

	//Load the EMN
	//Tell the EMR not to send us actions until we've finished adding the row.
	//(e.lally 2006-03-20) - The calls for locking and unlocking the spawning are probably not needed here 
		//as the Add Emn From Template function does it correctly now (PLID 19754), but it is too close to a release
		//to risk introducing other problems.
	m_EMR.LockSpawning();
	CEMN *pEMN;	

	if(nTemplateID == -2) {

		//create a brand new EMN
		pEMN = new CEMN(&m_EMR);

		// (a.walling 2013-01-22 10:00) - PLID 54762 - Emr Appointment linking
		if (-1 != nAppointmentID) {
			pEMN->SetAppointment(EMNAppointment(nAppointmentID));
		}

		// (j.jones 2011-07-05 11:39) - PLID 43603 - this now takes in a class
		EMNStatus emnStatus;
		emnStatus.nID = 0;
		emnStatus.strName = "Open";
		pEMN->SetStatus(emnStatus);

		// (j.jones 2009-09-24 10:13) - PLID 29718 - added preference to default to the last appt. date
		COleDateTime dtToUse = COleDateTime::GetCurrentTime();
		if(GetRemotePropertyInt("EMNUseLastApptDate", 0, 0, "<None>", true) == 1) {
			COleDateTime dtLast = GetLastPatientAppointmentDate(m_EMR.GetPatientID());
			//make sure the appt. isn't before the patient's birthdate
			COleDateTime dtBirthDate = m_EMR.GetPatientBirthDate();		
			if(dtLast.GetStatus() != COleDateTime::invalid
				&& (dtBirthDate.GetStatus() == COleDateTime::invalid || dtLast >= dtBirthDate)) {
				dtToUse = dtLast;
			}
		}

		pEMN->SetEMNDate(dtToUse);
		pEMN->ClearSourceActionInfo();

		//retrieve and display the currently set values from the patient's data
		_RecordsetPtr rsPatientInfo = CreateParamRecordset("SELECT First, Middle, Last, Gender, BirthDate FROM PersonT WHERE ID = {INT}", m_EMR.GetPatientID());
		if(!rsPatientInfo->eof) {
			//Name
			pEMN->SetPatientNameFirst(AdoFldString(rsPatientInfo, "First", ""));
			pEMN->SetPatientNameMiddle(AdoFldString(rsPatientInfo, "Middle", ""));
			pEMN->SetPatientNameLast(AdoFldString(rsPatientInfo, "Last", ""));
			//Gender
			pEMN->SetPatientGender(AdoFldByte(rsPatientInfo, "Gender", 0));
			//Age
			_variant_t varBirthDate = rsPatientInfo->Fields->GetItem("BirthDate")->Value;
			if(varBirthDate.vt == VT_DATE) {
				// (j.dinatale 2010-10-13) - PLID 38575 - need to call GetPatientAgeOnDate which no longer does any validation, 
				//  validation should only be done when bdays are entered/changed
				pEMN->SetPatientAge(::GetPatientAgeOnDate(VarDateTime(varBirthDate), COleDateTime::GetCurrentTime(), TRUE));
			}
		}

		pEMN->LoadDefaultProviderIDs();
		// (d.lange 2011-03-24 10:33) - PLID 42987 - Let's try to autofill the Assistant/Tech field with the current user
		// only if they are flagged as one
		// (a.walling 2011-11-16 10:15) - PLID 46497 - Was not actually checking if the current user was a technician before assigning it to the EMN!
		if (IsCurrentUserTechnician()) {
		long nAutoFillTech = GetRemotePropertyInt("EmrTechnicianAutofillBehavior", 0, 0, "<None>", true);
		if(nAutoFillTech == 1) {
			pEMN->SetCurrentUserTechnicianID();
		}
		}
	
		//DRT 9/20/2006 - PLID 22413 - If preference enabled, pull the default ICD9 codes from General 2.  This code is also 
		//	executed in CEMN::LoadFromTemplateID
		if(GetRemotePropertyInt("LoadGeneral2ICD9ToEMR", 0, 0, "<None>", true) != 0) {
			long nPatientID = m_EMR.GetPatientID();
			// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
			//TES 3/3/2014 - PLID 61046 - Added ICD-10 fields
			// (s.dhole 2014-03-07 14:39) - PLID 61261 Added DefaultICD10DiagID validation
			CSqlFragment sqlSql("SELECT DiagCodes.ID AS DiagCodeID, DiagCodes.CodeNumber, DiagCodes.CodeDesc, 1 AS OrderIndex, "
					"DiagCodes_10.ID AS DiagCodeID_ICD10, DiagCodes_10.CodeNumber AS ICD10CodeNumber, DiagCodes_10.CodeDesc AS ICD10CodeDesc "
					"FROM PatientsT LEFT JOIN DiagCodes on PatientsT.DefaultDiagID1 = DiagCodes.ID LEFT JOIN DiagCodes DiagCodes_10 ON PatientsT.DefaultICD10DiagID1 = DiagCodes_10.ID WHERE PatientsT.PersonID = {INT} "
					" AND ((Patientst.DefaultDiagID1 IS NOT NULL   AND DiagCodes.Active = 1) OR (Patientst.DefaultICD10DiagID1 IS NOT NULL AND DiagCodes_10.Active = 1)) "
					"UNION SELECT DiagCodes.ID AS DiagCodeID, DiagCodes.CodeNumber, DiagCodes.CodeDesc, 2 AS OrderIndex, "
					"DiagCodes_10.ID AS DiagCodeID_ICD10, DiagCodes_10.CodeNumber AS ICD10CodeNumber, DiagCodes_10.CodeDesc AS ICD10CodeDesc "
					"FROM PatientsT LEFT JOIN DiagCodes on PatientsT.DefaultDiagID2 = DiagCodes.ID LEFT JOIN DiagCodes DiagCodes_10 ON PatientsT.DefaultICD10DiagID2 = DiagCodes_10.ID WHERE PatientsT.PersonID = {INT} "
					" AND ((Patientst.DefaultDiagID2 IS NOT NULL  AND DiagCodes.Active = 1) OR (Patientst.DefaultICD10DiagID2 IS NOT NULL AND DiagCodes_10.Active = 1))  "
					"UNION SELECT DiagCodes.ID AS DiagCodeID, DiagCodes.CodeNumber, DiagCodes.CodeDesc, 3 AS OrderIndex, "
					"DiagCodes_10.ID AS DiagCodeID_ICD10, DiagCodes_10.CodeNumber AS ICD10CodeNumber, DiagCodes_10.CodeDesc AS ICD10CodeDesc "
					"FROM PatientsT LEFT JOIN DiagCodes on PatientsT.DefaultDiagID3 = DiagCodes.ID LEFT JOIN DiagCodes DiagCodes_10 ON PatientsT.DefaultICD10DiagID3 = DiagCodes_10.ID WHERE PatientsT.PersonID = {INT} "
					" AND ((Patientst.DefaultDiagID3 IS NOT NULL  AND DiagCodes.Active = 1) OR (Patientst.DefaultICD10DiagID3 IS NOT NULL AND DiagCodes_10.Active = 1) )  "
					"UNION SELECT DiagCodes.ID AS DiagCodeID, DiagCodes.CodeNumber, DiagCodes.CodeDesc, 4 AS OrderIndex, "
					"DiagCodes_10.ID AS DiagCodeID_ICD10, DiagCodes_10.CodeNumber AS ICD10CodeNumber, DiagCodes_10.CodeDesc AS ICD10CodeDesc "
					"FROM PatientsT LEFT JOIN DiagCodes on PatientsT.DefaultDiagID4 = DiagCodes.ID LEFT JOIN DiagCodes DiagCodes_10 ON PatientsT.DefaultICD10DiagID4 = DiagCodes_10.ID WHERE PatientsT.PersonID = {INT} "
					" AND ((Patientst.DefaultDiagID4 IS NOT NULL AND DiagCodes.Active = 1) OR (Patientst.DefaultICD10DiagID4 IS NOT NULL AND DiagCodes_10.Active = 1))  ", 
					nPatientID, nPatientID, nPatientID, nPatientID);

			_RecordsetPtr prsDiags = CreateParamRecordset("{SQL}", sqlSql);
			while(!prsDiags->eof) {
				EMNDiagCode *pCode = new EMNDiagCode;
				// (j.jones 2008-07-23 10:20) - PLID 30819 - changed the original nID to nDiagCodeID,
				// then added a new nID for the actual record ID
				pCode->nID = -1;
				pCode->nDiagCodeID = AdoFldLong(prsDiags, "DiagCodeID", -1);
				//TES 3/3/2014 - PLID 61046 - Added ICD-10 fields
				pCode->nDiagCodeID_ICD10 = AdoFldLong(prsDiags, "DiagCodeID_ICD10", -1);
				pCode->bIsNew = TRUE;
				pCode->strCode = AdoFldString(prsDiags, "CodeNumber", "");
				pCode->strCodeDesc = AdoFldString(prsDiags, "CodeDesc", "");
				//TES 3/3/2014 - PLID 61046 - Added ICD-10 fields
				pCode->strCode_ICD10 = AdoFldString(prsDiags, "ICD10CodeNumber", "");
				pCode->strCodeDesc_ICD10 = AdoFldString(prsDiags, "ICD10CodeDesc", "");
				// (j.jones 2007-01-05 10:07) - PLID 24070 - if -1, AddDiagCode will auto-generate it
				pCode->nOrderIndex = -1;
				pCode->bHasMoved = FALSE;
				pCode->bChanged = FALSE;
				// (j.jones 2014-12-23 15:07) - PLID 64491 - added bReplaced
				pCode->bReplaced = FALSE;
				pEMN->AddDiagCode(pCode);
				prsDiags->MoveNext();
			}
			prsDiags->Close();
		}

		pEMN->SetNew();

		//TES 2/20/2007 - PLID 24750 - This EMN is finished loading (since it didn't actually have to load at all).
		pEMN->SetLoaded();

		m_EMR.AddEMN(pEMN);

		// (a.walling 2007-07-11 16:05) - PLID 26261 - Generate the HTML file for a brand new non-template EMN
		// For the purposes of the preview generation, we always need an HTML file to base everything on, and
		// eventually to create the MHTML archive from.
		// (a.walling 2007-09-27 12:44) - PLID 25548 - HTML generation is moved into CEMN::SetLoaded now.
		// pEMN->GenerateHTMLFile(TRUE, FALSE);
	}
	else {
		// (a.walling 2013-01-22 10:00) - PLID 54762 - Emr Appointment linking
		pEMN = m_EMR.AddEMNFromTemplate(nTemplateID, sai, NULL, nAppointmentID);
	}
	m_EMR.UnlockSpawning();

	// (a.wetta 2007-01-10 09:25) - PLID 14635 - Prompt for the EMN category if they are using the tab view
	// (z.manning, 04/12/2007) - PLID 25600 - Moved this to within this if block so we only ask them if we aren't
	// actually loading from a template. Otherwise, the template may have a default category, so we wait and handle
	// that case in CEMN::LoadFromTemplateRecordsets().
	// (z.manning, 06/04/2007) - PLID 25701 - Moved this to after all the loading code so there's no longer a 
	// potentially long delay from the time a user selects a category to the time the EMN is done loading.
	if(pEMN->GetCategory().nID == -1) {
		// (z.manning, 08/17/2007) - PLID 27109 - The EMR tab view option is now a global preference.
		if (GetRemotePropertyInt("EMRTabView", 0, 0, "<None>", true) && GetRemotePropertyInt("PromptForEmnCategory", 1, 0, GetCurrentUserName(), true)) {
			CChooseEMNCategoryDlg dlg(this);
			// (z.manning, 06/04/2007) - PLID 26214 - Make sure we pass this EMN's chart ID to the category
			// selection dialog so it can filter only associated categories.
			dlg.m_nChartID = pEMN->GetChart();
			if(dlg.DoModal() == IDOK) {
				// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
				CEMNMoreInfoDlg *pdlgMoreInfo = GetMoreInfoDlg(pEMN, FALSE);
				// (z.manning, 06/04/2007) - PLID 25701 - If we have a more info dialog already, make sure to 
				// upate it as well as the EMN.
				if(pdlgMoreInfo) {
					pdlgMoreInfo->SetCategory(dlg.m_nEMNCategoryID);
				}
				pEMN->SetCategory(dlg.m_nEMNCategoryID, dlg.m_strEMNCategoryName);
			}
		}
	}

	return pEMN;
}

// (a.walling 2013-01-22 10:00) - PLID 54762 - Emr Appointment linking - auto-assign appointment or prompt to choose
CEMN* CEmrTreeWnd::AddEMNFromTemplateWithAppointmentPrompt(long nTemplateID, SourceActionInfo &sai)
{
	long nAppt = -1;

	// (j.jones 2013-06-13 10:51) - PLID 57129 - added preference to disable automatic appt. linking
	long nEMNAppointmentLinking = GetRemotePropertyInt("EMNAppointmentLinking", 1, 0, "<None>", true);
	//0 - do not link, 1 - auto-link (default)

	if(nEMNAppointmentLinking == 1) {
		COleDateTime dt = GetRemoteServerTime();
		std::vector<long> appts = FindEmnAppointments(m_EMR.GetPatientID(), dt);

		if (!appts.empty()) {
			if (appts.size() == 1) {
				nAppt = appts[0];
			} else {
				if (boost::optional<long> newAppt = PromptForEmnAppointment(this, m_EMR.GetPatientID(), dt)) {
					nAppt = *newAppt;
				}
			}
		}
	}

	return AddEMNFromTemplate(nTemplateID, sai, nAppt);
}

CEMN* CEmrTreeWnd::AddNewTemplateEMN(long nCollectionID)
{
	//Add the template EMN
	//Tell the EMR not to send us actions until we've finished adding the row.
	//(e.lally 2006-03-20) - The calls for locking and unlocking the spawning are probably not needed here 
		//as the Add New Template Emn function does it correctly now (PLID 19754), but it is too close to a release
		//to risk introducing other problems.
	m_EMR.LockSpawning();
	CEMN *pEMN = m_EMR.AddNewTemplateEMN(nCollectionID);
	m_EMR.UnlockSpawning();
	return pEMN;
}

void CEmrTreeWnd::HighlightActiveTopic(CEMRTopic* pTopic)
{
	//if (pTopic == EmrTree::ChildRow(m_pTree->CurSel).GetTopic()) {
	//	return;
	//}

	NXDATALIST2Lib::IRowSettingsPtr pRow = FindInTreeByColumn(TREE_COLUMN_OBJ_PTR, _variant_t(reinterpret_cast<long>(pTopic), VT_I4), NULL);
	//pTreeWnd->SetTreeSel(pRow);
	if (pRow == NULL) {
		// row was not found, but it may just not be found because it is a subtopic that has
		// not been added to the list yet. So we'll go up one level until we find a parent topic
		// that has a row, then ensure that row has loaded all subtopics.

		NXDATALIST2Lib::IRowSettingsPtr pHigherRow;

		CEMRTopic* pParentTopic = pTopic->GetParentTopic();
		// this loops until pHigherRow is not null, meaning we found the row in the datalist, or until we run out of rows
		while ( (pHigherRow == NULL) && (pParentTopic) ) {
			pHigherRow = FindInTreeByColumn(TREE_COLUMN_OBJ_PTR, _variant_t(reinterpret_cast<long>(pParentTopic), VT_I4), NULL); // don't autoselect

			if (pHigherRow) {
				EnsureTopicRowLoadedAllSubTopics(pHigherRow);
			}

			// (a.walling 2007-05-18 17:52) - PLID 25548 - Need to walk up the tree!
			pParentTopic = pParentTopic->GetParentTopic();
		}

		if (!pHigherRow) {
			pHigherRow = FindInTreeByColumn(TREE_COLUMN_OBJ_PTR, (long)pTopic->GetParentEMN(), NULL);

			if (pHigherRow) {
				EnsureTopicRowLoadedAllSubTopics(pHigherRow);
			}
		}

		if (pHigherRow) {
			// this means we ensured the row was loaded, and all its subtopics, so try finding the topic again
			pRow = FindInTreeByColumn(TREE_COLUMN_OBJ_PTR, _variant_t(reinterpret_cast<long>(pTopic), VT_I4), NULL);
			//pTreeWnd->SetTreeSel(pRow);
		}
	}

	// (j.jones 2007-07-06 11:50) - PLID 25457 - ensure the row is created
	// (a.walling 2007-10-10 10:27) - PLID 25548 - Only set the sel if we have a valid row
	if(pRow) {
		EnsureTopicRow(pRow);
		if (m_pTree->CurSel != pRow) {
			m_pTree->CurSel = pRow; // avoid firing the sel changing/chosen/etc
		}

		// (a.walling 2012-03-07 08:36) - PLID 48680 - Scroll the preview pane, if the preview ctrl is not currently handling input
		if (IsAutoScroll()) {
			if (CEMRPreviewCtrlDlg* pPreviewCtrl = GetEmrPreviewCtrl()) {
				CWnd* pFocus = CWnd::GetFocus();
				if (!pFocus || !pFocus->IsChild(pPreviewCtrl)) {
					pPreviewCtrl->ScrollToTopic(EmrTree::ChildRow(m_pTree->CurSel).GetTopic());
				}
			}
		}
	}
}

// (a.walling 2012-07-03 10:56) - PLID 51284 - Fixing activation ambiguities - Highlight active EMN
void CEmrTreeWnd::HighlightActiveEMN(CEMN* pEMN)
{
	if (!m_pTree) return;

	if (pEMN == EmrTree::ChildRow(m_pTree->CurSel).GetEMN()) {
		return;
	}

	CEMN* pPreviouslyActiveEMN = GetActiveEMN();

	m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, reinterpret_cast<long>(pEMN), NULL, VARIANT_TRUE);
}

long CEmrTreeWnd::GetPatientID()
{
	return m_EMR.GetPatientID();
}

// (j.jones 2007-07-26 09:23) - PLID 24686 - this is a horrible idea that should never occur
/*
void CEmrTreeWnd::RefreshAllItems() 
{
	m_EMR.RefreshAllItems();
}
*/

// (j.jones 2007-07-26 09:10) - PLID 24686 - converted RefreshContent into two functions,
// accepting an InfoID or a MasterID
void CEmrTreeWnd::RefreshContentByInfoID(long nEMRInfoID, BOOL bSetRegionAndInvalidateDetails)
{
	m_EMR.RefreshContentByInfoID(nEMRInfoID);

	//if a topic is displayed, refresh that topic
	NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pTree->CurSel;
	if(pCurSel) {
		EmrTreeRowType etrt = EmrTree::ChildRow(pCurSel).GetType();
		if(etrt == etrtTopic) {
			CEmrTopicWndPtr pWnd = EmrTree::ChildRow(pCurSel).GetTopicWnd();
			if(pWnd)
				pWnd->RepositionDetailsInTopicByInfoID(nEMRInfoID, bSetRegionAndInvalidateDetails);
		}
	}
}

// (j.jones 2007-07-26 09:10) - PLID 24686 - converted RefreshContent into two functions,
// accepting an InfoID or a MasterID
void CEmrTreeWnd::RefreshContentByInfoMasterID(long nEMRInfoMasterID, BOOL bSetRegionAndInvalidateDetails)
{
	m_EMR.RefreshContentByInfoMasterID(nEMRInfoMasterID);

	//if a topic is displayed, refresh that topic
	NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pTree->CurSel;
	if(pCurSel) {
		EmrTreeRowType etrt = EmrTree::ChildRow(pCurSel).GetType();
		if(etrt == etrtTopic) {
			CEmrTopicWndPtr pWnd = EmrTree::ChildRow(pCurSel).GetTopicWnd();
			if(pWnd)
				pWnd->RepositionDetailsInTopicByInfoMasterID(nEMRInfoMasterID, bSetRegionAndInvalidateDetails);
		}
	}
}

// (a.wetta 2007-04-09 13:30) - PLID 25532 - This function refreshes the content
// all all EMR items of a certain type.
// (a.walling 2008-12-19 09:21) - PLID 29800 - This was only used for images, and only to refresh the custom stamps, which was causing the content
// to be reloaded. This is all unnecessary, and the custom stamps is entirely UI. So let's just do what we need to do, and refresh the custom stamps,
// rather than flag as needed to reload content. This is all controlled by the new bRefreshCustomStampsOnly param. I could have renamed the function
// entirely, but I can see how this might come in handy in the future.
void CEmrTreeWnd::RefreshContentByType(EmrInfoType eitItemType, BOOL bSetRegionAndInvalidateDetails, BOOL bMaintainImagesSize, BOOL bRefreshCustomStampsOnly)
{
	m_EMR.RefreshContentByType(eitItemType, 0, bRefreshCustomStampsOnly);

	if (!bRefreshCustomStampsOnly) {
		//if a topic is displayed, refresh that topic
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pTree->CurSel;
		if(pCurSel) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pCurSel).GetType();
			if(etrt == etrtTopic) {
				CEmrTopicWndPtr pWnd = EmrTree::ChildRow(pCurSel).GetTopicWnd();
				if(pWnd) {
					// (j.jones 2007-07-26 09:31) - PLID 24686 - renamed this function
					pWnd->RepositionDetailsInTopicByInfoID(-1, bSetRegionAndInvalidateDetails, NULL, NULL, bMaintainImagesSize);
				}
			}
		}
	}
}

void CEmrTreeWnd::OnSelChangedTree(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		HandleSelChanged(lpOldSel, lpNewSel);
	} NxCatchAll("Error in OnSelChangedTree");
}

//TES 4/8/2009 - PLID 33376 - Moved these macros to GlobalUtils.h
/*#define FOR_ALL_ROWS_WITH_PARENT(datalist, parentrow)	NXDATALIST2Lib::IRowSettingsPtr pRow = parentrow->GetFirstChildRow();\
	CArray<LPDISPATCH,LPDISPATCH> arParentRows;\
	while(pRow) 

#define FOR_ALL_ROWS(datalist)	NXDATALIST2Lib::IRowSettingsPtr pRow = datalist->GetFirstRow();\
	CArray<LPDISPATCH,LPDISPATCH> arParentRows;\
	while(pRow) 

#define GET_NEXT_ROW(datalist)	if(pRow->GetFirstChildRow()) {\
		arParentRows.Add(pRow);\
		pRow = pRow->GetFirstChildRow();\
	}\
	else {\
		pRow = pRow->GetNextRow();\
		while(pRow == NULL && arParentRows.GetSize()) {\
			pRow = arParentRows.GetAt(arParentRows.GetSize()-1);\
			arParentRows.RemoveAt(arParentRows.GetSize()-1);\
			pRow = pRow->GetNextRow();\
		}\
	}\*/

void CEmrTreeWnd::OnDestroy()
{
	try {
		{
			// (a.walling 2008-05-14 18:01) - PLID 29114 - Clear any pending detail references
			POSITION pos = m_mapDetailsPendingUpdate.GetStartPosition();
			while (pos) {
				CEMNDetail* pDetail;
				BOOL bDummy;

				m_mapDetailsPendingUpdate.GetNextAssoc(pos, pDetail, bDummy);

				ASSERT(pDetail);
				if (pDetail) {
					//pDetail->Release();
					// (a.walling 2009-10-12 16:05) - PLID 36024
					pDetail->__Release("CEmrTreeWnd::OnDestroy pending updates");
				}
			}
		}

		// (c.haag 2007-08-30 16:34) - PLID 27058 - Destroy the E/M Checklist Setup window
		if (IsWindow(m_dlgEMChecklistSetup.GetSafeHwnd())) {
			m_dlgEMChecklistSetup.DestroyWindow();
		}

		// (a.walling 2008-06-11 10:59) - PLID 22049 - Destroy any dangling modeless windows
		POSITION pos = m_mapModelessSelfDestructWindows.GetStartPosition();

		while (pos) {
			CWnd* pModelessWnd = NULL;
			long nID = -1;

			m_mapModelessSelfDestructWindows.GetNextAssoc(pos, nID, pModelessWnd);

			if (::IsWindow(pModelessWnd->GetSafeHwnd())) {
				::DestroyWindow(pModelessWnd->GetSafeHwnd());
				// (a.walling 2010-03-29 15:54) - PLID 34289 - These windows self-destruct
				//delete pModelessWnd;
			}
		}

		// (a.walling 2008-07-03 15:07) - PLID 30498 - Destroy any dangling modeless windows again!
		pos = m_mapModelessWaitingWindows.GetStartPosition();

		while (pos) {
			CWnd* pModelessWnd = NULL;
			long nID = -1;

			m_mapModelessWaitingWindows.GetNextAssoc(pos, nID, pModelessWnd);

			if (::IsWindow(pModelessWnd->GetSafeHwnd())) {
				::DestroyWindow(pModelessWnd->GetSafeHwnd());
				delete pModelessWnd;
			}
		}
	} NxCatchAll("Error in OnDestroy");

	// (a.walling 2012-01-26 13:24) - PLID 47814 - Need to call base class when handling OnDestroy!
	__super::OnDestroy();
}

// (j.jones 2007-04-03 14:54) - PLID 25464 - needed for the splitter bar
int Round(double d)
{
	// (a.walling 2009-10-19 17:35) - PLID 36002 - Pedantic, I know
	return int((d > 0) ? (d + 0.5f) : (d - 0.5f));
}

void CEmrTreeWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
			
	if (m_wndTree.GetSafeHwnd()) {
		CRect rcClient;
		GetClientRect(&rcClient);
		m_wndTree.MoveWindow(rcClient);
	}
}

// (b.cardillo 2012-03-06 14:56) - PLID 48647 - Added bForceRecalculateCompletionStatus to cause the topic to store a new completion status based on the results of this operation.
BOOL CEmrTreeWnd::EnsureTopicBackColor(CEMRTopic *pTopic, CEMNDetail *pDetailToIgnore /*= NULL*/, BOOL bHideIfEmpty /*= TRUE*/, BOOL bForceRecalculateCompletionStatus /*= FALSE*/)
{
	try {
		// (c.haag 2007-06-27 16:18) - PLID 26213 - For safety purposes. I wonder why this wasn't
		// in here earlier!
		if (NULL == pTopic)
			return FALSE;

		//First, since this topic has changed, any parent topics may also need to change.
		if(pTopic->GetParentTopic()) {
			// (b.cardillo 2012-03-06 14:56) - PLID 48647 - Ability to recalculate completion status, not just set color according to existing status
			EnsureTopicBackColor(pTopic->GetParentTopic(), pDetailToIgnore, bHideIfEmpty, bForceRecalculateCompletionStatus);
		}

		// (c.haag 2007-08-04 08:54) - PLID 26213 - Fail if this topic is not finished loading.
		// For every topic that finishes loading, there should inevitably be a call to
		// EnsureTopicBackColor. That call is done by legacy code outside this function and outside
		// this punch list item.
		if (!pTopic->IsLoaded()) {
			if (pTopic->GetParentEMN() && !pTopic->GetParentEMN()->IsInitialLoadComplete()) {
				return FALSE;
			}
		}

		// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
		COLORREF cPartial = EmrColors::Topic::Partial();
		COLORREF cFull = EmrColors::Topic::Complete();
		// (j.jones 2007-06-15 09:30) - PLID 26297 - added locked color preference
		COLORREF cLocked = EmrColors::Topic::Locked();
		// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details default color
		COLORREF cRequired = EmrColors::Topic::Required();

		//find the topic in the list, and color the list row and the topic

		BOOL bFound = FALSE;

		NXDATALIST2Lib::IRowSettingsPtr pRowToColor = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pTopic, NULL, FALSE);

		// (b.cardillo 2012-03-06 14:56) - PLID 48647 - Ability to recalculate completion status, not just set color according to existing status
		EmrTopicCompletionStatus etcs = pTopic->GetCompletionStatus(pDetailToIgnore, bForceRecalculateCompletionStatus);

		if(pRowToColor == NULL) {
			// (b.cardillo 2006-11-20 10:37) - PLID 22565 - If we are a direct child of the EMN, and 
			// we have reconstructed details, then even though we're not coloring OUR OWN row in the 
			// tree (because it doesn't exist, i.e. pRowToColor == NULL) we still have the 
			// responsibility of coloring the EMN's row.
			CEMN *pParentEMN = pTopic->GetParentEMN();
			if (pParentEMN && pParentEMN->GetStatus() == 2 && pTopic->GetParentTopic() == NULL && pTopic->HasReconstructedDetails()) {
				NXDATALIST2Lib::IRowSettingsPtr pParentEMNRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pParentEMN, NULL, FALSE);
				if (pParentEMNRow) {
					// Okay, we're good to go, decide on the color
					pParentEMNRow->PutBackColor(GetReconstructedEMRDetailReviewStateColor(-1));
				}
			}

			//The row hasn't been created yet, that was easy.
			return etcs == etcsComplete;
		}

		//TES 7/21/2004: If we're on a template, no topic coloring allowed.  However, we DO want any checked off items (default items)
		//to be colored, for ease of reading...
		if(m_bIsTemplate) {
			//TES 1/18/2006 - If this topic will be hidden on the EMN, color the text gray.
			// (a.walling 2012-07-09 12:35) - PLID 51441 - IsEmpty now has some non-optional parameters
			if(pTopic->IsEmpty(pDetailToIgnore, FALSE) && !pTopic->ShowIfEmpty(FALSE)) {
				pRowToColor->PutForeColor(RGB(127,127,127));
			}
			else {
				pRowToColor->PutForeColor(RGB(0,0,0));
			}

			//If this topic won't be showing on EMNs, color it to indicate that.
			BOOL bHideOnEMN = pTopic->HideOnEMN();
			CEMRTopic *pParent = pTopic->GetParentTopic();
			while(!bHideOnEMN && pParent) {
				if(pParent->HideOnEMN()) bHideOnEMN = TRUE;
				pParent = pParent->GetParentTopic();
			}
			if(bHideOnEMN) {
				if (!m_bDragging || !m_bIsTemplate) {
					pRowToColor->PutBackColor(HIDDEN_COLOR);
				} else {
					// (a.walling 2007-05-18 11:16) - PLID 25092 - Save the color in the map to be applied when
					// dragging is complete.
					IRowSettings* pDeferRow = pRowToColor;
					OLE_COLOR oc;
					if (!m_mapTopicColors.Lookup(pDeferRow, oc))
						pDeferRow->AddRef();
					m_mapTopicColors[pDeferRow] = HIDDEN_COLOR;
				}
				pRowToColor->PutForeColor(RGB(127,127,127));
			}
			else {
				if (!m_bDragging || !m_bIsTemplate) {
					pRowToColor->PutBackColor(m_pTree->GetNewRow()->GetBackColor());
				} else {
					// (a.walling 2007-05-18 11:16) - PLID 25092 - Save the color in the map to be applied when
					// dragging is complete.
					IRowSettings* pDeferRow = pRowToColor;
					OLE_COLOR oc;
					if (!m_mapTopicColors.Lookup(pDeferRow, oc))
						pDeferRow->AddRef();
					m_mapTopicColors[pDeferRow] = m_pTree->GetNewRow()->GetBackColor();
				}
				pRowToColor->PutForeColor(RGB(0,0,0));
			}

			CEmrTopicWndPtr pTopicWnd = EmrTree::ChildRow(pRowToColor).GetTopicWnd();
			if(pTopicWnd) {
				pTopicWnd->SetHighlightColor(cPartial);
			}
			return etcs == etcsComplete;
		}
		else {
			//TES 6/9/2006 - If this topic only exists to hold spawned topics, hide it.
			// (z.manning, 03/05/2007) - PLID 24529 - Don't show topics that have only blank subtopics.
			// (a.walling 2007-09-17 15:56) - PLID 25599 - Only hide topics if the bHideIfEmpty is set.
			if(bHideIfEmpty && pTopic->IsEmpty(pDetailToIgnore,TRUE) && !pTopic->ShowIfEmpty(FALSE)) {
				// (c.haag 2007-09-27 08:49) - PLID 27509 - If pRowToColor is the current tree selection,
				// then the datalist will change its internal selection. We need to be cogniscent of that
				// and do more than just setting the visible flag. Set Tree Row Visible will do the work for us.
				SetTreeRowVisible(pRowToColor, FALSE);
				// (a.walling 2007-09-21 09:44) - PLID 25549 - Ensure the preview control's topic visibility
				// is up to date
				if (GetEmrPreviewCtrl()) {
					GetEmrPreviewCtrl()->ShowTopic(pTopic, FALSE);
				}
			}
			else {
				SetTreeRowVisible(pRowToColor, TRUE);
			}
		}
		
		//First off, is this topic locked?
		BOOL bLocked = (pTopic->GetParentEMN()->GetStatus() == 2);
		if(bLocked) {
			// (b.cardillo 2006-11-20 10:37) - PLID 22565 - We normally would do nothing more here than set the 
			// background color to LOCKED_COLOR, but since (for this pl item) we have to deal with the possibility 
			// of reconstructed data, we instead check the current background color, search this topic to see if 
			// it contains any reconstructed details, decide the color based on the answer, and then if that color 
			// is different from the current color, we set the new color and then also set the parent EMN if it 
			// happens that we are a direct child of the parent.
			COLORREF clrExistingTopicColor = pRowToColor->GetBackColor();
			COLORREF clrNewTopicColor;
			BOOL bReconstructed = FALSE;
			if (pTopic->HasReconstructedDetails()) {
				clrNewTopicColor = GetReconstructedEMRDetailReviewStateColor(-1);
				bReconstructed = TRUE;
			} else {
				clrNewTopicColor = cLocked;
			}
			if (clrNewTopicColor != clrExistingTopicColor) {
				// Set our color appropriately
				pRowToColor->PutBackColor(clrNewTopicColor);
				// See if our direct parent is the EMN itself
				if (pTopic->GetParentTopic() == NULL) {
					// It is, so it is our responsibility to make sure it's colored properly as well.  First make 
					// sure we can get the row object from the tree.
					CEMN *pParentEMN = pTopic->GetParentEMN();
					if (pParentEMN) {
						NXDATALIST2Lib::IRowSettingsPtr pParentEMNRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pParentEMN, NULL, FALSE);
						if (pParentEMNRow) {
							// Okay, we're good to go, decide on the color
							COLORREF clrEMNColor;
							if (bReconstructed) {
								// We're colored "reconstructed" so we know our parent EMN needs to be too
								clrEMNColor = clrNewTopicColor;
							} else {
								// We're no longer "reconstructed" but other children of the EMN might be; to 
								// find out I'm afraid we'll have to search!
								clrEMNColor = NXDATALIST2Lib::dlColorNotSet;
								for (long i=0, nCount=pParentEMN->GetTopicCount(); i<nCount; i++) {
									CEMRTopic* pSiblingTopic = pParentEMN->GetTopic(i);
									if (pSiblingTopic && pSiblingTopic != pTopic && pSiblingTopic->HasReconstructedDetails()) {
										clrEMNColor = GetReconstructedEMRDetailReviewStateColor(-1);;
										break;
									}
								}
							}
							// No we know the right color, set the EMN to be that color
							pParentEMNRow->PutBackColor(clrEMNColor);
						}
					}
				}
			}
		}

		switch(etcs) {
		case etcsPartiallyComplete:
			{
				if(!bLocked)
					pRowToColor->PutBackColor(cPartial);
				CEmrTopicWndPtr pTopicWnd = EmrTree::ChildRow(pRowToColor).GetTopicWnd();
				if(pTopicWnd) {
					pTopicWnd->SetHighlightColor(cPartial);
				}
			}
			break;
		case etcsComplete:
			{
				if(!bLocked)
					pRowToColor->PutBackColor(cFull);
				CEmrTopicWndPtr pTopicWnd = EmrTree::ChildRow(pRowToColor).GetTopicWnd();
				if(pTopicWnd) {
					pTopicWnd->SetHighlightColor(cFull);
				}
			}
			break;
		// (j.jones 2007-06-14 16:07) - PLID 26276 - ensured that we handle the reconstructed color
		case etcsReconstructed:
			{
				if(!bLocked)
					pRowToColor->PutBackColor(GetReconstructedEMRDetailReviewStateColor(-1));
				CEmrTopicWndPtr pTopicWnd = EmrTree::ChildRow(pRowToColor).GetTopicWnd();
				if(pTopicWnd) {
					pTopicWnd->SetHighlightColor(GetReconstructedEMRDetailReviewStateColor(-1));
				}
			}
			break;
			// (z.manning, 04/04/2008) - PLID 29495 - Added status for when topic has no details.
		case etcsBlank:
		case etcsIncomplete:
			{
				if(!bLocked)
					pRowToColor->PutBackColor(m_pTree->GetNewRow()->GetBackColor());
			}
			break;
		// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
		case etcsRequired:
			{
				if (!bLocked) {
					pRowToColor->PutBackColor(cRequired);
				}
				CEmrTopicWndPtr pTopicWnd = EmrTree::ChildRow(pRowToColor).GetTopicWnd();
				if(pTopicWnd) {
					pTopicWnd->SetHighlightColor(cRequired);
				}
			}
			break;
		}

		return etcs == etcsComplete;
	
	}NxCatchAll("Error in CEmrTreeWnd::EnsureTopicBackColor()");

	// Return TRUE if the topic contains all non-empty details, FALSE if any detail is empty
	return FALSE;
}

// (b.cardillo 2012-03-06 14:56) - PLID 48647 - Ability to recalculate completion status, not just set color according to existing status
void CEmrTreeWnd::EnsureAllTopicBackColors(BOOL bForceRecalculateCompletionStatus)
{
	for(int i = 0; i < m_EMR.GetEMNCount(); i++) {
		CArray<CEMRTopic*,CEMRTopic*> arTopics;
		m_EMR.GetEMN(i)->GetAllTopics(arTopics);
		for(int j = 0; j < arTopics.GetSize(); j++) {
			// (b.cardillo 2012-03-06 14:56) - PLID 48647 - Ability to recalculate completion status, not just set color according to existing status
			EnsureTopicBackColor(arTopics[j], NULL, TRUE, bForceRecalculateCompletionStatus);
		}
	}
}

void CEmrTreeWnd::OnRowExpandedTree(LPDISPATCH lpRow)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
		if(etrt == etrtEmn || etrt == etrtTopic) {
			LoadSubtopics(pRow);
		}
		EnsureModifiedState(pRow);
	} NxCatchAll("Error in OnRowExpandedTree");
}

void CEmrTreeWnd::OnRowCollapsedTree(LPDISPATCH lpRow)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		EnsureModifiedState(pRow);
	} NxCatchAll("Error in OnRowCollapsedTree");
}

// (a.walling 2008-06-11 09:08) - PLID 30351 - Decrease the buffer
//#define HORIZ_BUFFER	10
//#define VERT_BUFFER		10
#define HORIZ_BUFFER	5
#define VERT_BUFFER		5
#define MAX_TREE_WIDTH	200
#define ARROW_SIZE	35

NXDATALIST2Lib::IRowSettingsPtr CEmrTreeWnd::CalcNextRow(NXDATALIST2Lib::IRowSettingsPtr pCurRow)
{
	if(pCurRow == NULL) {
		if(m_pTree->GetRowCount()) {
			return CalcNextRow(m_pTree->GetFirstRow());
		}
		else {
			return NULL;
		}
	}

	//First, get the first child of the current row.  If there are no children, go the next sibling of the current row.
	//If there are no more siblings, go to our parent's next sibling, then grandparent's, etc.
	
	//Before calculating the child, we need to check whether we've loaded our subtopics (otherwise our first child will just
	//be a placeholder).
	EmrTreeRowType etrt = EmrTree::ChildRow(pCurRow).GetType();
	if((etrt == etrtEmn || etrt == etrtTopic) && !EmrTree::ChildRow(pCurRow).IsLoaded()) {
		LoadSubtopics(pCurRow);
	}

	//First child.
	NXDATALIST2Lib::IRowSettingsPtr pRowToReturn = NULL; // (d.moore 2007-08-30) - PLID 27238 - Could previously have been returned without being initialized.
	NXDATALIST2Lib::IRowSettingsPtr pRow = pCurRow->GetFirstChildRow();
	if(pRow == NULL) {
		//Next sibling.
		pRow = pCurRow->GetNextRow();
		if(pRow == NULL) {
			NXDATALIST2Lib::IRowSettingsPtr pParentRow = pCurRow->GetParentRow();
			while(pParentRow != NULL && pRow == NULL) {
				pRow = pParentRow->GetNextRow();
				pParentRow = pParentRow->GetParentRow();
			}
		}
	}
	if(pRow) {
		//Is this a valid row for selecting?
		if(!pRow->Visible) {
			//That's no good.
			pRowToReturn = CalcNextRow(pRow);
		}
		else {
			EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
			if(etrt == etrtEmn) {
				//Nope, so keep going.
				if(!EmrTree::ChildRow(pRow).IsLoaded()) {
					LoadSubtopics(pRow);
				}
				pRowToReturn = CalcNextRow(pRow);
			}
			else if(etrt == etrtTopic) {
				//If this topic is just a parent topic with no details, don't let them select it.
				CEMRTopic *pTopic = EmrTree::ChildRow(pRow).GetTopic();
				if(pTopic->HasSubTopics() && !pTopic->HasDetails()) {
					//Nope, keep going.
					if(!EmrTree::ChildRow(pRow).IsLoaded()) {
						LoadSubtopics(pRow);
					}
					pRowToReturn = CalcNextRow(pRow);
				}
				else {
					//Yup, return it.
					pRowToReturn = pRow;
				}
			}
			// (a.walling 2012-06-28 17:11) - PLID 51276 - More Info should be a clickable link
			//TES 2/12/2014 - PLID 60750 - Added Codes
			else if(etrt == etrtMoreInfo || etrt == etrtCodes) {
				pRowToReturn = pRow->GetParentRow();
			}
			else {
				pRowToReturn = pRow;
			}
		}
	}
	return pRowToReturn;
}

NXDATALIST2Lib::IRowSettingsPtr CEmrTreeWnd::GetAbsoluteLastChildRow(NXDATALIST2Lib::IRowSettingsPtr pCurRow)
{
	IRowSettingsPtr pRow = pCurRow;
	//Do we need to load the subtopics?
	EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
	if((etrt == etrtEmn || etrt == etrtTopic) && !EmrTree::ChildRow(pRow).IsLoaded()) {
		LoadSubtopics(pRow);
	}
	IRowSettingsPtr pChild = pRow->GetLastChildRow();
	while(pChild) {
		pRow = pChild;
		//Do we need to load the subtopics?
		EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
		if((etrt == etrtEmn || etrt == etrtTopic) && !EmrTree::ChildRow(pRow).IsLoaded()) {
			LoadSubtopics(pRow);
		}
		pChild = pRow->GetLastChildRow();
	}
	return pRow;
}

NXDATALIST2Lib::IRowSettingsPtr CEmrTreeWnd::CalcPreviousRow(NXDATALIST2Lib::IRowSettingsPtr pCurRow)
{
	IRowSettingsPtr pRow;
	if(pCurRow == NULL) {
		if(m_pTree->GetRowCount()) {
			pRow = GetAbsoluteLastChildRow(m_pTree->GetLastRow());
		}
		else {
			return NULL;
		}
	}
	else {
		//If we have a previous sibling, get that, and then find its last child (all the way down).
		//Otherwise, just get our parent.
		pRow = pCurRow->GetPreviousRow();
		if(pRow) {
			pRow = GetAbsoluteLastChildRow(pRow);
		}
		else {
			pRow = pCurRow->GetParentRow();
		}
	}
	//OK, we've got the previous row as far as the tree is concerned, but is it valid?
	IRowSettingsPtr pRowToReturn = pRow;
	if(pRow) {
		//Is this a valid row for selecting?
		if(!pRow->Visible) {
			//That's no good.
			pRowToReturn = CalcPreviousRow(pRow);
		}
		else {
			EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
			if(etrt == etrtEmn) {
				//Nope, keep going.
				pRowToReturn = CalcPreviousRow(pRow);
			}
			else if(etrt == etrtTopic) {
				//If this topic is just a parent topic with no details, don't let them select it.
				CEMRTopic *pTopic = EmrTree::ChildRow(pRow).GetTopic();
				//DRT 8/21/2007 - PLID 26655 - Must check for NULL.  If we get no topic, then we want to keep
				//	calculating the previous row.
				if(pTopic == NULL || (pTopic->HasSubTopics() && !pTopic->HasDetails())) {
					//Nope, keep going.
					pRowToReturn = CalcPreviousRow(pRow);
				}
			}
		}
	}
	return pRowToReturn;

}

NXDATALIST2Lib::IRowSettingsPtr CEmrTreeWnd::GetNextVisibleRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	// (c.haag 2007-10-11 15:46) - PLID 27509 - Returns the next visible row in the list
	if (NULL == pRow) {
		return NULL;
	}

	// Seek to the next visible row at this tree level
	IRowSettingsPtr p = pRow;
	do {
		p = p->GetNextRow();
	} while (NULL != p && VARIANT_FALSE == p->GetVisible());

	if (NULL == p) {
		// If p is NULL, we'll have to repeat this search up to the parent level
		return GetNextVisibleRow(pRow->GetParentRow());

	} else {
		// Success!
		return p;
	}
}

NXDATALIST2Lib::IRowSettingsPtr CEmrTreeWnd::GetPrevVisibleRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	// (c.haag 2007-10-11 16:07) - PLID 27509 - Returns the previous visible row in the list.
	if (NULL == pRow) {
		return NULL;
	}

	// Seek to the previous visible row at this tree level
	IRowSettingsPtr p = pRow;
	do {
		p = p->GetPreviousRow();
	} while (NULL != p && VARIANT_FALSE == p->GetVisible());

	if (NULL == p) {
		// If p is NULL, we're stuck. The next candidate is pRow's parent.
		// If the parent itself is NULL, then we're done. Otherwise, if
		// the parent is visible, then it's the candidate we want. If it's
		// invisible, then repeat our methods at the parent's tree level.

		p = pRow->GetParentRow();
		if (NULL != p) {
			
			if (VARIANT_FALSE != p->GetVisible()) {

				// If it's visible, then the parent will do just fine
				return p;
			} else {

				// If it's invisible, repeat the test on the parent level
				return GetPrevVisibleRow(p);
			}

		} else {
			// p's parent is NULL. Nothing we can do
			return NULL;
		}

		return GetPrevVisibleRow(pRow->GetParentRow());

	} else {
		// Success!
		return p;
	}
}

// (a.walling 2007-10-15 16:10) - PLID 27664 - added flag to suppress saving the preview for topic level saves
void CEmrTreeWnd::SaveRowObject(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bSilent, BOOL bSuppressPreviewSave /* = FALSE */)
{
	//DRT 8/23/2007 - PLID 27171 - This function needs to wait until the initial load is complete -- we can't start saving 
	//	until that is done!  I added a wait to each type of row.  If we add a new type in the future, make sure it waits on
	//	the load appropriately.

	//save the specified topic
	if(pRow) {
		EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
		if(etrt == etrtTopic) {
			CEMRTopic *pTopic = EmrTree::ChildRow(pRow).GetTopic();
			if(pTopic) {
				CEMN *pEMN = pTopic->GetParentEMN();

				// (a.walling 2008-06-09 13:02) - PLID 22049 - Don't save if we are not writable
				if (!pEMN->IsWritable())
					return;

				// (a.walling 2009-11-11 15:53) - PLID 36274 - Keep track of # of objects saved
				long nSavedObjects = 0;

				// (a.walling 2007-08-31 15:14) - PLID 24733 - If this is the initial save, force everything to save
				// unless the preference is off.
				// (a.walling 2007-09-04 09:13) - PLID 24733 - Don't do anything special if this is a template
				if (m_bEMNCompleteInitialSave && pEMN && (!pEMN->IsTemplate()) && (pEMN->GetID() == -1)) {
					// Save the entire EMN.
					pEMN->EnsureCompletelyLoaded();
					
					// (j.jones 2009-06-17 12:56) - PLID 34652 - make sure we check to see if the save failed
					if(FAILED(SaveEMR(esotEMN, (long)pEMN, TRUE))) {
						//return silently
						return;
					} else {
						nSavedObjects++;
					}

				} else if(pTopic->IsUnsaved(TRUE)) {
					//DRT 8/23/2007 - PLID 27171 - Must wait for the load to finish
					{
						if(pEMN) {
							pEMN->EnsureCompletelyLoaded();
						}
						else {
							//no EMN?!?!  We can't save.  This should never happen.
							ASSERT(FALSE);
							return;
						}
					}

					//warn the user if we will need to save other topics
					CArray<CEMRTopic*,CEMRTopic*> aryEMRTopicsToSave;
					//this will calculate if we need to save topics in a given order,
					//and populates the array with that desired saving order
					if(pTopic->CalculateTopicSavingOrderNeeded(aryEMRTopicsToSave)) {
						CString strWarning;
						CString strTopics;

						//filter this list down to only non-deleted topics
						CArray<CEMRTopic*,CEMRTopic*> aryActiveTopics;

						for(int i=0;i<aryEMRTopicsToSave.GetSize();i++) {
							NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)aryEMRTopicsToSave.GetAt(i), NULL, FALSE);
							if(pRow) {
								ASSERT(etrtTopic == EmrTree::ChildRow(pRow).GetType());
								CEMRTopic *pActiveTopic = EmrTree::ChildRow(pRow).GetTopic();
								if(pActiveTopic) {
									aryActiveTopics.Add(pActiveTopic);										
								}
							}
						}

						if(aryActiveTopics.GetSize() > 1 || 
							(aryActiveTopics.GetSize() == 1 && aryActiveTopics.GetAt(0) != pTopic)) {

							for(int i=0;i<aryActiveTopics.GetSize();i++) {
								strTopics += aryActiveTopics.GetAt(i)->GetName();
								strTopics += "\n";
							}

							if(!bSilent) {
								strWarning.Format("Saving this topic will require the saving of each of the following topics and their subtopics (this may be because narratives\n"
									"or tables link to or from these topics, or because details were dragged to or from these topics):\n\n"
									"%s\n"
									"Are you sure you wish to save these topics now?", strTopics);

								if(IDNO == MessageBox(strWarning, "Practice", MB_ICONQUESTION|MB_YESNO)) {
									return;
								}
							}
							
							//save the topics

							for(i=0;i<aryActiveTopics.GetSize();i++) {

								// (j.jones 2009-06-17 12:56) - PLID 34652 - make sure we check to see if the save failed
								if(FAILED(SaveEMR(esotTopic, (long)aryActiveTopics.GetAt(i), TRUE))) {
									//return silently
									return;
								} else {
									nSavedObjects++;
								}
							}							

							// (a.walling 2007-10-15 16:12) - PLID 27664 - We are saving a topic-level item, so update the preview
							// don't refresh, just save to documents and ignore unsaved.
							// (a.walling 2009-11-11 15:53) - PLID 36274 - Only generate the HTML file and MHT archive if something actually saved
							// (a.walling 2009-12-03 09:54) - PLID 36274 - We were skipping this step if we were saving the dependent topics.
							// So make sure to update the preview file as well, assuming we have not been told to suppress and assuming something saved.
							if (!bSuppressPreviewSave && nSavedObjects > 0) {
								if (pEMN) {
									pEMN->GenerateHTMLFile(FALSE, TRUE, TRUE);
								}
							}

							return;
						}
					}

					// (j.jones 2009-06-17 12:56) - PLID 34652 - make sure we check to see if the save failed
					if(FAILED(SaveEMR(esotTopic, (long)pTopic, TRUE))) {
						//return silently
						return;
					} else {
						nSavedObjects++;
					}
				}

				// (a.walling 2007-10-15 16:12) - PLID 27664 - We are saving a topic-level item, so update the preview
				// don't refresh, just save to documents and ignore unsaved.
				// (a.walling 2009-11-11 15:53) - PLID 36274 - Only generate the HTML file and MHT archive if something actually saved
				if (!bSuppressPreviewSave && nSavedObjects > 0) {
					if (pEMN) {
						pEMN->GenerateHTMLFile(FALSE, TRUE, TRUE);
					}
				}
			}
		}
		else if(etrt == etrtEmn) {
			CEMN *pEMN = EmrTree::ChildRow(pRow).GetEMN();
			if(pEMN) {
				// (a.walling 2008-06-09 13:02) - PLID 22049 - Don't save if we are not writable
				if (!pEMN->IsWritable())
					return;

				if(pEMN->IsUnsaved()) {
					//DRT 8/23/2007 - PLID 27171 - Must wait for the load to finish
					pEMN->EnsureCompletelyLoaded();

					// (j.jones 2009-06-17 12:56) - PLID 34652 - make sure we check to see if the save failed
					if(FAILED(SaveEMR(esotEMN, (long)pEMN, TRUE))) {
						//return silently
						return;
					}
				}
			}
		}
		//TES 2/12/2014 - PLID 60748 - Added new Codes topic, behaves the same way
		else if(etrt == etrtMoreInfo || etrt == etrtCodes) {

			//find the parent EMN and save it
			CEMN *pEMN = NULL;
			NXDATALIST2Lib::IRowSettingsPtr pEMNRow = pRow->GetParentRow();
			while(pEMNRow->GetParentRow()) {
				pEMNRow = pEMNRow->GetParentRow();
			}

			pEMN = EmrTree::ChildRow(pEMNRow).GetEMN();
			if(pEMN) {
				// (a.walling 2008-06-09 13:02) - PLID 22049 - Don't save if we are not writable
				if (!pEMN->IsWritable())
					return;

				if(pEMN->IsUnsaved()) {
					//DRT 8/23/2007 - PLID 27171 - Must wait for the load to finish
					pEMN->EnsureCompletelyLoaded();

					// (j.jones 2009-06-17 12:56) - PLID 34652 - make sure we check to see if the save failed
					if(FAILED(SaveEMR(esotEMN, (long)pEMN, TRUE))) {
						//return silently
						return;
					}
				}
			}
		}
	}
}

void CEmrTreeWnd::OnNextNode()
{
	try {
		// (j.jones 2006-02-08 10:24) - PLID 19019 - save the current topic, if our preference says so
		if(m_nEMRSavePref == 2)	//2 - save on Next/Prev
			SaveRowObject(m_pTree->CurSel, TRUE);
		
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = CalcNextRow(m_pTree->CurSel);
		SetTreeSel(pNewRow);
		AutoCollapse();
	} NxCatchAll("Error in OnNextNode");
}

void CEmrTreeWnd::OnPreviousNode()
{
	try {
		// (j.jones 2006-02-08 10:24) - PLID 19019 - save the current topic, if our preference says so
		if(m_nEMRSavePref == 2)	//2 - save on Next/Prev
			SaveRowObject(m_pTree->CurSel, TRUE);

		IRowSettingsPtr pNewRow = CalcPreviousRow(m_pTree->CurSel);
		SetTreeSel(pNewRow);
		AutoCollapse();
	} NxCatchAll("Error in OnPreviousNode");
}

void CEmrTreeWnd::HandleSelChanged(NXDATALIST2Lib::IRowSettingsPtr pOldRow, NXDATALIST2Lib::IRowSettingsPtr pNewRow)
{
	try {
		// (j.armen 2012-07-02 11:42) - PLID 51313 - If we don't have a license (or expired), then we don't want to handle any selections here
		if(m_lpDraggingRow || (g_pLicense->HasEMROrExpired(CLicense::cflrSilent) != 2))
			return;
		if(!pNewRow)
			return;

		// (a.walling 2012-04-02 08:29) - PLID 49304 - Removed a lot of dead code regarding moving items

		//Now, show the new window.
		try {
			EmrTree::ChildRow pNew(pNewRow);

			//Determine what type of row we're selecting.

			// (a.walling 2012-07-03 10:56) - PLID 51284 - Fixing activation ambiguities - much simpler now, just tell the frame to activate the object
			switch (pNew.GetType()) {
				case etrtTopic:
					GetEmrFrameWnd()->ActivateTopic(pNew.GetTopic());
					break;
				case etrtEmn:
					GetEmrFrameWnd()->ActivateEMNOnly(pNew.GetEMN());
					break;
				case etrtMoreInfo:
					GetEmrFrameWnd()->ShowMoreInfo(pNew.GetEMN());
					break;
				//TES 2/12/2014 - PLID 60748 - Codes topic
				case etrtCodes:
					GetEmrFrameWnd()->ShowCodesView(pNew.GetEMN());
					break;
			}
		} NxCatchAllThrow(__FUNCTION__ " handling new selection");

	}NxCatchAll(__FUNCTION__);
}

struct EmrTemplate
{
	long nID;
	CString strName;
};

void CEmrTreeWnd::OnNewEmn()
{
	try {
		//(c.haag 2006-04-04 09:59) - PLID 19890 - Check permissions
		// (j.jones 2007-05-15 17:05) - PLID 25431 - you can't create an EMR
		// without Create and Write permissions, this function cleanly checks for both
		// with only one password prompt or denial message
		if(!CheckHasEMRCreateAndWritePermissions())
		//if (!CheckCurrentUserPermissions(bioPatientEMR, sptCreate))
			return;

		//use the cached list of procedures
		CString strProcIDs;

		// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
		if (GetPicContainer()) {
			strProcIDs = GetPicContainer()->GetDelimitedProcedureList();
		}
		
		if(strProcIDs.IsEmpty()) {
			strProcIDs = "-1";
		}
		//Also, get all the template IDs we've already got, we may not want to add them again.
		CString strTemplateIDs;
		for(int i=0; i<m_EMR.GetEMNCount(); i++) {
			CString strTemplateID;
			strTemplateID.Format("%li,", m_EMR.GetEMN(i)->GetTemplateID());
			strTemplateIDs += strTemplateID;
		}
		strTemplateIDs.TrimRight(",");
		if(strTemplateIDs.IsEmpty()) {
			strTemplateIDs = "-1";
		}

		CArray<EmrTemplate,EmrTemplate&> arTemplates;
		//TES 5/28/2008 - PLID 30169 - First off, show any templates which are flagged as "universal."  These take precedence over
		// anything else.
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		ADODB::_RecordsetPtr rsTemplates = CreateParamRecordset("SELECT EMRTemplateT.ID, EMRTemplateT.Name,  "
			"EMRTemplateT.AddOnce, EMRCollectionT.ID AS CollectionID, EMRCollectionT.Name AS CollectionName "
			"FROM "
			"EMRTemplateT LEFT JOIN EMRCollectionT ON EMRTemplateT.CollectionID = EMRCollectionT.ID "
			"WHERE "
			"EmrTemplateT.Deleted = 0 AND EmrCollectionT.Inactive = 0 AND  "
			"EmrTemplateT.IsUniversal = 1 "
			"AND (EmrTemplateT.AddOnce = 0 OR EmrTemplateT.ID NOT IN ({INTSTRING})) "
			"ORDER BY MenuOrder, EMRTemplateT.Name, EMRCollectionT.Name", strTemplateIDs);
		while(!rsTemplates->eof) {
			EmrTemplate et;
			et.nID = AdoFldLong(rsTemplates, "ID");
			et.strName = AdoFldString(rsTemplates, "Name");
			arTemplates.Add(et);
			rsTemplates->MoveNext();
		}
		rsTemplates->Close();

		//OK, now get all the EMR Templates that match our criteria.
		// (z.manning, 02/21/2007) - PLID 24856 - Don't show templates from inactive collections.
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		rsTemplates = CreateParamRecordset("SELECT EmrTemplateT.ID, EmrTemplateT.Name "
			"FROM EmrTemplateT LEFT JOIN EmrCollectionT ON EmrTemplateT.CollectionID = EmrCollectionT.ID "
			"WHERE EmrTemplateT.Deleted = 0 AND EmrCollectionT.Inactive = 0 AND EmrTemplateT.ID IN (SELECT TemplateID FROM EmrTemplateTopicsT) "
			"AND EmrTemplateT.ID IN (SELECT EmrTemplateID FROM EmrTemplateProceduresT WHERE ProcedureID IN ({INTSTRING}) "
			"AND (EmrTemplateT.AddOnce = 0 OR EmrTemplateT.ID NOT IN ({INTSTRING}))) "
			"ORDER BY EmrCollectionT.MenuOrder, EmrTemplateT.Name", strProcIDs, strTemplateIDs);
		BOOL bProceduralAdded = FALSE;
		while(!rsTemplates->eof) {
			EmrTemplate et;
			et.nID = AdoFldLong(rsTemplates, "ID");
			et.strName = AdoFldString(rsTemplates, "Name");
			arTemplates.Add(et);
			bProceduralAdded = TRUE;
			rsTemplates->MoveNext();
		}
		rsTemplates->Close();
		if(!bProceduralAdded) {
			//We couldn't find any!  Let's use the default list.
			// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
			rsTemplates = CreateParamRecordset("SELECT EMRTemplateT.ID, EMRTemplateT.Name "
				"FROM EMRTemplateT LEFT JOIN EMRCollectionT ON EMRTemplateT.CollectionID = EMRCollectionT.ID "
				"WHERE EmrTemplateT.Deleted = 0 AND EmrTemplateT.ID IN (SELECT TemplateID FROM EMRTemplateTopicsT)  "
				"AND CollectionID IN  "
				"(SELECT TOP 1 EMRCollectionT.ID FROM EMRCollectionT INNER JOIN EMRTemplateT ON EMRCollectionT.ID = EMRTemplateT.CollectionID  "
				"INNER JOIN EMRTemplateTopicsT ON EMRTemplateT.ID = EMRTemplateTopicsT.TemplateID WHERE EmrCollectionT.Inactive = 0 ORDER BY MenuOrder) "
				"AND (EmrTemplateT.AddOnce = 0 OR EmrTemplateT.ID NOT IN ({INTSTRING})) "
				"ORDER BY MenuOrder, EMRTemplateT.Name, EMRCollectionT.Name", strTemplateIDs);
			while(!rsTemplates->eof) {
				EmrTemplate et;
				et.nID = AdoFldLong(rsTemplates, "ID");
				et.strName = AdoFldString(rsTemplates, "Name");
				arTemplates.Add(et);
				rsTemplates->MoveNext();
			}
			rsTemplates->Close();
		}

		//Now, construct a menu with all our options.
		// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
		CNxMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		
		//(e.lally 2012-03-28) PLID 49237 - Negative IDs behave differently with the new menus, so let's use a random offset.
		const long cnMenuIDOffset = 10000;

		//(e.lally 2012-03-28) PLID 49237 - Moved the universal templates to a submenu if there are more than 30
		CNxMenu subMenuUniv;
		bool bUseSubmenu = false;
		if(arTemplates.GetSize() > 30){
			bUseSubmenu = true;
			subMenuUniv.m_hMenu = CreatePopupMenu();	
		}

		for(i = 0; i < arTemplates.GetSize(); i++) {
			//m.hancock - 2/23/2006 - PLID 19428 - Replace newline characters with a space
			CString strTemplateName = arTemplates[i].strName;
			strTemplateName.Replace("\r\n", " ");

			//Our command ID will be one higher than our index, because if they click off the menu it returns 0.
			// (z.manning, 02/21/2007) - PLID 24452 - Added call to ConvertToControlText to avoid ampersand menu issues.
			//(e.lally 2012-03-28) PLID 49237 - Offset the ID
			if(bUseSubmenu){
				//Move the universal templates to a submenu
				subMenuUniv.InsertMenu(nIndex++, MF_BYPOSITION, i + cnMenuIDOffset, ConvertToControlText(strTemplateName));
			}
			else {
				//Place all the templates in the main menu
				mnu.InsertMenu(nIndex++, MF_BYPOSITION, i + cnMenuIDOffset, ConvertToControlText(strTemplateName));
			}
		}

		if(bUseSubmenu){
			//Add the submenu to the main menu
			mnu.InsertMenu(nIndex++, MF_BYPOSITION|MF_POPUP, (UINT)subMenuUniv.m_hMenu, "Add &New...");
		}

		//Add a separator, and give them the ability to select any template.
		mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
		//(e.lally 2012-03-28) PLID 49237 - Offset the ID
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, cnMenuIDOffset - 1, "&Select EMN Template...");

		CPoint pt;
		GetCursorPos(&pt);
		int nCmdID = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_BOTTOMALIGN, pt.x, pt.y, this, NULL);
		//mnu.DestroyMenu();

		//What did they select?
		switch(nCmdID) {
		case 0:
			//They clicked off the menu, do nothing.
			return;
			break;
		//(e.lally 2012-03-28) PLID 49237 - Check based on the offset ID
		case (cnMenuIDOffset - 1):
			//They selected "Select EMN Template..."
			OnSelectEmnTemplate();
			break;
		default:
			//Look up the ID they selected in our array (remember to shift the index).
			//(e.lally 2012-03-28) PLID 49237 - Revert the offset of the ID for the true index
			long nTemplateID = arTemplates.GetAt(nCmdID - cnMenuIDOffset).nID;
			SourceActionInfo saiBlank;

			if(CheckIsNexWebTemplate(nTemplateID)) {
				// (j.dinatale 2012-10-01 11:35) - PLID 45549 - save everytime a new emn is created
				if(GetEMR() && !m_bIsTemplate){
					SaveEMR(esotEMR, (long)GetEMR(), TRUE);
				}

				// (a.walling 2013-01-22 10:00) - PLID 54762 - Emr Appointment linking - auto-assign appointment or prompt to choose
				ShowEMN(AddEMNFromTemplateWithAppointmentPrompt(nTemplateID, saiBlank));
			}
			break;
		}
	}NxCatchAll("Error in CEmrTreeWnd::OnNewEMN()");
}

void CEmrTreeWnd::OnSelectEmnTemplate()
{
	CSelectDlg dlg(this);
	dlg.m_strTitle = "Select EMN Template";
	dlg.m_strCaption = "Select a template on which to base the new EMN";
	//DRT 9/20/2006 - PLID 22618 - Generic EMN should use -2, not -1!
	// (z.manning, 02/21/2007) - 24856 - Don't show templates from inactive collections.
	dlg.m_strFromClause = 
		"( "
		"	SELECT EmrTemplateT.ID, EmrTemplateT.Name FROM EmrTemplateT "
		"	LEFT JOIN EMRCollectionT ON EmrTemplateT.CollectionID = EmrCollectionT.ID "
		"	WHERE Deleted = 0 AND EmrCollectionT.Inactive = 0 "
		"	UNION "
		"	SELECT -2, '<Generic EMN>' "
		") SubQ";
	dlg.AddColumn("ID", "ID", FALSE, FALSE);
	dlg.AddColumn("Name", "Name", TRUE, TRUE);
	if(dlg.DoModal() == IDOK) {
		long nTemplateID = VarLong(dlg.m_arSelectedValues[0]);
		SourceActionInfo saiBlank;

		if(CheckIsNexWebTemplate(nTemplateID)) {
			// (j.dinatale 2012-10-01 11:35) - PLID 45549 - save everytime a new emn is created
			if(GetEMR() && !m_bIsTemplate){
				SaveEMR(esotEMR, (long)GetEMR(), TRUE);
			}

			// (a.walling 2013-01-22 10:00) - PLID 54762 - Emr Appointment linking - auto-assign appointment or prompt to choose
			ShowEMN(AddEMNFromTemplateWithAppointmentPrompt(nTemplateID, saiBlank));
		}
	}

}
void CEmrTreeWnd::ShowEMN(CEMN *pEMN)
{
	// (j.jones 2007-10-01 12:01) - PLID 27562 - added try/catch
	try {
		// (a.walling 2007-10-01 13:50) - PLID 25548 - Increase nInShowEMN count
		m_nInShowEMN++;
	
		CWaitCursor pWait;

		// (a.walling 2008-06-04 09:21) - PLID 22049
		TryAutoWriteAccess(pEMN, NULL);

		//First, is this EMN already being displayed?
		NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_pTree->CurSel;
		if(pCurRow) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = pCurRow;
			NXDATALIST2Lib::IRowSettingsPtr pParent = pRow->GetParentRow();
			while(pParent) {
				pRow = pParent;
				pParent = pRow->GetParentRow();
			}
			//pRow is now the top-level row.
			ASSERT(VarLong(pRow->GetValue(TREE_COLUMN_ROW_TYPE)) == (long)etrtEmn);
			if(EmrTree::ChildRow(pRow).GetEMN() == pEMN) {
				//This EMN is already selected.
				// (a.walling 2007-10-01 13:50) - PLID 25548 - Decrease nInShowEMN count
				m_nInShowEMN--;
				return;
			}
		}
		//Find this EMN.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTree->GetFirstRow();
		while(pRow != NULL && EmrTree::ChildRow(pRow).GetEMN() != pEMN) pRow = pRow->GetNextRow();
		if(pRow) {
			//We found it!
			LoadSubtopics(pRow);
			pRow->Expanded = true;
			NXDATALIST2Lib::IRowSettingsPtr pRowToSelect = CalcNextRow(pRow);
			SetTreeSel(pRowToSelect);
			// (a.walling 2013-03-20 16:13) - PLID 55790 - No more loading behavior
		} else {
			//What?  Let's assert, although if somebody comes up with a valid reason to call this function for an EMN that may not
			//be present, we can take out this assertion.
			//DRT 11/16/2007 - There's another bug somewhere else that lets us delete all the EMNs on an EMR and save, and that will
			//	trigger this ASSERT.  I'm going to leave it here because we shouldn't really be doing that, and we'll fix that bug
			//	eventually.  Just adding this note here in case someone gets it in the meantime.
			ASSERT(FALSE);
		}

		// (a.walling 2007-10-01 13:50) - PLID 25548 - Decrease nInShowEMN count
		m_nInShowEMN--;

	}NxCatchAll("Error in CEmrTreeWnd::ShowEMN (2)");
}

//TES 9/9/2009 - PLID 35495 - Moved GetIcon() to EmrUtils (and renamed to GetEmrTreeIcon())

// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
CEMNMoreInfoDlg* CEmrTreeWnd::GetMoreInfoDlg(CEMN *pEMN, BOOL bCreateIfNotCreated /*= TRUE*/)
{
	return GetEmrFrameWnd()->GetMoreInfoDlg(pEMN, !!bCreateIfNotCreated);
}

//TES 2/18/2014 - PLID 60740 - Added Codes topic
CEmrCodesDlg* CEmrTreeWnd::GetCodesDlg(CEMN *pEMN, BOOL bCreateIfNotCreated /*= TRUE*/)
{
	return GetEmrFrameWnd()->GetEmrCodesDlg(pEMN, !!bCreateIfNotCreated);
}

int CEmrTreeWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CreateControls();

	return 0;
}

//#include "MsgBox.h"

// (j.jones 2012-10-11 17:58) - PLID 52820 - added bIsClosing, TRUE if the user picked Save & Close
EmrSaveStatus CEmrTreeWnd::SaveEMR(EmrSaveObjectType esotSaveType /*= esotEMR*/, long nObjectPtr /*= -1*/, BOOL bShowProgressBar /*= TRUE*/, BOOL bIsClosing /*= FALSE*/)
{
	CNxPerform nxp(__FUNCTION__);
	// (c.haag 2006-03-28 12:13) - PLID 19890 - We now consider permissions when initializing topic windows
	// (c.haag 2007-02-20 16:25) - PLID 24137 - Don't check patient permissions on templates! This also
	// broke the "with password" permission for patients because my original call assumed you knew your
	// password. The successive call to CheckCurrentUserPermissions does everything right, so I just commented
	// the next two lines of code out altogether.
	/*if (!CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE))
		return essFailed;*/

	// (c.haag 2007-08-27 09:39) - PLID 27185 - Ensure any EMN's involved are fully loaded
	int i;
	switch(esotSaveType) {
		case esotTopic: {
			CEMRTopic *pTopic = (CEMRTopic*)nObjectPtr;
			if (pTopic && pTopic->GetParentEMN()) {
				pTopic->GetParentEMN()->EnsureCompletelyLoaded();
			}
			break;
		}
		case esotEMN: {
			CEMN *pEMN = (CEMN*)nObjectPtr;
			if(pEMN) {
				pEMN->EnsureCompletelyLoaded();
			}
			break;
		}
		case esotEMR:
		default:
			for (i = 0; i < m_EMR.GetEMNCount(); i++) {
				CEMN* pEMN = m_EMR.GetEMN(i);
				if (pEMN) {
					pEMN->EnsureCompletelyLoaded();
				}
			}
			break;				
	}

	BOOL bCanWriteToEMR = (m_bIsTemplate) ? TRUE :
		CheckCurrentUserPermissions(bioPatientEMR, sptWrite);

	// (a.walling 2007-11-28 11:25) - PLID 28044 - Check if expired
	if (g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
		bCanWriteToEMR = FALSE;
	}

	if (!bCanWriteToEMR)
		return essFailed;

	// (j.jones 2006-02-08 09:57) - verify they haven't changed the server time
	if(m_EMR.GetID() != -1) {
		CNxPerform nxp(__FUNCTION__ " - ValidateEMRTimestamp");
		if(!ValidateEMRTimestamp(m_EMR.GetID())) {
			return essFailed;
		}
	}

	// (j.jones 2010-01-14 10:18) - PLID 24095 - Warn before saving an EMN with a blank name,
	// or an EMR with a blank name. Check only unsaved objects, and if they approve one entry
	// with a blank name (an incredibly foolish choice), then stop warning about others.
	if(!m_bIsTemplate) {
		BOOL bStopWarning = FALSE;
		CString strDesc = m_EMR.GetDescription();
		strDesc.TrimLeft();
		strDesc.TrimRight();
		if(m_EMR.IsEMRUnsaved() && strDesc.IsEmpty() && m_EMR.GetID() == -1) {

			if(IDYES == MessageBox("This EMR has no description. You should cancel saving and enter a description for this EMR.\n\n"
				"Do you wish to cancel saving and edit the description now?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
				
				//cancel the save
				return essFailed;
			}

			//if they are here, they decided to continue, so do not continue warning if there are blank EMN names
			bStopWarning = TRUE;
		}

		//check each EMN
		for(int i=0; i<GetEMNCount() && !bStopWarning; i++) {
			CEMN* pEMN = GetEMN(i);

			strDesc = pEMN->GetDescription();
			strDesc.TrimLeft();
			strDesc.TrimRight();

			if(pEMN->IsUnsaved() && strDesc.IsEmpty() && pEMN->GetID() == -1) {

				if(IDYES == MessageBox("At least one EMN has no description. You should cancel saving and enter a description for each EMN on this EMR.\n\n"
					"Do you wish to cancel saving and edit the EMN description now?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
					
					//cancel the save
					return essFailed;
				}

				//if they are here, they decided to continue, so do not continue warning if there are blank EMN names
				bStopWarning = TRUE;
			}
		}
	}

	// (j.jones 2009-09-15 08:45) - PLID 30448 - ensure the template name doesn't already exist
	if(m_bIsTemplate && m_EMR.GetEMNCount() == 1) {
		CEMN* pEMN = GetEMN(0);
		if(pEMN != NULL && pEMN->IsUnsaved()) {
			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ID FROM EMRTemplateT "
				"WHERE Name = {STRING} AND Deleted = 0 AND ID <> {INT}", pEMN->GetDescription(), pEMN->GetID());
			if(!rs->eof) {
				// It's a dup, so we can't proceed
				MsgBox(MB_OK|MB_ICONEXCLAMATION, "There is already a template with this name.  Please enter a unique name for this template.");
				return essFailed;
			}

			//TES 11/2/2009 - PLID 35808 - Also, if this is the NexWeb template, warn them.
			if(pEMN->IsNexWebTemplate()) {
				// (b.cardillo 2010-09-22 09:18) - PLID 39568 - Corrected wording now that we allow more than one NexWeb EMR template
				if(IDYES != MsgBox(MB_YESNO|MB_ICONEXCLAMATION, "Warning: The template you are editing is a NexWeb template.  Any changes "
					"you have made will be published to your website, and will be visible to patients.  Are you sure you "
					"wish to continue saving your changes to this template?")) {
						return essFailed;
				}
			}

			// (j.gruber 2012-08-31 15:00) - PLID 52285 - OMR template
			if(pEMN->IsOMRTemplate()) {				
				if(IDYES != MsgBox(MB_YESNO|MB_ICONEXCLAMATION, "Warning: The template you are editing is associated with an OMR form. Are you sure you "
					"wish to continue saving your changes to this template?")) {
						return essFailed;
				}
			}
		}
	}

	// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
	if (!m_bIsTemplate) {
		BOOL bHasUnfilledRequiredDetails = FALSE;
		for (int iEMN = 0, nEMNCount = m_EMR.GetEMNCount(); iEMN < nEMNCount; iEMN++) {
			if (GetEMN(iEMN)->HasVisibleUnfilledRequiredDetails()) {
				bHasUnfilledRequiredDetails = TRUE;
				break;
			}
		}
		if (bHasUnfilledRequiredDetails) {
			if (IDYES != MsgBox(MB_YESNO|MB_ICONEXCLAMATION, 
				_T("Warning: The EMR you are saving contains at least one required detail that has not been filled in.  Are you ")
				_T("sure you wish to continue saving your changes?")
				)) 
			{
				return essFailed;
			}
		}
	}

	// (b.savon 2014-03-07 08:24) - PLID 60826 - SPAWN - Pop up a warning if you try to save an EMN that has
	// any spawned diag codes whose ICD-10 has not been selected yet
	long nWarnIfUnmatchedPreference = GetRemotePropertyInt("EMR_WarnWhenUnmatchedDiagnosisCodes", 1, 0, GetCurrentUserName());
	if (!m_bIsTemplate && bIsClosing && nWarnIfUnmatchedPreference != 0 ){
		BOOL bWarn = FALSE;
		for (int iEMN = 0, nEMNCount = m_EMR.GetEMNCount(); iEMN < nEMNCount; iEMN++) {
			if( GetEMN(iEMN)->HasUnmatchedDiagnosisCodes(nWarnIfUnmatchedPreference) ){
				bWarn = TRUE;
				break;
			}
		}
		if( bWarn ){
			CString strCodeTerm;
			if( nWarnIfUnmatchedPreference == 1 ){
				strCodeTerm = "ICD-9";
			}else if( nWarnIfUnmatchedPreference == 2){
				strCodeTerm = "ICD-10";
			}else if (nWarnIfUnmatchedPreference == 3){
				strCodeTerm = "ICD-9 or ICD-10";
			}else{
				//This shouldnt happen, a preference of 0 is do not warn; anything else is bad data
				ASSERT(FALSE);
				strCodeTerm = "unknown";
			}
			if (IDYES != MsgBox(MB_YESNO|MB_ICONEXCLAMATION, 
				_T("Warning: The EMR you are saving contains at least one diagnosis code that has not been matched to an " + strCodeTerm + " code.  Are you ")
				_T("sure you wish to continue saving your changes?")
				)) 
			{
				return essFailed;
			}
		}
	}

	// (c.haag 2008-07-23 17:22) - PLID 30820 - Check for any problems that were not explicitly deleted but
	// are still going to be flagged as deleted because their owner is as such, and warn the user. Return
	// if the user cancelled the dialog (meaning they did not want to delete problems, which means they cannot
	// delete anything else).
	{
		CNxPerform nxp(__FUNCTION__ " - CheckForDeletedEmrProblems");

		if(esotSaveType == esotEMR) {
			if (!CheckForDeletedEmrProblems(esotEMR, (long)&m_EMR, FALSE)) {
				return essFailed;
			}
		} else {
			if (!CheckForDeletedEmrProblems(esotSaveType, nObjectPtr, FALSE)) {
				return essFailed;
			}
		}
	}

	// (a.walling 2010-03-31 10:54) - PLID 38006 - If we are saving an EMN that was spawned from another EMN, we must ensure that the other
	// EMN is saved.

	{
		CNxPerform nxp(__FUNCTION__ " - Save dependent EMNs");
		CStringArray saErrors;

		switch(esotSaveType) {
			case esotTopic: {
				CEMRTopic *pTopic = (CEMRTopic*)nObjectPtr;
				if (pTopic && pTopic->GetParentEMN()) {
					CEMN* pEMN = pTopic->GetParentEMN();
					if (pEMN->GetID() == -1) {
						// this EMN has not been saved, so ensure the EMN that spawned it (if any) has been completely saved.
						SourceActionInfo sai = pEMN->GetSourceActionInfo();
						if (sai.pSourceDetail != NULL) {
							CEMN* pSourceEMN = sai.pSourceDetail->m_pParentTopic->GetParentEMN();
							if (pSourceEMN && pSourceEMN->IsUnsaved() && pSourceEMN != pEMN) {
								// OK, we have an unsaved source EMN, we must save it now.
								SaveSingleEMN(pSourceEMN, saErrors, bShowProgressBar);
							}
						}
					}
				}
				break;
			}
			case esotEMN: {
				CEMN *pEMN = (CEMN*)nObjectPtr;
				if(pEMN) {					
					if (pEMN->GetID() == -1) {
						// this EMN has not been saved, so ensure the EMN that spawned it (if any) has been completely saved.
						SourceActionInfo sai = pEMN->GetSourceActionInfo();
						if (sai.pSourceDetail != NULL) {
							CEMN* pSourceEMN = sai.pSourceDetail->m_pParentTopic->GetParentEMN();
							if (pSourceEMN && pSourceEMN->IsUnsaved() && pSourceEMN != pEMN) {
								// OK, we have an unsaved source EMN, we must save it now.
								SaveSingleEMN(pSourceEMN, saErrors, bShowProgressBar);
							}
						}
					}
				}
				break;
			}
			case esotEMR:
			default:
				// (a.walling 2010-03-31 11:11) - PLID 38006 - This is a bit trickier. We have to figure out what order in which to save these things.
				{
					bool bContinue = true;
					while (bContinue) {
						bool bFoundSource = false;
						for (i = 0; i < m_EMR.GetEMNCount() && !bFoundSource; i++) {
							CEMN* pEMN = m_EMR.GetEMN(i);
							if (pEMN) {				
								if (pEMN->GetID() == -1) {
									// this EMN has not been saved, so ensure the EMN that spawned it (if any) has been completely saved.
									SourceActionInfo sai = pEMN->GetSourceActionInfo();
									if (sai.pSourceDetail != NULL) {
										CEMN* pSourceEMN = sai.pSourceDetail->m_pParentTopic->GetParentEMN();
										if (pSourceEMN && pSourceEMN->IsUnsaved() && pSourceEMN != pEMN) {
											// OK, we have an unsaved source EMN, we must save it now.
											bFoundSource = true;
											SaveSingleEMN(pSourceEMN, saErrors, bShowProgressBar);
											break; // restart the loop
										}
									}
								}
							}
						}

						if (!bFoundSource) {
							bContinue = false;
						}
					}
				}
				break;				
		}
		
		// (a.walling 2010-03-31 11:18) - PLID 38006 - Warn the user if any EMNs could not be saved.
		if (saErrors.GetSize() > 0) {
			CString strErrors = "The following errors occurred while saving dependent data in other EMNs: \r\n\r\n";
			for (int f = 0; f < saErrors.GetSize(); f++) {
				strErrors += saErrors[f] + "\r\n";
			}
			
			MessageBox(strErrors, NULL, MB_ICONEXCLAMATION);

			return essFailed;
		}
	}

	// (j.jones 2012-09-27 15:31) - PLID 52820 - bDrugInteractionsChanged will be set to TRUE if something
	// changed in the save that affects drug interactions, such as adding medications
	BOOL bDrugInteractionsChanged = FALSE;

	//TES 10/31/2013 - PLID 59251 - Track any interventions that get triggered
	CDWordArray arNewCDSInterventions;

	// (j.jones 2007-08-30 09:29) - PLID 27243 - Office Visit incrementing is no longer
	// used in the L2 EMR, it's in Custom Records only


	EmrSaveStatus essSavedStatus = essFailed;
	{
		CNxPerform nxp(__FUNCTION__ " - Saving!");

		if(esotSaveType == esotEMR) {
			essSavedStatus = SaveEMRObject(esotEMR, (long)&m_EMR, bShowProgressBar, bDrugInteractionsChanged, arNewCDSInterventions);
		} else {
			essSavedStatus = SaveEMRObject(esotSaveType, nObjectPtr, bShowProgressBar, bDrugInteractionsChanged, arNewCDSInterventions);
		}
	}

	// (a.walling 2008-06-26 16:22) - PLID 30513 - Use SUCEEDED macro to check for success
	if (SUCCEEDED(essSavedStatus) && !m_bIsTemplate) {
		//
		// (c.haag 2006-04-12 10:29) - PLID 20040 - Update the IsCommitted flag of the PIC
		//
		// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
		if (GetEmrEditor()) {
			// (c.haag 2010-08-02 09:27) - PLID 38928 - Defer to the EMR editor. It needs to do
			// its own work on saving as well.
			CNxPerform nxp(__FUNCTION__ " - Commit");
			GetEmrEditor()->Commit();
		}
	} else if (essSavedStatus == essFailedConcurrency) {
		// (a.walling 2008-06-26 16:26) - PLID 30513 - Get more info for the user.
		CString strMessage = "The following objects could not save:\r\n\r\n";
		long nCount = 0;
		for (int i = 0; i < GetEMNCount(); i++) {
			CEMN* pEMN = GetEMN(i);

			if (pEMN->IsUnsaved()) {
				if (pEMN->GetID() != -1) {
					CWriteTokenInfo wtInfo;
					BOOL bIsVerified = pEMN->VerifyWriteToken(wtInfo);
					
					if (wtInfo.bIsOldRevision || !wtInfo.bIsVerified) {
						if (wtInfo.bIsOldRevision && !wtInfo.bIsVerified) {
							strMessage += FormatString("The EMN '%s' has been modified by another user and does not have write access.", pEMN->GetDescription());
						} else if (!wtInfo.bIsVerified) {
							strMessage += FormatString("The EMN '%s' does not have write access.", pEMN->GetDescription());
						} else if (wtInfo.bIsOldRevision) {
							strMessage += FormatString("The EMN '%s' has been modified by another user.", pEMN->GetDescription());
						}

						if (!wtInfo.bIsVerified && wtInfo.nHeldByUserID != -1) {
							strMessage += FormatString(" The user %s has held this object for editing %s at %s.\r\n",
								wtInfo.strHeldByUserName, FormatDateTimeForInterface(wtInfo.dtHeld, NULL, dtoDateWithToday),
								FormatDateTimeForInterface(wtInfo.dtHeld, 0, dtoTime));
						} else {
							strMessage += "\r\n";
						}

						nCount++;
					}
				}
			}
		}

		if (nCount > 0) {
			MessageBox(strMessage, NULL, MB_ICONEXCLAMATION);
		}
	}

	//TES 10/31/2013 - PLID 59251 - Notify the user for any interventions that were just triggered
	GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);

	FOR_ALL_ROWS(m_pTree) {
		EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
		//TES 2/12/2014 - PLID 60748 - Added etrtCodes
		if(etrt == etrtTopic || etrt == etrtEmn || etrt == etrtMoreInfo || etrt == etrtCodes) {
			EnsureModifiedState(pRow);
		}
		GET_NEXT_ROW(m_pTree)
	}

	// (j.jones 2012-09-27 16:14) - PLID 52820 - bDrugInteractionsChanged will have been set to TRUE if something
	// changed in the save that affects drug interactions, such as adding medications.	
	if(SUCCEEDED(essSavedStatus) && !m_bIsTemplate && bDrugInteractionsChanged) {
		
		CNxPerform nxp(__FUNCTION__ " - CheckShowDrugInteractions");

		//This will potentially open the drug interaction warning, based
		//on the user's EMRDrugInteractionChecks preference.

		//We may be saving and closing at the same time, this
		//function needs to know that so it knows whether to show
		//the PIC or MainFrame drug interaction dialog.
		CheckShowDrugInteractions(TRUE, FALSE, bIsClosing);

		//Track that interactions were changed during this EMR session.
		m_bDrugInteractionsChangedThisSession = TRUE;
	}

	return essSavedStatus;
}

// (j.jones 2012-09-28 09:27) - PLID 52820 - called after a save is completed where content
// that affects drug interactions was changed, and also after closing/locking an EMN when drug
// interactions changed while editing that EMN
void CEmrTreeWnd::CheckShowDrugInteractions(BOOL bIsSaving, BOOL bIsLocking, BOOL bIsClosing)
{
	try {

		//This function should only be called after a save has completed,
		//and that save modified content that affects drug interactions,
		//such as adding new medications.
		
		if(m_bIsTemplate) {
			//should have never been called on a template
			return;
		}

		//If we get here, we know that drug interaction content changed.
		//The EMRDrugInteractionChecks preference controls how frequently
		//we try to show the interaction warning.
		BOOL bShowInteractions = FALSE;
		
		if(bIsSaving && (m_eEMRDrugInteractionChecksPref == edictWarnWhenSaving || m_eEMRDrugInteractionChecksPref == edictSaveWarnWhenMedsChange)) {
			//They want to be warned any time they save, or when related content changes.
			//The edictWhenMedsChange setting would have triggered this save when content changed,
			//so we must warn now that the save is done.
			bShowInteractions = TRUE;
		}
		else if((bIsLocking || bIsClosing) && m_eEMRDrugInteractionChecksPref == edictWarnWhenClosingOrLocking) {
			//they only want to be warned when closing/locking, and we are doing that now
			bShowInteractions = TRUE;
		}
		
		//now only show the interaction dialog if the current situation matched their prference
		if(bShowInteractions) {

			//if the window is closing, show the mainframe interaction dialog,
			//otherwise show the EMR's interaction dialog such that it uses
			//the EMR window as its parent
			if(bIsClosing) {
				// Show the mainframe's drug interaction dialog, not the EMR version.
				CMainFrame *p = GetMainFrame();
				if(p) {
					//This will silently show nothing if there are no interactions,
					//and also silently do nothing if they are not licensed for FirstDataBank.
					p->ShowDrugInteractions(GetPatientID());
				}
			}
			else {
				// Show the EMR's drug interaction dialog, not the mainframe version.
				CPicContainerDlg *p = GetPicContainer();
				if(p) {
					//This will silently show nothing if there are no interactions,
					//and also silently do nothing if they are not licensed for FirstDataBank.
					p->ViewDrugInteractions();
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-09-28 11:17) - PLID 52820 - Used when closing or locking an EMN,
// anything that would show drug interactions once per session.
// This function checks the m_bDrugInteractionsChangedThisSession flag,
// if true it will call CheckShowDrugInteractions and then reset the flag.
void CEmrTreeWnd::TryShowSessionDrugInteractions(BOOL bIsLocking, BOOL bIsClosing)
{
	try {

		if(m_bDrugInteractionsChangedThisSession) {
			//this will do nothing unless the EMRDrugInteractionChecks preference
			//is set to edictWhenClosing, which also fires when locking an open EMN
			//(but not when locking an already-saved, closed EMN)
			CheckShowDrugInteractions(FALSE, bIsLocking, bIsClosing);
			m_bDrugInteractionsChangedThisSession = FALSE;
		}

	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2010-03-31 11:04) - PLID 38006 - Saves a single EMN
bool CEmrTreeWnd::SaveSingleEMN(CEMN* pEMN, CStringArray& saErrors, BOOL bShowProgressBar)
{		
	CString strMessage;

	if (!pEMN->IsWritable()) {
		CWriteTokenInfo wtInfo;
		if (!pEMN->RequestWriteToken(wtInfo))
		{
			// (j.armen 2013-05-14 11:09) - PLID 56680 - Write Token Info keeps track of external status
			strMessage.Format("'%s': ", pEMN->GetDescription());

			if (wtInfo.bIsOldRevision)
				strMessage += "The loaded EMN has been modified by another user since it was initially opened.";
			else if (wtInfo.bIsDeleted)
				strMessage += "The loaded EMN has been deleted by another user.";	
			else if (wtInfo.bIsExternal)
				strMessage += FormatString("The EMN is currently held for editing by the user '%s' %s at %s (using an external device, identified as: %s).", wtInfo.strHeldByUserName, FormatDateTimeForInterface(wtInfo.dtHeld, NULL, dtoDateWithToday), FormatDateTimeForInterface(wtInfo.dtHeld, 0, dtoTime), wtInfo.strDeviceInfo);
			else
				strMessage += FormatString("The EMN is currently held for editing by the user '%s' %s at %s (using workstation %s).", wtInfo.strHeldByUserName, FormatDateTimeForInterface(wtInfo.dtHeld, NULL, dtoDateWithToday), FormatDateTimeForInterface(wtInfo.dtHeld, 0, dtoTime), wtInfo.strDeviceInfo);

			saErrors.Add(strMessage);
		}
	}
	if (pEMN->IsWritable()) {
		if (FAILED(SaveEMR(esotEMN, (long)pEMN, bShowProgressBar))) {		
			strMessage.Format("'%s': ", pEMN->GetDescription());
			strMessage += "The EMN failed to save.";

			saErrors.Add(strMessage);
			return false;
		} else {
			return true;
		}
	} else {
		return false;
	}
}

IRowSettingsPtr CEmrTreeWnd::AddTopicRow(CEMRTopic *pTopic, IRowSettingsPtr pParentRow, OPTIONAL IRowSettingsPtr pInsertBefore /*= NULL*/)
{
	// (c.haag 2007-10-24 11:40) - PLID 27562 - Have additional error checking and
	// logging for pinning down problems rooted in one of many possible causes
	try {
		BOOL bLocked = pTopic->GetParentEMN()->GetStatus() == 2;
		NXDATALIST2Lib::IRowSettingsPtr pChildRow = m_pTree->GetNewRow();
		pChildRow->PutValue(TREE_COLUMN_ID, pTopic->GetID());
		pChildRow->PutValue(TREE_COLUMN_OBJ_PTR, (long)pTopic);
		pChildRow->PutValue(TREE_COLUMN_ICON, (long)0);
		pChildRow->PutValue(TREE_COLUMN_NAME, _bstr_t(pTopic->GetName()));
		pChildRow->PutValue(TREE_COLUMN_ROW_TYPE, (long)etrtTopic);
		pChildRow->PutValue(TREE_COLUMN_LOADED, (long)(pTopic->HasSubTopics()?0:1));
		if(pInsertBefore) {
			m_pTree->AddRowBefore(pChildRow, pInsertBefore);
		}
		else {
			m_pTree->AddRowAtEnd(pChildRow, pParentRow);
		}
		
		//TES 12/20/2005: If this topic has subtopics, put in a placeholder row for now, we'll load them when the user expands the row.
		//TES 1/17/2006 - Don't do this if the only subtopics are "potential" subtopics.
		if(pTopic->HasSubTopics()) {
			CString strName = VarString(pChildRow->GetValue(TREE_COLUMN_NAME));
			InsertPlaceholder(pChildRow);
		}

		//TES 1/17/2006 - If this topic only exists to hold spawned details which have yet to be spawned, don't show it.
		// (a.walling 2012-07-09 12:35) - PLID 51441 - IsEmpty now has some non-optional parameters
		if(pTopic->IsEmpty(NULL, FALSE) && !pTopic->ShowIfEmpty(FALSE)) {
			if(m_bIsTemplate) {
				pChildRow->PutForeColor(RGB(127,127,127));
			}
			else {
				// (c.haag 2007-09-27 10:32) - PLID 27509 - Set the child row to be invisible.
				SetTreeRowVisible(pChildRow, FALSE);
				// (a.walling 2007-09-21 09:53) - PLID 25549 - Although there is currently no code that requires this, the preview 
				// should remain consistent with the tree.
				if (GetEmrPreviewCtrl()) {
					GetEmrPreviewCtrl()->ShowTopic(pTopic, FALSE);
				}
			}
		}

		EnsureModifiedState(pChildRow);
		// (c.haag 2007-05-09 09:20) - PLID 25946 - If the topic hasn't been loaded yet,
		// do not attempt to set its topic background color or check for duplicate topics.
		// These will be done later after the topic is loaded in CEmrTreeWnd::OnTopicLoadComplete/ByPreloader.
		if (pTopic->IsLoaded()) {
			EnsureTopicBackColor(pTopic);
			CheckDuplicateTopics(pParentRow);
		}
		return pChildRow;
	} NxCatchAllSilentCallThrow(
		ASSERT(FALSE);
		Log("An exception was thrown from CEmrTreeWnd::AddTopicRow. The exception will quietly be passed to the calling function.");
	);
}

long CEmrTreeWnd::GetEMRID()
{
	return m_EMR.GetID();
}

LRESULT CEmrTreeWnd::OnPreDeleteEmrItem(WPARAM wParam, LPARAM lParam)
{
	try {

		CEMNDetail *pDoomedDetail = (CEMNDetail*)wParam;

		// (j.jones 2006-03-15 09:18) - it may have already been deleted!
		if(!pDoomedDetail)
			return 0;

		// (a.walling 2007-04-10 12:25) - PLID 25549
		if (GetEmrPreviewCtrl()) {
			GetEmrPreviewCtrl()->RemoveDetail(pDoomedDetail);
		}

		//DRT 8/20/2007 - PLID 26382 - Safety check!  If there's no parent topic on this detail, then obviously there's nothing
		//	to show or hide.  This should never happen.
		if(pDoomedDetail->m_pParentTopic == NULL)
			return 0;

		//We need to do three things: hide the row (maybe), update the exclamation buttons, and update the topic color.
		if(!m_bIsTemplate) {
			//If this topic is now just "potential", hide it.
			// (a.walling 2012-07-09 12:35) - PLID 51441 - IsEmpty now has some non-optional parameters
			if(pDoomedDetail->m_pParentTopic->IsEmpty(pDoomedDetail, FALSE) && !pDoomedDetail->m_pParentTopic->ShowIfEmpty()) {
				// (a.walling 2007-04-11 18:16)
				// first, tell our preview window to hide this.
				if (GetEmrPreviewCtrl()) {
					GetEmrPreviewCtrl()->ShowTopic(pDoomedDetail->m_pParentTopic, FALSE);
				}
				IRowSettingsPtr pRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pDoomedDetail->m_pParentTopic, NULL, FALSE);
				if(pRow) {
					// (c.haag 2006-03-09 13:28) - PLID 19493 - There is a bug where when a row is made
					// invisible, then any topics that are selected by the user for the first time will
					// not draw on the screen. This is because the topic being destroy is never hidden
					// from the screen. To remedy this solution, we undo the datalist 2's selection change
					// and then post a message to have the tree find another topic to make visible after
					// all the deleting and adding and spawning and whatnot is done.
					//
					// (c.haag 2007-09-27 08:30) - PLID 27509 - This function does all the work for us now.
					//
					SetTreeRowVisible(pRow, FALSE);
				}
				else {
					//This means the row for this topic hasn't been loaded yet.  Find the first visible parent, make sure it's 
					//displayed correctly.
					CEMRTopic *pTopic = pDoomedDetail->m_pParentTopic->GetParentTopic();
					CEMN *pEMN = NULL;
					pRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pTopic, NULL, FALSE);
					while(pTopic != NULL && pRow == NULL) {
						pTopic = pTopic->GetParentTopic();
						pRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pTopic, NULL, FALSE);
					}
					if(pRow == NULL) {
						//The EMN node at least has to be there.
						pEMN = pDoomedDetail->m_pParentTopic->GetParentEMN();
						pRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pEMN, NULL, FALSE);

						//DRT 8/20/2007 - PLID 26382 - Safety check!  The comment above claims the EMN must exist, but we want
						//	to check for NULL just in case.  If that so happens, we'll have to throw an exception, because we
						//	cannot continue on without an EMN to work from.
						if(pEMN == NULL) {
							AfxThrowNxException("Parent EMN was not found for detail to be removed.");
						}
					}

					//DRT 8/20/2007 - PLID 26382 - Safety check!  It's not good enough to just ASSERT that the pRow is not NULL.  If 
					//	it is, we have to do something, not just wildly use it.
					ASSERT(pRow != NULL);
					if(pRow == NULL) {
						//If this is true, then the topic row was not found above, we fell into the EMN section, and it was not found
						//	either.  If this is all true, then we must fail with an exception, we cannot go forward.
						AfxThrowNxException("Row not found when attempting to determine visible status.");
					}

					//Now make sure that this doesn't have a plus sign if it shouldn't.
					if(pRow->GetFirstChildRow()) {
						BOOL bVisibleFound = FALSE;
						if(pTopic) {
							for(int i = 0; i < pTopic->GetSubTopicCount() && !bVisibleFound; i++) {
								// (a.walling 2012-07-09 12:35) - PLID 51441 - IsEmpty now has some non-optional parameters
								if(!pTopic->GetSubTopic(i)->IsEmpty(pDoomedDetail, FALSE) || pTopic->GetSubTopic(i)->ShowIfEmpty()) bVisibleFound = TRUE;
							}
						}
						else {
							for(int i = 0; i < pEMN->GetTopicCount() && !bVisibleFound; i++) {
								// (a.walling 2012-07-09 12:35) - PLID 51441 - IsEmpty now has some non-optional parameters
								if(!pEMN->GetTopic(i)->IsEmpty(pDoomedDetail, FALSE) || pEMN->GetTopic(i)->ShowIfEmpty()) bVisibleFound = TRUE;
							}
						}
						if(!bVisibleFound) {
							//If the child row is a placeholder, remove it, otherwise, hide it.
							IRowSettingsPtr pChildRow = pRow->GetFirstChildRow();
							EmrTreeRowType etrt = EmrTree::ChildRow(pChildRow).GetType();
							if(etrt == etrtPlaceholder) {
								RemoveTreeRow(pChildRow);
							}
							else {
								// (a.walling 2007-09-21 09:47) - PLID 25549 - Ensure the preview control is up to date
								// Since visibility is inherited in the DOM, we only need to hide the highest-level object.
								if (etrt == etrtTopic && GetEmrPreviewCtrl() != NULL) {
									CEMRTopic* pChildTopic = EmrTree::ChildRow(pChildRow).GetTopic();
									GetEmrPreviewCtrl()->ShowTopic(pChildTopic, FALSE);
								}

								//We know all subtopics should be hidden.
								while(pChildRow) {
									// (c.haag 2007-09-27 11:11) - PLID 27509 - Use SetTreeRowVisible. If pChildRow is
									// somehow the current tree selection, it will ensure a new selection is made.
									SetTreeRowVisible(pChildRow, FALSE);
									pChildRow = pChildRow->GetNextRow();
								}
							}
						}
					}
				}

				//Any parents may also be empty.
				BOOL bNonEmptyFound = FALSE;
				while(pRow->GetParentRow() != NULL && !bNonEmptyFound) {
					pRow = pRow->GetParentRow();
					if(etrtTopic == EmrTree::ChildRow(pRow).GetType()) {
						CEMRTopic *pTopic = EmrTree::ChildRow(pRow).GetTopic();
						//DRT 8/20/2007 - PLID 26382 - Safety check added to make sure pTopic is valid.
						// (a.walling 2012-07-09 12:35) - PLID 51441 - IsEmpty now has some non-optional parameters
						if(pTopic != NULL && pTopic->IsEmpty(pDoomedDetail, FALSE)) {
							if(!pTopic->ShowIfEmpty()) {
								// (c.haag 2007-09-27 11:11) - PLID 27509 - Use SetTreeRowVisible. If pChildRow is
								// somehow the current tree selection, it will ensure a new selection is made.
								SetTreeRowVisible(pRow, FALSE);
								// (a.walling 2007-04-11 18:17) - PLID 25549
								// tell our preview window to hide this.
								if (GetEmrPreviewCtrl()) {
									GetEmrPreviewCtrl()->ShowTopic(pTopic, FALSE);
								}
							}
						}
						else {
							bNonEmptyFound = TRUE;
						}
					}
					else {
						bNonEmptyFound = TRUE;
					}
				}
			}
		}

		IRowSettingsPtr pParentRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pDoomedDetail->m_pParentTopic, NULL, FALSE);
		if(pParentRow) {
			EnsureModifiedState(pParentRow);
		}

		//Update our topic color.
		EnsureTopicBackColor(pDoomedDetail->m_pParentTopic, pDoomedDetail);

	} NxCatchAll("Error in OnPreDeleteEmrItem");

	return 0;
}

LRESULT CEmrTreeWnd::OnPostAddEmrItem(WPARAM wParam, LPARAM lParam)
{
	try {
		CEMNDetail *pAddedDetail = (CEMNDetail*)wParam;
		BOOL bIsInitialLoad = (BOOL)lParam; // (a.walling 2007-04-10 09:51) - PLID 25549 - Included the initial load bool in the unused lParam

		//We need to do three things: show the topic, update the exclamation buttons, and update the topic color.

		//If this topic was hidden before, it shouldn't be.
		IRowSettingsPtr pRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pAddedDetail->m_pParentTopic, NULL, FALSE);
		if(pRow) {
			SetTreeRowVisible(pRow, TRUE);
			//Also, any parents should be visible.
			while(pRow->GetParentRow()) {
				pRow = pRow->GetParentRow();
				SetTreeRowVisible(pRow, TRUE);
			}
		}
		else {
			//This means its parent hasn't loaded its subtopics.  Ensure that the parent at least shows a plus sign.
			IRowSettingsPtr pParent = NULL;
			CEMRTopic *pParentTopic = pAddedDetail->m_pParentTopic->GetParentTopic();
			if(pParentTopic) {
				pParent = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pParentTopic, NULL, FALSE);
			}
			else {
				pParent = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pAddedDetail->m_pParentTopic->GetParentEMN(), NULL, FALSE);
			}
			if(pParent) {
				//OK, we've got a parent, make sure it has a plus sign.
				if(pParent->GetFirstChildRow() == NULL) {
					//Nope, give it a placeholder.
					InsertPlaceholder(pParent);
					//TES 2/8/2006 - The parent may have THOUGHT it was loaded, but it isn't.
					pParent->PutValue(TREE_COLUMN_LOADED, (long)0);
				}
			}
			else {
				//This topic itself must not have been loaded, so when it is, the plus sign will be there.
			}
		}

		// (a.walling 2007-04-10 09:46) - PLID 25549
		if (!bIsInitialLoad) {
			if (GetEmrPreviewCtrl()) {
				// (a.walling 2008-11-17 14:50) - PLID 32060 - Only check for subdetail parents if we are actually a subdetail
				CEMNDetail* pSubDetailParent = pAddedDetail->IsSubDetail() ? pAddedDetail->GetSubDetailParent() : NULL;
				if (pSubDetailParent) {
					GetEmrPreviewCtrl()->UpdateDetail(pSubDetailParent);
					
					// (a.walling 2008-01-03 15:52) - PLID 28391 - Need to insert the detail before
					// mucking about with the visibility settings.
					// show the topic in the preview
					// but this is unnecessary when initially loading an EMN.
					GetEmrPreviewCtrl()->ShowTopic(pAddedDetail->GetSubDetailParentTopic(), TRUE);
				} else {
					CEMNDetail* pNextDetail = pAddedDetail->GetNextDetail();
					GetEmrPreviewCtrl()->InsertDetail(pAddedDetail, pNextDetail);

					
					// (a.walling 2008-01-03 15:52) - PLID 28391 - Need to insert the detail before
					// mucking about with the visibility settings.
					// show the topic in the preview
					// but this is unnecessary when initially loading an EMN.
					GetEmrPreviewCtrl()->ShowTopic(pAddedDetail->m_pParentTopic, TRUE);
				}

				// (a.walling 2007-12-17 17:23) - PLID 28391
				pAddedDetail->m_pParentTopic->RefreshHTMLVisibility();
			}
		}

		//update the modified state
		EnsureTopicModifiedState(pAddedDetail->m_pParentTopic);

		//Update our topic color.
		// (a.walling 2007-09-27 12:42) - PLID 25549 - Pass the 'do not hide' flag to EnsureTopicBackColor
		// so the preview pane is not prematurely updated.
		EnsureTopicBackColor(pAddedDetail->m_pParentTopic, NULL, FALSE);
	} NxCatchAll("Error in OnPostAddEmrItem");

	return 0;
}

void CEmrTreeWnd::EnsureAndPopupDetail(CEMNDetail *pDetail)
{
	// (j.jones 2005-12-15 10:56) - find which topic has this detail,
	// create it if necessary, and popup the detail

	FOR_ALL_ROWS(m_pTree) {
		EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
		if(etrt == etrtTopic) {
			CEMRTopic *pTopic = EmrTree::ChildRow(pRow).GetTopic();
			if(pTopic) {
				for(int i=0;i<pTopic->GetEMNDetailCount();i++) {
					if(pTopic->GetDetailByIndex(i) == pDetail) {
						//found it, now ensure the topic and pop up the detail
						EnsureTopicRow(pRow);

						pDetail->Popup();
						return;
					}
				}
			}
		}
		GET_NEXT_ROW(m_pTree)
	}	
}

CEmrTopicWndPtr CEmrTreeWnd::EnsureTopicRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	//DRT 8/21/2007 - PLID 26655 - Must check for NULL 
	if(pRow == NULL) {
		AfxThrowNxException("CEmrTreeWnd::EnsureTopicRow called with an empty row.");
	}

	CEMRTopic* pTopic = EmrTree::ChildRow(pRow).GetTopic();
	CEmrTopicWndPtr pTopicWnd = EmrTree::ChildRow(pRow).GetTopicWnd();
	if(pTopicWnd) {
		return pTopicWnd;
	}

	//DRT 8/21/2007 - PLID 26655 - Must check for NULL pointer.
	if(pTopic == NULL) {
		AfxThrowNxException("CEmrTreeWnd::EnsureTopicRow - Could not find topic.");
	}

	//DRT 8/21/2007 - PLID 26655 - Must check for NULL pointer.  If we can't determine the locked status, we must throw
	//	an exception, for safety.
	if(pTopic->GetParentEMN() == NULL) {
		AfxThrowNxException("CEmrTreeWnd::EnsureTopicRow - Could not find parent EMN.");
	}

	BOOL bLocked = (pTopic->GetParentEMN()->GetStatus() == 2);
	// (c.haag 2006-03-28 12:13) - PLID 19890 - We now consider permissions when initializing topic windows
	BOOL bCanWriteToEMR = (m_bIsTemplate) ? TRUE :
		CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE);

	// (a.walling 2007-11-28 11:25) - PLID 28044 - Check if expired
	if (g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
		bCanWriteToEMR = FALSE;
	}

	// (a.wetta 2006-11-16 09:42) - PLID 19474 - If this topic is a spawned topic on a template, then it should also be read only
	BOOL bSpawnedTemplateTopic = (m_bIsTemplate && pTopic->GetSourceActionID() != -1) ? TRUE : FALSE;
	
	// (a.walling 2008-06-03 17:55) - PLID 23138
	BOOL bIsReadOnly = !(pTopic->GetParentEMN()->IsWritable());

	// (a.walling 2011-10-26 10:21) - PLID 46175 - Infinite topic window! - Call the view to show the topic
	pTopicWnd = GetEmrFrameWnd()->EnsureEmrTopicView(pTopic->GetParentEMN())->EnsureTopic(pTopic);
	
	//Ensure the color while we're at it.
	EnsureTopicBackColor(pTopic);

	//This node has now been opened.
	EnsureModifiedState(pRow);

	return pTopicWnd;
}

CEmrTopicWndPtr CEmrTreeWnd::EnsureTopicView(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	CEmrTopicWndPtr pTopicWnd = EnsureTopicRow(pRow);

	if (pTopicWnd && pTopicWnd->GetTopic()) {
		//(e.lally 2012-03-16) PLID 48933 - We don't want to force the topic view to be the active tab
		//	because the preview pane might be open and the preferred active one.
		//	we just want the tree and topic view to reflect the new active topic.
		GetEmrFrameWnd()->ActivateTopic(pTopicWnd->GetTopic());
	}

	return pTopicWnd;
}

IRowSettingsPtr CEmrTreeWnd::FindInTreeByColumn(short nColIndex,
        const _variant_t & varValue,
        IDispatch * pStartRow)
{
	//
	// (c.haag 2007-09-28 11:34) - PLID 27509 - This encapsulates FindByColumn for the tree.
	// Returns NULL if the row wasn't found or if the tree is NULL.
	//
	if (NULL == m_pTree) {
		// The tree is NULL. Nothing we can do.
		return NULL;
	}
	else {
		return m_pTree->FindByColumn(nColIndex, varValue, pStartRow, VARIANT_FALSE);
	}
}

// (a.walling 2007-04-11 15:33) - PLID 25548 - Needed to be able to search the datalist for subtopics
			// Ensures that the topic row has loaded its subtopics, recursively throughout subtopics.
void CEmrTreeWnd::EnsureTopicRowLoadedAllSubTopics(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(pRow);

		LoadSubtopics(pRow);

		NXDATALIST2Lib::IRowSettingsPtr pChildRow = pRow->GetFirstChildRow();

		while (pChildRow) {
			if(!EmrTree::ChildRow(pChildRow).IsLoaded()) {
				// (a.walling 2007-10-10 10:22) - PLID 25548 - Recursively call on the child
				// This used to just call LoadSubtopics(pRow), which was incorrect because
				// it does not load for the subtopics, AND because we want to use the childrow!
				EnsureTopicRowLoadedAllSubTopics(pChildRow);			
			}

			pChildRow = pChildRow->GetNextRow();
		}

		EnsureModifiedState(pRow);
	}NxCatchAll("Error in CEmrTreeWnd::LoadSubtopics()");
}

LRESULT CEmrTreeWnd::OnMergeOverrideChanged(WPARAM wParam, LPARAM lParam)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		CEMNDetail *pChangedDetail = (CEMNDetail*)wParam;
		MergeOverrideChanged *pMoc = (MergeOverrideChanged*)lParam;
		pChangedDetail->m_pParentTopic->GetParentEMN()->UpdateMergeConflicts(pMoc->strOldName);
		pChangedDetail->m_pParentTopic->GetParentEMN()->UpdateMergeConflicts(pMoc->strNewName);

		
		// (a.walling 2008-06-12 16:44) - PLID 27301 - Ensure the display is updated
		pChangedDetail->ReflectCurrentState();

		IRowSettingsPtr pTopicRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)(pChangedDetail->m_pParentTopic), NULL, FALSE);
		if(pTopicRow) {
			EnsureModifiedState(pTopicRow);
		}

		// (a.walling 2008-06-02 16:31) - PLID 27716 - Update the preview pane
		if (GetEmrPreviewCtrl()) {
			UpdateDetail(pChangedDetail);
		}
	} NxCatchAll("Error in OnMergeOverrideChanged");

	return 0;
}

// (a.walling 2007-05-16 09:35) - PLID 25092
// Mark topics that we can't drag to
LRESULT CEmrTreeWnd::OnEmnDetailDragBegin(WPARAM wParam, LPARAM lParam)
{
	try {
		m_bDragging = TRUE;
		if (!m_bIsTemplate)
			return 0;

		CEmrItemAdvDlg* pAdvDlg = (CEmrItemAdvDlg*)wParam;
		ASSERT(pAdvDlg);
		if (pAdvDlg) {
			IRowSettingsPtr pRow = m_pTree->FindAbsoluteFirstRow(VARIANT_FALSE);
			while (pRow) {
				EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();

				if (etrt == etrtTopic) {
					CEMRTopic* pTopic = EmrTree::ChildRow(pRow).GetTopic();

					if (pTopic) {
						BOOL bSpawnedTemplateTopic = (m_bIsTemplate && pTopic->GetSourceActionID() != -1);

						if (bSpawnedTemplateTopic) {
							// this topic cannot be dragged to. So save its color to our map.
							OLE_COLOR color = pRow->GetBackColor();
							IRowSettings* pDerefRow = pRow;
							pDerefRow->AddRef();
							m_mapTopicColors[pDerefRow] = color;
							pRow->PutBackColor(RGB(192,192,192));
						}
					}
				}

				pRow = m_pTree->FindAbsoluteNextRow(pRow, VARIANT_FALSE);
			}
		}
	}NxCatchAll("Error in CEmrTreeWnd::OnEmnDetailDragBegin");

	return 0;
}

LRESULT CEmrTreeWnd::OnEmnDetailDragEnd(WPARAM wParam, LPARAM lParam)
{
	try {		
		GetEmrFrameWnd()->GetActiveEmrTopicView()->ForceRefreshTopicWindowPositions();
		m_bDragging = FALSE;
		// (a.walling 2007-05-16 10:30) - PLID 25092
		// restore the topic colors we modified
		try {
			POSITION pos = m_mapTopicColors.GetStartPosition();
			IRowSettings* pRow;
			OLE_COLOR color;
			while (pos != NULL)
			{
				m_mapTopicColors.GetNextAssoc(pos, pRow, color);

				ASSERT(pRow);

				if (pRow) {
					pRow->PutBackColor(color);
					pRow->Release();
				}
			}

			m_mapTopicColors.RemoveAll();
		} NxCatchAll("Error restoring colors in OnEmnDetailDragEnd");

		// (c.haag 2006-06-27 08:31) - PLID 20845 - Now we have to save all of the unsaved topics
		// that were affected by the drag operation
		// (a.walling 2007-04-25 13:00) - PLID 25549 - Use the ItemAdvDlg passed in wParam to get the EMN id
		CEmrItemAdvDlg* pAdvDlg = (CEmrItemAdvDlg*)wParam;
		if (pAdvDlg) {			
			// (a.walling 2012-04-02 08:29) - PLID 49304 - Removed a lot of dead code regarding moving items

			// (a.walling 2007-08-23 12:08) - PLID 27017
			if(pAdvDlg->m_pDetail) {
				if (GetEmrPreviewCtrl()) {
					// (a.walling 2008-11-17 14:50) - PLID 32060 - Only check for subdetail parents if we are actually a subdetail
					CEMNDetail* pSubDetailParent = pAdvDlg->m_pDetail->IsSubDetail() ? pAdvDlg->m_pDetail->GetSubDetailParent() : NULL;
					if (pSubDetailParent != NULL) {
						GetEmrPreviewCtrl()->UpdateDetail(pSubDetailParent);
					} else {
						CEMNDetail* pNextDetail = pAdvDlg->m_pDetail->GetNextDetail();
						GetEmrPreviewCtrl()->MoveDetail(pAdvDlg->m_pDetail, pNextDetail);
					}
				}
			}
		}

	}NxCatchAll("Error in CEmrTreeWnd::OnEmnDetailDragEnd");

	return 0;
}

void CEmrTreeWnd::AddNewTopic(BOOL bAddAsSubtopic /*= FALSE*/)
{
	try {
		IRowSettingsPtr pRow = m_pTree->CurSel;
		if(pRow) {
			CString strTopicName;
			// (a.walling 2007-02-20 10:53) - PLID 24487 - The limit for topic names is actually 200, but I'm leaving wiggle room
			if(IDOK != InputBoxLimited(this, "Please enter a name for the new topic", strTopicName, "",180,false,false,NULL)) return;
			IRowSettingsPtr pAddedRow;
			//Do we have a parent?
			IRowSettingsPtr pParent = pRow->GetParentRow();
			if(pParent == NULL || bAddAsSubtopic) {
				EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
				if(etrt == etrtEmn) {
					// Top level topic from the EMN row
					LoadSubtopics(pRow);
					CEMN *pEMN = EmrTree::ChildRow(pRow).GetEMN();
					//TES 4/15/2010 - PLID 24692 - Pass in -1 for the topic ID
					CEMRTopic *pNewTopic = pEMN->AddTopic(strTopicName, -1);
					// (a.walling 2007-10-17 14:31) - PLID 27017 - We are creating a blank topic, so as far as we are concerned
					// the post load is complete. This way IsPostLoadComplete is valid even for manually added topics.
					pNewTopic->SetLoaded();
					IRowSettingsPtr pLastChild = pRow->GetLastChildRow();
					//TES 2/26/2014 - PLID 60748 - Need to add before the Codes topic
					ASSERT(pLastChild);
					if(pLastChild) {
						ASSERT(EmrTree::ChildRow(pLastChild).GetType() == etrtMoreInfo);
						pLastChild = pLastChild->GetPreviousRow();
						ASSERT(pLastChild && EmrTree::ChildRow(pLastChild).GetType() == etrtCodes);
					}
					pAddedRow = AddTopicRow(pNewTopic, pRow, pLastChild);

					// (a.walling 2007-04-09 15:38) - PLID 25549
					if (GetEmrPreviewCtrl()) { // insert a top level topic at the end of the EMN
						GetEmrPreviewCtrl()->InsertTopic(pNewTopic);
					}
				}
				else if(etrt == etrtTopic) {
					// first level subtopic
					CEMRTopic *pTopic = EmrTree::ChildRow(pRow).GetTopic();
					//TES 4/15/2010 - PLID 24692 - Pass in -1 for the topic ID
					CEMRTopic *pNewTopic = pTopic->AddSubTopic(strTopicName, -1);
					// (a.walling 2007-10-17 14:31) - PLID 27017 - We are creating a blank topic, so as far as we are concerned
					// the post load is complete. This way IsPostLoadComplete is valid even for manually added topics.
					pNewTopic->SetLoaded();
					pAddedRow = AddTopicRow(pNewTopic, pRow);

					// (a.walling 2007-04-09 15:38) - PLID 25549
					if (GetEmrPreviewCtrl()) { // insert a sub topic at the end of the list of subtopics
						GetEmrPreviewCtrl()->InsertSubTopic(pTopic, pNewTopic);
					}
				}
			}
			else {
				//What type is the parent?
				EmrTreeRowType etrt = EmrTree::ChildRow(pParent).GetType();
				if(etrt == etrtTopic) {
					// new topic while subtopic is selected
					CEMRTopic *pTopic = EmrTree::ChildRow(pParent).GetTopic();
					//TES 4/15/2010 - PLID 24692 - Pass in -1 for the topic ID
					CEMRTopic *pNewTopic = pTopic->AddSubTopic(strTopicName, -1);
					// (a.walling 2007-10-17 14:31) - PLID 27017 - We are creating a blank topic, so as far as we are concerned
					// the post load is complete. This way IsPostLoadComplete is valid even for manually added topics.
					pNewTopic->SetLoaded();
					pAddedRow = AddTopicRow(pNewTopic, pParent);

					// (a.walling 2007-04-09 15:38) - PLID 25549
					if (GetEmrPreviewCtrl()) { // insert a sub topic at the end of the list of subtopics
						GetEmrPreviewCtrl()->InsertSubTopic(pTopic, pNewTopic);
					}
				}
				else if(etrt == etrtEmn) {
					// adding new topic while first level topic is selected (ie, parent is the EMN)
					CEMN *pEMN = EmrTree::ChildRow(pParent).GetEMN();
					//TES 4/15/2010 - PLID 24692 - Pass in -1 for the topic ID
					CEMRTopic *pNewTopic = pEMN->AddTopic(strTopicName, -1);
					// (a.walling 2007-10-17 14:31) - PLID 27017 - We are creating a blank topic, so as far as we are concerned
					// the post load is complete. This way IsPostLoadComplete is valid even for manually added topics.
					pNewTopic->SetLoaded();
					IRowSettingsPtr pLastChild = pParent->GetLastChildRow();
					//TES 2/26/2014 - PLID 60748 - Need to add before the Codes topic
					ASSERT(pLastChild);
					if(pLastChild) {
						ASSERT(EmrTree::ChildRow(pLastChild).GetType() == etrtMoreInfo);
						pLastChild = pLastChild->GetPreviousRow();
						ASSERT(pLastChild && EmrTree::ChildRow(pLastChild).GetType() == etrtCodes);
					}
					pAddedRow = AddTopicRow(pNewTopic, pParent, pLastChild);

					// (a.walling 2007-04-09 15:38) - PLID 25549
					if (GetEmrPreviewCtrl()) { // insert a top level topic at the end of the EMN
						GetEmrPreviewCtrl()->InsertTopic(pNewTopic);
					}
				}
				else {
					//MoreInfo and Placeholders can't be parents!
					ASSERT(FALSE);
					return;
				}
			}
			ASSERT(pAddedRow != NULL);
			SetTreeSel(pAddedRow);
			EnsureModifiedState(pAddedRow);
		}
		else {
			//Hmm, I'm not sure what to do in this case.  Let's do nothing for now.
		}
	} NxCatchAll("Error in AddNewTopic");
}

// (c.haag 2008-06-17 12:38) - PLID 17842 - Optional parameter to return the added items
void CEmrTreeWnd::AddItems(const CDWordArray &arInfoMasterIDs, CArray<CEMNDetail*,CEMNDetail*>* papNewDetails /*= NULL */)
{
	IRowSettingsPtr pEmnRow = m_pTree->CurSel;
	if(pEmnRow != NULL)
	{
		CEMN* pEmn = EmrTree::ChildRow(pEmnRow).GetEMN();
		if(pEmn != NULL)
		{
			//First, get our current topic.
			CEmrTopicView* pTopicView = GetEmrFrameWnd()->GetEmrTopicView(pEmn, false);
			if(pTopicView != NULL)
			{
				// (z.manning 2012-06-29 17:34) - PLID 49547 - I changed this function to use the active topic
				// rather than the current selection in the tree so that it still works even if the EMN row is 
				// selected in the tree.
				CEMRTopic *pActiveTopic = pTopicView->GetActiveTopic();
				CEmrTopicWndPtr pActiveTopicWnd = pTopicView->GetActiveTopicWnd();
				if(pActiveTopic != NULL && pActiveTopicWnd != NULL)
				{
					for(int i = 0; i < arInfoMasterIDs.GetSize(); i++) {
						//(e.lally 2006-03-20) PLID 19751 - We want to add the new detail to the topic first, then load it.
							//the Add New Detail From ID function was created to do this.
						SourceActionInfo saiBlank; // (z.manning 2009-03-05 09:52) - PLID 33338
						CEMNDetail* pDetail = pActiveTopic->AddNewDetailFromEmrInfoMasterID(arInfoMasterIDs[i], m_bIsTemplate, saiBlank);
						pActiveTopicWnd->AddDetail(pDetail);
						if (NULL != papNewDetails) { papNewDetails->Add(pDetail); }
					}
					EnsureTopicBackColor(pActiveTopic);
					EnsureTopicModifiedState(pActiveTopic);
				}
			}
		}
	}
}

void CEmrTreeWnd::OnEditingStartingTree(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		// (c.haag 2006-03-28 12:13) - PLID 19890 - We now consider permissions when initializing topic windows
		BOOL bCanWriteToEMR = (m_bIsTemplate) ? TRUE :
			CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE);
	
		// (a.walling 2007-11-28 11:25) - PLID 28044 - Check if expired
		if (g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
			bCanWriteToEMR = FALSE;
		}

		ASSERT(nCol == TREE_COLUMN_NAME);
		if(nCol == TREE_COLUMN_NAME) {
			IRowSettingsPtr pRow(lpRow);
			EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
			if(etrt == etrtEmn) {
				//Don't include the * that indicates that it's unsaved.
				CEMN *pEMN = EmrTree::ChildRow(pRow).GetEMN();
				GetEmrFrameWnd()->ActivateEMNOnly(pEMN);
				// (a.walling 2008-06-04 09:33) - PLID 23138
				BOOL bWritable = pEMN->IsWritable();
				if(pEMN->GetStatus() == 2 || !bCanWriteToEMR || !bWritable) { // locked
					*pbContinue = FALSE;
				} else {
					// (a.walling 2010-08-16 17:08) - PLID 40131 - Fix leak and crash
					VariantClear(pvarValue);
					*pvarValue = _variant_t(pEMN->GetDescription()).Detach();
					*pbContinue = TRUE;
				}
			}
			else if(etrt == etrtTopic) {
				//Don't include the * that indicates that it's unsaved.
				CEMRTopic *pTopic = EmrTree::ChildRow(pRow).GetTopic();
				GetEmrFrameWnd()->ActivateTopic(pTopic);
				// (a.walling 2008-06-04 09:33) - PLID 23138
				BOOL bWritable = pTopic->GetParentEMN()->IsWritable();
				if (pTopic->GetParentEMN()->GetStatus() == 2 || !bCanWriteToEMR || !bWritable) { // locked
					*pbContinue = FALSE;
				} else if (pTopic->IsTemplate() && pTopic->GetSourceActionID() > -1) {
					// (c.haag 2009-12-18 09:08) - PLID 29327 - Disallow renaming spawned template topics
					*pbContinue = FALSE;
					AfxMessageBox("You may not rename spawned template topics.", MB_ICONSTOP | MB_OK);
				} else {
					// (a.walling 2010-08-16 17:08) - PLID 40131 - Fix leak and crash
					VariantClear(pvarValue);
					*pvarValue = _variant_t(pTopic->GetName()).Detach();
					*pbContinue = TRUE;
				}
			}
			else {
				*pbContinue = FALSE;
			}
		}
	} NxCatchAll("Error in OnEditingStartingTree");
}

void CEmrTreeWnd::OnEditingFinishingTree(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		ASSERT(nCol == TREE_COLUMN_NAME);
		if(*pbCommit && nCol == TREE_COLUMN_NAME) {
			IRowSettingsPtr pRow(lpRow);
			//TODO: Validate what they entered.

			// (a.wetta 2006-10-26 10:39) - If the names are the same, do not commit. 
			// Additionally, make sure that if the only difference is the number that we
			// add onto a topic's name to indicate that it is a duplicate (i.e. "History (1)"), that we don't commit this.
			// At this point if the user hasn't changed anything it compares what is in the topic name right now ("History")
			// to what was there before ("History (1)") and thinks there was a change.  But in OnEditingFinishedTree this "(1)"
			// is added back on, so the user sees no change, but it will be marked with a red astrix.  The same goes for
			// the EMN name.

			// (a.walling 2007-02-20 10:40) - PLID 24487 - Validate the name lengths. Using exact maximum lengths. Handle templates.
			EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
			if(etrt == etrtTopic) {
				CString strNew = strUserEntered;
				if (strNew.GetLength() > 200) {
					*pbCommit = FALSE;
					long nResult = MessageBox(FormatString("The name of the topic is too long. Would you like to truncate the last %li characters?",
						 strNew.GetLength() - 200), "Practice", MB_YESNOCANCEL);
					if (nResult == IDYES) {
						strNew = strNew.Left(200);
						_variant_t varNew((LPCTSTR)strNew);

						if (pvarNewValue->vt != VT_EMPTY) {
							VariantClear(pvarNewValue);
						}
						*pvarNewValue = varNew.Detach();

						*pbCommit = TRUE;
					} else if (nResult == IDNO) {
						*pbContinue = FALSE;
					} else if (nResult == IDCANCEL) {
						// don't need to do anything since pbCommit is already FALSE.
					} else {ASSERT(FALSE);}
				} else {
					CEMRTopic *pTopic = EmrTree::ChildRow(pRow).GetTopic();
					if (pTopic->GetName() == strUserEntered) {
						*pbCommit = FALSE;
					}
				}
			}
			else if(etrt == etrtEmn) {
				CString strNew = VarString(*pvarNewValue, "");
				CEMN *pEMN = EmrTree::ChildRow(pRow).GetEMN();
				long nMaxLength = m_bIsTemplate ? 200 : 255; // maximum of 200 for EMRTemplateT, 255 for EMRMasterT
				if (strNew.GetLength() > nMaxLength) {
					*pbCommit = FALSE;
					long nResult = MessageBox(FormatString("The description of the EMN is too long. Would you like to truncate the last %li characters?",
						 strNew.GetLength() - nMaxLength), "Practice", MB_YESNOCANCEL);
					if (nResult == IDYES) {
						strNew = strNew.Left(nMaxLength);
						_variant_t varNew((LPCTSTR)strNew);

						if (pvarNewValue->vt != VT_EMPTY) {
							VariantClear(pvarNewValue);
						}
						*pvarNewValue = varNew.Detach();

						*pbCommit = TRUE;
					} else if (nResult == IDNO) {
						*pbContinue = FALSE;
					} else if (nResult == IDCANCEL) {
						// don't need to do anything since pbCommit is already FALSE.
					} else {ASSERT(FALSE);}
				}
				else {

					if(pEMN->GetDescription() == strUserEntered) {
						*pbCommit = FALSE;
					}
				}
			}
			else {
				*pbCommit = FALSE;
			}
		}
	} NxCatchAll("Error in OnEditingFinishingTree");
}

void CEmrTreeWnd::OnEditingFinishedTree(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		//Comment this back in once the datalist is fixed.
		ASSERT(nCol == TREE_COLUMN_NAME);
		if(bCommit && nCol == TREE_COLUMN_NAME) {
			IRowSettingsPtr pRow(lpRow);
			CString strNewName = VarString(varNewValue);
			EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
			if(etrt == etrtEmn) {
				CEMN *pEMN = EmrTree::ChildRow(pRow).GetEMN();
				pEMN->SetDescription(strNewName);

				if (GetEmrPreviewCtrl()) {
					// (a.walling 2007-10-12 09:07) - PLID 25548 - Ensure this is for the correct EMN
					if (pEMN == GetEmrPreviewCtrl()->GetCurrentEMN()) {
						GetEmrPreviewCtrl()->UpdateEMNTitle(strNewName);
					}
				}

				// (a.walling 2012-03-23 18:01) - PLID 50638 - Description auto updates on idle, no need to notify the more info dialog etc

				pRow->PutValue(TREE_COLUMN_NAME, _bstr_t(GetEmnRowDisplayText(pEMN)));
			}
			else if(etrt == etrtTopic) {
				// Set the topic's new name
				CEMRTopic *pTopic = EmrTree::ChildRow(pRow).GetTopic();
				pTopic->SetName(strNewName);

				if (GetEmrPreviewCtrl()) {
					GetEmrPreviewCtrl()->UpdateTopicTitle(pTopic, strNewName);
				}

				// (a.wetta 2006-10-26 10:14) - Now, check to make sure that there aren't any duplicate topic names
				IRowSettingsPtr pEmnRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pTopic->GetParentEMN(), NULL, FALSE);
				IRowSettingsPtr pTopicRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pTopic, pEmnRow, FALSE);
				if(pTopicRow) {
					CEMRTopic *pParentTopic = pTopic->GetParentTopic();
					if(pParentTopic) {
						IRowSettingsPtr pParentRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pParentTopic, pEmnRow, FALSE);
						if(pParentRow) {
							CheckDuplicateTopics(pParentRow);
						}
					}
					else {
						if (pEmnRow) {
							CheckDuplicateTopics(pEmnRow);
						}
					}
				}
			}
			else {
				//This shouldn't be editable!
				ASSERT(FALSE);
			}
			EnsureModifiedState(pRow);
		}
	} NxCatchAll("Error in OnEditingFinishedTree");
}

void CEmrTreeWnd::OnDragBeginTree(BOOL FAR* pbShowDrag, LPDISPATCH lpRow, short nCol, long nFlags)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		IRowSettingsPtr pRow(lpRow);
		EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
		// (c.haag 2006-04-03 15:42) - PLID 19890 - Don't let read-only users rearrange topics
		// (a.walling 2007-11-28 11:26) - PLID 28044 - Check if expired
		if (!CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE) || (g_pLicense->HasEMR(CLicense::cflrSilent) != 2)) {
			*pbShowDrag = FALSE;
		}
		
		if(etrt == etrtTopic) {
			// (j.jones 2006-08-18 11:49) - PLID 21888 - don't allow rearranging on locked EMNs
			// (a.wetta 2006-11-16 11:13) - PLID 19474 - Also, don't allow rearranging on subtopics of spawned topics on templates
			CEMRTopic *pTopic = EmrTree::ChildRow(pRow).GetTopic();
			// (a.walling 2008-06-04 09:34) - PLID 23138
			BOOL bWritable = pTopic->GetParentEMN()->IsWritable();
			if(pTopic->GetParentEMN()->GetStatus() == 2 || (m_bIsTemplate && pTopic->GetParentTopic() != NULL && pTopic->GetParentTopic()->GetSourceActionID() != -1) || !bWritable) {
				*pbShowDrag = FALSE;
			}
			else {
				*pbShowDrag = TRUE;
			}

		} else {
			*pbShowDrag = FALSE;
		}

		// (c.haag 2007-09-27 12:09) - PLID 27509 - If the drag source is different from the tree selection, then
		// we must change the tree selection to the drag source for two reasons: The user can see the details of 
		// the topic they are dragging, and so HandleSelChanged won't "skip a beat".
		if (pRow != m_pTree->CurSel) {
			SetTreeSel(pRow);
		}

		m_lpDraggingRow = lpRow;
	} NxCatchAll("Error in OnDragBeginTree");
}

#define IDT_DRAG_HOVER	1000
// (c.haag 2007-09-26 17:17) - PLID 27509 - We no longer use this timer
//#define IDT_CHECK_SEL_CHANGE	1001

void CEmrTreeWnd::OnDragOverCellTree(BOOL FAR* pbShowDrop, LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags)
{
	try {
		KillTimer(IDT_DRAG_HOVER);
		IRowSettingsPtr pToRow(lpRow);
		IRowSettingsPtr pFromRow(lpFromRow);
		ClearDragPlaceholders(pToRow);

		*pbShowDrop = IsValidDrag(pFromRow, pToRow);

		if(*pbShowDrop) {
			EmrTreeRowType etrtFrom = EmrTree::ChildRow(pFromRow).GetType();
			EmrTreeRowType etrtTo = EmrTree::ChildRow(pToRow).GetType();
			if(etrtFrom == etrtTopic) {
				if(etrtTo == etrtTopic || etrtTo == etrtEmn) {
					BOOL bNoVisibleChildren = TRUE;
					IRowSettingsPtr pChildRow = pToRow->GetFirstChildRow();
					while (bNoVisibleChildren && pChildRow != NULL) {
						if (pChildRow->Visible)
							bNoVisibleChildren = FALSE;
						pChildRow = pChildRow->GetNextRow();
					}
					if(!bNoVisibleChildren && pToRow->Expanded == FALSE) {
						pToRow->Expanded = TRUE;
						OnRowExpandedTree(pToRow);
					}
					else {
						if(etrtTo == etrtTopic) {
							//This is a leaf node.  But maybe they want to make the row they're dragging a child.  Start a timer, if
							//they hover long enough, we'll make this node into a parent.
							SetTimer(IDT_DRAG_HOVER, 1000, NULL);
						}
					}
				}
			}
		}
	}
	NxCatchAll("Error in CEmrTreeWnd::OnDragOverCellTree()");
}

BOOL CEmrTreeWnd::IsValidDrag(NXDATALIST2Lib::IRowSettings *pFromRow, NXDATALIST2Lib::IRowSettings *pToRow)
{
	if(!pFromRow || !pToRow) return FALSE;
	// (c.haag 2006-04-03 15:42) - PLID 19890 - Don't let read-only users rearrange topics
	// (a.walling 2007-11-28 11:27) - PLID 28044 - Also check if expired
	if (!CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE)  || (g_pLicense->HasEMR(CLicense::cflrSilent) != 2)) {
		return FALSE;
	}
	EmrTreeRowType etrtFrom = EmrTree::ChildRow(pFromRow).GetType();
	EmrTreeRowType etrtTo = EmrTree::ChildRow(pToRow).GetType();
	if(etrtFrom == etrtTopic) {

		// (j.jones 2006-08-18 11:49) - PLID 21888 - don't allow rearranging on locked EMNs
		// (a.wetta 2006-11-16 11:13) - PLID 19474 - Also, don't allow rearranging on subtopics of spawned topics on templates
		CEMRTopic *pFromTopic = EmrTree::ChildRow(pFromRow).GetTopic();
		if(pFromTopic->GetParentEMN()->GetStatus() == 2 || (m_bIsTemplate && pFromTopic->GetParentTopic() != NULL && pFromTopic->GetParentTopic()->GetSourceActionID() != -1)) {
			return FALSE;
		}
		// (a.walling 2008-06-27 09:24) - PLID 22049 - Cannot drag to/from non-writable topics
		if(pFromTopic != NULL && pFromTopic->GetParentEMN() != NULL && !pFromTopic->GetParentEMN()->IsWritable()) {
			return FALSE;
		}

		// (a.wetta 2006-11-16 11:47) - PLID 19474 - They are trying to drag onto a spawned topic of a template, which is illegal
		CEMRTopic *pToTopic = EmrTree::ChildRow(pToRow).GetTopic();
		if(m_bIsTemplate && pToTopic != NULL && pToTopic->GetParentTopic() != NULL && pToTopic->GetParentTopic()->GetSourceActionID() != -1) {
			return FALSE;
		}
		// (a.walling 2008-06-27 09:24) - PLID 22049 - Cannot drag to/from non-writable topics
		if(pToTopic != NULL && pToTopic->GetParentEMN() != NULL && !pToTopic->GetParentEMN()->IsWritable()) {
			return FALSE;
		}

		if(etrtTo == etrtTopic || etrtTo == etrtPlaceholder) {
			//This is only valid if they share the same top-level parent (EMN).
			IRowSettingsPtr pTopFromParent = pFromRow;
			while(pTopFromParent->GetParentRow()) pTopFromParent = pTopFromParent->GetParentRow();
			IRowSettingsPtr pTopToParent = pToRow;
			while(pTopToParent->GetParentRow()) {
				if(pTopToParent == pFromRow) {
					//They are attempting to drag a topic onto one of its subtopics, which is illegal.
					return FALSE;
				}
				pTopToParent = pTopToParent->GetParentRow();
			}
			if(pTopFromParent == pTopToParent) {
				return TRUE;
			}
			else {
				return FALSE;
			}
		}
		else {
			return FALSE;
		}
	}
	else {
		return FALSE;
	}
}

void CEmrTreeWnd::OnDragEndTree(LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags)
{
	try {
		KillTimer(IDT_DRAG_HOVER);
		m_lpDraggingRow = NULL;
		IRowSettingsPtr pRow(lpRow);
		IRowSettingsPtr pFromRow(lpFromRow);
		if(IsValidDrag(pFromRow, pRow)) {
			//If the from and to rows are siblings, and the row is moving down, then insert after, because the target row will
			//shift up when the source row is removed.
			IRowSettingsPtr pDestRow;
			if(pFromRow->GetParentRow() == pRow->GetParentRow() && m_pTree->IsRowEarlierInList(pFromRow, pRow)) {
				pDestRow = pRow->GetNextRow();
			}
			else {
				pDestRow = pRow;
			}

			IRowSettingsPtr pRowAdded;
			if(pDestRow) {
				pRowAdded = m_pTree->AddRowBefore(pFromRow, pDestRow);
			}
			else {
				pRowAdded = m_pTree->AddRowAtEnd(pFromRow, pRow->GetParentRow());
			}
			//Now, tell the old topic's parent to let go of it, and add it to the new parent.
			CEMRTopic *pOldTopic = EmrTree::ChildRow(pFromRow).GetTopic();
			CEMRTopic *pParentTopic = pOldTopic->GetParentTopic();
			if(pParentTopic) {
				pParentTopic->DetachSubTopic(pOldTopic);
				// (z.manning, 08/24/2006) - PLID 22148 - We do this here because if a subtopic was moved
				// such that an empty topic now has no subtopics, we need to possibly hide that topic 
				// (if not, then it will just sit there until it is chosen at which point it will disappear).
				EnsureTopicBackColor(pParentTopic);
			}
			else {
				CEMN *pParentEmn = pOldTopic->GetParentEMN();
				pParentEmn->DetachTopic(pOldTopic);
			}
			//Add to the new parent.
			IRowSettingsPtr pParentRow = pRow->GetParentRow();
			CEMRTopic *pInsertBeforeTopic = NULL;
			if(pDestRow) {
				EmrTreeRowType etrtDest = EmrTree::ChildRow(pDestRow).GetType();
				if(etrtDest == etrtTopic) {
					pInsertBeforeTopic = EmrTree::ChildRow(pDestRow).GetTopic();
					pInsertBeforeTopic->GetParentEMN()->SetDetailsHaveMoved(TRUE);
				}
			}
			EmrTreeRowType etrt = EmrTree::ChildRow(pParentRow).GetType();
			if(etrt == etrtEmn) {
				CEMN* pEmn = EmrTree::ChildRow(pParentRow).GetEMN();
				//TES 10/5/2009 - PLID 35755 - This was a manual change, so we want to re-calculate topic order indexes.
				pEmn->InsertTopic(pOldTopic, pInsertBeforeTopic, FALSE, TRUE);
				pEmn->SetDetailsHaveMoved(TRUE);

				// (a.walling 2007-04-09 16:21) - PLID 25549
				if (GetEmrPreviewCtrl()) {
					GetEmrPreviewCtrl()->MoveTopic(pOldTopic, pInsertBeforeTopic);
				}
			}
			else if(etrt == etrtTopic) {
				CEMRTopic *pTopic = EmrTree::ChildRow(pParentRow).GetTopic();
				//TES 10/5/2009 - PLID 35755 - This was a manual change, so we want to re-calculate topic order indexes.
				pTopic->InsertSubTopic(pOldTopic, pInsertBeforeTopic, FALSE, TRUE);
				pTopic->GetParentEMN()->SetDetailsHaveMoved(TRUE);

				// (a.walling 2007-04-09 16:21) - PLID 25549
				if (GetEmrPreviewCtrl()) {
					GetEmrPreviewCtrl()->MoveTopicToSubTopic(pOldTopic, pTopic, pInsertBeforeTopic);
				}
			}
			else {
				ASSERT(FALSE);
			}

			// (j.jones 2006-10-23 14:29) - PLID 22053 - tell the topic it has moved
			pOldTopic->SetHasMoved(TRUE);

			//Now that we're done dragging, make sure our topic area is up-to-date.
			// (c.haag 2007-09-27 11:37) - PLID 27509 - Dragging is a special case where do
			// not want to call SetTreeSel. The reason is that m_pTree->CurSel is not equal to
			// pFromRow, and HandleSelChanged intentionally ignores mid-drag selection changes.
			m_pTree->CurSel = pRowAdded;
			HandleSelChanged(pFromRow, pRowAdded);

			//The topic that moved, its current parent, and its old parent, all may now be modified.
			EnsureModifiedState(pRowAdded);
			EnsureModifiedState(pRowAdded->GetParentRow());
			EnsureModifiedState(pFromRow);
			EnsureModifiedState(pFromRow->GetParentRow());

			RemoveTreeRow(pFromRow);

			EnsureTopicBackColor(pOldTopic);

			// (a.walling 2008-02-06 14:17) - PLID 28391 - Ensure visiblity state is up to date when moving around topics
			if (pOldTopic) {
				pOldTopic->RefreshHTMLVisibility();
			}
			if (pParentTopic) {
				pParentTopic->RefreshHTMLVisibility();
			}
			
		}
		else {
			//Set the CurSel back to the original row.
			//TES 12/5/2006 - PLID 21425 - This was setting pOldRow to the CurSel, but the CurSel was the row being dragged over,
			// and therefore was never displayed on screen (as you can see in the HandleSelChanged code).  We want to make sure
			// that pOldRow is the currently displayed row, so it can be properly hidden if necessary.
			// (c.haag 2007-09-27 11:37) - PLID 27509 - We now ensure in OnDragBegin that pFromRow is indeed the 
			// row being currently displayed. So, all we should have to do is set m_pTree->CurSel to pFrom so
			// that it's consistent with the window content...but keep Tom's code in place for safety purposes.
			if (m_pTree->CurSel != pFromRow) {
				m_pTree->CurSel = pFromRow;
			}
		}
		ClearDragPlaceholders();
	}NxCatchAll("Error in CEmrTreeWnd::OnDragEndTree");
}

void CEmrTreeWnd::OnTimer(UINT nIDEvent)
{
	try {
		switch(nIDEvent) {
		case IDT_PREVIEW_REFRESH:
			{
				// (a.walling 2008-07-03 18:00) - PLID 29114 - Kill the timer first thing, otherwise it is possible it might get fired again!
				KillTimer(IDT_PREVIEW_REFRESH);

				// (a.walling 2008-05-14 16:24) - PLID 29114 - Timer has fired, which means it is time to update the preview pane!
				POSITION pos = m_mapDetailsPendingUpdate.GetStartPosition();
				while (pos) {
					CEMNDetail* pDetail;
					BOOL bDummy;

					m_mapDetailsPendingUpdate.GetNextAssoc(pos, pDetail, bDummy);

					ASSERT(pDetail);
					if (pDetail) {						
						//TRACE("**>> Refreshing      0x%08x (%s) update at %lu\n", pDetail, pDetail->GetLabelText(), GetTickCount());
						UpdateDetail(pDetail);
						//pDetail->Release();
						// (a.walling 2009-10-12 16:05) - PLID 36024
						pDetail->__Release("CEmrTreeWnd::OnTimer post update");
					}
				}

				m_mapDetailsPendingUpdate.RemoveAll();
				break;
			}

		case IDT_DRAG_HOVER:
			KillTimer(IDT_DRAG_HOVER);
			ASSERT(m_lpDraggingRow);
			if(m_lpDraggingRow) {
				IRowSettingsPtr pRow = m_pTree->CurSel;
				//We need to make sure that a placeholder only appears if the topic has no visible children
				BOOL bNoVisibleChildren = TRUE;
				IRowSettingsPtr pChildRow = pRow->GetFirstChildRow();
				while (bNoVisibleChildren && pChildRow != NULL) {
					if (pChildRow->Visible)
						bNoVisibleChildren = FALSE;
					pChildRow = pChildRow->GetNextRow();
				}
				if(bNoVisibleChildren) {
					//We need to insert a placeholder child row.
					// (a.wetta 2006-11-21 09:29) - PLID 19474 - First check to make sure that this isn't a spawned topic on a template, if it is then don't make
					// a placeholder because you can't drag a topic here.
					CEMRTopic *pToTopic = EmrTree::ChildRow(pRow).GetTopic();
					if(!m_bIsTemplate || pToTopic->GetSourceActionID() == -1) {
						m_arDragPlaceholders.Add(InsertPlaceholder(pRow));
						pRow->Expanded = TRUE;
					}
				}
			}
			break;

		case IDT_FORCE_ACCESS_TIMER:
			{
				// (a.walling 2008-07-03 15:17) - PLID 30498 - Iterate through any items that we are waiting on and 
				// post a message to handle them.
				
				KillTimer(IDT_FORCE_ACCESS_TIMER);

				POSITION pos = m_mapWaitingOnModalLoop.GetStartPosition();

				while (pos) {
					long nID;
					BOOL bDummy;

					m_mapWaitingOnModalLoop.GetNextAssoc(pos, nID, bDummy);

					if (nID > 0) {
						PostMessage(NXM_EMN_FORCE_ACCESS_REQUEST, nID, 0);
					}
				}
			}
			break;

		// (c.haag 2007-09-26 17:16) - PLID 27509 - This case is obselete
		/*case IDT_CHECK_SEL_CHANGE:
			{
				//TES 12/5/2006 - PLID 21425 - This means that we got a message that the datalist's selection changed.  We need
				// to check whether we have updated ourselves yet to reflect the new selection, and if not, do so.
				KillTimer(IDT_CHECK_SEL_CHANGE);
				m_bExpectingSelChange = FALSE;
				
				//TES 1/2/2007 - PLID 21425 - One problem: m_pCurrentlyDisplayedRow might have been removed from the list.
				// If so, trying to save it (which HandleSelChanged might do) will cause all kinds of problems, and anyway
				// the code should be able to assume that nobody's going to go around accessing rows that have already been
				// removed.  So, if the m_pCurrentlyDisplayedRow isn't in the list, don't pass it into HandleSelChanged().
				BOOL bFound = FALSE;
				FOR_ALL_ROWS(m_pTree) {
					if(pRow == m_pCurrentlyDisplayedRow) bFound = TRUE;
					GET_NEXT_ROW(m_pTree);
				}
				if(!bFound) {
					m_pCurrentlyDisplayedRow = NULL;
				}
				//Is the selection still out of whack?
				if(m_pTree->CurSel != m_pCurrentlyDisplayedRow) {
					HandleSelChanged(m_pCurrentlyDisplayedRow, m_pTree->CurSel);
				}
			}
			break;*/
		}
	}NxCatchAll("Error in CEmrTreeWnd::OnTimer()");
}

void CEmrTreeWnd::ClearDragPlaceholders(NXDATALIST2Lib::IRowSettings *pRowToPreserve /*= NULL*/)
{
	for(int i = m_arDragPlaceholders.GetSize()-1; i >= 0; i--) {
		if(m_arDragPlaceholders[i] != pRowToPreserve) {
			RemoveTreeRow(m_arDragPlaceholders[i]);
			m_arDragPlaceholders.RemoveAt(i);
		}
	}
}

void CEmrTreeWnd::OnRButtonDownTree(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		if (!lpRow) return;

		// (a.walling 2012-04-05 13:11) - PLID 49472 - Moved context menu handling to WM_CONTEXTMENU, but still set the sel
		SetTreeSel(lpRow);
	} NxCatchAll(__FUNCTION__);
}

void CEmrTreeWnd::OnOpenEmn()
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		// (j.jones 2011-07-05 11:56) - PLID 43603 - this is now a class
		EMNStatus emnStatus;
		emnStatus.nID = 0;
		emnStatus.strName = "Open";
		ChangeStatus(emnStatus, m_pTree->CurSel);
	} NxCatchAll("Error in OnOpenEMN");
}

void CEmrTreeWnd::OnFinishEmn()
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		// (j.jones 2011-07-05 11:56) - PLID 43603 - this is now a class
		EMNStatus emnStatus;
		emnStatus.nID = 1;
		emnStatus.strName = "Finished";
		ChangeStatus(emnStatus, m_pTree->CurSel);
	} NxCatchAll("Error in OnFinishEMN");
}

void CEmrTreeWnd::OnLockEmn()
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		// (z.manning 2009-08-11 15:22) - PLID 24277 - Use lock permission
		if (!CheckCurrentUserPermissions(bioPatientEMR, sptDynamic3)) {
			return;
		}

		// (j.jones 2011-07-05 11:56) - PLID 43603 - this is now a class
		EMNStatus emnStatus;
		emnStatus.nID = 2;
		emnStatus.strName = "Locked";
		ChangeStatus(emnStatus, m_pTree->CurSel);
	} NxCatchAll("Error in OnLockEMN");
}

void CEmrTreeWnd::OnSaveRow()
{
	try {
		SaveRowObject(m_pTree->CurSel, FALSE);
	} NxCatchAll("Error in OnSaveRow");
}

void CEmrTreeWnd::OnDeleteEmn()
{
	try {
		ASSERT(!m_bIsTemplate);
		if(m_bIsTemplate) return;
		// (c.haag 2006-04-03 16:57) - PLID 19890 - Check user permissions
		// (j.jones 2006-09-13 09:24) - PLID 22493 - removed bioPatientEMR delete permission,
		// it is now administrator-only
		// (a.walling 2007-11-28 13:11) - PLID 28044 - Also check for a valid current EMR license
		if(!IsCurrentUserAdministrator() || (g_pLicense->HasEMR(CLicense::cflrSilent) != 2)) {
			return;
		}
		
		IRowSettingsPtr pEmnRow = m_pTree->CurSel;
		ASSERT(pEmnRow != NULL);
		if(pEmnRow) {
			ASSERT(etrtEmn == EmrTree::ChildRow(pEmnRow).GetType());
			CEMN* pEMN = EmrTree::ChildRow(pEmnRow).GetEMN();

			// (j.jones 2009-10-01 11:36) - PLID 30479 - added a specific dialog
			// to prompt for deletion confirmation
			CDeleteEMNConfirmDlg dlg(ecdtEMN, pEMN->GetDescription(), this);
			if(dlg.DoModal() != DELETE_EMN_RETURN_DELETE) {
				//unless the return value specifically says to delete, leave now
				return;
			}

			//DRT 3/13/2006 - PLID 19687 - Do not allow delete if the EMN is locked.
			if(pEMN->GetStatus() == 2) {
				MessageBox("Cannot delete a locked procedure.");
				return;
				// (c.haag 2008-07-24 10:03) - PLID 30826 - Now check permissions and use the new way of detecting only saved problems
			} else if (!CanCurrentUserDeleteEmrProblems() && pEMN->DoesEmnOrChildrenHaveSavedProblems()) {
				MessageBox("You cannot delete this EMN because one or more topics in the EMN are associated with problem details.", NULL, MB_OK | MB_ICONERROR);
				return;
			}
			
			// (a.walling 2010-07-27 16:40) - PLID 39433 - Check for in-progress recording
			// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
			if (GetPicContainer()) {
				CAudioRecordDlg* pCurrentAudioRecordDlgInstance = CAudioRecordDlg::GetCurrentInstance();
				if (pCurrentAudioRecordDlgInstance != NULL && pCurrentAudioRecordDlgInstance->GetPatientID() == GetPatientID() && pCurrentAudioRecordDlgInstance->GetPicID() == GetPicContainer()->GetCurrentPicID() && pCurrentAudioRecordDlgInstance->GetEmn() == pEMN) {
					if (IDYES == MessageBox("A recording is in progress for this EMN. Continuing will disassociate the recording from this EMN, although it may still be attached to the EMR. Do you want to continue?", NULL, MB_ICONQUESTION | MB_YESNO))
					{
						pCurrentAudioRecordDlgInstance->ResetEmn();
					} else {
						return;
					}
				}
			}

			// (j.jones 2008-07-29 13:24) - PLID 30729 - track if this EMN has any problems within it
			CArray<CEmrProblem*, CEmrProblem*> aryProblems;
			pEMN->GetAllProblems(aryProblems);
			BOOL bHasProblems = aryProblems.GetSize() > 0;

			// (c.haag - 12/27/04 10:27 AM) - PLID 14424 - Before we delete the EMN, get the
			// list of all procedures associated only with this EMN.
			CArray<long,long> aryProcedureIDs;
			for(int i=0; i<pEMN->GetProcedureCount(); i++) {
				aryProcedureIDs.Add(pEMN->GetProcedure(i)->nID);				
			}

			//TES 6/5/2008 - PLID 30196 - Check if this EMN has either the Current Medications or Allergies table on it, if
			// so, we need to warn them (though only if the EMN was ever saved).
			BOOL bHasCurrentMeds = FALSE;
			BOOL bHasAllergies = FALSE;
			if(pEMN->GetID() != -1) {
				bHasCurrentMeds = pEMN->HasSystemTable(eistCurrentMedicationsTable);
				bHasAllergies = pEMN->HasSystemTable(eistAllergiesTable);
			}


			// (a.walling 2012-03-14 09:47) - PLID 46638 - Destroy more info when EMN is deleted
			if (CEmrMoreInfoView* pMoreInfoView = GetEmrFrameWnd()->GetEmrMoreInfoView(pEMN, false)) {
				pMoreInfoView->GetParentFrame()->SendMessage(WM_CLOSE);
			}
			//TES 2/12/2014 - PLID 60740 - Added for the new <Codes> topic
			if (CEmrCodesView* pCodesView = GetEmrFrameWnd()->GetEmrCodesView(pEMN, false)) {
				pCodesView->GetParentFrame()->SendMessage(WM_CLOSE);
			}
			
			// (a.walling 2012-03-30 10:49) - PLID 49329 - Destroy the topic view when EMN is deleted
			if (CEmrTopicView* pTopicView = GetEmrFrameWnd()->GetEmrTopicView(pEMN, false)) {
				pTopicView->GetParentFrame()->SendMessage(WM_CLOSE);
			}

			//Now remove from the memory object.
			m_EMR.RemoveEMN(pEMN);

			// (a.walling 2007-09-25 14:19) - PLID 25548 - Remove from the preview
			if (GetEmrPreviewCtrl()) {
				GetEmrPreviewCtrl()->RemoveEMN(pEMN);
			}

			// (a.walling 2010-08-24 15:39) - PLID 37923 - Clear out the source action info and reset the EMN Spawning Text
			{			
				for (int i = 0; i < m_EMR.GetEMNCount(); i++) {
					CEMN* pCheckEMN = m_EMR.GetEMN(i);

					SourceActionInfo sai = pCheckEMN->GetSourceActionInfo();
					if (!sai.IsBlank() &&
						(
						(sai.pSourceDetail != NULL && pEMN->GetDetailByPointer((long)sai.pSourceDetail) != NULL) ||
						(sai.nSourceDetailID != -1 && pEMN->GetDetailByID(sai.nSourceDetailID) != NULL)
						))
					{
						pCheckEMN->ClearSourceActionInfo();
					}
				}
			}

			//Now tell the PIC which procedures were removed
			CWnd *pWnd = GetParent();
			if(IsWindow(pWnd->GetSafeHwnd())) {
				for(int i=0; i<aryProcedureIDs.GetSize(); i++) {
					pWnd->SendMessage(NXM_PRE_DELETE_EMN_PROCEDURE, (WPARAM)aryProcedureIDs.GetAt(i), (LPARAM)pEMN);
					// (z.manning, 10/05/2007) - PLID 27630 - We now have a post-removal message as well.
					pWnd->SendMessage(NXM_POST_DELETE_EMN_PROCEDURE, (WPARAM)aryProcedureIDs.GetAt(i), (LPARAM)pEMN);
				}
			}

			// (j.jones 2008-07-29 13:26) - PLID 30729 - if we had problems, send the message
			// that the problems changed
			if(bHasProblems) {
				SendMessage(NXM_EMR_PROBLEM_CHANGED);
			}

			// (j.jones 2008-07-29 13:26) - PLID 30729 - if we had problems, send the message
			// that the problems changed
			if(bHasProblems) {
				SendMessage(NXM_EMR_PROBLEM_CHANGED);
			}

			//Remove the row.
			RemoveTreeRow(pEmnRow);

			if(bHasCurrentMeds || bHasAllergies) {
				//TES 6/5/2008 - PLID 30196 - Warn them to check the medications tab.
				CString strTables;
				if(bHasCurrentMeds && bHasAllergies) {
					strTables = "'Current Medications' and 'Allergies' tables";
				}
				else if(bHasCurrentMeds) {
					strTables = "'Current Medications' table";
				}
				else if(bHasAllergies) {
					strTables = "'Allergies' table";
				}

				MessageBox(FormatString("The deleted EMN contained the %s.  Please review this patient's Medications tab to make sure that "
					"its corresponding information is up to date.", strTables));
			}
		}
	} NxCatchAll("Error in OnDeleteEmn");
}

// (j.jones 2007-09-11 15:36) - PLID 27352 - added unique function for AddTopic vs. AddSubtopic
void CEmrTreeWnd::OnAddNewTopic()
{
	try {
		AddNewTopic(FALSE);
	} NxCatchAll("Error in OnAddNewTopic");
}

void CEmrTreeWnd::OnAddNewSubtopic()
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		AddNewTopic(TRUE);
	} NxCatchAll("Error in OnAddNewSubtopic");
}

// (a.walling 2012-04-02 08:29) - PLID 49304 - Removed a lot of dead code regarding moving items


// (a.walling 2012-03-29 08:04) - PLID 49297 - OnRequestClipRect is dead code

void CEmrTreeWnd::OnSelChangingTree(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		if(m_lpDraggingRow){
			IRowSettingsPtr pDraggingRow(m_lpDraggingRow);
			IRowSettingsPtr pNewRow(*lppNewSel);
			if(!IsValidDrag(pDraggingRow, pNewRow)) {
				// (b.cardillo 2006-11-16 15:53) - PLID 23265 - Need to do reference counting properly
				SafeSetCOMPointer(lppNewSel, lpOldSel);
			}
		} else if (*lppNewSel == NULL) {
			// (b.cardillo 2006-02-07 10:49) - PLID 19168 - Just made it so when the user clicks in 
			// the empty space in the tree (i.e. not on any node) it just leaves the selection as is.
			// (b.cardillo 2006-11-16 15:53) - PLID 23265 - Need to do reference counting properly
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
		else {
			//TES 2/27/2006 - If we're on an EMN, and this is a topic with no details, but has subtopics, don't show it.
			// (a.walling 2012-07-05 14:39) - PLID 51284 - With the infinite topic window UI, this does not really make as much sense any longer
			//if(!m_bIsTemplate) {
			//	IRowSettingsPtr pNewRow(*lppNewSel);
			//	if(etrtTopic == EmrTree::ChildRow(pNewRow).GetType()) {
			//		CEMRTopic *pTopic = EmrTree::ChildRow(pNewRow).GetTopic();
			//		if(!pTopic->HasDetails() && pTopic->HasSubTopics()) {
			//			// (b.cardillo 2006-11-17 14:59) - PLID 23265 - Need to do reference counting 
			//			// properly.  Notice this specifies which template implementation of the 
			//			// SafeSetCOMPointer() function to use, because otherwise the compiler can't 
			//			// figure it out.  Could just as easily have cast the CalcNextRow() return 
			//			// value as (IDispatch *) before passing it in, but this seemed a bit cleaner.
			//			SafeSetCOMPointer<IDispatch>(lppNewSel, CalcNextRow(pNewRow));
			//		}
			//	}
			//}
		}

		// (a.walling 2012-06-28 17:11) - PLID 51276 - More Info should be a clickable link
		//TES 2/19/2014 - PLID 60750 - Same for Codes
		if (lppNewSel && *lppNewSel && EmrTree::ChildRow(*lppNewSel).IsMoreInfoOrCodes()) {
			if (!lpOldSel || (lpOldSel && !EmrTree::ChildRow(lpOldSel).IsMoreInfoOrCodes())) {
				SafeSetCOMPointer(lppNewSel, lpOldSel);
			} else {
				SafeSetCOMPointer(lppNewSel, (LPDISPATCH)NULL);
			}
		}
	} NxCatchAll("Error in OnSelChangingTree");
}

void CEmrTreeWnd::OnDeleteTopic()
{
	try {
		IRowSettingsPtr pTopicRow = m_pTree->CurSel;
		ASSERT(pTopicRow != NULL);

		if(pTopicRow) {
			ASSERT(etrtTopic == EmrTree::ChildRow(pTopicRow).GetType());
			CEMRTopic *pTopic = EmrTree::ChildRow(pTopicRow).GetTopic();
			CEMN* pEMN = pTopic->GetParentEMN();
			// (c.haag 2008-07-24 12:07) - PLID 30826 - Check permissions and saved problems only
			if (!CanCurrentUserDeleteEmrProblems() && pTopic->DoesTopicOrChildrenHaveSavedProblems()) {
				MessageBox("You cannot delete this topic because the topic or its items are associated with one or more saved problems.", NULL, MB_OK | MB_ICONERROR);
				return;
			}

			if(pTopic->HasSubTopics() || pTopic->HasDetails()) {
				if(IDYES != MessageBox("Are you sure you wish to PERMANENTLY delete this topic, as well as any subtopics, and any details on this topic or any of its subtopics?  This action cannot be undone!", NULL, MB_YESNO)) return;
			}

			// (j.jones 2008-07-29 13:24) - PLID 30729 - track if this topic has any problems within it
			// this will include data problems
			CArray<CEmrProblem*, CEmrProblem*> aryProblems;
			pTopic->GetAllProblems(aryProblems);
			BOOL bHasProblems = aryProblems.GetSize() > 0;

			//Remove from the memory object.
			IRowSettingsPtr pParentRow = pTopicRow->GetParentRow();
			EmrTreeRowType etrt = EmrTree::ChildRow(pParentRow).GetType();

			//DRT 8/3/2007 - PLID 26932 - RemoveTopic no longer handles its own unspawning, we want to use the 
			//	Unspawner utility.  Do this before we remove the rows.
			//Our array of details to be removed includes those on the topic AND all those on subtopics.
			CArray<CEMNDetail*, CEMNDetail*> aryDetailsToBeRemoved;
			pTopic->GenerateEMNDetailArray(&aryDetailsToBeRemoved);

			CEMNUnspawner eu(pTopic->GetParentEMN());
			eu.RemoveActionsByDetails(&aryDetailsToBeRemoved);

			if(etrt == etrtEmn) {
				(EmrTree::ChildRow(pParentRow).GetEMN())->RemoveTopic(pTopic);
			}
			else {
				ASSERT(etrt == etrtTopic);
				CEMRTopic *pParentTopic = EmrTree::ChildRow(pParentRow).GetTopic();
				//DRT 8/9/2007 - PLID 26876 - New function to remove subtopics
				pParentTopic->RemoveSubTopic_External(pTopic);
			}

			EnsureModifiedState(pParentRow);

			// (j.jones 2008-07-29 13:25) - PLID 30729 - if we had problems, send the message
			// that the problems changed
			if(bHasProblems) {
				SendMessage(NXM_EMR_PROBLEM_CHANGED);
			}

			RemoveTreeRow(pTopicRow);
			
			// (a.walling 2012-03-30 10:48) - PLID 49329 - Notify the topic view that the tree has changed
			if (CEmrTopicView* pTopicView = GetEmrFrameWnd()->GetEmrTopicView(pEMN, false)) {
				pTopicView->HandleTreeChanged();
			}
						
			// (a.walling 2007-10-22 13:32) - PLID 27017 - Ensure the topic is removed
			if (GetEmrPreviewCtrl()) {
				GetEmrPreviewCtrl()->RemoveTopic(pTopic);
			}
		}
	} NxCatchAll(FormatString("Error in OnDeleteTopic()"));
}

LRESULT CEmrTreeWnd::OnEmrTopicAdded(WPARAM wParam, LPARAM lParam)
{
	try {
		if(!m_bInitialLoadComplete) {
			//We haven't loaded the tree yet, so don't do anything, and when we load the tree we'll load the new topic with it.
			return 0;
		}

		CEMRTopic *pTopic = (CEMRTopic*)wParam;
		// (c.haag 2007-08-22 10:38) - PLID 25881 - The lParam tells us whether we should write
		// to the preview control
		BOOL bSuppressFromPreviewCtrl = (LPARAM)lParam;

		//DRT 6/30/2006 - PLID 21310 - Extra error checking!
		if(pTopic == NULL) {
			//There really is no way this should be able to happen, but if it does, we're pretty much screwed.
			AfxThrowNxException("Error in OnEmrTopicAdded:  Topic parameter is NULL.");
		}
		
		// (a.walling 2007-12-17 17:24) - PLID 28391
		if (!bSuppressFromPreviewCtrl) {
			pTopic->RefreshHTMLVisibility();
		}

		//DRT 6/30/2006 - PLID 21310 - We got a generic "Invalid Pointer" error here once.  Unsure if the problem was
		//	the line above, or if this pTopic parameter was actually not a real CEMRTopic.  We never reproduced
		//	the problem, so if we get it again here, it is very likely that the problem is this line.  I had a sample
		//	try / catch(...) around this, but did not feel comfortable leaving that in place in a release.
		CEMN* pParentEMN = pTopic->GetParentEMN();

		//First, do we already have this topic?
		IRowSettingsPtr pEmnRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pTopic->GetParentEMN(), NULL, FALSE);
		IRowSettingsPtr pTopicRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pTopic, pEmnRow, FALSE);
		if(pTopicRow) {
			//Yup, show it.
			SetTreeRowVisible(pTopicRow, TRUE);
			// (a.wetta 2006-07-07 10:23) - PLID 21363 - If this is a spawned topic we need to check to see if it needs
			// to be renamed because it is a duplicate.
			//Find this topic's parent.
			CEMRTopic *pParentTopic = pTopic->GetParentTopic();
			if(pParentTopic) {
				IRowSettingsPtr pParentRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pParentTopic, pEmnRow, FALSE);
				if(pParentRow) {
					CheckDuplicateTopics(pParentRow);
				}
			}
			else {
				if (pEmnRow) {
					CheckDuplicateTopics(pEmnRow);
				}
			}
			return 0;
		}
		else {
			//Find this topic's parent.
			CEMRTopic *pParentTopic = pTopic->GetParentTopic();
			if(pParentTopic) {
				IRowSettingsPtr pParentRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pParentTopic, pEmnRow, FALSE);
				if(pParentRow) {
					if(EmrTree::ChildRow(pParentRow).IsLoaded()) {
						//TES 2/2/2006 - Find the right place to add it.
						CEMRTopic *pNextSibling = NULL;
						for(int i = 0; i < pParentTopic->GetSubTopicCount()-1 && !pNextSibling; i++) {
							if(pParentTopic->GetSubTopic(i) == pTopic)
								pNextSibling = pParentTopic->GetSubTopic(i+1);
						}
						IRowSettingsPtr pNextRow;
						if(pNextSibling) pNextRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pNextSibling, pEmnRow, FALSE);
						AddTopicRow(pTopic, pParentRow, pNextRow);

						// (a.walling 2007-04-10 09:56) - PLID 25549
						// (c.haag 2007-08-22 10:38) - PLID 25881 - Check bSuppressFromPreviewCtrl
						// to see whether we should update the preview window
						if (GetEmrPreviewCtrl() && !bSuppressFromPreviewCtrl) {
							GetEmrPreviewCtrl()->InsertSubTopicBefore(pParentTopic, pNextSibling, pTopic);
						}

						//Make sure all parents are visible.
						while(pParentRow) {
							SetTreeRowVisible(pParentRow, TRUE);
							pParentRow = pParentRow->GetParentRow();
						}
						return 0;
					}
					else {
						//This will be added when the subtopics are loaded.
						return 0;
					}
				}
				else {
					//The parent hasn't even been loaded yet, this will be there when it is.
					return 0;
				}
			}
			else { // Top level topic
				if(EmrTree::ChildRow(pEmnRow).IsLoaded()) {
					//TES 2/2/2006 - Find the right place to add it.
					CEMRTopic *pNextSibling = NULL;
					CEMN *pEMN = pTopic->GetParentEMN();
					for(int i = 0; i < pEMN->GetTopicCount()-1 && !pNextSibling; i++) {
						if(pEMN->GetTopic(i) == pTopic)
							pNextSibling = pEMN->GetTopic(i+1);
					}
					IRowSettingsPtr pNextRow;
					if(pNextSibling) pNextRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pNextSibling, pEmnRow, FALSE);
					if(pNextRow) {
						AddTopicRow(pTopic, pEmnRow, pNextRow);
						// (a.walling 2007-04-10 09:56) - PLID 25549
						// (c.haag 2007-08-22 10:38) - PLID 25881 - Check bSuppressFromPreviewCtrl
						// to see whether we should update the preview window
						if (GetEmrPreviewCtrl() && !bSuppressFromPreviewCtrl) {
							GetEmrPreviewCtrl()->InsertTopicBefore(pNextSibling, pTopic);
						}
					}
					else {
						//Add before the more info row.
						//TES 2/26/2014 - PLID 60748 - Add it before the Codes row instead
						NXDATALIST2Lib::IRowSettingsPtr pLastChild = pEmnRow->GetLastChildRow();
						ASSERT(pLastChild);
						if(pLastChild) {
							ASSERT(EmrTree::ChildRow(pLastChild).GetType() == etrtMoreInfo);
							pLastChild = pLastChild->GetPreviousRow();
							ASSERT(pLastChild && EmrTree::ChildRow(pLastChild).GetType() == etrtCodes);
						}
						AddTopicRow(pTopic, pEmnRow, pLastChild);
						// (a.walling 2007-04-10 09:56) - PLID 25549
						// (c.haag 2007-08-22 10:38) - PLID 25881 - Check bSuppressFromPreviewCtrl
						// to see whether we should update the preview window
						if (GetEmrPreviewCtrl() && !bSuppressFromPreviewCtrl) {
							GetEmrPreviewCtrl()->InsertTopic(pTopic); // inserts as very last topic
						}
					}
					return 0;
				}
				else {
					//This will be added when the topics are loaded.
					return 0;
				}
			}
		}
	} NxCatchAll("Error in OnEmrTopicAdded");

	return 0;
}

LRESULT CEmrTreeWnd::OnEmrTopicAddPreview(WPARAM wParam, LPARAM lParam)
{
	//
	// (c.haag 2007-05-31 12:45) - PLID 26175 - This message is sent from CEMN::ProcessEmrActions
	// after spawning EMR items from a template. The purpose of this message is to populate the
	// HTML preview pane. Most of this code was copied from OnEmrTopicAdded.
	//
	try {
		// (a.walling 2007-10-18 14:17) - PLID 25548 - Ignore if this is not for our EMN
		// (a.walling 2007-10-26 09:25) - PLID 25548 - If this is sent from an EMR, LPARAM will be NULL, that is fine. We discard any 
		// non-relevant topics in AddTopicPreview anyway.
		if (GetEmrPreviewCtrl() != NULL && ( (lParam == NULL) || (GetEmrPreviewCtrl()->GetCurrentEMN() == (CEMN*)lParam) )) {
			CArray<CEMRTopic*,CEMRTopic*>* papTopics = (CArray<CEMRTopic*,CEMRTopic*>*)wParam;

			AddTopicPreview(papTopics, FALSE);
		}		
	} NxCatchAll("Error in CEmrTreeWnd::OnEmrTopicAddPreview");

	return 0;
}

// (a.walling 2007-10-15 12:16) - PLID 25548 - The spawning is entirely complete, if we have any pending topics then we need to assert.
LRESULT CEmrTreeWnd::OnEmrSpawningComplete(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2007-10-18 14:17) - PLID 25548 - Ignore if this is not for our EMN

		CEMN* pMessageEMN = (CEMN*)wParam;
		if ((GetEmrPreviewCtrl() != NULL) && GetEmrPreviewCtrl()->GetCurrentEMN() == pMessageEMN) {
			if (m_aryTopicsPendingInsertion.GetSize() > 0) {
				AddTopicPreview(&m_aryTopicsPendingInsertion, TRUE);

				m_aryTopicsPendingInsertion.RemoveAll();
			}
		}
	} NxCatchAll("Error in CEmrTreeWnd::OnEmrTopicMintSpawnComplete");

	return 0;
}

// (a.walling 2007-10-15 12:41) - PLID 25548 - Shared function for adding new topic previews from spawning
// (a.walling 2012-07-09 17:40) - PLID 50862 - Modified to not rely on the tree for the UI state of topics -- using GetNextSibling now, much easier
void CEmrTreeWnd::AddTopicPreview(CArray<CEMRTopic*,CEMRTopic*>* papTopics, BOOL bFinal)
{
	if (NULL == papTopics) {
		ThrowNxException("A NULL topic array was passed in!");
	}
	else if (NULL == GetEmrPreviewCtrl()) {
		// No point in doing anything without a preview control
		return;
	}

	if (!GetEmrPreviewCtrl()->IsLoaded()) {
		// preview pane has not even loaded yet
		return;
	}

	// (a.walling 2007-10-09 14:01) - PLID 25548 - Copy and sort the topic array so we add them in order
	CArray<CEMRTopic*,CEMRTopic*> apTopics;
	apTopics.Copy(*papTopics);

	// (a.walling 2007-10-26 09:26) - PLID 25548 - Clean out any topics not for this EMN (m_pEMRPreviewCtrlDlg is guaranteed not null by the caller)
	CEMN* pCurrentDisplayedEMN = GetEmrPreviewCtrl()->GetCurrentEMN();
	for (int h = apTopics.GetSize() - 1; h >= 0; h--) {
		if (apTopics[h]->GetParentEMN() != pCurrentDisplayedEMN) {
			apTopics.RemoveAt(h);
		}
	}

	// (a.walling 2007-10-10 09:49) - PLID 25548 - Now we also have to go through and remove any subtopics
	// of any parent topics in this list. Otherwise we could add the parent topic, which also adds the subtopics,
	// and then go through and add the subtopics again!
	for (int i = 0; i < papTopics->GetSize(); i++) {
		CEMRTopic* pPossibleParent = papTopics->GetAt(i);
		for (int j = apTopics.GetSize() - 1; j >= 0; j--) {
			CEMRTopic* pPossibleChild = apTopics.GetAt(j);
			CEMRTopic* pParentCandidate = pPossibleChild->GetParentTopic();

			while (pParentCandidate) {
				if (pParentCandidate == pPossibleParent) {
					apTopics.RemoveAt(j);
					//LogDetail("* (new count:%li) Removing %s(0x%08x), is child of %s(0x%08x) via %s(0x%08x).", apTopics.GetSize(), pPossibleChild->GetName(), pPossibleChild, pParentCandidate->GetName(), pParentCandidate, pPossibleParent->GetName(), pPossibleParent);
					pParentCandidate = NULL;
				} else {
					pParentCandidate = pParentCandidate->GetParentTopic();
				}
			}
		}

		// (a.walling 2007-10-15 12:19) - PLID 25548 - Also clean out anything from m_aryTopicsPendingInsertion
		if (!bFinal) {
			for (int k = m_aryTopicsPendingInsertion.GetSize() - 1; k >= 0; k--) {
				// (a.walling 2007-10-18 14:23) - PLID 25548 - This used to be the wrong array
				CEMRTopic* pPossibleChild = m_aryTopicsPendingInsertion.GetAt(k);
				CEMRTopic* pParentCandidate = pPossibleChild->GetParentTopic();

				while (pParentCandidate) {
					if (pParentCandidate == pPossibleParent) {
						m_aryTopicsPendingInsertion.RemoveAt(k);
						//LogDetail("* (new count:%li) Removing pending topic %s(0x%08x), is child of %s(0x%08x) via %s(0x%08x).", m_aryTopicsPendingInsertion.GetSize(), pPossibleChild->GetName(), pPossibleChild, pParentCandidate->GetName(), pParentCandidate, pPossibleParent->GetName(), pPossibleParent);
						pParentCandidate = NULL;
					} else {
						pParentCandidate = pParentCandidate->GetParentTopic();
					}
				}
			}
		}
	}
	
	// now set papTopics to equal our clean array, and sort it.
	papTopics = &apTopics;
	SortTopicArray(*papTopics);

	CArray<CEMRTopic*, CEMRTopic*> arTopicsToRefreshHTMLVisibility;

	// (a.walling 2007-10-09 14:04) - PLID 25548 - One of the problems with the original incarnation of this
	// function was that child topics were depending on their parent topics already existing in the preview.
	// So we loop through, and remove topics from the list as they are successfully added into the preview. If
	// we ever loop through once without successfully inserting any topics, then we know we are at some kind of
	// deadlock, so we assert and revert to the safe behaviour of adding the topics at the very end of the list.
	long nPostCount = papTopics->GetSize();
	long nPreCount = papTopics->GetSize();
	long index = 0;
	BOOL bLoop = TRUE;
	while (bLoop) {
		nPreCount = papTopics->GetSize();
		index = 0;
		for (; index < papTopics->GetSize(); index++) {
			CEMRTopic* pTopic = papTopics->GetAt(index);

			// (a.walling 2007-10-11 17:44) - PLID 25548 - Don't bother with messages not for this EMN
			CEMN* pDisplayedEMN = GetEmrPreviewCtrl()->GetCurrentEMN();
			if (pTopic->GetParentEMN() != pDisplayedEMN) {
				papTopics->RemoveAt(index);
				index--;
				break;
			}

			CEMRTopic *pParentTopic = pTopic->GetParentTopic();
			if(pParentTopic) {
				if (!IsTopicInArray(arTopicsToRefreshHTMLVisibility, pParentTopic)) {
					arTopicsToRefreshHTMLVisibility.Add(pParentTopic);
				}

				//TES 2/2/2006 - Find the right place to add it.
				CEMRTopic *pNextSibling = pTopic->GetNextSiblingTopic();

				// (a.walling 2007-04-10 09:56) - PLID 25549
				if (GetEmrPreviewCtrl()->InsertSubTopicBefore(pParentTopic, pNextSibling, pTopic, TRUE)) {
					//LogDetail("* Inserted topic %s(0x%08x), child of %s(0x%08x), next sibling %s(0x%08x)", pTopic->GetName(), pTopic, pParentTopic->GetName(), pParentTopic, pNextSibling->GetName(), pNextSibling);
					// (a.walling 2007-10-10 08:54) - PLID 25548
					// (a.walling 2012-07-09 12:35) - PLID 51441 - IsEmpty now has some non-optional parameters
					GetEmrPreviewCtrl()->ShowTopic(pTopic, pTopic->ShowIfEmpty() || !pTopic->IsEmpty(NULL, FALSE));
					papTopics->RemoveAt(index);
					index--;
				} else {
					//LogDetail("* Failed to insert topic %s(0x%08x), waiting on parent %s(0x%08x) or next sibling %s(0x%08x)", pTopic->GetName(), pTopic, pParentTopic->GetName(), pParentTopic, pNextSibling->GetName(), pNextSibling);
				}
			}
			else { // Top level topic
				//TES 2/2/2006 - Find the right place to add it.
				CEMRTopic *pNextSibling = pTopic->GetNextSiblingTopic();

				if (pNextSibling) {
					// (a.walling 2007-04-10 09:56) - PLID 25549
					if (GetEmrPreviewCtrl()->InsertTopicBefore(pNextSibling, pTopic, TRUE)) {
						//LogDetail("* Inserted topic %s(0x%08x), child of ROOT, next sibling %s(0x%08x)", pTopic->GetName(), pTopic, pNextSibling->GetName(), pNextSibling);
						// (a.walling 2007-10-10 08:54) - PLID 25548
						papTopics->RemoveAt(index);
						index--;
					} else {
						//LogDetail("* Failed to insert topic %s(0x%08x), waiting on next sibling %s(0x%08x)", pTopic->GetName(), pTopic, pNextSibling->GetName(), pNextSibling);
					}
				}
				else {
					// (a.walling 2007-04-10 09:56) - PLID 25549
					GetEmrPreviewCtrl()->InsertTopic(pTopic); // inserts as very last topic
					//LogDetail("* Inserted topic %s(0x%08x), child of ROOT, next sibling (none)", pTopic->GetName(), pTopic);
					// (a.walling 2007-10-10 08:54) - PLID 25548
					papTopics->RemoveAt(index);
					index--;
				}
			}
		}
		nPostCount = papTopics->GetSize();

		if ( (nPostCount == nPreCount) || (papTopics->GetSize() == 0) ) {
			bLoop = FALSE;
		}
	}

	// (a.walling 2007-10-10 09:19) - PLID 25548 - Anything that remains in this array, we cannot find their siblings or parents.
	// (a.walling 2007-10-15 12:18) - PLID 25548 - Put these topics into the TopicsPendingInsertion array for now
	if (!bFinal) {
		if (papTopics->GetSize()) {
			for (int j = 0; j < papTopics->GetSize(); j++) {
				m_aryTopicsPendingInsertion.Add(papTopics->GetAt(j));
				//LogDetail("* (new count:%li) Topic %s(0x%08x) added to pending insertion array", m_aryTopicsPendingInsertion.GetSize(), papTopics->GetAt(j)->GetName(), papTopics->GetAt(j));
			}
		}
	}

	if (bFinal) {
		if (papTopics->GetSize()) {
			ASSERT(FALSE); // some topics will not necessarily be positioned correctly.
			
			for (int i=0; i < papTopics->GetSize(); i++) {
				CEMRTopic* pTopic = papTopics->GetAt(i);
				CEMRTopic* pParentTopic = pTopic->GetParentTopic();
				if (pParentTopic) {
					// don't be strict so this will always add to the preview
					// might raise an assertion however
					GetEmrPreviewCtrl()->InsertSubTopic(pParentTopic, pTopic, FALSE);
					//LogDetail("! Topic %s(0x%08x) blindly inserted into preview with parent %s(0x%08x)", pTopic->GetName(), pTopic, pParentTopic->GetName(), pParentTopic);
				} else {
					// top level
					GetEmrPreviewCtrl()->InsertTopic(pTopic);
					//LogDetail("! Topic %s(0x%08x) blindly inserted into preview", pTopic->GetName(), pTopic);
				}
			}
		}
	}

	// (a.walling 2007-12-17 17:30) - PLID 28391
	for (int s = 0; s < arTopicsToRefreshHTMLVisibility.GetSize(); s++) {
		arTopicsToRefreshHTMLVisibility[s]->RefreshHTMLVisibility();
	}
}

LRESULT CEmrTreeWnd::OnEmnChargeAdded(WPARAM wParam, LPARAM lParam)
{
	try {
		//DRT 1/12/2007 - PLID 24178 - Prompt here, not in the more info, it might not yet have been created.  All
		//	additions of charges post here to the interface, which handles distributing further, so this is the
		//	optimal spot for it.  Added a try/catch here too.
		PromptToLinkDiagCodesToCharge((EMNCharge*)wParam, (CEMN*)lParam, TRUE);

		//Pass to the more info.
		// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
		// (r.farnworth 2014-02-19 09:01) - PLID 60746 - MoreInfoDlg changed to CodesDlg
		CEmrCodesDlg *pDlg = GetCodesDlg((CEMN*)lParam, FALSE);
		if(pDlg) {
			return pDlg->SendMessage(NXM_EMN_CHARGE_ADDED, wParam, lParam);
		}
		else {
			return 0;
		}
	} NxCatchAll("Error in OnEmnChargeAdded");

	return 0;
}

//DRT 1/12/2007 - PLID 24234 - An EMNCharge pointer has changed, send it to the 
//	more info so the interface can be updated.
LRESULT CEmrTreeWnd::OnEmnChargeChanged(WPARAM wParam, LPARAM lParam)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
		// (s.dhole 2014-02-21 17:13) - PLID 60742  Change pointer from more info to code dlg
		CEmrCodesDlg *pDlg = GetCodesDlg((CEMN*)lParam, FALSE);
		if(pDlg) {
			return pDlg->SendMessage(NXM_EMN_CHARGE_CHANGED, wParam, 0);
		}
		else {
			return 0;
		}
	} NxCatchAll("Error in OnEmnChargeChanged");

	return 0;
}

LRESULT CEmrTreeWnd::OnEmnDiagAdded(WPARAM wParam, LPARAM lParam)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		LRESULT lrRetVal = 0;

		//Pass to the more info.
		// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
		// (r.farnworth 2014-02-19 09:01) - PLID 60746 - MoreInfoDlg changed to CodesDlg
		CEmrCodesDlg *pDlg = GetCodesDlg((CEMN*)lParam, FALSE);
		if(pDlg) {
			lrRetVal = pDlg->SendMessage(NXM_EMN_DIAG_ADDED, wParam, lParam);
		}
		else {
			//DRT 1/15/2007 - PLID 24179 - In EMNMoreInfoDlg, we load the code number and
			//	code description for the EMNDiagCode object.  But since we are now prompting, 
			//	we need to lookup this data ahead of time, if the more info does not exist.
			EMNDiagCode *pDiag = (EMNDiagCode*)wParam;
			// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
			//TES 2/28/2014 - PLID 61046 - Added ICD-10 fields
			_RecordsetPtr prs = CreateParamRecordset("SELECT CodeNumber, CodeDesc, ICD10 FROM DiagCodes WHERE ID IN ({INT}, {INT})", pDiag->nDiagCodeID, pDiag->nDiagCodeID_ICD10);
			while(!prs->eof) {
				if(AdoFldBool(prs, "ICD10")) {
					pDiag->strCode_ICD10 = AdoFldString(prs, "CodeNumber", "");
					pDiag->strCodeDesc_ICD10 = AdoFldString(prs, "CodeDesc", "");
				}
				else {
					pDiag->strCode = AdoFldString(prs, "CodeNumber", "");
					pDiag->strCodeDesc = AdoFldString(prs, "CodeDesc", "");
				}
				prs->MoveNext();
			}
			prs->Close();
			
			lrRetVal = 0;
		}

		//DRT 1/15/2007 - PLID 24179 - Prompt here to link the charges to the diagnosis code that
		//	is just spawned.  Note that we do this last because there are "efficiencies" built into
		//	the diagnosis code spawning that wait until after the more info is loaded to add the
		//	code & description fields.
		PromptToLinkChargesToDiagCode((EMNDiagCode*)wParam, (CEMN*)lParam, TRUE);
	} NxCatchAll("Error in OnEmnDiagAdded");

	return 0;
}

//DRT 3/3/2006 - PLID 19565
//We need to update the strCode member of an EMNDiagCode structure..  wParam is a pointer
//	to the EMNDiagCode structure, and lParam is a pointer to the EMN which the diag
//	code is a part of.
LRESULT CEmrTreeWnd::OnUpdateCodeOfEMNDiagCode(WPARAM wParam, LPARAM lParam)
{
	try {
		//Pass to the more info.
		// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
		// (r.farnworth 2014-02-18 11:41) - PLID 60746 - Rewritten for Codes
		//TES 2/26/2014 - PLID 60806 - We can't look this up on the Codes topic any more, because it doesn't have a combo
		// with all the codes in it.
		EMNDiagCode* pDiag = (EMNDiagCode*)wParam;
		CString strCode, strCodeDesc;
		//failed to find the code, we'll have to use data.  It might be inactive or new
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		//TES 2/28/2014 - PLID 61046 - Added ICD-10 fields
		// (c.haag 2014-03-17) - PLID 60929 - Added QuickListID
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT CodeNumber, CodeDesc, ICD10 FROM DiagCodes "
			"WHERE DiagCodes.ID IN ({INT}, {INT});\r\n"
			"\r\n"
			"SELECT QuickListID FROM dbo.GetQuickListIDForCodes({INT},{INT},{INT},{INT});\r\n"
				, pDiag->nDiagCodeID, pDiag->nDiagCodeID_ICD10
				, GetCurrentUserID(), DiagQuickListUtils::GetAPIDiagDisplayTypeInt(), pDiag->nDiagCodeID, pDiag->nDiagCodeID_ICD10
				);
		while(!prs->eof) {
			if(AdoFldBool(prs, "ICD10")) {
				pDiag->strCode_ICD10 = AdoFldString(prs, "CodeNumber", "");
				pDiag->strCodeDesc_ICD10 = AdoFldString(prs, "CodeDesc", "");
			}
			else {
				pDiag->strCode = AdoFldString(prs, "CodeNumber", "");
				pDiag->strCodeDesc = AdoFldString(prs, "CodeDesc", "");
			}
			prs->MoveNext();
		}
		prs = prs->NextRecordset(NULL);
		pDiag->nQuickListID = AdoFldLong(prs, "QuickListID", -1);

	} NxCatchAll("Error in OnUpdateCodeOfEMNDiagCode");

	return 0;
}

LRESULT CEmrTreeWnd::OnEmnMedicationAdded(WPARAM wParam, LPARAM lParam)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		//Pass to the more info.
		// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
		CEMNMoreInfoDlg *pDlg = GetMoreInfoDlg((CEMN*)lParam, FALSE);
		if(pDlg) {
			return pDlg->SendMessage(NXM_EMN_MEDICATION_ADDED, wParam, lParam);
		}
		else {
			return 0;
		}
	} NxCatchAll("Error in OnEmnMedicationAdded");

	return 0;
}

//DRT 9/6/2007 - PLID 27310 - This used to be the "guts" of OnEmrProcedureAdded.  Just pulled it out verbatim so
//	the multi procedure function could share it.
void CEmrTreeWnd::HandleNewProcedure(CEMN *pEMN, EMNProcedure *pProc)
{
	if(pEMN) {
		// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
		CEMNMoreInfoDlg *pDlg = GetMoreInfoDlg(pEMN, FALSE);
		if(pDlg) {
			pDlg->SendMessage(NXM_EMN_PROCEDURE_ADDED, (WPARAM)pProc, (LPARAM)pEMN);
		}
		IRowSettingsPtr pEmnRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pEMN, NULL, FALSE);
		if(pEmnRow) {
			EnsureModifiedState(pEmnRow);
			EnsureModifiedState(pEmnRow->GetLastChildRow());
		}
	}
}

//DRT 3/3/2006 - PLID 19410 - wParam changed from the ProcedureID to a ptr to an EMNProcedure structure
LRESULT CEmrTreeWnd::OnEmrProcedureAdded(WPARAM wParam, LPARAM lParam)
{
	try {
		EMNProcedure* pProc = (EMNProcedure*)wParam;

		//Pass to the more info.
		CEMN *pEMN = (CEMN*)lParam;

		HandleNewProcedure(pEMN, pProc);

		//Pass to the parent
		if(GetParent())
			GetParent()->SendMessage(NXM_EMN_PROCEDURE_ADDED, (WPARAM)pProc, lParam);

		// (a.walling 2007-07-13 12:26) - PLID 26640 - Update the more info preview
		UpdatePreviewMoreInfo(pEMN);
	} NxCatchAll("Error in OnEmrProcedureAdded");
	
	return 0;
}

//DRT 9/6/2007 - PLID 27310 - Allow multiple procedures to be added at once.  This is especially useful during
//	initial load when there may be many procedures being added at the same time.
//WPARAM is a pointer to a CArray<EMNProcedure*,EMNProcedure*>
//LPARAM is a pointer to a CEMN
LRESULT CEmrTreeWnd::OnEmrMultiProceduresAdded(WPARAM wParam, LPARAM lParam)
{
	try {
		//This function should do the same thing as the single OnEmrProcedureAdded, just en-masse.
		CEMN *pEMN = (CEMN*)lParam;
		CArray<EMNProcedure*,EMNProcedure*> *aryProcs = (CArray<EMNProcedure*,EMNProcedure*>*)wParam;

		for(int i = 0; i < aryProcs->GetSize(); i++) {
			EMNProcedure *pProc = aryProcs->GetAt(i);

			//Just use our shared code from the single proc function.  The more info does no database lookups, 
			//	so we're safe to just send them over 1 at a time there.
			HandleNewProcedure(pEMN, pProc);
		}

		//Inform our parent of this message
		if(GetParent())
			GetParent()->SendMessage(NXM_EMN_MULTI_PROCEDURES_ADDED, wParam, lParam);

		//Now that we're done, update the more info preview
		UpdatePreviewMoreInfo(pEMN);

	} NxCatchAll("Error in OnEmrMultiProceduresAdded");

	return 0;
}

//DRT 3/3/2006 - PLID 19410 - The more info dialog has a list of procedures, so in the interest
//	of avoiding data accesses, let's lookup from there.
//wParam is a pointer to the EMNProcedure structure, lParam is a pointer to the EMN to which we
//	are adding.
LRESULT CEmrTreeWnd::OnUpdateNameOfEMNProcedure(WPARAM wParam, LPARAM lParam)
{
	try {
		CEMN* pEMN = (CEMN*)lParam;
		//Pass to the more info.
		if(pEMN) {
			// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
			//TES 8/22/2012 - PLID 52238 - Don't force the dialog to be created.  If it's not created, just look up the name in data
			CEMNMoreInfoDlg *pDlg = GetMoreInfoDlg(pEMN, FALSE);
			EMNProcedure* pProc = (EMNProcedure*)wParam;
			if(pDlg) {
				CString strName;
				if(pDlg->LookupProcedureNameFromCombo(pProc->nID, strName)) {
					//Update from the found value
					pProc->strName = strName;
				}
				else {
					//Failed to find the name, the procedure may be inactive.  Look it up in data
					pProc->strName = VarString(GetTableField("ProcedureT", "Name", "ID", pProc->nID), "");
				}
			}
			else {
				pProc->strName = VarString(GetTableField("ProcedureT", "Name", "ID", pProc->nID), "");
			}
		}
	} NxCatchAll("Error in OnUpdateNameOfEMNProcedure");

	return 0;
}

LRESULT CEmrTreeWnd::OnEmnAdded(WPARAM wParam, LPARAM lParam)
{
	try {
		if(!m_bInitialLoadComplete) {
			//We haven't loaded our tree yet, so don't do anything, and when we load the tree, we'll load this EMN with it.
			return 0;
		}

		CEMN *pEMN = (CEMN*)wParam;
		CEMN* pNextEMN = NULL;

		NXDATALIST2Lib::IRowSettingsPtr pNextRow = NULL;

		BOOL bTryToPosition = (BOOL)lParam;

		if (bTryToPosition) {
			for (int i = 0; i < m_EMR.GetEMNCount(); i++) {
				pNextEMN = m_EMR.GetEMN(i);

				if (pEMN == pNextEMN) { // next one will do it
					i += 1;
					if (i < m_EMR.GetEMNCount()) {
						pNextEMN = m_EMR.GetEMN(i);
						break;
					}
				}
			}

			if (pNextEMN) {
				pNextRow = m_pTree->GetFirstRow();

				while (pNextRow) {
					CEMN* pRowEMN = (CEMN*)VarLong(pNextRow->GetValue(TREE_COLUMN_OBJ_PTR), 0);

					if (pRowEMN == pNextEMN) {
						break;
					}

					pNextRow = pNextRow->GetNextRow();
				}
			}
		}
		
		//Add to the tree.
		NXDATALIST2Lib::IRowSettingsPtr pEMNRow = m_pTree->GetNewRow();
		pEMNRow->PutValue(TREE_COLUMN_ID, pEMN->GetID());
		pEMNRow->PutValue(TREE_COLUMN_OBJ_PTR, (long)pEMN);
		pEMNRow->PutValue(TREE_COLUMN_ICON, (long)0);
		pEMNRow->PutValue(TREE_COLUMN_NAME, _bstr_t(GetEmnRowDisplayText(pEMN)));
		pEMNRow->PutValue(TREE_COLUMN_ROW_TYPE, (long)etrtEmn);
		pEMNRow->PutValue(TREE_COLUMN_LOADED, (long)0);

		if (pNextRow) {
			m_pTree->AddRowBefore(pEMNRow, pNextRow);
		} else {
			m_pTree->AddRowAtEnd(pEMNRow, NULL);
		}
		EnsureModifiedState(pEMNRow);

		//TES 2/12/2014 - PLID 60748 - Add the Codes child node that we know is always there.
		NXDATALIST2Lib::IRowSettingsPtr pChildRow = m_pTree->GetNewRow();
		pChildRow->PutValue(TREE_COLUMN_ID, (long)CODES_NODE_ID);
		pChildRow->PutValue(TREE_COLUMN_OBJ_PTR, (long)0);
		pChildRow->PutValue(TREE_COLUMN_ICON, (long)0);
		pChildRow->PutValue(TREE_COLUMN_NAME, _bstr_t("<Codes>"));
		pChildRow->PutValue(TREE_COLUMN_ROW_TYPE, (long)etrtCodes);
		pChildRow->PutValue(TREE_COLUMN_LOADED, (long)0);
		pChildRow->PutCellLinkStyle(TREE_COLUMN_NAME, dlLinkStyleTrue);
		m_pTree->AddRowAtEnd(pChildRow, pEMNRow);
		EnsureModifiedState(pChildRow);

		//Add the More Info child node that we know is always there.
		pChildRow = m_pTree->GetNewRow();
		pChildRow->PutValue(TREE_COLUMN_ID, (long)MORE_INFO_NODE_ID);
		pChildRow->PutValue(TREE_COLUMN_OBJ_PTR, (long)0);
		pChildRow->PutValue(TREE_COLUMN_ICON, (long)0);
		pChildRow->PutValue(TREE_COLUMN_NAME, _bstr_t("<More Info>"));
		pChildRow->PutValue(TREE_COLUMN_ROW_TYPE, (long)etrtMoreInfo);
		pChildRow->PutValue(TREE_COLUMN_LOADED, (long)0);
		// (a.walling 2012-06-28 17:11) - PLID 51276 - More Info should be a clickable link
		//pChildRow->PutCellLinkStyle(TREE_COLUMN_ICON, dlLinkStyleTrue); // icon cells don't handle this well
		pChildRow->PutCellLinkStyle(TREE_COLUMN_NAME, dlLinkStyleTrue);
		m_pTree->AddRowAtEnd(pChildRow, pEMNRow);
		EnsureModifiedState(pChildRow);

		
		
		//Pass it on to our parent.
		GetParent()->SendMessage(NXM_EMN_ADDED, (WPARAM)pEMN);
	} NxCatchAll("Error in OnEmnAdded");

	return 0;
}

LRESULT CEmrTreeWnd::OnPreDeleteEmnCharge(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
		// (s.dhole 2014-02-18 13:30) - PLID 60742  Change pointer from more info to code dlg
		CEmrCodesDlg *pDlg = GetCodesDlg((CEMN*)lParam, FALSE);
		//DRT 1/11/2007 - PLID 24220 - RemoveCharge now takes a EMNCharge* pointer
		if(pDlg) pDlg->RemoveCharge(((EMNCharge*)wParam));
	} NxCatchAll("Error in OnPreDeleteEmnCharge");
	return 0;
}

LRESULT CEmrTreeWnd::OnPreDeleteEmnDiag(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
		// (r.farnworth 2014-02-18 11:33) - PLID 60746 - Rewritten for CodesDlg
		CEmrCodesDlg *pDlg = GetCodesDlg((CEMN*)lParam, FALSE);
		if(pDlg) {
			//TES 2/26/2014 - PLID 60807 - Added ICD10
			pDlg->RemoveDiagCode(((EMNDiagCode*)wParam)->nDiagCodeID, ((EMNDiagCode*)wParam)->nDiagCodeID_ICD10);
		}

	} NxCatchAll("Error in OnPreDeleteEmnDiag");
	return 0;
}

LRESULT CEmrTreeWnd::OnPreDeleteEmnMedication(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2007-09-28 17:51) - PLID 27568 - we should be deleting by the actual prescription, not by the ID of the drug!!
		// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
		CEMNMoreInfoDlg *pDlg = GetMoreInfoDlg((CEMN*)lParam, FALSE);
		if(pDlg) {
			pDlg->RemoveMedication((EMNMedication*)wParam);
		}
	} NxCatchAll("Error in OnPreDeleteEmnMedication");
	return 0;
}

LRESULT CEmrTreeWnd::OnPreDeleteEmnProcedure(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
		CEMNMoreInfoDlg* pdlgMoreInfo = GetMoreInfoDlg((CEMN*)lParam, FALSE);
		if(pdlgMoreInfo) {
			pdlgMoreInfo->RemoveProcedure((long)wParam);
		}
		IRowSettingsPtr pEmnRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)lParam, NULL, FALSE);
		if(pEmnRow) {
			EnsureModifiedState(pEmnRow);
			EnsureModifiedState(pEmnRow->GetLastChildRow());
		}
		CWnd *pWnd = GetParent();
		if(pWnd) {
			pWnd->SendMessage(NXM_PRE_DELETE_EMN_PROCEDURE, wParam, lParam);
			pWnd->SendMessage(NXM_EMN_CHANGED, (WPARAM)lParam);		
		}
	} NxCatchAll("Error in OnPreDeleteEmnProcedure");

	return 0;
}

// (z.manning, 10/05/2007) - PLID 27630 - We now also have a message for after the procedure has been removed.
LRESULT CEmrTreeWnd::OnPostDeleteEmnProcedure(WPARAM wParam, LPARAM lParam)
{
	try
	{
		CWnd *pWnd = GetParent();
		if(pWnd) {
			pWnd->SendMessage(NXM_POST_DELETE_EMN_PROCEDURE, wParam, lParam);
		}

		// (a.walling 2007-07-13 12:26) - PLID 26640 - Update the more info preview
		UpdatePreviewMoreInfo((CEMN*)lParam);
	}NxCatchAll("CEmrTreeWnd::OnPostDeleteEmnProcedure");

	return 0;
}

LRESULT CEmrTreeWnd::OnHideEmrTopic(WPARAM wParam, LPARAM lParam)
{
	try {
		CEMRTopic *pTopic = (CEMRTopic*)wParam;
		IRowSettingsPtr pTopicRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pTopic, NULL, FALSE);
		if(pTopicRow) {
			// (a.walling 2007-09-21 09:47) - PLID 25549 - This message is only sent when a template, so we don't
			// need to update the preview or anything.
			// (c.haag 2007-09-27 11:11) - PLID 27509 - Use SetTreeRowVisible. If pChildRow is
			// somehow the current tree selection, it will ensure a new selection is made.
			SetTreeRowVisible(pTopicRow, FALSE);
			// (a.wetta 2006-07-07 10:05) - PLID 21362 - Be sure to reset the name in case it was changed because
			// this is a duplicate spawned topic, if this topic is made visible again then the duplicate check
			// will be run on it again and the topic will be renamed again if necessary
			pTopicRow->PutValue(TREE_COLUMN_NAME, _bstr_t(pTopic->GetName()));
		}
		else {
			//If it is the only child of its parent, then the parent shouldn't have a plus sign any more.
			CEMRTopic *pParent = pTopic->GetParentTopic();
			if(pParent) {
				bool bVisibleFound = false;
				for(int i = 0; i < pParent->GetSubTopicCount() && !bVisibleFound; i++) {
					if(pParent->GetSubTopic(i)->GetVisible() && pParent->GetSubTopic(i) != pTopic) bVisibleFound = true;
				}
				if(!bVisibleFound) {
					//OK, the parent row should have its plus sign removed.  Have we loaded the parent row?
					IRowSettingsPtr pParentRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pParent, NULL, FALSE);
					if(pParentRow) {
						//Yup!  Remove the placeholder.
						IRowSettingsPtr pPlaceholder = pParentRow->GetFirstChildRow();
						//TES 1/24/2007 - PLID 24410 - Check that the placeholder exists.  If not, then we must have
						// been unspawned twice (this can happen on templates, if the list item that was unchecked has
						// some older versions with the same action associated), and in that case the placeholder was
						// already removed so we don't have to worry about it.
						if(pPlaceholder != NULL) {
							ASSERT(etrtPlaceholder == EmrTree::ChildRow(pPlaceholder).GetType());
							RemoveTreeRow(pPlaceholder);
						}
						//This row now has all of its subtopics loaded (because it doesn't have any).
						pParentRow->PutValue(TREE_COLUMN_LOADED, (long)1);
					}
				}
			}
		}
	}NxCatchAll("Error in OnHideEmrTopic");
	
	return 0;
}

LRESULT CEmrTreeWnd::OnPreDeleteEmrTopic(WPARAM wParam, LPARAM lParam)
{
	try {
		CEMRTopic *pTopic = (CEMRTopic*)wParam;

		// (a.walling 2007-04-10 12:25) - PLID 25549
		if (GetEmrPreviewCtrl()) {
			GetEmrPreviewCtrl()->RemoveTopic(pTopic);
		}

		IRowSettingsPtr pTopicRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pTopic, NULL, FALSE);
		if(pTopicRow) {

			EnsureModifiedState(pTopicRow);

			RemoveTreeRow(pTopicRow);
		}
		else {
			//if not found, find its nearest parent
			// (a.walling 2007-09-19 17:44) - PLID 25549 - Use a temporary pointer here
			CEMRTopic* pParentTopic = pTopic;
			while(pParentTopic) {
				pParentTopic = pParentTopic->GetParentTopic();

				pTopicRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pParentTopic, NULL, FALSE);
				if(pTopicRow) {
					EnsureModifiedState(pTopicRow);
					// (a.walling 2007-09-19 15:52) - PLID 25549 - We still want to verify the visibility of the topics
					// via the code block below
					//return 0;
					break;
				}
			}
		}

		//TES 6/30/2006 - Should this topic's parent (and any of its parents) now be hidden?
		if(!m_bIsTemplate) {
			CEMRTopic *pParent = pTopic->GetParentTopic();
			while(pParent) {
				if(!pParent->ShowIfEmpty() && !pParent->HasDetails()) {
					//Well, it doesn't have any details, and if it's empty, it should be hidden. Does it have any visible subtopics,
					//apart from the one being deleted?
					BOOL bVisibleFound = FALSE;
					for(int nSubTopic = 0; nSubTopic < pParent->GetSubTopicCount() && !bVisibleFound; nSubTopic++) {
						CEMRTopic *pSubTopic = pParent->GetSubTopic(nSubTopic);
						// (a.walling 2012-07-09 12:35) - PLID 51441 - IsEmpty now has some non-optional parameters
						if(pSubTopic != pTopic && !pSubTopic->IsEmpty(NULL, FALSE)) bVisibleFound = TRUE;
					}
					if(!bVisibleFound) {
						//Nope, let's hide this row, and move on to its parents.
						pTopicRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pParent, NULL, FALSE);
						if(pTopicRow) {
							// (c.haag 2007-09-27 11:11) - PLID 27509 - Use SetTreeRowVisible. If pChildRow is
							// somehow the current tree selection, it will ensure a new selection is made.
							SetTreeRowVisible(pTopicRow, FALSE);
							// tell our preview window to hide this.
							if (GetEmrPreviewCtrl()) {
								GetEmrPreviewCtrl()->ShowTopic(pParent, FALSE);
							}
						}
						pParent = pParent->GetParentTopic();
					}
					else {
						pParent = NULL;
					}
				}
				else {
					pParent = NULL;
				}
			}
		}


	} NxCatchAll("Error in OnPreDeleteEmrTopic");

	return 0;
}

LRESULT CEmrTreeWnd::OnPreDeleteEmn(WPARAM wParam, LPARAM lParam)
{
	try {
		CEMN* pEMN = (CEMN*)wParam;
		// (a.walling 2007-09-26 10:02) - PLID 27503 - All confirmation has been moved to the NXM_QUERY_UNSPAWN_EMNS handler
		// (a.walling 2007-09-25 09:47) - PLID 27503 - Confirm that the user would really like to unspawn this EMN

		IRowSettingsPtr pEmnRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pEMN, NULL, FALSE);
		if(pEmnRow) {
			//Free up the windows.
			RemoveTreeRow(pEmnRow);
		}

		// (a.walling 2012-03-14 09:47) - PLID 46638 - Destroy more info when EMN is deleted
		if (CEmrMoreInfoView* pMoreInfoView = GetEmrFrameWnd()->GetEmrMoreInfoView(pEMN, false)) {
			pMoreInfoView->GetParentFrame()->SendMessage(WM_CLOSE);
		}
		//TES 2/12/2014 - PLID 60740 - Added for the new <Codes> topic
		if (CEmrCodesView* pCodesView = GetEmrFrameWnd()->GetEmrCodesView(pEMN, false)) {
			pCodesView->GetParentFrame()->SendMessage(WM_CLOSE);
		}
		
		// (a.walling 2012-03-30 10:49) - PLID 49329 - Destroy the topic view when EMN is deleted
		if (CEmrTopicView* pTopicView = GetEmrFrameWnd()->GetEmrTopicView(pEMN, false)) {
			pTopicView->GetParentFrame()->SendMessage(WM_CLOSE);
		}

		// (a.walling 2007-09-25 14:21) - PLID 25548 - Remove from the preview
		if (GetEmrPreviewCtrl()) {
			GetEmrPreviewCtrl()->RemoveEMN(pEMN);
		}

		return (LRESULT)0;
	} NxCatchAll("Error in OnPreDeleteEmn");

	// there was an error, return -1.
	return (LRESULT)-1;
}

LRESULT CEmrTreeWnd::OnPostDeleteEmn(WPARAM wParam, LPARAM lParam)
{
	try {
		//TES 6/5/2008 - PLID 30196 - If the deleted EMN had a system table on it, we need to warn them to check the Medications
		// tab (though only if the EMN was ever saved).
		CEMN* pEMN = (CEMN*)wParam;
		ASSERT(pEMN);
		if(pEMN) {
			BOOL bHasCurrentMeds = FALSE;
			BOOL bHasAllergies = FALSE;
			if(pEMN->GetID() != -1) {
				bHasCurrentMeds = pEMN->HasSystemTable(eistCurrentMedicationsTable);
				bHasAllergies = pEMN->HasSystemTable(eistAllergiesTable);
			}

			if(bHasCurrentMeds || bHasAllergies) {
				//TES 6/5/2008 - PLID 30196 - Warn them to check the medications tab.
				CString strTables;
				if(bHasCurrentMeds && bHasAllergies) {
					strTables = "'Current Medications' and 'Allergies' tables";
				}
				else if(bHasCurrentMeds) {
					strTables = "'Current Medications' table";
				}
				else if(bHasAllergies) {
					strTables = "'Allergies' table";
				}

				MessageBox(FormatString("The deleted EMN contained the %s.  Please review this patient's Medications tab to make sure that "
					"its corresponding information is up to date.", strTables));
			}
		}

	}NxCatchAll("Error in OnPostDeleteEmn()");

	return (LRESULT)0;
}

LRESULT CEmrTreeWnd::OnEmrItemEditContent(WPARAM wParam, LPARAM lParam)
{
	try {
		CEMNDetail *pEMNDetail = (CEMNDetail*)wParam;
		CEmrItemEditContentInfo *peieci = (CEmrItemEditContentInfo*)lParam;
		switch (peieci->nChangeType) {
		case (ITEM_ADD): {
			// Everything we need is in the cat tab info
			if (pEMNDetail) {
				// See what kind of emr info it is
				if (pEMNDetail->m_EMRInfoType == eitImage) {
					// (b.cardillo 2004-07-23 12:23) - PLID 13594 - For images the adv dlg sends a content 
					// change request but we're actually handling it like a "state" change, NOT a content 
					// change.  Notice how we don't write anything to data, because we're just changing the 
					// current state of this detail.  Thus, we need to prevent it if the user is editing a 
					// template.
					if (!m_bIsTemplate) {
						// Image

						// (j.jones 2006-07-12 16:36) - PLID 21421 - if there is already ink on the image,
						// warn the user it will be cleared out, then do so if they agree to proceed

						// To detect the ink status, first create an image state object based on the current detail state
						CEmrItemAdvImageState ais;
						ais.CreateFromSafeArrayVariant(peieci->pEMNDetail->GetState());

						if((ais.m_varInkData.vt != VT_EMPTY && ais.m_varInkData.vt != VT_NULL) ||
							(ais.m_varTextData.vt != VT_EMPTY && ais.m_varTextData.vt != VT_NULL) ) {
							//there is some ink on this image, so warn the user
							if(IDNO == MessageBox("Selecting a new image will erase the ink you have currently drawn.\n"
								"Are you sure you wish to continue?\n\n"
								"(Ink will not be erased until an image is chosen.)","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
								return IDCANCEL;
							}
						}

						CSelectImageDlg dlg(this);
						dlg.m_nPatientID = GetPatientID();
						// (j.jones 2013-09-19 15:19) - PLID 58547 - added a pointer to the pic
						dlg.m_pPicContainer = GetPicContainer();
						if (dlg.DoModal() == IDOK) {
							//use the image state from above
							ais.m_eitImageTypeOverride = dlg.m_nImageType;
							ais.m_strImagePathOverride = dlg.m_strFileName;
							//clear out the ink
							VariantClear(&ais.m_varInkData);
							peieci->pEMNDetail->SetInkErased();
							//TES 1/22/2007 - PLID 18159 - And the text.
							VariantClear(&ais.m_varTextData);
							peieci->pEMNDetail->SetImageTextRemoved();
							//assign the new state
							// (z.manning 2008-11-10 10:23) - PLID 31957 - Call RequestStateChange here instead
							// of SetState so that we handle other areas affected by this detail's new state
							// including topic completion status.
							//peieci->pEMNDetail->SetState(ais.AsSafeArrayVariant());
							peieci->pEMNDetail->RequestStateChange(ais.AsSafeArrayVariant());

							// (j.jones 2006-06-30 13:52) - PLID 20939 - Why in the world would
							// we refresh ALL items when we are changing the picture for one image?
							// why is even in an EditContent function? I don't know the answer to the latter,
							// but we can at least fix the former!

							// Refresh the static listing
							//RefreshAllItems();

							// Refresh the currently visible tab
							// (j.jones 2007-07-26 09:31) - PLID 24686 - renamed this function
							RefreshContentByInfoID(pEMNDetail->m_nEMRInfoID, FALSE);

							// (a.walling 2007-04-09 17:59) - PLID 25549 - Update with the new image
							if (GetEmrPreviewCtrl()) {
								GetEmrPreviewCtrl()->UpdateDetail(pEMNDetail);
							}
							return IDOK;
						} else {
							return IDCANCEL;
						}
					} else {
						return IDCANCEL;
					}
				} else {
					// The detail index refers to a detail whose tab isn't visible right now
					//DRT 9/11/2006 - PLID 22374 - If you get here, it *may* be because of code that I removed.
					ASSERT(FALSE);
					return -2;
				}
			} else {
				// (b.cardillo 2004-03-29 11:42) - I don't know of any time this function should be 
				// called with an invalid nDetailIndex.
				ASSERT(FALSE);
				return -5;
			}
		}

		//TES 2/22/2006 - None of the other options for this parameter are ever used any more.

		default:
			//ACW 5/21/04 It should never get to this point unless an invalid nChangeType value was passed
			ASSERT(FALSE);
			return -9;
		}
	} NxCatchAllCall("CEmrTreeWnd::OnEmrItemEditContent", {
		return -1;
	});
}

LRESULT CEmrTreeWnd::OnEmnStatusChanging(WPARAM wParam, LPARAM lParam)
{
	try {
		IRowSettingsPtr pEmnRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)lParam, NULL, FALSE);
		if(pEmnRow) {
			// (j.jones 2011-07-05 11:56) - PLID 43603 - this is now a class
			EMNStatus *pNewStatus = (EMNStatus*)wParam;
			EMNStatus emnNewStatus = *pNewStatus;
			ChangeStatus(emnNewStatus, pEmnRow);
			*pNewStatus = emnNewStatus;
		}
		else {
			ASSERT(FALSE);
		}
	} NxCatchAll("Error in OnEmnStatusChanging");
	return 0;
}

LRESULT CEmrTreeWnd::OnEmnMoreInfoChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2012-03-22 17:37) - PLID 49141 - This is simply a notification that it has changed now, nothing should depend on us getting this message really, 
		// nor should we do anything other than update the interface

		std::set<CEMN*> changedEMNs;
		{
			std::set<CEMN*> notified, existing;
			notified.insert((CEMN*)wParam);

			// (a.walling 2012-03-22 17:37) - PLID 49141 - Clear out any extra pending messages for all EMNs
			MSG msg = {};
			while (::PeekMessage(&msg, GetSafeHwnd(), NXM_EMN_MORE_INFO_CHANGED, NXM_EMN_MORE_INFO_CHANGED, PM_REMOVE | PM_NOYIELD | QS_POSTMESSAGE)) {
				if (NXM_EMN_MORE_INFO_CHANGED != msg.message) {
					ASSERT(FALSE);
					break;
				}
				
				notified.insert((CEMN*)wParam);
			}

			// (a.walling 2012-03-22 17:37) - PLID 49141 - Intersect with the actual EMNs on the EMR, just in case
			boost::set_intersection(
				notified, 
				boost::insert(existing, GetEMR()->GetAllEMNs()), 
				end_inserter(changedEMNs)
			);
		}

		foreach (CEMN* pEMN, changedEMNs) {
			if (pEMN->GetStatus() != 2) {
				IRowSettingsPtr pEmnRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pEMN, NULL, FALSE);
				if(pEmnRow) {
					pEmnRow->PutValue(TREE_COLUMN_NAME, _bstr_t(GetEmnRowDisplayText(pEMN)));
					EnsureModifiedState(pEmnRow);
					EnsureModifiedState(pEmnRow->GetLastChildRow());
				}
			}
			
			// (a.walling 2012-07-06 17:37) - PLID 49154 - More info synchronization
			if (CEMNMoreInfoDlg* pMoreInfo = GetMoreInfoDlg(pEMN, FALSE)) {
				pMoreInfo->OnMoreInfoChanged();
			}

			// (a.walling 2007-07-12 15:09) - PLID 26640 - Update the more info in the preview
			UpdatePreviewMoreInfo(pEMN);
		}

	} NxCatchAll("Error in OnEmnMoreInfoChanged");
	return 0;
}

//TES 2/20/2014 - PLID 60748 - Handler for NXM_EMN_CODES_CHANGED
LRESULT CEmrTreeWnd::OnEmnCodesChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2012-03-22 17:37) - PLID 49141 - This is simply a notification that it has changed now, nothing should depend on us getting this message really, 
		// nor should we do anything other than update the interface

		std::set<CEMN*> changedEMNs;
		{
			std::set<CEMN*> notified, existing;
			notified.insert((CEMN*)wParam);

			// (a.walling 2012-03-22 17:37) - PLID 49141 - Clear out any extra pending messages for all EMNs
			MSG msg = {};
			while (::PeekMessage(&msg, GetSafeHwnd(), NXM_EMN_CODES_CHANGED, NXM_EMN_CODES_CHANGED, PM_REMOVE | PM_NOYIELD | QS_POSTMESSAGE)) {
				if (NXM_EMN_CODES_CHANGED != msg.message) {
					ASSERT(FALSE);
					break;
				}
				
				notified.insert((CEMN*)wParam);
			}

			// (a.walling 2012-03-22 17:37) - PLID 49141 - Intersect with the actual EMNs on the EMR, just in case
			boost::set_intersection(
				notified, 
				boost::insert(existing, GetEMR()->GetAllEMNs()), 
				end_inserter(changedEMNs)
			);
		}

		foreach (CEMN* pEMN, changedEMNs) {
			if (pEMN->GetStatus() != 2) {
				IRowSettingsPtr pEmnRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pEMN, NULL, FALSE);
				if(pEmnRow) {
					pEmnRow->PutValue(TREE_COLUMN_NAME, _bstr_t(GetEmnRowDisplayText(pEMN)));
					EnsureModifiedState(pEmnRow);
					IRowSettingsPtr pMoreInfo = pEmnRow->GetLastChildRow();
					if(pMoreInfo) {
						EnsureModifiedState(pMoreInfo->GetPreviousRow());
					}
				}
			}
			
			// (a.walling 2012-07-06 17:37) - PLID 49154 - More info synchronization
			if (CEmrCodesDlg* pCodes = GetCodesDlg(pEMN, FALSE)) {
				pCodes->OnCodesChanged();
			}

			//TES 2/20/2014 - PLID 60748 - The Preview still treats both topics' information in the same section
			UpdatePreviewMoreInfo(pEMN);
		}

	} NxCatchAll("Error in OnEmnCodesChanged");
	return 0;
}

// (j.jones 2011-07-05 11:40) - PLID 43603 - this now takes in a class
void CEmrTreeWnd::ChangeStatus(IN OUT EMNStatus &emnNewStatus, LPDISPATCH lpEmnRow)
{
	try {
		IRowSettingsPtr pEmnRow(lpEmnRow);
		ASSERT(etrtEmn == EmrTree::ChildRow(pEmnRow).GetType());
		CEMN *pEMN = EmrTree::ChildRow(pEmnRow).GetEMN();

		ASSERT(pEMN->IsWritable());

		// (a.walling 2008-06-09 13:01) - PLID 22049 - Sanity check; this must be a writable EMN
		if (!pEMN->IsWritable())
			return;

		// (c.haag 2007-08-27 09:37) - PLID 27185 - Ensure that the EMN's initial load is done.
		// The prior code never checked to see whether EMN is null.
		if (NULL == pEMN) {
			ASSERT(FALSE);
			return;
		}
		pEMN->EnsureCompletelyLoaded();

		if(emnNewStatus.nID == 1 && GetRemotePropertyInt("EmnLockOnComplete", 0, 0, GetCurrentUserName(), true)) {
			emnNewStatus.nID = 2;
			emnNewStatus.strName = "Locked";
		}

		//If they're going from not-locked to locked, prompt them.
		BOOL bIsLocking = FALSE;
		if(pEMN->GetStatus() != 2 && emnNewStatus.nID == 2) {
			bIsLocking = TRUE;
		}

		if(bIsLocking) {
			//TES 12/26/2006 - PLID 23401 - They need to have a licensed provider before locking.
			//TES 12/26/2006 - PLID 23400 - Moreover, all of their assigned providers must be licensed.
			CArray<long,long> arProviderIDs;
			pEMN->GetProviders(arProviderIDs);
			if(!AreEmnProvidersLicensed(pEMN->GetID(), "", &arProviderIDs)) {
				//The user will have been notified, we just need to reset the status on the more info screen.
				// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
				CEMNMoreInfoDlg *pDlg = GetMoreInfoDlg(pEMN, FALSE);
				if(pDlg) {
					pDlg->SetStatus(pEMN->GetStatus());
				}
				return;
			}
			//At this point, we must have a licensed provider, so continue with the save.

			// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
			// Make sure there are no unfilled required details
			if (pEMN->HasVisibleUnfilledRequiredDetails()) {
				// Can't lock an EMN that has unfilled required details
				// Notify the user
				MessageBox(
					_T("This EMN may not be locked because it contains at least one required detail that has not ")
					_T("been filled in.  Please complete all required details and try again.")
					, NULL, MB_OK|MB_ICONEXCLAMATION);
				// Reset the status on the more info screen.
				CEMNMoreInfoDlg *pDlg = GetMoreInfoDlg(pEMN, FALSE);
				if(pDlg) {
					pDlg->SetStatus(pEMN->GetStatus());
				}
				// Abort
				return;
			}

			// (j.jones 2006-08-17 12:02) - PLID 22070 - first force a save
			//TES 2/12/2014 - PLID 60740 - No need to check IsMoreInfoUnsaved() if we already checked IsUnsaved()
			if(pEMN->IsUnsaved() || pEMN->GetID() == -1)
			{
				if(IDYES != MessageBox("Before this EMN can be locked, the changes you have made to the EMN must be saved.\n"
						"Would you like to save now?", NULL, MB_YESNO))
					{					
					//no? Well, we can't set the status then.
						// (a.walling 2012-04-06 12:13) - PLID 49496 - EMN Status - GetStatus now returns the object
					emnNewStatus = pEMN->GetStatus();

					//reset the status on the more info screen
					// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
					CEMNMoreInfoDlg *pDlg = GetMoreInfoDlg(pEMN, FALSE);
					if(pDlg) {
						pDlg->SetStatus(emnNewStatus.nID);
					}
					return;
				}

				// (j.jones 2009-06-17 12:56) - PLID 34652 - make sure we check to see if the save failed
				if(FAILED(SaveEMR(esotEMN, (long)pEMN, TRUE))) {
					// Reset the status on the more info screen.
					CEMNMoreInfoDlg *pDlg = GetMoreInfoDlg(pEMN, FALSE);
					if (pDlg) {
						pDlg->SetStatus(pEMN->GetStatus());
					}
					//return silently
					return;
				}
			}

			// (c.haag 2008-07-07 16:02) - PLID 30632 - Warn if there are any outstanding todo alarms for this EMN
			// (c.haag 2008-07-11 11:19) - PLID 30550 - Include non-detail-specific EMN todos
			if (pEMN->GetID() > 0) {
				// (z.manning 2016-01-13 16:00) - PLID 67778 - Moved this logic to shared function
				if (!PromptIfAnyOutstandingTodos(GetRemoteData(), pEMN->GetID(), GetSafeHwnd())) {
					// Reset the status on the more info screen.
					CEMNMoreInfoDlg *pDlg = GetMoreInfoDlg(pEMN, FALSE);
					if(pDlg) {
						pDlg->SetStatus(pEMN->GetStatus());
					}
					return;
				}
			}

			// (d.singleton 2013-07-24 16:35) - PLID 44840 - Having a pop up to warn doctor a diagnosis code and/or CPT needs to be selected before locking an EMN
			if(pEMN->GetID() > 0) {
				if(GetRemotePropertyInt("RequireCPTCodeEMNLocking", 0, 0, "<None>", true) && !ReturnsRecordsParam("SELECT * FROM EMRChargesT WHERE EMRID = {INT} AND Deleted = 0", pEMN->GetID())) {
					AfxMessageBox("There are no CPT codes selected on this EMN, and therefore it will not be locked.");
					//reset status back
					CEMNMoreInfoDlg *pDlg = GetMoreInfoDlg(pEMN, FALSE);
					if(pDlg) {
						pDlg->SetStatus(pEMN->GetStatus());
					}
					return;		
				}
				if(GetRemotePropertyInt("RequireDiagCodeEMNLocking", 0, 0, "<None>", true) && !ReturnsRecordsParam("SELECT * FROM EMRDiagCodesT WHERE EMRID = {INT} AND Deleted = 0", pEMN->GetID())) {
					AfxMessageBox("There are no Diagnosis codes selected on this EMN, and therefore it will not be locked.");
					//reset status back
					CEMNMoreInfoDlg *pDlg = GetMoreInfoDlg(pEMN, FALSE);
					if(pDlg) {
						pDlg->SetStatus(pEMN->GetStatus());
					}
					return;
				}
			}

			// (b.savon 2015-12-29 10:18) - PLID 58470 - Do not allow an EMN to be locked if it has an incomplete Rx associated with it.
			if (pEMN->GetID() > 0) {
				if (HasUnsettledPrescriptions(pEMN->GetID())) {
					// Reset the status on the more info screen.
					CEMNMoreInfoDlg *pDlg = GetMoreInfoDlg(pEMN, FALSE);
					if (pDlg) {
						pDlg->SetStatus(pEMN->GetStatus());
					}
					return;
				}
				else {
					// No unsettled prescriptions
				}
			}

			// (j.jones 2012-09-28 13:34) - PLID 52820 - Our preferences may request showing
			// drug interactions only upon closing/locking, if anything affected interactions
			// during this session. If so, we should show the drug interactions now, so that
			// they pop up first and the prompt to continue locking shows up on top of that dialog.
			TryShowSessionDrugInteractions(TRUE, FALSE);

			//and now prompt for locking
			if(IDYES != MessageBox("You have chosen to lock this EMN. Once locked, it will not be possible for any user to make any changes to the EMN, under any circumstances.\n"
				"Locking the EMN will take effect IMMEDIATELY and cannot be undone after this point.\n"
				"Are you SURE you wish to do this?", NULL, MB_YESNO)) {
					// (a.walling 2012-04-06 12:13) - PLID 49496 - EMN Status - GetStatus now returns the object
					emnNewStatus = pEMN->GetStatus();

					//reset the status on the more info screen
					// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
					CEMNMoreInfoDlg *pDlg = GetMoreInfoDlg(pEMN, FALSE);
					if(pDlg) {
						pDlg->SetStatus(emnNewStatus.nID);
					}
					return;
			}

#pragma TODO("SetEditMode")
			//if locked, disable buttons		
			//SetEditMode(FALSE);
		}
		pEMN->SetStatus(emnNewStatus);
		//Set the icon
		EnsureModifiedState(pEmnRow);

		//Now, if it's locked, tell all our windows.
		if(emnNewStatus.nID == 2) {

			// (j.jones 2007-06-15 09:30) - PLID 26297 - added locked color preference
			// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
			COLORREF cLocked = EmrColors::Topic::Locked();

			FOR_ALL_ROWS_WITH_PARENT(m_pTree, pEmnRow) {
				pRow->PutBackColor(cLocked);
				EmrTreeRowType etrt = (EmrTreeRowType)VarLong(pRow->GetValue(TREE_COLUMN_ROW_TYPE));
				if(etrt == etrtTopic) {
					CEmrTopicWndPtr pTopicWnd = EmrTree::ChildRow(pRow).GetTopicWnd();
					if (pTopicWnd) {
						pTopicWnd->SetReadOnly(TRUE);
					}
				}
				else if(etrt == etrtMoreInfo) {
					// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
					if (GetEmrFrameWnd()->GetMoreInfoDlg(pEMN)) {
						GetEmrFrameWnd()->GetMoreInfoDlg(pEMN)->SetReadOnly(TRUE);
					}
				}
				//TES 2/12/2014 - PLID 60748 - New Codes topic
				else if(etrt == etrtCodes) {
					if(GetEmrFrameWnd()->GetEmrCodesDlg(pEMN)) {
						GetEmrFrameWnd()->GetEmrCodesDlg(pEMN)->SetReadOnly(TRUE);
					}
				}
				GET_NEXT_ROW(m_pTree)
			}
		}

		//And in any case, tell the more info dlg.
		// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
		CEMNMoreInfoDlg *pDlg = GetMoreInfoDlg(pEMN, FALSE);
		if(pDlg) {
			pDlg->SetStatus(emnNewStatus.nID);
		}

		if(bIsLocking) {
			//and save again. Why? Because we saved before locking, then prompted to lock,
			//and now need to save the fact that it was locked!

			// (j.jones 2009-06-17 12:56) - PLID 34652 - make sure we check to see if the save failed
			if(FAILED(SaveEMR(esotEMN, (long)pEMN, TRUE))) {
				//return silently
				return;
			}
		}

	} NxCatchAll("Error in ChangeStatus");
}

LRESULT CEmrTreeWnd::OnEmnChanged(WPARAM wParam, LPARAM lParam)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {

		// (j.jones 2008-07-28 10:32) - PLID 30773 - ensured that this function called
		// EnsureModifiedState on the EMN in question

		CEMN *pEMN = (CEMN*)wParam;
		
		//don't mark as unsaved if the EMN is locked
		if (pEMN->GetStatus() != 2) {
			pEMN->SetUnsaved();
		}
		
		IRowSettingsPtr pEmnRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pEMN, NULL, FALSE);
		if(pEmnRow) {
			pEmnRow->PutValue(TREE_COLUMN_NAME, _bstr_t(GetEmnRowDisplayText(pEMN)));
			EnsureModifiedState(pEmnRow);
		}

		//and then pass on to our parent.
		return GetParent()->SendMessage(NXM_EMN_CHANGED, wParam, lParam);
	} NxCatchAll("Error in OnEmnChanged");

	return 0;
}

void CEmrTreeWnd::OnAddItemToTopic()
{
	try {
		AddItemsToTopic();
	} NxCatchAll("Error in OnAddItemToTopic");
}

void CEmrTreeWnd::AddItemsToTopic()
{
	// (j.jones 2007-03-14 15:06) - PLID 25195 - altered this functionality such that
	// if you choose to add a new item, it will reopen the list with that item
	// also selected, any any other items you may have selected, rather than automatically
	// adding the new item to the topic

	CDWordArray arynIDs;
	long nInfoIDToSelect = -1;

	BOOL bFinishedAdding = FALSE;

	// (c.haag 2008-06-17 12:41) - PLID 17842 - Track all added details
	CArray<CEMNDetail*,CEMNDetail*> apNewDetails;

	while(!bFinishedAdding) {

		CWaitCursor pWait;

		// (j.jones 2007-03-14 15:15) - PLID 25195 - we will not reopen the multiselect list
		// unless we create a new item
		bFinishedAdding = TRUE;
	
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "EmrInfoT");
		dlg.m_strOtherButtonText = "&New Item";
		dlg.m_eOtherButtonStyle = NXB_NEW; // (z.manning, 04/30/2008) - PLID 29845
		dlg.m_pfnContextMenuProc = AddItemToTabMultiSelectContextMenuProc;
		dlg.m_nContextMenuProcParam = (LPARAM)this;
		// (c.haag 2007-01-31 12:00) - PLID 24428 - Make the official Current Medications
		// colored in blue
		// (c.haag 2007-04-02 12:49) - PLID 25458 - Same with the official Allergies table
		dlg.m_strColorField.Format("CASE WHEN DataSubType IN (%d,%d) THEN 16711680 ELSE NULL END",
			eistCurrentMedicationsTable, eistAllergiesTable);
		// (z.manning 2009-08-19 17:40) - PLID 17932 - Show the filter button
		dlg.m_bShowFilterButton = TRUE;

		CStringArray straryExtraColumnFields, straryExtraColumnNames;
		straryExtraColumnFields.Add("CASE WHEN DataType = 1 THEN 'Text' WHEN DataType = 2 THEN 'Single-Select List' WHEN DataType = 3 THEN 'Multi-Select List' WHEN DataType = 4 THEN 'Image' WHEN DataType = 5 THEN 'Slider' WHEN DataType = 6 THEN 'Narrative' WHEN DataType = 7 THEN 'Table' END");
		straryExtraColumnNames.Add("Data Type");

		// (j.jones 2007-03-14 15:16) - PLID 25195 - if we are on a second pass,
		// we may have IDs to preselect. If so, select them.
		if(arynIDs.GetSize() > 0) {
			dlg.PreSelect(arynIDs);

			//jump down to the last item we created or selected,
			//then clear out our selection
			if(nInfoIDToSelect == -1) {
				nInfoIDToSelect = arynIDs.GetAt(arynIDs.GetSize()-1);
			}

			dlg.m_nIDToSelect = nInfoIDToSelect;
			nInfoIDToSelect = -1;
		}

		// (c.haag 2006-02-24 16:39) - PLID 12763 - We now filter out inactive items
		//TES 12/6/2006 - PLID 23724 - We want to return a list of EmrInfoMasterT.IDs, not EmrInfoT.IDs
		// (c.haag 2008-06-17 17:11) - PLID 30319 - Suppress the Emr Text Macro item
		// (j.jones 2010-06-04 16:55) - PLID 39029 - do not show generic tables (DataSubType = 3)
		int nResult = dlg.Open("EmrInfoT INNER JOIN EmrInfoMasterT ON EmrInfoT.ID = EmrInfoMasterT.ActiveEmrInfoID", FormatString("Inactive = 0 AND EmrInfoT.ID <> %li AND EMRInfoT.DataSubType <> %li", EMR_BUILT_IN_INFO__TEXT_MACRO, eistGenericTable), "EmrInfoMasterT.ID", "Name", "Please select the item you wish to add:", 0, 0xFFFFFFFF, &straryExtraColumnFields, &straryExtraColumnNames);
		if (nResult == IDOK) {

			CWaitCursor pWait;

			// (j.jones 2007-03-14 15:08) - PLID 25195 - clear the existing array
			// and re-fill with the dialog selections
			arynIDs.RemoveAll();
			dlg.FillArrayWithIDs(arynIDs);

			// (c.haag 2007-02-06 12:05) - PLID 24376 - Give a special warning to the user when
			// adding a Current Medications item. Don't access the data unless we know for sure
			// the message was not permanently dismissed
			const CString strDontShowName = "AddCurrentMedicationItem";
			if (arynIDs.GetSize() > 0 && !GetRemotePropertyInt(CString("dontshow ") + strDontShowName, 0, 0, GetCurrentUserName(), false)) {
				CString strWhere = GenerateDelimitedListFromLongArray(arynIDs, ",");
				// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
				if (ReturnsRecordsParam("SELECT ID FROM EmrInfoT WHERE DataSubType = {CONST} AND ID IN (SELECT ActiveEmrInfoID FROM EmrInfoMasterT WHERE ID IN ({INTSTRING}))",
					eistCurrentMedicationsTable, strWhere)) 
				{
					DontShowMeAgain(this, "You have chosen to add the Current Medications detail to this topic.\n\n"
						"The new EMR Medications integration allows "
						"you to automatically update the current medications list in the patient's Medications tab with "
						"any selections you make to this item. If the patient has any existing medications, they will "
						"automatically be selected in this item.\n\n"
						"If you previously maintained a multi-select or table item named 'Current Medications', it may "
						"have been inactivated as a result of this upgrade. Please review any templates which include "
						"a 'Current Medications' item to ensure they are set up correctly.",
						"AddCurrentMedicationItem");
				}
			}

			AddItems(arynIDs, &apNewDetails);

		} else if (nResult == CMultiSelectDlg::rvOtherBtn) {

			CWaitCursor pWait;

			// (j.jones 2007-03-14 15:08) - PLID 25195 - clear the existing array
			// and re-fill with the dialog selections
			arynIDs.RemoveAll();
			dlg.FillArrayWithIDs(arynIDs);

			// Create new item
			CEmrItemEntryDlg dlg(this);
			// (c.haag 2006-04-04 11:12) - PLID 19890 - Check permissions
			if(CheckCurrentUserPermissions(bioAdminEMR, sptRead)) {

				/*
				if(GetLastEMN()) {
					dlg.SetCurrentEMN(GetLastEMN());
				}
				dlg.m_bWillAddToTopic = TRUE;
				if (dlg.OpenWithMasterID(-1) == IDOK) {
					// The item has been created, so add that ID
					CDWordArray aryID;
					aryID.Add(dlg.GetInfoMasterID());
					AddItems(aryID);
				}
				*/

				// (j.jones 2007-03-14 15:12) - PLID 25195 - if adding a new item,
				// don't auto-add to the topic, instead add to our selections,
				// we will re-open the list
				if (dlg.OpenWithMasterID(-1) == IDOK) {
					// The item has been created, so add that ID to our selections
					long nNewInfoID = dlg.GetInfoMasterID();
					arynIDs.Add((long)nNewInfoID);
					nInfoIDToSelect = nNewInfoID;
				}
			}

			// (j.jones 2007-03-14 15:10) - PLID 25195 - after adding a new item
			// (or even cancelling) we should simply reopen the multi-select list
			bFinishedAdding = FALSE;
		}

		CWaitCursor pWait2;

		// (z.manning, 3/30/2006, PLID 19916) - If actions were deleted, we need to revoke them on a template.
		if(m_bIsTemplate) {
			GetLastEMN()->RevokeDeletedActions();
		}

		// (j.jones 2006-02-06 17:12) - regardless of whether we added an item, we may have changed things,
		// so update accordingly
		// (j.jones 2007-07-26 09:37) - PLID 24686 - we can now selectively update by Master ID
		// (c.haag 2008-06-12 21:56) - PLID 27831 - During our iterations, we gather an array of all the
		// details that have a matching master ID. We will use this array later on to figure out which details
		// have impending name changes.
		CArray<CEMNDetail*,CEMNDetail*> apMasterDetails;
		int i;
		for(i=0;i<dlg.m_aryOtherChangedMasterIDs.GetSize();i++) {

			long nEMRInfoMasterID = dlg.m_aryOtherChangedMasterIDs.GetAt(i);

			// (j.jones 2007-07-26 09:38) - PLID 24686 - update all items with this master info ID
			m_EMR.RefreshContentByInfoMasterID(nEMRInfoMasterID, FALSE, &apMasterDetails);

			IRowSettingsPtr pTopicRow = m_pTree->CurSel;
			if(pTopicRow) {
				EmrTreeRowType etrt = EmrTree::ChildRow(pTopicRow).GetType();
				if(etrt == etrtTopic) {
					EnsureTopicRow(pTopicRow);
					CEmrTopicWndPtr pTopicWnd = EmrTree::ChildRow(pTopicRow).GetTopicWnd();
					if(pTopicWnd) {
						// (j.jones 2007-07-26 09:31) - PLID 24686 - renamed this function and passed in the InfoMasterID
						pTopicWnd->RepositionDetailsInTopicByInfoMasterID(nEMRInfoMasterID, FALSE);
					}
				}
			}
		}

		// (c.haag 2008-06-12 21:57) - PLID 27831 - The purpose of this body of code is to look for
		// all details whose names changed as a result of editing; and for those details, update the
		// name and the rich text fields of all narratives they're bound to. All the narrative and
		// label work is done in CEMNDetail::SetLabelText.

		// First, we need to flush out details that cannot be attached to narratives...which are tables,
		// other narratives, and images.
		for (i=0; i < apMasterDetails.GetSize(); i++) {
			CEMNDetail* p = apMasterDetails[i];
			//TES 2/25/2010 - PLID 37535 - Made a new function to determine if the detail can be linked.
			if(!IsDetailLinkable(p)) {
				// Not allowed!
				apMasterDetails.RemoveAt(i--);
				break;
			}
		}

		if (apMasterDetails.GetSize() > 0) {
			// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
			_RecordsetPtr prs = CreateParamRecordset("SELECT Name, EmrInfoMasterID FROM EmrInfoT "
				"WHERE EmrInfoMasterID IN ({INTARRAY})", dlg.m_aryOtherChangedMasterIDs);
			FieldsPtr f = prs->Fields;
			while (!prs->eof && apMasterDetails.GetSize() > 0) {
				const CString strNewName = AdoFldString(f, "Name");
				const long nEMRInfoMasterID = AdoFldLong(f, "EmrInfoMasterID");
				for (i=0; i < apMasterDetails.GetSize(); i++) {
					CEMNDetail* pDetail = apMasterDetails[i];
					if (pDetail->m_nEMRInfoMasterID == nEMRInfoMasterID &&
						pDetail->GetLabelText() != strNewName)
					{
						// If we get here, we know for a fact that this detail's label will ultimately
						// update at some point. Do it now so that any narratives that it's bound to
						// will also update.
						pDetail->SetLabelText(strNewName);
						// No need to check this detail any further
						apMasterDetails.RemoveAt(i--);
					}
				}
				prs->MoveNext();
			}

		}

		// (c.haag 2008-06-17 12:22) - PLID 17842 - It may be the case that tables with linked details,
		// as well as those linked details, were added in one fell swoop. If that's the case, then it's
		// possible that tables with linked detail columns and remembered values may have been added before
		// those linked details; causing what should be remembered cells to fail to find their remembered
		// detail data. To handle this potential issue, we call DetectAndTryRepairMissingLinkedDetails.
		DetectAndTryRepairMissingLinkedDetails(apNewDetails, FALSE);

		// (j.jones 2006-10-13 09:18) - PLID 22461 - moved office visit incrementing to the save process
	}
}

void CEmrTreeWnd::ImportTopics(BOOL bImportAsSubtopics /*= FALSE*/)
{
	//First, get our current row
	IRowSettingsPtr pRow = m_pTree->CurSel;
	if(pRow) {

		long nEMNID = -1;
		CEMN *pEMN = GetEMNFromRow(pRow);
		CEMRTopic *pTopic = NULL;

		EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();

		//get the EMN info
		if(etrt == etrtTopic) {
			EnsureTopicRow(pRow);
			pTopic = EmrTree::ChildRow(pRow).GetTopic();
			pEMN = pTopic->GetParentEMN();
			nEMNID = pEMN->GetID();
		}

		//TES 5/2/2008 - PLID 27282 - Also, don't allow templates with inactive collections.
		CString strWhere = "Deleted = 0 AND CollectionID NOT IN (SELECT ID FROM EmrCollectionT WHERE Inactive = 1)";

		//if editing a template, don't allow it to be imported into itself
		if(m_bIsTemplate && nEMNID != -1) {
			strWhere.Format("Deleted = 0 AND CollectionID NOT IN (SELECT ID FROM EmrCollectionT WHERE Inactive = 1) "
				"AND ID <> %li", nEMNID);
		}

		//m.hancock - 2/21/2006 - PLID 19396 - Importing topics should allow import from multiple templates.
		//This now uses a CMultiSelectDlg instead of a CSelectDlg.
		CDWordArray arynIDs;
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "EmrTemplateT");
		//m.hancock - 2/24/2006 - PLID 19428 - Set the flag to allow text wrapping
		dlg.m_bWrapText = true;
		int nResult = dlg.Open("EmrTemplateT", strWhere, "ID", "Name", "Please select the EMN Templates to import topics from:", 0, 0xFFFFFFFF);
		if (nResult == IDOK)
			dlg.FillArrayWithIDs(arynIDs); //Get the selected IDs
		else //Cancel
			return;

		//TES 3/3/2006 - Here's the new plan.  Load each one as an EMN, then copy the topics.  This will make sure that
		//everything is positioned properly, including spawned topics and details.
		ASSERT(!bImportAsSubtopics || pTopic);
		if(bImportAsSubtopics && !pTopic) {
			MessageBox("You can not import subtopics unless you have a topic selected!");
		}
		CEMRTopic *pInsertUnder = NULL;
		if(bImportAsSubtopics) pInsertUnder = pTopic;
		else if(pTopic) pInsertUnder = pTopic->GetParentTopic();

		else pInsertUnder = NULL;

		for(int nEmn = 0; nEmn < arynIDs.GetSize(); nEmn++) {
			CEMR *pEMR = new CEMR(m_bIsTemplate);

			// (j.jones 2007-07-24 10:59) - PLID 26742 - set the current medication/allergy IDs,
			// so we don't have to query the database
			if(pEMN) {
				pEMR->SetCurrentMedicationsInfoID(pEMN->GetParentEMR()->GetCurrentMedicationsInfoID());
				pEMR->SetCurrentAllergiesInfoID(pEMN->GetParentEMR()->GetCurrentAllergiesInfoID());
				pEMR->SetCurrentGenericTableInfoID(pEMN->GetParentEMR()->GetCurrentGenericTableInfoID());
			}

			//TES 1/30/2007 - PLID 24377 - We need to track all the details we add, so that we can apply links to them.
			CArray<CEMNDetail*,CEMNDetail*> arNewDetails;
			// (j.jones 2006-06-14 13:00) - PLID 20808 - we have to call CreateNew here
			// so the EMR is properly initialized with data such as the PatientID
			// (a pEMR->SetPatientID() would theoretically be just as effective
			pEMR->CreateNew(GetPatientID(), m_bIsTemplate);
			SourceActionInfo saiBlank;
			// (a.walling 2013-01-22 10:00) - PLID 54762 - Emr Appointment linking
			CEMN *pEmnNew = pEMR->AddEMNFromTemplate(arynIDs[nEmn], saiBlank, NULL, -1);
			//TES 2/17/2007 - PLID 24768 - We used to pass in a parameter to AddEMNFromTemplate telling it to make the EMN
			// think it was new, but that led to problems where sometimes the SetNew() code would get called before the
			// EMN finished loading.  So we will just let the code create the EMN, and now that we're sure it's done, make
			// it new.
			//First, though, we have to make sure all its sourcedetail pointers are up to date, otherwise it will lose
			// its relationships when the IDs are cleared out by SetNew().
			pEmnNew->UpdateSourceDetailPointers(TRUE);
			pEmnNew->SetNew();
			for(int nTopic = 0; nTopic < pEmnNew->GetTopicCount(); nTopic++) {
				CEMRTopic *pNewTopic = NULL;
				if(pInsertUnder) {
					//TES 4/15/2010 - PLID 24692 - This is a new topic, so give it a new position entry
					pNewTopic = new CEMRTopic(pInsertUnder, new TopicPositionEntry);
					//TES 10/5/2009 - PLID 35755 - This was a manual change, so we want to re-calculate topic order indexes.
					pInsertUnder->InsertSubTopic(pNewTopic, NULL, FALSE, TRUE);
					*pNewTopic = *(pEmnNew->GetTopic(nTopic));
					//TES 1/30/2007 - PLID 24377 - Track the details we added.
					for(int nNewDetail = 0; nNewDetail < pNewTopic->GetSubTopicDetailCount(); nNewDetail++) {
						long nStartIndex = 0;
						arNewDetails.Add(pNewTopic->GetDetailByGlobalIndex(nNewDetail, nStartIndex));
					}
					//TES 3/31/2006 - If we're not on a template, we want this topic to remember what template it was imported from,
					//so that any details that are spawned will be properly positioned.
					if(!m_bIsTemplate) if(pNewTopic->GetOriginalTemplateTopicID() == -1) pNewTopic->SetOriginalTemplateTopicID(pEmnNew->GetTopic(nTopic)->GetTemplateTopicID());
				}
				else {
					//TES 4/15/2010 - PLID 24692 - This is a new topic, so give it a new position entry
					pNewTopic = new CEMRTopic(pEMN, new TopicPositionEntry);
					//TES 10/5/2009 - PLID 35755 - This was a manual change, so we want to re-calculate topic order indexes.
					pEMN->InsertTopic(pNewTopic, NULL, FALSE, TRUE);
					*pNewTopic = *(pEmnNew->GetTopic(nTopic));
					//TES 1/30/2007 - PLID 24377 - Track the details we added.
					for(int nNewDetail = 0; nNewDetail < pNewTopic->GetSubTopicDetailCount(); nNewDetail++) {
						long nStartIndex = 0;
						arNewDetails.Add(pNewTopic->GetDetailByGlobalIndex(nNewDetail, nStartIndex));
					}
					//TES 3/31/2006 - If we're not on a template, we want this topic to remember what template it was imported from,
					//so that any details that are spawned will be properly positioned.
					if(!m_bIsTemplate) if(pNewTopic->GetOriginalTemplateTopicID() == -1) pNewTopic->SetOriginalTemplateTopicID(pEmnNew->GetTopic(nTopic)->GetTemplateTopicID());
				}
				//Add to interface.
				SendMessage(NXM_EMR_TOPIC_ADDED, (WPARAM)pNewTopic);
			}

			// (j.jones 2010-03-11 08:36) - PLID 37318 - any SmartStamp items need re-linked
			pEMN->ReconfirmSmartStampLinks_PostEMNCopy();

			//TES 3/29/2006 - The topics may have spawned new EMNs, copy them as well.			
			CArray<CEMN*,CEMN*> arAddedEMNs; // (c.haag 2010-08-05 09:39) - PLID 39984 - Track what EMN's were added/spawned
			for(int nEMN = 0; nEMN < pEMR->GetEMNCount(); nEMN++) {
				CEMN *pEmnTemp = pEMR->GetEMN(nEMN);
				if(pEmnTemp != pEmnNew) {//Don't readd the one we already imported.
					CEMN *pCopy = new CEMN(&m_EMR);
					// (z.manning 2009-08-19 16:52) - PLID 35285 - We need to NOT copy the parent EMR when
					// copying the contents into the new EMN pointer or else the wrong EMR could end up 
					// getting the newly created problem pointers (if there are some) and thus they would
					// never get saved correctly.
					pCopy->CopyFromEmn(pEmnTemp, FALSE);
					// (z.manning, 09/21/2006) - PLID 22411 - We lose our parent when copying from pEmnTemp,
					// so let's make sure we set it again.
					pCopy->SetParentEMR(&m_EMR);
					m_EMR.AddEMN(pCopy);
					arAddedEMNs.Add(pCopy);
					//TES 2/16/2007 - PLID 24768 - Make sure the new copy's source details don't point to details on the
					// old copy.
					pCopy->UpdateSourceDetailsFromCopy(false);
					//TES 1/30/2007 - PLID 24377 - Also, track the details we added.
					for(int nNewDetail = 0; nNewDetail < pCopy->GetTotalDetailCount(); nNewDetail++) {
						arNewDetails.Add(pCopy->GetDetail(nNewDetail));
					}
					// (c.haag 2007-10-18 15:48) - PLID 27805 - Now that the EMN has been copied and added to
					// our EMR, flag it as having been loaded.
					pCopy->SetLoaded();
				}
			}

			// (c.haag 2010-08-05 09:39) - PLID 39984 - It may be the case that the source EMR has EMN's that
			// were spawned by details of other EMN's. We need to make sure that the source detail pointer of each
			// EMN, if they exist, do not point to anything from the source EMR.
			m_EMR.UpdateEMNSourceDetailsFromCopy(arAddedEMNs);

			// (z.manning, 4/3/2006, PLID 19865) - If any procedures, charges, or diagnosis codes
			// were spawned by item actions, we need to copy them to the actual EMN.
			BOOL bAddedADiag = FALSE;
			for(int i = 0; i < pEmnNew->GetDiagCodeCount(); i++) {
				EMNDiagCode* pDiagCode = new EMNDiagCode;
				// (z.manning 2009-08-18 10:13) - PLID 35207 - Set the override EMN as the EMN
				// we're adding the diag code to.
				pDiagCode->pEmnOverride = pEMN;
				*pDiagCode = *(pEmnNew->GetDiagCode(i));
				// Since we're not actually brining the template in, make sure to only spawn things
				// from the items on the template and ignore any diag codes, etc. that may be
				// part of the template itself.
				if(pDiagCode->sai.nSourceActionID != -1) {
					// Do not add the same diag code more than once.
					BOOL bDiagCodeAlreadyOnEMN = FALSE;
					for(int j = 0; j < pEMN->GetDiagCodeCount(); j++) {
						//TES 2/28/2014 - PLID 61046 - Check both IDs
						if(pDiagCode->nDiagCodeID == pEMN->GetDiagCode(j)->nDiagCodeID && pDiagCode->nDiagCodeID_ICD10 == pEMN->GetDiagCode(j)->nDiagCodeID_ICD10) {
							bDiagCodeAlreadyOnEMN = TRUE;
						}
					}
					if(!bDiagCodeAlreadyOnEMN) {
						SendMessage(NXM_UPDATE_CODE_EMNDIAGCODE, (WPARAM)pDiagCode, (LPARAM)pEMN);
						// (j.jones 2007-01-05 10:07) - PLID 24070 - if -1, AddDiagCode will auto-generate it,
						// and we can't just use the existing topic's index since we are appending
						pDiagCode->nOrderIndex = -1;
						pDiagCode->bHasMoved = FALSE;
						pDiagCode->bChanged = FALSE;
						// (j.jones 2014-12-23 15:07) - PLID 64491 - added bReplaced
						pDiagCode->bReplaced = FALSE;
						pEMN->AddDiagCode(pDiagCode);
						SendMessage(NXM_EMN_DIAG_ADDED, (WPARAM)pDiagCode, (LPARAM)pEMN);

						// (j.jones 2012-10-01 15:52) - PLID 52869 - track that we added a diag code
						bAddedADiag = TRUE;
					}
					else {
						//TES 6/20/2006 - Make sure this gets cleaned up!
						delete pDiagCode;
					}
				}
				else {
					//TES 6/20/2006 - Make sure this gets cleaned up!
					delete pDiagCode;
				}
			}
			for(i = 0; i < pEmnNew->GetChargeCount(); i++) {
				//DRT 1/11/2007 - PLID 24220 - This code is not documented and poorly named. (pEMNNew->GetCharge(i, ecExisting)????)
				//	I believe it's trying to copy all charges from pEmnNew to pEMN.  We now track all EMNCharge objects by
				//	pointer instead of reference, updated this code to use the new method.
				EMNCharge *pCharge = pEmnNew->GetCharge(i);

				// (z.manning 2009-02-23 12:48) - PLID 33141 - Use the new source action info class
				if(pCharge->sai.nSourceActionID != -1) {

					// (j.jones 2012-03-27 15:10) - PLID 44763 - warn if we're under a global period
					CheckWarnGlobalPeriod_EMR(pEMN->GetEMNDate());

					//Since we are copying this from 1 EMN to another, we will make a new memory object.
					EMNCharge* pNewCharge = new EMNCharge;
					// (z.manning 2009-08-18 10:13) - PLID 35207 - Set the override EMN as the EMN
					// we're adding the charge to.
					pNewCharge->pEmnOverride = pEMN;
					*pNewCharge = *pCharge;
					pNewCharge->bChanged = TRUE;	//Flag it modified since it's new

					// (j.jones 2012-08-22 09:23) - PLID 50486 - set the default insured party ID
					pNewCharge->nInsuredPartyID = pEMN->GetParentEMR()->GetDefaultChargeInsuredPartyID();

					pEMN->AddCharge(pNewCharge);

					SendMessage(NXM_EMN_CHARGE_ADDED, (WPARAM)pNewCharge, (LPARAM)pEMN);
				}
			}
			
			CArray<EMNMedication*, EMNMedication*> arypNewPrescriptions;

			for(i = 0; i < pEmnNew->GetMedicationCount(); i++) {
				// (a.walling 2007-10-01 09:15) - PLID 27568 - Use the EMNMedication pointer
				EMNMedication* pemExisting = pEmnNew->GetMedicationPtr(i);
				if (pemExisting) {
					if(pemExisting->sai.nSourceActionID != -1) {
						EMNMedication* pNewMedication = new EMNMedication;
						// (z.manning 2009-08-18 10:13) - PLID 35207 - Set the override EMN as the EMN
						// we're adding the medication to.
						pNewMedication->pEmnOverride = pEMN;
						*pNewMedication = *pemExisting; // copy
						pEMN->AddMedication(pNewMedication);
						SendMessage(NXM_EMN_MEDICATION_ADDED, (WPARAM)pNewMedication, (LPARAM)pEMN);

						// (j.jones 2012-09-28 15:46) - PLID 52922 - track that we added a medication
						// (j.jones 2013-01-09 11:55) - PLID 54530 - track the pointer
						arypNewPrescriptions.Add(pNewMedication);
					}
				} else {
					ASSERT(FALSE);
					ThrowNxException("Could not get valid medication pointer!");
				}
			}
			for(i = 0; i < pEmnNew->GetProcedureCount(); i++) {
				EMNProcedure* pProcedure = new EMNProcedure;
				*pProcedure = *(pEmnNew->GetProcedure(i));
				if(pProcedure->sai.nSourceActionID != -1) {
					SendMessage(NXM_UPDATE_NAME_EMNPROCEDURE, (WPARAM)pProcedure, (LPARAM)pEMN);
					pEMN->AddProcedure(pProcedure);
					SendMessage(NXM_EMN_PROCEDURE_ADDED, (WPARAM)pProcedure, (LPARAM)pEMN);
				}
				else {
					//TES 6/29/2006 - Make sure this gets cleaned up!
					delete pProcedure;
				}
			}

			//TES 2/16/2007 - Now, we just copied all those topics (and charges, diag codes, etc.), so their source details 
			// point to the wrong object in memory, specifically, the temporary EMN we loaded the topics in.  Tell the EMN 
			// to update all those source details to point to the correct EMN (the one we've imported into).
			pEMN->UpdateSourceDetailsFromCopy(false);

			//TES 1/30/2007 - PLID 24377 - Now, apply links to all the new details.
			m_EMR.LockSpawning();
			for(int nNewDetail = 0; nNewDetail < arNewDetails.GetSize(); nNewDetail++) {
				m_EMR.ApplyEmrLinks(arNewDetails[nNewDetail]);
			}
			m_EMR.UnlockSpawning();

			delete pEMR;

			// (j.jones 2012-09-28 15:45) - PLID 52922 - this function will check their preference
			// to save the EMN and warn about drug interactions
			// (j.jones 2012-10-01 15:52) - PLID 52869 - do the same if we added a diag. code
			if((arypNewPrescriptions.GetSize() > 0 || bAddedADiag) && !pEMN->IsLoading()) {

				// (j.jones 2013-01-11 11:18) - PLID 54530 - if we added new prescriptions, save the EMN
				if(arypNewPrescriptions.GetSize() > 0 && pEMN->IsWritable()) {

					if(SUCCEEDED(SaveEMR(esotEMN, (long)pEMN, FALSE))) {

						// (j.jones 2013-01-09 11:55) - PLID 54530 - open medication reconciliation
						CArray<long, long> aryNewPrescriptionIDs;
						for(int p=0;p<arypNewPrescriptions.GetSize(); p++) {
							EMNMedication *pMed = arypNewPrescriptions.GetAt(p);
							if(pMed->nID == -1) {
								//shouldn't be possible, we just saved this EMN
								ThrowNxException("ImportTopics addd an unsaved medication!");
							}
							else {
								aryNewPrescriptionIDs.Add(pMed->nID);
							}
						}

						ReconcileCurrentMedicationsWithNewPrescriptions(pEMN, aryNewPrescriptionIDs);

						//if reconciliation changed the current meds table, it would have forced a save

						// (j.jones 2012-10-22 17:40) - PLID 52819 - must reload medications
						pEMN->ReloadMedicationsFromData(TRUE);
					}
				}

				if(!m_bIsTemplate) {
					// (z.manning 2013-09-17 15:36) - PLID 58450 - New function for this
					pEMN->CheckSaveEMNForDrugInteractions(FALSE);
				}
			}
		}

	}
}

BOOL CEmrTreeWnd::IsEmpty()
{
	return m_EMR.GetEMNCount() == 0;
}

void CEmrTreeWnd::ShowEMN(long nEmnID)
{
	// (j.jones 2007-10-01 12:01) - PLID 27562 - added try/catch
	try {

		ShowEMN(m_EMR.GetEMNByID(nEmnID));

	}NxCatchAll("Error in CEmrTreeWnd::ShowEMN (1)");
}

IRowSettingsPtr CEmrTreeWnd::InsertPlaceholder(NXDATALIST2Lib::IRowSettings *pParentRow)
{
	IRowSettingsPtr pPlaceholder = m_pTree->GetNewRow();
	pPlaceholder->PutValue(TREE_COLUMN_ID, (long)0);
	pPlaceholder->PutValue(TREE_COLUMN_ROW_TYPE, (long)etrtPlaceholder);
	pPlaceholder->PutValue(TREE_COLUMN_OBJ_PTR, (long)0);
	pPlaceholder->PutValue(TREE_COLUMN_ICON, (long)0);
	pPlaceholder->PutValue(TREE_COLUMN_NAME, _bstr_t(""));
	pPlaceholder->PutValue(TREE_COLUMN_LOADED, (long)0);
	m_pTree->AddRowAtEnd(pPlaceholder, pParentRow);
	return pPlaceholder;
}

LRESULT CEmrTreeWnd::OnEmrItemAdded(WPARAM wParam, LPARAM lParam)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		// (a.walling 2012-06-22 14:01) - PLID 51150 - Forward to the topic if available
		CEMNDetail *pDetail = (CEMNDetail*)wParam;

		if (CEmrTopicWnd* pTopicWnd = pDetail->GetParentTopic()->GetTopicWndRaw()) {
			if (IsWindow(pTopicWnd->GetSafeHwnd())) {
				return pTopicWnd->SendMessage(NXM_EMR_ITEM_ADDED, wParam, lParam);
			}
		}

		//If we get here, it means there was no CEmrTopicWnd to send it to, so just notify ourselves that the item was 
		//just added to memory
		return SendMessage(NXM_POST_ADD_EMR_ITEM, wParam, lParam);
	} NxCatchAll("Error in OnEmrItemAdded");

	return 0;
}

void CEmrTreeWnd::OnToggleShowIfEmpty()
{
	try {
		IRowSettingsPtr pRow = m_pTree->CurSel;
		if(pRow) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
			if(etrt == etrtTopic) {
				CEMRTopic *pTopic = EmrTree::ChildRow(pRow).GetTopic();
				pTopic->SetShowIfEmpty(!pTopic->ShowIfEmpty());
				EnsureTopicBackColor(pTopic);
				EnsureTopicModifiedState(pTopic);
			}
		}
	} NxCatchAll("Error in OnToggleShowIfEmpty");
}


LRESULT CEmrTreeWnd::OnEmnRefreshCharges(WPARAM wParam, LPARAM lParam)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
		// (s.dhole 2014-02-19 09:21) - PLID 60742 change to CodeDlg
		CEmrCodesDlg *pDlg = GetCodesDlg((CEMN*)wParam, FALSE);
		if(pDlg) {
			pDlg->RefreshChargeList();
		}
	} NxCatchAll("Error in OnEmnRefreshCharges");
	
	return 0;
}

LRESULT CEmrTreeWnd::OnEmnRefreshPrescriptions(WPARAM wParam, LPARAM lParam)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
		CEMNMoreInfoDlg *pDlg = GetMoreInfoDlg((CEMN*)wParam, FALSE);
		if(pDlg) {
			pDlg->RefreshPrescriptionList();
		}
	} NxCatchAll("Error in OnEmnRefreshPrescriptions");

	return 0;
}

// (j.jones 2008-07-23 11:17) - PLID 30819 - added OnEmnRefreshDiagCodes
LRESULT CEmrTreeWnd::OnEmnRefreshDiagCodes(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
		// (r.farnworth 2014-02-19 09:01) - PLID 60746 - MoreInfoDlg changed to CodesDlg
		CEmrCodesDlg *pDlg = GetCodesDlg((CEMN*)wParam, FALSE);
		if(pDlg) {
			pDlg->RefreshDiagCodeList();
		}
	} NxCatchAll("Error in OnEmnRefreshDiagCodes");

	return 0;
}

void CEmrTreeWnd::OnEditMode()
{
	try {
#pragma TODO("SetEditMode")
		//SetEditMode(IsDlgButtonChecked(IDC_EDIT_MODE));
	} NxCatchAll("Error in OnEditMode");
}

void CEmrTreeWnd::OnSaveChanges()
{
	try {
		//the TRUE is so we do propagate new IDs, so we can
		//add new items, save, change those items, and we will
		//properly update the records rather then create new
		if(SaveEMR(esotEMR, -1, TRUE) != essSuccess)
			return;
	} NxCatchAll("Error in OnSaveChanges");
}

// (z.manning 2009-08-12 14:31) - PLID 27694 - Added save and close button
void CEmrTreeWnd::OnSaveAndClose()
{
	try
	{
		// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
		if (GetPicContainer()) {
			// (z.manning 2009-08-12 17:10) - PLID 27694 - This will handle the saving too.
			GetPicContainer()->SendMessage(NXM_CLOSE_PIC, cprNone);
		}
	}NxCatchAll(__FUNCTION__);
}

void CEmrTreeWnd::UpdateDetail(CEMNDetail* pChangedDetail)
{
	try {
		if (GetEmrPreviewCtrl()->IsLoaded()) {
			// (a.walling 2007-08-08 12:46) - PLID 27017 - We also need to ensure that we update all table
			// and narrative details that may link to us.

			// (a.walling 2007-10-09 12:07) - PLID 27017 - Should not do this unless the topic has loaded its details
			// (We don't care about the subtopics at this point, which is why we are not using IsLoaded()).
			CEMRTopic* pTopic = pChangedDetail->m_pParentTopic;
			if (pTopic != NULL && pTopic->IsPostLoadComplete()) {
				CEMN* pEMN = NULL;
				if (pTopic)
					pEMN = pTopic->GetParentEMN();

				if (pEMN) {
					long nCount = pEMN->GetTotalDetailCount();

					for (int n = 0; n < nCount; n++) {
						CEMNDetail* pCheckDetail = pEMN->GetDetail(n);

						// (a.walling 2007-10-09 12:13) - PLID 27017 - Don't try to LoadContent or GetLinkedDetails
						// if the topic has not loaded completely
						if (pCheckDetail && pCheckDetail->m_pParentTopic && (pCheckDetail->m_pParentTopic->IsPostLoadComplete())
							&& (pCheckDetail->m_EMRInfoType == eitNarrative || pCheckDetail->m_EMRInfoType == eitTable) )
						{
							if (pCheckDetail->m_EMRInfoType == eitTable) {
								// (a.walling 2007-08-08 13:07) - PLID 27017
								// need to load it's content, otherwise m_arTableElements is empty
								pCheckDetail->LoadContent();
							}
							CArray<CEMNDetail*, CEMNDetail*> arLinked;

							// (c.haag 2007-08-09 16:51) - PLID 27038 - Pass in the EMN
							pCheckDetail->GetLinkedDetails(arLinked, pEMN);
							if (arLinked.GetSize()) {
								for (int j = 0; j < arLinked.GetSize(); j++) {
									if (arLinked[j] == pChangedDetail) {
										GetEmrPreviewCtrl()->UpdateDetail(pCheckDetail);
										break;
									}
								}
							}
						}
					}
				}
				// (a.walling 2007-12-17 10:07) - PLID 28354 - Update this detail after updating the others.
				GetEmrPreviewCtrl()->UpdateDetail(pChangedDetail); 
				
				// (a.walling 2007-12-17 17:37) - PLID 28391
				pTopic->RefreshHTMLVisibility();
			}
		} else {
			GetEmrPreviewCtrl()->PendUpdateDetail(pChangedDetail);
		}
	} NxCatchAll("Error in CEmrTreeWnd::UpdateDetail");
}

LRESULT CEmrTreeWnd::OnEmrItemStateChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		//Make sure this detail's topic shows that it's been modified.
		CEMNDetail *pChangedDetail = (CEMNDetail*)wParam;
		EnsureTopicModifiedState(pChangedDetail->m_pParentTopic);

		CEmrItemValueChangedInfo* peivci = (CEmrItemValueChangedInfo*)lParam;

		// (a.walling 2007-04-09 17:59) - PLID 25549
		// (a.walling 2007-07-11 15:26) - PLID 26261 - Do not try to update when we are deleting a detail
		// (a.walling 2007-10-09 11:53) - PLID 27017 - No point doing any of this if the preview is not loaded
		if ((peivci->bDeleted == FALSE) && (GetEmrPreviewCtrl() != NULL) ) {
			// (a.walling 2008-05-14 16:25) - PLID 29114 - Large batches of changes was updating one by one. We can handle this better by pending the detail
			// updates until after a specified period of time. This way, typing into a narrative or drawing on a very large image etc can be much more efficient.

			long nDelay = GetPreviewRefreshDelay(pChangedDetail->m_EMRInfoType);

			if (nDelay > 0) {
				BOOL bDummy;
				if (!m_mapDetailsPendingUpdate.Lookup(pChangedDetail, bDummy)) {
					m_mapDetailsPendingUpdate[pChangedDetail] = TRUE;
					//pChangedDetail->AddRef();
					// (a.walling 2009-10-12 16:05) - PLID 36024
					pChangedDetail->__AddRef("CEmrTreeWnd::OnEmrItemStateChanged pend update");
					//TRACE("**>>      New Detail 0x%08x (%s) pending update at %lu\n", pChangedDetail, pChangedDetail->GetLabelText(), GetTickCount());
				} else {
					//TRACE("**>> Existing Detail 0x%08x (%s) pending update at %lu\n", pChangedDetail, pChangedDetail->GetLabelText(), GetTickCount());
				}
				SetTimer(IDT_PREVIEW_REFRESH, nDelay, NULL);			
			} else {
				UpdateDetail(pChangedDetail);
			}
		}

		BOOL bAllNonEmpty = EnsureTopicBackColor(pChangedDetail->m_pParentTopic);

		//Don't auto-advance on templates
		if(!m_bIsTemplate) {
			//TES 10/2/2006 - PLID 21709 - Only auto-advance if this detail is on the currently selected topic (sometimes the code
			// was auto-advancing twice for the same topic, thus resulting in a topic being skipped).
			if(m_pTree->CurSel != NULL && (CEMRTopic*)VarLong(m_pTree->CurSel->GetValue(TREE_COLUMN_OBJ_PTR)) ==
				pChangedDetail->m_pParentTopic) {
				// Decide if we'll be auto-advancing to the next tab
				// (b.cardillo 2005-06-30 18:08) - PLID 16353 - Take the EmnTabAutoAdvance user 
				// preference into account
				BOOL bAutoAdvance;

				// (a.walling 2008-02-07 09:05) - PLID 28811 - Use the detail's IsStateSet member function for consistency
				// which would handle the possibility of a pending info type, and future changes in IsStateSet, and because
				// only the detail itself and here calls the EmrUtils IsDetailStateSet, everywhere else uses ->IsStateSet.
				// Although checking for ->m_EMRInfoType == eitSingleList does not take the pending info type into account,
				// there is no way we would be in here before everything was synched in the RepositionDetails functions,
				// since that will happen as soon as the topic is interactive.
				if (pChangedDetail->m_EMRInfoType == eitSingleList && bAllNonEmpty && 
					GetRemotePropertyInt("EmnTabAutoAdvance", 0, 0, GetCurrentUserName(), true) && 
					!pChangedDetail->IsStateSet(&(peivci->varOldState))  && 
					pChangedDetail->IsStateSet(&(peivci->varNewState)) ) 
				{
					// We can advance because the one edited was a single-select list and 
					// all details on this tab are non-empty
					bAutoAdvance = TRUE;
				} else {
					// We can't advance, either because it wasn't a single-sel list, or 
					// because some other detail on this tab still hasn't been set.
					// (b.cardillo 2005-06-30 17:57) - PLID 16353 - Or because the user 
					// preference doesn't allow it.
					bAutoAdvance = FALSE;
				}

				// Then auto-advance if we decided to above
				if (bAutoAdvance) {
					OnNextNode();
				}
			}
		}
	} NxCatchAll("Error in OnEmrItemStateChanged");

	return 0;
}

void CEmrTreeWnd::EnsureTopicModifiedState(CEMRTopic* pTopic)
{
	IRowSettingsPtr pTopicRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pTopic, NULL, FALSE);
	CEMRTopic *pActualTopic = pTopic;
	if(pTopicRow == NULL) {
		//Find the highest level parent that actually exists.
		CEMRTopic *pParentTopic = pTopic->GetParentTopic();
		while(pParentTopic && pTopicRow == NULL) {
			pTopicRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pParentTopic, NULL, FALSE);
			pParentTopic = pParentTopic->GetParentTopic();
		}
		if(pTopicRow == NULL) {
			//Use the EMN node.
			pTopicRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pTopic->GetParentEMN(), NULL, FALSE);
		}
		else {
			pActualTopic = pParentTopic;
		}
	}
	if(pTopicRow == NULL) {
		//We couldn't find any row related to this topic at all, just give up.
		return;
	}
	else {
		EnsureModifiedState(pTopicRow);
	}
}

void CEmrTreeWnd::EnsureModifiedState(IRowSettingsPtr pRow)
{
	try {

		EnsureRowIcon(pRow);
		
		EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
		if(etrt == etrtTopic) {
			CEMRTopic *pTopic = EmrTree::ChildRow(pRow).GetTopic();

			//Now, we always need to update the EMN row.
			CEMN *pEMN = pTopic->GetParentEMN();
			IRowSettingsPtr pEmnRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pEMN, NULL, FALSE);
			if(pEmnRow) {
				EnsureRowIcon(pEmnRow);
			}

			//Also, if we are underneath a collapsed parent topic, update that parent topic.
			BOOL bAllExpanded = TRUE;
			IRowSettingsPtr pParent = pRow->GetParentRow();
			IRowSettingsPtr pHighestCollapsed = NULL;
			while(pParent) {
				if(!pParent->Expanded) {
					bAllExpanded = FALSE;
					pHighestCollapsed = pParent;
				}
				pParent = pParent->GetParentRow();
			}
			if(!bAllExpanded) {
				//Update pHighestCollapsed (if it's a topic, otherwise it's an EMN and we handled it already.)
				EmrTreeRowType etrt = EmrTree::ChildRow(pHighestCollapsed).GetType();
				if(etrt == etrtTopic) {
					EnsureRowIcon(pHighestCollapsed);
				}
			}
		}

	}NxCatchAll("Error in CEmrTreeWnd::EnsureModifiedState()");
}

void CEmrTreeWnd::EnsureRowIcon(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	//TES 9/9/2009 - PLID 35495 - Throughout this function, the GetIcon() member function has been changed to 
	// a global GetEmrTreeIcon() function, which also takes the icon size as a parameter.
	//TES 9/16/2009 - PLID 35495 - Update: this now takes a single boolean parameter, for whether we're using
	// the 24x24 or 32x32 icons.
	bool bUseSmallIcons = (m_nIconSize == 24) ? true : false;
	if(!bUseSmallIcons) {
		ASSERT(m_nIconSize == 32);
	}

	EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
	switch(etrt) {
	case etrtEmn:
		{
			CEMN *pEMN = EmrTree::ChildRow(pRow).GetEMN();
			// (a.walling 2008-07-23 14:09) - PLID 30790 - Use EMN icons with problem flags if we have problems.
			BOOL bHasProblems = pEMN->HasProblems();
			BOOL bHasOnlyClosedProblems = bHasProblems ? pEMN->HasOnlyClosedProblems() : FALSE;
			BOOL bReadOnly = !pEMN->IsWritable();
			if(pEMN->GetStatus() == 2) {
				//Locked
				if (bReadOnly) {
					if (bHasProblems) {
						pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(bHasOnlyClosedProblems ? etiReadOnlyLockedEmnClosedProblems : etiReadOnlyLockedEmnProblems, bUseSmallIcons));
					} else {
						pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiReadOnlyLockedEmn, bUseSmallIcons));
					}
				} else {
					if (bHasProblems) {
						pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(bHasOnlyClosedProblems ? etiLockedEmnClosedProblems : etiLockedEmnProblems, bUseSmallIcons));
					} else {
						pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiLockedEmn, bUseSmallIcons));
					}
				}
			}
			else {
				if(pEMN->IsUnsaved()) {
					if(EmrTree::ChildRow(pRow).IsLoaded()) {
						//Open, unsaved
						if (bHasProblems) {
							if (bReadOnly) {
								pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(bHasOnlyClosedProblems?etiReadOnlyUnsavedEmnClosedProblems:etiReadOnlyUnsavedEmnProblems, bUseSmallIcons));
							} else {
								pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(bHasOnlyClosedProblems?etiOpenUnsavedEmnClosedProblems:etiOpenUnsavedEmnProblems, bUseSmallIcons));
							}
						} else {
							pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(bReadOnly?etiReadOnlyUnsavedEmn:etiOpenUnsavedEmn, bUseSmallIcons));
						}
					}
					else {
						//Unopened, unsaved.
						if (bHasProblems) {
							if (bReadOnly) {
								pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(bHasOnlyClosedProblems?etiReadOnlyUnsavedEmnClosedProblems:etiReadOnlyUnsavedEmnProblems, bUseSmallIcons));
							} else {
								pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(bHasOnlyClosedProblems?etiFinishedUnsavedEmnClosedProblems:etiFinishedUnsavedEmnProblems, bUseSmallIcons));
							}
						} else {
							pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(bReadOnly?etiReadOnlyUnsavedEmn:etiFinishedUnsavedEmn, bUseSmallIcons));
						}
					}
				}
				else {
					if(EmrTree::ChildRow(pRow).IsLoaded()) {
						//Open, saved
						if (bHasProblems) {
							if (bReadOnly) {
								pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(bHasOnlyClosedProblems?etiReadOnlyEmnClosedProblems:etiReadOnlyEmnProblems, bUseSmallIcons));
							} else {
								pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(bHasOnlyClosedProblems?etiOpenSavedEmnClosedProblems:etiOpenSavedEmnProblems, bUseSmallIcons));
							}
						} else {
							pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(bReadOnly?etiReadOnlyEmn:etiOpenSavedEmn, bUseSmallIcons));
						}
					}
					else {
						//Unopened, saved
						if (bHasProblems) {
							if (bReadOnly) {
								pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(bHasOnlyClosedProblems?etiReadOnlyEmnClosedProblems:etiReadOnlyEmnProblems, bUseSmallIcons));
							} else {
								pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(bHasOnlyClosedProblems?etiFinishedSavedEmnClosedProblems:etiFinishedSavedEmnProblems, bUseSmallIcons));
							}
						} else {
							pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(bReadOnly?etiReadOnlyEmn:etiFinishedSavedEmn, bUseSmallIcons));
						}
					}
				}
			}
		}
		break;
	case etrtTopic:
		{
			CEMRTopic *pTopic = EmrTree::ChildRow(pRow).GetTopic();
			// (a.walling 2008-07-23 16:19) - PLID 30790 - Only show if the topic itself is marked as having a problem
			BOOL bHasProblems = pTopic->HasProblems();
			BOOL bHasOnlyClosedProblems = bHasProblems ? pTopic->HasOnlyClosedProblems() : FALSE;

			//Now, if this row is expanded, just use its modified state, otherwise, check if any children are modified.
			if(pTopic->IsUnsaved(!pRow->Expanded)) {
				if(EmrTree::ChildRow(pRow).GetTopicWnd()) {
					//Open, unsaved
					if (!bHasProblems) {
						pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiOpenUnsavedTopic, bUseSmallIcons));
					} else {
						if (!bHasOnlyClosedProblems) {
							pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiOpenUnsavedTopicProblems, bUseSmallIcons));
						} else {
							pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiOpenUnsavedTopicClosedProblems, bUseSmallIcons));
						}
					}
				}
				else {
					//Unopened, unsaved.
					if (!bHasProblems) {
						pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiClosedUnsavedTopic, bUseSmallIcons));
					} else {
						if (!bHasOnlyClosedProblems) {
							pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiClosedUnsavedTopicProblems, bUseSmallIcons));
						} else {
							pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiClosedUnsavedTopicClosedProblems, bUseSmallIcons));
						}
					}
				}
			}
			else {
				if(EmrTree::ChildRow(pRow).GetTopicWnd()) {
					//Open, saved.
					if (!bHasProblems) {
						pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiOpenSavedTopic, bUseSmallIcons));
					} else {
						if (!bHasOnlyClosedProblems) {
							pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiOpenSavedTopicProblems, bUseSmallIcons));
						} else {
							pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiOpenSavedTopicClosedProblems, bUseSmallIcons));
						}
					}
				}
				else {
					//Unopened, saved
					if (!bHasProblems) {
						pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiClosedSavedTopic, bUseSmallIcons));
					} else {
						if (!bHasOnlyClosedProblems) {
							pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiClosedSavedTopicProblems, bUseSmallIcons));
						} else {
							pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiClosedSavedTopicClosedProblems, bUseSmallIcons));
						}
					}
				}
			}
		}
		break;
	case etrtMoreInfo:
		{
			// (a.walling 2008-04-29 09:20) - PLID 29815 - Use the new more info icons
			CEMN *pEMN = GetEMNFromRow(pRow);
			if(pEMN->IsMoreInfoUnsaved()) {
				if(EmrTree::ChildRow(pRow).GetTopicWnd()) {
					//Open, unsaved.
					pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiOpenUnsavedMoreInfo, bUseSmallIcons));
				}
				else {
					//Unopened, unsaved.
					pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiClosedUnsavedMoreInfo, bUseSmallIcons));
				}
			}
			else {
				// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
				if(GetEmrFrameWnd()->GetMoreInfoDlg(pEMN)) {
					//Open, saved.
					pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiOpenSavedMoreInfo, bUseSmallIcons));
				}
				else {
					//Unopened, saved.
					pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiClosedSavedMoreInfo, bUseSmallIcons));
				}
			}
		}
		break;
		//TES 2/12/2014 - PLID 60748 - Handle the <Codes> row
		case etrtCodes:
		{
			CEMN *pEMN = GetEMNFromRow(pRow);
			if(pEMN->IsCodesUnsaved()) {
				if(EmrTree::ChildRow(pRow).GetTopicWnd()) {
					//Open, unsaved.
					pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiOpenUnsavedCodes, bUseSmallIcons));
				}
				else {
					//Unopened, unsaved.
					pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiClosedUnsavedCodes, bUseSmallIcons));
				}
			}
			else {
				if(GetEmrFrameWnd()->GetEmrCodesDlg(pEMN)) {
					//Open, saved.
					pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiOpenSavedCodes, bUseSmallIcons));
				}
				else {
					//Unopened, saved.
					pRow->PutValue(TREE_COLUMN_ICON, (long)GetEmrTreeIcon(etiClosedSavedCodes, bUseSmallIcons));
				}
			}
		}
		break;
	default:
		//This isn't valid to have an icon!
		ASSERT(FALSE);
		break;
	}
}

void CEmrTreeWnd::CheckDuplicateTopics(NXDATALIST2Lib::IRowSettingsPtr pParentRow)
{
	//Go through all child rows.
	IRowSettingsPtr pRow = pParentRow->GetFirstChildRow();
	while(pRow) {
		//Only process topics.
		if(etrtTopic == EmrTree::ChildRow(pRow).GetType()) {
			CEMRTopic *pTopic = EmrTree::ChildRow(pRow).GetTopic();
			BOOL bRenamed = FALSE;
			CString strSourceAction = "";
			//Only look at topics that are visible
			if (pTopic->GetVisible()) {
				//Go through the rest of the topics.
				IRowSettingsPtr pRow2 = pRow->GetNextRow();
				while(pRow2) {
					//Only process topics
					if(etrtTopic == EmrTree::ChildRow(pRow2).GetType()) {
						CEMRTopic *pTopic2 = EmrTree::ChildRow(pRow2).GetTopic();
						//Only look at topics that are visible
						if (pTopic2->GetVisible()) {
							//OK, we've got two non topic rows.  Are they duplicates?
							if(pTopic->GetName() == pTopic2->GetName()) {
								//Yup.  Do they have different source actions?
								//TES 3/18/2010 - PLID 37530 - The source actions can be the same iff this is a Smart Stamp action
								// (a.walling 2010-04-06 08:57) - PLID 38059 - This can also be true for table dropdowns. So let's just simply check for that sourcetype. We could
								// check even deeper for more information, but I think that's beyond the point, which is to distinguish sibling topics with the same names.
								if(pTopic->GetSourceActionID() != pTopic2->GetSourceActionID() || pTopic->GetSourceActionInfo().eaoSourceType == eaoSmartStamp || pTopic->GetSourceActionInfo().eaoSourceType == eaoEmrTableDropDownItem) {
									//Yup.  Are they named differently?
									CString strSourceAction2;
									if(strSourceAction.IsEmpty() && pTopic->GetSourceActionID() != -1)
										strSourceAction = pTopic->GetSourceActionName();
									if(pTopic2->GetSourceActionID() != -1)
										strSourceAction2 = pTopic2->GetSourceActionName();

									if(strSourceAction != strSourceAction2) {
										//Yes.  Rename!
										bRenamed = TRUE; //This will tell pRow to rename itself once it's gone through all other rows.
										//TES 4/4/2006 - PLID 19903 - Only rename it if it was spawned 
										//(if both this and pRow were unspawned, we wouldn't get here, so we know one of them will be
										//renamed).
										if(!strSourceAction2.IsEmpty()) {
											// (a.walling 2007-10-09 13:20) - PLID 25548 - Update the preview as well
											CString strNewName = pTopic2->GetName() + " (" + strSourceAction2 + ")";
											pRow2->PutValue(TREE_COLUMN_NAME, _bstr_t(strNewName));
											if (GetEmrPreviewCtrl()) {
												GetEmrPreviewCtrl()->UpdateTopicTitle(pTopic2, strNewName);
											}
										}
									}
								}
							}
						}
					}
					pRow2 = pRow2->GetNextRow();
				}
			}
			if(bRenamed) {
				//TES 4/4/2006 - PLID 19903 - Only rename it if it was spawned 
				//(if both this and pRow2 were unspawned, we wouldn't get here, so we know one of them will be
				//renamed).
				if(!strSourceAction.IsEmpty()) {
					// (a.walling 2007-10-09 14:14) - PLID 25548 - Update the preview as well
					CString strNewName = pTopic->GetName() + " (" + strSourceAction + ")";
					pRow->PutValue(TREE_COLUMN_NAME, _bstr_t(strNewName));
					if (GetEmrPreviewCtrl()) {
						GetEmrPreviewCtrl()->UpdateTopicTitle(pTopic, strNewName);
					}
				}
			}
		}
		pRow = pRow->GetNextRow();
	}
}

//TES 5/20/2008 - PLID 27905 - This is sometimes called when procedures are about to be deleted; if so, pass in the EMN
// they're being deleted from and this will only return true if the procedure is on some other EMN.
BOOL CEmrTreeWnd::IsProcedureInEMR(long nProcedureID, CEMN *pExcludingEmn /*= NULL*/)
{
	return m_EMR.IsProcedureInEMR(nProcedureID, pExcludingEmn);
}

BOOL CEmrTreeWnd::IsEMRUnsaved()
{
	return m_EMR.IsEMRUnsaved();
}

// (j.jones 2011-07-15 13:45) - PLID 42111 - takes in an image file name (could be a path),
// and returns TRUE if any Image detail on this EMR references it
BOOL CEmrTreeWnd::IsImageFileInUseOnEMR(const CString strFileName)
{
	return m_EMR.IsImageFileInUse(strFileName);
}

// (j.jones 2007-08-30 09:29) - PLID 27243 - Office Visit incrementing is no longer
// used in the L2 EMR, it's in Custom Records only
/*
LRESULT CEmrTreeWnd::OnProcessEMROfficeVisits(WPARAM wParam, LPARAM lParam)
{
	try {
		FOR_ALL_ROWS(m_pTree) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
			if(etrt == etrtEmn) {
				CEMN *pEMN = EmrTree::ChildRow(pRow).GetEMN();
				if(pEMN->GetStatus() != 2) {
					pEMN->TryToIncrementOfficeVisit();
				}
			}
			GET_NEXT_ROW(m_pTree)
		}
	} NxCatchAll("Error in OnProcessEMROfficeVisits");

	return 0;
}
*/

CEMN* CEmrTreeWnd::GetLastEMN()
{
	CEMN *pLastEMN = NULL;

	FOR_ALL_ROWS(m_pTree) {
		EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
		if(etrt == etrtEmn) {
			pLastEMN = EmrTree::ChildRow(pRow).GetEMN();
		}
		GET_NEXT_ROW(m_pTree)
	}

	return pLastEMN;
}

int CEmrTreeWnd::GetEMNCount()
{
	return m_EMR.GetEMNCount();
}

CEMN* CEmrTreeWnd::GetEMN(int nIndex)
{
	return m_EMR.GetEMN(nIndex);
}

CEMN* CEmrTreeWnd::GetEMNByID(int nID)
{
	int nEMNCount = GetEMNCount();
	for (int i=0; i < nEMNCount; i++) {
		CEMN* pEMN = GetEMN(i);
		if (nID == pEMN->GetID()) {
			return pEMN;
		}
	}
	return NULL;
}

void CEmrTreeWnd::OnToggleHideOnEmn()
{
	try {
		IRowSettingsPtr pTopicRow = m_pTree->CurSel;
		if(pTopicRow) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pTopicRow).GetType();
			if(etrt == etrtTopic) {
				CEMRTopic *pTopic = EmrTree::ChildRow(pTopicRow).GetTopic();
				pTopic->SetHideOnEMN(!pTopic->HideOnEMN());
				EnsureTopicBackColor(pTopic);
				EnsureTopicModifiedState(pTopic);
			}
			//Its children's appearance may also change.
			FOR_ALL_ROWS_WITH_PARENT(m_pTree, pTopicRow) {
				if(etrtTopic == EmrTree::ChildRow(pRow).GetType()) {
					EnsureTopicBackColor(EmrTree::ChildRow(pRow).GetTopic());
				}
				GET_NEXT_ROW(m_pTree)
			}

		}
	} NxCatchAll("Error in OnToggleHideOnEmn");
}

void CEmrTreeWnd::OnCopyEmn()
{
	try {

		CEMN *pEMN = (CEMN*)VarLong(m_pTree->CurSel->GetValue(TREE_COLUMN_OBJ_PTR));
		CEMN *pNewEMN = new CEMN(&m_EMR);
		*pNewEMN = *pEMN;

		// (j.jones 2011-07-05 11:39) - PLID 43603 - this now takes in a class
		EMNStatus emnStatus;
		emnStatus.nID = 0;
		emnStatus.strName = "Open";
		pNewEMN->SetStatus(emnStatus);

		// (j.jones 2009-09-24 10:13) - PLID 29718 - added preference to default to the last appt. date
		COleDateTime dtToUse = COleDateTime::GetCurrentTime();
		if(GetRemotePropertyInt("EMNUseLastApptDate", 0, 0, "<None>", true) == 1) {
			COleDateTime dtLast = GetLastPatientAppointmentDate(m_EMR.GetPatientID());
			//make sure the appt. isn't before the patient's birthdate
			COleDateTime dtBirthDate = m_EMR.GetPatientBirthDate();		
			if(dtLast.GetStatus() != COleDateTime::invalid
				&& (dtBirthDate.GetStatus() == COleDateTime::invalid || dtLast >= dtBirthDate)) {
				dtToUse = dtLast;
			}
		}

		pNewEMN->SetEMNDate(dtToUse);
		// (j.jones 2007-01-23 09:13) - PLID 24027 - source detail IDs/pointers need reassigned
		// prior to marking all the items as new
		//TES 5/29/2014 - PLID 52705 - Tell it to go ahead and reset the source IDs (this was the old default behavior)
		pNewEMN->UpdateSourceDetailsFromCopy(true);
		// (c.haag 2008-07-16 12:47) - PLID 30752 - Copy todo alarms
		pNewEMN->CopyTodoAlarms(pEMN);
		// (c.haag 2007-05-30 12:33) - PLID 25495 - Narratives won't show up correctly unless
		// we call SetLoaded, which will flag the EMN as having been completely loaded, and open
		// to post-load activities such as LoadAllNarratives. I think this applies to 26168 as well.
		// (a.walling 2009-12-29 09:11) - PLID 36659 - Flag to NOT generate the preview, since the generated preview
		// will still have the saved ID information from the original EMN.
		pNewEMN->SetLoaded(false);
		// (a.walling 2007-09-27 14:17) - PLID 25548 - Retain the order indexes of the topics
		pNewEMN->SetNew(TRUE);
		// (a.walling 2010-08-24 14:37) - PLID 37923 - Clear the source action info
		pNewEMN->ClearSourceActionInfo();
		
		// (j.jones 2013-06-18 13:23) - PLID 47217 - if the preference says to do so, remove signature details
		if(GetRemotePropertyInt("EMNCopy_SkipSignatures", 1, 0, "<None>", true) == 1) {
			pNewEMN->RemoveNewSignatureDetails();
		}

		// (a.walling 2009-12-29 09:11) - PLID 36659 - Now that everything has been set as new, we can generate the preview
		pNewEMN->GenerateHTMLFile(TRUE, FALSE);

		//(e.lally 2011-12-14) PLID 46968 - Reset the patient created status
		pNewEMN->SetPatientCreatedStatus(CEMN::pcsCreatedByPractice);

		// (z.manning, 06/27/2006) - PLID 20896 - Do this so we audit the fact that this EMN was copied.
		CString strAuditOldValue = FormatString("Copied from: %s", VarString(m_pTree->CurSel->GetValue(TREE_COLUMN_NAME)));
		pNewEMN->SetOldAuditValue(strAuditOldValue);

		m_EMR.AddEMN(pNewEMN);
		ShowEMN(pNewEMN);

		// (d.thompson 2013-11-06) - PLID 59352 - Audit that this (source) EMN was copied.
		AuditEvent(m_EMR.GetPatientID(), GetExistingPatientName(m_EMR.GetPatientID()), BeginNewAuditEvent(), aeiEMNCopied, pEMN->GetID(), "This EMN was copied within this EMR",  "", aepHigh, aetCreated);

	} NxCatchAll("Error in OnCopyEmn");
}

BOOL IsParent(IRowSettingsPtr pPossibleParent, IRowSettingsPtr pRow)
{
	IRowSettingsPtr pParent = pRow;
	while(pParent) {
		if(pParent == pPossibleParent) return TRUE;
		pParent = pParent->GetParentRow();
	}
	return FALSE;
}

BOOL CEmrTreeWnd::IsEMREMChecklistDlgVisible()
{
	// (c.haag 2007-08-30 17:02) - PLID 27058 - Returns TRUE if the E/M Checklist Setup dialog is visible
	if (!IsWindow(m_dlgEMChecklistSetup.GetSafeHwnd())) {
		return FALSE;
	}
	return m_dlgEMChecklistSetup.IsWindowVisible();
}

void CEmrTreeWnd::BringEMREMChecklistDlgToTop()
{
	// (c.haag 2007-08-30 17:03) - PLID 27058 - Brings the E/M Checklist Setup window to the top of the desktop
	if (IsWindow(m_dlgEMChecklistSetup.GetSafeHwnd())) {
		m_dlgEMChecklistSetup.ShowWindow(SW_RESTORE);
		m_dlgEMChecklistSetup.SetForegroundWindow();
	}
}

void CEmrTreeWnd::AutoCollapse()
{
	//First, make sure all the rows that need to be expanded are.
	m_pTree->SetRedraw(FALSE);
	IRowSettingsPtr pCurSel = m_pTree->CurSel;
	IRowSettingsPtr pParent = pCurSel->GetParentRow();
	IRowSettingsPtr pTopParent = pCurSel;
	while(pParent) {
		pParent->Expanded = TRUE;
		pTopParent = pParent;
		pParent = pParent->GetParentRow();
	}
	//Now, collapse everything that should be, based on our preference.
	if(m_nAutoCollapse == 2) {
		//Collapse everything that isn't a direct parent.
		FOR_ALL_ROWS(m_pTree) {
			if(!IsParent(pRow, pCurSel)) pRow->Expanded = FALSE;
			GET_NEXT_ROW(m_pTree)
		}
	}
	else if(m_nAutoCollapse == 1) {
		//Collapse all EMNs that aren't the same as the new row's top parent (EMN) row.
		IRowSettingsPtr pRow = m_pTree->GetFirstRow();
		while(pRow) {
			if(pRow != pTopParent) pRow->Expanded = FALSE;
			pRow = pRow->GetNextRow();
		}
	}
	m_pTree->SetRedraw(TRUE);
}

LRESULT CEmrTreeWnd::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	// (c.haag 2006-03-01 11:43) - PLID 19208 - Handle real-time changes to the EMR data from other
	// Practice clients.
	_RecordsetPtr prs;

	try {
		// (a.walling 2007-10-19 12:43) - PLID 23714 - EMNs that have been updated which may need to have narratives updated
		CMap<CEMN*, CEMN*, long, long&> mapEMNsToUpdateNarratives;

		switch (wParam)
		{

		// (c.haag 2007-02-13 10:19) - PLID 24721 - We no longer need to handle EmrInfoT-related
		// table checkers. When an EmrInfoT record changes, existing details on patient charts
		// are unchanged with one exception: The detail that the user right-clicked on to edit.
		// We already have code in place to retroactively update that item, so there's no need
		// to care about this table checker.
		//
		// As for EMR templates, we have a development rule stating that modal dialogs should not
		// handle table checker messages. If we decide to let users edit multiple templates at
		// once, we may want to restore this code, but still take out pDetail->LoadContent().
		//
		// As for NetUtils::EMRDataT_By_InfoID, EmrSetupDlg.cpp posts it when an item is deleted.
		// An item cannot be deleted if it's in use by patient details, so it's moot here.
		//
		// As for NetUtils::EMRDataT, EmrSetupDlg.cpp posts that too. As before, existing details
		// on patient charts are unchanged, and an existing detail being edited will reload its
		// content without a table checker, so it's moot.
		//
		//
		// Now, if the following code is ever commented back in, pDetail->LoadContent() must be
		// removed. Otherwise, Practice could crash because it could theoretically be called
		// when CEMNDetail::LoadContent is already lower in the call stack (this was proven per
		// PLID 24721). Calling LoadContent in LoadContent is bad.
		//
		//
		/*
		case NetUtils::EMRInfoT:
		case NetUtils::EMRDataT_By_InfoID: {
			long nEMNs = m_EMR.GetEMNCount();

			//
			// Go through all the details of all the EMN's and see if their EMRInfoID
			// value matches that of the parameter. If it does, flag it for refreshing.
			// 
			for (int i=0; i < nEMNs; i++) {
				CEMN* pEMN = m_EMR.GetEMN(i);

				// (z.manning, 01/29/2007) - PLID 24459 - The same info item can exist on a locked and an
				// unlocked EMN within the same EMR, so make sure we don't reload any details on a locked
				// EMN or else it may mark something as unsaved.
				if(pEMN->GetStatus() != 2) {
					long nDetails = pEMN->GetTotalDetailCount();
					for (int j=0; j < nDetails; j++) {
						CEMNDetail* pDetail = pEMN->GetDetail(j);

						// (c.haag 2004-06-01 15:30) - If the ID is -1 in a table checker, that
						// means we refresh everything.
						if (lParam == -1 || lParam == pDetail->m_nEMRInfoID) {
							pDetail->SetNeedContentReload();
							pDetail->LoadContent();
						}
					}
				}
			}

			} break;
		case NetUtils::EMRDataT:
			prs = CreateRecordset("SELECT EMRInfoID FROM EMRDataT WHERE ID = %d", lParam);
			if (!prs->eof) {
				OnTableChanged(NetUtils::EMRDataT_By_InfoID, AdoFldLong(prs, "EMRInfoID"));
			}
			break;*/
		case NetUtils::EmrInfoMasterT: {
				//TES 12/8/2006 - PLID 23790 - Any details that were up-to-date may not be any more.
				long nEMNs = m_EMR.GetEMNCount();
				for(int i = 0; i < nEMNs; i++) {
					CEMN* pEMN = m_EMR.GetEMN(i);
					long nDetails = pEMN->GetTotalDetailCount();
					for (int j=0; j < nDetails; j++) {
						CEMNDetail* pDetail = pEMN->GetDetail(j);

						// (c.haag 2004-06-01 15:30) - If the ID is -1 in a table checker, that
						// means we refresh everything.
						if (lParam == -1 || lParam == pDetail->m_nEMRInfoMasterID) {
							pDetail->RefreshIsActiveInfo();
						}
					}
				}
			}
			break;

		// (j.jones 2007-07-23 10:51) - PLID 26742 - if the medications or allergies info ID changed,
		// we will receive a tablechecker
		case NetUtils::CurrentMedicationsEMRInfoID:
			m_EMR.SetCurrentMedicationsInfoID((long)lParam);
			break;

		case NetUtils::CurrentAllergiesEMRInfoID:
			m_EMR.SetCurrentAllergiesInfoID((long)lParam);
			break;

		// (j.jones 2010-06-21 15:36) - PLID 37981 - supported generic tables
		case NetUtils::CurrentGenericTableEMRInfoID:
			m_EMR.SetCurrentGenericTableInfoID((long)lParam);
			break;

		// (a.walling 2007-07-27 10:22) - PLID 23714 - Update EMN data with any changed info
		case NetUtils::DiagCodes:
			try {
			if (lParam == -1) {
				// this is a mass-refresh. I'd prefer not to reload every single object on the EMR; instead I've gone
				// through and tried to use an ID for all TableChanged messages that I check here. However some situations
				// will require to refresh everything. I'm concerned about performance, so for now we'll just ignore.
				// refresh everything, ugh
			} else {
				// refresh this particular item in all EMNs.
				_RecordsetPtr rsUpdate = NULL;
				for (int i = 0; i < m_EMR.GetEMNCount(); i++) {
					CEMN* pEMN = m_EMR.GetEMN(i);
					if (pEMN) {
						// (j.jones 2008-07-23 10:20) - PLID 30819 - changed to reference the diag code ID
						//TES 2/26/2014 - PLID 60807 - With ICD10, there could potentially be multiple codes with the same DiagCodeID
						CArray<EMNDiagCode*,EMNDiagCode*> arCodes;
						pEMN->GetDiagCodesByDiagID(lParam, arCodes);
						for(int nDiag = 0; nDiag < arCodes.GetSize(); nDiag++) {
							EMNDiagCode* pDiag = arCodes[nDiag];
							if (rsUpdate == NULL) {
								// query for updated info
								rsUpdate = CreateParamRecordset("SELECT CodeNumber, CodeDesc FROM DiagCodes WHERE ID = {INT}", lParam);
							}

							// update diag with this info
							if (!rsUpdate->eof) {
								//TES 2/26/2014 - PLID 60807 - Update the ICD-9 values or ICD-10 values as appropriate
								if(pDiag->nDiagCodeID == (long)lParam) {
									pDiag->strCode = AdoFldString(rsUpdate, "CodeNumber", "");
									pDiag->strCodeDesc = AdoFldString(rsUpdate, "CodeDesc", "");
								}
								else {
									ASSERT(pDiag->nDiagCodeID_ICD10 == (long)lParam);
									pDiag->strCode_ICD10 = AdoFldString(rsUpdate, "CodeNumber", "");
									pDiag->strCodeDesc_ICD10 = AdoFldString(rsUpdate, "CodeDesc", "");
								}
							} else {
								// the diag code was deleted!
								ASSERT(FALSE);
							}
						}
						
						// (r.farnworth 2014-02-19 09:01) - PLID 60746 - MoreInfoDlg changed to CodesDlg
						CEmrCodesDlg* pCodesDlg = GetCodesDlg(pEMN, FALSE);
						if (pCodesDlg) {
							pCodesDlg->RefreshDiagCodeList();
						}
						UpdatePreviewMoreInfo(pEMN);
					}
				}
			}
			} NxCatchAllThrow("Error refreshing table DiagCodes");
			break;

		case NetUtils::AptPurposeT:
			try {
			if (lParam == -1) {
				// this is a mass-refresh. I'd prefer not to reload every single object on the EMR; instead I've gone
				// through and tried to use an ID for all TableChanged messages that I check here. However some situations
				// will require to refresh everything. I'm concerned about performance, so for now we'll just ignore.
				// refresh everything, ugh
			} else {
				// refresh this particular item in all EMNs.
				_RecordsetPtr rsUpdate = NULL;
				for (int i = 0; i < m_EMR.GetEMNCount(); i++) {
					CEMN* pEMN = m_EMR.GetEMN(i);
					if (pEMN) {
						EMNProcedure* pProc = pEMN->GetProcedureByID(lParam);
						if (pProc) {
							if (rsUpdate == NULL) {
								// query for updated info
								rsUpdate = CreateParamRecordset("SELECT Name FROM ProcedureT WHERE ID = {INT}", lParam);
							}

							// update with this info
							if (!rsUpdate->eof) {
								pProc->strName = AdoFldString(rsUpdate, "Name", "");
							} else {
								// the procedure was deleted!
								ASSERT(FALSE);
							}
						}

						UpdatePreviewMoreInfo(pEMN);
					}
				}
			}
			} NxCatchAllThrow("Error refreshing table AptPurposeT");
			break;

		case NetUtils::Providers:
			try {
			if (lParam == -1) {
				// this is a mass-refresh. I'd prefer not to reload every single object on the EMR; instead I've gone
				// through and tried to use an ID for all TableChanged messages that I check here. However some situations
				// will require to refresh everything. I'm concerned about performance, so for now we'll just ignore.
				// refresh everything, ugh
			} else {
				// refresh this particular item in all EMNs.
				_RecordsetPtr rsUpdate = NULL;
				for (int i = 0; i < m_EMR.GetEMNCount(); i++) {
					CEMN* pEMN = m_EMR.GetEMN(i);
					if (pEMN) {
						EMNProvider* pProv = pEMN->GetProviderByID(lParam);
						if (pProv) {
							if (rsUpdate == NULL) {
								// query for updated info
								rsUpdate = CreateParamRecordset("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS ProviderName FROM PersonT WHERE ID = {INT}", lParam);
							}

							// update with this info
							if (!rsUpdate->eof) {
								pProv->strName = AdoFldString(rsUpdate, "ProviderName", "");
								// (a.walling 2007-10-19 12:44) - PLID 23714 - Increase our count
								mapEMNsToUpdateNarratives[pEMN]++;
							} else {
								// the provider was deleted!
								ASSERT(FALSE);
							}
						}
				
						// need to update the secondary providers too
						pProv = pEMN->GetSecondaryProviderByID(lParam);
						if (pProv) {
							if (rsUpdate == NULL) {
								// query for updated info
								rsUpdate = CreateParamRecordset("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS ProviderName FROM PersonT WHERE ID = {INT}", lParam);
							}

							// update with this info
							if (!rsUpdate->eof) {
								pProv->strName = AdoFldString(rsUpdate, "ProviderName", "");
								// (a.walling 2007-10-19 12:44) - PLID 23714 - Increase our count
								mapEMNsToUpdateNarratives[pEMN]++;
							} else {
								// the provider was deleted!
								ASSERT(FALSE);
							}
						}

						UpdatePreviewMoreInfo(pEMN);
					}
				}
			}
			} NxCatchAllThrow("Error refreshing table Providers");
			break;

		case NetUtils::LocationsT:
			try{
			if (lParam == -1) {
				// this is a mass-refresh. I'd prefer not to reload every single object on the EMR; instead I've gone
				// through and tried to use an ID for all TableChanged messages that I check here. However some situations
				// will require to refresh everything. I'm concerned about performance, so for now we'll just ignore.
				// refresh everything, ugh
			} else {
				// refresh this particular item in all EMNs.
				_RecordsetPtr rsUpdate = NULL;
				for (int i = 0; i < m_EMR.GetEMNCount(); i++) {
					CEMN* pEMN = m_EMR.GetEMN(i);
					if (pEMN) {
						// (a.walling 2012-04-09 09:03) - PLID 49515 - Location - Return const ref
						const EMNLocation& emnLoc = pEMN->GetLocation();
						if (emnLoc.nID == lParam) {
							if (rsUpdate == NULL) {
								// query for updated info
								rsUpdate = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", lParam);
							}

							// update with this info
							if (!rsUpdate->eof) {
								pEMN->SetLocation(emnLoc.nID, AdoFldString(rsUpdate, "Name", ""), emnLoc.strLogo, emnLoc.nLogoWidth);
								UpdatePreviewMoreInfo(pEMN);
							} else {
								// the location was deleted!
								ASSERT(FALSE);
							}
						}
					}
				}
			}
			} NxCatchAllThrow("Error refreshing table LocationsT");
			break;

		case NetUtils::Products:
			// (a.walling 2007-10-03 14:08) - PLID 23714 - nothing that changes on a product will affect us.
			break;
		case NetUtils::CPTCodeT:
			// (a.walling 2007-08-06 17:04) - PLID 23714 - Thinking back on it, I'm not going to update any charges/products
			// at all, saved or not. Don't want to risk these changes being catastrophic or unexpected. Leaving the code here
			// in case we revisit this issue.
			// (a.walling 2007-10-03 13:58) - PLID 23714 - We should update at least the code/subcode since that is all that can
			// change. Modifiers will have to be handled in their own branch of logic.
			try {
			if (lParam == -1) {
				// (a.walling 2007-08-06 14:06) - PLID 23714
				// this is a mass-refresh. I'd prefer not to reload every single object on the EMR; instead I've gone
				// through and tried to use an ID for all TableChanged messages that I check here. However some situations
				// will require to refresh everything. I'm concerned about performance, so for now we'll just ignore.
				// refresh everything, ugh
				// However this is the only place where mass-refreshing might make a bit of sense; it could happen if all the 
				// prices were mass updated, which we have features to do. Or if they were mass-inactivated. But considering I 
				// am still iffy about updating the price at all, I think I am going to ignore it for now.
			} else {
				// refresh this particular item in all EMNs.
				_RecordsetPtr rsUpdate = NULL;
				for (int i = 0; i < m_EMR.GetEMNCount(); i++) {
					CEMN* pEMN = m_EMR.GetEMN(i);
					if (pEMN) {
						EMNCharge* pCharge = pEMN->GetChargeByServiceID(lParam);
						if (pCharge) {
							// query for updated info
							rsUpdate = CreateParamRecordset(
								"SELECT Code, SubCode FROM CPTCodeT WHERE ID = {INT}", lParam);

							// update with this info
							if (!rsUpdate->eof) {
								// (a.walling 2007-10-03 14:09) - PLID 23714 - Don't update the name
								//pCharge->strDescription = AdoFldString(rsUpdate, "Name","");
								// (a.walling 2007-08-06 14:06) - PLID 23714 - Uncomment this if you want to update the price
								// pCharge->cyUnitCost = AdoFldCurrency(rsUpdate, "UnitCost",COleCurrency(0,0));
								pCharge->strSubCode = AdoFldString(rsUpdate, "SubCode","");
								pCharge->strCode = AdoFldString(rsUpdate, "Code","");

								// (a.walling 2007-10-03 14:09) - PLID 23714 - MoreInfoDlg will get its own tablechecker after this
								// refresh more info if it is visible
								/* CEMNMoreInfoDlg* pMoreInfo = GetMoreInfoDlg(pEMN, FALSE);
								if (pMoreInfo) {
									pMoreInfo->RefreshChargeList();
								} */
							} else {
								// the code was deleted!
								ASSERT(FALSE);
							}

							UpdatePreviewMoreInfo(pEMN);
						}
					}
				}
			}
			} NxCatchAllThrow("Error refreshing table Products/CPTCodeT");
			break;

		case NetUtils::CPTModifierT:
			// (a.walling 2007-10-03 13:59) - PLID 23714 - Update the modifiers
			// Uh oh, there's not much we can do here. We don't know what modifier changed, nor do we know what it may have changed to!
			// The modifers cannot be renamed if they are saved on a charge somewhere, so we only need to worry about unsaved ones. 
			// Although their description may have changed.
			/*try {
				// Unfortunately, in order for this all to be up to date, we'll have to reload the modifiers from data, and prompt
				// if they have unsaved charges with modifiers that no longer exist.
			} NxCatchAllThrow("Error updating modifiers");*/
			break;

		case NetUtils::AppointmentsT:
			// nothing to do with this really, only used for the PIC and not necessarily the EMN
			break;

		case NetUtils::PatientBDate:
			try {
			if (lParam == -1 || lParam == GetPatientID()) {
				// refresh this particular item in all EMNs.
				_RecordsetPtr rsUpdate = NULL;
				for (int i = 0; i < m_EMR.GetEMNCount(); i++) {
					CEMN* pEMN = m_EMR.GetEMN(i);
					// don't modify saved EMNs
					if (pEMN && (pEMN->GetID() == -1)) {
						COleDateTime dt = pEMN->GetEMNDate();

						if (rsUpdate == NULL) {
							// query for updated info
							rsUpdate = CreateParamRecordset("SELECT BirthDate FROM PersonT WHERE ID = {INT}", lParam);
						}

						// update with this info
						if (!rsUpdate->eof) {
							COleDateTime dtBday = AdoFldDateTime(rsUpdate, "BirthDate");
							// (j.dinatale 2010-10-13) - PLID 38575 - need to call GetPatientAgeOnDate which no longer does any validation, 
							//  validation should only be done when bdays are entered/changed
							// (a.walling 2007-10-19 12:52) - PLID 23714 - Only set as updated if it actually changed
							// (z.manning 2010-01-13 11:15) - PLID 22672 - Age is now a string
							CString strPatientAge = GetPatientAgeOnDate(dtBday, dt, TRUE);
							if (strPatientAge != pEMN->GetPatientAge()) {
								// (a.walling 2007-10-19 12:44) - PLID 23714 - Increase our count
								mapEMNsToUpdateNarratives[pEMN]++;
								pEMN->SetPatientAge(strPatientAge);
							}
						} else {
							// the patient was deleted?! this needs to be handled somehow eventually
							ASSERT(FALSE);
						}

						// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
						CEMNMoreInfoDlg* pMoreInfo = GetMoreInfoDlg(pEMN, FALSE);
						if (pMoreInfo) {
							pMoreInfo->RefreshPatientDemographics();
						}
						
						UpdatePreviewMoreInfo(pEMN);
					}
				}
			}
			} NxCatchAllThrow("Error refreshing table PatientBDate");
			break;

		case NetUtils::PatCombo:
		case NetUtils::PatG1:
			try {
			if (lParam == GetPatientID()) {
				// refresh this particular item in all EMNs.
				//TES 8/13/2014 - PLID 63519 - Moved this code to a separate function, shared with the PatCombo's EX tablechecker
				HandleChangedPatient(mapEMNsToUpdateNarratives);
			}
			} NxCatchAllThrow("Error refreshing table G1");
			break;

		// (a.walling 2008-06-04 12:55) - PLID 22049
		case NetUtils::EMNAccessT:
		case NetUtils::EMNTemplateAccessT:
			{
				HandleEMNAccessTableChecker(lParam);
				break;
			}

		// (a.walling 2008-07-03 13:47) - PLID 30498
		case NetUtils::EMNForceAccessT:
			{
				PostMessage(NXM_EMN_FORCE_ACCESS_REQUEST, lParam, 0);
				break;
			}
		}

		// (a.walling 2007-10-19 12:58) - PLID 23714 - Now go through our map and update narratives
		POSITION pos = mapEMNsToUpdateNarratives.GetStartPosition();
		while (pos) {
			CEMN* pEMN;
			long nCount;
			mapEMNsToUpdateNarratives.GetNextAssoc(pos, pEMN, nCount);
			if (pEMN) {
				// Update!
				//TES 1/23/2008 - PLID 24157 - Renamed.
				pEMN->HandleDetailChange(NULL);
			}
		}
	}
	NxCatchAll("Error in CEMRTreeWnd::OnTableChanged"); 
	return 0;
}

LRESULT CEmrTreeWnd::OnEmrItemChanged(WPARAM wParam, LPARAM lParam)
{
	//OnEmrItemChanged is really just "this item changed",
	//and the tree needs to know it for display purposes

	//the parent topic to the detail should already have been marked unsaved
	//wParam is the CEMRDetail

	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		CEMNDetail *pChangedDetail = (CEMNDetail*)wParam;
		if(pChangedDetail) {
			CEMRTopic *pChangedTopic = pChangedDetail->m_pParentTopic;
			if(pChangedTopic) {
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pChangedTopic, NULL, FALSE);
				if(pRow) {
					if(pRow->Visible) {
						EnsureModifiedState(pRow);
					}
				}
			}

			// (a.walling 2008-10-09 14:46) - PLID 31643 - Update the preview if we are an image detail, which is the only kind
			// that may have different output depending on its size.
			if ( (pChangedDetail->m_EMRInfoType == eitImage) && (GetEmrPreviewCtrl() != NULL) ) {
				// (a.walling 2008-05-14 16:25) - PLID 29114 - Large batches of changes was updating one by one. We can handle this better by pending the detail
				// updates until after a specified period of time. This way, typing into a narrative or drawing on a very large image etc can be much more efficient.

				long nDelay = GetPreviewRefreshDelay(pChangedDetail->m_EMRInfoType);

				if (nDelay > 0) {
					BOOL bDummy;
					if (!m_mapDetailsPendingUpdate.Lookup(pChangedDetail, bDummy)) {
						m_mapDetailsPendingUpdate[pChangedDetail] = TRUE;
						// (a.walling 2009-10-12 16:05) - PLID 36024
						pChangedDetail->__AddRef("CEmrTreeWnd::OnEmrItemChanged pend update");
						//TRACE("**>>      New Detail 0x%08x (%s) pending update at %lu\n", pChangedDetail, pChangedDetail->GetLabelText(), GetTickCount());
					} else {
						//TRACE("**>> Existing Detail 0x%08x (%s) pending update at %lu\n", pChangedDetail, pChangedDetail->GetLabelText(), GetTickCount());
					}
					SetTimer(IDT_PREVIEW_REFRESH, nDelay, NULL);			
				} else {
					UpdateDetailPreview(pChangedDetail);
				}
			}
		}
	} NxCatchAll("Error in OnEmrItemChanged");

	return 0;
}

LRESULT CEmrTreeWnd::OnEmrTreeEnsureVisibleTopic(WPARAM wParam, LPARAM lParam)
{
	try {
		// (c.haag 2007-09-27 12:25) - PLID 27509 - Redone. This function will ensure that a row
		// is visible based on this order of preference:
		//
		// 1. The current tree selection
		// 2. A row suggested to us by the message poster (if it's a topic)
		// 3. Whatever CalcNextRow gives us
		//

		// Variables related to the old list selection
		IRowSettingsPtr pOldSel = m_pTree->CurSel;
		BOOL bIsOldSelValid = TRUE;

		// Variables related to the hint from the message poster
		void *pHint = (void*)wParam;
		IRowSettingsPtr pHintSel = (NULL == pHint) ? NULL : m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pHint, NULL, FALSE);
		BOOL bIsHintValid = FALSE;

		// 1. First, determine if the old selection is valid based on its visiblity and nullness. We do not
		// discriminate against row type here because I want to minimize the possibility of the current 
		// selection and the resultant calculations both being non-topic rows. Otherwise, if we handled a
		// hundred messages in a small period of time, we would be repeating a lot of work with zero net results.
		if (NULL != pOldSel) {

			IRowSettingsPtr p = pOldSel;
			while (NULL != p && bIsOldSelValid) {
				if (VARIANT_FALSE == p->GetVisible()) {
					// If p is invisible, then so must be pOldSel.
					bIsOldSelValid = FALSE;
				}
				p = p->GetParentRow();
			}

		} else {
			// We have no current selection, so it's clearly invalid
			bIsOldSelValid = FALSE;
		}

		// 2. If we cannot use the old selection as a favorable visible topic, then check to see whether
		// the message poster gave us a hint. If so, try to make that the candidate.
		if (!bIsOldSelValid) {

			EmrTreeRowType etrtHint = (NULL == pHintSel) ? etrtInvalid : EmrTree::ChildRow(pHintSel).GetType();
			// We only care about hints that are topic rows
			if (etrtTopic == etrtHint) {
				IRowSettingsPtr p = pHintSel;
				for (bIsHintValid = TRUE; NULL != p && bIsHintValid; p = p->GetParentRow()) {
					if (VARIANT_FALSE == p->GetVisible()) {
						// If p is invisible, then so must be pHintSel
						bIsHintValid = FALSE;
					}
				}
			}
		}

		// Now assign the new selection
		IRowSettingsPtr pNewSel;
		if (bIsOldSelValid) {
			pNewSel = pOldSel;
		} else if (bIsHintValid) {
			pNewSel = pHintSel;			
		} else {
			// 3. If the hint and the old selection are invalid, then we are forced to use CalcNextRow
			// to acquire a new selection. We could potentially pass in either pOldSel or pHintSel as a
			// starting candidate. I think the general rule was to make the new selection come right
			// after the old selection, if possible. So, we will pass in pOldSel if it's not null;
			// otherwise, we will pass in pHintSel because that is at least in the neighborhood of what
			// the message poster wants.
			//
			pNewSel = CalcNextRow((NULL != pOldSel) ? pOldSel : pHintSel);
		}

		// If the old and new selections differ, set the new selection
		if (pOldSel != pNewSel) {
			SetTreeSel(pNewSel);
		}

		/*
		// (c.haag 2006-03-14 09:27) - PLID 19493 - If we get here, it means the
		// current selection was marked invisible. That can happen if a topic unspawns
		// itself. This function will cause a substitute tree item to be selected.
		IRowSettingsPtr pOldSel = m_pTree->CurSel;
		IRowSettingsPtr pNewSel = NULL;

		// If we don't have a current selection, select the top row
		if (NULL == pOldSel) {
			pNewSel = m_pTree->GetFirstRow();
		}
		// See if the current selection is visible. If it is, there's no reason to
		// do anything.
		else {
			if (VARIANT_TRUE == pOldSel->GetVisible())
				return 0;

			// Act based on the type of row selected
			EmrTreeRowType etrt = EmrTree::ChildRow(pOldSel).GetType();
			if (etrtTopic == etrt) {
				// This is a topic, so try to select the first visible parent. OnSelChangingTree and HandleSelChanged
				// will do the rest.
				IRowSettingsPtr p = pOldSel;
				do {
					p = p->GetParentRow();
				} while (NULL != p && (!p->GetVisible()));
				pNewSel = p;
			}
		}

		// If we fail to make a valid selection, just pick the top row.
		if (NULL == pNewSel) {
			pNewSel = m_pTree->GetFirstRow();
		}

		// Now make our new selection
		if (pNewSel != pOldSel) {
			LPDISPATCH lpdispNewSel = (LPDISPATCH)pNewSel;
			// (b.cardillo 2006-11-17 14:15) - PLID 23265 - Since we just grabbed a reference to 
			// the row, we have to add ref.  Notice, if the OnSelChangingTree() function changes 
			// our lpdispNewSel, it will release it first.
			if (lpdispNewSel) {
				lpdispNewSel->AddRef();
			}
			// Call OnSelChangingTree because it will cause our pick to be discriminated just
			// like the user's pick, which is what we want. If we select an EMN, it will result
			// in the first visible topic being selected
			OnSelChangingTree(pOldSel, &lpdispNewSel);
			pNewSel = lpdispNewSel;
			// (b.cardillo 2006-11-17 14:15) - PLID 23265 - Now that we're done with lpdispNewSel, 
			// which is a fully fledged reference to a row, we need to release it.
			if (lpdispNewSel) {
				lpdispNewSel->Release();
				lpdispNewSel = NULL;
			}
			SetTreeSel(pNewSel);
		}*/

	} NxCatchAll("Error in CEmrTreeWnd::OnEmrTreeEnsureVisibleTopic");
	return 0;
}

NXDATALIST2Lib::IRowSettingsPtr CEmrTreeWnd::GetRowFromTopic(NXDATALIST2Lib::IRowSettingsPtr pRow, CEMRTopic *pTopicToFind)
{
	// (c.haag 2008-04-07 11:16) - PLID 29540 - Returns the EmrTreeWnd datalist 2 row that corresponds to pTopicToFind.
	// This is a recursive search, so the caller will need to pass in a row that is the topic's parent/grandparent/etc.

	// Do for all rows at this level
	while (NULL != pRow) {

		// See if pRow is the row we're looking for
		EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
		if (etrtTopic == etrt) {
			CEMRTopic* pRowTopic = EmrTree::ChildRow(pRow).GetTopic();
			if (pRowTopic == pTopicToFind) {
				// Jackpot
				return pRow;
			}
		}

		// See if pRow is the parent/grandparent of the row we're looking for
		if (NULL != pRow->GetFirstChildRow()) {
			NXDATALIST2Lib::IRowSettingsPtr p = GetRowFromTopic(pRow->GetFirstChildRow(), pTopicToFind);
			if (NULL != p) {
				// Jackpot
				return p;
			}
		}

		// The row is unrelated to the topic. Move on to the next one.
		pRow = pRow->GetNextRow();
	}
	return NULL;
}

LRESULT CEmrTreeWnd::OnTopicLoadComplete(WPARAM wParam, LPARAM lParam)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		long nTopicID = (long)wParam;
		BOOL bIsTemplateTopicID = (BOOL)lParam;
		// (c.haag 2008-04-07 11:03) - PLID 29540 - This loop is useless because the second loop effectively
		// does the same thing but with a little more work involved. I think it was made useless when PLID 22697
		// was done.
		//Find the topic with the given ID.
		//TES 5/26/2006 - We'll loop through twice, the first time only checking the top-level EMN topics.  The reason for this is
		//that calling GetTopicByID() will force topics to load their subtopics, and we want to avoid that if possible.
		/*CEMRTopic *pTopic = NULL;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTree->GetFirstRow();
		while(pRow != NULL) {
			CEMN *pEMN = EmrTree::ChildRow(pRow).GetEMN();
			for(int i = 0; i < pEMN->GetTopicCount() && !pTopic; i++) {
				if(pEMN->GetTopic(i)->GetID() == nTopicID ||
					//We tell the accessor functions not to ensure these variables; obviously we can't since we're trying to call
					//PostLoad() on the function (which is what ensuring them waits on), and anyway, if the variable wasn't synchronously
					//then it can't matter to us, since we're comparing to a variable that was used to initialize the loading.
					(bIsTemplateTopicID && 
					(pEMN->GetTopic(i)->GetTemplateTopicID(FALSE) == nTopicID || pEMN->GetTopic(i)->GetOriginalTemplateTopicID(FALSE) == nTopicID)) ) {
					pTopic = pEMN->GetTopic(i);
					pTopic->PostLoad();
					EnsureTopicBackColor(pTopic);
					CheckDuplicateTopics(pRow);
					// (z.manning, 09/26/2006) - PLID 22697 - We can't return here because we may have more than 1
					// EMN loaded from the same template, and we need to go through all of them.
					//return 0;
				}
			}
			pRow = pRow->GetNextRow();
		}
		//If we got here, it must be a subtopic, so we'll have to load subtopics.
		*/

		
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTree->GetFirstRow();
		while(pRow != NULL) {
			CEMN *pEMN = EmrTree::ChildRow(pRow).GetEMN();
			CEMRTopic *pTopic = bIsTemplateTopicID ? pEMN->GetTopicByTemplateTopicID(nTopicID, -1, NULL) : pEMN->GetTopicByID(nTopicID);
			if(pTopic) {
				pTopic->PostLoad();
				//Now that this topic is fully loaded, make sure it is displayed correctly.
				EnsureTopicBackColor(pTopic);

				// (c.haag 2008-04-07 11:28) - PLID 29540 - We only need to call CheckDuplicateTopics() with
				// the parent of the row that we know the topic exists in. There's no reason to constantly check
				// the top level topics every time.
				//CheckDuplicateTopics(pRow);

				// (c.haag 2008-04-07 11:04) - PLID 29540 - We used to call CheckDuplicateTopics(pRow),
				// meaning we only checked top-level topics for duplicate names. However, that didn't work
				// when the topic in question was the child of another topic. Now we call CheckDuplicateTopics
				// with the correct parent.
				NXDATALIST2Lib::IRowSettingsPtr pTopicRow = GetRowFromTopic(pRow->GetFirstChildRow(), pTopic);
				if (NULL != pTopicRow && NULL != pTopicRow->GetParentRow()) {
					CheckDuplicateTopics(pTopicRow->GetParentRow());						
				}

				// (z.manning, 09/26/2006) - PLID 22697 - We can't return here because we may have more than 1
				// EMN loaded from the same template, and we need to go through all of them.
				//return 0;
			}
			pRow = pRow->GetNextRow();
		}
	} NxCatchAll("Error in OnTopicLoadComplete");
	
	return 0;
}

LRESULT CEmrTreeWnd::OnTopicLoadCompleteByPreloader(WPARAM wParam, LPARAM lParam)
{
	//
	// (c.haag 2007-05-08 16:18) - PLID 25941 - This is just like OnTopicLoadCompleted,
	// but posted from the EMN loader object. This does the same thing that OnTopicLoadCompleted
	// was designed for; but it doesn't need to search for the topic object, and it doesn't
	// do unnecessary iterations or immediate topic loads.
	//
	try {
		CEMRTopic* pTopic = (CEMRTopic*)wParam;
		BOOL bIsTemplateTopicID = (BOOL)lParam;
		pTopic->PostLoad();
		EnsureTopicBackColor(pTopic);

		// Check for duplicate topics
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pTopic, NULL, FALSE);
		if (NULL != pRow) {
			CheckDuplicateTopics(pRow);

			// (c.haag 2007-09-18 10:55) - In CEmrTreeWnd::AddTopicRow, we used to call CheckDuplicateTopics on
			// this row's parent. We have transferred that responsibility to this function.
			IRowSettingsPtr pParentRow = pRow->GetParentRow();
			if (NULL != pParentRow) { CheckDuplicateTopics(pParentRow); }
		}

	} NxCatchAll("Error in CEmrTreeWnd::OnTopicLoadCompleteByPreloader");
	return 0;
}

LRESULT CEmrTreeWnd::OnTopicModifiedChanged(WPARAM wParam, LPARAM lParam)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		EnsureTopicModifiedState((CEMRTopic*)wParam);
		//
		// (c.haag 2006-07-24 10:44) - PLID 21535 - We now make sure the back
		// color properly indicates the topic's completion status.
		//
		// This change is done specifically for this punch list item, and no
		// other. At the same time, it really seems like it should be here 
		// since the call to EnsureTopicModifiedState updates the icon anyway.
		// My addition makes this a universal EnsureTopicTreeAppearanceLooksGood
		// kind of function.
		//
		EnsureTopicBackColor((CEMRTopic*)wParam);
	} NxCatchAll("Error in OnTopicModifiedChanged");

	return 0;
}

void CEmrTreeWnd::OnCreateToDo()
{
	try {

		/* Create a new todo from the task edit dialog */

		// (j.jones 2008-11-14 10:38) - PLID 31208 - we decided you should not be required to have patient permissions
		// to create a todo alarm
		/*
		if (!CheckCurrentUserPermissions(bioPatient, sptWrite))
			return;
		*/
	
		CTaskEditDlg dlg(this);
		// (c.haag 2008-07-11 10:13) - PLID 30550 - Flag the fact we're opening this from an EMN
		CEMN* pEMN = GetActiveEMN();
		// (c.haag 2008-08-25 09:46) - Check the IsWriteable flag. If it's TRUE, then let the user be able to
		// link the alarm. If FALSE, then don't even give the user the ability to link it (we'll say it wasn't
		// invoked from the EMN)
		if (NULL != pEMN && pEMN->IsWritable()) {
			dlg.m_bInvokedFromEMN = TRUE;
			dlg.m_nEMNID = pEMN->GetID(); // Can be -1
			dlg.m_strEMNDescription = pEMN->GetDescription();
		}
		dlg.m_nPersonID = GetPatientID(); // (a.walling 2008-07-07 17:52) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
		//(c.copits 2010-12-06) PLID 40794 - Permissions for individual todo alarm fields
		dlg.m_bIsNew = TRUE;

		long nResult = dlg.DoModal();

		if (nResult != IDOK)
			return;

		// (c.haag 2008-07-14 13:42) - PLID 30696 - If the following is true, it means
		// the todo was made for this EMN. The todo will have a Regarding ID of -1. We 
		// need to store the todo in memory so that when the EMN is saved, then the todo's
		// Regarding ID will change to that of the EMN. The reason we don't assign the EMN
		// ID as the regarding ID right away is so users won't be able to delete the todo
		// until the EMN is saved.
		CArray<long, long> anAssignToIDs;
		if (NULL != pEMN && dlg.m_bWasLinkedToEMNWhenSaved) {

			// (c.haag 2008-07-14 11:17) - PLID 30696 - Track this todo so that if the user cancels
			// out of the EMN, then we can undo the creation of the todo
			_RecordsetPtr prs = CreateParamRecordset("SELECT Remind, Done, EnteredBy, PersonID, Priority, Task, LocationID, CategoryID, dbo.GetTodoAssignToIDString(TodoList.TaskID) AS AssignToIDs, dbo.GetTodoAssignToNamesString(TodoList.TaskID) AS AssignToNames, Deadline, (SELECT Description FROM NoteCatsF WHERE ID = TodoList.CategoryID) AS Category, Notes FROM TodoList WHERE TodoList.TaskID = {INT}", dlg.m_iTaskID);
			if (!prs->eof) {
				FieldsPtr f = prs->Fields;
				
				CString strAssignToIDs = AdoFldString(f, "AssignToIDs");
				ParseDelimitedStringToLongArray(strAssignToIDs, " ", anAssignToIDs);
				pEMN->AddCreatedTodoWhileUnsaved(dlg.m_iTaskID, 
					// (c.haag 2012-10-17) - PLID 52863 - No need to pass null or empty values for spawning anymore.
					// This uses the no-spawning overload of AddCreatedTodoWhileUnsaved.
					f->Item["Remind"]->Value,
					f->Item["Done"]->Value,
					f->Item["Deadline"]->Value,
					f->Item["EnteredBy"]->Value,
					f->Item["PersonID"]->Value,
					f->Item["Notes"]->Value,
					f->Item["Priority"]->Value,
					f->Item["Task"]->Value,
					f->Item["LocationID"]->Value,
					f->Item["CategoryID"]->Value,
					(long)ttEMN,
					anAssignToIDs,
					f->Item["AssignToNames"]->Value
					);	

				// (c.haag 2008-07-07 13:36) - PLID 30550 - Update the todo list in the more info
				// window if it exists
				PostMessage(NXM_EMN_TODO_ADDED, (WPARAM)pEMN, (LPARAM)dlg.m_iTaskID);

			} else {
				// This should never happen
				ASSERT(FALSE);
			}
		}
		/*// (s.tullis 2014-08-21 10:09) - 63344 -Changed to Ex Todo -- removed Redundant tablechecker send
		if (anAssignToIDs.GetSize()==1){
			CClient::RefreshTodoTable(dlg.m_iTaskID,dlg.m_nPersonID, anAssignToIDs[0],TableCheckerDetailIndex::tddisAdded);
		}
		else
		{
			CClient::RefreshTodoTable(dlg.m_iTaskID, dlg.m_nPersonID, -1, TableCheckerDetailIndex::tddisAdded);
		}*/
		

		// (j.jones 2008-11-06 17:30) - PLID 31947 - mainframe will update the ToDo alarm, we should not have to
		/*
		CMainFrame *pMain = GetMainFrame();
		if(pMain) {
			if(pMain->m_dlgToDoAlarm.GetSafeHwnd()) {
				pMain->m_dlgToDoAlarm.OnTableChanged(NetUtils::TodoList, dlg.m_iTaskID);
			}
		}
		*/

	} NxCatchAll("Error in OnCreateToDo");
}

void CEmrTreeWnd::OnRecordAudio()
{
	//
	// (c.haag 2006-09-22 17:43) - PLID 21327 - We can now record audio files and attach them
	// to a patient's history
	//
	//CAudioRecordDlg dlg;
	try {
		//DRT 8/21/2007 - PLID 27133 - Moved this inside the try/catch
		
		// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
		if (GetPicContainer() && GetPicContainer()->GetCurrentPicID() > 0 && m_EMR.GetPatientID() > 0) {

			// Fail if there are no recording devices on the computer
			if (0 == waveInGetNumDevs()) {
				MessageBox("NexTech Practice could not detect any recording devices on your computer. Please contact your hardware administrator for assistance.");
				return;
			}
			
			// (a.walling 2010-07-27 16:37) - PLID 39433 - Send current EMN pointer
			// (a.walling 2010-04-13 14:18) - PLID 36821 - Modeless dialog is all handled through here now.
			CAudioRecordDlg::DoAudioRecord(this, m_EMR.GetPatientID(), true, GetPicContainer()->GetCurrentPicID(), GetCurrentEMN());

		} else {
			ThrowNxException("Could not specify a valid patient ID and procedure container ID to the recording window");
		}
	}
	NxCatchAll("Error in CEmrTreeWnd::OnRecordAudio()");
}

// (j.jones 2008-07-09 08:57) - PLID 24624 - added patient summary
void CEmrTreeWnd::OnPatientSummary()
{
	try {

		CPatientSummaryDlg dlg(this);
		dlg.m_nPatientID = m_EMR.GetPatientID();
		dlg.m_strPatientName = GetExistingPatientName(dlg.m_nPatientID);
		dlg.DoModal();

	} NxCatchAll("Error in CEmrTreeWnd::OnPatientSummary");
}


void CEmrTreeWnd::OnEMRDebug()
{
#ifdef _DEBUG
	// (c.haag 2007-08-04 10:10) - PLID 26946 - This is called when a developer presses the Debug
	// button in an EMR or template
	try {
		IRowSettingsPtr pRow = m_pTree->CurSel;
		if (NULL != pRow) {
			IRowSettingsPtr pParentRow = pRow->GetParentRow();
			CEMN* pActiveEMN = NULL;
			CEMRTopic* pTopic;

			EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
			switch (etrt) {
				case etrtEmn:
					pActiveEMN = EmrTree::ChildRow(pRow).GetEMN();
					break;
				case etrtTopic:
					pTopic = EmrTree::ChildRow(pRow).GetTopic();
					pActiveEMN = pTopic->GetParentEMN();
					break;
				case etrtMoreInfo:
				case etrtCodes: //TES 2/12/2014 - PLID 60748
					etrt = EmrTree::ChildRow(pParentRow).GetType();
					if (etrtEmn == etrt) {
						pActiveEMN = EmrTree::ChildRow(pParentRow).GetEMN();
					}
					break;
				default:
					break;
			}

			if (NULL != pActiveEMN) {
				CEmrDebugDlg dlg(this);
				dlg.SetEMR(&m_EMR);
				dlg.SetActiveEMN(pActiveEMN);
				dlg.DoModal();
			} else {
				MessageBox("Could not find the active EMN given the selection in the EMR tree!");
			}
		} else {
			MessageBox("Please select a row in the EMR tree");
		}
	}
	NxCatchAll("Error in CEmrTreeWnd::OnEMRDebug!");
#endif
}

void CEmrTreeWnd::OnEMChecklistSetup()
{
	try {
		// (c.haag 2007-08-30 16:32) - PLID 27058 - Display the modeless E/M Checklist Setup dialog
		if(!CheckCurrentUserPermissions(bioPatientEMChecklist, sptWrite))
			return;

		if (!m_dlgEMChecklistSetup.GetSafeHwnd()) {
			m_dlgEMChecklistSetup.SetModeless(TRUE);
			// (c.haag 2007-09-13 09:05) - PLID 27058 - Assign CEmrTreeWnd as the parent for
			// the checklist setup. That way, it will behave like the PracYakker in that it
			// will not hide if you click on the EMR template editor; and when you minimize it,
			// it "stacks" on the bottom-left corner of the screen.
			m_dlgEMChecklistSetup.Create(CEMREMChecklistSetupDlg::IDD, this);
			m_dlgEMChecklistSetup.CenterWindow();
		}
		else if (!m_dlgEMChecklistSetup.IsWindowVisible()) {
			m_dlgEMChecklistSetup.CenterWindow();
		}
		BringEMREMChecklistDlgToTop();

	} NxCatchAll("Error in CEmrTreeWnd::OnEMChecklistSetup");
}

CString CEmrTreeWnd::GetEmnRowDisplayText(CEMN* pEmn)
{
	// (z.manning, 09/20/2006) - PLID 22410 - We now show the EMNs date in the EMN tree rows.
	return FormatString("[%s]\r\n%s", FormatDateTimeForInterface(pEmn->GetEMNDate(), NULL, dtoDate), pEmn->GetDescription());
}

void CEmrTreeWnd::OnCurSelWasSetTree()
{
	// (c.haag 2007-09-26 17:16) - PLID 27509 - This code is now obselete. However, this makes for
	// quite a handy debugging event for future development, so we'll keep the event.
	/*
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		if(m_pTree->CurSel != m_pCurrentlyDisplayedRow) {
			//TES 12/4/2006 - PLID 21425 - OK, we need to make sure that we update our display to reflect the currently selected row.  
			// However, we can't do it immediately, because a.) maybe the datalist is about to post a "normal" event about this that 
			// we will handle properly, and more importantly b.) because anything in this function that caused the selection in the 
			// datalist to change, or even THINK about changing, could lead to infinite recursion.  So, we'll just set a timer for 
			// ourselves to do it.

			//Also, we need to remember that we're expecting the selection to change, then if it changes in an "unexpected" way (that
			// is, with the wrong oldsel, we will ignore it so that the proper topic gets hidden.
			m_bExpectingSelChange = TRUE;
			SetTimer(IDT_CHECK_SEL_CHANGE, 0, NULL);
		}
	} NxCatchAll("Error in OnCurSelWasSetTree");*/

	//Make sure our row is visible.
	try {
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pTree->CurSel;
		if(pNewRow) {
			IRowSettingsPtr pParent = pNewRow->GetParentRow();
			while(pParent) {
				pParent->Expanded = TRUE;
				pParent = pParent->GetParentRow();
			}

			m_pTree->EnsureRowInView(pNewRow);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-06-28 17:11) - PLID 51276 - More Info should be a clickable link
void CEmrTreeWnd::OnLeftClickTree(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		if (!lpRow) return;

		switch (nCol) {
			case TREE_COLUMN_ICON:
			case TREE_COLUMN_NAME:
				break;
			default:
				return;
		}

		EmrTree::ChildRow pNew(lpRow);

		//Determine what type of row we're selecting.

		// (a.walling 2012-07-03 10:56) - PLID 51284 - Fixing activation ambiguities - Just tell the framewnd to activate the object
		switch (pNew.GetType()) {
			case etrtTopic:
				GetEmrFrameWnd()->ActivateTopic(pNew.GetTopic());
				break;
			case etrtMoreInfo:
				GetEmrFrameWnd()->ShowMoreInfo(pNew.GetEMN());
				break;
			case etrtEmn:
				GetEmrFrameWnd()->ActivateEMNOnly(pNew.GetEMN());
				break;
			//TES 2/12/2014 - PLID 60748 - New Codes topic
			case etrtCodes:
				GetEmrFrameWnd()->ShowCodesView(pNew.GetEMN());
				break;
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2007-01-30 11:21) - PLID 24353 - we commit times when the dialog is closed
void CEmrTreeWnd::StopTrackingTimes()
{
	try {

		// (j.jones 2007-02-06 14:30) - PLID 24509 - stop tracking the EMR time
		m_EMR.TryStopTrackingTime();
	
		//for each EMN, stop tracking time
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTree->GetFirstRow();
		while(pRow != NULL) {

			EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
			if(etrt == etrtEmn) {
				//it is an EMN, stop tracking time
				CEMN *pEMN = EmrTree::ChildRow(pRow).GetEMN();
				pEMN->TryStopTrackingTime();
			}

			pRow = pRow->GetNextRow();
		}
	
	}NxCatchAll("Error in CEmrTreeWnd::StopTrackingEMNTimes");
}

// (j.jones 2007-02-06 15:06) - PLID 24493 - get the EMR start time
COleDateTime CEmrTreeWnd::GetEMRStartEditingTime()
{
	try {

		return m_EMR.GetStartEditingTime();
	
	}NxCatchAll("Error in CEmrTreeWnd::GetEMRStartEditingTime");

	COleDateTime dtBad;
	dtBad.SetStatus(COleDateTime::invalid);
	return dtBad;
}

void CEmrTreeWnd::TryStartTrackingEMRTime()
{
	try {

		m_EMR.TryStartTrackingTime();
	
	}NxCatchAll("Error in CEmrTreeWnd::TryStartTrackingEMRTime");
}

// (j.jones 2007-09-17 17:35) - PLID 27396 - send the server offset to the EMR
void CEmrTreeWnd::UpdateServerTimeOffset(COleDateTimeSpan dtOffset)
{
	m_EMR.UpdateServerTimeOffset(dtOffset);
}

// (a.walling 2007-04-05 16:53) - PLID 25548
LRESULT CEmrTreeWnd::OnLoadEmnPreview(WPARAM wParam, LPARAM lParam)
{
	// (a.walling 2007-04-11 13:38) - added handling for multiple EMNs!
	try {
		if (GetEmrPreviewCtrl()) {
			CEMN* pEMN = (CEMN*)wParam;

			if (!GetEmrPreviewCtrl()->IsInitialized()) {
				// this is the initial EMN, so set it.
				GetEmrPreviewCtrl()->SetEMN(pEMN, TRUE);
			} else {
				GetEmrPreviewCtrl()->SetEMN(pEMN, FALSE); // just add the info to the map, do not set as active
			}

			// (a.walling 2012-03-12 12:43) - PLID 48712 - Pass an LPCSTR, not a CString*
			CString strFullPath = (LPCTSTR)lParam;
			BOOL bResult = TRUE;

			if (strFullPath.IsEmpty()) {
				ASSERT(FALSE);
				return EMR_PREVIEW_FAILED;
			}

			// (a.walling 2012-03-12 12:43) - PLID 48712 - We should always have a file now
			// we were sent a full path to a file, so load that.
			if (!GetEmrPreviewCtrl()->SetSource(strFullPath, pEMN)) {
				ASSERT(FALSE);
				return EMR_PREVIEW_FAILED;
			}

			return EMR_PREVIEW_LOADED;
		}
	} NxCatchAll("Error in OnLoadEmnPreview");

	return EMR_PREVIEW_FAILED;
}

// (a.walling 2012-03-12 12:43) - PLID 48712 - No longer necessary since we are always regenerating anyway
// (a.walling 2007-04-12 17:56) - PLID 25605 - Put the file and any dependencies in the temp path.
//CString CEmrTreeWnd::PrepareSavedDocument(long nEmnID)
//{
//	CString strFile;
//	strFile.Format("EMN_%li.mht", nEmnID);
//
//	// (a.walling 2007-06-11 13:07) - PLID 26278 - Keep EMN Preview files in their own directory
//	CString strDocPath = GetSharedPath() ^ "EMNPreview\\";
//	//CString strDocPath = GetPatientDocumentPath(GetPatientID()) ^ "EMNPreview\\";
//	// (a.walling 2007-07-19 09:38) - PLID 26261 - Use the NxTemp path
//	CString strTempPath = GetNxTempPath();
//
//	if (!FileUtils::DoesFileOrStreamExist(strDocPath ^ strFile)) {
//		return "";
//	}
//
//	// there are situations where something may not be setup correctly that can cause creation of encrypted files to fail.
//	HANDLE hFile = CreateFile(strTempPath ^ strFile, GENERIC_WRITE, 0,  NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_TEMPORARY, NULL);
//
//	if (hFile == INVALID_HANDLE_VALUE) {
//		ASSERT(FALSE);
//		return "";
//	} else {
//		if (NxCDO::DecryptMHTFromFile(strDocPath ^ strFile, hFile)) {
//			// we are good
//			CloseHandle(hFile);
//
//			// (a.walling 2007-06-12 17:16) - PLID 26261 - For super-duper-extra-safety, go ahead and delay deletion
//			// of this file at reboot. This will help cover us for abnormal program terminations and poweroutages.
//			// Can't just have these sensitive files laying around in the temp drive. I created thousands of temp files
//			// and delayed move on all of them on Vista and 2000 (virtual machine) and noticed no delay or abnormality next
//			// startup. Harmless too if the file is successfully deleted beforehand.
//			MoveFileEx(strTempPath ^ strFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
//
//			return strTempPath ^ strFile;
//		} else {
//			CloseHandle(hFile);
//			ASSERT(FALSE);
//			return "";
//		}
//	}
//
//	// (a.walling 2007-06-11 12:37) - PLID 26263 - We don't need to move anything now, it's all compiled
//	// in the MHT.
//	/*
//	if (!CopyFile(strDocPath ^ strFile, strTempPath ^ strFile, FALSE)) {
//		// failed copying!
//		return strDocPath ^ strFile;
//	} else {
//		// now copy over any images or dependencies
//		CString strSearchString;
//		strSearchString.Format("EMN_%li_*.*", nEmnID);
//
//		long nFailed = 0;
//		CFileFind ff;
//		BOOL bContinue = ff.FindFile(strDocPath ^ strSearchString);
//		while (bContinue) {
//			bContinue = ff.FindNextFile(); // we will always get at least the ./.. entries.
//			if (!ff.IsDots() && !ff.IsDirectory()) {
//				if (!CopyFile(strDocPath ^ ff.GetFileName(), strTempPath ^ ff.GetFileName(), FALSE))
//					nFailed++;
//			}
//		}
//		ASSERT(nFailed == 0);
//
//		return strTempPath ^ strFile;
//	}
//	*/
//
//	ASSERT(FALSE);
//	return "";
//}

// (a.walling 2007-04-10 16:53) - PLID 25548 - Scroll the preview pane to this topic
void CEmrTreeWnd::OnGotoPreview()
{
	try {
		IRowSettingsPtr pRow = m_pTree->CurSel;

		if (pRow) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();

			if (etrt == etrtTopic) {
				CEMRTopic* pTopic = EmrTree::ChildRow(pRow).GetTopic();

				if (GetEmrPreviewCtrl()) {
					GetEmrPreviewCtrl()->ScrollToTopic(pTopic);
				}
			} else if (etrt == etrtMoreInfo) {
				// (a.walling 2007-07-12 16:44) - PLID 26640 - Scroll to the more info anchor
				if (GetEmrPreviewCtrl()) {
					GetEmrPreviewCtrl()->ScrollToMoreInfo();
				}
			}
		}
	} NxCatchAll("Error in OnGotoPreview");
}

// (j.armen 2012-07-02 11:44) - PLID 49831 - Check to see if the EMR has any editable EMNs
BOOL CEmrTreeWnd::HasEditableEMN()
{
	for(int i = 0; i < GetEMNCount(); i++)
	{
		CEMN* pEMN = GetEMN(i);
		if(pEMN->CanBeEdited())
			return TRUE;
	}
	return FALSE;
}

// (a.walling 2012-07-12 09:36) - PLID 46078 - Retrieve active topic
CEMRTopic* CEmrTreeWnd::GetActiveTopic()
{
	EmrTree::ChildRow childRow(m_pTree->GetCurSel());

	if (childRow && childRow.IsTopic()) {
		return childRow.GetTopic();
	}

	return NULL;
}

// (c.haag 2008-07-11 10:14) - PLID 30550 - Returns the active EMN
CEMN* CEmrTreeWnd::GetActiveEMN()
{
	CEMN* pEMN = NULL;

	// (a.walling 2014-05-12 17:07) - PLID 61778 - Check m_pTree for NULL (creation failure / destroying / message box or modal loop during creation)
	EmrTree::ChildRow childRow(m_pTree ? m_pTree->GetCurSel() : NULL);

	while (childRow) {
		// when pParentRow is NULL, then we know we have the top-level EMN row in pRow.
		if(childRow.GetType() == etrtEmn) {
			pEMN = childRow.GetEMN();
		}

		childRow = childRow.GetParent();
	}

	return pEMN;
}

// (a.walling 2007-04-11 12:23) - PLID 25548
void CEmrTreeWnd::SetActiveEMN(long nEmnID)
{
	CEMN* pEMN = m_EMR.GetEMNByID(nEmnID);

	// (z.manning 2011-05-20 17:08) - PLID 33114 - Make sure we got a valid EMN
	if (pEMN != NULL && GetEmrPreviewCtrl()) {
		GetEmrPreviewCtrl()->SetEMN(pEMN);
	}
}

// (a.walling 2007-06-18 11:52) - PLID 25549
void CEmrTreeWnd::UpdateDetailPreview(CEMNDetail* pDetail)
{
	// (c.haag 2007-10-24 11:42) - PLID 27562 - Have additional error checking and
	// logging for pinning down problems rooted in one of many possible causes
	try {
		if (GetEmrPreviewCtrl()) {
			GetEmrPreviewCtrl()->UpdateDetail(pDetail);
		}
	} NxCatchAllSilentCallThrow(
		ASSERT(FALSE);
		Log("An exception was thrown from CEmrTreeWnd::UpdateDetailPreview. The exception will quietly be passed to the calling function.");
	);
}

// (a.walling 2007-06-19 17:07) - PLID 25548 - Preference to automatically scroll along with the EMN
void CEmrTreeWnd::OnAutoScroll()
{
	try {
		// (a.walling 2007-06-19 17:07) - PLID 25548
		if (m_bAutoScroll)
			m_bAutoScroll = FALSE;
		else m_bAutoScroll = TRUE;

		SetRemotePropertyInt("EMNPreviewAutoScroll", m_bAutoScroll, 0, GetCurrentUserName());
		
		if (m_bAutoScroll) {
			// they just turned it on, so go ahead and scroll to the current topic
			OnGotoPreview();
		}
	} NxCatchAll("Error in OnAutoScroll");
}

// (a.walling 2007-07-11 09:29) - PLID 26545 - Ensure the EMN's charts and categories are valid, return a message
CString CEmrTreeWnd::CheckValidChartsCategories(CEMN* pEMN)
{
	try {
		CString strMessage;

		if (m_checkCategories.Changed() || m_checkCharts.Changed() || m_checkCategoryChartLink.Changed() ) {
			if (pEMN) {
				long nCatID = pEMN->GetCategory();
				long nChartID = pEMN->GetChart();

				BOOL bChartOK, bCatOK;

				bCatOK = bChartOK = TRUE;

				if (nChartID != -1) {
					// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
					bChartOK = (ReturnsRecordsParam("SELECT ID FROM EmnTabChartsT WHERE ID = {INT}", nChartID));
				}

				if (nCatID != -1) {
					// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
					// (z.manning 2013-06-05 17:08) - PLID 56962 - This had been using the chart ID instead of the category ID
					bCatOK = (ReturnsRecordsParam("SELECT ID FROM EmnTabCategoriesT WHERE ID = {INT}", nCatID));
				}

				if (!bChartOK || !bCatOK) {
					strMessage = "The chart and category setup has been changed outside of this EMN (possibly by another user) and this EMN's chart/category combination is now invalid. There is no longer a ";

					if (!bChartOK) {
						pEMN->SetChart(-1, "");
						strMessage += "chart";
					}

					if (!bCatOK && ! bChartOK) {
						strMessage += "or a ";
					}

					if (!bCatOK) {
						pEMN->SetCategory(-1, "");
						strMessage += "category";
					}
					
					strMessage += " selected on this EMN.";

					// (a.walling 2012-03-22 16:50) - PLID 49141 - interface notified via the Set* methods above
				}
			}
		}

		// need to manually set these as checked due to OR logic short circuiting
		m_checkCategories.m_changed = false;
		m_checkCharts.m_changed = false;
		m_checkCategoryChartLink.m_changed = false;

		return strMessage;
	} NxCatchAll("Error in CEmrTreeWnd::CheckValidChartsCategories");

	return "";
}

// (a.walling 2007-07-13 11:21) - PLID 26640 - Update the more info section of the preview
void CEmrTreeWnd::UpdatePreviewMoreInfo(CEMN* pEMN)
{
	try {
		if (GetEmrPreviewCtrl()) {
			// (a.walling 2007-10-12 08:47) - PLID 25548 - Ensure this is the correct EMN
			if (pEMN == GetEmrPreviewCtrl()->GetCurrentEMN()) {
				GetEmrPreviewCtrl()->UpdateMoreInfo(pEMN);

				// (a.walling 2007-10-15 15:44) - PLID 27664 - we used to update the EMN here,
				// which is probably not the best place. This will be revisited and improved under this plid.
			}
		}
	} NxCatchAll("Error updating preview (more info section)");
}

BOOL CEmrTreeWnd::UpdateEMNMoreInfoDlgChangedInfo(CEMN* pEMN)
{
	// (c.haag 2007-10-24 11:44) - PLID 27562 - Have additional error checking and
	// logging for pinning down problems rooted in one of many possible causes
	try {
		//
		// (c.haag 2007-09-28 11:11) - PLID 27509 - This function was written to provide functionality
		// to CEMN::GenerateSaveString without giving it full access to all protected members of this class.
		// Returns TRUE if the internal CEMNMoreInfoDlg object is not null and UpdateChangedInfo was called
		// on it. Returns FALSE if the CEMNMoreInfoDlg object is NULL.
		//
		// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
		// (r.farnworth 2014-02-19 09:01) - PLID 60746 - MoreInfoDlg changed to CodesDlg
		CEmrCodesDlg* pCodeDlg = GetCodesDlg(pEMN, FALSE);
		if (pCodeDlg) {
			// (z.manning 2013-06-05 09:48) - PLID 56962 - This no longer takes a param for output message.
			pCodeDlg->UpdateChangedInfo();
			return TRUE;
		}
		else {
			return FALSE;
		}
	} NxCatchAllSilentCallThrow(
		ASSERT(FALSE);
		Log("An exception was thrown from CEmrTreeWnd::UpdateEMNMoreInfoDlgChangedInfo. The exception will quietly be passed to the calling function.");
	);
}

BOOL CEmrTreeWnd::IsTreeNull() const
{
	// (c.haag 2007-09-28 11:44) - PLID 27509 - Returns TRUE if the tree is NULL
	return (m_pTree == NULL) ? TRUE : FALSE;
}

// (a.walling 2007-09-26 10:05) - PLID 27503 - Confirm with the user that they want to unspawn these EMNs.
LRESULT CEmrTreeWnd::OnQueryUnspawnEmns(WPARAM wParam, LPARAM lParam)
{
	try {
		CArray<CEMN*, CEMN*>* pEmnArray = (CArray<CEMN*, CEMN*>*)wParam;

		if (pEmnArray) {
			// Also, no point in prompting if we are on a template, but we shouldn't get this message in that
			// situation anyway.
			ASSERT(!m_bIsTemplate);
			CString strEMNs;
			bool bPromptWasPatientCreated = true;

			for(int i = pEmnArray->GetSize()-1; i >= 0; i--) {
				CEMN* pEmn = pEmnArray->GetAt(i);
				//(e.lally 2011-12-13) PLID 46968 - Skip and tell the user if the EMN is patient created and not finalized. 
				//	I decided to only give one message box if multiple are being unspawned even though it should be rare to happen at all.
				bool bIsPatientCreated = pEmn->GetPatientCreatedStatus() == CEMN::pcsCreatedByPatientNotFinalized;
				if(bIsPatientCreated && bPromptWasPatientCreated){
					bPromptWasPatientCreated = false;
					CString strMsg = FormatString("At least one EMN (%s, %s) was created by the patient, but the "
						"patient has not yet completed it. The patient created EMNs cannot be removed until the patient "
						"has finalized them."
						, pEmn->GetDescription(), FormatDateTimeForInterface(pEmn->GetEMNDate(),0,dtoDate));
					MessageBox(strMsg, "NexEMR", MB_OK|MB_ICONERROR);
				}

				// (j.jones 2009-10-01 11:36) - PLID 30479 - added a specific dialog
				// to prompt for deletion confirmation or EACH EMN being unspawned
				//(e.lally 2011-12-13) PLID 46968 - User does not get the option to delete if it is patient created and not finalized. It is always kept.
				CDeleteEMNConfirmDlg dlg(ecdtEMN, pEmn->GetDescription(), this);
				if(bIsPatientCreated || dlg.DoModal() != DELETE_EMN_RETURN_DELETE) {

					//unless the return value specifically says to delete,
					//we will keep the EMN, so remove from the array
					pEmnArray->RemoveAt(i);
				} else {			
					// (a.walling 2010-07-27 16:40) - PLID 39433 - Check for in-progress recording
					// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components

					if (GetPicContainer()) {		
						CAudioRecordDlg* pCurrentAudioRecordDlgInstance = CAudioRecordDlg::GetCurrentInstance();
						if (pCurrentAudioRecordDlgInstance != NULL && pCurrentAudioRecordDlgInstance->GetPatientID() == GetPatientID() && pCurrentAudioRecordDlgInstance->GetPicID() == GetPicContainer()->GetCurrentPicID() && pCurrentAudioRecordDlgInstance->GetEmn() == pEmn) {
							if (IDYES == MessageBox("A recording is in progress for this EMN. Continuing will disassociate the recording from this EMN, although it may still be attached to the EMR. Do you want to continue?", NULL, MB_ICONQUESTION | MB_YESNO))
							{
								pCurrentAudioRecordDlgInstance->ResetEmn();
							} else {
								//we will keep the EMN, so remove from the array
								pEmnArray->RemoveAt(i);
							}
						}
					}
				}
			}

			//if we have any EMNs left, we can remove them,

			if(pEmnArray->GetSize() == 0) {
				//we're keeping all EMNs
				return (LRESULT)0;
			} else {
				// go ahead and remove them!
				return (LRESULT)0xDEAD;
			}
		}

		return 0;
	} NxCatchAll("Error in CEmrTreeWnd::OnQueryUnspawnEmns");
	return -1;
}

void CEmrTreeWnd::SetTreeSel(NXDATALIST2Lib::IRowSettingsPtr pNewSel)
{
	//
	// (c.haag 2007-09-26 17:31) - PLID 27509 - This function is called when we want
	// to change the current tree selection. Unlike a direct call to PutCurSel() or
	// CurSel = ... this function will also handle the call to HandleSelChanged
	//

	// (a.walling 2012-06-28 17:11) - PLID 51276 - More Info should be a clickable link
	//TES 2/19/2014 - PLID 60750 - Same for Codes
	if (EmrTree::ChildRow(pNewSel).IsMoreInfoOrCodes()) {
		return;
	}

	// First, acquire the current selection
	IRowSettingsPtr pOldSel = m_pTree->CurSel;

	// Then change the current selection
	m_pTree->CurSel = pNewSel;

	// If the two selections differ, then we must call HandleSelChanged
	if (pOldSel != pNewSel) {
		HandleSelChanged(pOldSel, pNewSel);
	}
}

void CEmrTreeWnd::SetTreeRowVisible(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bVisible)
{
	//
	// (c.haag 2007-09-26 17:35) - PLID 27509 - This function must be called when
	// changing the Visible state of a row. If the currently selected row is made 
	// invisible, the datalist will internally change the current selection of the
	// list. This function is aware of that, and if the list selection needs to change
	// either because itself or one of its parents was made invisible, special handling
	// will be done to safely ensure that a valid topic is always visible.
	//

	// Sanity check
	if (NULL == pRow) {
		return;
	}

	// First, get the old and new visible values as variant booleans
	VARIANT_BOOL vbOld = pRow->Visible;
	VARIANT_BOOL vbNew = (bVisible) ? VARIANT_TRUE : VARIANT_FALSE;

	// Check whether the visibility actually changed. If so, then do nothing.
	// (c.haag 2007-10-11 15:30) - PLID 27509 - When comparing two booleans, one of which
	// came from a COM object, I prefer to test both for true and false rather than directly
	// comparing them...this is due to an experience I recently had with the ink control.
	if (!vbOld && !vbNew) {
		return;
	} else if (vbOld && vbNew) {
		return;
	}

	// If we are making a visible row into an invisible row, we need to check
	// whether it is the currently selected row. If it is so, then the currently
	// selected row will change; and we must account for that.
	if (VARIANT_FALSE == vbNew) {

		// Get the current selection
		IRowSettingsPtr pOldSel = m_pTree->CurSel;
		// (c.haag 2007-10-08 12:55) - These variables will help OnEmrTreeEnsureVisibleTopic find a next visible topic
		IRowSettingsPtr pNextSel = (NULL == pOldSel) ? NULL : GetNextVisibleRow(pOldSel); // CalcNextRow will make this go into an infinite loop
		void *pHint = (NULL == pNextSel) ? NULL : (CWnd*)VarLong(pNextSel->GetValue(TREE_COLUMN_OBJ_PTR));
		//TES 2/17/2014 - PLID 60750 - Added Codes
		if (NULL != pNextSel && 
			(etrtMoreInfo == VarLong(pNextSel->GetValue(TREE_COLUMN_ROW_TYPE)) || etrtCodes == VarLong(pNextSel->GetValue(TREE_COLUMN_ROW_TYPE)))) {
			// If the next topic is the More Info topic, the next selection should, ironically, be the
			// previous row
			pNextSel = GetPrevVisibleRow(pOldSel);
			pHint = (NULL == pNextSel) ? NULL : (CWnd*)VarLong(pNextSel->GetValue(TREE_COLUMN_OBJ_PTR));
		}
		

		// Update the row visibility (this is where the selection may change)
		pRow->Visible = vbNew;
		// Get the new selection
		IRowSettingsPtr pNewSel = m_pTree->CurSel;

		// Determine whether the visible row needs to change
		BOOL bRowMustChange = FALSE;

		if (pOldSel != pNewSel) {

			// If the act of setting the row invisible changed the current
			// tree selection, we must do special handling.
			bRowMustChange = TRUE;
		}
		else {

			// If the current selection did not change, it still might be
			// invalid because pRow could have been a parent of that selection.
			// Check for that.
			IRowSettingsPtr p = pOldSel;
			while (NULL != p && pRow != p) {
				p = p->GetParentRow();
			}

			// If the following is true, then pRow is a parent of the current
			// list selection...and because pRow is invisible, the current list
			// selection cannot be visible either.
			if (p == pRow) {
				bRowMustChange = TRUE;
			}
		}

		// Check to see whether we ultimately must change the selection
		if (bRowMustChange) {

			// The tree selection was, or must be changed...but the caller didn't
			// anticipate that. We can't call HandleSelChanged here because the caller
			// may intend to call it. So, we need to do our best to make sure that
			// HandleSelChanged(pOldSel, <new_visible_row>) is called later on. 
			//
			// The solution is to set the tree selection back to what it was when this
			// function was called, and to post a message which ultimately leads us to
			// HandleSelChanged(pOldSel, <new_visible_row>). If the caller does call
			// HandleSelChanged, then it will be a safe call, and the message will probably
			// not change anything. If the caller does not, then the message will find
			// a new best-fit visible row, and it will safely call HandleSelChanged. The
			// safe call would not be possible had we not restored the tree selection to
			// what it was before making the row invisible.
			//
			m_pTree->CurSel = pOldSel;
			// (c.haag 2007-10-08 12:56) - When the user deletes a tree row, the row is
			// made invisible before it is actually deleted. Chances are that the message
			// we are about to post will only be processed after the current tree selection
			// is set to NULL by RemoveTreeRow(). Therefore, we must pass in pHint; or else 
			// the message handler won't know what the previous selection was, and may decide
			// to change the current selection to the root of the EMR tree rather than an
			// adjacent topic.
			PostMessage(NXM_EMR_TREE_ENSURE_VISIBLE_TOPIC, (WPARAM)pHint);
		}

	} // if (VARIANT_FALSE == vbNew) {
	else {

		// If we are making an invisible row visible, then the datalist will not
		// change the current selection. So, just change the visible state, and
		// leave this function.
		pRow->Visible = vbNew;
	}
}

void CEmrTreeWnd::RemoveTreeRow(NXDATALIST2Lib::IRowSettingsPtr pRowToRemove)
{
	// (c.haag 2007-10-04 16:24) - PLID 27509 - This function safely removes a tree
	// row. If the currently selected row changes as a result of removing the row,
	// then m_pCurrentlyDisplayedRow will be updated, and we will post a message to
	// have the tree find a new selected topic

	// Sanity check
	if (NULL == pRowToRemove)
		return;

	// First, gather a couple values we will use soon after the row removal
	IRowSettingsPtr pOldSel = m_pTree->CurSel;
	// (c.haag 2007-10-08 12:55) - These variables will help OnEmrTreeEnsureVisibleTopic find a next visible topic
	void *pHint = NULL;
	if (NULL == m_lpDraggingRow) {
		IRowSettingsPtr pNextSel = (NULL == pOldSel) ? NULL : GetNextVisibleRow(pOldSel); // CalcNextRow will make this go into an infinite loop
		pHint = (NULL == pNextSel) ? NULL : (CWnd*)VarLong(pNextSel->GetValue(TREE_COLUMN_OBJ_PTR));
		//TES 2/17/2014 - PLID 60750 - Added Codes
		if (NULL != pNextSel && 
			(etrtMoreInfo == VarLong(pNextSel->GetValue(TREE_COLUMN_ROW_TYPE)) || etrtCodes == VarLong(pNextSel->GetValue(TREE_COLUMN_ROW_TYPE)))) {
			// If the next topic is the More Info topic, the next selection should, ironically, be the
			// previous row
			pNextSel = GetPrevVisibleRow(pOldSel);
			pHint = (NULL == pNextSel) ? NULL : (CWnd*)VarLong(pNextSel->GetValue(TREE_COLUMN_OBJ_PTR));
		}
	}

	// Second, do the actual row removal
	m_pTree->RemoveRow(pRowToRemove);

	// Lastly, if the selection changed as a result of the row removal, find a new
	// selection. The caller is responsible for having destroyed the window corresponding
	// to pOldSel, so there should not be a persistent window blocking what will become
	// the selected window.
	if (pOldSel != m_pTree->CurSel && NULL == m_lpDraggingRow) {
		m_pTree->CurSel = NULL;

		// (c.haag 2007-10-08 12:59) - Without passing in pHint, the act of posting
		// the message may result in the tree root being selected because the message
		// handler doesn't know what the row selection used to be. If SetTreeRowVisible()
		// had already posted this message, then this posting is not expected to have any
		// effect.
		PostMessage(NXM_EMR_TREE_ENSURE_VISIBLE_TOPIC, (WPARAM)pHint);
	}
}

// (a.walling 2007-10-12 11:19) - PLID 27017 - Return TRUE if the detail is in the Preview's pending update list
// (a.walling 2008-05-15 15:47) - PLID 29114 - Also return TRUE if the detail is in the delayed update queue
BOOL CEmrTreeWnd::IsDetailInPendingUpdateArray(CEMNDetail* pDetail)
{
	BOOL bInPreview = FALSE;

	try {
		if (GetEmrPreviewCtrl()) {
			bInPreview = GetEmrPreviewCtrl()->IsDetailInPendingUpdateArray(pDetail);
		}

		if (!bInPreview) {
			BOOL bDummy;
			bInPreview = m_mapDetailsPendingUpdate.Lookup(pDetail, bDummy);
		}
	} NxCatchAll("Error in CEmrTreeWnd::IsDetailInPendingUpdateArray");

	return bInPreview;
}

// (a.walling 2007-10-15 14:45) - PLID 25548 - Update the detail's position in the preview
LRESULT CEmrTreeWnd::OnEmnDetailUpdatePreviewPosition(WPARAM wParam, LPARAM lParam)
{
	try {
		if (GetEmrPreviewCtrl()) {
			CEMN* pDisplayedEMN = GetEmrPreviewCtrl()->GetCurrentEMN();
			CEMN* pMessageEMN = (CEMN*)lParam;
			if (pMessageEMN == pDisplayedEMN) {
				CEMNDetail* pDetail = (CEMNDetail*)wParam;
				CEMNDetail* pNextDetail = pDetail->GetNextDetail();
				GetEmrPreviewCtrl()->MoveDetail(pDetail, pNextDetail);
			}
		}
	} NxCatchAll("Error in CEmrTreeWnd::OnEmnDetailUpdatePreviewPosition");
	
	return 0;
}

// (a.walling 2007-12-17 16:18) - PLID 28391
LRESULT CEmrTreeWnd::OnRefreshTopicHTMLVisibility(WPARAM wParam, LPARAM lParam)
{
	try {
		if (GetEmrPreviewCtrl()) {
			CEMRTopic* pTopic = (CEMRTopic*)wParam;

			// (a.walling 2008-10-24 15:07) - PLID 31825 - I was using LOWORD twice!! nPrint is in the HIWORD. This
			// was causing all sorts of wierd issues with the visibility state when the preview was being dynamically
			// updated.
			long nScreen = LOWORD(lParam);
			long nPrint = HIWORD(lParam);

			if (nScreen == 0) 
				GetEmrPreviewCtrl()->EnsureTopicClass(pTopic, "hidescreen");
			else
				GetEmrPreviewCtrl()->EnsureTopicNotClass(pTopic, "hidescreen");

			if (nPrint == 0)
				GetEmrPreviewCtrl()->EnsureTopicClass(pTopic, "hideprint");
			else
				GetEmrPreviewCtrl()->EnsureTopicNotClass(pTopic, "hideprint");
		}
	} NxCatchAll("Error in CEmrTreeWnd::OnRefreshTopicHTMLVisibility");

	return 0;
}

// (a.walling 2008-05-14 16:31) - PLID 29114 - Delay (in ms) until refresh
long CEmrTreeWnd::GetPreviewRefreshDelay(EmrInfoType eit)
{
	switch (eit) {
	case eitImage:
	case eitTable: // (r.gonet 09/04/2013) - PLID 58432 - Added a table delay. Decided on 400 since we do query the database when tables are refreshed.
		return 400;
	case eitText:
	case eitNarrative:	
		return 250;
	default:
		return 0;
	}	
}

BOOL CEmrTreeWnd::OnEraseBkgnd(CDC *pDC)
{
	try
	{
		// (z.manning, 05/15/2008) - PLID 30050 - Fill solid background color based on our brush
		CRect rcClient;
		GetClientRect(rcClient);
		pDC->FillRect(rcClient, &m_brBackground);

	}NxCatchAll("CEmrTreeWnd::OnEraseBkgnd");

	//TRUE = No further erasing is required
	return TRUE;
}

// (z.manning, 05/15/2008) - PLID 30050 - Set the background color for the tree wnd
void CEmrTreeWnd::SetBackgroundColor(COLORREF clr)
{
	if(m_brBackground.GetSafeHandle() != NULL) {
		m_brBackground.DeleteObject();
	}
	m_brBackground.CreateSolidBrush(clr);
}

// (a.walling 2008-06-02 12:18) - PLID 22049 - An EMN has completed loading
LRESULT CEmrTreeWnd::OnEmnLoadSaveComplete(WPARAM wParam, LPARAM lParam)
{
	try {
		CEMN* pEMN = (CEMN*)wParam;

		if (lParam == 2) {
			// (a.walling 2008-08-13 13:45) - PLID 31037 - an EMR was saved and deleted this EMN
			if (GetEmrPreviewCtrl()) {
				GetEmrPreviewCtrl()->RemoveEMN(pEMN);
			}

			return 0;
		}

		// Get the displayed EMN
		NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_pTree->CurSel;
		if(pCurRow) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = pCurRow;
			NXDATALIST2Lib::IRowSettingsPtr pParent = pRow->GetParentRow();
			while(pParent) {
				pRow = pParent;
				pParent = pRow->GetParentRow();
			}
			//pRow is now the top-level row.
			ASSERT(VarLong(pRow->GetValue(TREE_COLUMN_ROW_TYPE)) == (long)etrtEmn);
			
			CEMN* pCurrentEMN = EmrTree::ChildRow(pRow).GetEMN();

			if (pEMN == pCurrentEMN) {
				TryAutoWriteAccess(pEMN, NULL);
			}
		}

		// (e.lally 2012-03-14) PLID 48891 - Call the EmrPatientFrameWnd version of this function too.
		// (e.lally 2012-04-03) PLID 48891 - Only call this upon save, not load
		if(lParam == 1 && GetPicContainer()){
			dynamic_cast<CEmrPatientFrameWnd*>(GetPicContainer())->OnEmnLoadSaveComplete();
		}
	
	} NxCatchAll("Error in CEmrTreeWnd::OnEmnLoadComplete");

	return 0; 

	/*
	try {
		CEMN* pEMN = (CEMN*)wParam;
		if (pEMN) {
			if (pEMN->GetID() != -1) {
				if (pEMN->GetStatus() != 2 && !pEMN->IsWritable()) {
					if (lParam == 1) {
						// just saved a new EMN, go ahead and acquire write access by default.
						#pragma TODO("// (a.walling 2008-05-22 16:44) - PLID 22049 - Only grab write access to an EMN when necessary")
						CWriteTokenInfo wtInfo;
						BOOL bSuccess = pEMN->RequestWriteToken(wtInfo);
						if (!bSuccess) {
							HandleWriteTokenRequestFailure(pEMN, wtInfo);
						}
					}
				}
			}
		}
	} NxCatchAll("Error in CEmrTreeWnd::OnEmnLoadComplete");

	return 0;
	*/
}

LRESULT CEmrTreeWnd::OnEmnWriteAccessChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		// Find the EMN!
		long nEmnID = wParam;
		CEMN* pTargetEMN = GetEMNByID(wParam);

		if (pTargetEMN == NULL) return 0;

		// (a.walling 2008-07-03 15:30) - PLID 30498 - If we were waiting on this, clear out our info
		if (lParam) {
			BOOL bDummy;
			if (m_mapWaitingOnRelease.Lookup(nEmnID, bDummy)) {
				m_mapWaitingOnRelease.RemoveKey(nEmnID);

				CWnd* pWnd = NULL;
				if (m_mapModelessWaitingWindows.Lookup(nEmnID, pWnd)) {
					m_mapModelessWaitingWindows.RemoveKey(nEmnID);
					if (::IsWindow(pWnd->GetSafeHwnd())) {
						::DestroyWindow(pWnd->GetSafeHwnd());
						delete pWnd;
					}
				}
			}

			// (a.walling 2008-07-07 11:12) - PLID 30498 - Clear the failed map
			m_mapWriteAccessFailed.RemoveKey(nEmnID);
		}

		NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_pTree->GetFirstRow();
		while(pCurRow) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pCurRow).GetType();
			if (etrt == etrtEmn) {
				CEMN *pEMN = EmrTree::ChildRow(pCurRow).GetEMN();

				if (pEMN == pTargetEMN) {
					// (c.haag 2006-03-28 12:13) - PLID 19890 - We now consider permissions when initializing topic windows
					BOOL bCanWriteToEMR = (m_bIsTemplate) ? TRUE :
						CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE);

					// (a.walling 2007-11-28 11:25) - PLID 28044 - Check if expired
					if (g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
						bCanWriteToEMR = FALSE;
					}
					
					BOOL bLocked = (pEMN->GetStatus() == 2);
					
					// (a.walling 2008-06-03 17:55) - PLID 23138
					BOOL bIsReadOnly = !(pEMN->IsWritable());

					EnsureRowIcon(pCurRow);

					CArray<CEMRTopic*,CEMRTopic*> arTopics;

					pEMN->GetAllTopics(arTopics);

					for (int i = 0; i < arTopics.GetSize(); i++) {
						CEmrTopicWndPtr pTopicWnd = arTopics[i]->GetTopicWnd();

						if (pTopicWnd && ::IsWindow(pTopicWnd->GetSafeHwnd())) {
							// (a.wetta 2006-11-16 09:42) - PLID 19474 - If this topic is a spawned topic on a template, then it should also be read only
							BOOL bSpawnedTemplateTopic = (m_bIsTemplate && arTopics[i]->GetSourceActionID() != -1) ? TRUE : FALSE;
					
							pTopicWnd->SetReadOnly((bLocked || !bCanWriteToEMR || bSpawnedTemplateTopic || bIsReadOnly) ? TRUE : FALSE);
							pTopicWnd->SetAllowEdit((!bSpawnedTemplateTopic && GetEmrFrameWnd()->IsEditMode() && !bIsReadOnly) ? TRUE : FALSE);
						}
					}

					// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
					CEMNMoreInfoDlg* pMoreInfo = GetMoreInfoDlg(pEMN, FALSE);
					if (pMoreInfo) {
						pMoreInfo->SetReadOnly(bLocked || !bCanWriteToEMR || bIsReadOnly);
					}

					//TES 2/21/2014 - PLID 60740 - Update the Codes topic as well
					CEmrCodesDlg* pCodes = GetCodesDlg(pEMN, FALSE);
					if(pCodes) {
						pCodes->SetReadOnly(bLocked || !bCanWriteToEMR || bIsReadOnly);
					}
				}
			}

			pCurRow = pCurRow->GetNextRow();
		}
	} NxCatchAll("Error in CEmrTreeWnd::OnEmnWriteAccessChanged");

	return 0;
}

// (a.walling 2008-06-04 09:42) - PLID 23138 - Command fired to toggle readonly
void CEmrTreeWnd::OnToggleReadOnly()
{	try {
		HandleToggleReadOnly(false);
	} NxCatchAll("Error in CEmrTreeWnd::OnEmnWriteAccessChanged");
}

//(e.lally 2011-12-13) PLID 46968 - Handles the toggling between Read Only and Write Access.
//	bFailIfReadOnly should only be set if the caller expects to always be releasing the user's write access. An exception will be throw if that is not the case.
void CEmrTreeWnd::HandleToggleReadOnly(bool bFailIfReadOnly /* = false*/)
{
		//First, is this EMN already being displayed?
		NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_pTree->CurSel;
		if(pCurRow) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = pCurRow;
			NXDATALIST2Lib::IRowSettingsPtr pParent = pRow->GetParentRow();
			while(pParent) {
				pRow = pParent;
				pParent = pRow->GetParentRow();
			}
			//pRow is now the top-level row.
			ASSERT(VarLong(pRow->GetValue(TREE_COLUMN_ROW_TYPE)) == (long)etrtEmn);
			
			CEMN* pEMN = EmrTree::ChildRow(pRow).GetEMN();

			if (pEMN && pEMN->GetID() != -1) {
				if (!pEMN->IsWritable()) {
					if(bFailIfReadOnly){
						AfxThrowNxException("Attempt to request write access when this type of request is not allowed.");
					}
					CWriteTokenInfo wtInfo;					
					m_mapAutoWriteAccessTried.SetAt(pEMN->GetID(), FALSE);
					BOOL bSuccess = pEMN->RequestWriteToken(wtInfo);
					if (!bSuccess) {
						HandleWriteTokenRequestFailure(pEMN, wtInfo);
					}
				} else {
					if (pEMN->IsUnsaved()) {
						// (a.walling 2010-09-09 10:56) - PLID 40224 - Don't allow releasing write access without saving!
						if (IDYES == MessageBox("This EMN has unsaved changes. To avoid losing your work, you must save before releasing write access. Do you want to save now?", NULL, MB_YESNO|MB_ICONHAND)) {
							if(SaveEMR(esotEMR, -1, TRUE) != essSuccess)
								return;
						} else {
							return;
						}
					}
					m_mapAutoWriteAccessTried.SetAt(pEMN->GetID(), FALSE);
					pEMN->ReleaseWriteToken();
				}
			}
		}		
}

// (a.walling 2008-06-04 12:56) - PLID 22049
void CEmrTreeWnd::HandleEMNAccessTableChecker(long nEmnID)
{
	try {
		if (nEmnID != -1) {
			for (int i = 0; i < GetEMNCount(); i++) {
				CEMN* pEMN = GetEMN(i);

				if (pEMN->GetID() == nEmnID) {
					BOOL bDummy;
					if (m_mapWaitingOnRelease.Lookup(nEmnID, bDummy)) {
						m_mapWaitingOnRelease.RemoveKey(nEmnID);
						
						// we were waiting on this one!

						CWnd* pWnd = NULL;
						if (m_mapModelessWaitingWindows.Lookup(nEmnID, pWnd)) {
							m_mapModelessWaitingWindows.RemoveKey(nEmnID);
							if (::IsWindow(pWnd->GetSafeHwnd())) {
								::DestroyWindow(pWnd->GetSafeHwnd());
								delete pWnd;
							}
						}

						CWriteTokenInfo wtReqInfo;
						if (!pEMN->RequestWriteToken(wtReqInfo)) {
							HandleWriteTokenRequestFailure(pEMN, wtReqInfo);
						}

						return;
					}

					CWriteTokenInfo wtInfo;
					BOOL bWasWritable = pEMN->IsWritable();
					BOOL bOwned = pEMN->VerifyWriteToken(wtInfo);
					
					if (bOwned) {
						// we own this already, no point saying anything, but the revision should be intact!
						ASSERT(!wtInfo.bIsOldRevision);
						// (a.walling 2008-08-13 14:20) - PLID 22049 - Handle the possibility of deletion
						if (wtInfo.bIsOldRevision && !wtInfo.bIsDeleted) {
							GetMainFrame()->NotifyUser(NT_DEFAULT, FormatString("The EMN '%s' has been modified by another user since it was opened.", pEMN->GetDescription()));
						} else if (wtInfo.bIsDeleted) {
							GetMainFrame()->NotifyUser(NT_DEFAULT, FormatString("The EMN '%s' has been deleted by another user since it was opened.", pEMN->GetDescription()));
							// (a.walling 2008-08-13 14:33) - PLID 22049 - Remove from display
							RemoveEMN(pEMN);
						}
					} else {
						// (a.walling 2008-08-13 14:22) - PLID 22049 - Don't prompt if deleted
						if (wtInfo.nHeldByUserID == -1 && !wtInfo.bIsDeleted) {
							// this EMN is free.
							// (a.walling 2008-07-07 11:15) - PLID 30498 - notify the user if they have failed to acquire write access here before
							BOOL bDummy;
							if (m_mapWriteAccessFailed.Lookup(pEMN->GetID(), bDummy)) {
								CString strNotify;
								strNotify.Format("The EMN '%s' %sis now free for editing.", pEMN->GetDescription(), wtInfo.bIsOldRevision ? "was modified by another user, and " : "");
								
								GetMainFrame()->NotifyUser(NT_DEFAULT, strNotify);
							}
						} else if (!wtInfo.bIsDeleted) {
							if (bWasWritable) {
								CWnd* pWnd = NULL;
								if (m_mapModelessSelfDestructWindows.Lookup(pEMN->GetID(), pWnd)) {
									pWnd->DestroyWindow();
									// (a.walling 2010-03-29 15:54) - PLID 34289 - These windows self-destruct
									//delete pWnd;
								}

								CEMRAlertDlg* dlgAlert = new CEMRAlertDlg(this);
								m_mapModelessSelfDestructWindows.SetAt(pEMN->GetID(), dlgAlert);

								// (j.armen 2013-05-14 11:09) - PLID 56680 - Write Token Info keeps track of external status
								CString strMessage = FormatString(
									"The EMN '%s' has been opened exclusively for editing by user '%s' %s at %s (using %s %s).\r\n\r\n"
									"Your write access has been withdrawn. You will be unable to save your changes unless the other user releases write access without saving.",
									pEMN->GetDescription(),
									wtInfo.strHeldByUserName,
									FormatDateTimeForInterface(wtInfo.dtHeld, 0, dtoDateWithToday),
									FormatDateTimeForInterface(wtInfo.dtHeld, 0, dtoTime),
									wtInfo.bIsExternal ? "an external device, identified as:" : "workstation",
									wtInfo.strDeviceInfo);
								
								dlgAlert->SetText(strMessage);
								dlgAlert->SetInfo(wtInfo, pEMN->GetID(), m_bIsTemplate, pEMN->GetDescription(), FALSE);
								dlgAlert->Create(IDD_EMR_ALERT_DLG, this);			
								dlgAlert->SetOwner(this);
								dlgAlert->CenterWindow();
								dlgAlert->ShowWindow(SW_SHOW);
							} else {
								// they aren't editing, they don't really need to care, but perhaps we should flag that it will be modified
							}
						} else if (wtInfo.bIsDeleted) {
							// (a.walling 2008-08-13 14:24) - PLID 22049 - Let them know if anything gets deleted.
							GetMainFrame()->NotifyUser(NT_DEFAULT, FormatString("The EMN '%s' has been deleted by another user since it was opened.", pEMN->GetDescription()));
							RemoveEMN(pEMN);
						}
					}
				}
			}
		}
	} NxCatchAll("Error refreshing EMN Access table");
}

// (a.walling 2008-06-10 11:56) - PLID 22049 - Command handler to reload an EMN
void CEmrTreeWnd::OnReloadEMN()
{
	try {
		CEMN* pEMN = NULL;

		NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_pTree->CurSel;
		if(pCurRow) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = pCurRow;
			NXDATALIST2Lib::IRowSettingsPtr pParent = pRow->GetParentRow();
			while(pParent) {
				pRow = pParent;
				pParent = pRow->GetParentRow();
			}
			//pRow is now the top-level row.
			ASSERT(VarLong(pRow->GetValue(TREE_COLUMN_ROW_TYPE)) == (long)etrtEmn);

			NXDATALIST2Lib::IRowSettingsPtr pEMNRow = pRow;
			
			pEMN = EmrTree::ChildRow(pRow).GetEMN();

			if (pEMN->IsUnsaved()) {
				if (IDCANCEL == MessageBox(
					"Unsaved changes to this EMN will be lost if you continue!",
					NULL,
					MB_OKCANCEL)) {
					return;
				}
			}

			ReloadEMN(pEMN, TRUE);
		}
	} NxCatchAll("Error in CEmrTreeWnd::OnReloadEMN");
}


// (a.walling 2008-06-10 11:56) - PLID 22049 - Reloads an EMN
BOOL CEmrTreeWnd::ReloadEMN(CEMN*& pEMN, BOOL bShow)
{
	try {		
		if (pEMN) {
			m_bReloadingEMN = TRUE;


			long nID = pEMN->GetID();
			long nEMNtoDisplay = nID;
			ASSERT(nID != -1);

			long nOriginalIndex = m_EMR.RemoveEMNByPointerRaw(pEMN);

			// (a.walling 2008-08-13 14:32) - PLID 22049 - Moved to shared function
			RemoveEMN(pEMN);

			pEMN = NULL;

			if (m_bIsTemplate) {
				m_EMR.LoadFromID(nID, TRUE, nEMNtoDisplay, nID);
				pEMN = m_EMR.GetEMNByID(nID);
			} else {
				m_EMR.LoadFromID(m_EMR.GetID(), m_EMR.GetIsTemplate(), nEMNtoDisplay, nID);
				pEMN = m_EMR.GetEMNByID(nID);
				
				m_EMR.ReorderEMN(pEMN, nOriginalIndex);
			}

			SendMessage(NXM_EMN_ADDED, (WPARAM)pEMN, TRUE);

			if (bShow) {
				ShowEMN(pEMN);
			}

			m_bReloadingEMN = FALSE;

			return TRUE;
		}
	} NxCatchAll("Error reloading EMN");

	m_bReloadingEMN = FALSE;

	return FALSE;
}

// (a.walling 2008-08-13 14:29) - PLID 22049 - Removes an EMN from the tree (and memory)
void CEmrTreeWnd::RemoveEMN(CEMN* pEMN)
{
	if (CEmrMoreInfoView* pMoreInfoView = GetEmrFrameWnd()->GetEmrMoreInfoView(pEMN, false)) {
		pMoreInfoView->GetParentFrame()->SendMessage(WM_CLOSE);
	}
	//TES 2/12/2014 - PLID 60740 - Added for the new <Codes> topic
	if (CEmrCodesView* pCodesView = GetEmrFrameWnd()->GetEmrCodesView(pEMN, false)) {
		pCodesView->GetParentFrame()->SendMessage(WM_CLOSE);
	}
	if (CEmrTopicView* pTopicView = GetEmrFrameWnd()->GetEmrTopicView(pEMN, false)) {
		pTopicView->GetParentFrame()->SendMessage(WM_CLOSE);
	}

	ASSERT(pEMN != NULL);
	NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pTree->GetCurSel();

	NXDATALIST2Lib::IRowSettingsPtr pEMNRow = m_pTree->GetFirstRow();
	if(pEMNRow) {
		while(pEMNRow) {
			if (pEMN == EmrTree::ChildRow(pEMNRow).GetEMN()) {
				break;
			}
			pEMNRow = pEMNRow->GetNextRow();
		}
	}

	if (pEMNRow == NULL) {
		ThrowNxException("Could not find EMN row!");
	}
	
	if (GetEmrPreviewCtrl()) {
		GetEmrPreviewCtrl()->RemoveEMN(pEMN);
	}

	if (pCurSel != NULL && pEMNRow->IsSameRow(pCurSel)) {
		SetTreeSel(NULL);
		pCurSel = NULL;
	}
	
	// (a.walling 2008-07-07 11:26) - PLID 22049 - We have the EMN, now we have to get
	// all the rows beneath us and ensure we have destroyed their topic pointers

	FOR_ALL_ROWS_WITH_PARENT(m_pTree, pEMNRow) {
		if (pCurSel != NULL && pRow->IsSameRow(pCurSel)) {
			SetTreeSel(NULL);
			pCurSel = NULL;
		}
		GET_NEXT_ROW(m_pTree)
	}
	m_pTree->RemoveRow(pEMNRow);

	// (a.walling 2008-08-22 13:26) - PLID 22049 - EMR still has a dangling pointer to this EMN
	m_EMR.RemoveEMNByPointerRaw(pEMN);
	delete pEMN;
}

// (a.walling 2008-06-10 15:30) - PLID 22049 - Display messages to the user and handle their decisions
// (a.walling 2008-07-03 12:35) - PLID 30498 - Pass whether we want to try to force if it failed
void CEmrTreeWnd::HandleWriteTokenRequestFailure(CEMN* pEMN, CWriteTokenInfo& wtInfo, BOOL bForce)
{
	try
	{
		BOOL bDummy;
		BOOL bShown = FALSE;
		if (m_mapWaitingOnRelease.Lookup(pEMN->GetID(), bDummy)) {
			CWnd* pWnd;
			if (m_mapModelessWaitingWindows.Lookup(pEMN->GetID(), pWnd)) {
				if (::IsWindow(pWnd->GetSafeHwnd())) {
					pWnd->ShowWindow(SW_SHOW);
					bShown = TRUE;
				}
			}
		}

		// (a.walling 2008-07-07 11:12) - PLID 30498 - Set the failed map
		if (pEMN != NULL && pEMN->GetID() != -1) {
			m_mapWriteAccessFailed.SetAt(pEMN->GetID(), TRUE);
		}

		if (bShown) return;

		if (bForce) {
			if (pEMN->GetID() != -1) {
				// (a.walling 2008-07-03 13:03) - PLID 30498 - Send the notification and popup the wait dlg
				m_mapWaitingOnRelease[pEMN->GetID()] = TRUE;
				CClient::RefreshTable(NetUtils::EMNForceAccessT, pEMN->GetID(), NULL, FALSE);
				CEMNWaitForAccessDlg* pWaitDlg = new CEMNWaitForAccessDlg(this);				
				m_mapModelessWaitingWindows.SetAt(pEMN->GetID(), pWaitDlg);

				pWaitDlg->SetInfo(pEMN->GetID(), pEMN->GetDescription(), wtInfo);
				pWaitDlg->Create(IDD_EMN_FORCE_ACCESS_WAIT_DLG, this);
				pWaitDlg->SetOwner(this);
				pWaitDlg->CenterWindow();
				pWaitDlg->ShowWindow(SW_SHOW);
			} else {
				ASSERT(FALSE);
			}
		} else {
			// a few basic logic paths. There would be an error if 1) the revision is out of date or 2) it is held by another user

			CString strMessage;
			// (a.walling 2008-08-13 14:26) - PLID 22049 - Alert them if it was deleted
			if (wtInfo.bIsDeleted) {
				strMessage.Format("The %s '%s' has been deleted by another user since it was opened! This EMN can not be modified or saved.",
					m_bIsTemplate ? "template" : "EMN",
					pEMN->GetDescription());
				MessageBox(strMessage, NULL, MB_YESNO | MB_ICONEXCLAMATION);

				// (a.walling 2008-08-13 14:33) - PLID 22049 - Remove from display
				RemoveEMN(pEMN);
			} else if (wtInfo.bIsOldRevision) {
				// not held by a user, but has been modified
				strMessage.Format("The %s '%s' has been modified by another user since it was opened and needs to be reloaded before it can be edited. %s\r\n\r\nWould you like to reload and request write access now?",
					m_bIsTemplate ? "template" : "EMN",
					pEMN->GetDescription(),
					pEMN->IsUnsaved() ? "\r\n\r\nUnsaved changes to this EMN will be lost if you continue. If you must preserve these changes, you can cancel and copy this EMN before reloading." : ""
				);

				long nRet = MessageBox(strMessage, NULL, MB_YESNO | MB_ICONEXCLAMATION);
				if (nRet == IDYES) {
					if (ReloadEMN(pEMN, TRUE)) {
						if(pEMN->GetPatientCreatedStatus() == CEMN::pcsCreatedByPatientNotFinalized) {
							//(e.lally 2011-12-13) PLID 46968 - We may not allow them to even attempt
							// to get a write token if the EMN is patient-created, but not finalized (NexWeb).
							// We should warn them about that as well so they know why they can't edit this EMN.
							CString strMsg = FormatString("This EMN (%s, %s) was created by the patient, but the "
								"patient has not yet completed it. You may not edit this EMN until the patient "
								"has finalized it."
								, pEMN->GetDescription(), FormatDateTimeForInterface(pEMN->GetEMNDate(),0,dtoDate));
							MessageBox(strMsg, "NexEMR", MB_OK|MB_ICONINFORMATION);
						}
						else if (!pEMN->IsWritable()) {
							BOOL bSuccess = pEMN->RequestWriteToken(wtInfo);
							if (!bSuccess) {
								HandleWriteTokenRequestFailure(pEMN, wtInfo);
							}
						}
					} else {
						ThrowNxException("Failed to reload EMN!");
					}
				}
			} else if (wtInfo.nHeldByUserID != -1 && wtInfo.bIsVerified == FALSE) {
				// held by a user (that is not us!). we'll use the same message regardless of whether it has been modified yet or not.
				CWnd* pWnd = NULL;
				if (m_mapModelessSelfDestructWindows.Lookup(pEMN->GetID(), pWnd)) {
					pWnd->DestroyWindow();
					// (a.walling 2010-03-29 15:54) - PLID 34289 - These windows self-destruct
					//delete pWnd;
				}

				CEMRAlertDlg* dlgAlert = new CEMRAlertDlg(this);
				m_mapModelessSelfDestructWindows.SetAt(pEMN->GetID(), dlgAlert);
				
				// (j.armen 2013-05-14 11:09) - PLID 56680 - Write Token Info keeps track of external status
				strMessage.Format("The %s '%s' has been opened exclusively for editing by user '%s' %s at %s.\r\n\r\n(using %s %s)",
					m_bIsTemplate ? "template" : "EMN",
					pEMN->GetDescription(),
					wtInfo.strHeldByUserName,
					FormatDateTimeForInterface(wtInfo.dtHeld, 0, dtoDateWithToday),
					FormatDateTimeForInterface(wtInfo.dtHeld, 0, dtoTime),
					wtInfo.bIsExternal ? "an external device, identified as:" : "workstation",
					wtInfo.strDeviceInfo);
				
				dlgAlert->SetText(strMessage);
				dlgAlert->SetInfo(wtInfo, pEMN->GetID(), m_bIsTemplate, pEMN->GetDescription(), TRUE);
				dlgAlert->Create(IDD_EMR_ALERT_DLG, this);			
				dlgAlert->SetOwner(this);
				dlgAlert->CenterWindow();
				dlgAlert->ShowWindow(SW_SHOW);
			}
			else if(pEMN->GetPatientCreatedStatus() == CEMN::pcsCreatedByPatientNotFinalized) {
				// (z.manning 2009-11-19 10:16) - PLID 35810 - We may not allow them to even attempt
				// to get a write token if the EMN is patient-created, but not finalized (NexWeb).
				// We should warn them about that as well so they know why they can't edit this EMN.
				CString strMsg = FormatString("This EMN (%s, %s) was created by the patient, but the "
					"patient has not yet completed it. You may not edit this EMN until the patient "
					"has finalized it."
					, pEMN->GetDescription(), FormatDateTimeForInterface(pEMN->GetEMNDate(),0,dtoDate));
				MessageBox(strMsg, "NexEMR", MB_OK|MB_ICONINFORMATION);
			}
		}
	} NxCatchAll("Error in HandleWriteTokenRequestFailure");
}

// (a.walling 2008-06-11 11:46) - PLID 22049 - A modeless window is closing
LRESULT CEmrTreeWnd::OnWindowClosing(WPARAM wParam, LPARAM lParam)
{
	try {
		CWnd* pClosingWnd = (CWnd*)wParam;
		long nID = lParam;

		m_mapModelessSelfDestructWindows.RemoveKey(nID);
	} NxCatchAll("Error in CEmrTreeWnd::OnWindowClosing");

	return 0;
}

// (a.walling 2008-06-11 11:46) - PLID 22049 - A request has been made to try to acquire write access
LRESULT CEmrTreeWnd::OnTryAcquireWriteAccess(WPARAM wParam, LPARAM lParam)
{
	long nEmnID = (long)wParam;
	BOOL bForce = (BOOL) (HIWORD(lParam) ? TRUE : FALSE);
	BOOL bForceNow = (BOOL) (LOWORD(lParam) ? TRUE : FALSE);

	CEMN* pEMN = m_EMR.GetEMNByID(nEmnID);

	if (pEMN != NULL && !pEMN->IsWritable()) {
		CWriteTokenInfo wtInfo;
		m_mapAutoWriteAccessTried.SetAt(nEmnID, FALSE);
		// (a.walling 2008-07-03 12:34) - PLID 30498 - Don't force immediately unless Forcing NOW
		// (a.walling 2008-07-07 16:32) - PLID 30498 - Or if a template
		BOOL bSuccess = pEMN->RequestWriteToken(wtInfo, m_bIsTemplate ? bForce : bForceNow);
		if (!bSuccess) {
			HandleWriteTokenRequestFailure(pEMN, wtInfo, bForce);
		}
	}

	return 0;
}

void CEmrTreeWnd::TryAutoWriteAccess(CEMN* pNewEMN, CEMN* pOldEMN)
{
	try {		
		BOOL b = FALSE;
		BOOL bNewEMNManuallyToggledOrFailed = (pNewEMN == NULL) ? TRUE : m_mapAutoWriteAccessTried.Lookup(pNewEMN->GetID(), b);
		BOOL bOldEMNManuallyToggledOrFailed = (pOldEMN == NULL) ? TRUE : m_mapAutoWriteAccessTried.Lookup(pOldEMN->GetID(), b);

//#pragma TODO("Better handling of releasing automatic write tokens for other EMNs")
		if (!bOldEMNManuallyToggledOrFailed && pNewEMN != pOldEMN && pOldEMN != NULL && pOldEMN->IsWritable() && pOldEMN->GetID() != -1 && !pOldEMN->IsUnsaved()) {
			pOldEMN->ReleaseWriteToken();
		}

		if (!bNewEMNManuallyToggledOrFailed && pNewEMN != NULL && !pNewEMN->IsWritable()) {
			CWriteTokenInfo wtInfo;
			if (!pNewEMN->RequestWriteToken(wtInfo)) {
				m_mapAutoWriteAccessTried.SetAt(pNewEMN->GetID(), FALSE);
				HandleWriteTokenRequestFailure(pNewEMN, wtInfo);
			}
		}
	} NxCatchAll("Error auto-acquiring/releasing write access");
}


// (a.walling 2008-07-01 09:29) - PLID 30571 - Added OnUpdateEmrPreview to update the detail/topic in the EMR PreviewPane
LRESULT CEmrTreeWnd::OnUpdateEmrPreview(WPARAM wParam, LPARAM lParam)
{
	try {
		if (wParam) {
			// topic
			CEMRTopic* pTopic = (CEMRTopic*)lParam;

			if (GetEmrPreviewCtrl() != NULL && pTopic != NULL) {
				CEMRTopic* pParentTopic = pTopic->GetParentTopic();

				if (pParentTopic) {
					GetEmrPreviewCtrl()->UpdateTopic(pParentTopic);
				} else {
					// curses, we have to reload.
					if (pTopic->GetParentEMN() != NULL) {
						pTopic->GetParentEMN()->GenerateHTMLFile(TRUE, FALSE, FALSE);
					}
				}
			}

			EnsureTopicModifiedState(pTopic);
		} else {
			// detail
			CEMNDetail* pDetail = (CEMNDetail*)lParam;


			if(GetEmrPreviewCtrl() != NULL) {
				// (r.gonet 09/04/2013) - PLID 58432 - Copied a.walling's optimization that was used elsewhere in OnEmrItemChanged and OnEmrItemStateChanged. We will
				//  delay the update of the preview for certain item types due to the potential for there to be many rapid changes. The effect is to combine
				//  these separate updates into one once the series of rapid changes is stops or slows down.
				long nDelay = GetPreviewRefreshDelay(pDetail->m_EMRInfoType);

				if (nDelay > 0) {
					BOOL bDummy;
					// (r.gonet 09/04/2013) - PLID 58432 - See if we have already requested that this detail be changed in the preview.
					if (!m_mapDetailsPendingUpdate.Lookup(pDetail, bDummy)) {
						// (r.gonet 09/04/2013) - PLID 58432 - Add the detail to the pending details map. Once the series of rapid changes is finished, we will update
						//  the preview for any details appearing in this map.
						m_mapDetailsPendingUpdate[pDetail] = TRUE;
						pDetail->__AddRef("CEmrTreeWnd::OnUpdateEmrPreview pend update");
						//TRACE("**>>      New Detail 0x%08x (%s) pending update at %lu\n", pDetail, pDetail->GetLabelText(), GetTickCount());
					} else {
						// (r.gonet 09/04/2013) - PLID 58432 - We are already waiting on this detail to be updated in the preview. Keep waiting.
						//TRACE("**>> Existing Detail 0x%08x (%s) pending update at %lu\n", pDetail, pDetail->GetLabelText(), GetTickCount());
					}
					// (r.gonet 09/04/2013) - PLID 58432 - Reset the timer. If nDelay milliseconds passes without further changes, we will update the preview.
					SetTimer(IDT_PREVIEW_REFRESH, nDelay, NULL);			
				} else {
					// (r.gonet 09/04/2013) - PLID 58432 - We can't delay. Update the preview now.
					UpdateDetailPreview(pDetail);
				}
			} else {
				// (r.gonet 09/04/2013) - PLID 58432 - There is nothing to refresh! This can occur for instance
				//  if the preview pane is never loaded when opening an EMN.
			}

			if (pDetail->m_pParentTopic) {
				EnsureTopicModifiedState(pDetail->m_pParentTopic);
			}
		}
	} NxCatchAll("Error updating preview");
	return 0;
}

// (a.walling 2008-07-01 09:59) - PLID 30570
void CEmrTreeWnd::OnHideTopicPreview()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pTree->CurSel;
		if(pCurSel) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pCurSel).GetType();
			if(etrt == etrtTopic) {
				CEMRTopic* pTopic = EmrTree::ChildRow(pCurSel).GetTopic();
				if(pTopic) {
					pTopic->SetPreviewFlags(pTopic->GetPreviewFlags() ^ epfHideItem);
				}
			}
		}	
	} NxCatchAll("Error attempting to set topic preview flags");
}


// (a.walling 2008-07-01 17:58) - PLID 30570
void CEmrTreeWnd::OnHideTopicTitle()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pTree->CurSel;
		if(pCurSel) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pCurSel).GetType();
			if(etrt == etrtTopic) {
				CEMRTopic* pTopic = EmrTree::ChildRow(pCurSel).GetTopic();
				if(pTopic) {
					pTopic->SetPreviewFlags(pTopic->GetPreviewFlags() ^ epfHideTitle);
				}
			}
		}	
	} NxCatchAll("Error attempting to set topic title preview flags");
}

void CEmrTreeWnd::OnHideAllDetailsPreview()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pTree->CurSel;
		if(pCurSel) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pCurSel).GetType();
			if(etrt == etrtTopic) {
				CEMRTopic* pTopic = EmrTree::ChildRow(pCurSel).GetTopic();
				if(pTopic) {
					if (IDYES == MessageBox("This action will set each detail in this topic to hide when printing the preview. This cannot be undone except by individually marking details to be shown. Do you want to continue?", NULL, MB_YESNO)) {
						for (int i = 0; i < pTopic->GetEMNDetailCount(); i++) {
							CEMNDetail* pDetail = pTopic->GetDetailByIndex(i);

							// only refresh the parent if the last one
							pDetail->SetPreviewFlags(pDetail->GetPreviewFlags() | epfHideItem, (i == (pTopic->GetEMNDetailCount() - 1)));
						}
					}
				}
			}
		}	
	} NxCatchAll("Error attempting to set all detail preview flags");
}

void CEmrTreeWnd::OnHideAllDetailsTitle()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pTree->CurSel;
		if(pCurSel) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pCurSel).GetType();
			if(etrt == etrtTopic) {
				CEMRTopic* pTopic = EmrTree::ChildRow(pCurSel).GetTopic();
				if(pTopic) {
					if (IDYES == MessageBox("This action will set each detail in this topic to hide its title when printing the preview. This cannot be undone except by individually marking details to show the title. Do you want to continue?", NULL, MB_YESNO)) {
						for (int i = 0; i < pTopic->GetEMNDetailCount(); i++) {
							CEMNDetail* pDetail = pTopic->GetDetailByIndex(i);

							// only refresh the parent if the last one
							pDetail->SetPreviewFlags(pDetail->GetPreviewFlags() | epfHideTitle, (i == (pTopic->GetEMNDetailCount() - 1)));
						}
					}
				}
			}
		}	
	} NxCatchAll("Error attempting to set all detail title preview flags");
}

// (a.walling 2008-07-03 13:48) - PLID 30498 - Handle request to release write access
LRESULT CEmrTreeWnd::OnForceAccessRequest(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2008-07-07 16:50) - PLID 30498 - If only there were a better way. This will
		// still not catch all situations, such as when tracking a popup menu or performing a drag-
		// drop, but this should at least be safe enough for the hopefully rare situation of forcing
		// a save.
		BOOL bModal = FALSE;

		if (GetEmrFrameWnd() != NULL && GetEmrFrameWnd()->IsWindowEnabled()) {
			bModal = FALSE;
		} else {
			bModal = TRUE;
		}

		if (bModal) {
			m_mapWaitingOnModalLoop.SetAt(wParam, TRUE);
#ifdef _DEBUG
			TRACE("Modal loop detected -- waiting for interactive state\n");
#endif

			SetTimer(IDT_FORCE_ACCESS_TIMER, 2500, NULL);
		} else {
			m_mapWaitingOnModalLoop.RemoveKey(wParam);
			CEMN* pEMN = GetEMNByID(wParam);

			if (pEMN && pEMN->IsWritable()) {
				// OK, we have the EMN, and we are holding it.
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTree->GetFirstRow();

				while (pRow) {
					if (VarLong(pRow->GetValue(TREE_COLUMN_ROW_TYPE)) == (long)etrtEmn) {
						CEMN* pFindingEMN = EmrTree::ChildRow(pRow).GetEMN();
						if (pFindingEMN) {
							if (pFindingEMN == pEMN) {
								// we've found it!
								break;
							}
						}
					}

					pRow = pRow->GetNextRow();
				}

				if (pRow) {
					// we have the EMN row, so now let us save and then release if we can.
					SaveRowObject(pRow, TRUE);

					// now we must release write access if possible.
					m_mapAutoWriteAccessTried.SetAt(pEMN->GetID(), TRUE);
					pEMN->ReleaseWriteToken();

					BOOL bSaved = pEMN->IsUnsaved() ? FALSE : TRUE;

					if (bSaved) {
						MessageBox(FormatString("Another user forced write access from you. Your EMN '%s' has been saved and your write access released.", pEMN->GetDescription()), NULL, MB_ICONEXCLAMATION);
					} else {
						MessageBox(FormatString("Another user forced write access from you. Your write access has been released, but the EMN '%s' was unable to save.", pEMN->GetDescription()), NULL, MB_ICONEXCLAMATION);
					}
				}
			}
		}
	} NxCatchAll("Error in HandleForceAccessRequest");

	return 0;
}

// (a.walling 2008-07-07 11:34) - PLID 30496 - Get the current displayed / selected EMN
CEMN* CEmrTreeWnd::GetCurrentEMN()
{
	return GetActiveEMN();
}

// (c.haag 2008-07-07 13:40) - PLID 30607 - Posted from a CEMN object to the interface
// window when an EMN todo alarm is added to an EMN
// WPARAM - Pointer to EMN object
// LPARAM - Todo ID
LRESULT CEmrTreeWnd::OnEmnTodoAdded(WPARAM wParam, LPARAM lParam)
{
	try {
		CEMN* pEMN = (CEMN*)wParam;
		long nTaskID = (long)lParam;
		if (NULL != pEMN) {
			// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
			CEMNMoreInfoDlg *pdlgMoreInfo = GetMoreInfoDlg(pEMN, FALSE);
			if (NULL != pdlgMoreInfo) {
				pdlgMoreInfo->RefreshTodoListItem(nTaskID);
			} else {
				// The more info topic hasn't been created yet
			}
			// (c.haag 2008-07-14 16:37) - PLID 30696 - Make sure the EMN icon is up to date
			IRowSettingsPtr pEmnRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pEMN, NULL, FALSE);
			if(pEmnRow) {
				EnsureModifiedState(pEmnRow);
			}

		} else {
			// This should never happen
		}
	}
	NxCatchAll("Error in CEmrTreeWnd::OnEmnTodoAdded");
	return 0;
}

// (c.haag 2008-07-07 13:40) - PLID 30607 - Posted from a CEMN object to the interface
// window when an EMN todo alarm is deleted from an EMN
// WPARAM - Pointer to EMN object
// LPARAM - Todo ID
LRESULT CEmrTreeWnd::OnEmnTodoDeleted(WPARAM wParam, LPARAM lParam)
{
	try {
		CEMN* pEMN = (CEMN*)wParam;
		long nTaskID = (long)lParam;
		if (NULL != pEMN) {
			// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
			CEMNMoreInfoDlg *pdlgMoreInfo = GetMoreInfoDlg(pEMN, FALSE);
			if (NULL != pdlgMoreInfo) {
				pdlgMoreInfo->RemoveTodoListItem(nTaskID);
			} else {
				// The more info topic hasn't been created yet
			}
			// (c.haag 2008-07-14 16:37) - PLID 30696 - Make sure the EMN icon is up to date
			IRowSettingsPtr pEmnRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pEMN, NULL, FALSE);
			if(pEmnRow) {
				EnsureModifiedState(pEmnRow);
			}
		} else {
			// This should never happen
		}
	}
	NxCatchAll("Error in CEmrTreeWnd::OnEmnTodoDeleted");
	return 0;
}

// (c.haag 2008-07-09 10:01) - PLID 30607 - Posted from a CEMN detail to the interface
// window to refresh the todo list
LRESULT CEmrTreeWnd::OnEmnTodoRefreshList(WPARAM wParam, LPARAM lParam)
{
	try {
		CEMN* pEMN = (CEMN*)wParam;
		long nTaskID = (long)lParam;
		if (NULL != pEMN) {
			// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
			CEMNMoreInfoDlg *pdlgMoreInfo = GetMoreInfoDlg(pEMN, FALSE);
			if (NULL != pdlgMoreInfo) {
				pdlgMoreInfo->RefreshTodoList();
			} else {
				// The more info topic hasn't been created yet
			}
		} else {
			// This should never happen
		}
	}
	NxCatchAll("Error in CEmrTreeWnd::OnEmnTodoRefreshList");
	return 0;
}

// (j.jones 2008-07-23 15:00) - PLID 30789 - added ability to add EMN problems
void CEmrTreeWnd::OnAddEmnProblem()
{
	try {
		// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
		GetEmrFrameWnd()->PostMessage(WM_COMMAND, ID_EMR_ADD_NEW_PROBLEM_TO_EMN);
	}NxCatchAll("Error in CEmrTreeWnd::OnAddEmnProblem");
}

// (j.jones 2008-07-23 15:00) - PLID 30789 - added ability to edit EMN problems
void CEmrTreeWnd::OnEditEmnProblem()
{
	try {

		IRowSettingsPtr pRow = m_pTree->CurSel;

		if (pRow) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();

			if (etrt == etrtEmn) {
				CEMN* pEMN = EmrTree::ChildRow(pRow).GetEMN();

				// (c.haag 2009-09-11 10:27) - PLID 35077 - Edit the problems
				EditEmnProblem(pEMN);
			}
		}

	}NxCatchAll("Error in CEmrTreeWnd::OnEditEmnProblem");
}

// (c.haag 2009-09-11 10:23) - PLID 35077 - This function will edit a problem to an EMN (moved from OnEditEmnProblem)
void CEmrTreeWnd::EditEmnProblem(CEMN* pEMN)
{
	try {
		if(pEMN == NULL) {
			ThrowNxException("Edit Emn Problem - failed because no EMN object was found!");
		}

		// (c.haag 2009-05-26 11:47) - PLID 34312 - Use the new EMR problem linking structure
		if(pEMN->m_apEmrProblemLinks.GetSize() == 0) {
			MessageBox("This EMN has no problems.");
			return;
		}

		//close the current problem list, if there is one
		CMainFrame *pFrame = GetMainFrame();
		if(pFrame) {
			pFrame->SendMessage(NXM_EMR_DESTROY_PROBLEM_LIST);
		}

		//now open filtered on this diagnosis
		CEMRProblemListDlg dlg(this);
		dlg.SetDefaultFilter(pEMN->GetParentEMR()->GetPatientID(), eprtEmrEMN, pEMN->GetID(), pEMN->GetDescription());
		// (c.haag 2009-05-26 11:47) - PLID 34312 - Use the new EMR problem linking structure
		dlg.LoadFromProblemList(this, &pEMN->m_apEmrProblemLinks);
		dlg.DoModal();

		//see if any problem changed, if so, mark the EMN as changed
		if(pEMN->HasChangedProblems() && !pEMN->IsLockedAndSaved()) {

			pEMN->SetUnsaved();
		}

		// (j.jones 2008-07-24 08:35) - PLID 30729 - refresh the EMR problem icon,
		// because anything could have changed
		PostMessage(NXM_EMR_PROBLEM_CHANGED);

		// (a.walling 2008-07-23 16:08) - PLID 30790 - Ensure the EMN icon is up to date
		// (c.haag 2009-09-11 10:25) - PLID 35077 - Find the row
		IRowSettingsPtr pRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pEMN, NULL, FALSE);
		if(pRow != NULL) {
			EnsureModifiedState(pRow);
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2009-05-28 10:36) - PLID 34249 - Link existing problems with the EMN
void CEmrTreeWnd::OnLinkEmnProblems()
{
	try {
		// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
		GetEmrFrameWnd()->PostMessage(WM_COMMAND, ID_EMR_LINK_PROBLEM_TO_EMN);
	}
	NxCatchAll("Error in CEmrTreeWnd::OnLinkEmnProblems");
}

// (j.jones 2008-07-23 15:00) - PLID 30789 - added ability to add topic problems
void CEmrTreeWnd::OnAddTopicProblem()
{
	try {
		// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
		GetEmrFrameWnd()->PostMessage(WM_COMMAND, ID_EMR_ADD_NEW_PROBLEM_TO_TOPIC);
	}NxCatchAll("Error in CEmrTreeWnd::OnAddTopicProblem");
}

// (j.jones 2008-07-23 15:00) - PLID 30789 - added ability to edit topic problems
void CEmrTreeWnd::OnEditTopicProblem()
{
	try {

		IRowSettingsPtr pRow = m_pTree->CurSel;

		if (pRow) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();

			if (etrt == etrtTopic) {
				CEMRTopic* pTopic = EmrTree::ChildRow(pRow).GetTopic();

				if(pTopic == NULL) {
					ThrowNxException("View Topic Problems - failed because no Topic object was found!");
				}

				// (c.haag 2009-05-26 11:47) - PLID 34312 - Use the new EMR problem linking structure
				if(pTopic->m_apEmrProblemLinks.GetSize() == 0) {
					MessageBox("This Topic has no problems.");
					return;
				}

				//close the current problem list, if there is one
				CMainFrame *pFrame = GetMainFrame();
				if(pFrame) {
					pFrame->SendMessage(NXM_EMR_DESTROY_PROBLEM_LIST);
				}

				//now open filtered on this diagnosis
				CEMRProblemListDlg dlg(this);
				dlg.SetDefaultFilter(pTopic->GetParentEMN()->GetParentEMR()->GetPatientID(), eprtEmrTopic, pTopic->GetID(), pTopic->GetName());
				// (c.haag 2009-05-26 11:47) - PLID 34312 - Use the new EMR problem linking structure
				dlg.LoadFromProblemList(this, &pTopic->m_apEmrProblemLinks);
				dlg.DoModal();

				//also see if any problem changed, if so, mark the EMN as changed
				if(pTopic->HasChangedProblems() && !pTopic->GetParentEMN()->IsLockedAndSaved()) {
					pTopic->SetUnsaved();
				}

				// (j.jones 2008-07-24 08:35) - PLID 30729 - refresh the EMR problem icon,
				// because anything could have changed
				PostMessage(NXM_EMR_PROBLEM_CHANGED);

				// (a.walling 2008-07-23 16:06) - PLID 30790 - Ensure the topic row icon is up to date
				EnsureModifiedState(pRow);
			}
		}

	}NxCatchAll("Error in CEmrTreeWnd::OnEditTopicProblem");
}

// (c.haag 2009-06-01 12:59) - PLID 34249 - This function lets a user link an object in the tree window with an EMR problem
void CEmrTreeWnd::OnLinkTopicProblems()
{
	try {
		// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
		GetEmrFrameWnd()->PostMessage(WM_COMMAND, ID_EMR_LINK_PROBLEM_TO_TOPIC);
	}
	NxCatchAll("Error in CEmrTreeWnd::OnLinkTopicProblems");
}

// (j.jones 2008-07-24 11:16) - PLID 30729 - added OnEmrProblemChanged
LRESULT CEmrTreeWnd::OnEmrProblemChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		//TES 10/30/2008 - PLID 31269 - Now that any problem icons we own have been updated, tell all our child windows
		// to update any problem icons they own as well.
		FOR_ALL_ROWS(m_pTree) {
			EmrTreeRowType etrt = (EmrTreeRowType)VarLong(pRow->GetValue(TREE_COLUMN_ROW_TYPE));
			if(etrt == etrtTopic) {
				CEmrTopicWndPtr pWnd = EmrTree::ChildRow(pRow).GetTopicWnd();
				if(pWnd) {
					ASSERT(IsWindow(pWnd->GetSafeHwnd()));
					pWnd->HandleProblemChange((CEmrProblem*)wParam);
				}
				// (z.manning 2011-11-15 08:55) - PLID 46231 - Call this to ensure the topic row's icon is updated.
				EnsureModifiedState(pRow);
			}
			else if(etrt == etrtMoreInfo) {
				// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
				CEMNMoreInfoDlg *pDlg = GetEmrFrameWnd()->GetMoreInfoDlg(GetEMNFromRow(pRow));
				if(pDlg) {
					ASSERT(IsWindow(pDlg->GetSafeHwnd()));
					pDlg->HandleProblemChange((CEmrProblem*)wParam);
				}
			}
			else if(etrt == etrtCodes) {
				// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
				CEmrCodesDlg *pDlg = GetEmrFrameWnd()->GetEmrCodesDlg(GetEMNFromRow(pRow));
				if(pDlg) {
					ASSERT(IsWindow(pDlg->GetSafeHwnd()));
					pDlg->HandleProblemChange((CEmrProblem*)wParam);
				}
			}
			else if(etrt == etrtEmn) {
				EnsureModifiedState(pRow);
			}
			GET_NEXT_ROW(m_pTree)
		}

	}NxCatchAll("Error in CEmrTreeWnd::OnEmrProblemChanged");
	
	return 0;
}

// (j.jones 2008-07-28 10:54) - PLID 30773 - added OnEmrTopicChanged
LRESULT CEmrTreeWnd::OnEmrTopicChanged(WPARAM wParam, LPARAM lParam)
{
	try {

		CEMRTopic *pTopic = (CEMRTopic*)wParam;

		if(pTopic) {

			//don't change the saved state if locked
			if (pTopic->GetParentEMN()->GetStatus() != 2) {
				pTopic->SetUnsaved();
			}
			
			IRowSettingsPtr pTopicRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pTopic, NULL, FALSE);
			if(pTopicRow) {
				EnsureModifiedState(pTopicRow);
			}
		}	

	}NxCatchAll("Error in CEmrTreeWnd::OnEmrTopicChanged");
	
	return 0;
}

//(e.lally 2008-07-29) PLID 30732 - Tell our parent, the PIC, to minimize
LRESULT CEmrTreeWnd::OnEmrMinimizePic(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
		if (GetPicContainer()) {
			//Forward it on
			GetPicContainer()->SendMessage(NXM_EMR_MINIMIZE_PIC, wParam, lParam);
		}
	}NxCatchAll("Error in CEmrTreeWnd::OnEmrMinimizePic");	
	return 0;
}

//(e.lally 2008-07-30) PLID 30732 - Save everything
LRESULT CEmrTreeWnd::OnMessageEmrSaveAll(WPARAM wParam, LPARAM lParam)
{
	try {
		BOOL bShowProgressBar = TRUE;
		if(wParam){
			bShowProgressBar = (BOOL)wParam;
		}
		return SaveEMR(esotEMR, -1, bShowProgressBar);
	}NxCatchAll("Error in CEmrTreeWnd::OnMessageEmrSaveAll");	
	return 0;
}

// (a.walling 2008-08-15 13:22) - PLID 22049 - Helper function to get the EMN of a row
CEMN* CEmrTreeWnd::GetEMNFromRow(NXDATALIST2Lib::IRowSettings* pRow)
{
	NXDATALIST2Lib::IRowSettingsPtr pRowPtr(pRow);
	CEMN* pEMN = NULL;

	while (pRowPtr) {
		EmrTreeRowType etrt = EmrTree::ChildRow(pRowPtr).GetType();
		if(etrt == etrtEmn) {
			pEMN = EmrTree::ChildRow(pRowPtr).GetEMN();
			break;
		}

		pRowPtr = pRowPtr->GetParentRow();
	}

	return pEMN;
}

// (a.walling 2008-12-30 17:04) - PLID 30252 - Change the EMN date from the treewnd
void CEmrTreeWnd::OnChangeEmnDate()
{
	try {
		IRowSettingsPtr pEmnRow = m_pTree->CurSel;
		if (pEmnRow) {
			CEMN* pCurEMN = GetEMNFromRow(pEmnRow);

			if (pCurEMN) {
				
				
				COleDateTime dtOld, dtNew;

				dtOld = pCurEMN->GetEMNDate();
				
				dtOld.SetDate(dtOld.GetYear(), dtOld.GetMonth(), dtOld.GetDay());

				CChooseDateDlg dlg(this);
				dtNew = dlg.Open(dtOld);
				
				if (COleDateTime::valid != dtNew.GetStatus())
					return;

				dtNew.SetDate(dtNew.GetYear(), dtNew.GetMonth(), dtNew.GetDay());

				if (dtNew != dtOld) {
					// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
					//TES 8/24/2012 - PLID 52238 - No need to force the dialog to be created
					CEMNMoreInfoDlg* pMoreInfo = GetMoreInfoDlg(pCurEMN, FALSE);
					if (pMoreInfo) {
						pMoreInfo->SetDate(dtNew, TRUE);
					}
					else {
						pCurEMN->SetEMNDate(dtNew);
					}
				}
			}
		}

	} NxCatchAll("CEmrTreeWnd::OnChangeEmnDate");
}

// (j.jones 2009-09-23 16:49) - PLID 29718 - added multiple options for changing dates
void CEmrTreeWnd::OnSetEmnDateToday()
{
	try {
		IRowSettingsPtr pEmnRow = m_pTree->CurSel;
		if (pEmnRow) {
			CEMN* pCurEMN = GetEMNFromRow(pEmnRow);

			if (pCurEMN) {
				
				
				COleDateTime dtOld, dtNew;

				dtOld = pCurEMN->GetEMNDate();
				
				dtOld.SetDate(dtOld.GetYear(), dtOld.GetMonth(), dtOld.GetDay());

				dtNew = COleDateTime::GetCurrentTime();

				dtNew.SetDate(dtNew.GetYear(), dtNew.GetMonth(), dtNew.GetDay());

				if (dtNew != dtOld) {
					// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
					//TES 8/24/2012 - PLID 52238 - No need to force the dialog to be created
					CEMNMoreInfoDlg* pMoreInfo = GetMoreInfoDlg(pCurEMN, FALSE);
					if (pMoreInfo) {
						pMoreInfo->SetDate(dtNew, TRUE);
					}
					else {
						pCurEMN->SetEMNDate(dtNew);
					}
				}
			}
		}

	} NxCatchAll("CEmrTreeWnd::OnSetEmnDateToday");
}

// (j.jones 2009-09-23 16:49) - PLID 29718 - added multiple options for changing dates
void CEmrTreeWnd::OnSetEmnDateLastAppt()
{
	try {
		IRowSettingsPtr pEmnRow = m_pTree->CurSel;
		if (pEmnRow) {
			CEMN* pCurEMN = GetEMNFromRow(pEmnRow);

			if (pCurEMN) {
				
				COleDateTime dtOld, dtNew;

				dtOld = pCurEMN->GetEMNDate();

				//find the last non-cancelled appointment, that is not in the future
				dtNew = GetLastPatientAppointmentDate(pCurEMN->GetParentEMR()->GetPatientID());

				//GetLastPatientAppointmentDate will return an invalid date if no appt. exists
				if(dtNew.GetStatus() == COleDateTime::invalid) {
					MessageBox("This patient has no appointments prior to today.", "Practice", MB_ICONINFORMATION|MB_YESNO);
					return;
				}

				dtNew.SetDate(dtNew.GetYear(), dtNew.GetMonth(), dtNew.GetDay());

				if (dtNew != dtOld) {
					// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
					//TES 8/24/2012 - PLID 52238 - No need to force the dialog to be created
					CEMNMoreInfoDlg* pMoreInfo = GetMoreInfoDlg(pCurEMN, FALSE);
					if (pMoreInfo) {					
						pMoreInfo->SetDate(dtNew, TRUE);
					}
					else {
						pCurEMN->SetEMNDate(dtNew);
					}
				}
			}
		}

	} NxCatchAll("CEmrTreeWnd::OnSetEmnDateLastAppt");
}

// (a.walling 2009-01-08 13:46) - PLID 32659 - Clear/float options for topic elements
void CEmrTreeWnd::OnTopicColumnOne()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pTree->CurSel;
		if(pCurSel) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pCurSel).GetType();
			if(etrt == etrtTopic) {
				CEMRTopic* pTopic = EmrTree::ChildRow(pCurSel).GetTopic();
				if(pTopic) {
					DWORD nPreviewFlags = pTopic->GetPreviewFlags();

					// (a.walling 2009-07-06 10:14) - PLID 34793 - epfFloatLeft is now epfColumnOne, epfFloatRight is now epfColumnTwo
					nPreviewFlags &= ~epfColumnTwo;
					nPreviewFlags ^= epfColumnOne;
		
					// (a.walling 2009-07-06 11:29) - PLID 34793
					pTopic->SetPreviewFlags(nPreviewFlags, TRUE);
				}
			}
		}	
	} NxCatchAll("Error attempting to set topic column one preview flags");
}

// (a.walling 2009-01-08 13:46) - PLID 32659 - Clear/float options for topic elements
void CEmrTreeWnd::OnTopicColumnTwo()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pTree->CurSel;
		if(pCurSel) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pCurSel).GetType();
			if(etrt == etrtTopic) {
				CEMRTopic* pTopic = EmrTree::ChildRow(pCurSel).GetTopic();
				if(pTopic) {
					DWORD nPreviewFlags = pTopic->GetPreviewFlags();

					// (a.walling 2009-07-06 10:14) - PLID 34793 - epfFloatLeft is now epfColumnOne, epfFloatRight is now epfColumnTwo
					nPreviewFlags &= ~epfColumnOne;
					nPreviewFlags ^= epfColumnTwo;

					// (a.walling 2009-07-06 11:29) - PLID 34793
					pTopic->SetPreviewFlags(nPreviewFlags, TRUE);
				}
			}
		}	
	} NxCatchAll("Error attempting to set topic column two preview flags");
}

// (a.walling 2009-07-06 12:35) - PLID 34793 - Grouping for columns
void CEmrTreeWnd::OnTopicGroupBegin()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pTree->CurSel;
		if(pCurSel) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pCurSel).GetType();
			if(etrt == etrtTopic) {
				CEMRTopic* pTopic = EmrTree::ChildRow(pCurSel).GetTopic();
				if(pTopic) {
					DWORD nPreviewFlags = pTopic->GetPreviewFlags();

					// turn off group end if it is on
					nPreviewFlags &= ~epfGroupEnd;
					
					// toggle group begin
					nPreviewFlags ^= epfGroupBegin;

					pTopic->SetPreviewFlags(nPreviewFlags);
				}
			}
		}	
	} NxCatchAll("Error attempting to set topic group begin preview flags");
}

// (a.walling 2009-07-06 12:35) - PLID 34793 - Grouping for columns
void CEmrTreeWnd::OnTopicGroupEnd()
{
	try {		
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pTree->CurSel;
		if(pCurSel) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pCurSel).GetType();
			if(etrt == etrtTopic) {
				CEMRTopic* pTopic = EmrTree::ChildRow(pCurSel).GetTopic();
				if(pTopic) {
					DWORD nPreviewFlags = pTopic->GetPreviewFlags();

					// turn off group begin if it is on
					nPreviewFlags &= ~epfGroupBegin;
					
					// toggle group end
					nPreviewFlags ^= epfGroupEnd;

					pTopic->SetPreviewFlags(nPreviewFlags);
				}
			}
		}	
	} NxCatchAll("Error attempting to set topic group end preview flags");
}

// (a.walling 2009-01-08 14:11) - PLID 32660 - Align text right
void CEmrTreeWnd::OnTopicTextRight()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pTree->CurSel;
		if(pCurSel) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pCurSel).GetType();
			if(etrt == etrtTopic) {
				CEMRTopic* pTopic = EmrTree::ChildRow(pCurSel).GetTopic();
				if(pTopic) {
					DWORD nPreviewFlags = pTopic->GetPreviewFlags();

					nPreviewFlags ^= epfTextRight;
					pTopic->SetPreviewFlags(nPreviewFlags);
				}
			}
		}	
	} NxCatchAll("Error attempting to set topic clear right preview flags");
}

// (a.walling 2010-08-31 18:20) - PLID 36148 - Page breaks
void CEmrTreeWnd::OnTopicNewPageBefore()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pTree->CurSel;
		if(pCurSel) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pCurSel).GetType();
			if(etrt == etrtTopic) {
				CEMRTopic* pTopic = EmrTree::ChildRow(pCurSel).GetTopic();
				if(pTopic) {
					DWORD nPreviewFlags = pTopic->GetPreviewFlags();

					nPreviewFlags &= ~epfPageBreakAfter;
					nPreviewFlags ^= epfPageBreakBefore;
					pTopic->SetPreviewFlags(nPreviewFlags);
				}
			}
		}	
	} NxCatchAll("Error attempting to set new page before preview flags");
}

// (a.walling 2010-08-31 18:20) - PLID 36148 - Page breaks
void CEmrTreeWnd::OnTopicNewPageAfter()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pTree->CurSel;
		if(pCurSel) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pCurSel).GetType();
			if(etrt == etrtTopic) {
				CEMRTopic* pTopic = EmrTree::ChildRow(pCurSel).GetTopic();
				if(pTopic) {
					DWORD nPreviewFlags = pTopic->GetPreviewFlags();

					nPreviewFlags &= ~epfPageBreakBefore;
					nPreviewFlags ^= epfPageBreakAfter;
					pTopic->SetPreviewFlags(nPreviewFlags);
				}
			}
		}	
	} NxCatchAll("Error attempting to set new page after preview flags");
}

// (j.jones 2009-03-03 12:06) - PLID 33308 - added E-Prescribing
void CEmrTreeWnd::OnEPrescribing()
{
	try {

		if(m_bIsTemplate) {
			//This should never happen!
			ASSERT(FALSE);
			MsgBox("This feature is unavailable when editing a template.");
			return;
		}

		// (j.jones 2012-10-19 15:46) - PLID 52818 - This function has now been split up to have different processes for
		// NewCrop versus SureScripts, with SureScripts being "NexTech E-Prescribing".
		CLicense::EPrescribingType eRxType = g_pLicense->HasEPrescribing(CLicense::cflrUse);

		//Since this is a Use call, if they have the license but can't use it (expired, etc.)
		//then the return will have been eptNone. A warning would have been given.
		if(eRxType == CLicense::eptNewCrop) {
			EPrescribeWithNewCrop();
		}
		else {
			// (j.jones 2013-02-12 13:44) - PLID 54817 - This function is now called from More Info in order
			// to add new medications, which means to just open the queue. This is now supported regardless
			// of whether they have our SureScripts license. The queue will disable SureScripts-specific abilities.
			EPrescribeWithSureScripts();
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-10-19 15:46) - PLID 52818 - I split up E-Prescribing functionality into NewCrop vs. SureScripts,
//with SureScripts being "NexTech E-Prescribing". The old OnEPrescribing() was formerly NewCrop-only, its contents
//have been moved to this function.
void CEmrTreeWnd::EPrescribeWithNewCrop()
{
	try {

		//the caller should have checked licensing prior to calling this function

		// (j.gruber 2009-03-31 11:54) - PLID 33328 - make sure they have the correct role		
		// (j.gruber 2009-06-08 10:33) - PLID 34515 - added role
		NewCropUserTypes ncuTypeID;
		if (!CheckEPrescribingStatus(ncuTypeID, GetCurrentUserID(), GetCurrentUserName())) {
			//this pops up the message for us
			return;
		}

		long nPatientID = m_EMR.GetPatientID();

		if(nPatientID == -1) {
			//this should be impossible
			ASSERT(FALSE);
			AfxMessageBox("E-Prescribing is only available when accessing a patient account.");
			return;
		}

		//we have to add it to the current EMN
		IRowSettingsPtr pEmnRow = m_pTree->CurSel;
		CEMN* pCurEMN = NULL;
		if (pEmnRow) {
			pCurEMN = GetEMNFromRow(pEmnRow);
		}

		if(pCurEMN == NULL) {
			AfxMessageBox("You must have an EMN selected in order to access E-Prescribing.");
			return;
		}

		//force saving first, if anything on the EMR is not saved
		if(IsEMRUnsaved()) {
			
			if(IDNO == MessageBox("This EMR will be saved prior to accessing E-Prescribing.\n"
				"Are you sure you wish to continue?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}

			//save now
			if(SaveEMR(esotEMR, -1, TRUE) != essSuccess) {
				AfxMessageBox("The save did not complete, E-Prescribing will be cancelled.");
				return;
			}
		}

		long nEMNID = pCurEMN->GetID();
		ASSERT(nEMNID != -1);

		//if locked, warn about the limitations of this
		if(pCurEMN->IsLockedAndSaved()) {
			if(IDNO == MessageBox("The current EMN is locked. If you continue, any electronic prescriptions you create "
				"will be linked only with the current patient, and not the current EMN.\n"
				"Are you sure you wish to continue?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
			else {
				//send -2, which will identify this EMN as locked
				nEMNID = -2;
			}
		}
		// (j.jones 2012-01-03 08:48) - PLID 47275 - add a separate warning if read-only
		else if(!pCurEMN->IsWritable()) {
			if(IDNO == MessageBox("The current EMN is read-only. If you continue, any electronic prescriptions you create "
				"will be linked only with the current patient, and not the current EMN.\n"
				"Are you sure you wish to continue?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
			else {
				//send -2, which will identify this EMN as read-only
				nEMNID = -2;
			}
		}

		CString strWindowDescription;
		strWindowDescription.Format("Accessing Electronic Prescription Account for Patient: %s", GetExistingPatientName(nPatientID));
		
		// (j.jones 2013-01-04 08:52) - PLID 49624 - removed role, and added UserDefinedID
		long nUserDefinedID = GetExistingPatientUserDefinedID(nPatientID);
		GetMainFrame()->OpenNewCropBrowser(strWindowDescription, ncatAccessPatientAccount, nPatientID, nPatientID, nUserDefinedID, nEMNID, this, "");

		// (j.jones 2012-09-26 10:42) - PLID 52820 - This would check drug interactions, but we concluded it is
		// not needed when using NewCrop, as NewCrop handles this itself. It would be needed when we switch to
		// non-NewCrop usage.
		/*
		{
			// Show the EMR's drug interaction dialog, not the mainframe version.
			CPicContainerDlg *p = GetPicContainer();
			if(p) {
				//This will silently show nothing if there are no interactions,
				//and also silently do nothing if they are not licensed for FirstDataBank.
				p->ViewDrugInteractions();
			}
		}
		*/

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-10-19 15:46) - PLID 52818 - Created this function to handle NexTech's non-NewCrop E-Prescribing.
void CEmrTreeWnd::EPrescribeWithSureScripts()
{
	try {

		//the caller should have checked licensing prior to calling this function,
		//so this is just a silent call
		CLicense::EPrescribingType eRxType = g_pLicense->HasEPrescribing(CLicense::cflrSilent);
		if(eRxType == CLicense::eptNewCrop) {
			ASSERT(FALSE);
			//switch to the NewCrop function
			EPrescribeWithNewCrop();
			return;
		}

		// (j.jones 2012-11-20 12:46) - PLID 53818 - this should be impossible on a template
		if(m_bIsTemplate) {
			ThrowNxException("Prescription queue attempted to load from a template.");
		}

		long nPatientID = m_EMR.GetPatientID();

		if(nPatientID == -1) {
			//this should be impossible
			ASSERT(FALSE);
			if(eRxType == CLicense::eptNone) {
				//don't use the word E-Prescribing
				AfxMessageBox("Prescription writing is only available when accessing a patient account.");
			}
			else {
				AfxMessageBox("E-Prescribing is only available when accessing a patient account.");
			}
			return;
		}

		//we have to add it to the current EMN
		IRowSettingsPtr pEmnRow = m_pTree->CurSel;
		CEMN* pCurEMN = NULL;
		if (pEmnRow) {
			pCurEMN = GetEMNFromRow(pEmnRow);
		}

		if(pCurEMN == NULL) {
			if(eRxType == CLicense::eptNone) {
				//don't use the word E-Prescribing
				AfxMessageBox("You must have an EMN selected in order to write prescriptions.");
			}
			else {
				AfxMessageBox("You must have an EMN selected in order to access E-Prescribing.");
			}
			return;
		}

		//force saving first, if anything on the EMR is not saved
		if(IsEMRUnsaved()) {
			
			CString strWarning;
			if(eRxType == CLicense::eptNone) {
				//don't use the word E-Prescribing
				strWarning = "This EMR will be saved prior to writing prescriptions.\n"
					"Are you sure you wish to continue?";
			}
			else {
				strWarning = "This EMR will be saved prior to accessing E-Prescribing.\n"
					"Are you sure you wish to continue?";
			}		

			if(IDNO == MessageBox(strWarning, "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}

			//save now
			if(SaveEMR(esotEMR, -1, TRUE) != essSuccess) {
				if(eRxType == CLicense::eptNone) {
					//don't use the word E-Prescribing
					AfxMessageBox("The save did not complete, no prescriptions will be written.");
				}
				else {
					AfxMessageBox("The save did not complete, E-Prescribing will be cancelled.");
				}
				return;
			}
		}

		long nEMNID = pCurEMN->GetID();
		ASSERT(nEMNID != -1);

		//if locked, warn about the limitations of this
		if(pCurEMN->IsLockedAndSaved()) {
			if(IDNO == MessageBox("The current EMN is locked. If you continue, any electronic prescriptions you create "
				"will be linked only with the current patient, and not the current EMN.\n"
				"Are you sure you wish to continue?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
			else {
				//send -1
				nEMNID = -1;
			}
		}
		// (j.jones 2012-01-03 08:48) - PLID 47275 - add a separate warning if read-only
		else if(!pCurEMN->IsWritable()) {
			if(IDNO == MessageBox("The current EMN is read-only. If you continue, any electronic prescriptions you create "
				"will be linked only with the current patient, and not the current EMN.\n"
				"Are you sure you wish to continue?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
			else {
				//send -1
				nEMNID = -1;
			}
		}
		// (j.jones 2012-10-19 15:57) - PLID 52820 - Check drug interactions. The queue will open on top afterwards.
		// (j.jones 2012-11-16 15:41) - PLID 52818 - removed, because the queue now shows the drug interactions itself,
		// with its own interaction dialog that uses the queue as a parent
		/*
		{
			// Show the EMR's drug interaction dialog, not the mainframe version.
			CPicContainerDlg *p = GetPicContainer();
			if(p) {
				//This will silently show nothing if there are no interactions,
				//and also silently do nothing if they are not licensed for FirstDataBank.
				p->ViewDrugInteractions();
			}
		}
		*/

		// (j.jones 2012-11-20 10:18) - PLID 52818 - we will pass in the EMN provider, location, and date,
		// so new prescriptions use the EMN data
		long nProviderID = -1;
		if(pCurEMN->GetProviderCount() > 0) {
			EMNProvider *pProv = pCurEMN->GetProvider(0);
			if(pProv) {
				nProviderID = pProv->nID;
			}
		}

		//use the EMN location
		long nLocationID = pCurEMN->GetLocationID();
		if(nLocationID == -1) {
			nLocationID = GetCurrentLocationID();
		}

		// (j.jones 2013-02-12 15:37) - PLID 55139 - get the current active allergy info ID
		long nOldAllergiesInfoID = GetActiveAllergiesInfoID();

		// (j.jones 2012-10-19 15:58) - PLID 52818 - open the prescription queue, provide the active EMN ID (-1 if read only),
		// also provide the default provider, location, and date to use on new prescriptions
		// (j.fouts 2012-11-01 16:41) - PLID 53566 - This opens the Queue, rather than edit prescription
		CPrescriptionQueueDlg dlg(this, nPatientID, nEMNID, true, false,
			nProviderID, nLocationID, pCurEMN->GetEMNDate());
		CNxModalParentDlg dlgParent(this, &dlg, CString("Prescriptions"), CRect(0,0,EMR_PRESCRIPTION_QUEUE_WIDTH,EMR_PRESCRIPTION_QUEUE_HEIGHT));
		dlgParent.DoModal();

		//reload medications for all open EMNs, but do not open interactions,
		//because the queue would have already done so
		ReloadAllEMNMedicationsFromData(FALSE);

		// (j.jones 2013-02-12 15:37) - PLID 55139 - if our EMN is updateable (would be -1 if not), reload allergies
		if(pCurEMN != NULL && nEMNID != -1) {
			pCurEMN->ReloadPatientAllergiesFromData(nOldAllergiesInfoID, GetActiveAllergiesInfoID());
		}

		// (j.jones 2013-06-05 14:37) - PLID 57049 - if we are showing meds or allergies in the preview pane,
		// reload the preview pane to refresh those fields with any changes we made have made
		long nOptions = GetRemotePropertyInt("EMRPreview_MedAllergy", 5, 0, "<None>", true);
		bool bShowMedicationsInPreview = (nOptions & maoDisplayMedications) > 0;
		bool bShowAllergiesInPreview = (nOptions & maoDisplayAllergies) > 0;
		if(bShowMedicationsInPreview || bShowAllergiesInPreview) {
			pCurEMN->GenerateHTMLFile(TRUE, FALSE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2009-04-01 10:15) - PLID 33736 - added OnNewCropBrowserClosed
LRESULT CEmrTreeWnd::OnNewCropBrowserClosed(WPARAM wParam, LPARAM lParam)
{
	BOOL bNeedToReconcileMeds = FALSE;
	CStringArray astrNewCropRxGUIDs_ToReconcile;

	try {

		//For every EMN in the EMR that currently has medications,
		//we need to reload those medications from data.
		//For the EMNID returned in the wParam, we need to reload no matter what,
		//as prescriptions may have been added to this EMN.
		// (z.manning 2011-09-22 15:24) - PLID 45623 - Note: We now only update the current EMN, not the entire EMR.
		NewCropBrowserResult* pNCBR = (NewCropBrowserResult*)wParam;

		// (j.jones 2013-01-08 09:56) - PLID 47302 - We used to open medication reconciliation inside,
		// ReloadPatientCurrentMedicationsFromData_ForNewCrop, but that has been moved so that it
		// happens here, right after the browser closes.
		// We always have to reconcile medications, but we might not always be reloading the current meds.
		// So call the MainFrame version, and then later this function might reload the current medications.

		// (j.jones 2013-01-11 13:29) - PLID 54462 - For NewCrop only, we now we have the option to not update the
		// current meds table, which poses a bit of a problem in that we can would have to reconcile after reloading
		// the current meds. This logic now needs to ensure we reconcile at the proper place.
		// (j.jones 2013-02-12 15:32) - PLID 55139 - Now we never update current meds unless it's NewCrop AND this option is turned off.
		bNeedToReconcileMeds = (GetRemotePropertyInt("ReconcileNewRxWithCurMeds", 0, 0, "<None>", true) == 1);
		BOOL bReconcileSkipEMRTable = TRUE;
		if((g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptNewCrop) && (GetRemotePropertyInt("ReconcileNewRxWithCurMeds_SkipEMRTable", 1, 0, "<None>", true) == 0)) {
			bReconcileSkipEMRTable = FALSE;
		}

		//fill our array of GUIDs for reconciliation later
		if(bNeedToReconcileMeds && pNCBR->aNewlyAddedPatientPrescriptions.GetSize() > 0) {			
			for (int i=0; i < pNCBR->aNewlyAddedPatientPrescriptions.GetSize(); i++) {
				astrNewCropRxGUIDs_ToReconcile.Add( pNCBR->aNewlyAddedPatientPrescriptions[i].strPrescriptionGUID );
			}
		}

		// (j.jones 2013-01-11 13:34) - PLID 54462 - track whether we are updating this EMN
		BOOL bProcessEMN = TRUE;

		// (j.jones 2012-01-03 09:12) - PLID 47275 - -2 is a sentinel value for locked/read-only EMNs,
		// so if -2 do not assert, as it just means we cannot update this EMN
		if(bProcessEMN && pNCBR->nEMNID == -2) {
			bProcessEMN = FALSE;
		}

		//-1 means that no EMN ID was given, which is ok, but unexpected if called
		//from an EMR screen
		if(bProcessEMN && pNCBR->nEMNID == -1) {
			ASSERT(FALSE);
			bProcessEMN = FALSE;
		}

		//if the EMN ID is not found in this EMR, that's ok, but unexpected since
		//NewCrop was presumably called from this EMR
		CEMN *pEmn = NULL;
		if(bProcessEMN) {
			pEmn = m_EMR.GetEMNByID(pNCBR->nEMNID);
			if(pEmn == NULL) {
				ASSERT(FALSE);
				bProcessEMN = FALSE;
			}
		}

		// (j.jones 2012-01-03 09:04) - PLID 47275 - we should not get here if the EMN is locked
		// or read-only, so error out if we did
		if(bProcessEMN) {
			if(pEmn->IsLockedAndSaved()) {
				ThrowNxException("Attempted to update a locked EMN!");
			}
			else if(!pEmn->IsWritable()) {
				ThrowNxException("Attempted to update a read-only EMN!");
			}

			// (a.walling 2014-08-27 18:02) - PLID 63502 - Force reload of revision when modified by API. This is a dirty hack.
			pEmn->ForceReloadRevision();

			// (j.jones 2013-01-11 13:36) - PLID 54462 - Reconcile medications if requested, and if we are
			// not skipping the current meds. table. This will save to the patient's account first, and the
			// following code will reload the current meds. table with these new changes.
			if(bNeedToReconcileMeds && !bReconcileSkipEMRTable && astrNewCropRxGUIDs_ToReconcile.GetSize() > 0) {

				//track that we no longer need to reconcile meds, because we're doing it now
				bNeedToReconcileMeds = FALSE;

				//this function takes in an optional prescription ID list, but it is not used here
				CArray<long, long> aryNewPrescriptionIDs;
				//TES 10/31/2013 - PLID 59251 - If this triggers any interventions, notify the user
				CDWordArray arNewCDSInterventions;
				ReconcileCurrentMedicationsWithMultipleNewPrescriptions(m_EMR.GetPatientID(), aryNewPrescriptionIDs, astrNewCropRxGUIDs_ToReconcile, GetSysColor(COLOR_BTNFACE), this, arNewCDSInterventions);
				GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);
			}

			// (z.manning 2011-09-22 15:24) - PLID 45623 - Only reload on the current EMN
			pEmn->ReloadMedicationsFromData(TRUE);

			// (j.jones 2009-09-18 13:12) - PLID 35599 - these functions should only be done from NewCrop usage,
			// so I renamed each of them accordingly

			// (c.haag 2009-05-13 16:15) - PLID 34256 - Make sure every active EMR allergy detail in the EMR
			// is updated with the latest content and state
			// (z.manning 2011-09-22 15:24) - PLID 45623 - Only reload on the current EMN
			pEmn->ReloadPatientAllergiesFromData(pNCBR->nOldAllergiesInfoID, pNCBR->nNewAllergiesInfoID);

			// (c.haag 2009-05-15 09:49) - PLID 34271 - Make sure every active EMR current medications detail
			// in the EMR is updated with the latest content and state
			// (c.haag 2010-02-18 11:17) - PLID 37424 - Include the entire browser result because we also need to
			// know what patient prescriptions were added by the NewCrop integration in this session.
			// (z.manning 2011-09-22 15:24) - PLID 45623 - Only reload on the current EMN
			pEmn->ReloadPatientCurrentMedicationsFromData_ForNewCrop(pNCBR);

			// (c.haag 2011-01-25) - PLID 42222 - We now force a save for two reasons: We don't want NewCropGUID's
			// hanging out in memory and contributing to shady behavior after multiple trips to NewCrop or manual changes
			// to the Current Medications detail; and to prevent the user from cancelling out of the EMR thus taking NewCrop
			// and Practice out of sync.
			// (z.manning 2011-09-22 14:56) - PLID 45623 - We now save only the current EMN rather than the entire EMR.
			if(FAILED(SaveEMR(esotEMN, (long)pEmn, TRUE))) 
			{
				// The only reason we'd have a failure here is if someone on another station took over the EMR, or maybe the
				// workstation got disconnected.
				AfxMessageBox("The automatic EMR save did not complete. Please save this EMR to ensure that it is updated with relevant changes made in NewCrop.", MB_ICONERROR | MB_OK);
			}
		}

		// (c.haag 2009-05-13 16:20) - PLID 34256 - Needs to be deleted
		delete pNCBR;

	}NxCatchAll("Error in CEmrTreeWnd::OnNewCropBrowserClosed");

	try {

		// (j.jones 2013-01-11 13:36) - PLID 54462 - If we were supposed to reconcile medications,
		// and did not do so earlier in this function, do it now. Likely causes are:
		// - the EMN was not writable
		// - the EMN has no blue Current Medications table
		// - the ReconcileNewRxWithCurMeds_SkipEMRTable preference is set to not update this EMNs blue table
		// If we get here, we're only updating the patient's account, and not this EMN's blue table.
		if(bNeedToReconcileMeds && astrNewCropRxGUIDs_ToReconcile.GetSize() > 0) {

			//track that we no longer need to reconcile meds, because we're doing it now
			bNeedToReconcileMeds = FALSE;

			//this function takes in an optional prescription ID list, but it is not used here
			CArray<long, long> aryNewPrescriptionIDs;
			CDWordArray arNewCDSInterventions;
			//TES 10/31/2013 - PLID 59251 - If this triggers any interventions, notify the user
			ReconcileCurrentMedicationsWithMultipleNewPrescriptions(m_EMR.GetPatientID(), aryNewPrescriptionIDs, astrNewCropRxGUIDs_ToReconcile, GetSysColor(COLOR_BTNFACE), this, arNewCDSInterventions);
			GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);
		}

	}NxCatchAll("Error in CEmrTreeWnd::OnNewCropBrowserClosed - Medication Reconciliation");

	return 0;
}

// (z.manning 2009-08-26 10:01) - PLID 33911 - Added message to insert signature
// (a.walling 2009-10-29 09:37) - PLID 36089 - Made InsertSignature generic
LRESULT CEmrTreeWnd::OnInsertStockEmrItem(WPARAM wParam, LPARAM lParam)
{
	try
	{
		CEMRTopic *pTopic = (CEMRTopic*)wParam;
		if(pTopic == NULL) {
			// (z.manning 2009-08-26 10:37) - Called should have passed in a topic
			ASSERT(FALSE);
			return 0;
		}

		IRowSettingsPtr pRow = m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, (long)pTopic, NULL, FALSE);
		// (a.walling 2012-03-14 12:55) - PLID 46078 - Always ensure the topic row until we remove all dependencies on the topic wnd
		EnsureTopicRow(pRow);
		
		// (a.walling 2009-10-29 09:37) - PLID 36089
		// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicitly get the topic wnd
		pTopic->GetTopicWnd()->SendMessage(NXM_INSERT_STOCK_EMR_ITEM, wParam, lParam);

	}NxCatchAll(__FUNCTION__);
	return 0;
}

void CEmrTreeWnd::OnPositionTopics()
{
	try {
		//TES 9/9/2009 - PLID 35495 - Pop up a dialog to position the topics in this template (not finished yet).

		if(!m_bIsTemplate) {
			//This should never happen!
			ASSERT(FALSE);
			MsgBox("Topics can only be positioned on Templates, not on Patient EMR records.");
			return;
		}

		if(IsEMRUnsaved()) {
			if(IDYES != MsgBox(MB_YESNO, "Before positioning topics, the current changes must be saved.  Would you like to "
				"save them now?")) {
					return;
			}
			else {
				if(SaveEMR(esotEMR, -1, TRUE) != essSuccess) {
					return;
				}
			}
		}

		//TODO: Make sure the template is writeable.

		DontShowMeAgain(this, "This dialog will allow you to position topics, including spawned topics, within this template.  "
			"Note that any topics which have never been spawned in the template editor will not appear in this dialog.  "
			"In order to position new spawned topics, you must first spawn them in the template editor, then click 'Position Topics' "
			"in order to position them relative to all spawned topics", "EmrTemplatePositionTopicsDlg", "Positioning Spawned Topics");

		CEmrPositionSpawnedTopicsDlg dlg(this);
		CEMN *pEMN = m_EMR.GetEMN(0);
		if(IDOK == dlg.Open(pEMN->GetID(), pEMN->GetDescription(), pEMN->GetEMNDate())) {
			ReloadEMN(pEMN, TRUE);
		}
	}NxCatchAll("Error in CEmrTreeWind::OnPositionTopics()");
}

bool CEmrTreeWnd::CheckIsNexWebTemplate(long nTemplateID)
{
	//TES 11/4/2009 - PLID 35807 - Do they want to be warned if this is the NexWeb template?
	if(!m_bIsTemplate && GetRemotePropertyInt("WarnWhenCreatingNexWebEmn", 1, 0, GetCurrentUserName())) {
		//TES 11/4/2009 - PLID 35807 - They do, so check whether it is.
		//(e.lally 2011-05-04) PLID 43537 - Use new NexWebDisplayT structure
		_RecordsetPtr rsNexWebTemplate = CreateParamRecordset("SELECT EmrTemplateID FROM NexWebDisplayT "
			"WHERE EmrTemplateID = {INT} AND Visible = 1 ", nTemplateID);
		if(!rsNexWebTemplate->eof) {
			// (b.cardillo 2010-09-22 09:18) - PLID 39568 - Corrected wording now that we allow more than one NexWeb EMR template
			if(IDYES != MsgBox(MB_YESNO|MB_ICONEXCLAMATION, "Warning: This template is a NexWeb template.  It is designed "
				"to be created and filled out by your patients, through your website.  "
				"Are you sure you wish to continue creating an EMN based on this template?")) {
				return false;
			}
		}
	}
	return true;
}

// (a.walling 2009-11-23 12:32) - PLID 36404 - Are we printing?
bool CEmrTreeWnd::IsPrinting()
{
	if (GetEmrPreviewCtrl()) {
		return GetEmrPreviewCtrl()->IsPrinting();
	} else {
		return false;
	}
}

// (a.walling 2009-12-03 10:20) - PLID 36418 - Print multiple EMNs for the current EMR
// (c.haag 2013-03-07) - PLID 55365 - Repurposed for printing one EMN with multiple layouts, or multiple EMN's
LRESULT CEmrTreeWnd::OnPrintMultiple(WPARAM wParam, LPARAM lParam)
{
	try {
		CEMN* pCurrentEMN = GetCurrentEMN();
		LPUNKNOWN lpunkEMRCustomPreviewLayouts = (LPUNKNOWN)wParam;
		long nSingleEmnID = (long)lParam;

		if (pCurrentEMN && pCurrentEMN->GetParentEMR()) 
		{
			CEMR* pCurrentEMR = pCurrentEMN->GetParentEMR();

			// We should be within an EMR here. Make sure the whole EMR is saved.
			if(pCurrentEMR->IsEMRUnsaved()) {
				if(IDYES != AfxMessageBox("Before continuing, the changes you have made to the EMR must be saved.  Would you like to continue?", MB_YESNO)) {
					return 0;
				}
				if(FAILED(SaveEMR(esotEMR, (long)pCurrentEMR, TRUE))) {
					AfxMessageBox("The EMR was not saved. The operation will now be cancelled.", MB_OK | MB_ICONEXCLAMATION);
					return 0;
				}
			}

			// (b.savon 2011-11-22 11:54) - PLID 25782 - Added PatientID
			CEMRPreviewMultiPopupDlg dlg(pCurrentEMR->GetPatientID(), this);
			// (c.haag 2013-02-28) - PLID 55368 - Assign the custom preview layouts
			dlg.SetCustomPreviewLayoutList(lpunkEMRCustomPreviewLayouts);
			// (c.haag 2013-02-28) - PLID 55368 - Release the custom preview layouts if we assigned them to the dialog
			if (NULL != lpunkEMRCustomPreviewLayouts) {
				lpunkEMRCustomPreviewLayouts->Release();
			}

			COleDateTime dtNull;
			dtNull.SetStatus(COleDateTime::null);

			for (int i = 0; i < pCurrentEMR->GetEMNCount(); i++) {
				CEMN* pAvailableEMN = pCurrentEMR->GetEMN(i);
				// (c.haag 2013-03-07) - PLID 55365 - Filter on a single EMN if we need to
				if (nSingleEmnID <= 0 || pAvailableEMN->GetID() == nSingleEmnID)
				{
					COleDateTime dtInputDate;
					// (a.walling 2009-12-03 10:09) - PLID 36418 - Get the input date, if we have it.
					if (pAvailableEMN->GetID() != -1) {
						// we should have an input date.
						_RecordsetPtr prs = CreateParamRecordset("SELECT InputDate FROM EMRMasterT WHERE ID = {INT}", pAvailableEMN->GetID());
						if (!prs->eof) {
							dtInputDate = AdoFldDateTime(prs, "InputDate", dtNull);
						} else {
							dtInputDate = dtNull;
						}
					} else {
						dtInputDate = dtNull;
					}

					// (z.manning 2012-09-11 17:53) - PLID 52543 - Added modified date
					// (c.haag 2013-02-28) - PLID 55368 - Added template ID
					dlg.AddAvailableEMN(pAvailableEMN, pAvailableEMN->GetID(), pAvailableEMN->GetTemplateID(), pCurrentEMR->GetDescription()
						, pAvailableEMN->GetDescription(), pAvailableEMN->GetEMNDate(), dtInputDate
						, pAvailableEMN->GetEMNModifiedDate());
				}
			}

			dlg.DoModal();
		}
	} NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.jones 2010-04-01 10:41) - PLID 37980 - added ability to tell the topic to add a given image
LRESULT CEmrTreeWnd::OnAddImageToEMR(WPARAM wParam, LPARAM lParam)
{
	try {

		//get our current topic
		IRowSettingsPtr pTopicRow = m_pTree->CurSel;
		if(pTopicRow) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pTopicRow).GetType();
			if(etrt == etrtTopic) {
				EnsureTopicRow(pTopicRow);
				CEmrTopicWndPtr pTopicWnd = EmrTree::ChildRow(pTopicRow).GetTopicWnd();
				CEMRTopic *pTopic = EmrTree::ChildRow(pTopicRow).GetTopic();

				//CanBeEdited() checks locked status, write token, and permissions
				if(IsWindow(pTopicWnd->GetSafeHwnd()) && pTopic->GetParentEMN()->CanBeEdited()) {
					//tell the topic to add the image
					pTopicWnd->PostMessage(NXM_ADD_IMAGE_TO_EMR, wParam, lParam);
				}
				EnsureTopicBackColor(pTopic);
				EnsureTopicModifiedState(pTopic);

				return 0;
			}
		}

		//if we are still here, log that we failed			
		BSTR bstrTextData = (BSTR)wParam;
		CString strImageFile(bstrTextData);
		SysFreeString(bstrTextData);

		Log("CEmrTreeWnd::OnAddImageToEMR - Could not add image file '%s' to the EMR because the active EMR has no active, writeable topic.", strImageFile);

	}NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.jones 2010-03-31 17:33) - PLID 37980 - returns TRUE if this EMR has an EMN opened to a topic that is writeable
BOOL CEmrTreeWnd::HasWriteableEMRTopicOpen()
{
	IRowSettingsPtr pTopicRow = m_pTree->CurSel;
	if(pTopicRow) {
		EmrTreeRowType etrt = EmrTree::ChildRow(pTopicRow).GetType();
		if(etrt == etrtTopic) {
			EnsureTopicRow(pTopicRow);
			CEmrTopicWndPtr pTopicWnd = EmrTree::ChildRow(pTopicRow).GetTopicWnd();
			CEMRTopic *pTopic = EmrTree::ChildRow(pTopicRow).GetTopic();

			//CanBeEdited() checks locked status, write token, and permissions
			if(IsWindow(pTopicWnd->GetSafeHwnd()) && pTopic->GetParentEMN()->CanBeEdited()) {
				//we can definitely add new items to this topic
				return TRUE;
			}
		}
	}

	return FALSE;
}

// (a.walling 2010-04-13 13:20) - PLID 37150 - The EMR is entirely loaded
LRESULT CEmrTreeWnd::OnEMRLoaded(WPARAM wParam, LPARAM lParam)
{
	try {
		if (!m_bIsTemplate) {
			// (j.jones 2006-12-19 08:55) - PLID 22794 - check the preference to open the room manager
			if(GetRemotePropertyInt("OpenRoomMgrWhenOpenEMR", 0, 0, GetCurrentUserName(), true) == 1) {
				GetMainFrame()->PostMessage(NXM_OPEN_ROOM_MANAGER, (long)-1,
					(BOOL)GetRemotePropertyInt("OpenRoomMgrWhenOpenEMRMinimized", 0, 0, GetCurrentUserName(), true) == 1);
			}

			// (j.jones 2012-10-01 12:18) - PLID 52922 - if a new EMN contains medications, we may
			// need to save the EMN and warn about drug interactions
			// (j.jones 2012-10-01 15:44) - PLID 52869 - also trigger an interaction check if the EMN
			// contains diagnosis codes
			for(int i=0;i<GetEMNCount();i++) {
				CEMN *pEmn = GetEMN(i);
				if(!m_bIsTemplate && pEmn->GetID() == -1 && !pEmn->IsLoading() && (pEmn->GetMedicationCount() > 0 || pEmn->GetDiagCodeCount() > 0)) {

					// (j.jones 2013-01-09 11:55) - PLID 54530 - open medication reconciliation
					if(pEmn->GetMedicationCount() > 0 && pEmn->IsWritable()) {
						//we must save now
						if(SUCCEEDED(SaveEMR(esotEMN, (long)pEmn, FALSE))) {

							//this is a new EMN, so all prescriptions on this EMN would have been newly created
							CArray<long, long> aryNewPrescriptionIDs;
							for(int m=0; m<pEmn->GetMedicationCount(); m++) {
								EMNMedication *pMed = pEmn->GetMedicationPtr(m);
								aryNewPrescriptionIDs.Add(pMed->nID);
							}
							ReconcileCurrentMedicationsWithNewPrescriptions(pEmn, aryNewPrescriptionIDs);

							//if reconciliation changed the current meds table, it would have forced a save
						}
					}

					// This function will check their preference to save the EMN immediately,
					// potentially do so, and warn about drug interactions.
					// (z.manning 2013-09-17 15:36) - PLID 58450 - New function for this
					pEmn->CheckSaveEMNForDrugInteractions(FALSE);
				}
			}
		}
	} NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.jones 2010-06-21 10:22) - PLID 39010 - added ability to add a generic table to the EMR
LRESULT CEmrTreeWnd::OnAddGenericTableToEMR(WPARAM wParam, LPARAM lParam)
{
	try {
		
		//get our current topic
		IRowSettingsPtr pTopicRow = m_pTree->CurSel;
		if(pTopicRow) {
			EmrTreeRowType etrt = EmrTree::ChildRow(pTopicRow).GetType();
			if(etrt == etrtTopic) {
				EnsureTopicRow(pTopicRow);
				CEmrTopicWndPtr pTopicWnd = EmrTree::ChildRow(pTopicRow).GetTopicWnd();
				CEMRTopic *pTopic = EmrTree::ChildRow(pTopicRow).GetTopic();

				//CanBeEdited() checks locked status, write token, and permissions
				if(IsWindow(pTopicWnd->GetSafeHwnd()) && pTopic->GetParentEMN()->CanBeEdited()) {
					//tell the topic to add the table
					// (d.lange 2011-04-12 17:16) - PLID 42754 - Using SendMessage instead of PostMessage
					pTopicWnd->SendMessage(NXM_ADD_GENERIC_TABLE_TO_EMR, wParam, lParam);
				}
				EnsureTopicBackColor(pTopic);
				EnsureTopicModifiedState(pTopic);

				return 0;
			}
		}

		//if we are still here, log that we failed
		Log("CEmrTreeWnd::OnAddGenericTableToEMR - Could not add a generic table to the EMR because the active EMR has no active, writeable topic.");

	}NxCatchAll(__FUNCTION__);

	return 0;
}

//(e.lally 2011-12-13) PLID 46968 - Makes the existing patient EMN available for a patient to fill out online via NexWeb
void CEmrTreeWnd::OnPublishToNexWebPortal()
{
	try {
		if(!g_pLicense){
			return;
		}

		//Is the EMR license expired?
		if(g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
			return;
		}

		//(e.lally 2011-12-13) PLID 46968 - officially validate license and check permissions
		if(!g_pLicense->CheckForLicense(CLicense::lcNexWebPortal, CLicense::cflrUse) 
			|| !(CheckCurrentUserPermissions(bioPublishEMNToNexWeb, sptDynamic0)) ){
				//denied
				return;
		}

		////Check for other invalid states////
		IRowSettingsPtr pEmnRow = m_pTree->CurSel;
		CEMN* pCurEMN = NULL;
		if (pEmnRow) {
			pCurEMN = GetEMNFromRow(pEmnRow);
		}

		if(pCurEMN == NULL) {
			return;
		}

		long nEMNID = pCurEMN->GetID();
		if(nEMNID == -1){
			//We're not on an existing EMN? This should be impossible
			AfxThrowNxException("Invalid attempt to publish a non pre-existing patient EMN to the NexWeb Portal.");
		}

		//if locked, throw an error. we need to better prevent this if we're in this state
		if(pCurEMN->GetStatus() == 2){
			//This should be impossible
			AfxThrowNxException("Invalid attempt to publish a Locked EMN to the NexWeb Portal.");
		}

		long nPatientID = m_EMR.GetPatientID();
		if(nPatientID == -1) {
			//this should be impossible
			AfxThrowNxException("Invalid patient ID when attempting to publish an EMN to the NexWeb Portal.");
		}

		if(pCurEMN->IsTemplate()){
			AfxThrowNxException("Attempting to publish an EMN template is invalid");
		}

		//Should be in a valid state if we got here.
		//(e.lally 2011-12-13) PLID 46968 - Force save before continuing. This prompt won't happen a second time in the toggle.
		if (pCurEMN->IsUnsaved()) {
			if (IDYES == MessageBox("This EMN has unsaved changes. You must save before publishing this EMN. Do you want to save now?", NULL, MB_YESNO|MB_ICONHAND)) {
				if(SaveEMR(esotEMR, -1, TRUE) != essSuccess) {
					return;
				}
			} 
			else {
				return;
			}
		}

		//(e.lally 2011-12-13) PLID 46968 - Release our Write Access. Fail if somehow we do not have write access
		HandleToggleReadOnly(true);

		//Prompt the user to double check the answers they are making available are ok for the patient to see
		if(IDOK != MessageBox(("This patient will be able to view and edit all the content "
			"already saved on this EMN through their NexWeb Patient Portal. Make sure you have "
			"reviewed its contents before continuing.\n\n"
			"Are you sure you wish to continue?"), "NexTech Practice", MB_OKCANCEL|MB_ICONWARNING)){
				//Give the user back the write access if we can
				HandleToggleReadOnly(false);
				return;
		}


		CEMN::EPatientCreatedStatus ePatientCreatedStatus = (CEMN::EPatientCreatedStatus)pCurEMN->GetPatientCreatedStatus();
		CString strOldStatus = "";
		if(ePatientCreatedStatus == CEMN::pcsCreatedByPatientFinalized){
			strOldStatus = "Patient-Finalized EMN";
		}
		else if(ePatientCreatedStatus == CEMN::pcsCreatedByPractice){
			strOldStatus = "Practice-Created EMN";
		}

		_variant_t varRevision = g_cvarNull;
		//(e.lally 2011-12-13) PLID 46968 - Update it to '1' (patient created) because we are not adding a separate status at this time for manually publishing an EMN.
		//	Get the revision value (that gets auto set) so we can update the cached value to match it.
		_RecordsetPtr rs = CreateParamRecordset("SET NOCOUNT ON\r\n"
			"UPDATE EMRMasterT SET PatientCreatedStatus = 1 WHERE EMRMasterT.ID = {INT}; \r\n"
			"SET NOCOUNT OFF\r\n"
			"SELECT Revision FROM EMRMasterT WHERE EMRMasterT.ID = {INT} ", nEMNID, nEMNID);
		if(!rs->eof){
			varRevision = rs->Fields->Item["Revision"]->Value;
		}
		rs->Close();
		//(e.lally 2011-12-13) PLID 46968 - Audit
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), BeginNewAuditEvent(), aeiFinalizeNexWebEMN, nEMNID, strOldStatus, "Patient-Entered EMN", aepMedium, aetChanged);

		//(e.lally 2011-12-14) PLID 46968 - Update the patient created status and revision to reflect what is now in data.
		pCurEMN->SetPatientCreatedStatus(CEMN::pcsCreatedByPatientNotFinalized);
		if(varRevision.vt != VT_NULL){
			pCurEMN->SetRevision(varRevision);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-03-27 14:59) - PLID 44763 - added global period warning
void CEmrTreeWnd::CheckWarnGlobalPeriod_EMR(COleDateTime dtToCheck)
{
	try {
		
		if(!m_bIsTemplate && GetEmrEditor()) {
			GetEmrEditor()->CheckWarnGlobalPeriod_EMR(dtToCheck);
		}

	}NxCatchAll(__FUNCTION__);
}

void CEmrTreeWnd::OnContextMenu(CWnd* pWnd, CPoint pos)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		// (a.walling 2012-04-05 13:11) - PLID 49472 - Moved context menu handling to WM_CONTEXTMENU
		m_wndTree.ScreenToClient(&pos);

		// (a.walling 2012-04-05 13:11) - PLID 49472 - Get the appropriate row from the datalist
		IRowSettingsPtr pRow = m_pTree->GetRowFromPoint(pos.x, pos.y);

		if (!pRow) return;

		// (b.cardillo 2006-02-17 16:01) - PLID 19168 - Moved these two lines to inside the if-
		// statement so that the user wouldn't be changing to sriNoRow by right-clicking in the 
		// white space around the tree nodes.
		if (pRow != m_pTree->CurSel) {
			SetTreeSel(pRow);
		}

		// (c.haag 2006-03-28 12:30) - PLID 19890 - We now consider user permissions
		BOOL bCanWriteToEMR = (m_bIsTemplate) ? TRUE :
			CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE);

		// (c.haag 2006-04-04 09:49) - PLID 19890 - We now consider user permissions
		// (j.jones 2007-05-15 17:07) - PLID 25431 - you can't create an EMR without Create and Write permissions
		BOOL bCanCreateEMR = bCanWriteToEMR && CheckCurrentUserPermissions(bioPatientEMR, sptCreate, FALSE, 0, TRUE, TRUE);
		// (j.jones 2006-09-13 09:24) - PLID 22493 - removed bioPatientEMR delete permission,
		// it is now administrator-only
		// (a.walling 2007-11-28 13:10) - PLID 28044 - Check for current EMR license here too
		BOOL bCanDeleteEMR = IsCurrentUserAdministrator() && (g_pLicense->HasEMR(CLicense::cflrSilent) == 2);

		// (a.walling 2007-11-28 11:23) - PLID 28044 - Also check for expired
		if (g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
			bCanWriteToEMR = FALSE;
			bCanCreateEMR = FALSE;
			bCanDeleteEMR = FALSE;
		}

		EmrTreeRowType etrt = EmrTree::ChildRow(pRow).GetType();
		switch(etrt) {
		case etrtEmn:
			{
				//Let them change the status.
				CEMN *pEMN = EmrTree::ChildRow(pRow).GetEMN();
				long nStatus = pEMN->GetStatus();
				// Create the pop up menu
				// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
				CNxMenu mnu;
				CNxMenu *pMenu = &mnu;
				pMenu->CreatePopupMenu();

				// (j.jones 2009-09-23 16:40) - PLID 29718 - added submenu for changing dates
				CMenu pChangeDateSubMenu;
				pChangeDateSubMenu.CreatePopupMenu();

				int nPos = 0;

				pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->GetID() == -1 ? MF_DISABLED|MF_GRAYED : 0), IDM_RELOAD_EMN, "Reload EMN");

				if(bCanWriteToEMR) {					
					// (a.walling 2008-06-03 16:51) - PLID 23138 - Add menu items for write access
					pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->GetID() == -1 ? MF_DISABLED|MF_GRAYED : 0), IDM_TOGGLEREADONLY, (pEMN->IsWritable() ? "Release Write Access" : "Request Write Access"));
					//(e.lally 2011-12-13) PLID 46968 - added menu option to publish an EMN to the patient's NexWeb Portal
					//	Silently check NexWeb Patient Portal license and silently get the user permissions
					if(!m_bIsTemplate){
						UINT uPublishFlags = 0;
						if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcNexWebPortal, CLicense::cflrSilent)
						  || !(GetCurrentUserPermissions(bioPublishEMNToNexWeb) & (sptDynamic0 | sptDynamic0WithPass))
						  || pEMN->GetID() == -1 || !pEMN->IsWritable() || nStatus==2) {
						  
							uPublishFlags = MF_DISABLED|MF_GRAYED;
						}

						pMenu->InsertMenu(nPos++, MF_BYPOSITION | uPublishFlags, IDM_PUBLISH_TO_NEXWEB_PORTAL, pEMN->GetPatientCreatedStatus() == CEMN::pcsCreatedByPatientFinalized ? "Re-Publish To Portal" : "Publish To Portal");
					}
					pMenu->InsertMenu(nPos++, MF_BYPOSITION|MF_SEPARATOR, 0, "");

					// Add the appropriate menu items to the pop up menu
						// (a.walling 2008-06-09 12:58) - PLID 23138 - Secure commands when not writable
					if (nStatus != 2) {
						// (j.jones 2007-09-11 15:38) - PLID 27352 - renamed label and gave it a unique message handler
						pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_ADD_NEW_TOPIC, "Add New Topic");
						if(!m_bIsTemplate) {
							// (m.hancock 2006-05-30 13:17) - PLID 20832 - Moved the permission checks up a few lines so 
							// other conditions can access the values without going out of scope.
							pMenu->InsertMenu(nPos++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
							if(nStatus != 0) pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_OPEN_EMN, "Mark EMN Open");
							if(nStatus != 1) pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_FINISH_EMN, "Mark EMN Finished");

							// (a.walling 2008-12-30 17:04) - PLID 30252 - Change the EMN date from the treewnd
							// (j.jones 2009-09-23 16:40) - PLID 29718 - added multiple options for changing dates
							pChangeDateSubMenu.InsertMenu(0, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_CHANGE_EMN_DATE, "&Select a Date");
							pChangeDateSubMenu.InsertMenu(1, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_SET_EMN_DATE_TODAY, "&Today's Date");
							pChangeDateSubMenu.InsertMenu(2, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_SET_EMN_DATE_LAST_APPT, "&Last Appointment Date");								
							pMenu->InsertMenu(nPos++, MF_BYPOSITION|MF_POPUP, (UINT)pChangeDateSubMenu.m_hMenu, "Change EMN Date...");

							pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_SAVE_ROW, "Save EMN");
							pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_LOCK_EMN, "Lock EMN");
							pMenu->InsertMenu(nPos++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
							pMenu->InsertMenu(nPos++, MF_BYPOSITION|((bCanCreateEMR)?0:(MF_DISABLED|MF_GRAYED)), IDM_COPY_EMN, "Copy EMN");
						}
					}
					else {
						// (a.wetta 2006-03-30 09:08) - PLID 19942 - If an EMN is locked you should still be able to copy it
						if(!m_bIsTemplate) {
							// (m.hancock 2006-05-30 13:18) - PLID 20832 - A user should be able to copy an EMN.. but only if
							// they have permission to create EMNs.  So, we need to check bCanCreateEMR and display the menu
							// entry appropriately.
							pMenu->InsertMenu(nPos++, MF_BYPOSITION|((bCanCreateEMR)?0:(MF_DISABLED|MF_GRAYED)), IDM_COPY_EMN, "Copy EMN");
						}						
					}

					// (j.jones 2008-07-23 14:53) - PLID 30789 - add ability to manage problems
					if(!m_bIsTemplate) {
						pMenu->InsertMenu(nPos++, MF_SEPARATOR);
						// (j.jones 2008-08-12 14:41) - PLID 30854 - disable the add option, but not the update option,
						// if the EMN is not writeable
						pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_ADD_EMN_PROBLEM, "Link with New &Problem");
						// (c.haag 2009-05-28 10:58) - PLID 34249 - Link existing problems with the EMN
						pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_LINK_EMN_PROBLEMS, "Link with Existing Problems");
						if (pEMN->HasProblems()) {
							pMenu->InsertMenu(nPos++, MF_BYPOSITION, IDM_EDIT_EMN_PROBLEM, "Update Problem &Information");
						}

						// (z.manning 2009-07-23 12:09) - PLID 34049 - Move the delete option to the bottom of the menu
						if(nStatus != 2) {
							pMenu->InsertMenu(nPos++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
							pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED))|((bCanDeleteEMR)?0:(MF_DISABLED|MF_GRAYED)), IDM_DELETE_EMN, "Delete EMN");
						}
					}
				}

				if (nPos > 0) {
					CPoint pt;
					GetMessagePos(pt);
					pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
				}
				//pMenu->DestroyMenu();
				//delete pMenu;
			}
			break;
		case etrtTopic:
			{
				CEMRTopic* pTopic = EmrTree::ChildRow(pRow).GetTopic();
				CEMN* pEMN = pTopic->GetParentEMN();

				// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
				CNxMenu mnu;
				CNxMenu *pMenu = &mnu;
				pMenu->CreatePopupMenu();
				CMenu mnuPosition;
				int nPos = 0;

				if(pEMN->GetStatus() != 2 && bCanWriteToEMR) {

					// (a.walling 2008-06-03 16:51) - PLID 23138 - Add menu items for write access
					pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->GetID() == -1 ? MF_DISABLED|MF_GRAYED : 0), IDM_TOGGLEREADONLY, (pEMN->IsWritable() ? "Release Write Access" : "Request Write Access"));
					pMenu->InsertMenu(nPos++, MF_BYPOSITION|MF_SEPARATOR, 0, "");

					// (j.jones 2007-09-11 15:38) - PLID 27352 - differentiated "Insert Subtopic" into abilities to "Add New Topic" or "Add New Subtopic"
					// (a.walling 2008-06-09 12:59) - PLID 23138 - Secure commands when not writable
					pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_ADD_NEW_TOPIC, "Add New Topic");
					pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_ADD_NEW_SUBTOPIC, "Add New Subtopic");
					//TES 6/20/2011 - PLID 31198 - Disable the Add Items option if the topic is read-only
					BOOL bTopicReadOnly = (m_bIsTemplate && pTopic->GetSourceActionID() != -1) ? TRUE : FALSE;
					//(e.lally 2012-04-04) PLID 48065 - Changed Add Items to global command ID
					pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() && !bTopicReadOnly ? 0 : (MF_DISABLED|MF_GRAYED)), ID_EMR_ADD_ITEM, "Add Items To Topic");
					pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_SAVE_ROW, "Save Topic");
					pMenu->InsertMenu(nPos++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
					pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_DELETE_TOPIC, "Delete Topic");

					if(m_bIsTemplate) {
						if(pTopic->ShowIfEmpty()) {
							pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_TOGGLE_SHOW_IF_EMPTY, "Hide when empty (on patient EMNs)");
						}
						else {
							pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_TOGGLE_SHOW_IF_EMPTY, "Show when empty (on patient EMNs)");
						}
						if(pTopic->GetSpawnedGroupID() != -1) {
							//This is a spawned embedded topic, let them toggle whether it appears on EMNs.
							//Although, if any of its parents are hidden, don't give them the option here, because it won't 
							//really have an effect.
							BOOL bParentHidden = FALSE;
							CEMRTopic *pParent = pTopic->GetParentTopic();
							while(!bParentHidden && pParent) {
								if(pParent->HideOnEMN()) bParentHidden = TRUE;
								pParent = pParent->GetParentTopic();
							}
							if(!bParentHidden) {
								if(pTopic->HideOnEMN()) {
									pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_TOGGLE_HIDE_ON_EMN, "Show on patient EMNs");
								}
								else {
									pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_TOGGLE_HIDE_ON_EMN, "Hide on patient EMNs");
								}
							}
						}
					} else {
						// (a.walling 2007-04-10 16:50) - PLID 25548 - Handle scrolling the preview via the treewnd
						pMenu->InsertMenu(nPos++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
						pMenu->InsertMenu(nPos++, MF_BYPOSITION, IDM_GOTO_PREVIEW, "Scroll to...");

						pMenu->InsertMenu(nPos++, MF_BYPOSITION | (m_bAutoScroll ? MF_CHECKED : MF_UNCHECKED), IDM_AUTOSCROLL, "Auto scroll");							
					}

					// (a.walling 2008-07-01 09:55) - PLID 30570 - Handle preview pane flags
					pMenu->InsertMenu(nPos++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
					pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED))|((pTopic->GetPreviewFlags() & epfHideTitle) ? MF_CHECKED : MF_UNCHECKED), IDM_HIDETOPICTITLE, "Hide Title when Printing");
					pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED))|((pTopic->GetPreviewFlags() & epfHideItem) ? MF_CHECKED : MF_UNCHECKED), IDM_HIDETOPICPREVIEW, "Hide Entire Topic when Printing");
					pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_HIDEALLDETAILSTITLE, "Mark All Details in this Topic to Hide Titles when Printing...");
					pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_HIDEALLDETAILSPREVIEW, "Mark All Details in this Topic to Hide Entirely when Printing...");

					// (a.walling 2009-01-08 13:40) - PLID 32659 - Prepare menu for floating elements
					mnuPosition.CreatePopupMenu();
					long nSub = 0;
					DWORD nPreviewFlags = pTopic->GetPreviewFlags();

					// clear both is default
					// (a.walling 2009-07-06 10:10) - PLID 34793 - Clearing is deprecated
					/*
					BOOL bClearLeft = TRUE;
					BOOL bClearRight = TRUE;

					if ((nPreviewFlags & epfClearNone) == epfClearNone) {
						bClearLeft = FALSE;
						bClearRight = FALSE;
					} else {
						if (nPreviewFlags & epfClearLeft) {
							bClearLeft = TRUE;
							bClearRight = FALSE;
						} else if (nPreviewFlags & epfClearRight) {
							bClearLeft = FALSE;
							bClearRight = TRUE;
						}
					}
					*/
					
					// (a.walling 2009-07-06 10:14) - PLID 34793 - epfFloatLeft is now epfColumnOne, epfFloatRight is now epfColumnTwo
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED))|((nPreviewFlags & epfColumnOne) ? MF_CHECKED : MF_UNCHECKED), IDM_COLUMN_ONE, "Column One");
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED))|((nPreviewFlags & epfColumnTwo) ? MF_CHECKED : MF_UNCHECKED), IDM_COLUMN_TWO, "Column Two");

					// (a.walling 2009-07-06 10:10) - PLID 34793 - Clearing is deprecated
					/*
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED))|(bClearLeft ? MF_CHECKED : MF_UNCHECKED), IDM_CLEAR_LEFT, "Clear Left");
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED))|(bClearRight ? MF_CHECKED : MF_UNCHECKED), IDM_CLEAR_RIGHT, "Clear Right");
					*/

					// (a.walling 2009-07-06 12:30) - PLID 34793 - Grouping for columns
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED))|((nPreviewFlags & epfGroupBegin) ? MF_CHECKED : MF_UNCHECKED), IDM_GROUP_BEGIN, "Group Columns at Beginning");
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED))|((nPreviewFlags & epfGroupEnd) ? MF_CHECKED : MF_UNCHECKED), IDM_GROUP_END, "Group Columns at End");

					// (a.walling 2009-01-08 14:13) - PLID 32660 - Align text right
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED))|((nPreviewFlags & epfTextRight) ? MF_CHECKED : MF_UNCHECKED), IDM_TEXT_RIGHT, "Align Text Right");

					// (a.walling 2010-08-31 18:20) - PLID 36148 - Page breaks
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED))|((nPreviewFlags & epfPageBreakBefore) ? MF_CHECKED : MF_UNCHECKED), IDM_PAGE_BREAK_BEFORE, "New Page Before");
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED))|((nPreviewFlags & epfPageBreakAfter) ? MF_CHECKED : MF_UNCHECKED), IDM_PAGE_BREAK_AFTER, "New Page After");

					pMenu->InsertMenu(nPos++, MF_BYPOSITION|MF_POPUP|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), (UINT_PTR)mnuPosition.GetSafeHmenu(), "Positioning");
				} else if (!m_bIsTemplate) {

					// (a.walling 2008-06-03 16:51) - PLID 23138 - Add menu items for write access
					pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->GetID() == -1 ? MF_DISABLED|MF_GRAYED : 0), IDM_TOGGLEREADONLY, (pEMN->IsWritable() ? "Release Write Access" : "Request Write Access"));
					pMenu->InsertMenu(nPos++, MF_BYPOSITION|MF_SEPARATOR, 0, "");

					// (a.walling 2007-04-10 16:50) - PLID 25548 - Handle scrolling the preview via the treewnd
					// (a.walling 2007-06-19 16:57) - PLID 26373 - Still need to scroll to even if it is locked or we have no permission
					pMenu->InsertMenu(nPos++, MF_BYPOSITION, IDM_GOTO_PREVIEW, "Scroll to...");

					// (a.walling 2007-06-19 17:07) - PLID 25548 - Preference to automatically scroll along with the EMN
					pMenu->InsertMenu(nPos++, MF_BYPOSITION | (m_bAutoScroll ? MF_CHECKED : MF_UNCHECKED), IDM_AUTOSCROLL, "Auto scroll");
				}

				// (j.jones 2008-07-23 14:53) - PLID 30789 - add ability to manage problems
				if(!m_bIsTemplate) {
					if(nPos != 0) {
						pMenu->InsertMenu(nPos++, MF_SEPARATOR);
					}
					// (j.jones 2008-08-12 14:41) - PLID 30854 - disable the add option, but not the update option,
					// if the EMN is not writeable
					pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_ADD_TOPIC_PROBLEM, "Link with New &Problem");
					// (c.haag 2009-05-28 10:58) - PLID 34249 - Link existing problems with the EMN
					pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), IDM_LINK_TOPIC_PROBLEMS, "Link with Existing Problems");
					if (pTopic->HasProblems()) {
						pMenu->InsertMenu(nPos++, MF_BYPOSITION, IDM_EDIT_TOPIC_PROBLEM, "Update Problem &Information");
					}
				}

				CPoint pt;
				GetCursorPos(&pt);
				pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
				//pMenu->DestroyMenu();
				//delete pMenu;
			}
			break;
		case etrtMoreInfo:
		case etrtCodes: //TES 2/12/2014 - PLID 60748
			{
				IRowSettingsPtr pParentRow = pRow->GetParentRow();
				
				CEMN* pEMN = EmrTree::ChildRow(pParentRow).GetEMN();
				ASSERT(pEMN != NULL);

				// (a.walling 2007-07-12 16:41) - PLID 26640 - Context menu for scrolling to more info
				// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
				CNxMenu mnu;
				CNxMenu *pMenu = &mnu;
				pMenu->CreatePopupMenu();
				long nPos = 0;

				// (a.walling 2008-06-03 16:51) - PLID 23138 - Add menu items for write access
				pMenu->InsertMenu(nPos++, MF_BYPOSITION|(pEMN->GetID() == -1 ? MF_DISABLED|MF_GRAYED : 0), IDM_TOGGLEREADONLY, (pEMN->IsWritable() ? "Release Write Access" : "Request Write Access"));
				pMenu->InsertMenu(nPos++, MF_BYPOSITION|MF_SEPARATOR, 0, "");

				if (!m_bIsTemplate) { // only for patient EMNs, not templates!
					// (a.walling 2007-04-10 16:50) - PLID 25548 - Handle scrolling the preview via the treewnd
					// (a.walling 2007-06-19 16:57) - PLID 26373 - Still need to scroll to even if it is locked or we have no permission
					pMenu->InsertMenu(nPos++, MF_BYPOSITION, IDM_GOTO_PREVIEW, "Scroll to...");

					// (a.walling 2007-06-19 17:07) - PLID 25548 - Preference to automatically scroll along with the EMN
					pMenu->InsertMenu(nPos++, MF_BYPOSITION | (m_bAutoScroll ? MF_CHECKED : MF_UNCHECKED), IDM_AUTOSCROLL, "Auto scroll");
				}
				
				CPoint pt;
				GetCursorPos(&pt);
				pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
				//pMenu->DestroyMenu();
				//delete pMenu;
			}
			break;
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-04-03 12:52) - PLID 49377 - An emr item is dragged over us and hovering; show the topic if necessary
void CEmrTreeWnd::OnDragHover(CPoint pt)
{
	try {
		EmrTree::ChildRow curRow(m_pTree->CurSel);

		if (!curRow) return;

		m_wndTree.ScreenToClient(&pt);

		EmrTree::ChildRow row(m_pTree->GetRowFromPoint(pt.x, pt.y));
		
		if (!row) return;		

		if (!row.IsTopic()) return;
		if (row == curRow) return;
		if (row.GetEMN() != curRow.GetEMN()) return;

		if (row->GetFirstChildRow()) {
			LoadSubtopics(row);
			row->Expanded = VARIANT_TRUE;
		}

		CEMRTopic* pHoverTopic = row.GetTopic();
		if (!pHoverTopic || (m_bIsTemplate && pHoverTopic->GetSourceActionID() != -1) ) {
			return;
		}

		GetEmrFrameWnd()->GetEmrTopicView(row.GetEMN(), true)->ShowTopicAnimated(pHoverTopic, true);

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-09-28 15:15) - PLID 52922 - Used after something is added to an EMN
// that might affect drug interactions. If the EMRDrugInteractionChecks preference is
// set to save immediately, this function will perform that save.
// CheckShowDrugInteractions will later show any interactions post-save.
// (j.jones 2012-11-13 10:06) - PLID 52869 - changed to be only called via posted message,
// the wParam must be the EMN pointer to save.
// (j.jones 2013-02-06 15:13) - PLID 55045 - lParam is now a bool value to signify that
// something definitely changed that can affect drug interactions, and it already saved.
// This is used for prescriptions that save in place without stating the EMN was modified.
LRESULT CEmrTreeWnd::CheckSaveEMNForDrugInteractions(WPARAM wParam, LPARAM lParam)
{
	try {

		CEMN *pEMN = (CEMN*)wParam;
		BOOL bAlreadySavedAnInteractionChange = (BOOL)lParam;

		// (j.jones 2013-02-06 15:59) - PLID 55045 - If the caller told us it already saved the change,
		// which is normal for prescription changes, then we need to act as though the save succeeded,
		// and try to open interactions based on their preference. It is possible this function may
		// later save the EMN in full and try to refresh interactions again if more content changed.
		if(bAlreadySavedAnInteractionChange && m_eEMRDrugInteractionChecksPref != edictNoPrompt) {
			if(m_eEMRDrugInteractionChecksPref == edictSaveWarnWhenMedsChange || m_eEMRDrugInteractionChecksPref == edictWarnWhenSaving) {
				CheckShowDrugInteractions(TRUE, FALSE, FALSE);
			}
			//Track that interactions were changed during this EMR session.
			m_bDrugInteractionsChangedThisSession = TRUE;
		}

		if(pEMN == NULL) {
			return 0;
		}

		if(pEMN->IsLockedAndSaved()) {
			//we can't save, the EMN is locked!
			return 0;
		}

		if(pEMN->IsLoading()) {
			//do not try to save if the EMN is loading

			//it is not necessarily a failure if we hit this assertion,
			//but the caller should be changed to not call this function
			//if the EMN is still loading, and ensure that this function
			//is called again when the EMN finishes loading
			ASSERT(FALSE);
			return 0;
		}

		//do nothing unless the preference is set to warn when medications
		//or other relevant drug interaction data changes, which will force
		//a save
		if(m_eEMRDrugInteractionChecksPref == edictSaveWarnWhenMedsChange){
			
			//Save the EMN. The saving code will call CheckShowDrugInteractions
			//if anything changed (presumably something did or else this function
			//shouldn't have been called) and show drug interactions if any exist.
			if(!pEMN->IsTemplate() && pEMN->IsUnsaved() && pEMN->IsWritable()){
				SaveEMR(esotEMN, (long)pEMN, FALSE);
			}
		}

	}NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.jones 2012-10-08 09:18) - PLID 36220 - exposed m_eEMRDrugInteractionChecksPref
EMRDrugInteractionCheckType CEmrTreeWnd::GetEMRDrugInteractionChecksPref()
{
	return m_eEMRDrugInteractionChecksPref;
}

// (j.jones 2013-02-27 10:18) - PLID 55343 - moved E/M Checklist code to the treewnd
void CEmrTreeWnd::OpenEMChecklist(CEMN *pEMN)
{
	try {

		if(pEMN == NULL) {
			return;
		}

		// (j.jones 2007-08-29 11:27) - PLID 27135 - added permissions
		if(!CheckCurrentUserPermissions(bioPatientEMChecklist, sptWrite))
			return;

		long nVisitType = pEMN->GetVisitTypeID();
		if(nVisitType == -1) {
			AfxMessageBox("This EMN must have a Visit Type selected before an E/M checklist can be used.");
			return;
		}

		CEMREMChecklistDlg dlg(this);
		dlg.m_nVisitTypeID = nVisitType;
		dlg.m_strVisitTypeName = pEMN->GetVisitTypeName();

		// (j.jones 2013-04-22 16:21) - PLID 56372 - added a read only flag
		BOOL bEMNIsReadOnly = (!pEMN->IsWritable() || pEMN->IsLockedAndSaved());
		dlg.m_bIsReadOnly = bEMNIsReadOnly;
		dlg.m_pEMN = pEMN;
		
		//find the checklist ID - but the checklist has to have columns, coding levels, and rules
		//to be valid
		_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ID FROM EMChecklistsT "
			"WHERE VisitTypeID = {INT} "
			"AND ID IN (SELECT ChecklistID FROM EMChecklistColumnsT) "
			"AND ID IN (SELECT ChecklistID FROM EMChecklistCodingLevelsT) "
			"AND ID IN (SELECT ChecklistID FROM EMChecklistRulesT) ", nVisitType);
		if(!rs->eof) {
			dlg.m_nChecklistID = AdoFldLong(rs, "ID",-1);
		}
		else {
			AfxMessageBox("The selected Visit Type does not have a complete E/M checklist associated with it.\n"
				"Each Visit Type must have an E/M checklist with columns, coding levels, and rules created for it in order to be used on a patient's EMN.");
			return;
		}
		rs->Close();

		// (j.jones 2013-02-14 09:38) - PLID 54668 - force a save
		//TES 2/12/2014 - PLID 60740 - No need to check IsMoreInfoUnsaved() if we've already checked IsMoreInfoUnsaved()
		if(pEMN->IsUnsaved() || pEMN->GetID() == -1) {
			if(IDYES != MsgBox(MB_YESNO, "Before opening the E/M checklist, the changes you have made to the EMN must be saved.  Would you like to continue?")) {
				return;
			}
			if(FAILED(SaveEMR(esotEMN, (long)pEMN, TRUE))) {
				AfxMessageBox("The EMN was not saved. E/M Coding cannot be used until the EMN has been saved.");
				return;
			}
		}
		
		if(dlg.DoModal() == IDCANCEL)
			return;

		// (j.jones 2013-04-22 16:19) - PLID 56372 - if the EMN is read only, do nothing,
		// we only allowed showing the checklist to see the content they had approved
		if(bEMNIsReadOnly) {
			return;
		}

		//grab the service ID
		long nServiceID = -1;
		CString strServiceCode = "";

		{
			ChecklistCodingLevelInfo *pCodingLevelToUse = dlg.m_pCodingLevelToUse;
			if(pCodingLevelToUse) {
				nServiceID = pCodingLevelToUse->nServiceID;
				strServiceCode = pCodingLevelToUse->strCodeNumber;
			}
		}

		//did we get one?
		if(nServiceID == -1)
			return;

		//great, let's grab its info. from data
		CString strCode, strSubCode, strDescription;
		COleCurrency cyPrice;
		BOOL bBillable = TRUE;

		rs = CreateParamRecordset("SELECT Code, SubCode, Name, Price, Active, Billable "
			"FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"WHERE ServiceT.ID = {INT}", nServiceID);
		if(!rs->eof) {
			strCode = AdoFldString(rs, "Code","");
			strSubCode = AdoFldString(rs, "SubCode","");
			strDescription = AdoFldString(rs, "Name","");
			cyPrice = AdoFldCurrency(rs, "Price", COleCurrency(0,0));
			// (j.jones 2011-03-28 15:22) - PLID 42575 - added Billable
			bBillable = AdoFldBool(rs, "Billable", TRUE);

			//is this code still active?
			BOOL bActive = AdoFldBool(rs, "Active", TRUE);
			if(!bActive) {
				CString strWarning;
				// (a.walling 2010-07-15 16:26) - PLID 39687 - This did not have a parameter for %s, leading to an access violation.
				// I am using strCode to be consistent with the rest of this dialog, which ignore the subcode.
				strWarning.Format("The %s service code is inactive, and cannot be added to this EMN.\n"
					"Please edit your E/M Checklist accordingly to use only active service codes.", strCode);
				AfxMessageBox(strWarning);
				return;	
			}
		}
		rs->Close();

		// (b.spivey, March 07, 2012) - PLID 48581 - We use this function to find and remove any conflicting codes. If successful, 
		//		we add the charge. We need to check if we can allow increments, because if so we modify an existing charge. 
		if(pEMN->FindAndRemoveEMChargeConflicts(nServiceID, strCode)) {

			BOOL bAllowQtyIncrement = GetRemotePropertyInt("EMNChargesAllowQtyIncrement",
			GetRemotePropertyInt("ChargeAllowQtyIncrement", 0, 0, "<None>", false),
			0, "<None>", true);

			EMNCharge *pFoundCharge = NULL;
			//don't need to check that the charge exists if we aren't increasing quantity
			if(bAllowQtyIncrement) {
				// (j.jones 2012-02-20 09:25) - PLID 47886 - need to search our memory objects, not the datalist
				pFoundCharge = pEMN->FindByServiceID(nServiceID);
			}
			if (pFoundCharge) {
				long nID = pFoundCharge->nID;
				long nServiceID = pFoundCharge->nServiceID;

				//Set the updated quantity
				// (b.spivey, March 07, 2012) - PLID 48581 - This will only ever be one. 
				pFoundCharge->dblQuantity += 1.0;

				//Mark the object as having been changed
				pFoundCharge->bChanged = TRUE;

				// (b.spivey, March 07, 2012) - PLID 48581 - Let the interface know things have changed.
				SendMessage(NXM_EMN_CHARGE_CHANGED, (WPARAM)pFoundCharge, (LPARAM)pEMN);
				// (a.walling 2012-03-22 16:50) - PLID 49141 - notify the interface
				//TES 2/21/2014 - PLID 60972 - Charges are on the Codes topic now
				pEMN->SetCodesUnsaved();
				// (s.dhole 2014-02-18 12:54) - PLID 60742  Change pointer from more info to code dlg
				CEmrCodesDlg *pDlg = GetCodesDlg (pEMN, FALSE);
				if(pDlg) {
					pDlg->RefreshChargeList();
				}
			}
			else {

				// (j.jones 2012-03-27 15:10) - PLID 44763 - warn if we're under a global period
				CheckWarnGlobalPeriod_EMR(pEMN->GetEMNDate());

				// (b.spivey, March 07, 2012) - PLID 48581 - If we don't allow increments or if this is a new charge, just add it. 
				// (j.jones 2013-04-22 14:48) - PLID 54596 - moved the charge generation here, only now that we know we are adding it,
				// otherwise we had memory leaks
				EMNCharge *pCharge = new EMNCharge; 
				pCharge->nID = -1; 
				pCharge->nServiceID = nServiceID;
				pCharge->bBillable = bBillable;
				pCharge->cyUnitCost = cyPrice; 
				pCharge->strCode = strCode;
				pCharge->strSubCode = strSubCode; 
				pCharge->strDescription = strDescription; 
				pCharge->dblQuantity = 1.0; 
				pCharge->bChanged = TRUE; 

				// (j.jones 2012-08-22 09:23) - PLID 50486 - set the default insured party ID
				pCharge->nInsuredPartyID = pEMN->GetParentEMR()->GetDefaultChargeInsuredPartyID();
				
				// (j.jones 2013-04-22 10:57) - PLID 54596 - The checklist dialog now saves all rule/coding level approvals
				// as they occur, so we no longer need to copy the checklist's pending audits.
				// The checklist used to also add the checklist audit for the new charge, but that is now the responsibility of
				// this code as it adds the new charge.
				CString strNewValue;
				strNewValue.Format("%s added from %s checklist", strCode, pEMN->GetVisitTypeName());
				CPendingAuditInfo* pChargeAdded = new CPendingAuditInfo(pEMN->GetParentEMR()->GetPatientID(), GetExistingPatientName(m_EMR.GetPatientID()), aeiEMChecklistUsageAddedCharge, pEMN->GetID(), "", strNewValue, aepHigh, aetCreated);
				pCharge->aryPendingEMAuditInfo.Add(pChargeAdded);

				// (a.walling 2012-03-22 16:50) - PLID 49141 - notifies the interface
				pEMN->AddCharge(pCharge); 

				// (b.spivey, March 07, 2012) - PLID 48581 - Let the interface know things have changed. 
				SendMessage(NXM_EMN_CHARGE_CHANGED, (WPARAM)pCharge, (LPARAM)pEMN);
				// (s.dhole 2014-02-18 12:54) - PLID 60742  Change pointer from more info to code dlg
				CEmrCodesDlg *pDlg = GetCodesDlg (pEMN, FALSE);
				if(pDlg) {
					pDlg->RefreshChargeList();
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-10-22 16:10) - PLID 52818 - this function will cause all open EMNs
// in the current EMR to reload their medications from the database
// (j.jones 2013-02-07 09:25) - PLID 55045 - now takes in an optional parameter to disable
// showing drug interactions when the requery finishes
void CEmrTreeWnd::ReloadAllEMNMedicationsFromData(BOOL bShowDrugInteractions /*= TRUE*/)
{
	try {

		if(m_bIsTemplate) {
			//this should be impossible
			ASSERT(FALSE);
			ThrowNxException("ReloadAllEMNMedicationsFromData called on a template!");
		}

		for(int i = 0; i < m_EMR.GetEMNCount(); i++) {
			CEMN *pEMN = m_EMR.GetEMN(i);
			pEMN->ReloadMedicationsFromData(TRUE, bShowDrugInteractions);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-10-25 15:54) - PLID 53322 - Called after closing an EMN,
// this function will check whether a blue Allergies table was on the EMN,
// and was saved with no allergies. If so, and PatientsT.HasNoAllergies is false,
// the user will be prompted to fill this setting.
void CEmrTreeWnd::CheckPromptPatientHasNoAllergies(BOOL bIsClosing)
{
	try {

		//this is for patient EMRs only
		if(m_bIsTemplate) {
			return;
		}

		//if we already prompted once, don't do it again
		if(m_bHasAnsweredHasNoAllergiesPrompt) {
			return;
		}

		//If the allergy table was saved, and could have updated the patient allergies
		//(was blue, was newest EMN), then this flag would have been set. If there were
		//no allergies, then the patient account would not have been updated, but the flag
		//is set just before that determination is made.
		if(!m_EMR.GetHadSavedAllergies()) {
			return;
		}

		long nPatientID = m_EMR.GetPatientID();

		//check to see if the patient has no allergies, and the "has no allergies" box is not checked?
		if(!ReturnsRecordsParam("SELECT PersonID FROM PatientsT "
			"WHERE PersonID = {INT} "
			"AND HasNoAllergies = 0 "
			"AND PersonID NOT IN (SELECT PersonID FROM PatientAllergyT WHERE Discontinued = 0 AND PersonID = {INT})", nPatientID, nPatientID)) {
			//the patient has active allergies, or the "has no allergies" box is already checked,
			//either way we don't need to prompt
			return;
		}
		
		//Now prompt to toggle the "Has No Allergies" flag.
		//If we're closing, post a message to MainFrame to have it prompt (MainFrm will be the messagebox parent.)
		//If we're not closing, prompt from here (EMR will be the messagebox parent.)
		if(bIsClosing) {
			//When closing, we can't prompt inside the EMR because the window is in the process of being destroyed.
			GetMainFrame()->PostMessage(NXM_PROMPT_PATIENT_HAS_NO_ALLERGIES, (WPARAM)nPatientID);
		}
		else {
			PromptPatientHasNoAllergies(nPatientID);
		}

		//Regardless of their choice, track that we have shown this prompt,
		//such that we won't try and ask again this session.
		//This boolean is somewhat meaningless if we're currently closing.
		m_bHasAnsweredHasNoAllergiesPrompt = TRUE;

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-10-26 16:32) - PLID 53324 - Called after closing an EMN,
// this function will check whether a blue Current Medications table was
// on the EMN, and was saved with no meds. If so, and PatientsT.HasNoMeds
// is false, the user will be prompted to fill this setting.
void CEmrTreeWnd::CheckPromptPatientHasNoCurrentMeds(BOOL bIsClosing)
{
	try {

		//this is for patient EMRs only
		if(m_bIsTemplate) {
			return;
		}

		//if we already prompted once, don't do it again
		if(m_bHasAnsweredHasNoCurrentMedsPrompt) {
			return;
		}

		//current meds. can't update Medications anymore unless using NewCrop,
		//therefore this prompt will also only be for NewCrop
		if(g_pLicense->HasEPrescribing(CLicense::cflrSilent) != CLicense::eptNewCrop) {
			return;
		}

		//If the current meds. table was saved, and could have updated the patient medications
		//(was blue, was newest EMN), then this flag would have been set. If there were no
		//meds. in the table, then the patient account would not have been updated, but the flag
		//is set just before that determination is made.
		if(!m_EMR.GetHadSavedCurrentMeds()) {
			return;
		}

		long nPatientID = m_EMR.GetPatientID();

		//check to see if the patient has no current meds., and the "has no meds" box is not checked?
		if(!ReturnsRecordsParam("SELECT PersonID FROM PatientsT "
			"WHERE PersonID = {INT} "
			"AND HasNoMeds = 0 "
			"AND PersonID NOT IN (SELECT PatientID FROM CurrentPatientMedsT WHERE Discontinued = 0 AND PatientID = {INT})", nPatientID, nPatientID)) {
			//the patient has active current meds., or the "has no meds" box is already checked,
			//either way we don't need to prompt
			return;
		}
		
		//Now prompt to toggle the "Has No Medications" flag.
		//If we're closing, post a message to MainFrame to have it prompt (MainFrm will be the messagebox parent.)
		//If we're not closing, prompt from here (EMR will be the messagebox parent.)
		if(bIsClosing) {
			//When closing, we can't prompt inside the EMR because the window is in the process of being destroyed.
			GetMainFrame()->PostMessage(NXM_PROMPT_PATIENT_HAS_NO_CURRENT_MEDS, (WPARAM)nPatientID);
		}
		else {
			PromptPatientHasNoCurrentMedications(nPatientID);
		}

		//Regardless of their choice, track that we have shown this prompt,
		//such that we won't try and ask again this session.
		//This boolean is somewhat meaningless if we're currently closing.
		m_bHasAnsweredHasNoCurrentMedsPrompt = TRUE;

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-01-09 11:55) - PLID 54530 - moved medication reconciliation functions here from EMNMoreInfoDlg

// (c.haag 2010-02-17 15:02) - PLID 37384 - Returns any current medications info detail,
// that has a valid state, or NULL if none exists
CEMNDetail* CEmrTreeWnd::GetActiveCurrentMedicationDetail(CEMN *pEMN)
{
	CEMR* pEMR = (pEMN) ? pEMN->GetParentEMR() : NULL;
	// Fail if we have no EMR or EMN
	if (NULL == pEMR || NULL == pEMN) {
		return NULL;
	}
	// Fail if this EMN is locked. We don't support locked details in the design of this function.
	if (pEMN->GetStatus() == 2) {
		return NULL;
	}

	// Get the info ID
	long nActiveCurrentMedicationsInfoID = pEMR->GetCurrentMedicationsInfoID();

	// Go through this EMN looking for the first Current Medications item. All of them should have
	// identical states, so it doesn't matter which one we actually find.
	const unsigned long nDetails = pEMN->GetTotalDetailCount();
	for (unsigned long n=0; n < nDetails; n++) {
		CEMNDetail* pDetail = pEMN->GetDetail(n);
		if (pDetail->IsCurrentMedicationsTable() && nActiveCurrentMedicationsInfoID == pDetail->m_nEMRInfoID) {
			// If we get here, we found an active Current Medications item. Return its state.
			if (VT_EMPTY != pDetail->GetStateVarType()) {
				return pDetail;
			}
		}
	}

	// If we get here, then this EMR has no Current Medications items. Return empty.
	return NULL;
}

// (c.haag 2010-02-17 10:19) - PLID 37384 - Lets the user apply a new prescription to the current medications list
// (j.jones 2012-11-16 13:51) - PLID 53765 - this now is called after the medication is saved, and only needs the medication ID
void CEmrTreeWnd::ReconcileCurrentMedicationsWithNewPrescription(CEMN *pEMN, long nNewPrescriptionID)
{
	// (j.jones 2013-01-09 11:55) - PLID 54530 - just call the same function as when we add multiple prescriptions
	CArray<long, long> aryNewPrescriptionIDs;
	aryNewPrescriptionIDs.Add(nNewPrescriptionID);
	ReconcileCurrentMedicationsWithNewPrescriptions(pEMN, aryNewPrescriptionIDs);
}

// (j.jones 2013-01-09 11:55) - PLID 54530 - added ability to reconcile multiple prescriptions
void CEmrTreeWnd::ReconcileCurrentMedicationsWithNewPrescriptions(CEMN *pEMN, CArray<long, long> &aryNewPrescriptionIDs)
{
	//don't do this on a template
	if(m_bIsTemplate) {
		//this is supported on patient EMRs only
		ASSERT(FALSE);
		return;
	}

	if(aryNewPrescriptionIDs.GetSize() == 0) {
		//no need to reconcile if we have no new prescriptions
		return;
	}

	// (j.jones 2013-01-11 13:12) - PLID 54462 - check preferences here
	if(GetRemotePropertyInt("ReconcileNewRxWithCurMeds", 0, 0, "<None>", true) == 0) {
		return;
	}
	// (j.jones 2013-01-11 13:30) - PLID 54462 - added the SkipEMRTable option, NewCrop-only
	// (j.jones 2013-02-12 15:32) - PLID 55139 - Now we never update current meds unless it's NewCrop AND this option is turned off.
	BOOL bSkipEMRTable = TRUE;
	if((g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptNewCrop) && (GetRemotePropertyInt("ReconcileNewRxWithCurMeds_SkipEMRTable", 1, 0, "<None>", true) == 0)) {
		bSkipEMRTable = FALSE;
	}

	// (c.haag 2010-02-17 10:19) - PLID 37384 - Let the user apply the prescription to the current
	// medications list. This only applies to active Current Medications details in this EMN that are
	// accessible.
	CEMR* pEMR = (pEMN) ? pEMN->GetParentEMR() : NULL;
	CEMNDetail* pCurrentMedsDetail = NULL;

	// (j.jones 2013-01-11 13:14) - PLID 54462 - don't need to check for this data if we aren't
	// wanting to update the current meds. table anyways
	if(!bSkipEMRTable) {
		if (pEMN != NULL && pEMN->GetStatus() == 2) {
			// (j.jones 2013-01-08 11:35) - PLID 47302 - if we're locked, how did we get a new prescription?
			ThrowNxException("ReconcileCurrentMedicationsWithNewPrescriptions called on a locked EMN!");
		}
		
		if(pEMN) {
			pCurrentMedsDetail = GetActiveCurrentMedicationDetail(pEMN);
		}
	}

	// (j.jones 2013-01-08 11:52) - PLID 47303 - if we have no current meds table,
	// or (less likely) somehow don't have valid EMN/EMR pointers, use the mainframe
	// reconciliation so the changes save directly to the patient's account, rather
	// than to a blue table
	// (j.jones 2013-01-11 13:14) - PLID 54462 - now we have a preference to skip
	// updating the blue table even if we do have one
	if(bSkipEMRTable || pCurrentMedsDetail == NULL || pEMN == NULL || pEMR == NULL) {
		//this function requires a newcrop array, but it is always empty here
		CStringArray astrNewCropRxGUIDs;
		CDWordArray arNewCDSInterventions;
		//TES 10/31/2013 - PLID 59251 - If this triggers any interventions, notify the user
		ReconcileCurrentMedicationsWithMultipleNewPrescriptions(pEMN->GetParentEMR()->GetPatientID(), aryNewPrescriptionIDs, astrNewCropRxGUIDs, GetSysColor(COLOR_BTNFACE), this, arNewCDSInterventions);
		GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);
		return;
	}
	else if (pCurrentMedsDetail && pEMN && pEMR && CReconcileMedicationsDlg::CanCurrentUserAccess()) {

		CReconcileMedicationsDlg dlgReconcile(pEMN->GetParentEMR()->GetPatientID(), this);
		//add all our prescriptions
		dlgReconcile.AddPrescriptionByPMIDs(aryNewPrescriptionIDs);
		dlgReconcile.SetBackColor(EmrColors::Topic::Background(!!m_bIsTemplate));
			
		// Get the state of the active current medications item in this EMN

		// Get the Emr Data ID's out of the state
		_variant_t vState = pCurrentMedsDetail->GetState();
		CString str = VarString(vState, "");
		CEmrTableStateIterator etsi(str);
		CArray<long,long> anEmrDataIDs;
		CMap<long, long, CString, LPCTSTR> mapDataIDsToSig;
		long X, Y, nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID;
		CString strData;
		
		// (j.jones 2011-05-04 14:32) - PLID 43527 - get the Sig column, if we have it
		TableColumn *ptcSig = pCurrentMedsDetail->GetColumnByListSubType(lstCurrentMedicationSig);

		// (z.manning 2010-02-18 12:41) - PLID 37427 - Added EmrDetailImageStampID
		// (z.manning 2011-03-02 14:54) - PLID 42335 - Added nStampID
		while (etsi.ReadNextElement(X,Y,strData,nEmrDetailImageStampID,nEmrDetailImageStampPointer,nStampID)) {
			// The X value corresponds to EmrDataT.ID
			anEmrDataIDs.Add(X);

			// (j.jones 2011-05-04 14:31) - PLID 43527 - grab the Sig, if they filled it in
			if(ptcSig != NULL && ptcSig->nID == Y) {
				mapDataIDsToSig.SetAt(X, strData);
			}
		}

		// Add them to the dialog as current medications
		dlgReconcile.AddCurrentMedicationsFromEmrDataIDs(anEmrDataIDs, mapDataIDsToSig);

		if (IDOK == dlgReconcile.DoModal()) {
			// (s.dhole 2013-06-21 16:44) - PLID 55964 chanege due to change in class structure
			CReconcileMedicationsDlg::CMergeMedicationArray& aRequestedChanges = dlgReconcile.GetRequestedChanges();
			if (aRequestedChanges.GetSize() > 0) {

				for (int i=0; i < aRequestedChanges.GetSize(); i++) {
					CReconcileMedicationsDlg::MergeMedication ch = aRequestedChanges[i];					
					if (ch.Action == ch.eAddMed  || ch.Action == ch.eMergeCurMed ) {
						// Select a Current Medication row based on the prescription
						anEmrDataIDs.Add(ch.nEMRDataID);	
						mapDataIDsToSig.SetAt(ch.nEMRDataID, ch.strPatientExplanation);
					}
					else if (ch.Action == ch.eDeleteMed  ){
						// Unselect a Current Medication row based on the current medication	
						for (int j=0; j < anEmrDataIDs.GetSize(); j++) {
							if (anEmrDataIDs[j] == ch.nCurrentEMRDataID) {
								anEmrDataIDs.RemoveAt(j--);
							}
						}
					}
				}

				CWaitCursor wc;
				// Apply the new selections to the official current medications detail we got earlier
				// (j.jones 2011-05-04 14:09) - PLID 43527 - added mapDataIDsToSig, which tracks the Sig for each current medication
				pEMN->ApplyOfficialCurrentMedications(pCurrentMedsDetail, pEMR->GetCurrentMedicationsInfoID(), anEmrDataIDs, mapDataIDsToSig, TRUE);
				// Now get the new state and apply it to all the other official current medications details in this EMN
				_variant_t vNewState = pCurrentMedsDetail->GetState();
				pCurrentMedsDetail->RequestStateChange(vNewState, vState);
				// Make sure the content reflects the state
				pCurrentMedsDetail->ReflectCurrentContent();
				// Now update all the other details
				pEMN->UpdateAllMedicationListDetails(vState, pEMR->GetCurrentMedicationsInfoID(), pCurrentMedsDetail);

				//If the EMN is unsaved, save it now.
				CEmrTreeWnd *pTreeWnd = GetEmrTreeWnd();
				//TES 2/12/2014 - PLID 60740 - No need to check IsMoreInfoUnsaved() if we've already checked IsMoreInfoUnsaved()
				if(!pEMN->IsLoading() && pTreeWnd && (pEMN->IsUnsaved() || pEMN->GetID() == -1) && pEMN->IsWritable()) {
					pTreeWnd->SaveEMR(esotEMN, (long)pEMN, FALSE);
				}

			} // if (aRequestedChanges.GetSize() > 0) {
			else {
				// If we get here, there were no changes to make
			}
		}
		else {
			// User changed their mind
		}
	}
	else {
		// Preference is turned off, do nothing
	}
}

// (b.savon 2013-01-24 10:59) - PLID 54817
void CEmrTreeWnd::ShowEPrescribing()
{
	OnEPrescribing();
}

// (j.jones 2013-05-16 14:21) - PLID 56596 - moved from the .h to the .cpp
CEMR* CEmrTreeWnd::GetEMR()
{
	return &m_EMR;
};

// (b.savon 2014-02-25 14:22) - PLID 61029 - Summary of Care - EMR Relay
void CEmrTreeWnd::GenerateCareSummary(bool customize)
{
	CEMN* pEMN = NULL;
	pEMN = GetActiveEMN();
	if(!pEMN) {
		MessageBox("Failed to generate Summary of Care.", "NexTech Practice", MB_OK|MB_ICONWARNING);
		return;
	}				
	if(!pEMN->GetParentEMR()) {
		MessageBox("Failed to generate Summary of Care.", "NexTech Practice", MB_OK|MB_ICONWARNING);
		return;
	}		

	//save before continuing,  abort if failure
	EmrSaveStatus status = SaveEMR();
	if(status != essSuccess) {
		return;
	}

	//get current person id
	long nCurrentPersonID = pEMN->GetParentEMR()->GetPatientID();

	// (b.savon 2014-02-25 14:22) - PLID 61029 - Call the Utility
	if (customize) {
		// (a.walling 2014-05-12 09:24) - PLID 61787 - Customized CCDA summaries
		// (r.gonet 05/07/2014) - PLID 61805 - Pass -1 for the PIC since summaries of care are not to be associated with PICs.
		// (a.walling 2014-06-10 09:02) - PLID 61788 - Generated summary of care should associate with the EMN, but not the PIC
		// this is true even though the summary of care data/document itself is not influenced by any particular EMN or PIC etc. 
		CSelectCCDAInfoDlg dlg(ctSummaryOfCare, -1, pEMN->GetID(), nCurrentPersonID, this);
		dlg.DoModal();
	}
	else {
		CCDAUtils::GenerateSummaryOfCare(nCurrentPersonID, pEMN->GetID(), GetSafeHwnd());
	}
}

// (b.savon 2014-02-25 14:22) - PLID 61029 - Clinical Summary - EMR Relay
void CEmrTreeWnd::GenerateClinicalSummary(bool customize)
{
	try {

		CNxPerform nxp(__FUNCTION__);

		//need to get the emn id
		CEMN* pEMN = NULL;

		//save before continuing,  abort if failure
		EmrSaveStatus status = SaveEMR();
		if(status != essSuccess) {
			MessageBox("Failed to generate Clinical Summary.", "NexTech Practice", MB_OK|MB_ICONWARNING);
			return;
		}

		pEMN = GetActiveEMN();
		if(!pEMN) {
			MessageBox("Failed to generate Clinical Summary.", "NexTech Practice", MB_OK|MB_ICONWARNING);
			return;
		}
		long nEMNID = pEMN->GetID();

		if(!pEMN->GetParentEMR()) {
			MessageBox("Failed to generate Clinical Summary.", "NexTech Practice", MB_OK|MB_ICONWARNING);
			return;
		}

		// (r.gonet 05/07/2014) - PLID 61805 - Get the PIC ID from data. We don't have access to this without a trip to the database.
		long nPICID = -1;
		long nEMRID = pEMN->GetParentEMR()->GetID();
		_RecordsetPtr prs = CreateParamRecordset("SELECT ID FROM PicT WHERE EMRGroupID = {INT}", nEMRID);
		if(!prs->eof) {
			nPICID = AdoFldLong(prs, "ID", -1);
		}
		prs->Close();
		long nCurrentPersonID = pEMN->GetParentEMR()->GetPatientID();

		// (b.savon 2014-02-25 14:22) - PLID 61029 - Call the Utility
		if (customize) {
			// (a.walling 2014-05-12 09:24) - PLID 61787 - Customized CCDA summaries
			// (r.gonet 05/07/2014) - PLID 61805 - Pass along the PICID so the MailSent record will be associated with a PIC
			CSelectCCDAInfoDlg dlg(ctClinicalSummary, nPICID, nEMNID, nCurrentPersonID, this);
			dlg.DoModal();
		}
		else {
			// (r.gonet 05/07/2014) - PLID 61805 - Pass along the PICID so the MailSent record will be associated with a PIC
			CCDAUtils::GenerateClinicalSummary(nCurrentPersonID, nPICID, nEMNID, GetSafeHwnd());
		}
		
	} NxCatchAll(__FUNCTION__);
}

//TES 8/13/2014 - PLID 63519 - Moved a bunch of tablechecker handling code to this function, to be shared between the normal and EX tablechecker
void CEmrTreeWnd::HandleChangedPatient(CMap<CEMN*, CEMN*, long, long&> &mapEMNsToUpdateNarratives)
{
	_RecordsetPtr rsUpdate = NULL;
	for (int i = 0; i < m_EMR.GetEMNCount(); i++) {
		CEMN* pEMN = m_EMR.GetEMN(i);
		// don't modify saved EMNs
		if (pEMN && (pEMN->GetID() == -1)) {
			if (rsUpdate == NULL) {
				// query for updated info
				rsUpdate = CreateParamRecordset("SELECT Last, First, Middle, Gender FROM PersonT WHERE ID = {INT}", GetPatientID());
			}

			// update with this info
			if (!rsUpdate->eof) {
				// (a.walling 2007-10-19 12:49) - PLID 23714 - Only increase our changed count if fields we watch
				// have actually changed
				BYTE iNewGender = AdoFldByte(rsUpdate, "Gender", 0);
				CString strNewFirst = AdoFldString(rsUpdate, "First", "");
				CString strNewMiddle = AdoFldString(rsUpdate, "Middle", "");
				CString strNewLast = AdoFldString(rsUpdate, "Last", "");

				if (iNewGender != pEMN->GetPatientGender()) {
					// (a.walling 2007-10-19 12:44) - PLID 23714 - Increase our count
					mapEMNsToUpdateNarratives[pEMN]++;

					pEMN->SetPatientGender(iNewGender);
				}
				if (strNewFirst != pEMN->GetPatientNameFirst()) {
					// (a.walling 2007-10-19 12:44) - PLID 23714 - Increase our count
					mapEMNsToUpdateNarratives[pEMN]++;

					pEMN->SetPatientNameFirst(strNewFirst);
				}
				if (strNewMiddle != pEMN->GetPatientNameMiddle()) {
					// (a.walling 2007-10-19 12:44) - PLID 23714 - Increase our count
					mapEMNsToUpdateNarratives[pEMN]++;

					pEMN->SetPatientNameMiddle(strNewMiddle);
				}
				if (strNewLast != pEMN->GetPatientNameLast()) {
					// (a.walling 2007-10-19 12:44) - PLID 23714 - Increase our count
					mapEMNsToUpdateNarratives[pEMN]++;

					pEMN->SetPatientNameLast(strNewLast);
				}
			}
			else {
				// the patient was deleted?! this needs to be handled somehow eventually
				ASSERT(FALSE);
			}

			// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
			CEMNMoreInfoDlg* pMoreInfo = GetMoreInfoDlg(pEMN, FALSE);
			if (pMoreInfo) {
				pMoreInfo->RefreshPatientDemographics();
			}

			UpdatePreviewMoreInfo(pEMN);
		}
	}
}

//TES 8/13/2014 - PLID 63519 - Added support for EX tablecheckers
LRESULT CEmrTreeWnd::OnTableChangedEx(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2007-10-19 12:43) - PLID 23714 - EMNs that have been updated which may need to have narratives updated
		CMap<CEMN*, CEMN*, long, long&> mapEMNsToUpdateNarratives;

		switch (wParam)
		{
		case NetUtils::PatCombo:
			CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
			long nPatientID = VarLong(pDetails->GetDetailData(CClient::pcdiPersonID), -1);
			if (nPatientID == GetPatientID()) {
				//TES 8/13/2014 - PLID 63194 - Most of the code was moved to this function.
				HandleChangedPatient(mapEMNsToUpdateNarratives);
			}
			break;
		}

		// (a.walling 2007-10-19 12:58) - PLID 23714 - Now go through our map and update narratives
		POSITION pos = mapEMNsToUpdateNarratives.GetStartPosition();
		while (pos) {
			CEMN* pEMN;
			long nCount;
			mapEMNsToUpdateNarratives.GetNextAssoc(pos, pEMN, nCount);
			if (pEMN) {
				// Update!
				//TES 1/23/2008 - PLID 24157 - Renamed.
				pEMN->HandleDetailChange(NULL);
			}
		}

	}NxCatchAll(__FUNCTION__);
	return 0;
}