#if !defined(AFX_EMRTOPICWND_H__42FA0749_5423_4AAF_BA52_EA5BBAA52E58__INCLUDED_)
#define AFX_EMRTOPICWND_H__42FA0749_5423_4AAF_BA52_EA5BBAA52E58__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrTopicWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEmrTopicWnd window

//#include "EmrUtils.h"
#include "DevicePluginUtils.h"
#include "EmrTopicHeaderWnd.h"

class CEMR;
class CEMRTopic;
class CEMNDetail;
class CEmrEditorDlg;
class CSignatureDlg;

class CEmrTopicWnd;
class CEmrProblem;

////

// (b.savon 2012-06-06 12:28) - PLID 49144 - Our DI settings struct
struct DeviceImportSettings{
	
	DeviceImportSettings()
	{
		m_nLeft = -1;
		m_nTop = -1;
		m_nDeviceTop = -1;
	}

	DeviceImportSettings(long nLeft, long nTop)
	{
		m_nLeft = nLeft;
		m_nTop = nTop;
		m_nDeviceTop = -1;
	}

	long m_nLeft;
	long m_nTop;
	long m_nDeviceTop;
};

// (b.savon 2014-07-14 07:46) - PLID 62743 - Create an enum for preference
enum ClinicalSummaryAfterSignature{
	csasNever = 0,
	csasAlways,
	csasPrompt,
};

class CEmrTopicWnd : public CWnd, public Emr::TopicInterfaceAccessImpl<CEmrTopicWnd>, public boost::enable_shared_from_this<CEmrTopicWnd>
{
// Construction
public:
	friend class CEmrTopicView;

	CEmrTopicWnd();

	DECLARE_DYNCREATE(CEmrTopicWnd);
// Attributes
public:
	// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
	void Initialize(CEMRTopic *pTopic, CWnd* pParent, class CEmrTreeWnd *pTreeWnd, CRect rcWindow, BOOL bIsTemplate, BOOL bIsReadOnly, BOOL bAllowEdit);

protected:
	class CEmrTreeWnd* m_pEmrTreeWnd;

	CSize m_szContentSize;
	long m_nMinContentHeight;

public:
	void OnActiveTopicChanged();

	const CSize& GetContentSize() const {
		return m_szContentSize;
	}

	CSize GetDisplayContentSize() const {		
		CSize szDisplayContentSize = m_szContentSize;
		if (szDisplayContentSize.cy < m_nMinContentHeight) {
			szDisplayContentSize.cy = m_nMinContentHeight;
		}
		return szDisplayContentSize;
	}

	CEmrTreeWnd* GetEmrTreeWnd()
	{
		return m_pEmrTreeWnd;
	}

	void SetReadOnly(BOOL bIsReadOnly);
	void SetAllowEdit(BOOL bAllowEdit);

	BOOL IsReadOnly() const
	{
		return m_bIsReadOnly;
	}

	// (j.jones 2007-07-26 09:10) - PLID 24686 - converted RepositionDetailsInTopic into two functions,
	// accepting an InfoID or a MasterID
	// (c.haag 2011-11-01) - PLID 46223 - Added pDetailToPreventLoadContentAndState
	void RepositionDetailsInTopicByInfoID(const long nReloadContentForInfoID, const BOOL bSetRegionAndInvalidate, OPTIONAL IN CEMNDetail *pOverrideDetail = NULL, OPTIONAL IN LPCRECT prcOverrideDetailRect = NULL, BOOL bMaintainImagesSize = FALSE, CEMNDetail* pDetailToPreventLoadContentAndState = NULL);
	void RepositionDetailsInTopicByInfoMasterID(const long nReloadContentForInfoMasterID, const BOOL bSetRegionAndInvalidate, OPTIONAL IN CEMNDetail *pOverrideDetail = NULL, OPTIONAL IN LPCRECT prcOverrideDetailRect = NULL, BOOL bMaintainImagesSize = FALSE, CEMNDetail* pDetailToPreventLoadContentAndState = NULL);
	// (j.jones 2007-07-26 09:28) - PLID 24686 - created non-public function to accept an InfoID or InfoMasterID
	// (c.haag 2011-11-01) - PLID 46223 - Added pDetailToPreventLoadContentAndState
	void RepositionDetailsInTopicByInfoOrMasterID(const long nReloadContentForInfoID, const long nReloadContentForInfoMasterID, const BOOL bSetRegionAndInvalidate, OPTIONAL IN CEMNDetail *pOverrideDetail = NULL, OPTIONAL IN LPCRECT prcOverrideDetailRect = NULL, BOOL bMaintainImagesSize = FALSE, CEMNDetail* pDetailToPreventLoadContentAndState = NULL);

	void PostButtonClickedMergeConflict();
	
	void SetHighlightColor(COLORREF colorHighlight);

	//Make the given detail visible.
	void ShowDetail(CEMNDetail *pDetail, BOOL bIsInitialLoad);

	void DeleteItem(CEMNDetail *pItemToDelete);

	//Adds a new detail
	void AddDetail(CEMNDetail *pDetail);

	// (a.walling 2012-03-29 08:04) - PLID 49297 - Handle the new rect of an item that is being sized
	// (a.walling 2012-04-02 08:29) - PLID 49304 - Unified size/move handler
	void HandleItemSizeMove(class CEmrItemAdvDlg* pWnd, UINT nSide, LPRECT lprcNewArea);
	void NotifyItemPosChanged(class CEmrItemAdvDlg* pWnd);

	//TES 10/30/2008 - PLID 31269 - Call after a problem gets changed to refresh all the problem icons.
	void HandleProblemChange(CEmrProblem* pChangedProblem);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrTopicWnd)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEmrTopicWnd();

	CEMRTopic* GetTopic() const
	{
		return m_pTopic;
	}

	// Generated message map functions
protected:
	CEMRTopic *m_pTopic;
	BOOL m_bIsTemplate;
	BOOL m_bIsReadOnly;
	BOOL m_bAllowEdit;

	COLORREF m_colorHighlight;

	LRESULT OnEmrItemRemove(WPARAM wParam, LPARAM lParam);

	//Called when the user right-clicks and selects "Insert Image"

	// (b.savon 2012-06-06 12:29) - PLID 49144 - Add optional DI flag
	// (j.jones 2010-04-01 10:57) - PLID 37980 - Added optional overrides to default an image file and a label
	void AddImage(OPTIONAL CString strImageFile = "", OPTIONAL CString strLabel = "", OPTIONAL BOOL bDeviceImport = FALSE);
	// (c.haag 2008-06-05 17:41) - PLID 30319 - Called when the user right-clicks and selects "Add Text Macro To Topic"
	void AddTextMacro();

public:
	// (z.manning 2008-10-17 10:17) - PLID 23110 - Adds a signature image to the current topic
	// (z.manning 2011-10-31 10:57) - PLID 44594 - Made this function public
	void AddSignatureImage();

	// (z.manning 2011-10-31 14:05) - PLID 44594 - Added an overload that takes a signature dialog
	// (j.jones 2013-08-08 13:35) - PLID 42958 - added optional user ID and username, for cases when
	// the signature is being created by a user who is not the logged-in user
	void AddSignatureImage(CSignatureDlg *pdlgSignature,
		OPTIONAL long nSignatureUserID = GetCurrentUserID(),
		OPTIONAL CString strSignatureUserName = GetCurrentUserName());

	// (j.jones 2013-08-07 14:37) - PLID 42958 - added ability to let another user sign an EMN
	void AddSignatureImageForAnotherUser();

protected:
	// (j.jones 2010-06-21 10:22) - PLID 39010 - added ability to add a generic table to the EMR
	void AddGenericTable(DevicePluginUtils::TableContent *pGenericTableContent, CString strTableLabel);
	
	//used only as workaround fror MFC bug
	// (a.walling 2012-05-07 09:21) - PLID 48934 - This bug is now fixed via CNxOccManager
	//CNxStatic m_wndInvisibleAnchor;

	//CBrush m_brBackground;

	// (j.jones 2008-07-24 11:16) - PLID 30729 - added OnEmrProblemChanged
	// (j.jones 2008-07-28 10:28) - PLID 30773 - added OnEmnMoreInfoChanged, OnEmnChanged, OnEmrTopicChanged,
	// OnEmnRefreshCharges, OnEmnRefreshPrescriptions, and OnEmnRefreshDiagCodes
	// (z.manning 2009-08-26 10:01) - PLID 33911 - Added OnInsertSignature
	// (a.walling 2009-10-29 09:35) - PLID 36089 - Made InsertSignature generic
	//{{AFX_MSG(CEmrTopicWnd)
	virtual void OnSize(UINT nType, int cx, int cy);	

	// (a.walling 2012-06-22 14:01) - PLID 51150 - Forward these messages to the tree wnd
	LRESULT ForwardMessageToTreeWnd(UINT nMsg, WPARAM wParam, LPARAM lParam);

	//afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	// (a.walling 2012-06-22 14:01) - PLID 51150 - Forward current message to the tree wnd
	afx_msg LRESULT ForwardCurrentMessageToTreeWnd(WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT OnEmrItemAdded(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnConvertRectForData(WPARAM wParam, LPARAM lParam);
	virtual void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	// (a.walling 2012-03-22 16:50) - PLID 49141 - All more info changed messages go to the EMN's interface now
	afx_msg LRESULT OnInsertStockEmrItem(WPARAM wParam, LPARAM lParam);
	// (j.jones 2010-04-01 10:41) - PLID 37980 - added ability to tell the topic to add a given image
	afx_msg LRESULT OnAddImageToEMR(WPARAM wParam, LPARAM lParam);
	// (j.jones 2010-06-21 10:22) - PLID 39010 - added ability to add a generic table to the EMR
	afx_msg LRESULT OnAddGenericTableToEMR(WPARAM wParam, LPARAM lParam);
	// (a.walling 2011-03-19 12:23) - PLID 42694 - Added handlers for WM_SETCURSOR and the various mouse messages
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg LRESULT OnNcHitTest(CPoint pt);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

typedef shared_ptr<CEmrTopicWnd> CEmrTopicWndPtr;
typedef weak_ptr<CEmrTopicWnd> CEmrTopicWndRef;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRTOPICWND_H__42FA0749_5423_4AAF_BA52_EA5BBAA52E58__INCLUDED_)
