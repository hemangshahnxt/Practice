#pragma once

#include <memory>
#include "EmrDocument.h"
#include "EmrDocTemplate.h"
#include "EmrTreePane.h"
#include "EmrPreviewPane.h"
#include "EmrPhotosPane.h"
#include "EmrClassicButtonPane.h" //(e.lally 2012-02-16) PLID 48065
#include "EmrMUProgressPane.h" //(e.lally 2012-02-23) PLID 48016

// (a.walling 2012-04-09 14:34) - PLID 49527 - NxAdvancedUI
#include <NxAdvancedUILib/NxRibbonBar.h>
#include <NxAdvancedUILib/NxMDIFrameWndEx.h>
#include <NxAdvancedUILib/NxMessageBar.h>
// (a.walling 2012-05-22 15:15) - PLID 50582 - NxRibbonStatusBar status bar
#include <NxAdvancedUILib/NxRibbonStatusBar.h>
#include <NxUILib/SafeMsgProc.h>

#include "EmrUIState.h"

class CEmrMedAllergyViewerDlg;
class CEMRBarcodeDlg;

class CEMRTopic;
class CEMN;

namespace EmrContextCategory
{
	enum Category : UINT {
		None				= 0x0,
		Begin				= 0x1,

		// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
		PreviewPane			= 0x1,
		PhotosPane			= 0x2, //(e.lally 2012-04-12) PLID 49566 

		End					= 0x8, // one bit pos past the last value
	};
};

// CEmrFrameWnd frame

// (a.walling 2012-02-28 14:53) - PLID 48451 - moved patient-related logic here to CEmrPatientFrameWnd

// (a.walling 2012-02-29 06:42) - PLID 46644 - TreePane sets m_bIsTemplate in constructor; moved some construction logic to derived classes
// (a.walling 2012-03-02 16:51) - PLID 48598

// (a.walling 2012-04-17 16:56) - PLID 49750 - Use CNxMDIFrameWndEx base class

// (a.walling 2012-07-05 10:54) - PLID 50853 - Use SafeMsgProc to implement try/catch around window messages and commands
class CEmrFrameWnd : public SafeMsgProc<CNxMDIFrameWndEx>
{
	DECLARE_DYNAMIC(CEmrFrameWnd)
protected:
	CEmrFrameWnd(bool bIsTemplate);
	virtual ~CEmrFrameWnd();

public:
	void InitializeEmrFrame();

	bool IsTemplate()
	{
		return m_bIsTemplate;
	}

	// (a.walling 2012-07-03 10:56) - PLID 51284 - Fixing activation ambiguities
	void Activate(CWnd* pWnd);

	bool IsEditMode() const
	{
		return m_bEditMode;
	}

	bool IsClosing() const
	{
		return !!m_bClosing;
	}

	CNxRibbonBar& GetRibbonBar()
	{
		return m_wndRibbonBar;
	}

	CNxRibbonStatusBar& GetStatusBar()
	{
		return m_wndStatusBar;
	}


protected:
	void ShowContextCategory(EmrContextCategory::Category category, bool bShow = true);
	void HideContextCategory(EmrContextCategory::Category category);

	// (a.walling 2012-07-03 10:56) - PLID 51284 - Keep track of last active EMN / topic -- should only be used for comparison (might be destroyed)
	CEMN* m_pLastActiveEMN;
	CEMRTopic* m_pLastActiveTopic;

	// (a.walling 2012-10-01 09:15) - PLID 52119 - Maintain UI state somewhere accessible a bit easier outside of the frame wnd
	Emr::UIStateMap m_uiState;
	

	// (j.dinatale 2012-09-18 14:21) - PLID 52702 - store the EMR Save Pref for reference later
	long m_nEMRSavePref;
	
	// (e.lally 2012-04-03) PLID 48891
	// (a.walling 2012-07-03 10:56) - PLID 51284 - virtual events can be overridden by derived classes
	// (j.dinatale 2012-09-18 16:56) - PLID 52702 - pass in to and from pointers
	virtual void OnActiveEMNChanged(CEMN *pFrom, CEMN *pTo);
	virtual void OnActiveTopicChanged(CEMRTopic *pFrom, CEMRTopic *pTo);

	UINT m_contextCategories;
	//(e.lally 2012-04-12) PLID 49566 - Made this virtual
	virtual void UpdateContextCategories() {}
	void RefreshContextCategories();

	DECLARE_MESSAGE_MAP()

	void Create();

	bool m_bIsTemplate;
	
	scoped_ptr<CEmrDocTemplate> m_pEmrTopicViewDocTemplate;

	// (a.walling 2014-04-04 16:36) - PLID 61670 - extends CMFCApplicationButton to support > 1 image for multiple states
	CNxRibbonApplicationButton m_mainButton;
	CNxRibbonBar m_wndRibbonBar;

	// (a.walling 2011-12-09 10:16) - PLID 46642 - Artwork
	CMFCToolBarImages m_ribbonToolImages; 
	
	CEmrTreePane			m_emrTreePane;
	CEmrPreviewPane			m_emrPreviewPane;
	CEMRClassicButtonPane	m_emrClassicButtonPane; //(e.lally 2012-02-16) PLID 48065
	// (a.walling 2012-05-22 15:15) - PLID 50582 - NxRibbonStatusBar status bar
	CNxRibbonStatusBar			m_wndStatusBar;	
	// (a.walling 2011-12-12 09:15) - PLID 46645 - Message bar
	// (a.walling 2012-04-24 14:29) - PLID 50485 - Now uses a CNxMessageBar
	CNxMessageBar			m_wndMessageBar; 

	afx_msg LRESULT OnGetTabToolTip(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);

	// (a.walling 2012-03-20 09:24) - PLID 48990 - Called before a drop down or menu is displayed; ensure it has all necessary data available via SynchronizeDelayedRibbonSubitems
	afx_msg LRESULT OnBeforeShowRibbonItemMenu(WPARAM wParam, LPARAM lParam);
	// (a.walling 2012-03-20 09:24) - PLID 48990 - Ensure the ribbon item has its menu fully populated with all data
	virtual void SynchronizeDelayedRibbonSubitems(CMFCRibbonBaseElement* pElement);

	// (a.walling 2012-04-06 13:19) - PLID 48990 - Ensure the most common lists are cached
	virtual void EnsureCachedData() {}
	
	// (a.walling 2012-03-26 12:49) - PLID 49205 - Chart, category
	void SynchronizeDelayedChartSubitems(CMFCRibbonBaseElement* pElement);
	// (a.walling 2012-03-26 12:49) - PLID 49205 - Chart, category
	void SynchronizeDelayedCategorySubitems(CMFCRibbonBaseElement* pElement);
	
	virtual void InitializeRibbon() = 0;
	virtual void InitializeStatusBar() = 0;
	virtual void InitializeRibbonTabButtons() {}
	virtual void InitializePanes() = 0;
	// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
	std::auto_ptr<CMFCRibbonButtonsGroup> CreateNavigationButtonsGroup();

	void InitializeCaptionBar(); // (a.walling 2011-12-12 09:15) - PLID 46645 - Message bar
	virtual CString GenerateTitleBarText() = 0;
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL OnShowMDITabContextMenu(CPoint point, DWORD dwAllowedItems, BOOL bDrop);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	virtual void PostNcDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	// (a.walling 2012-03-12 17:21) - PLID 46076 - Apply our icon within PreCreateWindow since IDR_EMRFRAME does not have an icon
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	// (a.walling 2012-03-05 13:14) - PLID 48615 - calls TranslateAppCommand, checks via UpdateCmdUI that the command is enabled, posts WM_COMMAND if so
	afx_msg LRESULT OnAppCommand(WPARAM wParam, LPARAM lParam);

	// (a.walling 2012-03-05 13:14) - PLID 48615 - translate appcommand into command id; return 0 for no handler.
	virtual UINT TranslateAppCommand(HWND hwndFrom, UINT nCmd, UINT nDevice, UINT nKey);

	////
	/// UI state helpers

	// (a.walling 2012-03-23 15:27) - PLID 49187 - Audit history
	static bool CanAccess_EMRAuditHistory(bool bSilent = true);
	static bool CheckAccess_EMRAuditHistory()
	{
		return CanAccess_EMRAuditHistory(false);
	}

	// (a.walling 2012-03-23 16:04) - PLID 49190 - Confidential info
	static bool CanAccess_ConfidentialInfo(bool bSilent = true);
	static bool CheckAccess_ConfidentialInfo()
	{
		return CanAccess_ConfidentialInfo(false);
	}

	////
	/// UI state

	afx_msg void OnUpdate_AlwaysEnable(CCmdUI* pCmdUI);

	afx_msg void OnUpdateSave(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSaveAll(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCloseEMR(CCmdUI* pCmdUI);

	afx_msg void OnUpdateMdiTabbedDocument(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditMode(CCmdUI* pCmdUI);

	afx_msg void OnUpdateViewFullScreen(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewMessageBar(CCmdUI* pCmdUI); // (a.walling 2011-12-12 09:15) - PLID 46645 - Message bar

	afx_msg void OnUpdateViewPreviewPane(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewTreePane(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewClassicButtonPane(CCmdUI* pCmdUI); //(e.lally 2012-02-16) PLID 48065

	afx_msg void OnUpdateWriteAccess(CCmdUI* pCmdUI);
	afx_msg void OnUpdateChart(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCategory(CCmdUI* pCmdUI);

	// (a.walling 2012-06-28 17:13) - PLID 51277 - More Info button
	afx_msg void OnUpdateMoreInfo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDescription(CCmdUI* pCmdUI);

	//TES 2/13/2014 - PLID 60749 - Codes button
	afx_msg void OnUpdateCodes(CCmdUI* pCmdUI);

	// (r.gonet 08/03/2012) - PLID 51027 - Wound Care Auto Coding Button
	afx_msg void OnUpdateWoundCareAutoCoding(CCmdUI* pCmdUI);

	afx_msg void OnUpdateEmrDebug(CCmdUI* pCmdUI);

	// (a.walling 2012-03-07 08:36) - PLID 48680 - Preview pane commands and options
	afx_msg void OnUpdatePreviewPanePrint(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePreviewPanePrintPreview(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePreviewPanePrintMultiple(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePreviewPaneConfigure(CCmdUI* pCmdUI);

	afx_msg void OnUpdatePreviewPaneAutoScroll(CCmdUI* pCmdUI);

	//TES 1/17/2014 - PLID 60397 - Option to hide the EMN title on the preview pane, per template
	afx_msg void OnUpdateTemplateHideTitle(CCmdUI* pCmdUI);

	////
	/// UI Commands

	// (a.walling 2012-07-16 08:56) - PLID 51547 - Reset layout command
	afx_msg void OnResetLayout();

	afx_msg void OnSave();
	afx_msg void OnSaveAll();
	afx_msg void OnSaveAndClose();
	afx_msg void OnCloseEMR();

	afx_msg void OnMdiTabbedDocument();
	afx_msg void OnMdiMoveToNextGroup();
	afx_msg void OnMdiMoveToPrevGroup();
	afx_msg void OnMdiNewHorzTabGroup();
	afx_msg void OnMdiNewVertGroup();

	afx_msg void OnEditMode();

	afx_msg void OnViewFullScreen();
	afx_msg void OnViewMessageBar(); // (a.walling 2011-12-12 09:15) - PLID 46645 - Message bar

	afx_msg void OnViewPreviewPane();
	afx_msg void OnViewTreePane();
	afx_msg void OnViewClassicButtonPane();  //(e.lally 2012-02-16) PLID 48065

	afx_msg void OnWriteAccess();

	// (a.walling 2012-03-26 12:49) - PLID 49205 - Chart, category
	afx_msg void OnChart();
	afx_msg void OnCategory();

	// (a.walling 2012-06-28 17:13) - PLID 51277 - More Info button
	afx_msg void OnMoreInfo();
	afx_msg void OnDescription();

	//TES 2/13/2014 - PLID 60749 - Codes button
	afx_msg void OnCodes();

	// (r.gonet 08/03/2012) - PLID 51027 - Wound Care Auto Coding Button
	afx_msg void OnWoundCareAutoCoding();

	afx_msg void OnEmrDebug();

	// (a.walling 2012-03-07 08:36) - PLID 48680 - Preview pane commands and options
	afx_msg void OnPreviewPanePrint();
	afx_msg void OnPreviewPanePrintPreview();
	afx_msg void OnPreviewPanePrintMultiple();
	afx_msg void OnPreviewPaneConfigure();

	afx_msg void OnPreviewPaneAutoScroll();

	//TES 1/17/2014 - PLID 60397 - Option to hide the EMN title on the preview pane, per template
	afx_msg void OnTemplateHideTitle();

protected:

	void SetEditMode(bool bEditMode);
	bool m_bEditMode;

protected:
	virtual void OnDatabaseReconnect() {};	// (j.armen 2012-06-12 08:45) - PLID 48808
	void DoActivateEMN(CEMN* pEMN, CEMRTopic* pTopic, bool bActivateTopicView = true);

	// (a.walling 2013-02-13 09:30) - PLID 52629 - Dynamic view layout - create views from path
	virtual CMDIChildWndEx* CreateDocumentWindow(LPCTSTR lpcszDocName, CObject* pObj) override;
	virtual CMDIChildWndEx* CreateNewWindow(LPCTSTR lpcszDocName, CObject* pObj) override;
	
	// (a.walling 2013-04-17 17:25) - PLID 52629 - Modify the tab groups before opening
	virtual void PreOpenTabGroups(std::vector<Nx::MDI::TabGroup>& tabGroups) override;

public:
	// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
	void ShowMoreInfo(CEMN* pEMN);
	//TES 2/12/2014 - PLID 60740 - Added for the new <Codes> topic
	void ShowCodesView(CEMN* pEMN);
	void ActivateTopic(CEMRTopic* pTopic, bool bActivateTopicView = true);
	void ActivateEMN(CEMN* pEMN, CEMRTopic* pTopic, bool bActivateTopicView = true);
	void ActivateEMNOnly(CEMN* pEMN);
	// (a.walling 2013-01-17 12:36) - PLID 54666 - ShowEMN will behave like EmrTreeWnd's ShowEMN, such as when first creating an EMN
	void ShowEMN(CEMN* pEMN);

	bool IsPreviewPaneActive();


	CEmrTopicView* GetEmrTopicView(CEMN* pEMN, bool bAutoCreate = false);
	CEmrTopicView* EnsureEmrTopicView(CEMN* pEMN)
	{
		return GetEmrTopicView(pEMN, true);
	}
	bool HasEmrTopicView();

	// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
	CEmrMoreInfoView* GetEmrMoreInfoView(CEMN* pEMN, bool bAutoCreate = false);
	CEmrMoreInfoView* EnsureEmrMoreInfoView(CEMN* pEMN)
	{
		return GetEmrMoreInfoView(pEMN, true);
	}

	// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
	CEMNMoreInfoDlg* GetMoreInfoDlg(CEMN* pEMN, bool bAutoCreate = false);
	CEMNMoreInfoDlg* EnsureMoreInfoDlg(CEMN* pEMN)
	{
		return GetMoreInfoDlg(pEMN, true);
	}

	//TES 2/12/2014 - PLID 60740 - Added for the new <Codes> topic
	CEmrCodesView* GetEmrCodesView(CEMN* pEMN, bool bAutoCreate = false);
	CEmrCodesView* EnsureEmrCodesView(CEMN* pEMN)
	{
		return GetEmrCodesView(pEMN, true);
	}
	//TES 2/12/2014 - PLID 60740 - Added for the new <Codes> topic
	CEmrCodesDlg* GetEmrCodesDlg(CEMN* pEMN, bool bAutoCreate = false);
	CEmrCodesDlg* EnsureEmrCodesDlg(CEMN* pEMN)
	{
		return GetEmrCodesDlg(pEMN, true);
	}

	CEmrEditorBase* GetEmrEditorBase()
	{
		return m_emrTreePane.GetEmrEditorBase();
	}

	class CEMRPreviewCtrlDlg* GetEmrPreviewCtrl();
	class CEmrTreeWnd* GetEmrTreeWnd();
	class CEmrTopicView* GetActiveEmrTopicView();

	BOOL HasEditableEMN();	// (j.armen 2012-07-02 11:30) - PLID 49831
	CEMN* GetActiveEMN();
	//(e.lally 2012-05-07) PLID 48951 - Renamed Writable to Editable to be consistent with the EMN.cpp code.
	//	This function silently checks write token, locked status, licensing and permissions
	CEMN* GetActiveEditableEMN();

	CEMR* GetActiveEMR();

	// (j.dinatale 2012-09-18 15:13) - PLID 52702 - be able to retrieve the cached value for the EMR save preference
	long GetEMRSavePref();
};

inline void CEmrFrameWnd::ShowContextCategory(EmrContextCategory::Category category, bool bShow)
{
	if (bShow) {
		m_contextCategories |= category;
	} else {
		m_contextCategories &= ~category;
	}
}

inline void CEmrFrameWnd::HideContextCategory(EmrContextCategory::Category category)
{
	ShowContextCategory(category, false);
}

