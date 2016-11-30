#pragma once

#include "EmrItemAdvImageDlg.h"
#include "EmrItemAdvTableBase.h"
#include "EmrItemAdvNarrativeBase.h"
#include "EmrItemAdvTextDlg.h"

// (a.walling 2010-05-19 08:26) - PLID 38558 - Include NxInkPictureImport.h rather than #import "NxInkPicture.tlb" so the proper tlb is chosen based on current SDK path (patch or main)
#include "NxInkPictureImport.h"
#include <set>

class CEMRItemAdvPopupDlg;

/////////////////////////////////////////////////////////////////////////////
// CEMRItemAdvPopupWnd window

class CEMRItemAdvPopupWnd :
	public CWnd, 
	public CEmrItemAdvTableBase,
	public CEmrItemAdvNarrativeBase // (c.haag 2008-11-25 11:31) - PLID 31693 - Support for narrative functionality
{
// Construction
public:
	CEMRItemAdvPopupWnd();

private:
	// (c.haag 2008-01-15 17:42) - PLID 17936 - This function returns the source
	// string for a dropdown column given a column ID. The string is generated
	// from information stored inside a member map where the key is the column ID.
	// (z.manning 2011-10-11 11:14) - PLID 42061 - Added stamp ID
	CString GetDropdownSource(long nColumnID, const long nStampID);

// Attributes
public:

	CEMNDetail *m_pDetail; //This dialog assumes that m_pDetail is an independently loaded copy, so, for example, it will lock spawning and never unlock it.
	//TES 4/6/2007 - PLID 25456 - The above comment is no longer quite true; in the case of narratives, m_pDetail is NOT
	// an independently loaded copy, so this variable must be set to FALSE so that the dialog knows not to, for example,
	// lock spawning and never unlock it.
	CEMNDetail *m_pRealDetail; // (a.walling 2008-01-16 15:53) - PLID 14982

	// (a.walling 2008-02-07 10:36) - PLID 14982 - A list of actual selections on the non-popped up item
	CDWordArray m_dwaRealSelectedDataGroupIDs;

	BOOL m_bIsDetailIndependent;

	//TES 1/15/2008 - PLID 24157 - This is now a CWnd* instead of a CEMRItemAdvPopupDlg*, see the comments on Initialize().
	CWnd *m_pParentDlg;

	long m_clrHilightColor;
	CBrush m_brHilight;
	//TES 1/15/2008 - PLID 24157 - This function should be called to set the hilight color if the window is already
	// created, it will recreate the hilight brush and redraw the screen using it, if necessary.
	void SetHilightColor(long clrHilight);

	CArray<CWnd*,CWnd*> m_arypControls;

	void CreateControls();
	void DestroyControls();

	NxWindowlessLib::NxFreeLabelControl m_wndLabel;

	//for text
	// (j.jones 2011-03-23 10:12) - PLID 42965 - this is now a CEmrItemAdvTextEdit
	CEmrItemAdvTextEdit m_edit;
	// (z.manning 2008-12-16 12:51) - PLID 27682 - Added spell check icon to popped up text boxes
	CNxIconButton m_btnSpellCheck;
	// (z.manning 2009-09-21) - PLID 33612 - Lab button
	CNxIconButton m_btnOpenLab;

public:
	// (c.haag 2008-10-21 09:54) - PLID 31700 - This was merged into EmrItemAdvTableBase, but outside callers need to access
	// the function as well.
	void UpdateLinkedItemList(CEMNDetail* pDetail);

	// (a.walling 2008-12-19 09:30) - PLID 29800 - Set custom stamps on the image
	void SetCustomStamps(LPCTSTR szNewStamps);

	// (r.gonet 02/14/2012) - PLID 37682
	void SetImageTextStringFilter(CTextStringFilterPtr pTextStringFilter);

	// (r.gonet 02/14/2013) - PLID 40017 - Sets the flag to remember the column widths in a popped up table.
	void SetRememberPoppedUpTableColumnWidths(BOOL bRemember);

public:
	//DRT 7/28/2008 - PLID 30824 - Converted to datalist2
	CString GenerateNewVarState();

	//for image
	long m_nCurPenColor;
	float m_fltCurPenSize;
	BOOL m_bIsEraserState;

	//for sliders
	CNxSliderCtrl m_Slider;
	CNxStatic m_Caption;
	CNxStatic m_MinCaption;
	CNxStatic m_MaxCaption;
	int ValueToSliderPos(double dValue);
	double SliderPosToValue(int nSliderPos);

	//TES 4/5/2007 - PLID 25456 - For narratives, copied out of CEMRItemAdvNarrativeDlg
	CWnd m_wndRichEdit;
	RICHTEXTEDITORLib::_DRichTextEditorPtr m_RichEditCtrl;

	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated massive amounts of recordset-related narrative code

	// (a.walling 2010-06-21 15:07) - PLID 38779 - This is never called with not bCalcOnly
	BOOL RepositionTextControls(IN OUT CSize &szArea);
	BOOL RepositionListControls(IN OUT CSize &szArea);
	BOOL RepositionImageControls(IN OUT CSize &szArea);
	BOOL RepositionSliderControls(IN OUT CSize &szArea);
	BOOL RepositionTableControls(IN OUT CSize &szArea);
	BOOL RepositionNarrativeControls(IN OUT CSize &szArea);
	void CallRepositionControls();

	//TES 1/23/2008 - PLID 24157 - Needed a master function to update the display when the state changes.
	void ReflectCurrentState();
	
	void ReflectListState();	
	// (a.walling 2012-12-11 09:44) - PLID 53988 - Try to reflect current state, and recreate content if necessary due to state
	bool TryReflectListState();
	void ReflectTableState();
	void ReflectImageState(); // (z.manning, 08/10/2007) - PLID 26630
	void ReflectNarrativeState(); // (a.walling 2007-08-22 17:04) - PLID 27160
	void ReflectNarrativeContent(); //TES 6/3/2008 - PLID 29098
	void UpdateRichTextAppearance(); // (c.haag 2007-10-04 15:46) - PLID 26858
	// (a.walling 2009-11-17 11:38) - PLID 36365
	// (a.walling 2010-05-27 09:25) - PLID 38910 - Parameter to force update
	void UpdateNarrativeFields(COleSafeArray& saFieldValues, bool bForceUpdate);
	//TES 1/23/2008 - PLID 24157 - Need to cover all types.
	void ReflectTextState();
	void ReflectSliderState();

	BOOL DataElementPopUp(CPoint pt);

	CWnd m_wndImage;
	NXINKPICTURELib::_DNxInkPicturePtr m_Image;
	Nx3DLib::_DNx3DPtr m_p3DControl;
	// (a.walling 2013-06-27 13:15) - PLID 57348 - NxImageLib - More versatile replacement for g_EmrImageCache

	// (a.walling 2012-12-03 12:57) - PLID 53988 - Keep track of expanded parent labels
	std::set<long> m_expandedLabelIDs;

	// (a.walling 2010-05-19 11:20) - PLID 38778 - Option to shrink to fit the display area
	bool m_bShrinkFitToArea;

protected:
	// (c.haag 2007-08-15 16:32) - PLID 27084 - Used for garbage collection with the 
	// OnRequestFormatRichTextCtrl function
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	//CString* m_pstrRequestFormatRichTextCtrlResult;
	// (a.walling 2009-11-25 12:21) - PLID 36365 - We don't need a CString, just use a LPTSTR
	LPTSTR m_szRequestFormatBuffer;

protected:
	// (c.haag 2007-08-13 16:15) - PLID 25970 - Set to true when we're adding
	// a batch of merge fields to a detail
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	//BOOL m_bAddingMergeFields;

	// (r.gonet 02/14/2013) - PLID 40017 - Flag to save the column sizes in a popped up table. Otherwise, no attempt is made to save the column sizes when the user resizes columns in the popup.
	BOOL m_bRememberPoppedUpTableColumnWidths;

protected:
	// (j.jones 2008-06-05 10:08) - PLID 18529 - TryAdvanceNextDropdownList will potentially send
	// NXM_START_EDITING_EMR_TABLE which will fire this function, which will start editing the
	// row and column in question
	//DRT 7/24/2008 - PLID 30824 - Converted to use NxDataList2
	// (a.walling 2008-10-02 09:16) - PLID 31564 - VS2008 - Message handlers must fit the LRESULT fn(WPARAM, LPARAM) format
	LRESULT OnStartEditingEMRTable(WPARAM wParam, LPARAM lParam);

// Operations
public:
	//TES 1/14/2008 - PLID 24157 - Added parameters so that CEMRItemAdvMultiPopupDlg could specify:
	// -a margin to offset this window from the top left of its parent.
	// -a minimum allowable size for the window.
	// -that the window should be drawn with a border.
	//TES 1/15/2008 - PLID 24157 - Additionally, changed pParentDlg to a CWnd* (instead of a CEMRItemAdvPopupDlg*).
	// That dialog will get messages when the detail window resizes itself (NXM_EMR_POPUP_RESIZED), and when the state 
	// of a detail has been changed (NXM_EMR_POPUP_POST_STATE_CHANGED)
	// (a.walling 2008-01-18 16:22) - PLID 14982 - Added pRealDetail parameter
	//TES 2/21/2008 - PLID 28827 - Replaced szMin with szIdeal, which specifies the ideal size for any details (such
	// as text or slider details) that don't really care what size they are.
	// (j.jones 2009-12-16 14:17) - PLID 31021 - we once again take a minimum size
	void Initialize(CWnd *pParentDlg, CWnd *pParent, CRect rcWindow, CEMNDetail *pDetail, CEMNDetail *pRealDetail, BOOL bIsDetailIndependent, CSize szMax, CSize szMin, int nTopMargin = 0, int nLeftMargin = 0, CSize szIdeal = CSize(0,0), bool bDrawBorder = false);

	//TES 2/21/2008 - PLID 28827 - Allow the user to change the maximum size after initialization.
	void SetMaxSize(CSize szMax);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRItemAdvPopupWnd)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEMRItemAdvPopupWnd();

	// Generated message map functions
protected:
	// (a.walling 2008-01-16 15:24) - PLID 14982
	void OnEditItem();
	// (c.haag 2008-01-29 12:16) - PLID 28686 - We now allow the caller to define special dialog behaviors
	void OpenItemEntryDlg(EEmrItemEntryDlgBehavior behavior = eEmrItemEntryDlgBehavior_Normal, LPVOID pBehaviorData = NULL);

	//TES 1/14/2008 - PLID 24157 - Added a minimum size, and an offset from the top left.
	//TES 2/21/2008 - PLID 28827 - Replaced m_szMin with m_szIdeal, which specifies the ideal size for any details (such
	// as text or slider details) that don't really care what size they are.
	// (j.jones 2009-12-16 14:17) - PLID 31021 - we once again take a minimum size
	CSize m_szMax, m_szIdeal, m_szMin;
	int m_nTopMargin, m_nLeftMargin;

	BOOL IsScrollBarVisible(HWND hWndParent, int nBar);
	void OnScroll(int nBar, const int nScrollType);

	BOOL m_bReadOnly;

	// (j.jones 2007-09-26 09:59) - PLID 27510 - used only as workaround fror MFC bug
	// (a.walling 2012-05-07 09:21) - PLID 48934 - This bug is now fixed via CNxOccManager
	//CNxStatic m_wndInvisibleAnchor;

	//TES 10/12/2007 - PLID 23816 - Copied from CEmrItemAdvImageDlg.
	BOOL HasValidImage();

	//DRT 4/30/2008 - PLID 29771 - For background painting
	CBrush m_brBackground;

	// (z.manning 2010-05-05 16:45) - PLID 38503 - Moved some repeated code to its own function
	void UpdateRememberedPenState();

	void Handle3DStampChange(); // (z.manning 2011-09-12 11:20) - PLID 45335

	void HandleStampSetup(); // (z.manning 2011-10-25 14:47) - PLID 39401

	// (r.gonet 02/14/2012) - PLID 37682 - Opens the filter editor and sets up the filter on the image.
	void HandleStampFilterSetup();

	// (z.manning 2012-03-27 16:58) - PLID 33710 - Call this function to request a detail's state change from the popup
	BOOL RequestDetailStateChange(const _variant_t &varNewState);

	//TES 7/6/2012 - PLID 51419 - Split this out so the context menu function can be called by both RTF and HTML narratives.
	int GetNarrativeRightClickFieldOption(const CString &strField, const CString &strFlags, long x, long y);

	// (a.walling 2007-06-25 10:26) - PLID 22097 - Added handler for OnResolvePendingMergeFieldValue event
	// (z.manning, 02/21/2008) - PLID 28690 - Added message handlers for hot spot clicking.
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnStrokeInkPicture(LPDISPATCH Cursor, LPDISPATCH Stroke, BOOL FAR* Cancel);
	afx_msg void OnBrowseInkPicture(BOOL FAR* pbProcessed);
	// (a.walling 2010-05-19 11:25) - PLID 38778 - Toggle between resized / full mode
	afx_msg void OnFullScreenInkPicture();
	afx_msg void OnButtonClicked(UINT nID);
	afx_msg BOOL OnButtonClickedEvent(UINT nID);
	afx_msg void OnChangeEdit();
	afx_msg void OnTextChangedInkPicture(NXINKPICTURELib::ETextChangeType TextChangeType);
	afx_msg void OnTextChangedRichTextCtrl();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnLinkRichTextCtrl(LPCTSTR strMergeField);
	afx_msg void OnRequestFormatRichTextCtrl(LPCTSTR strField, LPCTSTR strFlags, LPCTSTR *pstrValue);
	afx_msg void OnRightClickFieldRichTextCtrl (LPCTSTR strField, LPCTSTR strFlags, long x, long y, long nIndex);
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	//afx_msg void OnRequestFieldRichTextCtrl(LPCTSTR strField);
	//afx_msg void OnRequestAllFieldsRichTextCtrl();
	afx_msg void OnCustomStampsChangedInkPicture(LPCTSTR strNewCustomStamps);
	//afx_msg void OnResolvePendingMergeFieldValue(LPDISPATCH lpdispMergeSet);
	afx_msg void OnRequestLWMergeFieldData();
	// (a.walling 2009-11-19 14:21) - PLID 36367 - Requesting list of various merge fields from the host
	afx_msg void OnRequestAllAvailableFields(VARIANT_BOOL bIncludeLetterWriting, VARIANT_BOOL bIncludeEMR, VARIANT_BOOL bIncludeListItems, VARIANT* psafearrayFields);
	// (z.manning 2011-11-10 10:10) - PLID 46382 - Added an event for getting the version of available fields
	afx_msg void OnRequestAvailableFieldsVersion(short* pnVersion);
	afx_msg LRESULT OnAddNewDropdownColumnSelection(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClickedHotSpotInkPicture(long nHotSpotID);
	afx_msg void OnClickedHotSpot3DControl(short n3DHotSpotID); // (z.manning 2011-07-25 12:02) - PLID 44649
	afx_msg LRESULT OnToggleHotSpot(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEditingFinishedTable(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingTable(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingStartingTable(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	// (r.gonet 02/14/2013) - PLID 40017 - Added
	afx_msg void OnColumnSizingFinishedTable(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	afx_msg void OnTextSpellCheck();
	afx_msg void OnLabButton(); // (z.manning 2009-09-21) - PLID 33612
	// (j.jones 2009-10-02 11:14) - PLID 35161 - added ability to right-click on a table
	afx_msg void OnRButtonDownTable(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (a.walling 2012-11-06 11:56) - PLID 53609 - Display context menu during datalist ShowContextMenu event
	afx_msg void OnShowContextMenuTable(LPDISPATCH lpRow, short nCol, long x, long y, long hwndFrom, VARIANT_BOOL* pbContinue);
	afx_msg void OnLButtonUpTable(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (j.jones 2010-02-10 14:44) - PLID 37312 - handle when the image tries to open the stamp setup
	afx_msg void OnOpenStampSetupInkPicture();
	// (r.gonet 02/14/2012) - PLID 37682 - Handles clicks from the user on the filter setup button.
	afx_msg void OnOpenStampFilterSetupInkPicture();
	afx_msg void OnOpenStampSearch();
	afx_msg void OnOpenStampSearch3D(); // (j.gruber 2012-08-14 12:24) - PLID 52134
	afx_msg void OnOpenStampFilterSetup3DControl();
	afx_msg void OnOpenStampSetup3DControl(); // (z.manning 2011-09-14 10:13) - PLID 44693
	// (r.gonet 05/31/2011) - PLID 43896 - Put back in if we want to load popped up images zoomed.
	//afx_msg void OnZoomLevelChangedInkPicture(double dZoomLevel);
	//afx_msg void OnViewportOffsetsChangedInkPicture(long nOffsetX, long nOffsetY);
	// (j.jones 2011-03-23 10:15) - PLID 42965 - supported the macro abilities in the popup
	afx_msg LRESULT OnPasteMacroText(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSmartStampAdd3DControl(); // (z.manning 2011-09-12 11:09) - PLID 45335
	afx_msg void OnSmartStampErase3DControl(); // (z.manning 2011-09-12 11:09) - PLID 45335
	afx_msg void OnPreviewModified3DControl(); // (z.manning 2011-10-05 13:33) - PLID 45842
	afx_msg void OnPaint();
	//TES 7/3/2012 - PLID 51357 - Added handler for the new CheckValidIDs event
	afx_msg void OnCheckValidIDsRichTextCtrl(long nDetailID, BOOL *pbDetailIDExists, long nDataID, BOOL *pbDataIDExists);
	afx_msg void OnRightClickHtmlFieldRichTextCtrl(LPCTSTR strField, LPCTSTR strFlags, long x, long y, LPCTSTR strNexGUID);
	afx_msg void OnRightClickHtmlRichTextCtrl(long x, long y, bool bCanCopy, bool bCanPaste);	// (j.armen 2012-10-05 15:03) - PLID 52986
	// (d.singleton 2013-4-22 14:32) - PLID 56421 get the signature form the topaz sig pad and display on ink control
	afx_msg void OnClickedTopazSignature();
	// (j.armen 2014-07-22 09:15) - PLID 62836 - Handle pen color and pen size changed
	afx_msg void OnCurrentPenColorChanged();
	afx_msg void OnCurrentPenSizeChanged();
	

	// (a.walling 2012-08-30 07:05) - PLID 51953 - Detect actual VK_TAB keypresses to ignore stuck keys
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()

	DECLARE_EVENTSINK_MAP()
};