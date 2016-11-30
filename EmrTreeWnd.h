#if !defined(AFX_EMRTREEWND_H__1672E883_8B0C_46DF_8FF8_0D033F61EEC8__INCLUDED_)
#define AFX_EMRTREEWND_H__1672E883_8B0C_46DF_8FF8_0D033F61EEC8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrTreeWnd.h : header file
//

#include "EmrUtils.h"
#include "AudioRecordDlg.h"
#include "EMRPreviewCtrlDlg.h"
#include "EMREMChecklistSetupDlg.h"
#include "EmrHelpers.h"

class CEMNMoreInfoDlg;
class CEmrTopicWnd;
class CEMRTopic;
class CEMNDetail;
class CEMRPreviewCtrlDlg;
// (j.jones 2013-05-16 13:58) - PLID 56596 - turned EMR/EMN into forward declares
class CEMR;
class CEMN;
class EMNStatus;
class EMNProcedure;
typedef shared_ptr<CEmrTopicWnd> CEmrTopicWndPtr;
class CEmrCodesDlg;

// (a.walling 2008-06-25 13:35) - PLID 30496 - Moved these to header file
// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define	IDM_OPEN_EMN		34100
#define IDM_FINISH_EMN		34101
#define IDM_SAVE_ROW		34102
#define IDM_LOCK_EMN		34103
#define IDM_DELETE_EMN		34104
// (j.jones 2007-09-11 15:36) - PLID 27352 - added unique function for AddTopic vs. AddSubtopic
#define IDM_ADD_NEW_TOPIC	34105
#define IDM_ADD_NEW_SUBTOPIC	34106
#define IDM_DELETE_TOPIC	34107
#define IDM_TOGGLE_SHOW_IF_EMPTY	34108
#define IDM_TOGGLE_HIDE_ON_EMN	34109
#define IDM_COPY_EMN		34110
#define IDM_GOTO_PREVIEW	34111
#define IDM_AUTOSCROLL		34112
// (a.walling 2008-06-03 16:47) - PLID 23138
#define IDM_TOGGLEREADONLY	34113
// (a.walling 2008-06-09 17:06) - PLID 22049 - debug menu item for now
#define IDM_RELOAD_EMN		34114
// (a.walling 2008-07-01 09:57) - PLID 30570
#define IDM_HIDETOPICPREVIEW	34115
#define IDM_HIDETOPICTITLE		34116
#define IDM_HIDEALLDETAILSPREVIEW 34117
#define IDM_HIDEALLDETAILSTITLE 34118
// (j.jones 2008-07-23 14:58) - PLID 30789 - added ability to manage EMN and Topic problems
#define IDM_ADD_EMN_PROBLEM		34119
#define IDM_EDIT_EMN_PROBLEM	34120
#define IDM_ADD_TOPIC_PROBLEM	34121
#define IDM_EDIT_TOPIC_PROBLEM	34122
// (a.walling 2008-12-30 17:04) - PLID 30252 - Change the EMN date from the treewnd
#define IDM_CHANGE_EMN_DATE		34123
// (a.walling 2009-01-08 13:42) - PLID 32659 - Modify topic float/clear flags
// (a.walling 2009-07-06 10:29) - PLID 34793
#define IDM_COLUMN_ONE			34124
#define IDM_COLUMN_TWO			34125
/*
#define IDM_CLEAR_LEFT			126
#define IDM_CLEAR_RIGHT			127
*/
// (a.walling 2009-07-06 12:34) - PLID 34793
#define IDM_GROUP_BEGIN			34126
#define IDM_GROUP_END			34127
// (a.walling 2009-01-08 14:07) - PLID 32660 - Align text right
#define IDM_TEXT_RIGHT			34128
// (c.haag 2009-05-28 10:34) - PLID 34249 - Linking with existing problems
#define IDM_LINK_EMN_PROBLEMS	34129
#define IDM_LINK_TOPIC_PROBLEMS	34130

// (j.jones 2009-09-23 16:49) - PLID 29718 - added multiple options for changing dates
#define IDM_SET_EMN_DATE_TODAY	34131
#define IDM_SET_EMN_DATE_LAST_APPT	34132

// (a.walling 2010-08-31 18:20) - PLID 36148 - Page breaks
#define IDM_PAGE_BREAK_BEFORE 34133
#define IDM_PAGE_BREAK_AFTER 34134
//(e.lally 2011-12-13) PLID 46968
#define IDM_PUBLISH_TO_NEXWEB_PORTAL 34135

// (a.walling 2008-05-14 16:31) - PLID 29114 - timer id for refreshing preview pane
#define IDT_PREVIEW_REFRESH	1288
// (a.walling 2008-07-03 13:53) - PLID 30498 - timer to see if we are out of modal loop
#define IDT_FORCE_ACCESS_TIMER 1289

//(e.lally 2012-04-04) PLID 48065 - Removed a bunch of the old IDC control defines for buttons, hence the gaps in IDs
#define IDC_EMR_TREE_CTL	1000
#define IDC_TOPIC_AREA	1003 //Not an actual control.
#define IDC_EDIT_MODE	1005
// (j.jones 2007-04-03 15:03) - PLID 25464 - added the splitter bar
#define IDC_SPLITTER_BAR	1011
// (j.jones 2007-04-05 12:11) - PLID 18217 - added the preview pane
#define	IDC_EMR_PREVIEW_PANE	1012
//TES 8/13/2009 - PLID 35227 - Added a button to toggle the preview pane
#define IDC_TOGGLE_PREVIEW		1019


#define TREE_COLUMN_ID			0
#define TREE_COLUMN_ROW_TYPE	1
#define TREE_COLUMN_OBJ_PTR		2
#define TREE_COLUMN_ICON		3
#define TREE_COLUMN_NAME		4
#define TREE_COLUMN_LOADED		5 //Are the subtopics loaded? (etrtEmn and etrtEmrTopic rows)

//TES 8/13/2009 - PLID 35227 - Determines the size of the button for toggling the preview window.
const long gc_nTogglePreviewBtnWidth = 16;
const long gc_nTogglePreviewBtnHeight = 16;

/////////////////////////////////////////////////////////////////////////////
// CEmrTreeWnd window

//TES 9/9/2009 - PLID 35495 - Moved the EmrTreeIcon enum to EmrUtils.h

// (a.walling 2011-10-20 21:22) - PLID 46077 - Facelift - EMR tree pane
// Helper classes for interacting with the tree

// (j.jones 2013-05-08 09:31) - PLID 56596 - moved function defines to the .cpp
namespace EmrTree
{
	class ChildRow
	{
	public:
		ChildRow();

		ChildRow(NXDATALIST2Lib::IRowSettingsPtr pRow);

		NXDATALIST2Lib::IRowSettingsPtr GetRow();

		operator NXDATALIST2Lib::IRowSettings*();

		NXDATALIST2Lib::IRowSettingsPtr& operator->();

		EmrTreeRowType GetType();

		long IsLoaded();

		bool IsEMN();

		bool IsTopic();

		bool IsValid();

		bool IsMoreInfo();

		bool IsPlaceholder();

		//TES 2/12/2014 - PLID 60748 - New row type for Codes topic
		bool IsCodes();

		//TES 2/19/2014 - PLID 60750 - Several places treat Codes and More Info the same way
		bool IsMoreInfoOrCodes();

		CEMN* GetEMN();

		CEMRTopic* GetTopic();

		CEmrTopicWndPtr GetTopicWnd();

		////

		ChildRow GetNext();

		ChildRow GetPrev();

		ChildRow GetParent();

	protected:
		NXDATALIST2Lib::IRowSettingsPtr m_pRow;
	};

	class TreeRow : public ChildRow
	{
	public:
		TreeRow();

		TreeRow(NXDATALIST2Lib::IRowSettingsPtr pChildRow, NXDATALIST2Lib::IRowSettingsPtr pRootRow);

		TreeRow GetNext();

		TreeRow GetPrev();

		TreeRow GetParent();

	protected:
		NXDATALIST2Lib::IRowSettingsPtr m_pRootRow;
	};
};

class CEmrTreeWnd : public CWnd, public Emr::InterfaceAccessImpl<CEmrTreeWnd>
{
	//
	// (c.haag 2007-09-28 10:59) - PLID 27509 - Don't use "friend", or else you
	// will allow entities outside of the class to mess with member variables and
	// put the class object in states unexpected by the developer working on the class.
	// It's best to leave this class all alone with no friends...don't worry, it won't
	// get lonely or depressed.
	//
	//friend class CEMRPreviewCtrlDlg;
	//friend class CEMN;

	// Even CEmrTreeWnds get lonely sometimes
	friend class CEmrFrameWnd;
	// (a.walling 2012-02-28 14:53) - PLID 48451 - CEmrTreeWnd has a nice social circle growing
	friend class CEmrPatientFrameWnd;
	friend class CPicContainerDlg;
	friend class CEmrTemplateFrameWnd;
	friend class CEmrTopicWnd;
// Construction
public:
	CEmrTreeWnd();

// Attributes
public:
	DECLARE_DYNAMIC(CEmrTreeWnd);

	// (a.walling 2012-04-03 12:52) - PLID 49377 - An emr item is dragged over us and hovering; show the topic if necessary
	void OnDragHover(CPoint ptScreen);
	
	// (a.walling 2009-11-23 12:32) - PLID 36404 - Are we printing?
	bool IsPrinting();

	//Call exactly one of these functions to initialize the window.
	// (c.haag 2007-03-07 16:46) - PLID 25110 - Added support for a patient ID
	//DRT 7/27/2007 - PLID 26836 - Added a parameter nEMNIDToBeDisplayed, see implementation for details.
	//TES 11/22/2010 - PLID 41582 - Added nPicID (only really needed for new EMRs)
	void SetPatientEMR(long nEmrID, long nPicID, long nPatientID, long &nEMNIDToBeDisplayed);
	void SetTemplate(long nEmrTemplateID);
	void CreatePatientEMR(long nPatientID);
	void CreateEMRTemplate();

	// (j.jones 2007-01-10 15:39) - PLID 24027 - supported SourceDetailID
	// (z.manning 2009-03-04 15:40) - PLID 33338 - Use the new source action info class
	CEMN* AddEMNFromTemplate(long nTemplateID, SourceActionInfo &sai, long nAppointmentID);
	// (a.walling 2013-01-22 10:00) - PLID 54762 - Emr Appointment linking - auto-assign appointment or prompt to choose
	CEMN* AddEMNFromTemplateWithAppointmentPrompt(long nTemplateID, SourceActionInfo &sai);
	CEMN* AddNewTemplateEMN(long nCollectionID);	

	// (a.walling 2012-07-03 10:56) - PLID 51284 - Fixing activation ambiguities - Highlighting an EMN
	void HighlightActiveTopic(CEMRTopic* pTopic);
	void HighlightActiveEMN(CEMN* pEMN);
	
	long GetPatientID();

	// (j.jones 2007-07-26 09:23) - PLID 24686 - this is a horrible idea that should never occur
	//void RefreshAllItems();
	// (j.jones 2007-07-26 09:10) - PLID 24686 - converted RefreshContent into two functions,
	// accepting an InfoID or a MasterID
	void RefreshContentByInfoID(long nEMRInfoID, BOOL bSetRegionAndInvalidateDetails);
	void RefreshContentByInfoMasterID(long nEMRInfoMasterID, BOOL bSetRegionAndInvalidateDetails);
	// (a.wetta 2007-04-09 13:30) - PLID 25532 - This function refreshes the content
	// all all EMR items of a certain type.
	// (a.walling 2008-12-19 09:21) - PLID 29800 - This was only used for images, and only to refresh the custom stamps, which was causing the content
	// to be reloaded. This is all unnecessary, and the custom stamps is entirely UI. So let's just do what we need to do, and refresh the custom stamps,
	// rather than flag as needed to reload content. This is all controlled by the new bRefreshCustomStampsOnly param. I could have renamed the function
	// entirely, but I can see how this might come in handy in the future.
	void RefreshContentByType(EmrInfoType eitItemType, BOOL bSetRegionAndInvalidateDetails = FALSE, BOOL bMaintainImagesSize = FALSE, BOOL bRefreshCustomStampsOnly = FALSE);

	//TES 12/15/2005 - You can pass in a detail to ignore in the calculation (if it's about to be deleted).
	// (a.walling 2007-09-17 15:55) - PLID 25599
	// (b.cardillo 2012-03-06 14:56) - PLID 48647 - Added bForceRecalculateCompletionStatus to cause the topic to store a new completion status based on the results of this operation.
	BOOL EnsureTopicBackColor(CEMRTopic *pTopic, CEMNDetail *pDetailToIgnore = NULL, BOOL bHideIfEmpty = TRUE, BOOL bForceRecalculateCompletionStatus = FALSE);
	void EnsureAllTopicBackColors(BOOL bForceRecalculateCompletionStatus);

	//If the currently selected row is not already in the given EMN, it will expand that EMN, and highlight the first row in it.
	void ShowEMN(CEMN *pEMN);
	void ShowEMN(long nEmnID);

	BOOL HasEditableEMN();	// (j.armen 2012-07-02 11:45) - PLID 49831

	// (a.walling 2012-07-12 09:36) - PLID 46078 - Retrieve active topic
	CEMRTopic* GetActiveTopic();

	// (c.haag 2008-07-11 10:14) - PLID 30550 - Returns the active EMN
	CEMN* GetActiveEMN();
	// (a.walling 2007-04-11 13:37) - PLID 25548 - Only one EMN should be 'active' at a given time, for the preview window to display.
	void SetActiveEMN(long nEmnID);

	// (j.jones 2012-10-11 17:58) - PLID 52820 - added bIsClosing, TRUE if the user picked Save & Close
	EmrSaveStatus SaveEMR(EmrSaveObjectType esotSaveType = esotEMR, long nObjectPtr = -1, BOOL bShowProgressBar = TRUE, BOOL bIsClosing = FALSE);

	// (j.jones 2012-09-28 11:17) - PLID 52820 - Used when closing or locking an EMN,
	// anything that would show drug interactions once per session.
	// This function checks the m_bDrugInteractionsChangedThisSession flag,
	// if true it will call CheckShowDrugInteractions and then reset the flag.
	void TryShowSessionDrugInteractions(BOOL bIsLocking, BOOL bIsClosing);

	// (j.jones 2012-10-25 15:54) - PLID 53322 - Called after closing an EMN,
	// this function will check whether a blue Allergies table was on the EMN,
	// and was saved with no allergies. If so, and PatientsT.HasNoAllergies is false,
	// the user will be prompted to fill this setting.
	void CheckPromptPatientHasNoAllergies(BOOL bIsClosing);

	// (j.jones 2012-10-26 16:32) - PLID 53324 - Called after closing an EMN,
	// this function will check whether a blue Current Medications table was
	// on the EMN, and was saved with no meds. If so, and PatientsT.HasNoMeds
	// is false, the user will be prompted to fill this setting.
	void CheckPromptPatientHasNoCurrentMeds(BOOL bIsClosing);

	// (j.jones 2013-01-09 11:55) - PLID 54530 - moved medication reconciliation functions here from EMNMoreInfoDlg

	// (c.haag 2010-02-17 15:02) - PLID 37384 - Returns any current medications info detail,
	// that has a valid state, or NULL if none exists
	CEMNDetail* GetActiveCurrentMedicationDetail(CEMN *pEMN);

	// (c.haag 2010-02-17 10:19) - PLID 37384 - Lets the user apply a new prescription to the current medications list
	// (j.jones 2012-11-16 13:51) - PLID 53765 - this now is called after the medication is saved, and only needs the medication ID
	void ReconcileCurrentMedicationsWithNewPrescription(CEMN *pEMN, long nNewPrescriptionID);
	// (j.jones 2013-01-09 11:55) - PLID 54530 - added ability to reconcile multiple prescriptions
	void ReconcileCurrentMedicationsWithNewPrescriptions(CEMN *pEMN, CArray<long, long> &aryNewPrescriptionIDs);

	// (a.walling 2010-03-31 11:04) - PLID 38006 - Saves a single EMN
	bool SaveSingleEMN(CEMN* pEMN, CStringArray& saErrors, BOOL bShowProgressBar);

	long GetEMRID();

	void EnsureAndPopupDetail(CEMNDetail *pDetail);

	void AddNewTopic(BOOL bAddAsSubtopic = FALSE);

	// (c.haag 2008-06-17 12:38) - PLID 17842 - Optional parameter to return the added items
	void AddItems(const CDWordArray &arInfoMasterIDs, CArray<CEMNDetail*,CEMNDetail*>* papNewDetails = NULL);

	void ImportTopics(BOOL bImportAsSubtopics = FALSE);

	//Is our associated EMR empty?
	BOOL IsEmpty();

	//TES 5/20/2008 - PLID 27905 - This is sometimes called when procedures are about to be deleted; if so, pass in the EMN
	// they're being deleted from and this will only return true if the procedure is on some other EMN.
	BOOL IsProcedureInEMR(long nProcedureID, CEMN* pExcludingEmn = NULL);

	BOOL IsEMRUnsaved();

	// (j.jones 2011-07-15 13:45) - PLID 42111 - takes in an image file name (could be a path),
	// and returns TRUE if any Image detail on this EMR references it
	BOOL IsImageFileInUseOnEMR(const CString strFileName);

	// (a.walling 2007-10-12 11:19) - PLID 27017 - Return TRUE if the detail is in the Preview's pending update list
	BOOL IsDetailInPendingUpdateArray(CEMNDetail* pDetail);

	//Prompts the user to select items, and adds them to the current topic.
	void AddItemsToTopic();

	CEMN* GetLastEMN();

	// (a.walling 2007-06-18 11:50) - PLID 25549 - Update the preview for this detail
	void UpdateDetailPreview(CEMNDetail* pDetail);

	// (a.walling 2007-07-13 11:22) - PLID 26640 - Update the preview's more info section
	void UpdatePreviewMoreInfo(CEMN* pEMN);

	// (a.walling 2012-03-07 08:36) - PLID 48680
	BOOL IsAutoScroll()
	{
		return m_bAutoScroll;
	}

	// (a.walling 2012-07-03 10:56) - PLID 51284 - OnActiveEMNChanged now virtual CEmrFrameWnd member fn
public:
	// (c.haag 2007-09-28 11:11) - PLID 27509 - This function was written to provide functionality
	// to CEMN::GenerateSaveString without giving it full access to all protected members of this class.
	// Returns TRUE if the internal CEMNMoreInfoDlg object is not NULL and UpdateChangedInfo was called
	// on it. Returns FALSE if the CEMNMoreInfoDlg object is NULL.
	// (z.manning 2013-06-05 09:48) - PLID 56962 - This no longer takes a param for output message.
	BOOL UpdateEMNMoreInfoDlgChangedInfo(CEMN* pEMN);

	// (c.haag 2007-09-28 11:44) - PLID 27509 - Returns TRUE if the tree is NULL
	BOOL IsTreeNull() const;

public:
	int GetEMNCount();
	CEMN* GetEMN(int nIndex);
	CEMN* GetEMNByID(int nID);

	// (a.walling 2008-07-07 11:34) - PLID 30496 - Get the current displayed / selected EMN
	CEMN* GetCurrentEMN();

	// (a.walling 2008-06-10 11:58) - PLID 22049 - Reloads an EMN
	BOOL ReloadEMN(CEMN*& pEMN, BOOL bShow);

	// (a.walling 2008-08-13 14:29) - PLID 22049 - Removes an EMN from the tree (and memory)
	void RemoveEMN(CEMN* pEMN);

public:
	// (c.haag 2009-09-11 10:23) - PLID 35077 - This function will edit a problem to an EMN (moved from OnEditEmnProblem)
	void EditEmnProblem(CEMN* pEMN);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrTreeWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEmrTreeWnd();

	// (j.jones 2007-07-06 11:49) - PLID 25457 - made this function public
	//Makes sure that the given row is loaded (in particular, that it has a valid CWnd*)
	CEmrTopicWndPtr EnsureTopicRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
	CEmrTopicWndPtr EnsureTopicView(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// (c.haag 2007-09-28 11:34) - PLID 27509 - This encapsulates FindByColumn for the tree.
	// Returns NULL if the row wasn't found or if the tree is NULL.
	NXDATALIST2Lib::IRowSettingsPtr FindInTreeByColumn(short nColIndex,
        const _variant_t & varValue,
        IDispatch * pStartRow
		// We absolutely forbid auto-selections
		);

public:
	// (c.haag 2007-08-30 15:59) - PLID 27256 - Flags whether this is a template. This
	// should only be called by CEmrTemplateEditorDlg::OnInitDialog() to indicate to us
	// whether this is a template before CreateControls is called.
	void SetIsTemplate(BOOL bIsTemplate);

	// (a.walling 2008-06-27 15:49) - PLID 30482 - Get the EMR
	// (j.jones 2013-05-16 14:21) - PLID 56596 - moved the function to the .cpp
	CEMR* GetEMR();

	// Generated message map functions
protected:
	// (j.jones 2013-05-16 14:25) - PLID 56596 - turned into a reference
	CEMR &m_EMR;

	// (c.haag 2007-03-07 16:46) - PLID 25110 - Added support for a patient ID
	//DRT 7/27/2007 - PLID 26836 - Added a parameter nEMNIDToBeDisplayed, see implementation for details.
	//TES 11/22/2010 - PLID 41582 - Added nPicID (only really needed for new EMRs)
	void SetEMR(long nEmrID, long nPicID, BOOL bIsTemplate, long nPatientID, long &nEMNIDToBeDisplayed);

	CWnd m_wndTree; //The CWnd that contains the tree datalist.
	NXDATALIST2Lib::_DNxDataListPtr m_pTree; //The tree datalist.

	// (a.walling 2012-03-12 12:43) - PLID 48712 - No longer necessary since we are always regenerating anyway
	//CString PrepareSavedDocument(long nEmnID); // (a.walling 2007-04-12 17:56) - PLID 25605 - Put the file and any dependencies in the temp path.

	// (c.haag 2007-08-30 16:31) - PLID 27058 - We allow users to access a modeless E/M Checklist setup dialog
	CEMREMChecklistSetupDlg m_dlgEMChecklistSetup;

	void CreateControls();
	
	BOOL m_bIsTemplate;
	BOOL m_bDragging; // (a.walling 2007-05-18 11:09) - PLID 25092 - Are we dragging a detail?
	long m_nInShowEMN; // (a.walling 2007-10-01 13:49) - PLID 25548 - Whether we are in ShowEMN or not
											// (Increases when enteres ShowEMN, decreases when exits)
	BOOL m_bReloadingEMN; // (a.walling 2008-09-03 12:51) - PLID 31239 - We are currently reloading an EMN

	//Load the subtopics for a particular topic.
	void LoadSubtopics(NXDATALIST2Lib::IRowSettings *pParentRow);
	CFont *m_pButtonFont;

	// (a.walling 2008-06-11 10:15) - PLID 22049 - List of modeless windows
	// (a.walling 2010-03-29 15:54) - PLID 34289 - Updated name of this collection so it is known that these windows will self-destruct when destroyed
	CMap<long, long, CWnd*, CWnd*> m_mapModelessSelfDestructWindows;
	CMap<long, long, BOOL, BOOL> m_mapAutoWriteAccessTried;
	// (a.walling 2008-07-03 12:39) - PLID 30498 - EMN IDs that are waiting on a release
	CMap<long, long, BOOL, BOOL> m_mapWaitingOnRelease;
	CMap<long, long, CWnd*, CWnd*> m_mapModelessWaitingWindows;
	// (a.walling 2008-07-03 15:13) - PLID 30498 - EMN IDs requested to save and release but waiting on end of modal loop!
	CMap<long, long, BOOL, BOOL> m_mapWaitingOnModalLoop;

	// (a.walling 2008-07-07 11:11) - PLID 30498 - List of IDs where write access has failed
	CMap<long, long, BOOL, BOOL> m_mapWriteAccessFailed;

	void TryAutoWriteAccess(CEMN* pNewEMN, CEMN* pOldEMN);

	// (j.jones 2012-09-28 15:15) - PLID 52922 - Used after something is added to an EMN
	// that might affect drug interactions. If the EMRDrugInteractionChecks preference is
	// set to save immediately, this function will perform that save.
	// CheckShowDrugInteractions will later show any interactions post-save.
	// (j.jones 2012-11-13 10:06) - PLID 52869 - changed to be only called via posted message,
	// the wParam must be the EMN pointer to save.
	// (j.jones 2013-02-06 15:13) - PLID 55045 - lParam is now a bool value to signify that
	// something definitely changed that can affect drug interactions. This is used mainly
	// for prescriptions that save in place without stating the EMN was modified.
	LRESULT CheckSaveEMNForDrugInteractions(WPARAM wParam, LPARAM lParam);

	// (j.jones 2012-09-28 09:27) - PLID 52820 - called after a save is completed where content
	// that affects drug interactions was changed, and also after closing/locking an EMN when drug
	// interactions changed while editing that EMN
	void CheckShowDrugInteractions(BOOL bIsSaving, BOOL bIsLocking, BOOL bIsClosing);

public:
	// (a.walling 2007-04-11 15:33) - PLID 25548 - Ensures that the topic row has loaded its subtopics, recursively throughout subtopics.
	void EnsureTopicRowLoadedAllSubTopics(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// (j.jones 2012-09-28 10:50) - PLID 52820 - track if drug interactions changed during this EMR session
	// (j.jones 2013-02-07 09:46) - PLID 55045 - made this public
	BOOL m_bDrugInteractionsChangedThisSession;

protected:
	NXDATALIST2Lib::IRowSettingsPtr CalcNextRow(NXDATALIST2Lib::IRowSettingsPtr pCurRow);
	NXDATALIST2Lib::IRowSettingsPtr CalcPreviousRow(NXDATALIST2Lib::IRowSettingsPtr pCurRow);
	// (c.haag 2007-10-11 15:46) - PLID 27509 - Returns the next visible row in the list
	NXDATALIST2Lib::IRowSettingsPtr GetNextVisibleRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
	// (c.haag 2007-10-11 16:07) - PLID 27509 - Returns the previous visible row in the list
	NXDATALIST2Lib::IRowSettingsPtr GetPrevVisibleRow(NXDATALIST2Lib::IRowSettingsPtr pRow);

	void HandleSelChanged(NXDATALIST2Lib::IRowSettingsPtr pOldRow, NXDATALIST2Lib::IRowSettingsPtr pNewRow);

	//TES 9/9/2009 - PLID 35495 - Moved GetEmrIcon() and m_Icons to EmrUtils, renamed to GetEmrTreeIcon and g_Icons
	
	// (a.walling 2008-04-29 09:18) - PLID 29815 - Icon size calculated per-emn.
	long m_nIconSize;

	// (z.manning, 05/15/2008) - PLID 30050 - Used to fill the background color
	CBrush m_brBackground;
	
	// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
	CEMNMoreInfoDlg *GetMoreInfoDlg(CEMN *pEMN, BOOL bCreateIfNotCreated = TRUE);

	//TES 2/18/2014 - PLID 60740 - Added Codes topic
	CEmrCodesDlg *GetCodesDlg(CEMN *pEMN, BOOL bCreateIfNotCreated = TRUE);

protected:
	// (a.walling 2008-08-15 13:22) - PLID 22049 - Helper function to get the EMN of a row
	CEMN* GetEMNFromRow(NXDATALIST2Lib::IRowSettings* pRow);

	NXDATALIST2Lib::IRowSettingsPtr AddTopicRow(CEMRTopic *pTopic, NXDATALIST2Lib::IRowSettingsPtr pParentRow, OPTIONAL NXDATALIST2Lib::IRowSettingsPtr pInsertBefore = NULL);

	//We don't want to change the selection when we're dragging.
	LPDISPATCH m_lpDraggingRow;
	CArray<NXDATALIST2Lib::IRowSettings*,NXDATALIST2Lib::IRowSettings*> m_arDragPlaceholders;

	//Sometimes while dragging we insert placeholder child nodes in case they want to drag underneath leaf nodes.
	//This function gets rid of all those (except the passed in row)
	void ClearDragPlaceholders(NXDATALIST2Lib::IRowSettings *pRowToPreserve = NULL);

	BOOL IsValidDrag(NXDATALIST2Lib::IRowSettings *pFromRow, NXDATALIST2Lib::IRowSettings *pToRow);

	BOOL m_bInitialLoadComplete;

	//nNewStatus may change if it is 2 (locked) and the user changes their mind.
	// (j.jones 2011-07-05 11:40) - PLID 43603 - this now takes in a class
	void ChangeStatus(IN OUT EMNStatus &emnNewStatus, LPDISPATCH lpEmnRow);

	//Utility function, puts a placeholder row as a child of the given parent row, so that it will have a plus sign.
	NXDATALIST2Lib::IRowSettingsPtr InsertPlaceholder(NXDATALIST2Lib::IRowSettings *pParentRow);

	//This function will get the last child row of pRow, iterating down to the lowest level (and loading subtopics as needed).
	NXDATALIST2Lib::IRowSettingsPtr GetAbsoluteLastChildRow(NXDATALIST2Lib::IRowSettingsPtr pRow);

	//Makes sure that the given row, as well as any potentially affected parent rows, displays as "modified" if it has been.
	void EnsureModifiedState(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void EnsureTopicModifiedState(CEMRTopic* pTopic);

	//Makes sure the given row has the correct icon.
	void EnsureRowIcon(NXDATALIST2Lib::IRowSettingsPtr pRow);

	//Checks the children of the given row for duplicated topic names, attempts to override them.
	void CheckDuplicateTopics(NXDATALIST2Lib::IRowSettingsPtr pParentRow);

	long m_nEMRSavePref;
	long m_nAutoCollapse;
	// (j.jones 2012-09-27 09:23) - PLID 52820 - store the drug interaction preference
	EMRDrugInteractionCheckType m_eEMRDrugInteractionChecksPref;

	// (j.jones 2012-10-25 16:24) - PLID 53322 - track whether we prompted for "has no allergies", such that
	// we never prompt twice in one EMR session
	BOOL m_bHasAnsweredHasNoAllergiesPrompt;

	// (j.jones 2012-10-29 08:58) - PLID 53324 - track whether we prompted for "has no current meds.", such that
	// we never prompt twice in one EMR session
	BOOL m_bHasAnsweredHasNoCurrentMedsPrompt;

	// (a.walling 2012-04-02 08:29) - PLID 49304 - Removed a lot of dead code regarding moving items

	// (a.walling 2007-10-15 12:17) - PLID 25548 - These are topics that could not find correct positioning in the preview
	// and are being held until all the spawning for the mint items is complete
	CArray<CEMRTopic*, CEMRTopic*> m_aryTopicsPendingInsertion;

	// (a.walling 2008-05-14 15:28) - PLID 29114 - details that need to be updated
	CMap<CEMNDetail*, CEMNDetail*, BOOL, BOOL> m_mapDetailsPendingUpdate;

	// (a.walling 2008-05-14 15:28) - PLID 29114 - Actually update pending details
	void UpdateDetail(CEMNDetail* pChangedDetail);

	// (a.walling 2008-05-14 16:31) - PLID 29114 - Delay (in ms) until refresh
	static long GetPreviewRefreshDelay(EmrInfoType eit);

	// (a.walling 2007-10-15 12:41) - PLID 25548 - Shared function for adding new topic previews from spawning
	void AddTopicPreview(CArray<CEMRTopic*,CEMRTopic*>* papTopics, BOOL bFinal);

	// (a.walling 2007-05-16 09:42) - PLID 25092 - Stores the colors of topics that may be greyed out
	// when dragging a detail
	CMap<NXDATALIST2Lib::IRowSettings*, NXDATALIST2Lib::IRowSettings*, OLE_COLOR, OLE_COLOR> m_mapTopicColors;

	// (z.manning, 09/19/2006) - PLID 22410 - Function that returns the display text for EMN rows, which
	// now includes the date, but we don't want the date to be editable from the tree.
	CString GetEmnRowDisplayText(CEMN* pEmn);

	// (a.walling 2012-07-03 10:56) - PLID 51284 - Removed m_pCurrentlyDisplayedRow
protected:
	// (a.walling 2007-08-31 15:18) - PLID 24733 - Save the entire EMN when the first save action occurs
	BOOL m_bEMNCompleteInitialSave;

	// (a.walling 2007-07-11 09:26) - PLID 26545 - Need to watch out for chart/category table changes
	CTableChecker m_checkCategories, m_checkCharts, m_checkCategoryChartLink;

public:
	// (a.walling 2007-07-11 09:29) - PLID 26545 - Ensure the EMN's charts and categories are valid, return a message
	CString CheckValidChartsCategories(CEMN* pEMN);

	// (j.jones 2007-07-23 11:00) - PLID 26742 - added medications/allergies info checkers
	// (j.jones 2010-06-21 15:21) - PLID 37981 - supported generic tables
	CTableChecker m_CurrentMedicationsInfoChecker, m_CurrentAllergiesInfoChecker, m_CurrentGenericTableInfoChecker;

public:
	// (c.haag 2007-08-30 17:02) - PLID 27058 - Returns TRUE if the E/M Checklist Setup dialog is visible
	BOOL IsEMREMChecklistDlgVisible();

	// (c.haag 2007-08-30 17:03) - PLID 27058 - Brings the E/M Checklist Setup window to the top of the desktop
	void BringEMREMChecklistDlgToTop();

protected:
	// (c.haag 2008-04-07 11:16) - PLID 29540 - Returns the EmrTreeWnd datalist 2 row that corresponds to pTopicToFind.
	// This is a recursive search, so the caller will need to pass in a row that is the topic's parent/grandparent/etc.
	NXDATALIST2Lib::IRowSettingsPtr GetRowFromTopic(NXDATALIST2Lib::IRowSettingsPtr pRow, CEMRTopic *pTopicToFind);

protected:

	void AutoCollapse();

	// (a.walling 2007-10-15 16:38) - PLID 27664 - Flag to suppress saving the preview for topic level saves
	void SaveRowObject(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bSilent, BOOL bSuppressPreviewSave = FALSE);

	//DRT 9/6/2007 - PLID 27310 - Helper function for the 2 "add procedure" messages
	void HandleNewProcedure(CEMN *pEMN, EMNProcedure *pProc);

	// (a.walling 2007-06-19 17:17) - PLID 25548
	BOOL m_bAutoScroll;

public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);

	//TES 8/13/2014 - PLID 63519 - Added support for EX tablecheckers
	LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);

	// (j.jones 2007-01-30 11:21) - PLID 24353 - we commit times when the dialog is closed
	void StopTrackingTimes();

	// (j.jones 2007-02-06 15:06) - PLID 24493 - get the EMR start time
	COleDateTime GetEMRStartEditingTime();
	void TryStartTrackingEMRTime();

	// (j.jones 2007-09-17 17:35) - PLID 27396 - send the server offset to the EMR
	void UpdateServerTimeOffset(COleDateTimeSpan dtOffset);

public:
	// (c.haag 2007-09-26 17:31) - PLID 27509 - This function is called when we want
	// to change the current tree selection. Unlike a direct call to PutCurSel() or
	// CurSel = ... this function will also handle the call to HandleSelChanged
	void SetTreeSel(NXDATALIST2Lib::IRowSettingsPtr pNewSel);

	// (z.manning, 05/15/2008) - PLID 30050 - Function to set the background color
	void SetBackgroundColor(COLORREF clr);

	// (j.jones 2010-03-31 17:33) - PLID 37980 - returns TRUE if this EMR has an EMN opened to a topic that is writeable
	BOOL HasWriteableEMRTopicOpen();

	// (j.jones 2012-03-27 14:59) - PLID 44763 - added global period warning
	void CheckWarnGlobalPeriod_EMR(COleDateTime dtToCheck);

	// (j.jones 2012-10-08 09:18) - PLID 36220 - exposed m_eEMRDrugInteractionChecksPref
	EMRDrugInteractionCheckType GetEMRDrugInteractionChecksPref();

	// (j.jones 2013-02-27 10:18) - PLID 55343 - moved E/M Checklist code to the treewnd
	void OpenEMChecklist(CEMN *pEMN);

	// (j.jones 2012-10-22 16:10) - PLID 52818 - this function will cause all open EMNs
	// in the current EMR to reload their medications from the database
	// (j.jones 2013-02-07 09:25) - PLID 55045 - now takes in an optional parameter to disable
	// showing drug interactions when the requery finishes
	void ReloadAllEMNMedicationsFromData(BOOL bShowDrugInteractions = TRUE);

	// (b.savon 2013-01-24 10:58) - PLID 54817 - Public facing to call from more info
	void ShowEPrescribing();

	// (b.savon 2014-02-25 14:19) - PLID 61029 - Emr Shared Summary of Car and Clinical Summary forwarder
	// (a.walling 2014-05-12 09:24) - PLID 61787 - Customized CCDA summaries
	void GenerateCareSummary(bool customize = false);
	void GenerateClinicalSummary(bool customize = false);

protected:
	// (c.haag 2007-09-26 17:35) - PLID 27509 - This function must be called when
	// changing the Visible state of a row. If the currently selected row, is made 
	// invisible, the datalist will internally change the current selection of the
	// list. This function is aware of that, and if the list selection needs to change
	// either because itself or one of its parents was made invisible, special handling
	// will be done to safely ensure that a valid topic is always visible.
	void SetTreeRowVisible(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bVisible);

	// (c.haag 2007-10-04 16:24) - PLID 27509 - This function safely removes a tree
	// row. If the currently selected row changes as a result of removing the row,
	// then m_pCurrentlyDisplayedRow will be updated, and we will post a message to
	// have the tree find a new selected topic
	void RemoveTreeRow(NXDATALIST2Lib::IRowSettingsPtr pRowToRemove);

	// (a.walling 2008-06-04 12:53) - PLID 22049
	void HandleEMNAccessTableChecker(long nEmnID);

	// (a.walling 2008-06-10 15:29) - PLID 22049
	// (a.walling 2008-07-03 12:35) - PLID 30498 - Pass whether we want to try to force if it failed
	void HandleWriteTokenRequestFailure(CEMN* pEMN, CWriteTokenInfo& wtInfo, BOOL bForce = FALSE);

	//TES 11/4/2009 - PLID 35807 - Call this before creating an EMN based on the template ID, if it returns false,
	// that means this was the NexWeb template and the user chose not to continue, so don't create the EMN.
	bool CheckIsNexWebTemplate(long nTemplateID);

	//(e.lally 2011-12-13) PLID 46968 - Moved OnToggleReadOnly code into a new function
	void HandleToggleReadOnly(bool bFailIfReadOnly = false);

	// (j.jones 2012-10-19 15:46) - PLID 52818 - split up E-Prescribing functionality into NewCrop vs. SureScripts,
	//with SureScripts being "NexTech E-Prescribing"
	void EPrescribeWithNewCrop();
	void EPrescribeWithSureScripts();

	//TES 8/13/2014 - PLID 63519 - Split out some code that responds to tablecheckers
	void HandleChangedPatient(CMap<CEMN*, CEMN*, long, long&> &mapEMNsToUpdateNarratives);

	// (c.haag 2007-05-31 12:49) - PLID 26175 - Added OnEmrTopicAddPreview 
	// (a.walling 2007-09-26 10:03) - PLID 27503 - Added OnQueryUnspawnEmns
	// (a.walling 2007-10-15 12:13) - PLID 25548 - Added /OnEmrTopicMintSpawnComplete/ renamed to OnEmrSpawningComplete
	// (a.walling 2007-10-15 14:44) - PLID 25548 - Added OnEmnDetailUpdatePreviewPosition
	// (a.walling 2008-06-02 12:17) - PLID 22049 - Added OnEmnLoadSaveComplete
	// (a.walling 2008-06-04 12:13) - PLID 22049 - Added OnTableChangedEx
	// (a.walling 2008-07-01 09:29) - PLID 30571 - Added OnUpdateEmrPreview to update the detail/topic in the EMR PreviewPane
	// (a.walling 2008-07-01 09:58) - PLID 30570 - Added OnHideTopicPreview
	// (a.walling 2008-07-01 09:58) - PLID 30570 - Added OnHideTopicTitle
	// (a.walling 2008-07-03 13:48) - PLID 30498 - Added OnForceAccessRequest - Handle request to release write access
	// (j.jones 2008-07-09 09:01) - PLID 24624 - Added OnPatientSummary
	// (j.jones 2008-07-17 13:04) - PLID 30729 - Added OnEMRProblemList
	// (j.jones 2008-07-23 11:17) - PLID 30819 - added OnEmnRefreshDiagCodes
	// (j.jones 2008-07-23 15:00) - PLID 30789 - added ability to manage EMN and Topic problems
	// (j.jones 2008-07-24 11:16) - PLID 30729 - added OnEmrProblemChanged
	// (j.jones 2008-07-28 10:56) - PLID 30773 - added OnEmrTopicChanged
	// (a.walling 2008-09-18 10:38) - PLID 26781 - Added NXM_EMR_LIMIT_RESOURCES handler
	// (j.jones 2009-03-03 12:07) - PLID 33308 - added OnEPrescribing
protected:
	//{{AFX_MSG(CEmrTreeWnd)
	afx_msg void OnSelChangedTree(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	virtual void OnDestroy();
	virtual void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRowExpandedTree(LPDISPATCH lpRow);
	afx_msg void OnNextNode();
	afx_msg void OnNewEmn();
	afx_msg void OnSelectEmnTemplate();
	afx_msg LRESULT OnPreDeleteEmrItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPostAddEmrItem(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnMergeOverrideChanged(WPARAM wparam, LPARAM lParam);
	afx_msg LRESULT OnEmnDetailDragBegin(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmnDetailDragEnd(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmnDetailUpdatePreviewPosition(WPARAM wParam, LPARAM lParam);
	virtual int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnEditingStartingTree(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishingTree(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedTree(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnDragBeginTree(BOOL FAR* pbShowDrag, LPDISPATCH lpRow, short nCol, long nFlags);
	afx_msg void OnDragOverCellTree(BOOL FAR* pbShowDrop, LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags);
	afx_msg void OnDragEndTree(LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnRButtonDownTree(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnOpenEmn();
	afx_msg void OnFinishEmn();
	afx_msg void OnLockEmn();
	afx_msg void OnSaveRow();
	// (a.walling 2008-12-30 17:04) - PLID 30252 - Change the EMN date from the treewnd
	afx_msg void OnChangeEmnDate();
	// (j.jones 2009-09-23 16:49) - PLID 29718 - added multiple options for changing dates
	afx_msg void OnSetEmnDateToday();
	afx_msg void OnSetEmnDateLastAppt();
	afx_msg void OnDeleteEmn();
	// (j.jones 2007-09-11 15:36) - PLID 27352 - added unique function for AddTopic vs. AddSubtopic
	afx_msg void OnAddNewTopic();
	afx_msg void OnAddNewSubtopic();
	// (a.walling 2012-03-29 08:04) - PLID 49297 - OnRequestClipRect is dead code
	afx_msg void OnSelChangingTree(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnDeleteTopic();
	afx_msg LRESULT OnEmrTopicAdded(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmrTopicAddPreview(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmrSpawningComplete(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmnChargeAdded(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmnChargeChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmnDiagAdded(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateCodeOfEMNDiagCode(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmnMedicationAdded(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmrProcedureAdded(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmrMultiProceduresAdded(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateNameOfEMNProcedure(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmnAdded(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPreDeleteEmnCharge(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPreDeleteEmnDiag(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPreDeleteEmnMedication(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPreDeleteEmnProcedure(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPostDeleteEmnProcedure(WPARAM wParam, LPARAM lParam); // (z.manning, 10/05/2007) - PLID 27630
	afx_msg LRESULT OnPreDeleteEmn(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPostDeleteEmn(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnQueryUnspawnEmns(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPreDeleteEmrTopic(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnHideEmrTopic(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmrItemEditContent(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmnStatusChanging(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmnMoreInfoChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmnCodesChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmnChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmrItemAdded(WPARAM wParam, LPARAM lParam);
	afx_msg void OnToggleShowIfEmpty();
	afx_msg void OnPreviousNode();
	afx_msg LRESULT OnEmnRefreshCharges(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmnRefreshPrescriptions(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmnRefreshDiagCodes(WPARAM wParam, LPARAM lParam);	
	afx_msg void OnEditMode();
	afx_msg void OnAddItemToTopic();
	afx_msg void OnSaveChanges();
	afx_msg void OnSaveAndClose();
	afx_msg void OnCreateToDo();
	afx_msg void OnRecordAudio();
	afx_msg void OnEPrescribing();
	afx_msg void OnPatientSummary();
	afx_msg void OnEMRDebug();
	afx_msg void OnEMChecklistSetup();
	afx_msg LRESULT OnEmrItemStateChanged(WPARAM wParam, LPARAM lParam);
	// (j.jones 2007-08-30 09:29) - PLID 27243 - Office Visit incrementing is no longer
	// used in the L2 EMR, it's in Custom Records only
	//afx_msg LRESULT OnProcessEMROfficeVisits(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRowCollapsedTree(LPDISPATCH lpRow);
	afx_msg void OnToggleHideOnEmn();
	afx_msg void OnCopyEmn();
	afx_msg LRESULT OnEmrItemChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmrTreeEnsureVisibleTopic(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTopicLoadComplete(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTopicLoadCompleteByPreloader(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTopicModifiedChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLoadEmnPreview(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRefreshTopicHTMLVisibility(WPARAM wParam, LPARAM lParam);
	afx_msg void OnCurSelWasSetTree();
	// (a.walling 2012-06-28 17:11) - PLID 51276 - More Info should be a clickable link
	afx_msg void OnLeftClickTree(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (j.jones 2007-04-03 14:53) - PLID 25464 - support for the splitter bar
	afx_msg void OnGotoPreview();
	afx_msg void OnAutoScroll();
	afx_msg void OnHideTopicPreview();
	afx_msg void OnHideTopicTitle();
	afx_msg void OnHideAllDetailsPreview();
	afx_msg void OnHideAllDetailsTitle();
	afx_msg void OnTopicColumnOne();
	afx_msg void OnTopicColumnTwo();
	// (a.walling 2009-07-06 10:10) - PLID 34793 - Clearing is deprecated
	/*
	afx_msg void OnTopicClearLeft();
	afx_msg void OnTopicClearRight();
	*/
	// (a.walling 2009-07-06 12:34) - PLID 34793 - Grouping for columns
	afx_msg void OnTopicGroupBegin();
	afx_msg void OnTopicGroupEnd();
	afx_msg void OnTopicTextRight();
	// (a.walling 2010-08-31 18:20) - PLID 36148 - Page breaks
	afx_msg void OnTopicNewPageBefore();
	afx_msg void OnTopicNewPageAfter();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnToggleReadOnly();
	afx_msg LRESULT OnEmnLoadSaveComplete(WPARAM wParam, LPARAM lParam);	
	afx_msg LRESULT OnEmnWriteAccessChanged(WPARAM wParam, LPARAM lParam);
	afx_msg void OnReloadEMN();
	afx_msg LRESULT OnWindowClosing(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTryAcquireWriteAccess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateEmrPreview(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnForceAccessRequest(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmnTodoAdded(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmnTodoDeleted(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmnTodoRefreshList(WPARAM wParam, LPARAM lParam);
	afx_msg void OnAddEmnProblem();
	afx_msg void OnEditEmnProblem();
	afx_msg void OnLinkEmnProblems();
	afx_msg void OnAddTopicProblem();
	afx_msg void OnEditTopicProblem();
	afx_msg void OnLinkTopicProblems();
	afx_msg LRESULT OnEmrProblemChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmrTopicChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmrMinimizePic(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageEmrSaveAll(WPARAM wParam, LPARAM lParam);
	// (j.jones 2009-04-01 10:15) - PLID 33736 - added OnNewCropBrowserClosed
	afx_msg LRESULT OnNewCropBrowserClosed(WPARAM wParam, LPARAM lParam);
	// (a.walling 2009-10-29 09:35) - PLID 36089 - Made InsertSignature generic
	afx_msg LRESULT OnInsertStockEmrItem(WPARAM wParam, LPARAM lParam); // (z.manning 2009-08-26 10:36) - PLID 33911
	afx_msg void OnPositionTopics();
	// (c.haag 2013-03-07) - PLID 55365 - Repurposed for printing one EMN with multiple layouts, or multiple EMN's
	afx_msg LRESULT OnPrintMultiple(WPARAM wParam, LPARAM lParam); // (a.walling 2009-11-24 13:39) - PLID 36418
	// (j.jones 2010-04-01 10:41) - PLID 37980 - added ability to tell the topic to add a given image
	afx_msg LRESULT OnAddImageToEMR(WPARAM wParam, LPARAM lParam);
	// (a.walling 2010-04-13 13:20) - PLID 37150 - The EMR is entirely loaded
	afx_msg LRESULT OnEMRLoaded(WPARAM wParam, LPARAM lParam);
	// (j.jones 2010-06-21 10:22) - PLID 39010 - added ability to add a generic table to the EMR
	afx_msg LRESULT OnAddGenericTableToEMR(WPARAM wParam, LPARAM lParam);
	//(e.lally 2011-12-13) PLID 46968 - Makes the existing patient EMN available for a patient to fill out online via NexWeb
	afx_msg void OnPublishToNexWebPortal();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRTREEWND_H__1672E883_8B0C_46DF_8FF8_0D033F61EEC8__INCLUDED_)
