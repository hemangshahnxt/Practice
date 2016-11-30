#if !defined(AFX_EMRPREVIEWCTRLDLG_H__01B4850A_10C0_432B_91BB_CF7F1C431D6A__INCLUDED_)
#define AFX_EMRPREVIEWCTRLDLG_H__01B4850A_10C0_432B_91BB_CF7F1C431D6A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRPreviewCtrlDlg.h : header file
//

#include "EmrTreeWnd.h"
#include "EmrHelpers.h"
#include <NxUILib/NxHtmlControl.h>

// (j.jones 2013-05-08 10:09) - PLID 56596 - added CEMNDetail and CEMN forward declared classes
class CEMNDetail;
class CEMN;

_COM_SMARTPTR_TYPEDEF(IHTMLElement2, __uuidof(IHTMLElement2));

_COM_SMARTPTR_TYPEDEF(IHTMLDocument3, __uuidof(IHTMLDocument3));

/////////////////////////////////////////////////////////////////////////////
// CEMRPreviewCtrlDlg dialog

// (a.walling 2007-12-27 16:42) - PLID 28632 - Static variable to only ensure once per session
static bool g_bEnsureCSSFileOK;

class CEmnPreviewInfo;

// (a.walling 2012-05-08 16:35) - PLID 50241 - CEMNPrintInfo is now DocInfo in the NxPrintTemplate namespace
namespace NxPrintTemplate
{

// (a.walling 2012-05-08 16:35) - PLID 50241 - DocInfo defines the source of the document ("document") and the html header / footer text,
// as well as providing some helper functions and a way to generate the print template with the document def and header/footer def
struct DocInfo {
	DocInfo()
		: strDocumentPath("document")
	{}

	CString strDocumentPath;
	CString strHeaderHTML;
	CString strFooterHTML;

	DocInfo& SetHeaderText(LPCTSTR szText);
	DocInfo& SetFooterText(LPCTSTR szText);
	DocInfo& SetHeaderInnerHTML(LPCTSTR szHtml);
	DocInfo& SetFooterInnerHTML(LPCTSTR szHtml);
	static CString WrapHeaderFooter(LPCTSTR szHtml, LPCTSTR szClass, LPCTSTR szID);
	
	CString GetDocumentDefinition(long nIndex = 0);

	CString PreparePrintTemplate();
	// (r.gonet 06/13/2013) - PLID 56850 - Added the bDisplayPerDocumentPageCount variable, which controls whether or not reset the page count each document.
	static CString PreparePrintTemplate(const CString& strDocumentDefinition, bool bDisplayPerDocumentPageCount = true);
};
}

// (a.walling 2011-11-11 11:11) - PLID 46634 - These need to be CWnd-derived rather than CDialog, so we can ignore the size set in the resource, as well as avoid some of CDialog's default handling
// (a.walling 2012-11-05 11:58) - PLID 53588 - Use CNxHtmlControl base for CEMRPreviewCtrlDlg
class CEMRPreviewCtrlDlg 
	: public CNxHtmlControl
	, public Emr::InterfaceAccessImpl<CEMRPreviewCtrlDlg>
{
// Construction
public:
	CEMRPreviewCtrlDlg();   // standard constructor
	~CEMRPreviewCtrlDlg();
		
	// (a.walling 2010-01-12 08:37) - PLID 36840 - Moved formerly registered messages from CEMRPreviewCtrlDlg to NxMessageDef.h

	// (a.walling 2009-11-23 12:30) - PLID 36404 - Are we currently printing?
	bool IsPrinting();

	void SetInteractive(BOOL bSetInteractive);
	// (a.walling 2008-10-23 16:42) - PLID 31819 - Get the detail object from the url
	CEMNDetail* GetDetailFromURL(const CString& strUrl);
	// (a.walling 2008-10-24 11:13) - PLID 31819 - Get the topic object from the url
	CEMRTopic* GetTopicFromURL(const CString& strUrl);

	// (a.walling 2009-10-28 14:05) - PLID 35989 - Find the best topic for a new signature (either a *signature* topic, or the very last topic on an EMN)
	CEMRTopic* FindAppropriateSignatureTopic(bool* pbIsSignatureTopic);

	// (a.walling 2008-10-14 10:23) - PLID 31678 - Configure the preview
	void ConfigurePreview();

	// (j.jones 2007-07-06 10:09) - PLID 25457 - added SetAllowEdit, so the control knows whether editing is allowed
	void SetAllowEdit(BOOL bAllowEdit);
	BOOL SetSource(CString strUrl, CEMN* pEMN = NULL); // set the source html page, refresh if the same
	void SetEMN(CEMN* pEMN, BOOL bSetActive = TRUE);
	void RemoveEMN(CEMN* pEMN); // (a.walling 2007-09-25 14:22) - PLID 25548 - Remove an EMN from the preview

	// (a.walling 2007-10-11 17:39) - PLID 25548 - Return the currently displayed EMN
	CEMN* GetCurrentEMN();

	// (a.walling 2012-03-07 08:36) - PLID 48680 - Preview pane commands and options
	void Print();
	void PrintPreview();
	// (a.walling 2009-11-24 09:45) - PLID 36418
	void PrintMultipleEMNs();

private:
	// (c.haag 2013-02-28) - PLID 55365 - Critical path for printing single EMN's with no custom preview layouts
	void DoPrint(UINT uMenuID);
	// (c.haag 2013-02-28) - PLID 55365 - Critical path for printing multiple EMN's or an EMN with at least one
	// custom preview layout. If you have the EMN's custom preview layouts, you should pass them in here. Otherwise 
	// leave it NULL, and the multiple print dialog will get the list on its own later on. If nSingleEMNID is a positive number,
	// it means we're only printing one EMN. Otherwise we're printing the whole EMR.
	void DoPrintMultiple(LPUNKNOWN lpunkEMRCustomPreviewLayouts, long nSingleEMNID);

public:
	// (a.walling 2010-11-12 11:18) - PLID 38135 - Popup preview with all patient EMNs
	void PopupAllPatientEMNs();

	// (a.walling 2008-11-14 08:55) - PLID 32024 - When displayed outside of the EMR, this will let us get/set the ID
	long GetDisplayedEMNID();
	void SetDisplayedEMNID(long nID);

	void SetTreeWnd(class CEmrTreeWnd* pTreeWnd);

	BOOL ShowTopic(CEMRTopic* pTopic, BOOL bShow); // show hidden topic

	BOOL IsInitialized();
	BOOL IsLoaded();
	// (a.walling 2008-07-01 12:53) - PLID 30570 - Is the current EMN writable?
	BOOL IsEmnWritable();

	// (a.walling 2007-12-17 16:20) - PLID 28391
	BOOL EnsureTopicClass(CEMRTopic* pTopic, CString strClass);
	BOOL EnsureTopicNotClass(CEMRTopic* pTopic, CString strClass);

	// (a.walling 2007-04-10 17:11) - PLID 25549 - These topics are pretty self-explanatory
	// they update the EMR preview window accordingly.
	BOOL InsertDetail(CEMNDetail* pDetail, CEMNDetail* pDetailInsertBefore = NULL);
	BOOL MoveDetail(CEMNDetail* pDetail, CEMNDetail* pDetailInsertBefore);
	BOOL RemoveDetail(CEMNDetail* pDetail);

	BOOL UpdateEMNTitle(CString &strTitle);
	BOOL UpdateMoreInfo(CEMN* pEMN); // (a.walling 2007-07-12 15:50) - PLID 26640 - Update more info and header

	// (a.walling 2007-10-12 10:42) - PLID 27017 - Used to update a detail when the preview has not completed loading yet
	void PendUpdateDetail(CEMNDetail* pDetail);
	// (a.walling 2007-10-12 11:23) - PLID 27017 - Returns whether the detail is in any pending update array
	BOOL IsDetailInPendingUpdateArray(CEMNDetail* pDetail);
	BOOL UpdateDetail(CEMNDetail* pDetail);
	BOOL UpdateTopic(CEMRTopic* pTopic);
	BOOL UpdateTopicTitle(CEMRTopic* pTopic, CString &strTitle);

	// (a.walling 2007-10-10 09:22) - PLID 25548 - added bStrict parameter for all InsertTopic functions to prevent
	// adding at the end if the parent or sibling was not found.
	BOOL InsertTopic(CEMRTopic* pTopic);
	BOOL InsertSubTopic(CEMRTopic* pParentTopic, CEMRTopic* pTopic, BOOL bStrict = FALSE);

	BOOL InsertTopicBefore(CEMRTopic* pNextTopic, CEMRTopic* pTopic, BOOL bStrict = FALSE);
	BOOL InsertSubTopicBefore(CEMRTopic* pParentTopic, CEMRTopic* pNextTopic, CEMRTopic* pTopic, BOOL bStrict = FALSE);

	BOOL MoveTopic(CEMRTopic* pTopic, CEMRTopic* pInsertBeforeTopic);
	BOOL MoveTopicToSubTopic(CEMRTopic* pTopic, CEMRTopic* pSubTopic, CEMRTopic* pInsertBeforeTopic);
	BOOL RemoveTopic(CEMRTopic* pTopic);

	BOOL ScrollToTopic(CEMRTopic* pTopic);
	BOOL ScrollToTop(); // (a.walling 2007-06-19 17:28) - PLID 25548 - Scroll to the top of the window (that is, the EMN title)
	BOOL ScrollToMoreInfo(); // (a.walling 2007-07-12 16:00) - PLID 26640 - Scroll to the moreinfo topic

	CString GetCurrentHTML(IHTMLElementPtr pElement = NULL); // Returns the current calculated HTML of the element, using the body if no element sent.

	// (a.walling 2015-11-16 11:52) - PLID 67494 - Allow an optional parameter to scroll to upon document load
	BOOL SafeNavigate(COleVariant &url, long nScrollTop = 0);

	// (a.walling 2007-10-01 10:39) - PLID 25648 - Added ability to navigate to a raw HTML
	BOOL SafeNavigateToHTML(const CString &strHTML);

	// (a.walling 2007-08-08 15:19) - PLID 27017
	long GetScrollTop(); // Return the current scroll position (top of visible content - top of document)
	BOOL PutScrollTop(long nTop); // Set the current scroll position

	// ensure the CSS file exists and also in the temp folder, copies the default from resources if not found
	static void EnsureCSSFile();
	
	// (a.walling 2008-10-06 16:49) - PLID 31430 - Prepare a print template for the current EMN, return the path
	// (a.walling 2009-11-23 11:45) - PLID 36395 - Prepare the multi-document print template for printing the currently displayed document
	CString PrepareCurrentPrintTemplate();

	// (a.walling 2008-10-06 16:49) - PLID 31430 - Prepare a print template for the current EMN, return the path
	// (a.walling 2009-11-23 11:45) - PLID 36395 - Prepare the multi-document print template for the list of documents passed in
	// (r.gonet 06/13/2013) - PLID 56850 - Added the bDisplayPerDocumentPageCount variable, which controls whether or not reset the page count each document.
	CString PreparePrintTemplate(CList<NxPrintTemplate::DocInfo, NxPrintTemplate::DocInfo&>& listEMNs, bool bDisplayPerDocumentPageCount = true);
	// (a.walling 2009-11-23 11:46) - PLID 36395 - Define the document sources for the print template
	CString PrepareDocumentDefinition(CList<NxPrintTemplate::DocInfo, NxPrintTemplate::DocInfo&>& listEMNs);

	// (a.walling 2009-11-23 11:47) - PLID 36396 - Shared method to decrypt a saved EMN Preview and return the path to the decrypted MHT file
	// (a.walling 2011-06-17 15:35) - PLID 42367 - Output params for actual file name and time
	// (z.manning 2012-09-11 14:03) - PLID 52543 - Added modified date
	static bool GetMHTFile(long nID, COleDateTime dtEmnModifiedDate, CString& strDecryptedFilePath, CString* pstrActualFileName = NULL, FILETIME* pLastWriteTime = NULL);

// Implementation
protected:
	// (a.walling 2007-04-10 17:12) - PLID 25548 - Helper functions return a pointer to the element
	IHTMLElementPtr GetElementByID(const CString &strID); // search by ID
	IHTMLElementPtr GetElementByIDOrPointer(const CString &strType, long nID, long nPtr); // failsafe search by both ID and pointer (for unsaved elements)

	CStringArray m_arTempNavigations;

	// (a.walling 2008-02-06 09:46) - Modifying inline styles is now deprecated; this is now handled via multiple CSS class inheritance
	// BOOL ShowElement(IHTMLElementPtr pElement, BOOL bShow);

	BOOL m_bInteractive;
	// (j.jones 2007-07-06 10:09) - PLID 25457 - added m_bAllowEdit, so the control knows whether editing is allowed
	BOOL m_bAllowEdit;
	long m_nPatientID;
	long m_nEmrID;
	
	// (a.walling 2010-11-12 11:18) - PLID 38135 - Popup preview with all patient EMNs
	class CEMRPreviewPopupDlg* m_pPoppedUpPreviewDlg;

	// (a.walling 2007-08-08 15:34) - PLID 27017 - Margin for scrolling (so we don't scroll to the exact pixel where text begins)
	const long m_nScrollMargin;

	CEmnPreviewInfo* m_pCurrentInfo;

	// (a.walling 2008-11-14 08:44) - PLID 32024 - ID of the currently displayed EMN in the preview (not in the actual EMR editor)
	long m_nDisplayedEMNID;
	long m_nPendingScrollTop = 0; // (a.walling 2015-11-16 11:52) - PLID 67494 - When not using m_pCurrentInfo, this is used to scroll upon document load

	class CEmrTreeWnd* m_pTreeWnd;

	CMapPtrToPtr m_mapEmnToInfo;

	// (a.walling 2007-10-01 13:12) - PLID 25548 - Scrolls the EMN to the pending scrolls within the m_pCurrentInfo structure
	BOOL ScrollPending();
	BOOL ScrollToElementByID(const CString &strID); // (a.walling 2007-10-01 13:57) - PLID 25548 - Shared function scrolls to the element passed

	// (j.jones 2007-07-06 11:01) - PLID 25457 - created NavigateToTopic
	// (c.haag 2009-09-10 17:31) - PLID 35077 - Made public
public:
	void NavigateToTopic(CString strParam, CString strCommand);

protected:

	// (a.walling 2012-11-05 11:58) - PLID 53588 - EmrPreviewCtrlInterface no longer necessary; merged into here
	
	struct ContextMenuInfo
	{
		ContextMenuInfo()
			: pt(0, 0)
			, bValid(false)
		{
		}

		bool bValid;

		CPoint pt;
		DWORD dwID;

		CString strUrl;

		
		enum MenuItems {
			// (j.jones 2007-07-05 14:48) - PLID 25457 - added popup, go to topic, and edit
			miPopup = 0x01a4,
			// (a.walling 2008-07-01 10:04) - PLID 30570
			miHideTitle,
			miHideItem,
			miSubDetail, // (a.walling 2008-10-23 10:24) - PLID 31808
			miHideIfIndirectlyOnNarrative, // (a.walling 2012-07-13 16:38) - PLID 48896
			miHideItemOnIPad,	// (j.armen 2013-01-16 16:42) - PLID 54412
				// (a.walling 2009-01-07 15:03) - PLID 32695 - Positioning (floating) setup submenu
				// (a.walling 2009-07-06 10:27) - PLID 34793
				miColumnOne,
				miColumnTwo,
				// (a.walling 2009-07-06 10:27) - PLID 34793
				/*
				miClearLeft,
				miClearRight,
				*/
				// (a.walling 2009-07-06 12:27) - PLID 34793
				miGroupBegin,
				miGroupEnd,
				// (a.walling 2009-01-08 14:07) - PLID 32660 - Align text right
				miTextRight,
				// (a.walling 2010-08-31 18:20) - PLID 36148 - Page breaks
				miPageBreakBefore,
				miPageBreakAfter,
			miGoToTopic,
			miEdit,
			miFind, // (a.walling 2008-10-22 12:17) - PLID 31795 - Open the 'Find' dialog
			miPrint,
			miPrintPreview,
			miPrintMultiple, // (a.walling 2009-11-24 09:43) - PLID 36418
			miPopupOther, // (a.walling 2010-11-12 11:18) - PLID 38135 - Popup preview with all patient EMNs
			miViewSource,
			miViewGeneratedSource, // (a.walling 2007-10-18 14:26) - PLID 25548 - To help debug, you can view the generated source.
			miProperties,
			miForceRefresh,
			miConfigure, // (a.walling 2008-10-14 10:28) - PLID 31678 - Configure the preview
			miAddSignature, // (z.manning 2009-08-26 09:36) - PLID 33911
			miAddOtherUsersSignature,	// (j.jones 2013-08-07 15:14) - PLID 42958
			miLinkWithNewProblem, // (c.haag 2009-09-10 15:09) - PLID 35077
			miLinkWithExistingProblems, // (c.haag 2009-09-10 15:09) - PLID 35077
			miUpdateProblemInformation, // (c.haag 2009-09-10 15:16) - PLID 35077
			miFax, //(e.lally 2009-10-01) PLID 32503
			miAddTextMacro, // (a.walling 2009-10-29 09:35) - PLID 36089 - Add text macros
			miCopy, // (a.walling 2010-03-25 20:34) - PLID 27372 - Copy
		};
	};

	ContextMenuInfo m_contextMenuInfo;

	virtual HRESULT OnShowContextMenu(DWORD dwID, LPPOINT ppt,
		LPUNKNOWN pcmdtReserved, LPDISPATCH pdispReserved);
	// (j.jones 2007-07-05 14:38) - PLID 25457 - added pdispReserved
	// (a.walling 2010-03-25 20:34) - PLID 27372 - pass the id of the type of context
	HRESULT CustomContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved);

	virtual void OnInitializing();
	virtual void OnDocumentLoaded(LPDISPATCH pDisp, VARIANT FAR* URL);	
	virtual void OnPrintTemplatesTornDown(LPDISPATCH pDisp);

	// Generated message map functions
	//{{AFX_MSG(CEMRPreviewCtrlDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBeforeNavigate2EmrPreviewBrowser(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags, VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers, BOOL FAR* Cancel);
	afx_msg void OnDestroy();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// (j.jones 2013-05-08 10:10) - PLID 56596 - turned into a proper class so its functions can be moved to the .cpp file
class CEmnPreviewInfo // this is a struct instead of a class for aesthetic purposes; everything is public by default. only difference.
{
public:
	CEMN* pEMN;
	CString strSource;
	// CString strHash; // unused for now
	long nScrollTop; // (a.walling 2007-08-08 16:06) - PLID 27017 - Our last saved scroll position
	CString strPendingScrollID; // (a.walling 2007-10-01 13:05) - PLID 25548
	CString strPendingScrollPT;

	// (a.walling 2007-10-12 10:43) - PLID 27017 - Details pending update when preview is done loading
	CArray<CEMNDetail*, CEMNDetail*> arDetailsPendingUpdate;

	// (a.walling 2012-03-12 12:43) - PLID 48712 - No HTMLPreviewTempFile anymore basically
	CEmnPreviewInfo(CEMN* pCurrentEMN);
	~CEmnPreviewInfo();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRPREVIEWCTRLDLG_H__01B4850A_10C0_432B_91BB_CF7F1C431D6A__INCLUDED_)
