#pragma once

#include "EmrItemAdvDlg.h"
#include "EmrItemAdvNarrativeBase.h"
#include "NxRichEditCtrl.h"

//TES 1/29/2008 - PLID 28673 - Define IDCs for any controls we create.
#define RICH_TEXT_IDC	1000
// (c.haag 2012-04-02) - PLID 49346
#define HTML_TEXT_IDC	1001
/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvNarrativeDlg dialog

class CEmrItemAdvNarrativeDlg :
	public CEmrItemAdvDlg,
	public CEmrItemAdvNarrativeBase // (c.haag 2008-11-25 11:31) - PLID 31693 - Support for narrative functionality
{
// Construction
public:
	CEmrItemAdvNarrativeDlg(class CEMNDetail *pDetail);
	// (c.haag 2007-06-28 10:12) - PLID 25970 - Need to overload the destructor
	// to handle garbage collection
	~CEmrItemAdvNarrativeDlg();


	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated massive amounts of recordset-related narrative code

	//Is the narrative "complete"? (Are there any fields which are on the narrative but not actually filled in?
	BOOL IsStateSet();
	//TES 9/16/2009 - PLID 35529 - Assigns this dialog to the given detail (only used for the global, "windowless" dialog)
	void SetDetail(CEMNDetail *pDetail);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrItemAdvNarrativeDlg)
	//}}AFX_VIRTUAL

public:
	virtual void ReflectCurrentState();
	virtual void ReflectCurrentContent();
	//TES 3/23/2010 - PLID 37757 - This dialog doesn't maintain its own ReadOnly flag, so I changed the function name from
	// SetReadOnly() to ReflectReadOnlyStatus()
	virtual void ReflectReadOnlyStatus(BOOL bReadOnly);

public:
	// (c.haag 2007-07-30 12:47) - PLID 26858 - This function ensures that the
	// rich text and the narrative merge fields are "in synchronization". You 
	// should call this function if this detail's narrative merge field list is
	// maintained by an EMN (rather than by the detail), and the merge field list
	// was modified.
	void UpdateRichTextAppearance();

	// (a.walling 2009-11-17 11:38) - PLID 36365
	// (a.walling 2010-05-27 09:25) - PLID 38910 - Parameter to force update
	void UpdateNarrativeFields(COleSafeArray& saFieldValues, bool bForceUpdate);

public:
	// (c.haag 2013-04-26) - PLID 56450 - Ensure the HTML is XHTML compliant for HTML narratives
	void ValidateHtml();

public:
	// (c.haag 2007-05-14 10:04) - PLID 25970 - If the detail's merge merge
	// field recordset object changed (not the content, but the object itself),
	// then we need to call this function to make sure that the form control
	// is up to date
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	//virtual void SetNarrativeFieldRecordset(ADODB::_RecordsetPtr& prs);

public:
	virtual BOOL RepositionControls(IN OUT CSize &szArea, BOOL bCalcOnly);

private:
	// (c.haag 2012-04-02) - PLID 49346 - This function will assign HTML to the browser control. This should only
	// be used for HTML narratives.
	void SetHtmlText(const CString& strHtml);

// Implementation
protected:
	CWnd m_wndRichEdit;
	RICHTEXTEDITORLib::_DRichTextEditorPtr m_RichEditCtrl;
	// (c.haag 2012-04-02) - PLID 49346 - The browser window for showing HTML narratives
	CWnd m_wndHtmlEdit;
	// (c.haag 2012-04-02) - PLID 49346 - The browser object
	IWebBrowser2Ptr m_pHtmlEdit;
	// (c.haag 2012-04-02) - PLID 49346 - This is TRUE if ReflectCurrentState was called at least once and
	// the editing control has been updated with the state data.
	BOOL m_bHasReflectedInitialState;

protected:
	// (c.haag 2007-05-17 15:56) - PLID 25970 - Set to true when we're adding
	// a batch of merge fields to a detail
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	//BOOL m_bAddingMergeFields;

protected:
	// (c.haag 2007-08-15 16:34) - PLID 27084 - Used for garbage collection with the 
	// OnRequestFormatRichTextCtrl function
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	//CString* m_pstrRequestFormatRichTextCtrlResult;
	// (a.walling 2009-11-25 12:21) - PLID 36365 - We don't need a CString, just use a LPTSTR
	LPTSTR m_szRequestFormatBuffer;

protected:
	//Hides merge field 'tags', enables linking.
	CString RichTextToNxRichText(const CString &strRichText);

	//Not visible to user, used to facilitate converting rich text to "nxrichtext"
	CNxRichEditCtrl m_richEdit;

	//TES 7/6/2012 - PLID 51419 - Split this out so the context menu function can be called by both RTF and HTML narratives.
	int GetNarrativeRightClickFieldOption(const CString &strField, const CString &strFlags, long x, long y);

	// Generated message map functions
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTextChangedRichTextCtrl();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnLinkRichTextCtrl(LPCTSTR strMergeField);
	afx_msg void OnRequestFormatRichTextCtrl(LPCTSTR strField, LPCTSTR strFlags, LPCTSTR *pstrValue);
	afx_msg void OnRightClickFieldRichTextCtrl (LPCTSTR strField, LPCTSTR strFlags, long x, long y, long nIndex);
	//TES 7/3/2012 - PLID 51357 - Added handler for the new CheckValidIDs event
	afx_msg void OnCheckValidIDsRichTextCtrl(long nDetailID, BOOL *pbDetailIDExists, long nDataID, BOOL *pbDataIDExists);
	afx_msg void OnRightClickHtmlFieldRichTextCtrl(LPCTSTR strField, LPCTSTR strFlags, long x, long y, LPCTSTR strNexGUID);
	afx_msg void OnRightClickHtmlRichTextCtrl(long x, long y, bool bCanCopy, bool bCanPaste);	// (j.armen 2012-10-05 15:15) - PLID 52986
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	//afx_msg void OnRequestFieldRichTextCtrl(LPCTSTR strField);
	//afx_msg void OnRequestAllFieldsRichTextCtrl();
	//afx_msg void OnResolvePendingMergeFieldValue(LPDISPATCH lpdispMergeSet);
	afx_msg void OnRequestLWMergeFieldData();
	// (a.walling 2009-11-19 14:21) - PLID 36367 - Requesting list of various merge fields from the host
	afx_msg void OnRequestAllAvailableFields(VARIANT_BOOL bIncludeLetterWriting, VARIANT_BOOL bIncludeEMR, VARIANT_BOOL bIncludeListItems, VARIANT* psafearrayFields);
	// (z.manning 2011-11-10 10:10) - PLID 46382 - Added an event for getting the version of available fields
	afx_msg void OnRequestAvailableFieldsVersion(short* pnVersion);
	DECLARE_EVENTSINK_MAP()
	DECLARE_MESSAGE_MAP()
	void OnBeforeNavigate2HtmlEdit(LPDISPATCH pDisp, VARIANT* URL, VARIANT* Flags, VARIANT* TargetFrameName, VARIANT* PostData, VARIANT* Headers, BOOL* Cancel);
	void OnNavigateComplete2HtmlEdit(LPDISPATCH pDisp, VARIANT* URL);
};