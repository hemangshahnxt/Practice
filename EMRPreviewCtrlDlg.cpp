// EMRPreviewCtrlDlg.cpp : implementation file
//

#include "stdafx.h"
#include <NxPracticeSharedLib/RichEditUtils.h>
#include "patientsRc.h"
#include "EmrTreeWnd.h"
#include "EmrFrameWnd.h"
#include "EMRPreviewCtrlDlg.h"
#include "FileUtils.h"
#include "EmrItemAdvDlg.h"
#include "ConfigEMRPreviewDlg.h"
#include "shlwapi.h"
#pragma comment(lib, "shlwapi.lib")
#include "WinInet.h"
#include "NxCDO.h"
#include "EmrItemAdvNarrativeBase.h"
#include "EMRPreviewPopupDlg.h"
#include "DontShowDlg.h"
#include <mshtmcid.h>
#include <exdispid.h>
#include "NxAPI.h"
#include "EMRTopic.h"
#include "EMNDetail.h"
#include "EMR.h"
#include "EMN.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2012-03-07 18:06) - PLID 48712 - Made sure all navigate calls go through SafeNavigate[ToHTML]

_COM_SMARTPTR_TYPEDEF(IHTMLDOMNode, __uuidof(IHTMLDOMNode));

// (a.walling 2011-11-11 11:11) - PLID 46634 - Now CWnd-derived

// (a.walling 2012-11-05 11:58) - PLID 53588 - Now CNxHtmlControl-derived

//
// (c.haag 2007-09-28 11:49) - PLID 27509 - All calls to m_pTreeWnd->m_pTree->FindByColumn have
// been replaced with m_pTreeWnd->FindInTreeByColumn
//

// (a.walling 2007-12-27 16:42) - PLID 28632 - Static variable to only ensure once per session
extern bool g_bEnsureCSSFileOK = false;

// (a.walling 2010-01-12 08:37) - PLID 36840 - Moved formerly registered messages from CEMRPreviewCtrlDlg to NxMessageDef.h

// (j.jones 2013-05-08 10:11) - PLID 56596 - defined the CEmnPreviewInfo class functions here, instead of the .h

// (a.walling 2012-03-12 12:43) - PLID 48712 - No HTMLPreviewTempFile anymore basically
CEmnPreviewInfo::CEmnPreviewInfo(CEMN* pCurrentEMN)
	: pEMN(pCurrentEMN)
	, nScrollTop(0)
{
}

CEmnPreviewInfo::~CEmnPreviewInfo() {
	// (a.walling 2007-10-12 10:50) - PLID 27017
	// clear out any pending detail updates
	for (int i = 0; i < arDetailsPendingUpdate.GetSize(); i++) {
		// (a.walling 2009-10-12 16:05) - PLID 36024
		arDetailsPendingUpdate[i]->__Release("Details pending update");
	}
	arDetailsPendingUpdate.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// CEMRPreviewCtrlDlg dialog

// (a.walling 2007-08-08 15:34) - PLID 27017
CEMRPreviewCtrlDlg::CEMRPreviewCtrlDlg()
	: m_nScrollMargin(5) // Margin for scrolling (so we don't scroll to the exact pixel where text begins)
{
	//{{AFX_DATA_INIT(CEMRPreviewCtrlDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bInteractive = TRUE;
	// (z.manning, 04/24/2008) - PLID 29773 - Changed the default of m_bAllowEdit to FALSE
	m_bAllowEdit = FALSE;
	m_pCurrentInfo = NULL;
	m_pTreeWnd = NULL;

	// (a.walling 2008-11-14 08:44) - PLID 32024 - ID of the currently displayed EMN in the preview (not in the actual EMR editor)
	m_nDisplayedEMNID = -1;

	// (a.walling 2010-11-12 11:18) - PLID 38135 - Popup preview with all patient EMNs
	m_pPoppedUpPreviewDlg = NULL;
}

CEMRPreviewCtrlDlg::~CEMRPreviewCtrlDlg()
{
	try {
		POSITION pos = m_mapEmnToInfo.GetStartPosition();
		while (pos) {
			void* pEMNKey;
			CEmnPreviewInfo* pInfo;

			m_mapEmnToInfo.GetNextAssoc(pos, pEMNKey, reinterpret_cast<void *&>(pInfo));
			delete pInfo;
		}

		for (int i = 0; i < m_arTempNavigations.GetSize(); i++) {
			CString str = m_arTempNavigations.GetAt(i);
			str = str.Left(str.ReverseFind('?'));
			DeleteFileWhenPossible(str);
		}

		m_arTempNavigations.RemoveAll();
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::~CEMRPreviewCtrlDlg");
}

BEGIN_MESSAGE_MAP(CEMRPreviewCtrlDlg, CNxHtmlControl)
	//{{AFX_MSG_MAP(CEMRPreviewCtrlDlg)
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRPreviewCtrlDlg message handlers

BEGIN_EVENTSINK_MAP(CEMRPreviewCtrlDlg, CNxHtmlControl)
    //{{AFX_EVENTSINK_MAP(CEMRPreviewCtrlDlg)
	ON_EVENT(CEMRPreviewCtrlDlg, IDC_NXHTML_CONTROL, 250 /* BeforeNavigate2 */, OnBeforeNavigate2EmrPreviewBrowser, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()
	
void CEMRPreviewCtrlDlg::OnInitializing()
{
	try {
		if (m_pBrowser) {
			m_pBrowser->put_RegisterAsDropTarget(VARIANT_FALSE);
		}
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::OnInitDialog");
}

BOOL CEMRPreviewCtrlDlg::IsInitialized()
{
	return m_pCurrentInfo != NULL;
}

BOOL CEMRPreviewCtrlDlg::IsLoaded()
{
	if (!IsInitialized() || m_pCurrentInfo->strSource.IsEmpty() || !m_pBrowser) {
		return FALSE;
	}

	IDispatchPtr pDisp = NULL;
	m_pBrowser->get_Document(&pDisp);

	if (!pDisp) {
		return FALSE;
	}

	return TRUE;
}

void CEMRPreviewCtrlDlg::SetInteractive(BOOL bSetInteractive)
{
	m_bInteractive = bSetInteractive;
}

// (j.jones 2007-07-06 10:09) - PLID 25457 - added SetAllowEdit, so the control knows whether editing is allowed
void CEMRPreviewCtrlDlg::SetAllowEdit(BOOL bAllowEdit)
{
	m_bAllowEdit = bAllowEdit;
}

void CEMRPreviewCtrlDlg::SetTreeWnd(CEmrTreeWnd* pTreeWnd)
{
	m_pTreeWnd = pTreeWnd;
}

// (a.walling 2007-10-11 17:39) - PLID 25548 - Return the currently displayed EMN
CEMN* CEMRPreviewCtrlDlg::GetCurrentEMN()
{
	if (m_pCurrentInfo) {
		return m_pCurrentInfo->pEMN;
	}

	return NULL;
}

// (a.walling 2008-11-14 08:55) - PLID 32024 - When displayed outside of the EMR, this will let us get/set the ID
long CEMRPreviewCtrlDlg::GetDisplayedEMNID()
{
	return m_nDisplayedEMNID;
}

// (a.walling 2008-11-14 08:55) - PLID 32024 - When displayed outside of the EMR, this will let us get/set the ID
void CEMRPreviewCtrlDlg::SetDisplayedEMNID(long nID)
{
	m_nDisplayedEMNID = nID;
}

// (a.walling 2015-11-16 11:52) - PLID 67494 - Allow an optional parameter to scroll to upon document load
BOOL CEMRPreviewCtrlDlg::SafeNavigate(COleVariant &url, long nScrollTop)
{
	try {
		CString strUrl;
		if (url.vt == VT_BSTR) {
			strUrl = VarString(url);
		}

		// For some reason, MHTML files will not accept anchors unless it follows some GET variables.
		// (a.walling 2007-10-10 14:03) - PLID 25548 - Do not append any variables to about:blank
		if ((strUrl.CompareNoCase("about:blank") != 0) && (strUrl.Find("?") == -1)) {
			strUrl += FormatString("?%lu", GetTickCount());
		}

		m_nPendingScrollTop = nScrollTop;

		return LoadUrl(strUrl) ? TRUE : FALSE;

	} NxCatchAll("Error navigating");

	return FALSE;
}

// (a.walling 2007-10-01 10:42) - PLID 25648 - Safely navigate to an HTML string
BOOL CEMRPreviewCtrlDlg::SafeNavigateToHTML(const CString &strHTML)
{
	try {
		// (a.walling 2008-11-14 08:56) - PLID 32024 - Reset the displayed EMN ID
		m_nDisplayedEMNID = -1;
		m_nPendingScrollTop = 0;

		return LoadHtml(strHTML);

	} NxCatchAll("Error in CEMRPreviewCtrlDlg::SafeNavigateToHTML");

	return FALSE;
}

// (a.walling 2007-09-25 14:14) - PLID 25548 - Remove an EMN from the preview
void CEMRPreviewCtrlDlg::RemoveEMN(CEMN* pEMN)
{
	try {
		// is this the current emn?
		if (m_pCurrentInfo != NULL && m_pCurrentInfo->pEMN == pEMN) {
			// yes it is!
			m_pCurrentInfo = NULL;

			COleVariant blankUrl("about:blank");

			// (a.walling 2007-10-01 10:31) - PLID 25648 - Safe navigate instead of navigate
			SafeNavigate(blankUrl);

			//m_pBrowser->Navigate2(blankUrl, NULL, NULL, NULL, NULL);
		}

		// look up the info pointer
		CEmnPreviewInfo* pInfo = NULL;

		if (m_mapEmnToInfo.Lookup(pEMN, reinterpret_cast<void *&>(pInfo))) {
			// remove it from the map
			m_mapEmnToInfo.RemoveKey(pEMN);
			// and free memory
			// (a.walling 2007-10-12 10:52) - PLID 27017 - this will also free any details pending update
			delete pInfo;
		}		
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::RemoveEMN");
}

void CEMRPreviewCtrlDlg::SetEMN(CEMN* pEMN, BOOL bSetActive/* = TRUE*/)
{
	// bSetActive will set this as the active EMN and navigate to its preview
	// otherwise, it will just add the EMN to the info map.
	try {
		if (m_pCurrentInfo) {
			if (pEMN == m_pCurrentInfo->pEMN) {
				// this is the same EMN, so just exit.
				return;
			} else {
				// (a.walling 2007-10-12 10:50) - PLID 27017
				// clear out any pending detail updates
				for (int i = 0; i < m_pCurrentInfo->arDetailsPendingUpdate.GetSize(); i++) {
					//m_pCurrentInfo->arDetailsPendingUpdate[i]->Release();
					// (a.walling 2009-10-12 16:05) - PLID 36024
					m_pCurrentInfo->arDetailsPendingUpdate[i]->__Release("CEMRPreviewCtrlDlg::SetEMN pending details");
				}
				m_pCurrentInfo->arDetailsPendingUpdate.RemoveAll();
			}
		}

		// this is a different EMN,
		
		// first, ensure the other emn has written its changes to the file.
		// actually, this is all handled outside of this function.
		/*
		if (m_pCurrentInfo) {
			if (m_pCurrentInfo->pEMN) {
				if (m_pCurrentInfo->pEMN->IsUnsaved() {
					m_pCurrentInfo->pEMN->GenerateHTMLFile(FALSE, FALSE); // don't send message, don't copy to documents (it might be unsaved).
				}
			}
		}
		*/
		
		// set the current info pointer or create a new one.
		CEmnPreviewInfo* pInfo;

		if (m_mapEmnToInfo.Lookup(pEMN, reinterpret_cast<void *&>(pInfo))) {
			if (bSetActive) {
				// (a.walling 2007-07-12 17:06) - PLID 26261 - We secure the previews now, so the temp files are
				// analagous to the working EMN and the archive is the saved EMN. So we need to ensure that our
				// working EMN is saved to disk otherwise we'll lose all of our dynamic changes since the last save.

				// ensure we have the latest everything
				// (a.walling 2008-02-22 18:00) - PLID 28354 - if the EMN is still in its initial load, this will all be taken care of in PostInitialLoad.
				// (a.walling 2008-08-12 13:56) - PLID 31037 - This was actually incorrect, but I'm fixing it as part of this item.
				if (m_pCurrentInfo && m_pCurrentInfo->pEMN && m_pCurrentInfo->pEMN->IsUnsaved() && !m_pCurrentInfo->pEMN->IsLoading()) {
					// first, we need to write our current EMN to disk, although not to documents
					// since it may be unsaved, just to the temp file
										
					// (a.walling 2013-09-05 11:24) - PLID 58369 - GenerateHTMLFile returns path to newly-generated html file
					m_pCurrentInfo->strSource = m_pCurrentInfo->pEMN->GenerateHTMLFile(FALSE, FALSE);
				}
				
				m_pCurrentInfo = pInfo;
			}
		} else {
			// not found, so add to map
			pInfo = new CEmnPreviewInfo(pEMN);
			m_mapEmnToInfo.SetAt(pEMN, pInfo);

			if (bSetActive)
				m_pCurrentInfo = pInfo;
		}

		// now set the new EMN as the current one

		// (a.walling 2008-08-12 13:57) - PLID 31037 - Ensure we have a source. If we don't, we must create one.
		if (!m_pCurrentInfo->pEMN->IsLoading() && (m_pCurrentInfo->strSource.IsEmpty() || (!m_pCurrentInfo->pEMN->IsPreviewCurrent()))) {
			// there is no source, or it is old
			// we need to create it
			// (a.walling 2013-09-05 11:24) - PLID 58369 - GenerateHTMLFile returns path to newly-generated html file
			m_pCurrentInfo->strSource = m_pCurrentInfo->pEMN->GenerateHTMLFile(FALSE, FALSE);
		}

		if (bSetActive) {
			if (m_pCurrentInfo->strSource.GetLength() > 0) {
				// navigate to the new source file
				if (FileUtils::DoesFileOrStreamExist(m_pCurrentInfo->strSource)) {
					COleVariant varUrl(m_pCurrentInfo->strSource);

					if (m_pBrowser) {
						SafeNavigate(varUrl);
					}
				} else {
					// file does not exist, or is not accessible!
					ASSERT(FALSE);
				}		
			} else {
				// there is no source file... we'll just have to wait.
			}
		}
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::SetEMN");
}

BOOL CEMRPreviewCtrlDlg::SetSource(CString strUrl, CEMN* pEMN /*= NULL*/)
{
	try {
		// first off, get the appropriate current info.
		CEmnPreviewInfo* pInfo;
		if (pEMN) {
			if (!m_mapEmnToInfo.Lookup(pEMN, reinterpret_cast<void *&>(pInfo))) {
				// setting source on an EMN that is not associated with the preview!
				return FALSE;
			}
		} else {
			// this is our current emn
			pInfo = m_pCurrentInfo;
		}

		// now pInfo is the info object for the appropriate emn.
		if (pInfo != m_pCurrentInfo) {
			// not the active emn, just set the variable.
			pInfo->strSource = strUrl;
			if (FileUtils::DoesFileOrStreamExist(strUrl)) {
				return TRUE;
			} else {
				return FALSE;
			}
		} else {
			// current emn
			// (a.walling 2007-08-08 16:07) - PLID 27017 - Save the current scroll position
			pInfo->nScrollTop = GetScrollTop();
			pInfo->strSource = strUrl;
			COleVariant varUrl(strUrl);
			SafeNavigate(varUrl);
			return TRUE;
		}
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::SetSource");

	return FALSE;
}

void CEMRPreviewCtrlDlg::OnBeforeNavigate2EmrPreviewBrowser(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags, VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers, BOOL FAR* Cancel) 
{
	try {
		__super::OnBeforeNavigate2(pDisp, URL, Flags, TargetFrameName, PostData, Headers, Cancel);


		CString strUrl = VarString(URL, "");

		// all valid internally handled addresses should be 9 chars (for the nexemr://) plus the command. So we know this is bad.
		if (strUrl.GetLength() <= 9)
			return;

		if (strUrl.Left(9) == "nexemr://") {
			long ixParam = strUrl.Find("/?", 0);
			if (ixParam > 0) {
				CString strCommand = strUrl.Mid(9, ixParam - 9);
				CString strParam = strUrl.Mid(ixParam + 2, strUrl.GetLength() - ixParam - 2);

				// Cancels the navigation action so we can handle it here, internally.
				*Cancel = TRUE;

				// by definition, JavaScript is case sensitive, although HTML is not. So we'll be case sensitive here.

				// (a.walling 2007-04-13 12:48) - PLID 25648 - Prevent navigation actions when not interactive
				//(e.lally 2009-10-01) PLID 32503 - Allow just a few navigation actions that don't actually interact with the EMR
				if (strCommand == "faxemnmhtfile") {
					try {
						if(m_bInteractive) {
							ASSERT(m_pCurrentInfo);
							ASSERT(m_pCurrentInfo->pEMN);

							if (m_pCurrentInfo != NULL && m_pCurrentInfo->pEMN != NULL) {

								//(e.lally 2009-10-26) PLID 32503 - Try to fax our preview, just send in the pointer to our EMN
								FaxEMNPreview(m_pCurrentInfo->pEMN);

							} else {
								ThrowNxException("Fax EMN - Attempted to use current EMN when no EMN is present!");
							}
						}
					} NxCatchAllThrow("Error faxing EMN preview");
					//We processed our command, so stop.
					return;
				}
				// (a.walling 2010-01-11 17:58) - PLID 36840 - Send the custom command to our parent, with the param in LPARAM
				else if (strCommand == "customcommand") {
					long nParam = 0;
					if (!strParam.IsEmpty()) {
						nParam = atoi(strParam);
					}
					// (a.walling 2010-01-12 08:38) - PLID 36840 - Changed to defined message with EMRPREVIEW_ rather than a registered message
					GetParent()->PostMessage(NXM_EMRPREVIEW_CUSTOM_COMMAND, 0, nParam);
					return;
				}
				else if (strCommand == "viewgeneratedsource") {
					try {
						// (a.walling 2007-10-18 14:24) - PLID 25548 - In debug mode, you can view generated source. This is the source of the entire
						// current document as gathered from the DOM.
						// (j.armen 2011-10-25 11:06) - PLID 46136 - To maintain consistency, place this in the session path
						CString str = GetCurrentHTML();

						CStdioFile fOut;
						if (fOut.Open(GetPracPath(PracPath::SessionPath) ^ "NxEMNPreviewSource.html", CFile::modeCreate | CFile::modeWrite | CFile::shareCompat)) {
							fOut.WriteString(str);

							fOut.Close();

							// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
							int nResult = (int)ShellExecute ((HWND)this, NULL, "notepad.exe", (CString("'") + GetPracPath(PracPath::SessionPath) ^ "NxEMNPreviewSource.html" + CString("'")), NULL, SW_SHOW);
						}

						return;
						
					} NxCatchAllThrow("Error viewing generated source");
				}

				//(e.lally 2009-10-01) PLID 32503 - Now we can quit if we are not interactive
				if (!m_bInteractive) {
					return;
				}

				if ( (m_pCurrentInfo == NULL) ||
					(m_pCurrentInfo->pEMN == NULL) ) {
					// (a.walling 2007-10-23 13:58) - PLID 25548 - The only way this can occur is if
					// no EMN is displayed, so just silently return. The action is already cancelled.
					/*ASSERT(FALSE);
					ThrowNxException("Navigate handler invoked before a current EMN has been set!");*/
					return;
				}

				// (j.jones 2007-07-06 12:05) - PLID 25457 - for the purposes of the preference,
				// the HTML code for just "detailID" or "detailPT" will do nothing alone, but instead
				// check the preference and change the command according to the default
				if(strCommand == "detailID" || strCommand == "detailPT") {
					long nEMRPreviewPaneLeftClick = GetRemotePropertyInt("EMRPreviewPaneLeftClick", 0, 0, GetCurrentUserName(), true);
					if(nEMRPreviewPaneLeftClick == 1) //go to topic
						strCommand.Replace("detail","topicDT");
					else //popup detail
						strCommand.Replace("detail","popupdetail");
				}

				// (a.walling 2010-03-26 12:44) - PLID 29099 - Handle clicks on an embedded detail within a narrative
				if(strCommand == "narrativeID" || strCommand == "narrativePT") {					
					if (m_pCurrentInfo->pEMN) {
						long nFieldIndexStart = strParam.Find("/");
						if (nFieldIndexStart != -1) {
							CString strFieldIndex = strParam.Mid(nFieldIndexStart + 1);
							strParam = strParam.Left(nFieldIndexStart);

							long nFieldIndex = atol(strFieldIndex);

							CEMNDetail* pDetail = NULL;
							if (strCommand == "narrativeID") {
								long nDetailID = atol(strParam);
								pDetail = m_pCurrentInfo->pEMN->GetDetailByID(nDetailID);
							} else {
								long nDetailPtr = atol(strParam);
								pDetail = m_pCurrentInfo->pEMN->GetDetailByPointer(nDetailPtr);
							}

							if (pDetail) {
								// (j.armen 2012-12-03 13:56) - PLID 52752 - Changed template for Narrative Field Arrays
								CArray<NarrativeField> arNarrativeFields;
								CString strNxRichText = VarString(pDetail->GetState(), "");

								strNxRichText.TrimRight();

								//TES 6/29/2012 - PLID 50854 - New function that handles HTML narratives
								GetNarrativeFieldArray(strNxRichText, arNarrativeFields);

								if (nFieldIndex >= 0 && nFieldIndex < arNarrativeFields.GetCount()) {
									// (a.walling 2010-03-26 12:44) - PLID 29099 - Begin the traversal of popups from the narrative
									CEmrItemAdvNarrativeBase::HandleLinkRichTextCtrl(arNarrativeFields.GetAt(nFieldIndex).strField, pDetail, m_pCurrentInfo->pEMN->GetInterface()->GetSafeHwnd(), NULL);
								}
							}
						}
					}
				} else if (strCommand == "popupdetailID") {
					// detail by ID
					long nDetailID = atol(strParam);
					try {
						if (nDetailID >= 0) {
							if (m_pCurrentInfo->pEMN) {
								CEMNDetail* pDetail = m_pCurrentInfo->pEMN->GetDetailByID(nDetailID);

								if (pDetail)
								{
									// (a.walling 2007-06-21 15:32) - PLID 22097 - Popups now inherit the readonly state from the detail
									//if (CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE)) {
									pDetail->Popup();
									// Updating the detail is handled in the NXM_EMR_ITEM_STATECHANGED handler in the EmrTreeWnd
									//}
								}
							}
						}
					} NxCatchAllThrow("Error navigating to detail ID");
				} else if (strCommand == "popupdetailPT") {
					// detail by pointer
					long nDetailPtr = atol(strParam);
					try {
						CEMNDetail* pDetail = m_pCurrentInfo->pEMN->GetDetailByPointer(nDetailPtr);
						if (pDetail) {
							// (a.walling 2007-06-21 15:32) - PLID 22097 - Popups now inherit the readonly state from the detail
							//if (CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE)) {
							pDetail->Popup();
							// Updating the detail is handled in the NXM_EMR_ITEM_STATECHANGED handler in the EmrTreeWnd
							//}		
						}
					} NxCatchAllThrow("Error navigating to detail PT");
				// (j.jones 2007-07-06 09:59) - PLID 25457 - migrated all topic browsing to NavigateToTopic,
				// and added topicDTID and topicDTPT to represent jumping to a detail's topic by detail ID or pointer
				} else if (strCommand == "topicID" || strCommand == "topicDTID" ||
					strCommand == "topicPT" || strCommand == "topicDTPT") {

					NavigateToTopic(strParam, strCommand);
				
				} else if (strCommand == "editdetailID" || strCommand == "editdetailPT") {

					// edit detail by ID or pointer
					CEMNDetail* pDetail = NULL;
					
					try {

						//before editing, navigate to that topic
						CString str = strCommand;
						str.Replace("editdetail", "topicDT");
						NavigateToTopic(strParam, str);

						if(strCommand == "editdetailID") {
							//we were given a detail ID
							long nDetailID = atol(strParam);
							if (nDetailID >= 0) {
								if (m_pCurrentInfo->pEMN) {
									pDetail = m_pCurrentInfo->pEMN->GetDetailByID(nDetailID);
								}
							}
						}
						else if(strCommand == "editdetailPT") {
							//we were given a detail pointer
							long nDetailPtr = atol(strParam);
							pDetail = m_pCurrentInfo->pEMN->GetDetailByPointer(nDetailPtr);
						}

						//(e.lally 2010-05-03) PLID 15155 - Check for Writable EMN state instead of Edit Mode now
						if (pDetail && IsEmnWritable()) {
							if(pDetail->m_pEmrItemAdvDlg) {
								// (a.walling 2008-03-25 08:17) - PLID 28811 - Pass a pointer to the now static function
								pDetail->m_pEmrItemAdvDlg->OpenItemEntryDlg(pDetail, FALSE);
							}
						}
					} NxCatchAllThrow("Error editing detail");
				} else if (strCommand == "detailhidetitleID" || strCommand == "detailhidetitlePT"
					|| strCommand == "detailhideitemID" || strCommand == "detailhideitemPT" 
					|| strCommand == "detailtogglesubdetailPT" || strCommand == "detailtogglesubdetailID"
					|| strCommand == "detailtogglehideifindirectPT" || strCommand == "detailtogglehideifindirectID"
					|| strCommand == "detailtogglehideitemonipadID"
					|| strCommand == "detailcoloneID" || strCommand == "detailcolonePT" 
					|| strCommand == "detailcoltwoID" || strCommand == "detailcoltwoPT" 
					/*
					|| strCommand == "detailclearleftID" || strCommand == "detailclearleftPT" 
					|| strCommand == "detailclearrightID" || strCommand == "detailclearrightPT" 
					*/
					|| strCommand == "detailtextrightID" || strCommand == "detailtextrightPT"
					// (a.walling 2010-08-31 18:20) - PLID 36148 - Page breaks
					|| strCommand == "detailpagebreakbeforeID" || strCommand == "detailpagebreakbeforePT"
					|| strCommand == "detailpagebreakafterID" || strCommand == "detailpagebreakafterPT"
					) {
					// (a.walling 2009-01-07 15:21) - PLID 32695 - handle toggling detail float/clear/text properties
					// (a.walling 2008-10-23 10:39) - PLID 31808 - handle toggling subdetail display
					// (a.walling 2008-07-01 10:26) - PLID 30570 - Toggle these preview flags	
					// (a.walling 2009-07-06 10:24) - PLID 34793 - floatleft/floatright are now colone/coltwo
					CEMNDetail* pDetail = NULL;
					if (strCommand.Right(2) == "ID") {
						//we were given a detail ID
						long nDetailID = atol(strParam);
						if (nDetailID >= 0) {
							if (m_pCurrentInfo->pEMN) {
								pDetail = m_pCurrentInfo->pEMN->GetDetailByID(nDetailID);
							}
						}					
					} else if (strCommand.Right(2) == "PT") {
						//we were given a detail pointer
						long nDetailPtr = atol(strParam);							
						if (m_pCurrentInfo->pEMN) {
							pDetail = m_pCurrentInfo->pEMN->GetDetailByPointer(nDetailPtr);
						}
					}

					if (pDetail) {
						DWORD nNewFlags = pDetail->GetPreviewFlags();
						BOOL bRefreshParent = FALSE;
						BOOL bRefreshOnlySelf = FALSE;

						if (strCommand.Find("hideitem") != -1 && strCommand.Find("hideitemonipad") == -1) {
							nNewFlags ^= epfHideItem;
							bRefreshParent = TRUE;
						} else if (strCommand.Find("hidetitle") != -1) {
							nNewFlags ^= epfHideTitle;
							bRefreshOnlySelf = TRUE;
						} else if (strCommand.Find("togglesubdetail") != -1) {
							// (a.walling 2008-10-23 10:39) - PLID 31808 - handle toggling subdetail display
							nNewFlags ^= epfSubDetail;
							bRefreshParent = TRUE;
						} else if (strCommand.Find("togglehideifindirect") != -1) {
							// (a.walling 2012-07-13 16:38) - PLID 48896
							nNewFlags ^= epfHideIfIndirect;
							bRefreshParent = TRUE;
						} else if (strCommand.Find("detailtogglehideitemonipadID") != -1) {
							// (j.armen 2013-01-16 16:41) - PLID 54412 - No need to refresh since this will not affect the preview here.
							nNewFlags ^= epfHideOnIPad;
							bRefreshOnlySelf = TRUE;
						} else if (strCommand.Find("colone") != -1) {
							// (a.walling 2009-07-06 10:14) - PLID 34793 - epfFloatLeft is now epfColumnOne, epfFloatRight is now epfColumnTwo
							nNewFlags &= ~epfColumnTwo;
							nNewFlags ^= epfColumnOne;

							bRefreshParent = TRUE;
						} else if (strCommand.Find("coltwo") != -1) {
							// (a.walling 2009-07-06 10:14) - PLID 34793 - epfFloatLeft is now epfColumnOne, epfFloatRight is now epfColumnTwo
							nNewFlags &= ~epfColumnOne;
							nNewFlags ^= epfColumnTwo;

							bRefreshParent = TRUE;
						} else if (strCommand.Find("textright") != -1) {
							// (a.walling 2009-01-08 14:07) - PLID 32660 - Align text right
							nNewFlags ^= epfTextRight;
							bRefreshOnlySelf = TRUE;
						} else if (strCommand.Find("pagebreakbefore") != -1) {
							// (a.walling 2010-08-31 18:20) - PLID 36148 - toggle page break before
							nNewFlags &= ~epfPageBreakAfter;
							nNewFlags ^= epfPageBreakBefore;
							bRefreshOnlySelf = TRUE;
						} else if (strCommand.Find("pagebreakafter") != -1) {
							// (a.walling 2010-08-31 18:20) - PLID 36148 - toggle page break after
							nNewFlags &= ~epfPageBreakBefore;
							nNewFlags ^= epfPageBreakAfter;
							bRefreshOnlySelf = TRUE;
						}
						// (a.walling 2009-07-06 08:40) - PLID 34793 - Clearing is deprecated
						/*else if (strCommand.Find("clearleft") != -1) {
							if ((nNewFlags & epfClearNone) == 0) { // both
								nNewFlags ^= epfClearRight;
							} else {
								if ((nNewFlags & epfClearNone) == epfClearNone) { // none
									nNewFlags &= ~epfClearRight;
									nNewFlags |= epfClearLeft;
								} else if (nNewFlags & epfClearRight) { // right
									nNewFlags &= ~epfClearRight;
									nNewFlags &= ~epfClearLeft;
								} else if (nNewFlags & epfClearLeft) { // left
									nNewFlags |= epfClearNone;
								}
							}
							bRefreshOnlySelf = TRUE;
						} else if (strCommand.Find("clearright") != -1) {
							if ((nNewFlags & epfClearNone) == 0) { // both
								nNewFlags ^= epfClearLeft;
							} else {
								if ((nNewFlags & epfClearNone) == epfClearNone) { // none
									nNewFlags &= ~epfClearLeft;
									nNewFlags |= epfClearRight;
								} else if (nNewFlags & epfClearLeft) { // left
									nNewFlags &= ~epfClearLeft;
									nNewFlags &= ~epfClearRight;
								} else if (nNewFlags & epfClearRight) { // right
									nNewFlags |= epfClearNone;
								}
							}
							bRefreshOnlySelf = TRUE;
						}*/

						pDetail->SetPreviewFlags(nNewFlags, bRefreshParent, bRefreshOnlySelf);
					}
				} else if (strCommand == "topichideitemID" || strCommand == "topichideitemPT"
					|| strCommand == "topichidetitleID" || strCommand == "topichidetitlePT"
					|| strCommand == "topichidetitleID" || strCommand == "topichidetitlePT"
					|| strCommand == "topiccoloneID" || strCommand == "topiccolonePT" 
					|| strCommand == "topiccoltwoID" || strCommand == "topiccoltwoPT" 
					/*
					|| strCommand == "topicclearleftID" || strCommand == "topicclearleftPT" 
					|| strCommand == "topicclearrightID" || strCommand == "topicclearrightPT"
					*/
					// (a.walling 2009-07-06 12:41) - PLID 34793
					|| strCommand == "topicgroupbeginID" || strCommand == "topicgroupbeginPT"
					|| strCommand == "topicgroupendID" || strCommand == "topicgroupendPT"
					|| strCommand == "topictextrightID" || strCommand == "topictextrightPT"
					// (a.walling 2010-08-31 18:20) - PLID 36148 - Page breaks
					|| strCommand == "topicpagebreakbeforeID" || strCommand == "topicpagebreakbeforePT" 
					|| strCommand == "topicpagebreakafterID" || strCommand == "topicpagebreakafterPT") {
					// (a.walling 2009-01-07 15:21) - PLID 32695 - handle toggling topic float/clear properties
					// (a.walling 2008-07-01 10:26) - PLID 30570 - Toggle these preview flags								
					// (a.walling 2009-07-06 10:24) - PLID 34793 - floatleft/floatright are now colone/coltwo
					CEMRTopic* pTopic = NULL;
					if (strCommand.Right(2) == "ID") {
						//we were given a topic ID
						long nTopicID = atol(strParam);

						if (nTopicID >= 0) {
							if (m_pCurrentInfo->pEMN) {								
								pTopic = m_pCurrentInfo->pEMN->GetTopicByID(nTopicID);
							}
						}			
					} else if (strCommand.Right(2) == "PT") {
						//we were given a topic pointer
						long nTopicPtr = atol(strParam);								
						if (m_pCurrentInfo->pEMN) {						
							pTopic = m_pCurrentInfo->pEMN->GetTopicByPointer(nTopicPtr);
						}
					}

					if (pTopic) {
						BOOL bRefreshParent = FALSE;
						DWORD nNewFlags = pTopic->GetPreviewFlags();
						if (strCommand.Find("hideitem") != -1) {
							nNewFlags ^= epfHideItem;
						} else if (strCommand.Find("hidetitle") != -1) {
							nNewFlags ^= epfHideTitle;
						} else if (strCommand.Find("colone") != -1) {
							// (a.walling 2009-07-06 10:14) - PLID 34793 - epfFloatLeft is now epfColumnOne, epfFloatRight is now epfColumnTwo
							nNewFlags &= ~epfColumnTwo;
							nNewFlags ^= epfColumnOne;

							bRefreshParent = TRUE;
						} else if (strCommand.Find("coltwo") != -1) {
							// (a.walling 2009-07-06 10:14) - PLID 34793 - epfFloatLeft is now epfColumnOne, epfFloatRight is now epfColumnTwo
							nNewFlags &= ~epfColumnOne;
							nNewFlags ^= epfColumnTwo;
							
							bRefreshParent = TRUE;
						} else if (strCommand.Find("textright") != -1) {
							// (a.walling 2009-01-08 14:07) - PLID 32660 - Align text right
							nNewFlags ^= epfTextRight;
						} else if (strCommand.Find("groupbegin") != -1) {
							// (a.walling 2009-07-06 12:44) - PLID 34793 - Grouping options for columns
							
							// turn off group end if it is on
							nNewFlags &= ~epfGroupEnd;
							
							// toggle group begin
							nNewFlags ^= epfGroupBegin;
						} else if (strCommand.Find("groupend") != -1) {
							// (a.walling 2009-07-06 12:44) - PLID 34793 - Grouping options for columns
							
							// turn off group begin if it is on
							nNewFlags &= ~epfGroupBegin;
							
							// toggle group end
							nNewFlags ^= epfGroupEnd;
						} else if (strCommand.Find("pagebreakbefore") != -1) {
							// (a.walling 2010-08-31 18:20) - PLID 36148 - toggle page break before
							nNewFlags &= ~epfPageBreakAfter;
							nNewFlags ^= epfPageBreakBefore;
						} else if (strCommand.Find("pagebreakafter") != -1) {
							// (a.walling 2010-08-31 18:20) - PLID 36148 - toggle page break after
							nNewFlags &= ~epfPageBreakBefore;
							nNewFlags ^= epfPageBreakAfter;
						}

						// (a.walling 2009-07-06 08:40) - PLID 34793 - Clearing is deprecated
						/*} else if (strCommand.Find("clearleft") != -1) {
							if ((nNewFlags & epfClearNone) == 0) { // both
								nNewFlags ^= epfClearRight;
							} else {
								if ((nNewFlags & epfClearNone) == epfClearNone) { // none
									nNewFlags &= ~epfClearRight;
									nNewFlags |= epfClearLeft;
								} else if (nNewFlags & epfClearRight) { // right
									nNewFlags &= ~epfClearRight;
									nNewFlags &= ~epfClearLeft;
								} else if (nNewFlags & epfClearLeft) { // left
									nNewFlags |= epfClearNone;
								}
							}
						} else if (strCommand.Find("clearright") != -1) {
							if ((nNewFlags & epfClearNone) == 0) { // both
								nNewFlags ^= epfClearLeft;
							} else {
								if ((nNewFlags & epfClearNone) == epfClearNone) { // none
									nNewFlags &= ~epfClearLeft;
									nNewFlags |= epfClearRight;
								} else if (nNewFlags & epfClearLeft) { // left
									nNewFlags &= ~epfClearLeft;
									nNewFlags &= ~epfClearRight;
								} else if (nNewFlags & epfClearRight) { // right
									nNewFlags |= epfClearNone;
								}
							}
						}*/

						pTopic->SetPreviewFlags(nNewFlags, bRefreshParent);
					}					
				} else if (strCommand == "forcerefresh") {
					try {
						ASSERT(m_pCurrentInfo);
						ASSERT(m_pCurrentInfo->pEMN);

						if (m_pCurrentInfo != NULL && m_pCurrentInfo->pEMN != NULL) {
							// (a.walling 2007-08-08 16:41) - PLID 27017 - Don't copy to documents, that should only be done when saving.
							m_pCurrentInfo->pEMN->GenerateHTMLFile(TRUE, FALSE, FALSE);
						} else {
							ThrowNxException("Attempted to manually refresh when no EMN is currently present!");
						}
					} NxCatchAllThrow("Error manually refreshing preview");
				} else if (strCommand == "moreinfo") {
					// (a.walling 2007-07-12 15:50) - PLID 26640 - Navigate to the more info topic
					try {
						ASSERT(m_pTreeWnd);

						if(m_pTreeWnd) {
							//etrtMoreInfo
							// (c.haag 2007-09-28 11:42) - PLID 27509 - Dont use protected variables
							/*
							NXDATALIST2Lib::IRowSettingsPtr pEmnRow = m_pTreeWnd->m_pTree->FindByColumn(TREE_COLUMN_OBJ_PTR, _variant_t(long(m_pCurrentInfo->pEMN)), NULL, FALSE);
							
							NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTreeWnd->m_pTree->FindByColumn(TREE_COLUMN_ROW_TYPE, _variant_t((long)etrtMoreInfo), pEmnRow, TRUE);*/

							NXDATALIST2Lib::IRowSettingsPtr pEmnRow = m_pTreeWnd->FindInTreeByColumn(TREE_COLUMN_OBJ_PTR, _variant_t(long(m_pCurrentInfo->pEMN)), NULL);
							NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTreeWnd->FindInTreeByColumn(TREE_COLUMN_ROW_TYPE, _variant_t((long)etrtMoreInfo), pEmnRow);
							if (NULL != pRow) { m_pTreeWnd->SetTreeSel(pRow); }

						} else {
							ThrowNxException("Attempted to navigate to more info topic without a treewnd!");
						}
					} NxCatchAllThrow("Error navigating to more info");
				} else {
					MessageBox(FormatString("Unsupported command! (%s)", strUrl));
				}
			}
		}
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::OnBeforeNavigate2EmrPreviewBrowser");
}

BOOL CEMRPreviewCtrlDlg::InsertDetail(CEMNDetail* pDetail, CEMNDetail* pDetailInsertBefore /* = NULL*/)
{
	try {
		if (!IsLoaded()) return FALSE;

		// (a.walling 2007-10-12 09:07) - PLID 25548 - Ensure this is for the correct EMN
		if (	(pDetail != NULL && pDetail->m_pParentTopic != NULL && pDetail->m_pParentTopic->GetParentEMN() != GetCurrentEMN()) ||
				(pDetailInsertBefore != NULL && pDetailInsertBefore->m_pParentTopic != NULL && pDetailInsertBefore->m_pParentTopic->GetParentEMN() != GetCurrentEMN())
			) {
			return FALSE;
		}

		// okay, first we need to find the topic, then insert our detail
		// optional pDetailInsertBefore parameter to support inserting in order
		if (pDetail) {
			IHTMLElementPtr pDetailElement = GetElementByIDOrPointer("DetailDiv", pDetail->GetID(), reinterpret_cast<long>(pDetail));

			if (pDetailElement) {
				// (a.walling 2007-04-11 10:35) - PLID 25549 - Ensure we are not duplicating items
				// the element already exists in the preview!!
				return MoveDetail(pDetail, pDetailInsertBefore);
			} else {
				IHTMLElementPtr pElement = NULL;
				CString strPosition;

				if (pDetailInsertBefore) {
					// get the DOM element of this detail to insert before (ie, the next element)
					pElement = GetElementByIDOrPointer("DetailDiv", pDetailInsertBefore->GetID(), reinterpret_cast<long>(pDetailInsertBefore));
					strPosition = "beforeBegin"; // insert directly before the relative element begins
				}

				if (pElement == NULL) {
					// try to get the topic
					CEMRTopic* pTopic = pDetail->m_pParentTopic;
					if (pTopic) {
						 pElement = GetElementByIDOrPointer("TopicDivDetails", pTopic->GetID(), reinterpret_cast<long>(pTopic));
						 strPosition = "beforeEnd"; // insert at the end of the topic's details
					}
				}
				
				if (pElement) {
					// (a.walling 2008-10-23 09:52) - PLID 27552 - Set bCheckSpawnedOutput to TRUE so it will simply return
					// an empty string if the detail has no real output (such as if it is displaying under a different detail)
					CString strNewHTML = pDetail->GetHTML(TRUE);

					if (!strNewHTML.IsEmpty()) {
						BSTR bstrNewHTML = strNewHTML.AllocSysString();

						// insert the new item before the end of the topic
						HRESULT hr = pElement->insertAdjacentHTML(_bstr_t(strPosition), bstrNewHTML);

						SysFreeString(bstrNewHTML);

						if (SUCCEEDED(hr))
							return TRUE;
						else
							return FALSE;
					} else {
						// (a.walling 2008-10-23 10:00) - PLID 27552 - Update the parent detail
						if (pDetail->IsSubDetail()) {
							CEMNDetail* pParentDetail = pDetail->GetSubDetailParent();

							if (pParentDetail) {
								return UpdateDetail(pParentDetail);
							}
						}
						return TRUE;
					}
				}
				
			}
		}
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::InsertDetail");
	return FALSE;
}

BOOL CEMRPreviewCtrlDlg::MoveDetail(CEMNDetail* pDetail, CEMNDetail* pDetailInsertBefore)
{
	try {
		if (!IsLoaded()) return FALSE;

		// we need to find the detail, find the topic, and then insert the detail to the end of that topic
		// (inserting an existing html element will move that element)

		if (pDetail) {
			// (a.walling 2007-10-12 09:07) - PLID 25548 - Ensure this is for the correct EMN
			if (	(pDetail->m_pParentTopic != NULL && pDetail->m_pParentTopic->GetParentEMN() != GetCurrentEMN()) ||
					(pDetailInsertBefore != NULL && pDetailInsertBefore->m_pParentTopic != NULL && pDetailInsertBefore->m_pParentTopic->GetParentEMN() != GetCurrentEMN())
				) {
				return FALSE;
			}

			CString strPosition;
			IHTMLElement2Ptr pRelativeElement = NULL;
			
			if (pDetailInsertBefore) {
				pRelativeElement = GetElementByIDOrPointer("DetailDiv", pDetailInsertBefore->GetID(), reinterpret_cast<long>(pDetailInsertBefore));
				strPosition = "beforeBegin"; // move immediately before this detail
			}

			if (pRelativeElement == NULL) { // try to get the parent topic
				CEMRTopic* pTopic = pDetail->m_pParentTopic;
				if (pTopic) {
					pRelativeElement = GetElementByIDOrPointer("TopicDivDetails", pTopic->GetID(), reinterpret_cast<long>(pTopic));
					strPosition = "beforeEnd"; // move immediately before the end of the topic details
				}
			}

			if (pRelativeElement) {
				// we have our relative element, so get the DOM object to move and then insert it.
				IHTMLElementPtr pElement = GetElementByIDOrPointer("DetailDiv", pDetail->GetID(), reinterpret_cast<long>(pDetail));

				if (pElement) {
					IHTMLElementPtr pParentElement = NULL;
					HRESULT hrParent = pElement->get_parentElement(&pParentElement);
					// don't need to check this HRESULT, really. We'll check for pParentElement = NULL soon.

					// insert/move the element relative to RelativeElement (based on strPosition)
					HRESULT hr = pRelativeElement->insertAdjacentElement(_bstr_t(strPosition), pElement, NULL);

					// (a.walling 2008-10-23 09:55) - PLID 27552 - Need to take care of the SubDetailDiv section (if it exists) since it is
					// a sibling of the DetailDiv and not a child.
					{
						IHTMLElementPtr pSubElement = GetElementByIDOrPointer("SubDetailDiv", pDetail->GetID(), reinterpret_cast<long>(pDetail));
						IHTMLElement2Ptr pElement2(pElement);

						if (pSubElement && pElement2) {
							pElement2->insertAdjacentElement(_bstr_t("afterEnd"), pSubElement, NULL);
						}
					}

					if (SUCCEEDED(hr)) {
						// we've moved the element! Now let's check out our pParentElement... internet explorer will secretly
						// insert an &nbsp; where the detail used to be. Let's get rid of it!

						// Amazingly, setting innerHTML or innerText will not rid us of this pesky non-breaking space.
						// But by getting the outerHTML and removing it, it works!
						// It is possible that elsewhere in the outerHTML are some more &nbsp;'s, so we check
						// for some key characters to minimize that possibility.

						// (c.haag 2014-02-12) - PLID 45629 - This code also destroys legitimate &nbsp; symbols. I also do not
						// see the root issue happening any longer; at least not in IE 10.

						/*
						if (SUCCEEDED(hrParent) && (pParentElement != NULL)) {
							BSTR bstrOuterHTML;
							hr = pParentElement->get_outerHTML(&bstrOuterHTML);
							if (SUCCEEDED(hr)) {
								// (a.walling 2007-07-09 09:26) - The false in the constructor means that no copy is made
								_bstr_t bstrtOuterHTML(bstrOuterHTML, false);

								CString str = (LPCTSTR)bstrtOuterHTML;
								// IE's DOM model internally stores all tags uppercase
								str.Replace(">&nbsp;</DIV>", "></DIV>");

								_bstr_t bstrtNewHTML((LPCTSTR)str);
									
								hr = pParentElement->put_outerHTML(bstrtNewHTML);
								return SUCCEEDED(hr);
							}
						}
						return FALSE;*/

						return TRUE;

					} else {
						return FALSE;
					}
				}
			}
		}
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::MoveDetail");

	return FALSE;
}

BOOL CEMRPreviewCtrlDlg::RemoveDetail(CEMNDetail* pDetail)
{
	try {
		if (!IsLoaded()) return FALSE;

		if (pDetail) {
			// (a.walling 2007-10-12 09:07) - PLID 25548 - Ensure this is for the correct EMN
			if (pDetail->m_pParentTopic != NULL && pDetail->m_pParentTopic->GetParentEMN() != GetCurrentEMN()) {
				return FALSE;
			}

			IHTMLElementPtr pElement;

			// (a.walling 2007-04-11 10:35) - PLID 25549 - Ensure this item, and any duplicates, are gone
			do {
				{
					// (a.walling 2008-10-23 09:55) - PLID 27552 - Need to take care of the SubDetailDiv section (if it exists) since it is
					// a sibling of the DetailDiv and not a child.
					IHTMLElementPtr pSubElement = GetElementByIDOrPointer("SubDetailDiv", pDetail->GetID(), reinterpret_cast<long>(pDetail));

					if (pSubElement) {
						HRESULT hr = pSubElement->put_outerHTML(_bstr_t(""));
					}
				}

				pElement = GetElementByIDOrPointer("DetailDiv", pDetail->GetID(), reinterpret_cast<long>(pDetail));

				if (pElement) {
					HRESULT hr = pElement->put_outerHTML(_bstr_t(""));

					if (!SUCCEEDED(hr))
						return FALSE;
				}
			} while (pElement);

			return TRUE;
		}
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::RemoveDetail");

	return FALSE;
}

// (a.walling 2007-10-12 11:23) - PLID 27017 - Returns whether the detail is in any pending update array
BOOL CEMRPreviewCtrlDlg::IsDetailInPendingUpdateArray(CEMNDetail* pDetail)
{
	try {
		POSITION pos = m_mapEmnToInfo.GetStartPosition();
		while (pos) {
			void* pEMNKey;
			CEmnPreviewInfo* pInfo;

			m_mapEmnToInfo.GetNextAssoc(pos, pEMNKey, reinterpret_cast<void *&>(pInfo));
			
			if (pInfo) {
				for (int i = 0; i < pInfo->arDetailsPendingUpdate.GetSize(); i++) {
					if (pInfo->arDetailsPendingUpdate[i] == pDetail) {
						return TRUE;
					}
				}
			}
		}		
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::IsDetailInPendingUpdateArray");

	return FALSE;
}

// (a.walling 2007-10-12 10:42) - PLID 27017 - Used to update a detail when the preview has not completed loading yet
void CEMRPreviewCtrlDlg::PendUpdateDetail(CEMNDetail* pDetail)
{
	try {
		if (m_pCurrentInfo) {
			if (pDetail->m_pParentTopic != NULL && pDetail->m_pParentTopic->GetParentEMN() != GetCurrentEMN()) {
				return;
			}

			// (a.walling 2007-10-15 17:44) - PLID 27017 - Unfortunately, we were not checking here to ensure that no
			// duplicates were being added into the pending update array, which was causing a dangling reference.
			if (!IsDetailInPendingUpdateArray(pDetail)) {
				//pDetail->AddRef();
				// (a.walling 2009-10-12 16:05) - PLID 36024
				pDetail->__AddRef("CEMRPreviewCtrlDlg::PendUpdateDetail");
				m_pCurrentInfo->arDetailsPendingUpdate.Add(pDetail);
			}
		}
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::PendUpdateDetail");
}

BOOL CEMRPreviewCtrlDlg::UpdateDetail(CEMNDetail* pDetail)
{
	try {
		if (!IsLoaded()) return FALSE;

		// (a.walling 2007-10-12 09:07) - PLID 25548 - Ensure this is for the correct EMN
		if (pDetail->m_pParentTopic != NULL && pDetail->m_pParentTopic->GetParentEMN() != GetCurrentEMN()) {
			return FALSE;
		}

		{
			// (a.walling 2008-10-23 09:55) - PLID 27552 - Need to take care of the SubDetailDiv section (if it exists) since it is
			// a sibling of the DetailDiv and not a child.
			IHTMLElementPtr pSubElement = GetElementByIDOrPointer("SubDetailDiv", pDetail->GetID(), reinterpret_cast<long>(pDetail));
			if (pSubElement) {
				pSubElement->put_outerHTML(_bstr_t(""));
			}
		}

		{
			IHTMLElementPtr pElement = GetElementByIDOrPointer("DetailDiv", pDetail->GetID(), reinterpret_cast<long>(pDetail));

			if (pElement) {
				// Images have a ? parameter of GetTickCount to force the MSHTML control to ignore the cache.
				CString strNewHTML = pDetail->GetHTML();
		
				BSTR bstrNewHTML = strNewHTML.AllocSysString();
				
				pElement->put_outerHTML(bstrNewHTML);

				SysFreeString(bstrNewHTML);

				return TRUE;
			} else {
				// (a.walling 2007-07-09 16:47) - PLID 27017 - Log when this fails
				// (a.walling 2007-08-09 09:42) - PLID 27017 - I think we are done with the logging. There are valid situations
				// where this would fail.
				/*
				ASSERT(FALSE);
				LogDetail("Could not find detail (ID:%li, PT:%li)", pDetail->GetID(), reinterpret_cast<long>(pDetail));
				*/
			}
		}

	} NxCatchAll("Error in CEMRPreviewCtrlDlg::UpdateDetail");

	return FALSE;
}

BOOL CEMRPreviewCtrlDlg::UpdateTopic(CEMRTopic* pTopic)
{
	try {
		if (!IsLoaded()) return FALSE;

		// (a.walling 2007-10-12 09:07) - PLID 25548 - Ensure this is for the correct EMN
		if (pTopic->GetParentEMN() != GetCurrentEMN()) {
			return FALSE;
		}

		IHTMLElementPtr pElement = GetElementByIDOrPointer("TopicDiv", pTopic->GetID(), reinterpret_cast<long>(pTopic));

		if (pElement) {
			CString strNewHTML = pTopic->GetHTML();
			BSTR bstrNewHTML = strNewHTML.AllocSysString();
			
			pElement->put_outerHTML(bstrNewHTML);

			SysFreeString(bstrNewHTML);

			return TRUE;
		} else {
			// (a.walling 2007-07-09 16:47) - PLID 27017 - Log when this fails
			// (a.walling 2007-08-09 09:42) - PLID 27017 - I think we are done with the logging. There are valid situations
			// where this would fail.
			/*
			ASSERT(FALSE);
			LogDetail("Could not find topic (ID:%li, PT:%li)", pTopic->GetID(), reinterpret_cast<long>(pTopic));
			*/
		}
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::UpdateTopic");

	return FALSE;
}

BOOL CEMRPreviewCtrlDlg::UpdateTopicTitle(CEMRTopic* pTopic, CString &strTitle)
{
	try {
		if (!IsLoaded()) return FALSE;

		// (a.walling 2007-10-12 09:07) - PLID 25548 - Ensure this is for the correct EMN
		if (pTopic->GetParentEMN() != GetCurrentEMN()) {
			return FALSE;
		}

		IHTMLElement2Ptr pElement = GetElementByIDOrPointer("TopicDiv", pTopic->GetID(), reinterpret_cast<long>(pTopic));

		if (pElement) {
			IHTMLElementCollectionPtr pChildren = NULL;
			HRESULT hr = pElement->getElementsByTagName(_bstr_t("a"), &pChildren);

			if (SUCCEEDED(hr) && pChildren != NULL) {
				COleVariant varIndex((long)0, VT_I4);

				long nCount = 0;
				pChildren->get_length(&nCount);

				// our first anchor is the title we need.

				IHTMLElementPtr pTitleElement = NULL;
				// (b.cardillo 2007-09-19 12:06) - PLID 27439 - Changed this from a regular pointer variable to a smart-pointer 
				// object variable so it would automatically be released when it goes out of scope.  Prior to this change, the 
				// underlying COM object was being leaked.
				IDispatchPtr pDispItem = NULL;
				hr = pChildren->item(varIndex, varIndex, &pDispItem);

				if (SUCCEEDED(hr) && pDispItem != NULL) {
					hr = pDispItem->QueryInterface(IID_IHTMLElement, (void**)&pTitleElement);

					if (SUCCEEDED(hr) && pTitleElement != NULL) {
						pTitleElement->put_innerText(_bstr_t(strTitle));

						return TRUE;
					}
				}	
			}
		}
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::UpdateTopicTitle");

	return FALSE;
}

BOOL CEMRPreviewCtrlDlg::UpdateEMNTitle(CString &strTitle)
{
	try {
		if (!IsLoaded()) return FALSE;

		IHTMLElement2Ptr pElement = GetElementByID("EMN");

		if (pElement) {
			IHTMLElementCollectionPtr pChildren = NULL;
			HRESULT hr = pElement->getElementsByTagName(_bstr_t("h1"), &pChildren);

			if (SUCCEEDED(hr) && pChildren != NULL) {
				COleVariant varIndex((long)0, VT_I4);

				long nCount = 0;
				pChildren->get_length(&nCount);

				// our first h1 is the title we need.

				IHTMLElementPtr pTitleElement = NULL;
				// (b.cardillo 2007-09-19 12:06) - PLID 27439 - Changed this from a regular pointer variable to a smart-pointer 
				// object variable so it would automatically be released when it goes out of scope.  Prior to this change, the 
				// underlying COM object was being leaked.
				IDispatchPtr pDispItem = NULL;
				hr = pChildren->item(varIndex, varIndex, &pDispItem);

				if (SUCCEEDED(hr) && pDispItem != NULL) {
					hr = pDispItem->QueryInterface(IID_IHTMLElement, (void**)&pTitleElement);

					if (SUCCEEDED(hr) && pTitleElement != NULL) {
						pTitleElement->put_innerText(_bstr_t(strTitle));

						return TRUE;
					}
				}	
			}
		}
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::UpdateEMNTitle");

	return FALSE;
}

// (a.walling 2007-07-12 15:11) - PLID 26640 - Update the More Info section and the header
BOOL CEMRPreviewCtrlDlg::UpdateMoreInfo(CEMN* pEMN)
{
	try {
		if (!IsLoaded()) return FALSE;
		// first update the header

		// (a.walling 2007-10-12 09:07) - PLID 25548 - Ensure this is for the correct EMN
		if (pEMN != GetCurrentEMN()) {
			return FALSE;
		}

		ASSERT(pEMN);
		if (pEMN) {
			{
				IHTMLElementPtr pHeader = GetElementByID("demographics");

				if (pHeader) {
					pHeader->put_outerHTML(_bstr_t(pEMN->GenerateHeaderHTML()));
				}
			}

			{
				IHTMLElementPtr pMoreInfo = GetElementByID("moreinfo");

				if (pMoreInfo) {
					pMoreInfo->put_outerHTML(_bstr_t(pEMN->GenerateMoreInfoHTML()));
				}
			}
		}

		return TRUE;
	} NxCatchAll("Error updating More Info preview section");

	return FALSE;
}

BOOL CEMRPreviewCtrlDlg::InsertTopic(CEMRTopic* pTopic)
{
	try {
		if (!IsLoaded()) return FALSE;

		// (a.walling 2007-10-12 09:07) - PLID 25548 - Ensure this is for the correct EMN
		if (pTopic != NULL && pTopic->GetParentEMN() != GetCurrentEMN()) {
			return FALSE;
		}

		// (a.walling 2007-08-24 10:46) - PLID 26640 - the end of our list of topics is before the more info section, if it exists.
		IHTMLElementPtr pMoreInfoElement = GetElementByID("moreinfo");
		if (pMoreInfoElement) {
			// we will be inserting a new Topic at the end of the list but before moreinfo
			CString strNewHTML = pTopic->GetHTML();

			BSTR bstrNewHTML = strNewHTML.AllocSysString();

			HRESULT hr = pMoreInfoElement->insertAdjacentHTML(_bstr_t("beforeBegin"), bstrNewHTML);
			SysFreeString(bstrNewHTML);

			if (SUCCEEDED(hr)) {
				return TRUE;
			} else {
				return FALSE;
			}
		}

		// if there was no more info, put it at the end of the EMN.

		IHTMLElementPtr pEMNElement = GetElementByID("EMN");

		if (pEMNElement) {
			// pEMNElement is the root element (<div ID="EMN">)
			// we will be inserting a new Topic after this element, but before its closing, effectively the end of the list
			CString strNewHTML = pTopic->GetHTML();

			BSTR bstrNewHTML = strNewHTML.AllocSysString();

			HRESULT hr = pEMNElement->insertAdjacentHTML(_bstr_t("beforeEnd"), bstrNewHTML);
			SysFreeString(bstrNewHTML);

			if (SUCCEEDED(hr)) {
				return TRUE;
			} else {
				return FALSE;
			}
		}
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::InsertTopic");

	return FALSE;
}

// (a.walling 2007-10-10 09:22) - PLID 25548 - added bStrict parameter for all InsertTopic functions to prevent
// adding at the end if the parent or sibling was not found.
BOOL CEMRPreviewCtrlDlg::InsertSubTopic(CEMRTopic* pParentTopic, CEMRTopic* pTopic, BOOL bStrict /*= FALSE*/)
{
	try {
		if (!IsLoaded()) return FALSE;

		// (a.walling 2007-10-12 09:07) - PLID 25548 - Ensure this is for the correct EMN
		if (	(pTopic != NULL && pTopic->GetParentEMN() != GetCurrentEMN()) ||
				(pParentTopic != NULL && pParentTopic->GetParentEMN() != GetCurrentEMN())
			) {
			return FALSE;
		}

		IHTMLElementPtr pParentElement = GetElementByIDOrPointer("TopicDivTopics", pParentTopic->GetID(), reinterpret_cast<long>(pParentTopic));

		if (pParentElement) {
			// pParentElement is the parent topic
			// we will be inserting a new Topic after this element, but before its closing, effectively the end of the list
			CString strNewHTML = pTopic->GetHTML();

			BSTR bstrNewHTML = strNewHTML.AllocSysString();

			HRESULT hr = pParentElement->insertAdjacentHTML(_bstr_t("beforeEnd"), bstrNewHTML);
			SysFreeString(bstrNewHTML);

			if (SUCCEEDED(hr)) {
				return TRUE;
			} else {
				return FALSE;
			}
		} else {
			if (!bStrict) {
				_ASSERTE(pParentElement != NULL);
				// (a.walling 2007-10-10 09:26) - PLID 25548 - go ahead and stick the item at the end of the preview rather than not show it at all.
				return InsertTopic(pTopic);
			}
		}
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::InsertSubTopic");

	return FALSE;
}

// (a.walling 2007-10-10 09:22) - PLID 25548 - added bStrict parameter for all InsertTopic functions to prevent
// adding at the end if the parent or sibling was not found.
BOOL CEMRPreviewCtrlDlg::InsertTopicBefore(CEMRTopic* pNextTopic, CEMRTopic* pTopic, BOOL bStrict /*= FALSE*/)
{
	try {
		if (!IsLoaded()) return FALSE;

		// (a.walling 2007-10-12 09:07) - PLID 25548 - Ensure this is for the correct EMN
		if (	(pNextTopic != NULL && pNextTopic->GetParentEMN() != GetCurrentEMN()) ||
				(pTopic != NULL && pTopic->GetParentEMN() != GetCurrentEMN())
			) {
			return FALSE;
		}

		if (pNextTopic) {
			IHTMLElementPtr pNextElement = GetElementByIDOrPointer("TopicDiv", pNextTopic->GetID(), reinterpret_cast<long>(pNextTopic));

			if (pNextElement) {
				// pNextElement is the element immediately after this one.
				// we will be inserting our new topic before pNextElement, but still within any parent topics.
				CString strNewHTML = pTopic->GetHTML();

				BSTR bstrNewHTML = strNewHTML.AllocSysString();

				HRESULT hr = pNextElement->insertAdjacentHTML(_bstr_t("beforeBegin"), bstrNewHTML);
				SysFreeString(bstrNewHTML);

				if (SUCCEEDED(hr)) {
					return TRUE;
				} else {
					return FALSE;
				}
			}
		} else {
			// pNextTopic is invalid, which basically means it is before NOTHING!
			return InsertTopic(pTopic);
		}

		// if we got here, we either have no pNextTopic, or pNextTopic was not found, so just add the topic at the end of the tree

		// (a.walling 2007-10-09 14:11) - PLID 25548 - this means that we were expecting to place this topic in order,
		// but we could not find the relative topic, so we should alert that the ordering may be incorrect.
		if (!bStrict) {
			_ASSERTE(pNextTopic == NULL);
			return InsertTopic(pTopic);
		}		
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::InsertTopicBefore");

	return FALSE;
}

// (a.walling 2007-10-10 09:22) - PLID 25548 - added bStrict parameter for all InsertTopic functions to prevent
// adding at the end if the parent or sibling was not found.
BOOL CEMRPreviewCtrlDlg::InsertSubTopicBefore(CEMRTopic* pParentTopic, CEMRTopic* pNextTopic, CEMRTopic* pTopic, BOOL bStrict /*= FALSE*/)
{
	try {
		if (!IsLoaded()) return FALSE;

		// (a.walling 2007-10-12 09:07) - PLID 25548 - Ensure this is for the correct EMN
		if (	(pNextTopic != NULL && pNextTopic->GetParentEMN() != GetCurrentEMN()) ||
				(pTopic != NULL && pTopic->GetParentEMN() != GetCurrentEMN()) ||
				(pParentTopic != NULL && pParentTopic->GetParentEMN() != GetCurrentEMN())
			) {
			return FALSE;
		}

		if (pNextTopic) {
			IHTMLElementPtr pNextElement = GetElementByIDOrPointer("TopicDiv", pNextTopic->GetID(), reinterpret_cast<long>(pNextTopic));

			if (pNextElement) {
				// pNextElement is the element immediately after this one.
				// we will be inserting our new topic before pNextElement, but still within any parent topics.
				CString strNewHTML = pTopic->GetHTML();

				BSTR bstrNewHTML = strNewHTML.AllocSysString();

				HRESULT hr = pNextElement->insertAdjacentHTML(_bstr_t("beforeBegin"), bstrNewHTML);
				SysFreeString(bstrNewHTML);

				if (SUCCEEDED(hr)) {
					return TRUE;
				} else {
					return FALSE;
				}
			}
		} else {
			// pNextTopic is invalid, just add at the end
			return InsertSubTopic(pParentTopic, pTopic, bStrict);
		}

		// (a.walling 2007-10-09 14:11) - PLID 25548 - this means that we were expecting to place this topic in order,
		// but we could not find the relative topic, so we should alert that the ordering may be incorrect.
		if (!bStrict) {
			_ASSERTE(pNextTopic == NULL); 
			// if we got here, we either have no pNextTopic, or pNextTopic was not found, so just add the subtopic at the end of the subtree.
			return InsertSubTopic(pParentTopic, pTopic);
		}
		
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::InsertSubTopicBefore");

	return FALSE;
}

BOOL CEMRPreviewCtrlDlg::MoveTopic(CEMRTopic* pTopic, CEMRTopic* pInsertBeforeTopic)
{
	try {
		if (!IsLoaded()) return FALSE;

		// (a.walling 2007-10-12 09:07) - PLID 25548 - Ensure this is for the correct EMN
		if (	(pTopic->GetParentEMN() != GetCurrentEMN()) ||
				(pInsertBeforeTopic != NULL && pInsertBeforeTopic->GetParentEMN() != GetCurrentEMN())
			) {
			return FALSE;
		}

		CString strWhere;

		IHTMLElement2Ptr pRelativeElement;
		if (pInsertBeforeTopic) {
			pRelativeElement = GetElementByIDOrPointer("TopicDiv", pInsertBeforeTopic->GetID(), reinterpret_cast<long>(pInsertBeforeTopic));
			strWhere = "beforeBegin"; // insert before the entire topic
		} else {
			// (a.walling 2007-08-24 10:45) - PLID 26640 - the end of our list of topics is before the more info section, if it exists.
			pRelativeElement = GetElementByID("moreinfo"); // get the more info
			if (pRelativeElement) {
				strWhere = "beforeBegin";
			} else {
				pRelativeElement = GetElementByID("EMN"); // get the root EMN element
				strWhere = "beforeEnd"; // insert at the end of the EMN, but within it.
			}

			if (pRelativeElement) {
				// we are good
			} else {
				ASSERT(FALSE);
			}
		}

		if (pRelativeElement) {
			IHTMLElementPtr pElement = GetElementByIDOrPointer("TopicDiv", pTopic->GetID(), reinterpret_cast<long>(pTopic));

			if (pElement) {
				HRESULT hr = pRelativeElement->insertAdjacentElement(_bstr_t((LPCTSTR)strWhere), pElement, NULL); // inserting an element that already exists on the page moves that element

				if (SUCCEEDED(hr))
					return TRUE;
				else
					return FALSE;
			}
		}
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::MoveTopic");

	return FALSE;
}

BOOL CEMRPreviewCtrlDlg::MoveTopicToSubTopic(CEMRTopic* pTopic, CEMRTopic* pSubTopic, CEMRTopic* pInsertBeforeTopic)
{
	try { 
		if (!IsLoaded()) return FALSE;

		// (a.walling 2007-10-12 09:07) - PLID 25548 - Ensure this is for the correct EMN
		if (	(pTopic->GetParentEMN() != GetCurrentEMN()) ||
				(pSubTopic != NULL && pSubTopic->GetParentEMN() != GetCurrentEMN()) ||
				(pInsertBeforeTopic != NULL && pInsertBeforeTopic->GetParentEMN() != GetCurrentEMN())
			) {
			return FALSE;
		}

		CString strWhere;

		IHTMLElement2Ptr pRelativeElement;
		if (pInsertBeforeTopic) {
			pRelativeElement = GetElementByIDOrPointer("TopicDiv", pInsertBeforeTopic->GetID(), reinterpret_cast<long>(pInsertBeforeTopic));
			strWhere = "beforeBegin"; // insert before the entire topic
		} else {
			if (pSubTopic) {
				pRelativeElement = GetElementByIDOrPointer("TopicDivTopics", pSubTopic->GetID(), reinterpret_cast<long>(pSubTopic));
				strWhere = "beforeEnd"; // and add to the end of the subtopic, but within the subtopic
			} else {
				ASSERT(FALSE);
			}
		}

		if (pRelativeElement) {
			IHTMLElementPtr pElement = GetElementByIDOrPointer("TopicDiv", pTopic->GetID(), reinterpret_cast<long>(pTopic));

			if (pElement) {
				IHTMLElement2Ptr pInsertedElement;

				HRESULT hr = pRelativeElement->insertAdjacentElement(_bstr_t((LPCTSTR)strWhere), pElement, NULL); // inserting an element that already exists on the page moves that element

				if (SUCCEEDED(hr))
					return TRUE;
				else
					return FALSE;
			}
		}
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::MoveTopicToSubTopic");

	return FALSE;
}

BOOL CEMRPreviewCtrlDlg::RemoveTopic(CEMRTopic* pTopic)
{
	try {
		if (!IsLoaded()) return FALSE;

		if (pTopic) {
			// (a.walling 2007-10-12 09:07) - PLID 25548 - Ensure this is for the correct EMN
			if (pTopic->GetParentEMN() != GetCurrentEMN()) {
				return FALSE;
			}

			IHTMLElementPtr pElement;
			
			// (a.walling 2007-04-11 10:35) - PLID 25549 - Ensure this topic, and any duplicates (which has never happened with topics), are gone
			do {
				pElement = GetElementByIDOrPointer("TopicDiv", pTopic->GetID(), reinterpret_cast<long>(pTopic));

				if (pElement) {
					HRESULT hr = pElement->put_outerHTML(_bstr_t(""));

					if (!SUCCEEDED(hr))
						return FALSE;
				}
			} while (pElement);

			return TRUE;
		}
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::RemoveTopic");
	return FALSE;
}

// (a.walling 2007-07-12 15:59) - PLID 26640 - Scroll to the more info topic
BOOL CEMRPreviewCtrlDlg::ScrollToMoreInfo()
{
	try {
		if (!IsLoaded()) {
			if (m_pCurrentInfo) {
				// (a.walling 2007-10-01 13:06) - PLID 25548 - If we are not loaded yet, save our scroll info
				// to be restored after we recieve the DocumentComplete event
				m_pCurrentInfo->strPendingScrollPT.Empty();
				m_pCurrentInfo->strPendingScrollID = "moreinfo";
			}	
			return FALSE;
		}

		ScrollToElementByID("moreinfo");

	}NxCatchAll("Error in CEMRPreviewCtrlDlg::ScrollToMoreInfo");

	return FALSE;
}

// (a.walling 2007-04-10 16:59) - PLID 25548 - Scroll the preview pane to this topic
// (a.walling 2007-04-11 10:34) - PLID 25548 - Different scrolling method is faster and more reliable
// (a.walling 2007-06-19 09:39) - PLID 25548 - Even better scrolling method! Rather than rely on anchors,
// now we simply find the element and get it's offsetTop property (which is the actual position from top,
// as compared to the top property which is the CSS value string (and which may return percentages and picas
// and all other measurements)), which is then passed to the window's ScrollTo function.
BOOL CEMRPreviewCtrlDlg::ScrollToTopic(CEMRTopic* pTopic)
{
	try {
		if (!IsLoaded()) {
			if (m_pCurrentInfo) {
				// (a.walling 2007-10-01 13:06) - PLID 25548 - If we are not loaded yet, save our scroll info
				// to be restored after we recieve the DocumentComplete event
				m_pCurrentInfo->strPendingScrollPT.Format("TopicAnchorPT%li", reinterpret_cast<long>(pTopic));
				if (pTopic && pTopic->GetID() != -1) {
					m_pCurrentInfo->strPendingScrollID.Format("TopicAnchorID%li", pTopic->GetID());
				}
			}
			return FALSE;	
		}

		if (pTopic) {
			CString strIDAnchor, strPtrAnchor;
			long nID = pTopic->GetID();
			strPtrAnchor.Format("TopicAnchorPT%li", reinterpret_cast<long>(pTopic));
			strIDAnchor.Format("TopicAnchorID%li", nID);

			// (a.walling 2007-10-01 13:18) - PLID 25548 - first try scrolling to the ID, then the pointer
			if (!ScrollToElementByID(strIDAnchor)) {
				return ScrollToElementByID(strPtrAnchor);
			} else {
				return TRUE;
			}
		}

		// (a.walling 2007-10-01 13:19) - PLID 25548 - Moved a lot of this into the ScrollToElementByID function

		/*
		if (pTopic) {
			HRESULT hr;

			// (b.cardillo 2007-09-19 12:06) - PLID 27439 - Changed this from a regular pointer variable to a smart-pointer 
			// object variable so it would automatically be released when it goes out of scope.  Prior to this change, the 
			// underlying COM object was being leaked.
			IDispatchPtr p;

			if (m_pBrowser == NULL)
				return NULL;

			hr = m_pBrowser->get_Document(&p);
			if (SUCCEEDED(hr)) {
				IHTMLDocument2Ptr pDoc;
				hr = p->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);

				if (SUCCEEDED(hr)) {
					CString strIDAnchor, strPtrAnchor;
					long nID = pTopic->GetID();
					strPtrAnchor.Format("TopicAnchorPT%li", reinterpret_cast<long>(pTopic));
					strIDAnchor.Format("TopicAnchorID%li", nID);


					IHTMLElementPtr pElement = GetElementByID(strIDAnchor);
					IHTMLElementPtr pPtElement = GetElementByID(strPtrAnchor);

					long nTop = -1;

					if (pElement)
						pElement->get_offsetTop(&nTop);
					else if (pPtElement)
						pPtElement->get_offsetTop(&nTop);

					if (nTop >= 0) {
						IHTMLWindow2Ptr pWindow = NULL;
						hr = pDoc->get_parentWindow(&pWindow);

						if (SUCCEEDED(hr) && pWindow != NULL) {
							// (a.walling 2007-08-08 15:38) - PLID 27017 - Adjust for the scroll margin constant
							nTop -= m_nScrollMargin;
							if (nTop < 0) nTop = 0;

							// Alternatively we can set the scrollTop DOM property of the BODY element, however
							// this is simpler (and slightly but imperceptibly faster)
							hr = pWindow->scrollTo(0, nTop);

							// (a.walling 2007-08-08 15:22) - PLID 27017 - Save our scroll pos
							if (m_pCurrentInfo) {
								m_pCurrentInfo->nScrollTop = GetScrollTop();
							}

							return SUCCEEDED(hr);
						}
					}

					/*IHTMLLocationPtr pLocation;

					hr = pDoc->get_location(&pLocation);

					if (SUCCEEDED(hr)) {
						// we have a valid location pointer now!
						CString strIDAnchor, strPtrAnchor;
						long nID = pTopic->GetID();
						strPtrAnchor.Format("TopicAnchorPT%li", reinterpret_cast<long>(pTopic));
						strIDAnchor.Format("TopicAnchorID%li", nID);

						IHTMLElementPtr pElement = GetElementByID(strIDAnchor);
						IHTMLElementPtr pPtElement = GetElementByID(strPtrAnchor);

						if (pElement)
							hr = pLocation->put_hash(_bstr_t(strIDAnchor));
						else if (pPtElement)
							hr = pLocation->put_hash(_bstr_t(strPtrAnchor));

						ASSERT(SUCCEEDED(hr));
						return SUCCEEDED(hr);
					}*//*
		
				}
			}
		}*/
	}NxCatchAll("Error in CEMRPreviewCtrlDlg::ScrollToTopic");

	return FALSE;
}

// (a.walling 2007-06-19 17:28) - PLID 25548 - Scroll to the top of the window (that is, the EMN title)
BOOL CEMRPreviewCtrlDlg::ScrollToTop()
{
	try {
		if (!IsLoaded()) {
			if (m_pCurrentInfo) {
				// (a.walling 2007-10-01 13:06) - PLID 25548 - If we are not loaded yet, save our scroll info
				// to be restored after we recieve the DocumentComplete event
				m_pCurrentInfo->strPendingScrollPT.Empty();
				m_pCurrentInfo->strPendingScrollID = "0";
			}				
			return FALSE;
		}

		return ScrollToElementByID("0");
		
		// (a.walling 2007-10-01 13:22) - PLID 25548 - Moved this to the ScrollToElementByID function

		/*
		HRESULT hr;

		// (b.cardillo 2007-09-19 12:06) - PLID 27439 - Changed this from a regular pointer variable to a smart-pointer 
		// object variable so it would automatically be released when it goes out of scope.  Prior to this change, the 
		// underlying COM object was being leaked.
		IDispatchPtr p;

		if (m_pBrowser == NULL)
			return NULL;

		hr = m_pBrowser->get_Document(&p);
		if (SUCCEEDED(hr)) {
			IHTMLDocument2Ptr pDoc;
			hr = p->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);

			if (SUCCEEDED(hr)) {
				IHTMLWindow2Ptr pWindow = NULL;
				hr = pDoc->get_parentWindow(&pWindow);

				if (SUCCEEDED(hr) && pWindow != NULL) {
					// Alternatively we can set the scrollTop DOM property of the BODY element, however
					// this is simpler (and slightly but imperceptibly faster)
					hr = pWindow->scrollTo(0, 0);

					// (a.walling 2007-08-08 15:22) - PLID 27017 - Save our scroll pos
					if (m_pCurrentInfo) {
						m_pCurrentInfo->nScrollTop = 0;
					}

					return SUCCEEDED(hr);
				}
			}
		}
		*/
	}NxCatchAll("Error in CEMRPreviewCtrlDlg::ScrollToTop");

	return FALSE;
}

// (a.walling 2007-10-01 13:13) - PLID 25548 - Scrolls the EMN to the pending scrolls within the m_pCurrentInfo structure 
BOOL CEMRPreviewCtrlDlg::ScrollPending()
{
	if (m_pCurrentInfo) {
		// first try ID, then PT.
		BOOL bSuccess = FALSE;
		if (m_pCurrentInfo->strPendingScrollID.GetLength() > 0) {
			bSuccess = ScrollToElementByID(m_pCurrentInfo->strPendingScrollID);
		}
		if (!bSuccess && m_pCurrentInfo->strPendingScrollPT.GetLength() > 0) {
			bSuccess = ScrollToElementByID(m_pCurrentInfo->strPendingScrollPT);
		}		
		
		// (a.walling 2012-11-19 17:37) - PLID 53834 - Clear out the pending scroll info, otherwise it will always scroll to this pending ID every time the document reloads!
		m_pCurrentInfo->strPendingScrollID.Empty();
		m_pCurrentInfo->strPendingScrollPT.Empty();

		return bSuccess;
	}

	return FALSE;
}

namespace {
	// (a.walling 2014-05-05 11:25) - PLID 62032 - AutoScroll will jump to the top when trying to go to any topic in a column
	// even just right clicking the topic name in the preview pane.
	// offsetY is used to calculate, but this is only relative to offsetParent
	// need to 'walk' the offset tree, basically.
	long CalcPageYOffset(IHTMLElementPtr pElem)
	{
		if (!pElem) {
			return -1;
		}

		IHTMLElementPtr pOffsetParent;

		long pageYOffset = 0;

		do {
			long nOffset = 0;
			pElem->get_offsetTop(&nOffset);

			pageYOffset += nOffset;

			pElem->get_offsetParent(&pOffsetParent);

			if (pOffsetParent == pElem) {
				pOffsetParent = nullptr;
			}
			pElem = pOffsetParent;
		} while (pElem);

		return pageYOffset;
	}
}

// (a.walling 2007-10-01 13:14) - PLID 25548 - Shared function scrolls to the element passed (throws errors to caller)
BOOL CEMRPreviewCtrlDlg::ScrollToElementByID(const CString &strID)
{
	// (a.walling 2007-10-01 13:25) - PLID 25548 - Special case -- if strID == "0" scroll to the very top.
	if (strID == "0") {
		return PutScrollTop(0);
	}

	HRESULT hr;

	// (b.cardillo 2007-09-19 12:06) - PLID 27439 - Changed this from a regular pointer variable to a smart-pointer 
	// object variable so it would automatically be released when it goes out of scope.  Prior to this change, the 
	// underlying COM object was being leaked.
	IDispatchPtr p;

	if (m_pBrowser == NULL)
		return NULL;

	hr = m_pBrowser->get_Document(&p);
	// (a.walling 2012-11-16 11:46) - PLID 53794 - Some functions may return S_FALSE which is success, but pointer is invalid / NULL
	if (SUCCEEDED(hr) && p) {
		IHTMLDocument2Ptr pDoc;
		hr = p->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);

		if (SUCCEEDED(hr)) {
			IHTMLWindow2Ptr pWindow = NULL;
			hr = pDoc->get_parentWindow(&pWindow);

			if (SUCCEEDED(hr) && pWindow != NULL) {
				IHTMLElementPtr pElement = GetElementByID(strID);

				// (a.walling 2014-05-05 11:25) - PLID 62032
				long nTop = CalcPageYOffset(pElement);

				if (nTop >= 0) {
					// (a.walling 2007-08-08 15:38) - PLID 27017 - Adjust for the scroll margin constant
					nTop -= m_nScrollMargin;
					if (nTop < 0) nTop = 0;

					// Alternatively we can set the scrollTop DOM property of the BODY element, however
					// this is simpler (and slightly but imperceptibly faster)
					hr = pWindow->scrollTo(0, nTop);

					// (a.walling 2007-08-08 15:22) - PLID 27017 - Save our scroll pos
					if (m_pCurrentInfo) {
						m_pCurrentInfo->nScrollTop = GetScrollTop();
					}

					return SUCCEEDED(hr);
				}
			}
		}
	}

	return FALSE;
}

BOOL CEMRPreviewCtrlDlg::ShowTopic(CEMRTopic* pTopic, BOOL bShow)
{
	try {
		if (!IsLoaded()) return FALSE;

		// (a.walling 2007-10-12 09:07) - PLID 25548 - Ensure this is for the correct EMN
		if (pTopic->GetParentEMN() != GetCurrentEMN()) {
			return FALSE;
		}

		// (a.walling 2007-12-21 12:58) - PLID 28436 - Cosmetic changes to EMR.CSS
		if (bShow) {
			EnsureTopicNotClass(pTopic, "hide");
		} else {
			EnsureTopicClass(pTopic, "hide");
		}
		
		// (a.walling 2008-02-05 17:09) - PLID 28391 - Much more robust and efficient method
		// of showing the topic hierarchy
		if (bShow) {
			CEMRTopic* pParentTopic = pTopic->GetParentTopic();
			while (pParentTopic != NULL) {
				if (bShow) {
					EnsureTopicNotClass(pParentTopic, "hide");
				} else {
					EnsureTopicClass(pParentTopic, "hide");
				}

				pParentTopic = pParentTopic->GetParentTopic();
			}
		}

	}NxCatchAll("Error in CEMRPreviewCtrlDlg::ShowTopic");

	return FALSE;
}

// (a.walling 2008-02-06 09:46) - Modifying inline styles is now deprecated; this is now handled via multiple CSS class inheritance
/*BOOL CEMRPreviewCtrlDlg::ShowElement(IHTMLElementPtr pElement, BOOL bShow)
{
	try {
		if (pElement) {
			IHTMLStylePtr pStyle;

			HRESULT hr = pElement->get_style(&pStyle);

			if (SUCCEEDED(hr) ) {
				CString strStyle;

				if (bShow) {
					strStyle = "block";
				} else {
					strStyle = "none";
				}

				_bstr_t bstrStyle(strStyle);
				
				hr = pStyle->put_display(bstrStyle);

				return SUCCEEDED(hr);
			}
		}
	}NxCatchAll("Error in CEMRPreviewCtrlDlg::ShowElement");

	return FALSE;
}
*/

/* Utility functions */


// throws errors to caller
IHTMLElementPtr CEMRPreviewCtrlDlg::GetElementByIDOrPointer(const CString &strType, long nID, long nPtr)
{
	BOOL bUsePointer = FALSE;

	if (nID == -1) {
		bUsePointer = TRUE;
	}

	CString strHTMLID;

	if (bUsePointer) {
		strHTMLID.Format("%sPT%li", strType, nPtr);
	} else {
		strHTMLID.Format("%sID%li", strType, nID);
	}

	IHTMLElementPtr pElement = GetElementByID(strHTMLID);
	if (pElement) {
		return pElement;
	} else {
		if (!bUsePointer) {
			// if we could not find the element, and we searched by ID, try searching by pointer.

			strHTMLID.Format("%sPT%li", strType, nPtr);
			pElement = GetElementByID(strHTMLID);
			if (pElement)
				return pElement;
		}
	}

	return NULL;
}

// throws errors to caller
IHTMLElementPtr CEMRPreviewCtrlDlg::GetElementByID(const CString &strID)
{
	COleVariant varDivID(strID, VT_BSTR);
	COleVariant varIndex((long)0, VT_I4);

	HRESULT hr;

	// (b.cardillo 2007-09-19 12:06) - PLID 27439 - Changed this from a regular pointer variable to a smart-pointer 
	// object variable so it would automatically be released when it goes out of scope.  Prior to this change, the 
	// underlying COM object was being leaked.
	IDispatchPtr p;

	if (m_pBrowser == NULL)
		return NULL;

	hr = m_pBrowser->get_Document(&p);
	// (a.walling 2012-11-16 11:46) - PLID 53794 - Some functions may return S_FALSE which is success, but pointer is invalid / NULL
	if (SUCCEEDED(hr) && p) {
		IHTMLDocument2Ptr pDoc;
		hr = p->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);

		if (SUCCEEDED(hr)) {
			IHTMLElementCollectionPtr pElemColl = NULL;
			hr = pDoc->get_all(&pElemColl);

			// (a.walling 2012-11-16 11:46) - PLID 53794 - Some functions may return S_FALSE which is success, but pointer is invalid / NULL
			if (SUCCEEDED(hr) && pElemColl) {
				// (b.cardillo 2007-09-19 12:06) - PLID 27439 - Changed this from a regular pointer variable to a smart-pointer 
				// object variable so it would automatically be released when it goes out of scope.  Prior to this change, the 
				// underlying COM object was being leaked.
				IDispatchPtr pDispItem = NULL;
				IHTMLElementPtr pElement = NULL;

				hr = pElemColl->item(varDivID, varIndex, &pDispItem);

				if (SUCCEEDED(hr)) {
					if (pDispItem) {
						hr = pDispItem->QueryInterface(IID_IHTMLElement, (void**)&pElement);

						if (SUCCEEDED(hr)) {
							if (pElement) {
								return pElement; // caller's smart pointer should add ref
							}
						} else {
							// it is possible that several items meet this criteria... rather than fail,
							// try to get the IHTMLElementCollection and return the first item (asserting too).
							IHTMLElementCollectionPtr pElementCollection;

							hr = pDispItem->QueryInterface(IID_IHTMLElementCollection, (void**)&pElementCollection);

							if (SUCCEEDED(hr) && (pElementCollection != NULL)) {
								// yep, returned multiple items, so let's get the first one.
								long nLength;

								hr = pElementCollection->get_length(&nLength);

								if (SUCCEEDED(hr)) {
									// (b.cardillo 2007-09-19 12:06) - PLID 27439 - Changed this from a regular pointer variable to a smart-pointer 
									// object variable so it would automatically be released when it goes out of scope.  Prior to this change, the 
									// underlying COM object was being leaked.
									IDispatchPtr pDispSingleItem;

									_variant_t varIndex = (long)0;

									hr = pElementCollection->item(varIndex, varIndex, &pDispSingleItem);

									if (SUCCEEDED(hr) && (pDispSingleItem != NULL)) {
										// alright, try to get the IHTMLElement interface again
										hr = pDispSingleItem->QueryInterface(IID_IHTMLElement, (void**)&pElement);

										if (SUCCEEDED(hr) && (pElement != NULL)) {
#ifdef _DEBUG
											LogDetail("WARNING: GetElementByID matched %li items, only first item returned.", nLength);
#endif										
											ASSERT(FALSE);
											return pElement;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return NULL; // element was not found, or some other error.
}

// (a.walling 2007-04-12 09:34) - Returns the current calculated HTML of the element, using the body if no element sent.
// Used mostly for debugging to check the results of dynamic DOM modification
CString CEMRPreviewCtrlDlg::GetCurrentHTML(IHTMLElementPtr pElement /* = NULL*/)
{
	if (pElement == NULL) {
		if (m_pBrowser == NULL)
			return "";

		// (b.cardillo 2007-09-19 12:06) - PLID 27439 - Changed this from a regular pointer variable to a smart-pointer 
		// object variable so it would automatically be released when it goes out of scope.  Prior to this change, the 
		// underlying COM object was being leaked.
		IDispatchPtr p;
		HRESULT hr = m_pBrowser->get_Document(&p);
		// (a.walling 2012-11-16 11:46) - PLID 53794 - Some functions may return S_FALSE which is success, but pointer is invalid / NULL
		if (SUCCEEDED(hr) && p) {
			IHTMLDocument2Ptr pDoc;
			
			hr = p->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);

			if (SUCCEEDED(hr)) {
				hr = pDoc->get_body(&pElement);

				if (!SUCCEEDED(hr) || (pElement == NULL)) {
					return "";
				}
			}
		}
	}

	if (pElement == NULL)
		return "";

	// if we are here, the pElement is valid
	BSTR bstr;
	HRESULT hr = pElement->get_innerHTML(&bstr);

	if (SUCCEEDED(hr)) {
		CString str;
		// (a.walling 2007-07-09 09:28) - Ensure the BSTR is freed by attaching it to the _bstr_t object
		_bstr_t bstrWrapper(bstr, false);
		str = (LPCTSTR)bstrWrapper;

		return str;
	} else {
		return "";
	}
}

void CEMRPreviewCtrlDlg::OnDocumentLoaded(LPDISPATCH pDisp, VARIANT FAR* URL) 
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		__super::OnDocumentLoaded(pDisp, URL);

		LRESULT nNotifyParentResult = 0;
		// (a.walling 2009-11-23 11:44) - PLID 24194 - Alert the parent when a document has been completely loaded
		if (GetParent()) {
			// (a.walling 2010-01-12 08:38) - PLID 36840 - Changed to defined message with EMRPREVIEW_ rather than a registered message
			nNotifyParentResult = GetParent()->SendMessage(NXM_EMRPREVIEW_DOCUMENT_COMPLETE, (WPARAM)pDisp, (LPARAM)URL);
		}

		_bstr_t bstrUrl;

		if (URL && VT_BSTR == URL->vt) {
			bstrUrl = URL->bstrVal;
		}

		if (0 == wcscmp(bstrUrl, L"about:blank")) {
			return;
		}

		// (a.walling 2007-08-08 15:23) - PLID 27017 - Restore our saved scroll position
		if (m_pCurrentInfo) {
			m_nPendingScrollTop = 0;

			// (a.walling 2007-10-01 13:11) - PLID 25548 - Scroll only if there are no pending scroll saved
			if (m_pCurrentInfo->strPendingScrollID.IsEmpty() && m_pCurrentInfo->strPendingScrollPT.IsEmpty() ) {
				PutScrollTop(m_pCurrentInfo->nScrollTop);
			} else {
				// well this means we have some pending scrolls. So do so!
				if (!ScrollPending()) {
					// we could not execute that scroll, so at least try to restore our last position.
					PutScrollTop(m_pCurrentInfo->nScrollTop);
				}
			}

			// (a.walling 2007-10-12 11:04) - PLID 27017 - Update any pending details now that we are complete
			for (int i = 0; i < m_pCurrentInfo->arDetailsPendingUpdate.GetSize(); i++) {
				UpdateDetail(m_pCurrentInfo->arDetailsPendingUpdate[i]);
				// (a.walling 2007-12-17 17:43) - PLID 28391
				if (m_pCurrentInfo->arDetailsPendingUpdate[i]->m_pParentTopic != NULL) {
					m_pCurrentInfo->arDetailsPendingUpdate[i]->m_pParentTopic->RefreshHTMLVisibility();
				}
				//m_pCurrentInfo->arDetailsPendingUpdate[i]->Release();
				// (a.walling 2009-10-12 16:05) - PLID 36024
				m_pCurrentInfo->arDetailsPendingUpdate[i]->__Release("CEMRPreviewCtrlDlg::OnDocumentCompleteEmrPreviewBrowser pending details");
			}
			m_pCurrentInfo->arDetailsPendingUpdate.RemoveAll();
		}
		else {
			// (a.walling 2015-11-16 11:52) - PLID 67494 - Restore the scroll position if one was set

			long nScrollTop = m_nPendingScrollTop;
			m_nPendingScrollTop = 0;

			if (nScrollTop) {
				PutScrollTop(nScrollTop);
			}
		}

		// (a.walling 2011-10-18 17:10) - PLID 45975 - Inject the drag-scrolling javascript into the document
		// this must be injected if we want to be able to interact with pre-existing EMN previews

		// basically, a double click will drop you into normal / text-selection mode, as will clicking and holding
		// the mouse button with no(minimal) movement for half a second. Otherwise it does the drag scrolling, with
		// the decay at the end.
		try {
			IHTMLDocument2Ptr pDoc;
			IHTMLDocument3Ptr pDoc3;

			{
				IDispatchPtr p;
				m_pBrowser->get_Document(&p);
				pDoc = p;
				pDoc3 = p;
			}

			if (!pDoc || !pDoc3) return;
			
			IHTMLElementCollectionPtr pHeadColl;
			HR(pDoc3->getElementsByTagName(_bstr_t("head"), &pHeadColl));

			if (!pHeadColl) return;

			IHTMLElementPtr pHead;
			{
				IDispatchPtr pDispItem;

				HR(pHeadColl->item(_variant_t((long)0), _variant_t((long)0), &pDispItem));
				pHead = pDispItem;
			}

			if (!pHead) return;

			IHTMLElementPtr pScriptElement;
			HR(pDoc->createElement(_bstr_t("script"), &pScriptElement));
			IHTMLScriptElementPtr pScript = pScriptElement;
			pScript->put_type(_bstr_t("text/javascript"));
			static _bstr_t dragScript(
"var mousedownElement = null; \r\n"
"var mouseupElement = null; \r\n"
"var mouseclickIgnore = false; \r\n"
"var mousedownTicks = 0; \r\n"
"var mousedownX = 0; \r\n"
"var mousedownY = 0; \r\n"
"var mousemoveX = 0; \r\n"
"var mousemoveY = 0; \r\n"
"var mousedownTimeout = null; \r\n"
"var curOffsetX = 0; \r\n"
"var curOffsetY = 0; \r\n"
""
"function endMousedownTimeout() { \r\n"
	"if (mousedownTimeout) { \r\n"
		"window.clearTimeout(mousedownTimeout); \r\n"
		"mousedownTimeout = null; \r\n"
	"} \r\n"
"} \r\n"
""
"var dragScrollDecayStep = 0; \r\n"
"var dragScrollDecayX = 0; \r\n"
"var dragScrollDecayY = 0; \r\n"
"var dragScrollHistory = new Array(); \r\n"
"var dragScrollDecayInterval = null; \r\n"
"var dragScrollRememberInterval = null; \r\n"
""
"function rememberScroll() { \r\n"
	"if (dragScrollHistory.length > 5) { \r\n"
		"dragScrollHistory.shift(); \r\n"
	"} \r\n"
	""
	"dragScrollHistory.push({ x: curOffsetX, y: curOffsetY }); \r\n"
	""
	"curOffsetX = 0; \r\n"
	"curOffsetY = 0; \r\n"
"} \r\n"
""
"function endRememberScroll() \r\n"
"{ \r\n"
	"if (dragScrollRememberInterval) { \r\n"
		"window.clearInterval(dragScrollRememberInterval); \r\n"
		"dragScrollRememberInterval = null; \r\n"
	"} \r\n"
"} \r\n"
""
"function beginRememberScroll() \r\n"
"{ \r\n"
	"endRememberScroll(); \r\n"
	""
	"curOffsetX = 0; \r\n"
	"curOffsetY = 0; \r\n"
	""
	"dragScrollRememberInterval = window.setInterval(rememberScroll, 20); \r\n"
"} \r\n"
""
"function calcDragScrollDecay() { \r\n"
	"dragScrollDecayStep = 0; \r\n"
	""
	"dragScrollDecayX = 0; \r\n"
	"dragScrollDecayY = 0; \r\n"
""
	"if (dragScrollHistory.length) { \r\n"
		"for (var i = 0; i < dragScrollHistory.length; i++) { \r\n"
			"dragScrollDecayX += dragScrollHistory[i].x; \r\n"
			"dragScrollDecayY += dragScrollHistory[i].y; \r\n"
		"} \r\n"
		"dragScrollDecayX /= dragScrollHistory.length; \r\n"
		"dragScrollDecayY /= dragScrollHistory.length; \r\n"
		""
		"dragScrollHistory = new Array(); \r\n"
	"} \r\n"
"} \r\n"
""
"function endDragScrollDecay() { \r\n"
	"dragScrollDecayX = 0; \r\n"
	"dragScrollDecayY = 0; \r\n"
""
	"if (dragScrollDecayInterval) { \r\n"
		"window.clearInterval(dragScrollDecayInterval); \r\n"
		"dragScrollDecayInterval = null; \r\n"
	"} \r\n"
"} \r\n"
""
"function doDragScrollDecay() { \r\n"
	"dragScrollDecayStep++; \r\n"
	"var dragScrollDecaySpeed; \r\n"
	""
	"if (dragScrollDecayStep > 20) { \r\n"
		"dragScrollDecaySpeed = 0; \r\n"
	"} else { \r\n"
		"dragScrollDecaySpeed = (Math.cos((0.05 * dragScrollDecayStep) * Math.PI) + 1) / 2; \r\n"
	"} \r\n"
	""
	"var dragOffsetX = dragScrollDecayX * dragScrollDecaySpeed; \r\n"
	"var dragOffsetY = dragScrollDecayY * dragScrollDecaySpeed; \r\n"
	""
	"if ( (Math.abs(dragOffsetX) > 0) || (Math.abs(dragOffsetY) > 0) ) { \r\n"
		"window.scrollBy(dragOffsetX, dragOffsetY); \r\n"
	"} else { \r\n"
		"endDragScrollDecay(); \r\n"
	"} \r\n"
"} \r\n"
""
"function beginDragScrollDecay() { \r\n"
	"calcDragScrollDecay(); \r\n"
	""
	"if ( (Math.abs(dragScrollDecayX) > 4) || (Math.abs(dragScrollDecayY) > 4) ) { \r\n"
		"dragScrollDecayInterval = window.setInterval(doDragScrollDecay, 20); \r\n"
	"} else { \r\n"
		"endDragScrollDecay(); \r\n"
	"} \r\n"
"} \r\n"
""
"function beginDragScroll() { \r\n"
	"endMousedownTimeout(); \r\n"
	"beginRememberScroll(); \r\n"
	"document.onselectstart = function() { return false; } \r\n"
	"document.onmousemove = doDragScroll; \r\n"
"} \r\n"
""
"function endDragScroll() { \r\n"
	"endMousedownTimeout(); \r\n"
	"endRememberScroll(); \r\n"
	"document.onselectstart = null; \r\n"
	"document.onmousemove = null; \r\n"
	""
	"beginDragScrollDecay(); \r\n"
"} \r\n"
""
"function doDragScroll() { \r\n"
	"mouseclickIgnore = false; \r\n"
	"endDragScrollDecay(); \r\n"
	""
	"mousedownTicks = 0; \r\n"
	"mousedownX = 0; \r\n"
	"mousedownY = 0; \r\n"
	""
	"if (window.event.shiftKey) { \r\n"
		"endDragScroll(); \r\n"
	"} else if (window.event.button) { \r\n"
		"var newMousemoveX = window.event.clientX + document.body.scrollLeft; \r\n"
		"var newMousemoveY = window.event.clientY + document.body.scrollTop; \r\n"
		"var offsetX = mousemoveX - newMousemoveX; \r\n"
		"var offsetY = mousemoveY - newMousemoveY; \r\n"
		""
		"window.scrollBy(offsetX, offsetY); \r\n"
		""
		"curOffsetX += offsetX; \r\n"
		"curOffsetY += offsetY; \r\n"
		""
		"mousemoveX = newMousemoveX; \r\n"
		"mousemoveY = newMousemoveY; \r\n"
	"} else { \r\n"
		"endDragScroll(); \r\n"
	"} \r\n"
"} \r\n"
""
"function checkDragScroll() { \r\n"
	"var newMousemoveX = window.event.clientX + document.body.scrollLeft; \r\n"
	"var newMousemoveY = window.event.clientY + document.body.scrollTop; \r\n"
	"var offsetX = mousemoveX - newMousemoveX; \r\n"
	"var offsetY = mousemoveY - newMousemoveY; \r\n"
	""
	"if ( (Math.abs(offsetX) > 16) || (Math.abs(offsetY) > 16) ) { \r\n"
		"beginDragScroll(); \r\n"
	"} \r\n"
"} \r\n"
""
"function handleMouseup() { \r\n"
	"mouseupElement = window.event.srcElement; \r\n"
	""
	"if (dragScrollRememberInterval) { \r\n"
		"endDragScroll(); \r\n"
		"window.event.cancelBubble = true; \r\n"
		"window.event.returnValue = false; \r\n"
		""
		"if (mouseupElement && mouseupElement == mousedownElement) { \r\n"
			"mouseclickIgnore = true; \r\n"
		"} \r\n"
	"} else { \r\n"
		"endDragScroll(); \r\n"
	"} \r\n"
"} \r\n"
""
"function handleClick() { \r\n"
	"if (mouseclickIgnore) { \r\n"
		"mouseclickIgnore = false; \r\n"
		""
		"window.event.cancelBubble = true; \r\n"
		"window.event.returnValue = false; \r\n"
	"} \r\n"
"} \r\n"
""
"function handleMousedown() { \r\n"
	"endDragScroll(); \r\n"
	"endDragScrollDecay(); \r\n"
	""
	"mousedownElement = window.event.srcElement; \r\n"
	 ""
	"if (window.event.shiftKey) { \r\n"
		"return; \r\n"
	"} \r\n"
	""
	"var newMousedownX = window.event.clientX + document.body.scrollLeft; \r\n"
	"var newMousedownY = window.event.clientY + document.body.scrollLeft; \r\n"
	""
	"var curTime = new Date(); \r\n"
	"var curTicks = curTime.getTime(); \r\n"
	""
	"if ( ((mousedownTicks - curTicks) < 250) && (Math.abs(newMousedownX - mousedownX) <= 16) && (Math.abs(newMousedownY - mousedownY) <= 16) ) { \r\n"
		"endDragScroll(); \r\n"
	"} else { \r\n"
		"mousemoveX = newMousedownX; \r\n"
		"mousemoveY = newMousedownY; \r\n"
		 ""
		"document.onselectstart = function() { return false; } \r\n"
		"document.onmousemove = checkDragScroll; \r\n"
		"mousedownTimeout = window.setTimeout(endDragScroll, 500); \r\n"
		""
		"window.event.cancelBubble = true; \r\n"
		"window.event.returnValue = false; \r\n"
	"} \r\n"
	""
	"mousedownTicks = curTicks; \r\n"
	"mousedownX = newMousedownX; \r\n"
	"mousedownY = newMousedownY; \r\n"
"} \r\n"
""
"if (document.addEventListener) { \r\n"
	"document.addEventListener('mouseup', handleMouseup, true); \r\n"
	"document.addEventListener('mousedown', handleMousedown, true); \r\n"
	"document.addEventListener('click', handleClick, true); \r\n"
"} else { \r\n"
	"document.onmouseup = handleMouseup; \r\n"
	"document.onmousedown = handleMousedown; \r\n"
	"document.onclick = handleClick; \r\n"
"} \r\n"
""
"document.ondblclick = endDragScroll; "
			);
			pScript->put_text(dragScript);

			IHTMLDOMNodePtr pAppendedScript;
			IHTMLDOMNodePtr pHeadNode = pHead;
			IHTMLDOMNodePtr pScriptNode = pScript;

			ASSERT(pHeadNode);
			ASSERT(pScriptNode);
			// (a.walling 2012-03-15 18:34) - PLID 48712 - injecting the script seems to be causing a leak; I removed the output val for the appended node
			// similar to how MSDN examples do it, but not sure if it has any effect atm. Might be due to the javascript itself.
			HR(pHeadNode->appendChild(pScriptNode, NULL));
		} NxCatchAllIgnore();

	} NxCatchAll("Error in OnDocumentCompleteEmrPreviewBrowser");
}

// ensure the CSS file exists and also in the temp folder, copies the default from resources if not found
void CEMRPreviewCtrlDlg::EnsureCSSFile()
{
	try {
		// (a.walling 2007-12-27 16:40) - PLID 28632 - Overview of this function:
		//	1.	Ensure the EMNPreview path on the server
		//	2.	Ensure the emr.css in the temp path is up to date
		//	3.	Ensure the user.css file exists on the server, and create if not.
		//	4.	Copy the user.css from the server into the temp path.

		if (g_bEnsureCSSFileOK)
			return;

		// (a.walling 2007-06-11 15:40) - PLID 26278 - Move these files into the shared path\EMNPreview instead of Documents
		// (a.walling 2008-05-27 17:42) - PLID 30174 - CreatePath could be very slow if it tries to do a filefind on \\server\pracstation
		if (!DoesExist(GetSharedPath() ^ "EMNPreview")) {
			CreateDirectory(GetSharedPath() ^ "EMNPreview", NULL);
		}

		// (a.walling 2007-12-27 16:32) - PLID 28632 - This is all no longer necessary, apparently, due to my changes to include the
		// CSS inside the MHTML archive. The only necessity is ensuring a local copy of user.css, which is currently not being used
		// by any clients, so we don't need to bother them with an error if we can't ensure it. Regardless, the user.css file should
		// only be changed rarely, so I am using a global variable to only ensure once per session. Copying emr.css from resources
		// to the temp path will rarely fail. If it does, then merging to word and etc will be failing too.

		
		// (a.walling 2007-07-19 09:51) - PLID 26261 - Use the NxTemp path
		CString strTempPath = GetNxTempPath();

		CString strDefaultPath = strTempPath ^ "emr.css";
		CString strUserPath = GetSharedPath() ^ "EMNPreview" ^ "user.css";

		// we'll go ahead and copy over anyway to ensure we always have the most up-to-date version.
		// create the default CSS file.
		HRSRC rcCSSResource = FindResource(NULL, MAKEINTRESOURCE(IDR_EMR_CSS), "CSS");
		if (rcCSSResource) {
			HGLOBAL hgResource = LoadResource(NULL, rcCSSResource); // not really an HGLOBAL, wierd.

			if (hgResource) {
				DWORD dwSize = SizeofResource(NULL, rcCSSResource);

				if (dwSize > 0) {
					LPVOID pData = LockResource(hgResource);

					if (pData) {
						CFile fCSSFile;
						// (a.walling 2007-12-27 16:34) - PLID 28632 - Just ensure the local emr.css is up to date.
						if (fCSSFile.Open(strDefaultPath, CFile::modeCreate | CFile::modeReadWrite | CFile::shareCompat)) {
							fCSSFile.Write(pData, dwSize);
							fCSSFile.Close();

							FreeResource(hgResource);
						} else {
							CString strError = FormatLastError();

							// (a.walling 2007-12-27 18:25) - PLID 28632 - This is the only critical part of this function. Throw if we don't have a local emr.css
							FreeResource(hgResource);

							ThrowNxException("Could not ensure local emr.css at '%s': %s",
								strDefaultPath, strError);	
						}
					}
				}
			}
		}

		if (!FileUtils::DoesFileOrStreamExist(strUserPath)) {
			// the user.css file does not exist, so create.
			CFile fCSSUserFile;
			if (fCSSUserFile.Open(strUserPath, CFile::modeCreate | CFile::modeReadWrite | CFile::shareCompat)) {
				char* szUserCSS = "/* user.css - place optional user-defined CSS styles for the EMR output in this file. You may want to use the !important modifier. */";
				fCSSUserFile.Write(szUserCSS, strlen(szUserCSS));
				fCSSUserFile.Close();
			} else {
				// failed to open/create the file
				// (a.walling 2007-12-27 18:22) - PLID 28632 - This is non-critical, so just log.
				LogDetail("WARNING: Could not create initial user.css at '%s': %s",
					strUserPath, FormatLastError());
			}
		}

		// (a.walling 2007-12-27 16:35) - PLID 28632 - No longer needed; we just copy the user.css now.
		// CopyFile(strDefaultPath, strTempPath ^ "emr.css", FALSE);
		
		// copy the default CSS file
		if (!CopyFile(strUserPath, strTempPath ^ "user.css", FALSE)) {
			// (a.walling 2007-12-27 18:24) - PLID 28632 - Non-critical, just log.
			// Copy file failed.
			LogDetail("WARNING: Could not ensure shared user.css at '%s': %s",
					strUserPath, FormatLastError());
		}

		g_bEnsureCSSFileOK = true;
	} NxCatchAll_NoParent("Error in EnsureCSSFile"); // (a.walling 2014-05-05 13:32) - PLID 61945
}

// (j.jones 2007-07-06 11:01) - PLID 25457 - created NavigateToTopic
void CEMRPreviewCtrlDlg::NavigateToTopic(CString strParam, CString strCommand)
{
	if (strCommand == "topicID" || strCommand == "topicDTID") {

		// topic by ID
		CEMRTopic* pTopic = NULL;

		try {

			if(strCommand == "topicID") {
				//we were given a topic ID
				long nTopicID = atol(strParam);

				if (nTopicID >= 0) {
					if (m_pCurrentInfo->pEMN) {								
						pTopic = m_pCurrentInfo->pEMN->GetTopicByID(nTopicID);
					}
				}
			}
			else if(strCommand == "topicDTID") {
				//we were given a detail ID
				long nDetailID = atol(strParam);

				if (nDetailID >= 0) {
					if (m_pCurrentInfo->pEMN) {
						CEMNDetail *pDetail = m_pCurrentInfo->pEMN->GetDetailByID(nDetailID);
						if(pDetail) {
							pTopic = pDetail->m_pParentTopic;
						}
					}
				}
			}
		
			CEmrTreeWnd* pTreeWnd = m_pTreeWnd;

			if (pTopic) {
				if (pTreeWnd) {
					// (c.haag 2007-09-28 11:45) - PLID 27509 - Use IsTreeNull
					if (pTreeWnd->IsTreeNull()) {
						ASSERT(FALSE);
						ThrowNxException("Tried to navigate to topic ID with an invalid tree");
						return;
					}
					// first, get the row object for this topic
					// hooray! this appropriately fires Changing and Changed events, apparently!
					NXDATALIST2Lib::IRowSettingsPtr pRow = pTreeWnd->FindInTreeByColumn(TREE_COLUMN_OBJ_PTR, _variant_t(reinterpret_cast<long>(pTopic), VT_I4), NULL);
					//pTreeWnd->SetTreeSel(pRow);
					if (pRow == NULL) {
						// row was not found, but it may just not be found because it is a subtopic that has
						// not been added to the list yet. So we'll go up one level until we find a parent topic
						// that has a row, then ensure that row has loaded all subtopics.

						NXDATALIST2Lib::IRowSettingsPtr pHigherRow = NULL;

						CEMRTopic* pParentTopic = pTopic->GetParentTopic();
						// this loops until pHigherRow is not null, meaning we found the row in the datalist, or until we run out of rows
						while ( (pHigherRow == NULL) && (pParentTopic) ) {
							pHigherRow = pTreeWnd->FindInTreeByColumn(TREE_COLUMN_OBJ_PTR, _variant_t(reinterpret_cast<long>(pParentTopic), VT_I4), NULL); // don't autoselect

							if (pHigherRow) {
								pTreeWnd->EnsureTopicRowLoadedAllSubTopics(pHigherRow);
							}

							// (a.walling 2007-05-18 17:52) - PLID 25548 - Need to walk up the tree!
							pParentTopic = pParentTopic->GetParentTopic();
						}

						if (pHigherRow != NULL) {
							// this means we ensured the row was loaded, and all its subtopics, so try finding the topic again
							pRow = pTreeWnd->FindInTreeByColumn(TREE_COLUMN_OBJ_PTR, _variant_t(reinterpret_cast<long>(pTopic), VT_I4), NULL);
							//pTreeWnd->SetTreeSel(pRow);
						}
					}

					// (j.jones 2007-07-06 11:50) - PLID 25457 - ensure the row is created
					// (a.walling 2007-10-10 10:27) - PLID 25548 - Only set the sel if we have a valid row
					if(pRow) {
						//pTreeWnd->EnsureTopicRow(pRow);
						pTreeWnd->EnsureTopicView(pRow);
						pTreeWnd->SetTreeSel(pRow);
					}
				}
			} 
		} NxCatchAllThrow("Error navigating to topic ID");

	} else if (strCommand == "topicPT" || strCommand == "topicDTPT") {

		// topic by pointer
		CEMRTopic* pTopic = NULL;

		try {

			if(strCommand == "topicPT") {
				//we were given a topic pointer
				long nTopicPtr = atol(strParam);								
				if (m_pCurrentInfo->pEMN) {						
					pTopic = m_pCurrentInfo->pEMN->GetTopicByPointer(nTopicPtr);
				}
			}
			if(strCommand == "topicDTPT") {
				//we were given a detail pointer
				long nDetailPtr = atol(strParam);								
				if (m_pCurrentInfo->pEMN) {
					CEMNDetail *pDetail = m_pCurrentInfo->pEMN->GetDetailByPointer(nDetailPtr);
					if(pDetail) {
						pTopic = pDetail->m_pParentTopic;
					}
				}
			}

			CEmrTreeWnd* pTreeWnd = m_pTreeWnd;

			if (pTopic) {
				if (pTreeWnd) {
					// (c.haag 2007-09-28 11:47) - PLID 27509 - Use IsTreeNull
					if (pTreeWnd->IsTreeNull()) {
						ASSERT(FALSE);
						ThrowNxException("Tried to navigate to topic PT with an invalid tree");
						return;
					}
					// first, get the row object for this topic
					// hooray! this appropriately fires Changing and Changed events, apparently!
					NXDATALIST2Lib::IRowSettingsPtr pRow = pTreeWnd->FindInTreeByColumn(TREE_COLUMN_OBJ_PTR, _variant_t(reinterpret_cast<long>(pTopic), VT_I4), NULL);
					if (NULL != pRow) { 
						pTreeWnd->EnsureTopicView(pRow);
						pTreeWnd->SetTreeSel(pRow); 
					}
					if (pRow == NULL) {
						// row was not found, but it may just not be found because it is a subtopic that has
						// not been added to the list yet. So we'll go up one level until we find a parent topic
						// that has a row, then ensure that row has loaded all subtopics.

						NXDATALIST2Lib::IRowSettingsPtr pHigherRow = NULL;

						CEMRTopic* pParentTopic = pTopic->GetParentTopic();
						// this loops until pHigherRow is not null, meaning we found the row in the datalist, or until we run out of rows
						while ( (pHigherRow == NULL) && (pParentTopic) ) {
							pHigherRow = pTreeWnd->FindInTreeByColumn(TREE_COLUMN_OBJ_PTR, _variant_t(reinterpret_cast<long>(pParentTopic), VT_I4), NULL); // don't autoselect

							if (pHigherRow) {
								pTreeWnd->EnsureTopicRowLoadedAllSubTopics(pHigherRow);
							}

							// (a.walling 2007-05-18 17:52) - PLID 25548 - Need to walk up the tree!
							pParentTopic = pParentTopic->GetParentTopic();
						}

						if (pHigherRow != NULL) {
							// this means we ensured the row was loaded, and all its subtopics, so try finding the topic again
							pRow = pTreeWnd->FindInTreeByColumn(TREE_COLUMN_OBJ_PTR, _variant_t(reinterpret_cast<long>(pTopic), VT_I4), NULL);
							if (NULL != pRow) { 
								pTreeWnd->EnsureTopicView(pRow);
								pTreeWnd->SetTreeSel(pRow); 
							}
						}
					}
				}
			}
		} NxCatchAllThrow("Error navigating to topic pointer");
	}
}

// (a.walling 2007-08-08 15:19) - PLID 27017 - Return the current scroll position (top of visible content - top of document)
long CEMRPreviewCtrlDlg::GetScrollTop()
{
	try {
		HRESULT hr;

		// (b.cardillo 2007-09-19 12:06) - PLID 27439 - Changed this from a regular pointer variable to a smart-pointer 
		// object variable so it would automatically be released when it goes out of scope.  Prior to this change, the 
		// underlying COM object was being leaked.

		if (m_pBrowser == NULL)
			return 0;

		// (a.walling 2008-05-28 10:51) - PLID 29377 - I was hoping that web browser
		// inconsistencies would not bother me by doing all this in code. Unfortunately
		// that is incorrect. Some versions of IE need to use the body element for getting/
		// putting the scroll position. However others need to use the HTML element.
		// Thankfully the other option will be ignored, so by setting the scrolltop
		// twice, once for each element, and getting the scrolltop twice and adding them
		// (one will be zero) we achieve our desired result.

		long nBodyTop = 0;
		long nDocumentTop = 0;
	
		{
			IDispatchPtr p;
			hr = m_pBrowser->get_Document(&p);
			if (SUCCEEDED(hr) && p) {
				IHTMLDocument2Ptr pDoc;
				hr = p->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);

				if (SUCCEEDED(hr)) {	
					// We have our document, now get the body
					IHTMLElementPtr pElement;
					if (SUCCEEDED(pDoc->get_body(&pElement)) && pElement) {
						IHTMLElement2Ptr pBody(pElement);

						if (pBody) {
							if (!SUCCEEDED(pBody->get_scrollTop(&nBodyTop)))
								nBodyTop = 0;
						}
					}
				}
			}
		}

		{
			IDispatchPtr p;
			hr = m_pBrowser->get_Document(&p);
			if (SUCCEEDED(hr) && p) {
				IHTMLDocument3Ptr pDoc;
				hr = p->QueryInterface(IID_IHTMLDocument3, (void**)&pDoc);

				if (SUCCEEDED(hr)) {	
					// We have our document, now get the body
					IHTMLElementPtr pElement;
					if (SUCCEEDED(pDoc->get_documentElement(&pElement))) {
						IHTMLElement2Ptr pDocumentElement(pElement);

						if (pDocumentElement) {
							if (!SUCCEEDED(pDocumentElement->get_scrollTop(&nDocumentTop)))
								nDocumentTop = 0;
						}
					}
				}
			}
		}

		return nDocumentTop + nBodyTop;
	} NxCatchAll("Error getting scroll position");

	return 0;
}

// (a.walling 2007-08-08 15:19) - PLID 27017 - Set the current scroll position
BOOL CEMRPreviewCtrlDlg::PutScrollTop(long nTop)
{
	try {
		HRESULT hr;

		if (m_pBrowser == NULL)
			return FALSE;

		// (a.walling 2007-10-01 13:21) - PLID 25548 - Update our stored info
		if (m_pCurrentInfo) {
			m_pCurrentInfo->nScrollTop = nTop;
		}

		// (a.walling 2008-05-28 10:51) - PLID 29377 - I was hoping that web browser
		// inconsistencies would not bother me by doing all this in code. Unfortunately
		// that is incorrect. Some versions of IE need to use the body element for getting/
		// putting the scroll position. However others need to use the HTML element.
		// Thankfully the other option will be ignored, so by setting the scrolltop
		// twice, once for each element, and getting the scrolltop twice and adding them
		// (one will be zero) we achieve our desired result.
		BOOL bBody = FALSE, bDocument = FALSE;

		{
			// (b.cardillo 2007-09-19 12:06) - PLID 27439 - Changed this from a regular pointer variable to a smart-pointer 
			// object variable so it would automatically be released when it goes out of scope.  Prior to this change, the 
			// underlying COM object was being leaked.
			IDispatchPtr p;
			hr = m_pBrowser->get_Document(&p);
			// (a.walling 2012-11-16 11:46) - PLID 53794 - Some functions may return S_FALSE which is success, but pointer is invalid / NULL
			if (SUCCEEDED(hr) && p) {
				IHTMLDocument2Ptr pDoc;
				hr = p->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc);

				if (SUCCEEDED(hr)) {	
					// We have our document, now get the body
					IHTMLElementPtr pElement;
					if (SUCCEEDED(pDoc->get_body(&pElement)) && pElement) {
						IHTMLElement2Ptr pDocumentElement(pElement);

						if (pDocumentElement) {
							if (SUCCEEDED(pDocumentElement->put_scrollTop(nTop)))
								bBody = TRUE;
							else
								bBody = FALSE;
						}
					}
				}
			}
		}

		{
			IDispatchPtr p;
			hr = m_pBrowser->get_Document(&p);
			// (a.walling 2012-11-16 11:46) - PLID 53794 - Some functions may return S_FALSE which is success, but pointer is invalid / NULL
			if (SUCCEEDED(hr) && p) {
				IHTMLDocument3Ptr pDoc;
				hr = p->QueryInterface(IID_IHTMLDocument3, (void**)&pDoc);

				if (SUCCEEDED(hr)) {	
					// We have our document, now get the body
					IHTMLElementPtr pElement;
					if (SUCCEEDED(pDoc->get_documentElement(&pElement)) && pElement) {
						IHTMLElement2Ptr pBody(pElement);

						if (pBody) {
							if (SUCCEEDED(pBody->put_scrollTop(nTop)))
								bDocument = TRUE;
							else
								bDocument = FALSE;
						}
					}
				}
			}
		}

		return bDocument || bBody;
	} NxCatchAll("Error getting scroll position");

	return FALSE;
}

void CEMRPreviewCtrlDlg::OnDestroy() 
{
	try {
		if (m_pPoppedUpPreviewDlg) {
			if (m_pPoppedUpPreviewDlg->GetSafeHwnd()) {
				::DestroyWindow(m_pPoppedUpPreviewDlg->GetSafeHwnd());
			}
			delete m_pPoppedUpPreviewDlg;
			m_pPoppedUpPreviewDlg = NULL;
		}
	} NxCatchAll(__FUNCTION__);

	__super::OnDestroy();
	
	// (a.walling 2011-11-11 11:11) - PLID 46634 - What was this doing here?
	//OleUninitialize();	
}


// (a.walling 2007-12-17 16:22) - PLID 28391 - Ensures the given class is in the current topic's class string
BOOL CEMRPreviewCtrlDlg::EnsureTopicClass(CEMRTopic* pTopic, CString strClass)
{
	try {
		// (a.walling 2007-10-12 09:07) - PLID 25548 - Ensure this is for the correct EMN
		if (pTopic->GetParentEMN() != GetCurrentEMN()) {
			return FALSE;
		}

		IHTMLElementPtr pElement = GetElementByIDOrPointer("TopicDiv", pTopic->GetID(), reinterpret_cast<long>(pTopic));

		if (pElement) {
			BSTR bstrClassRaw;

			pElement->get_className(&bstrClassRaw);

			_bstr_t bstrClasses(bstrClassRaw, false);

			CString strClasses = (LPCTSTR)bstrClasses;

			CStringArray saClasses;
			long nDelim = strClasses.Find(" ");
			if (nDelim == -1) {
				// only one!
				saClasses.Add(strClasses);
			} else {
				long nStart = 0;
				do {
					saClasses.Add(strClasses.Mid(nStart, nDelim - nStart));
					nStart = nDelim + 1;
				} while ((nDelim = strClasses.Find(" ", nStart)) != -1);
				saClasses.Add(strClasses.Mid(nStart));
			}

			for (int i = 0; i < saClasses.GetSize(); i++) {
				if (saClasses[i].CompareNoCase(strClass) == 0) {
					// the class is already here!
					return TRUE;
				}
			}

			// if we got here, then the class we are ensuring is not present
			saClasses.Add(strClass);

			CString strNewClasses;
			for (int c = 0; c < saClasses.GetSize(); c++) {
				strNewClasses += saClasses[c] + " ";
			}
			strNewClasses.TrimRight();

			// clear out any inline display styles
			IHTMLStylePtr pStyle;
			HRESULT hr = pElement->get_style(&pStyle);
			// (a.walling 2012-11-16 11:46) - PLID 53794 - Some functions may return S_FALSE which is success, but pointer is invalid / NULL
			if (SUCCEEDED(hr) && pStyle) {
				_bstr_t bstrStyle("");
				
				pStyle->put_display(bstrStyle);
			}

			_bstr_t bstrNewClasses(strNewClasses);
			pElement->put_className(bstrNewClasses);

			return TRUE;
		}
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::EnsureTopicClass");
	
	return FALSE;
}

// (a.walling 2007-12-17 16:22) - PLID 28391 - Ensures the given class is NOT in the current topic's class string
BOOL CEMRPreviewCtrlDlg::EnsureTopicNotClass(CEMRTopic* pTopic, CString strClass)
{
	try {
		// (a.walling 2007-10-12 09:07) - PLID 25548 - Ensure this is for the correct EMN
		if (!pTopic || pTopic->GetParentEMN() != GetCurrentEMN()) {
			return FALSE;
		}

		IHTMLElementPtr pElement = GetElementByIDOrPointer("TopicDiv", pTopic->GetID(), reinterpret_cast<long>(pTopic));

		if (pElement) {
			BSTR bstrClassRaw;

			pElement->get_className(&bstrClassRaw);

			_bstr_t bstrClasses(bstrClassRaw, false);

			CString strClasses = (LPCTSTR)bstrClasses;

			CStringArray saClasses;
			long nDelim = strClasses.Find(" ");
			if (nDelim == -1) {
				// only one!
				saClasses.Add(strClasses);
			} else {
				long nStart = 0;
				do {
					saClasses.Add(strClasses.Mid(nStart, nDelim - nStart));
					nStart = nDelim + 1;
				} while ((nDelim = strClasses.Find(" ", nStart)) != -1);
				saClasses.Add(strClasses.Mid(nStart));
			}

			BOOL bUpdate = FALSE;
			for (int i = saClasses.GetSize() - 1; i >= 0; i--) {
				if (saClasses[i].CompareNoCase(strClass) == 0) {
					// the class is already here!
					// it must be destroyed.
					saClasses.RemoveAt(i);
					bUpdate = TRUE;
				}
			}

			if (bUpdate) {
				// we modified the classes, so update.
				
				// clear out any inliine sytles
				IHTMLStylePtr pStyle;
				HRESULT hr = pElement->get_style(&pStyle);
				// (a.walling 2012-11-16 11:46) - PLID 53794 - Some functions may return S_FALSE which is success, but pointer is invalid / NULL
				if (SUCCEEDED(hr) && pStyle) {
					_bstr_t bstrStyle("");
					
					pStyle->put_display(bstrStyle);
				}

				CString strNewClasses;
				for (int c = 0; c < saClasses.GetSize(); c++) {
					strNewClasses += saClasses[c] + " ";
				}
				strNewClasses.TrimRight();

				_bstr_t bstrNewClasses(strNewClasses);
				pElement->put_className(bstrNewClasses);
			}

			return TRUE;
		}
	} NxCatchAll("Error in CEMRPreviewCtrlDlg::EnsureTopicNotClass");

	return FALSE;
}

// (a.walling 2008-07-01 12:53) - PLID 30570 - Is the current EMN writable?
BOOL CEMRPreviewCtrlDlg::IsEmnWritable()
{
	if (m_pCurrentInfo != NULL && m_pCurrentInfo->pEMN != NULL) {
		if (m_pCurrentInfo->pEMN->IsLockedAndSaved())
			return FALSE;
		if (!m_pCurrentInfo->pEMN->IsWritable())
			return FALSE;

		return TRUE;
	}
	
	return FALSE;
}

// (a.walling 2008-10-14 10:22) - PLID 31678 - Configure the preview
void CEMRPreviewCtrlDlg::ConfigurePreview()
{
	try {
		CConfigEMRPreviewDlg dlg(GetParent());
		if (IDOK == dlg.DoModal()) {
			// the options have changed!

			// (a.walling 2007-12-17 13:53) - PLID 28354 - We need to ensure that the preview is entirely rebuilt with our new options.
			// (a.walling 2008-07-01 16:13) - PLID 30586 - Including if the logo option switched on/off
			if (dlg.m_bEMNGlobalDataChanged) {
				if (GetCurrentEMN()) {
					GetCurrentEMN()->GenerateHTMLFile(TRUE, FALSE, FALSE);
				}
			} else if (dlg.m_bMoreInfoChanged) {
				if (m_pTreeWnd != NULL && GetCurrentEMN() != NULL) {
					m_pTreeWnd->UpdatePreviewMoreInfo(GetCurrentEMN());
				}
			}
		}
	} NxCatchAll("Error in ConfigurePreview");
}

// (a.walling 2008-10-23 16:42) - PLID 31819 - Get the detail object from the url
CEMNDetail* CEMRPreviewCtrlDlg::GetDetailFromURL(const CString& strUrl)
{
	if (strUrl.Left(15) == "nexemr://detail") {
		long ixParam = strUrl.Find("/?", 0);
		if (ixParam > 0) {
			CString strCommand = strUrl.Mid(9, ixParam - 9);
			CString strParam = strUrl.Mid(ixParam + 2, strUrl.GetLength() - ixParam - 2);


			if(strCommand == "detailID") {
				long nDetailID = atol(strParam);
				if (nDetailID >= 0) {
					if (m_pCurrentInfo->pEMN) {
						return m_pCurrentInfo->pEMN->GetDetailByID(nDetailID);
					}
				}
			} else if (strCommand == "detailPT") {
				long nDetailPtr = atol(strParam);
				return m_pCurrentInfo->pEMN->GetDetailByPointer(nDetailPtr);
			}
		}
	} else if (strUrl.Left(18) == "nexemr://narrative") {
		// (a.walling 2010-03-26 12:44) - PLID 29099 - Support getting the detail from an embedded narrative detail url
		if (m_pCurrentInfo->pEMN) {
			
			long ixParam = strUrl.Find("/?", 0);
			if (ixParam > 0) {
				CString strCommand = strUrl.Mid(9, ixParam - 9);
				CString strParam = strUrl.Mid(ixParam + 2, strUrl.GetLength() - ixParam - 2);

				long nFieldIndexStart = strParam.Find("/");
				if (nFieldIndexStart != -1) {
					CString strFieldIndex = strParam.Mid(nFieldIndexStart + 1);
					strParam = strParam.Left(nFieldIndexStart);

					long nFieldIndex = atol(strFieldIndex);

					CEMNDetail* pNarrativeDetail = NULL;
					if (strCommand == "narrativeID") {
						long nDetailID = atol(strParam);
						pNarrativeDetail = m_pCurrentInfo->pEMN->GetDetailByID(nDetailID);
					} else {
						long nDetailPtr = atol(strParam);
						pNarrativeDetail = m_pCurrentInfo->pEMN->GetDetailByPointer(nDetailPtr);
					}

					if (pNarrativeDetail) {
						// (j.armen 2012-12-03 13:56) - PLID 52752 - Changed template for Narrative Field Arrays
						CArray<NarrativeField> arNarrativeFields;
						CString strNxRichText = VarString(pNarrativeDetail->GetState(), "");

						strNxRichText.TrimRight();

						//TES 6/29/2012 - PLID 50854 - New function that handles HTML narratives
						GetNarrativeFieldArray(strNxRichText, arNarrativeFields);

						if (nFieldIndex >= 0 && nFieldIndex < arNarrativeFields.GetCount()) {
							return pNarrativeDetail->GetDetailFromNarrativeMergeField(arNarrativeFields.GetAt(nFieldIndex).strField);
						}
					}
				}
			}
		}
	}

	return NULL;
}


// (a.walling 2008-10-24 11:13) - PLID 31819 - Get the topic object from the url
CEMRTopic* CEMRPreviewCtrlDlg::GetTopicFromURL(const CString& strUrl)
{
	if (strUrl.Left(14) == "nexemr://topic") {
		long ixParam = strUrl.Find("/?", 0);
		if (ixParam > 0) {
			CString strCommand = strUrl.Mid(9, ixParam - 9);
			CString strParam = strUrl.Mid(ixParam + 2, strUrl.GetLength() - ixParam - 2);


			if(strCommand == "topicID") {
				long nTopicID = atol(strParam);

				if (nTopicID >= 0) {
					if (m_pCurrentInfo->pEMN) {								
						return m_pCurrentInfo->pEMN->GetTopicByID(nTopicID);
					}
				}
			} else if (strCommand == "topicPT") {
				long nTopicPtr = atol(strParam);								
				if (m_pCurrentInfo->pEMN) {						
					return m_pCurrentInfo->pEMN->GetTopicByPointer(nTopicPtr);
				}
			}
		}
	}

	return NULL;
}

// (a.walling 2009-10-28 14:05) - PLID 35989 - Find the best topic for a new signature (either a *signature* topic, or the very last topic on an EMN)
CEMRTopic* CEMRPreviewCtrlDlg::FindAppropriateSignatureTopic(bool* pbIsSignatureTopic)
{
	if (m_pCurrentInfo && m_pCurrentInfo->pEMN) {
		// (z.manning 2011-10-28 17:57) - PLID 44594 - Moved core logic to CEMN class
		return m_pCurrentInfo->pEMN->FindAppropriateSignatureTopic(pbIsSignatureTopic);
	}
	return NULL;
}

// (a.walling 2009-11-23 11:45) - PLID 36395 - Prepare the multi-document print template for printing the currently displayed document
CString CEMRPreviewCtrlDlg::PrepareCurrentPrintTemplate()
{
	try {
		// (a.walling 2008-10-15 10:21) - PLID 31404 - Generate the header and footer strings
		NxPrintTemplate::DocInfo printInfo;

		// (a.walling 2008-11-14 08:39) - PLID 32024 - We now can generate the header and footer
		// with either a CEMN object or an ID for querying the database
		CEMN* pEMN = GetCurrentEMN();
		long nEmnID = -1;

		if (pEMN == NULL) {
			nEmnID = GetDisplayedEMNID();
		}

		if(pEMN == NULL && nEmnID == -1){
			//(e.lally 2012-05-08) PLID 50248 - We must not have an EMN to print.
			//If you tried to print an EMR with no EMNs in it, you can ignore this assertion.
			ASSERT(FALSE);
			return "";
		}
		
		CEMN::GeneratePrintHeaderFooterHTML(pEMN, nEmnID, printInfo.strHeaderHTML, printInfo.strFooterHTML);

		// (a.walling 2012-05-08 16:35) - PLID 50241 - NxPrintTemplate::DocInfo
		CList<NxPrintTemplate::DocInfo, NxPrintTemplate::DocInfo&> listEMNs;
		listEMNs.AddTail(printInfo);

		return PreparePrintTemplate(listEMNs);

	} NxCatchAll("Error preparing current print template");

	ASSERT(FALSE);
	return "";
}

// (a.walling 2008-10-06 16:49) - PLID 31430 - Prepare a print template for the current EMN, return the path
// (a.walling 2009-11-23 11:45) - PLID 36395 - Prepare the multi-document print template for the list of documents passed in
// (r.gonet 06/13/2013) - PLID 56850 - Added a flag bDisplayPerDocumentPageCount to control whether we reset the page count each document or not. Might want to consider
//  making a settings struct if we get too many of these things. Right now this code smells a little funky, but not too bad.
CString CEMRPreviewCtrlDlg::PreparePrintTemplate(CList<NxPrintTemplate::DocInfo, NxPrintTemplate::DocInfo&>& listEMNs, bool bDisplayPerDocumentPageCount/* = true*/)
{
	try {
		// (a.walling 2012-05-08 16:35) - PLID 50241 - Use NxPrintTemplate::DocInfo::PreparePrintTemplate
		// (r.gonet 06/13/2013) - PLID 56850 - Pass the bDisplayPerDocumentPageCount
		return NxPrintTemplate::DocInfo::PreparePrintTemplate(PrepareDocumentDefinition(listEMNs), bDisplayPerDocumentPageCount);
	} NxCatchAll(__FUNCTION__);
	return "";
}

// (a.walling 2009-11-23 11:46) - PLID 36395 - Define the document sources for the print template
CString CEMRPreviewCtrlDlg::PrepareDocumentDefinition(CList<NxPrintTemplate::DocInfo, NxPrintTemplate::DocInfo&>& listEMNs)
{
	long nCount = listEMNs.GetSize();

	if (nCount == 0) {
		ThrowNxException("No documents to define");
	}

	CString strDefinition;
	strDefinition.Format("\t\tallDocuments = new Array(%li);\r\n", nCount);

	long nCurrent = 0;
	POSITION pos = listEMNs.GetHeadPosition();
	while (pos) {
		NxPrintTemplate::DocInfo& printInfo(listEMNs.GetNext(pos));

		// (a.walling 2012-05-08 16:35) - PLID 50241 - Use NxPrintTemplate::DocInfo::GetDocumentDefinition
		strDefinition += printInfo.GetDocumentDefinition(nCurrent);
 
		nCurrent++;
	}

	return strDefinition;
}

// (a.walling 2009-11-23 11:47) - PLID 36396 - Shared method to decrypt a saved EMN Preview and return the path to the decrypted MHT file
// (a.walling 2011-06-17 15:35) - PLID 42367 - Output params for actual file name and time
// (z.manning 2012-09-11 14:03) - PLID 52543 - Added modified date
bool CEMRPreviewCtrlDlg::GetMHTFile(long nID, COleDateTime dtEmnModifiedDate, CString& strDecryptedFilePath, CString* pstrActualFileName, FILETIME* pLastWriteTime)
{	
	// (a.walling 2007-06-11 11:08) - PLID 26263 - use a web archive to keep all images and etc in one file
	CString strFile, strTempFileBase;
	strFile.Format("EMN_%li.mht", nID);
	// (a.walling 2007-07-25 11:34) - PLID 26261 - Use a common prefix for temp files
	strTempFileBase.Format("nexemrt_EMN_%li", nID);

	// (a.walling 2007-06-11 13:07) - PLID 26278 - Keep EMNPreviews all in one directory
	CString strPath = GetSharedPath() ^ "EMNPreview\\";

	// (z.manning 2012-09-13 14:26) - PLID 52630 - It's not much more likely that an EMN was created outside of Practice
	// now that we have the API and iPad app. So if we could not find a preview file, then generate one here.
	BOOL bAlreadyGeneratedPreview = FALSE;
	if (!FileUtils::DoesFileOrStreamExist(strPath ^ strFile)) {
		RegenerateEmnPreviewFromData(nID, FALSE);
		// (z.manning 2012-09-13 14:43) - PLID 52630 - Failsafe to make sure we don't generate the same preview again
		bAlreadyGeneratedPreview = TRUE;
	}

	if (FileUtils::DoesFileOrStreamExist(strPath ^ strFile))
	{
		COleDateTime dtModifiedDateField = NxCDO::GetModifiedDateFieldFromFile(strPath ^ strFile);

#ifdef _DEBUG
		// (z.manning 2012-09-13 14:37) - For debugging, so you can see the actual date/time values.
		SYSTEMTIME stEmnModifiedDate = AsSystemTime(dtEmnModifiedDate);
		SYSTEMTIME stModifiedDateField = AsSystemTime(dtModifiedDateField);
#endif
		// (z.manning 2012-09-11 17:15) - PLID 52543 - We need to check the given EMN modified date against the modified
		// date stored in the file because it's now possible that an EMN was updated outside of Practice, such as by the API.
		// If that happened then we need to regenerate the preview from data.
		// Note: Milliseconds can cause some slight quirkiness here which is why there is a 1 second threshold in the time 
		// comparison so that we don't do this update needlessly.
		if(!bAlreadyGeneratedPreview && dtEmnModifiedDate.GetStatus() != COleDateTime::valid || dtModifiedDateField.GetStatus() != COleDateTime::valid
			|| dtEmnModifiedDate > dtModifiedDateField + COleDateTimeSpan(0, 0, 0, 1))
		{
			RegenerateEmnPreviewFromData(nID, FALSE);
		}

		// the file exists! load it.
		// (a.walling 2007-07-19 09:57) - PLID 26261 - Use NxTemp
		CString strTempPath = GetNxTempPath();

		// (a.walling 2007-07-25 11:30) - PLID 26261 - Changed the way this file is created; sometimes this can fail if the
		// system has an invalid encryption setup. Might be best to avoid this altogether to avoid causing issues for clients.
		// I implemented yet another failsafe to try to cleanup this temp data, so hopefully we should be good.

		HANDLE hFile = CreateNxTempFile(strTempFileBase, "mht", &strDecryptedFilePath, TRUE, FILE_ATTRIBUTE_HIDDEN);

		if (hFile == INVALID_HANDLE_VALUE) {
			ThrowNxException("Could not create temp file for EMN preview");
		}

		// (a.walling 2011-06-17 15:35) - PLID 42367 - Update the actual file name
		if (pstrActualFileName) {
			*pstrActualFileName = strPath ^ strFile;
		}

		// (a.walling 2011-06-17 15:35) - PLID 42367 - and the file time
		if (!NxCDO::DecryptMHTFromFile(strPath ^ strFile, hFile, pLastWriteTime)) {
			CloseHandle(hFile);
			ThrowNxException("Could not decrypt secure EMN preview!");
		}

		CloseHandle(hFile);

		// (a.walling 2007-06-12 17:16) - PLID 26261 - For super-duper-extra-safety, go ahead and delay deletion
		// of this file at reboot. This will help cover us for abnormal program terminations and poweroutages.
		// Can't just have these sensitive files laying around in the temp drive. I created thousands of temp files
		// and delayed move on all of them on Vista and 2000 (virtual machine) and noticed no delay or abnormality next
		// startup. Harmless too if the file is successfully deleted beforehand.
		MoveFileEx(strDecryptedFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

		return true;
	}

	return false;
}

// (a.walling 2009-11-23 12:30) - PLID 36404 - Are we currently printing?
bool CEMRPreviewCtrlDlg::IsPrinting()
{
	return m_nPrintTemplateInstantiationCount > 0;
}

// (a.walling 2009-11-23 12:54) - PLID 36404 - We want to know when printing is complete. We can detect this by tracking the print template
// instantiation and teardown events. I originally used a bool but thankfully tested on IE6. For whatever reason, IE6 would fire Create 
// twice and immediately Destroy once, then Destroy a second time when the print template is actually being destroyed. Thankfully just
// using a count works out fine.
//
// Now, the first thing I thought was "Why not just pass the PRINT_WAITFORCOMPLETION flag in the IOleCommandTarget->Exec parameters?
// Then I remembered that we are passing the print template string as that parameter, and we can't combine them at all. But this approach
// will function the same. Since this occurs very quickly, it is unlikely that anyone would actually encounter problems, but we risk 
// problems ignoring this since we clean up temp files when closing the host. There is also unknown behaviour regarding how MSHTML would
// handle its host being destroyed while in process of printing, though MSHTML does seem to handle it gracefully.
//
// Debugging into MSHTML shows me that printing involves spawning a thread which hosts another browser control instance which handles all
// the printing. It appears it clones the current document into that instance, so the 'document' source that can be passed into the print
// template remains valid even if we immediately modify or clear our instance's internal document.

void CEMRPreviewCtrlDlg::OnPrintTemplatesTornDown(LPDISPATCH pDisp)
{
	try {
		if (GetParent() && ::IsWindow(GetParent()->GetSafeHwnd())) {				
			TRACE("\tNotifying parent...\n", m_nPrintTemplateInstantiationCount);
			// (a.walling 2010-01-12 08:38) - PLID 36840 - Changed to defined message with EMRPREVIEW_ rather than a registered message
			GetParent()->PostMessage(NXM_EMRPREVIEW_PRINT_COMPLETE, (WPARAM)0, LPARAM(0));
		}
	} NxCatchAll("OnPrintTemplatesTornDown");
}

// (a.walling 2012-03-07 08:36) - PLID 48680 - Preview pane commands and options
void CEMRPreviewCtrlDlg::Print()
{
	try {
		// (c.haag 2013-02-28) - PLID 55365 - Consolidated duplicate logic into one function
		DoPrint(IDM_PRINT);
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-03-07 08:36) - PLID 48680 - Preview pane commands and options
void CEMRPreviewCtrlDlg::PrintPreview()
{
	try {
		// (c.haag 2013-02-28) - PLID 55365 - Consolidated duplicate logic into one function
		DoPrint(IDM_PRINTPREVIEW);
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2009-11-24 09:45) - PLID 36418
void CEMRPreviewCtrlDlg::PrintMultipleEMNs()
{
	try {
		// (c.haag 2013-02-28) - PLID 55365 - Consolidated duplicate logic into one function
		DoPrintMultiple(NULL, 0 /* Entire EMR */);
	} NxCatchAll("Error printing multiple EMNs");
}

// (c.haag 2013-02-28) - PLID 55365 - Critical path for printing single EMN's
void CEMRPreviewCtrlDlg::DoPrint(UINT uMenuID)
{
		// (c.haag 2013-02-28) - PLID 55365 - The first thing we do is determine whether this EMN has any
		// custom previews. If it does, then the user will need to choose whether to print the default print
		// template or print one or more custom preview layouts.
		//
		// This function can be called from within EMR, or from outside EMR. 
		//
		BOOL bHasCustomLayouts = FALSE;
		long nEmnID = GetDisplayedEMNID();
		if (-1 == nEmnID) {
			CEMN* pEMN = GetCurrentEMN();
			if (NULL != pEMN) {

				// We should be within an EMR here. Make sure the EMN is saved.
				if(pEMN->IsUnsaved()) {
					if(IDYES != AfxMessageBox("Before continuing, the changes you have made to the EMN must be saved.  Would you like to continue?", MB_YESNO)) {
						return;
					}
					if(FAILED(m_pTreeWnd->SaveEMR(esotEMN, (long)pEMN, TRUE))) {
						AfxMessageBox("The EMN was not saved. The operation will now be cancelled.", MB_OK | MB_ICONEXCLAMATION);
						return;
					}
				}

				nEmnID = pEMN->GetID();
			} else {
				ThrowNxException("Could not determine current EMN");
			}
		}
		// Load the custom preview layouts
		NexTech_Accessor::_EMRCustomPreviewLayoutFilterPtr pFilter(__uuidof(NexTech_Accessor::EMRCustomPreviewLayoutFilter)); 
		NexTech_Accessor::_EMRCustomPreviewLayoutOptionsPtr pOptions(__uuidof(NexTech_Accessor::EMRCustomPreviewLayoutOptions));
		pFilter->EmnIDs = Nx::SafeArray<BSTR>::FromValue(_bstr_t(nEmnID));
		pOptions->IncludeData = VARIANT_TRUE;
		//	Create our SAFEARRAY to be passed to the function in the API
		Nx::SafeArray<IUnknown *> saFilters = Nx::SafeArray<IUnknown *>::FromValue(pFilter);
		NexTech_Accessor::_EMRCustomPreviewLayoutsPtr pLayouts = GetAPI()->GetEMRCustomPreviewLayouts(
			GetAPISubkey(), GetAPILoginToken(), saFilters, pOptions);		
		if (NULL != pLayouts->Layouts) 
		{
			Nx::SafeArray<IUnknown *> saryLayouts(pLayouts->Layouts);
			if (saryLayouts.GetCount() > 0)
			{
				bHasCustomLayouts = TRUE;
			}
		}

		// (c.haag 2013-02-28) - PLID 55365 - If there are any custom preview layouts, we need to use the multiple
		// EMN print dialog so the user can choose what they want to print
		if (bHasCustomLayouts)
		{
			DoPrintMultiple(pLayouts.Detach(), nEmnID);
		}
		else
		{
			// (a.walling 2008-09-18 15:57) - PLID 31430 - Warn if the page title will not be displayed
			// CheckAndWarnPageSetup(hwnd);
			// Print contents of WebBrowser control.
			// (a.walling 2008-10-08 10:31) - PLID 31430 - Get the PrintTemplate path. If it is empty,
			// something failed, so fall back to the built-in default print template
			// (a.walling 2009-11-23 11:51) - PLID 36395 - Prepare the multi-document print template for only the current document
			CString strPrintTemplate = PrepareCurrentPrintTemplate();
			_variant_t varTemplate = _bstr_t(strPrintTemplate);
			GetOleCommandTarget()->Exec(&CGID_MSHTML, 
				uMenuID, // (c.haag 2013-02-28) - PLID 55365 - Pass in the menu ID
				OLECMDEXECOPT_PROMPTUSER, 
				strPrintTemplate.IsEmpty() ? NULL : &varTemplate,
				NULL);

			// (d.thompson 2013-11-07) - PLID 59351 - Audit when printing.  It is intentionally that always audit on the printing 
			//	"attempt".  If they cancel the print dialog, the printer fails, isn't connected, whatever, we'll still audit that
			//	a print was attempted.
			// (d.thompson 2013-12-11) - PLID 59351 - Also support printing from the NexEMR tab.  In that case, we don't have
			//	a loaded CEMN object, but must lookup ourselves.
			{
				long nPatientID = -1, nEMNIDToAudit = -1;
				CString strPatientName, strEMNDescription;

				CEMN *pEMNPrinted = GetCurrentEMN();
				if(pEMNPrinted) {
					//We have a pointer to an EMN.  Most likely you printed from within an EMN itself.
					nPatientID = pEMNPrinted->GetParentEMR()->GetPatientID();
					strPatientName = pEMNPrinted->GetPatientName();
					nEMNIDToAudit = pEMNPrinted->GetID();
					strEMNDescription = pEMNPrinted->GetDescription();
				}
				else if(nEmnID != -1) {
					//We don't have a pointer to an EMN, but we do have a known EMN ID.  Most likely you're printing from the NexEMR
					//	tab, or somewhere else that doesn't actually have an EMN loaded.  We'll have to do a database lookup.
					//We are intentionally loading the name from the EMN, not the name from PersonT, as that's the same thing audited above.
					ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT PatientID, PatientLast + ', ' + PatientFirst + ' ' + PatientMiddle AS PatientName, Description "
						"FROM EMRMasterT WHERE ID = {INT}", nEmnID);
					if(!prs->eof) {
						nPatientID = AdoFldLong(prs->Fields, "PatientID");
						strPatientName = AdoFldString(prs->Fields, "PatientName", "");
						nEMNIDToAudit = nEmnID;
						strEMNDescription = AdoFldString(prs->Fields, "Description");
					}
					else {
						//Should be impossible.  We have an EMR ID to print but it doesn't exist anymore?
						AfxThrowNxException("EMN Print was unable to audit.");
					}
				}
				else {
					//We are printing, but not from within an EMN, and not from the NexEMR tab.  Should we be auditing here?  How did you get
					//	here?  Please evaluate what is being printed and if it's an EMN, and if so, how did we get here without an EMN ID?
					//	Either that code should set the EMN ID, or this printing needs to find another way to get the patient information.
					ASSERT(FALSE);
					return;
				}

				AuditEventItems aei;
				CString strNewText;
				if(uMenuID == IDM_PRINT) {
					aei = aeiEMNPrinted;
					strNewText = "EMN " + strEMNDescription + " Printed";
				}
				else {//IDM_PRINTPREVIEW
					aei = aeiEMNPrintPreviewed;
					strNewText = "EMN " + strEMNDescription + " Print Previewed";
				}

				AuditEvent(nPatientID, strPatientName, BeginNewAuditEvent(), aei, nEMNIDToAudit, 
					"", strNewText, aepMedium, aetOpened);
			}
		}
}

// (c.haag 2013-02-28) - PLID 55365 - Critical path for printing multiple EMN's or an EMN with at least one
// custom preview layout. If you have the EMN's custom preview layouts, you should pass them in here. Otherwise 
// leave it NULL, and the multiple print dialog will get the list on its own later on. If nSingleEMNID is a positive number,
// it means we're only printing one EMN. Otherwise we're printing the whole EMR.
void CEMRPreviewCtrlDlg::DoPrintMultiple(LPUNKNOWN lpunkEMRCustomPreviewLayouts, long nSingleEMNID)
{
		CWnd* pWndContainer = m_pTreeWnd;

		if (!pWndContainer) {
			pWndContainer = GetParent();
		}

		if (!pWndContainer) {
			ASSERT(FALSE);
			return;
		}

		// (a.walling 2010-01-12 08:38) - PLID 36840 - Changed to defined message with EMRPREVIEW_ rather than a registered message
		// (c.haag 2013-02-28) - PLID 55365 - Pass in the list of custom preview layouts so the preview doesn't have to calculate it on its own (if available),
		// and pass in the single EMN ID so that the dialog doesn't show all EMN's if it should not.
		pWndContainer->SendMessage(NXM_EMRPREVIEW_PRINT_MULTIPLE, (WPARAM)lpunkEMRCustomPreviewLayouts, nSingleEMNID);
}


// (a.walling 2010-11-12 11:18) - PLID 38135 - Popup preview with all patient EMNs
void CEMRPreviewCtrlDlg::PopupAllPatientEMNs()
{
	try {
		long nCurrentPatientID = -1;

		CEMN* pEMN = GetCurrentEMN();
		if (pEMN) {
			if (pEMN->GetParentEMR()) {
				nCurrentPatientID = pEMN->GetParentEMR()->GetPatientID();
			}
		}

		if (nCurrentPatientID < 0) {
			ThrowNxException("Could not determine current patient");
		}

		// (z.manning 2012-09-10 15:57) - PLID 52543 - Added modified date
		ADODB::_RecordsetPtr prs = CreateParamRecordset(
			"SELECT EMRMasterT.ID, EmrMasterT.ModifiedDate "
			"FROM EMRMasterT "
			"WHERE "
				"EMRMasterT.Deleted = 0 "
				"AND EMRMasterT.PatientID = {INT} "
			"ORDER BY EMRMasterT.Date DESC, EMRMasterT.ModifiedDate DESC"
			, nCurrentPatientID
		);

		if (prs->eof) {
			MessageBox("No other patient EMNs are available to be displayed.");
			return;
		}
		
		DontShowMeAgain(this, 
			"All saved EMN previews for this patient will ordered by date and available in the popup.\r\n\r\n"
			"Previewing an EMN which is currently being edited will not be updated automatically, however it may still be used to refer to the EMN "
			"as it was when it was last saved.\r\n\r\n"
			"You may also open up EMN previews from the History tab within the EMR editor."
			, "PopupAllPatientEMNs");

		// (z.manning 2012-09-10 15:57) - PLID 52543 - Use the new EmnPreviewPopup struct
		CArray<EmnPreviewPopup, EmnPreviewPopup&> aryEMNs;
		long nIndex = 0;
		while (!prs->eof) {
			aryEMNs.Add(EmnPreviewPopup(AdoFldLong(prs, "ID"), AdoFldDateTime(prs, "ModifiedDate")));
			prs->MoveNext();
		}

		if (!m_pPoppedUpPreviewDlg) {
			m_pPoppedUpPreviewDlg = new CEMRPreviewPopupDlg(this);	
			m_pPoppedUpPreviewDlg->Create(IDD_EMR_PREVIEW_POPUP, this);

			// (a.walling 2010-01-11 15:11) - PLID 31482 - Restore size
			m_pPoppedUpPreviewDlg->RestoreSize("EMREditor");
		}
		
		// (j.jones 2009-09-22 11:55) - PLID 31620 - PreviewEMN now takes in an array
		// of all available EMN IDs, but since we haven't opened the dialog yet,
		// we can pass in an empty array.
		m_pPoppedUpPreviewDlg->SetPatientID(nCurrentPatientID, aryEMNs);
		m_pPoppedUpPreviewDlg->PreviewEMN(aryEMNs, nIndex);
		m_pPoppedUpPreviewDlg->ShowWindow(SW_SHOWNA);

	} NxCatchAll("Error popping up other patient EMNs");
}

// (a.walling 2012-11-05 11:58) - PLID 53588 - EmrPreviewCtrlInterface no longer necessary; merged into here

// (a.walling 2007-04-10 16:41) - PLID 25548 - Called when context menu about to be shown
HRESULT CEMRPreviewCtrlDlg::OnShowContextMenu(DWORD dwID, POINT *ppt, 
					IUnknown *pcmdtReserved, IDispatch *pdispReserved)
{
	HRESULT result	= S_FALSE;
	BOOL	handled	= FALSE;

	// (a.walling 2007-04-10 16:42) - PLID 25548 - Not doing much here now, just tracing in debug mode.
	// see mshtmhst.h for other possible dwID's
	// (j.jones 2007-07-05 11:26) - PLID 25457 - added right click menu for hyperlinks	
	// (a.walling 2010-03-25 20:34) - PLID 27372 - Handle text selections for copying to clipboard
	if (dwID == CONTEXT_MENU_ANCHOR) {		
		result = CustomContextMenu(dwID, ppt, pcmdtReserved, pdispReserved);
		handled	= TRUE;
	}
	else if (dwID == CONTEXT_MENU_TEXTSELECT)
	{
		result = CustomContextMenu(dwID, ppt, pcmdtReserved, NULL);
		handled	= TRUE;
	}
	else if (dwID == CONTEXT_MENU_DEFAULT)
	{
		result = CustomContextMenu(dwID, ppt, pcmdtReserved, NULL);
		handled	= TRUE;
	} else {
		return S_OK; // disable all other popup menus
	}

	// (a.walling 2007-04-04 10:45) - We don't want any other context menus. however, 
	// it is useful to have refresh and view source and properties etc when debugging
	
	if (!handled)
	{
		result = S_FALSE;
	}	

	return result;
}

// (j.jones 2007-07-05 14:38) - PLID 25457 - added pdispReserved
// (a.walling 2010-03-25 20:34) - PLID 27372 - pass the id of the type of context
HRESULT CEMRPreviewCtrlDlg::CustomContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved)
{
	// (a.walling 2007-04-10 16:24) - Whenever we actually have a context menu,
	// then we can display it here.
	// (a.walling 2007-06-19 10:39) - PLID 26376 - We "need" to be able to print/preview

	// (a.walling 2012-03-16 11:34) - PLID 48947 - Context menu bug causing MSHTML to stop loading new documents
	// (a.walling 2012-03-16 11:34) - PLID 48947 - Could it be something as simple as failing to release this IOleWindow* reference?
	// I changed it to release and it seemed to work. We don't really even need it in the first place though.
	//IOleWindow*	oleWnd			= NULL;
    //HWND		hwnd			= NULL;
	//HMENU		hMainMenu		= NULL;
	//HMENU		hPopupMenu		= NULL;
	HWND		hwnd			= GetSafeHwnd();
	HRESULT		hr				= 0;

	if ((ppt == NULL) || (pcmdtReserved == NULL))
		return S_OK;

	/*
    hr = pcmdtReserved->QueryInterface(IID_IOleWindow, (void**)&oleWnd);
	if ( (hr != S_OK) || (oleWnd == NULL))
		return S_OK;

	hr = oleWnd->GetWindow(&hwnd);
	if ( (hr != S_OK) || (hwnd == NULL))
		return S_OK;
	*/

	if (!hwnd) {
		ASSERT(FALSE);
		return S_OK;
	}

	ContextMenuInfo contextMenuInfo;
	
	contextMenuInfo.dwID = dwID;

	 ::GetMessagePos(&contextMenuInfo.pt);

	if(pdispReserved) {
		//grab the Url
		IHTMLAnchorElementPtr pElement(pdispReserved);
		if(pElement) {
			_bstr_t bstrHref;
			pElement->get_href(bstrHref.GetAddress());
			contextMenuInfo.strUrl = (LPCTSTR)bstrHref;
		}
	}

	contextMenuInfo.bValid = true;
	m_contextMenuInfo = contextMenuInfo;
	PostMessage(WM_CONTEXTMENU, (WPARAM)hwnd, MAKELPARAM(contextMenuInfo.pt.x, contextMenuInfo.pt.y));

	// (a.walling 2012-03-16 11:34) - PLID 48947 - Handle async in the parent

	return S_OK;
}

// (a.walling 2012-03-16 11:34) - PLID 48947 - Now this handles all the context menu stuff itself outside of the ui interface handler
void CEMRPreviewCtrlDlg::OnContextMenu(CWnd* pWnd, CPoint pos)
{
	if (!m_contextMenuInfo.bValid)
	{
		__super::OnContextMenu(pWnd, pos);
		return;
	}

	try {

		ContextMenuInfo contextMenuInfo;
		std::swap(contextMenuInfo, m_contextMenuInfo);

		CString strUrl = contextMenuInfo.strUrl;

		// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
		//CNxMenu mnu;
		// (a.walling 2012-03-16 11:34) - PLID 48947 - For now just use a normal menu; won't be pretty, but avoids the trigger of a strange issue in the pane
		// (a.walling 2012-03-19 13:41) - PLID 48947 - Fixed, back to CNxMenu
		CNxMenu mnu;
		long n = 0;
		mnu.CreatePopupMenu();

		CMenu mnuPosition;
		mnuPosition.CreatePopupMenu();

		// (j.jones 2007-07-05 14:41) - PLID 25457 - supported a right click menu for hyperlinks,
		// indicated by pdispReserved not being NULL (remains NULL for non-hyperlink right clicks)
		
		BOOL bIsEmbeddedDetail = FALSE;
		BOOL bIsDetail = FALSE;
		BOOL bIsTopic = FALSE;
		BOOL bIsMoreInfoTopic = FALSE;

		// (a.walling 2010-03-25 20:34) - PLID 27372 - Copy
		if (contextMenuInfo.dwID == CONTEXT_MENU_TEXTSELECT) {
			mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miCopy, "&Copy");
			mnu.InsertMenu(n++, MF_BYPOSITION|MF_SEPARATOR, 0, "");	
		}

		if (!m_bInteractive) {
			// (a.walling 2008-11-14 08:39) - PLID 32024 - Moved all this to its own block for ease of maintenance
			// only show print, print preview		
			if (!m_bInteractive && GetDisplayedEMNID() != -1) {
				// an EMN is actually displayed in the popup
				mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miPrint, "&Print...");
				mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miPrintPreview, "Print Previe&w...");
				//(e.lally 2009-10-26) PLID 32503 - Added EMN faxing from outside the editor
				//Show the menu option as disabled if they are not licensed for eFaxing to promote its features. Otherwise users
					//don't know what they are missing out on.
				DWORD dwHasEFaxLicense = (MF_DISABLED | MF_GRAYED);
				if(g_pLicense->CheckForLicense(CLicense::lcFaxing, CLicense::cflrSilent)) {
					dwHasEFaxLicense = MF_ENABLED;
				}
				mnu.InsertMenu(n++, MF_BYPOSITION|dwHasEFaxLicense, ContextMenuInfo::miFax, "Fa&x...");
				
				{ // (a.walling 2009-11-24 09:43) - PLID 36418
					mnu.InsertMenu(n++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
					mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miPrintMultiple, "Print / Preview M&ultiple EMNs...");
				}
				mnu.InsertMenu(n++, MF_BYPOSITION|MF_SEPARATOR, 0, "");			
				mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miFind, "&Find...");
				mnu.InsertMenu(n++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
			}
			// (a.walling 2008-10-14 10:57) - PLID 31678 - Configure preview option
			mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miConfigure, "&Configure Preview...");

			#ifdef _DEBUG
				// (a.walling 2008-08-14 09:42) - PLID 30570 - Removed accelerators on these to lower the possibility of conflicts
				// for new menu items. Besides, I don't think anyone uses them, and these are debug-only.
				mnu.InsertMenu(n++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
				// (a.walling 2007-10-18 14:27) - PLID 25548 - To help debug, you can view the generated source.
				mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miViewGeneratedSource, "View Generated Source");
				mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miProperties, "Properties");
			#endif
		} else {

			long nIndexToDefault = -1;

			// (a.walling 2010-01-26 13:56) - PLID 37052 - Prepare our disabled flags regardless immediately
			DWORD dwDisabled = 0;
			if (!IsEmnWritable()) {
				dwDisabled = MF_DISABLED|MF_GRAYED;
			}

			if(!strUrl.IsEmpty()) {

				// (a.walling 2010-03-26 12:44) - PLID 29099 - Are we an embedded narrative detail?
				bIsEmbeddedDetail = strUrl.Left(18) == "nexemr://narrative";
				bIsDetail = strUrl.Left(15) == "nexemr://detail";
				bIsMoreInfoTopic = strUrl.Left(17) == "nexemr://moreinfo";
				bIsTopic = (strUrl.Left(14) == "nexemr://topic") || bIsMoreInfoTopic;
				

				//if a valid Url, add our menu items
				// (a.walling 2010-03-26 12:44) - PLID 29099 - Most options removed for embedded narrative details.
				if(bIsDetail || bIsTopic || bIsEmbeddedDetail) {

					//check our preference, and bold accordingly
					long nEMRPreviewPaneLeftClick = GetRemotePropertyInt("EMRPreviewPaneLeftClick", 0, 0, GetCurrentUserName(), true);

					//do we default this entry?
					// (a.walling 2010-03-26 12:44) - PLID 29099 - embedded narrative details will always default to Popup
					if(!bIsEmbeddedDetail && (bIsTopic || nEMRPreviewPaneLeftClick == 1))
						nIndexToDefault = n;

					mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miGoToTopic, "&Go To Topic");

					//(e.lally 2010-05-03) PLID 15155 - Check for Writable EMN state instead of Edit Mode now
					if((bIsEmbeddedDetail || bIsDetail) && IsEmnWritable()) {
						mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miEdit, "&Edit Detail");
					}
					mnu.InsertMenu(n++, MF_BYPOSITION|MF_SEPARATOR, 0, "");

					if(bIsDetail || bIsEmbeddedDetail) {

						//do we default this entry?
						// (a.walling 2010-03-26 12:44) - PLID 29099 - embedded narrative details will always default to Popup
						if(nEMRPreviewPaneLeftClick == 0 || bIsEmbeddedDetail)
							nIndexToDefault = n;

						mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miPopup, "Popup &Detail");

						// (a.walling 2010-03-26 12:44) - PLID 29099 - ignore embedded narrative details
						if (!bIsEmbeddedDetail) {
							// (a.walling 2008-10-23 16:42) - PLID 31819 - Get the detail object from the url
							CEMNDetail* pDetail = GetDetailFromURL(strUrl);

							// (a.walling 2008-07-29 12:42) - PLID 30570 - Changed 'Toggle' to 'Show/Hide'
							// (a.walling 2008-07-01 10:20) - PLID 30570

							// (a.walling 2008-10-23 16:38) - PLID 31819 - Provide a better context menu since now we can access the actual CEMNDetail
							if (pDetail == NULL) {
								ASSERT(FALSE);
								mnu.InsertMenu(n++, MF_BYPOSITION|dwDisabled, ContextMenuInfo::miHideTitle, "Show/Hide &Title Display when Printing");
								mnu.InsertMenu(n++, MF_BYPOSITION|dwDisabled, ContextMenuInfo::miHideItem, "Show/Hide &Item Display when Printing");
								// (a.walling 2008-10-23 10:13) - PLID 31808 - Toggle display as subdetail
								mnu.InsertMenu(n++, MF_BYPOSITION|dwDisabled, ContextMenuInfo::miSubDetail, "Toggle Display Under Spawnin&g Item");
								// (a.walling 2012-07-13 16:38) - PLID 48896
								mnu.InsertMenu(n++, MF_BYPOSITION|dwDisabled, ContextMenuInfo::miHideIfIndirectlyOnNarrative, "Toggle Allow Hide if Indirectl&y Included on a Narrative");
							} else {

								if (CEMRTopic* pTopic = pDetail->GetParentTopic()) {
									// (a.walling 2012-07-12 09:29) - PLID 46078 - Ensure the topic is active so commands can be routed appropriately
									GetEmrFrameWnd()->ActivateTopic(pTopic, false);
								}

								long nPreviewFlags = pDetail->GetPreviewFlags();
								
								mnu.InsertMenu(n++, MF_BYPOSITION|dwDisabled|((nPreviewFlags & epfHideTitle) ? MF_UNCHECKED : MF_CHECKED), ContextMenuInfo::miHideTitle, "Display &Title when Printing");
								mnu.InsertMenu(n++, MF_BYPOSITION|dwDisabled|((nPreviewFlags & epfHideItem) ? MF_UNCHECKED : MF_CHECKED), ContextMenuInfo::miHideItem, "Display &Item when Printing");

								// (a.walling 2008-10-23 10:13) - PLID 31808 - Toggle display as subdetail
								// (a.walling 2008-10-24 11:04) - PLID 31808 - Disable if we are not spawned, eh?
								DWORD dwMenuFlags = (nPreviewFlags & epfSubDetail) ? MF_CHECKED : MF_UNCHECKED;
								CEMNDetail* pParentDetail = pDetail->GetSubDetailParent();
								if (pParentDetail == NULL) {
									dwMenuFlags |= (MF_DISABLED|MF_GRAYED);
								}
								mnu.InsertMenu(n++, MF_BYPOSITION|dwDisabled|dwMenuFlags, ContextMenuInfo::miSubDetail, "Display Under Spawnin&g Item");
								// (a.walling 2012-07-13 16:38) - PLID 48896
								mnu.InsertMenu(n++, MF_BYPOSITION|dwDisabled|((nPreviewFlags & epfHideIfIndirect) ? MF_CHECKED : MF_UNCHECKED), ContextMenuInfo::miHideIfIndirectlyOnNarrative, "Allow Hide if Indirectl&y Included on a Narrative");
								// (j.armen 2013-01-16 16:42) - PLID 54412
								mnu.InsertMenu(n++, MF_BYPOSITION|dwDisabled|((nPreviewFlags & epfHideOnIPad) ? MF_CHECKED : MF_UNCHECKED), ContextMenuInfo::miHideItemOnIPad, "Hide Item on iPad");
							}
						}
					} else if (bIsTopic && !bIsMoreInfoTopic) {
						// (a.walling 2008-08-14 10:55) - PLID 30570 - Do not show for more info. Also use the i in topic as the accelator
						// so it matches up with the same for details
						// (a.walling 2008-07-01 17:55) - PLID 30570 - Title hiding for topics too
						
						// (a.walling 2008-10-24 11:17) - PLID 31819 - Provide a better context menu since now we can access the actual CEMRTopic
						CEMRTopic* pTopic = GetTopicFromURL(strUrl);

						if (pTopic == NULL) {
							ASSERT(FALSE);
							mnu.InsertMenu(n++, MF_BYPOSITION|dwDisabled, ContextMenuInfo::miHideTitle, "Show/Hide &Title Display when Printing");
							mnu.InsertMenu(n++, MF_BYPOSITION|dwDisabled, ContextMenuInfo::miHideItem, "Show/Hide Top&ic Display when Printing");
						} else {
						
							// (a.walling 2012-07-12 09:29) - PLID 46078 - Ensure the topic is active so commands can be routed appropriately
							if (CEmrFrameWnd* pFrameWnd = GetEmrFrameWnd()) {
								pFrameWnd->ActivateTopic(pTopic, false);
							}

							long nPreviewFlags = pTopic->GetPreviewFlags();
							
							mnu.InsertMenu(n++, MF_BYPOSITION|dwDisabled|((nPreviewFlags & epfHideTitle) ? MF_UNCHECKED : MF_CHECKED), ContextMenuInfo::miHideTitle, "Display &Title when Printing");
							mnu.InsertMenu(n++, MF_BYPOSITION|dwDisabled|((nPreviewFlags & epfHideItem) ? MF_UNCHECKED : MF_CHECKED), ContextMenuInfo::miHideItem, "Display &Item when Printing");
						}
					}

					
					// (a.walling 2009-01-07 15:03) - PLID 32659 - Positioning submenu
					// (a.walling 2010-03-26 12:44) - PLID 29099 - ignore embedded narrative details
					if (!bIsEmbeddedDetail) {
						long nPreviewFlags = 0;
						if (bIsTopic) {
							CEMRTopic* pTopic = GetTopicFromURL(strUrl);
							if (pTopic) {
								nPreviewFlags = pTopic->GetPreviewFlags();
							}
						} else if (bIsDetail) {
							CEMNDetail* pDetail = GetDetailFromURL(strUrl);
							if (pDetail) {
								nPreviewFlags = pDetail->GetPreviewFlags();
							}
						}

						long nSub = 0;

						// clear both is default
						// (a.walling 2009-07-06 08:38) - PLID 34793 - Clearing is all deprecated now
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
						mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|dwDisabled|((nPreviewFlags & epfColumnOne) ? MF_CHECKED : MF_UNCHECKED), ContextMenuInfo::miColumnOne, "Column One");
						mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|dwDisabled|((nPreviewFlags & epfColumnTwo) ? MF_CHECKED : MF_UNCHECKED), ContextMenuInfo::miColumnTwo, "Column Two");
						
						// (a.walling 2009-07-06 08:38) - PLID 34793 - Clearing is all deprecated now
						/*
						mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
						mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|dwDisabled|(bClearLeft ? MF_CHECKED : MF_UNCHECKED), ContextMenuInfo::miClearLeft, "Clear Left");
						mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|dwDisabled|(bClearRight ? MF_CHECKED : MF_UNCHECKED), ContextMenuInfo::miClearRight, "Clear Right");
						*/

						// (a.walling 2009-07-06 12:30) - PLID 34793 - Grouping for columns
						if (bIsTopic) {
							mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
							mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|dwDisabled|((nPreviewFlags & epfGroupBegin) ? MF_CHECKED : MF_UNCHECKED), ContextMenuInfo::miGroupBegin, "Group Columns at Beginning");
							mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|dwDisabled|((nPreviewFlags & epfGroupEnd) ? MF_CHECKED : MF_UNCHECKED), ContextMenuInfo::miGroupEnd, "Group Columns at End");
						}


						// (a.walling 2009-01-08 14:07) - PLID 32660 - Align text right
						mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
						mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|dwDisabled|((nPreviewFlags & epfTextRight) ? MF_CHECKED : MF_UNCHECKED), ContextMenuInfo::miTextRight, "Align Text Right");
						// (a.walling 2010-08-31 18:20) - PLID 36148 - Page breaks
						mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
						mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|dwDisabled|((nPreviewFlags & epfPageBreakBefore) ? MF_CHECKED : MF_UNCHECKED), ContextMenuInfo::miPageBreakBefore, "New Page Before");
						mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|dwDisabled|((nPreviewFlags & epfPageBreakAfter) ? MF_CHECKED : MF_UNCHECKED), ContextMenuInfo::miPageBreakAfter, "New Page After");
						
						mnu.InsertMenu(n++, MF_BYPOSITION|MF_POPUP|dwDisabled, (UINT_PTR)mnuPosition.GetSafeHmenu(), "Positioning");
					}
					mnu.InsertMenu(n++, MF_BYPOSITION|MF_SEPARATOR, 0, "");

				} // if(bIsDetail || bIsTopic) {
			} // if(!strUrl.IsEmpty)

			mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miPrint, "&Print...");
			mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miPrintPreview, "Print Previe&w...");
			//(e.lally 2009-10-26) PLID 32503 - Show the Fax option under the print preview
				//Show the menu option as disabled if they are not licensed for eFaxing to promote its features. Otherwise users
				//don't know what they are missing out on.
			DWORD dwHasEFaxLicense = (MF_DISABLED | MF_GRAYED);
			if(g_pLicense->CheckForLicense(CLicense::lcFaxing, CLicense::cflrSilent)) {
				dwHasEFaxLicense = MF_ENABLED;
			}
			mnu.InsertMenu(n++, MF_BYPOSITION|dwHasEFaxLicense, ContextMenuInfo::miFax, "Fa&x...");

			{ // (a.walling 2009-11-24 09:43) - PLID 36418
				CEMN* pCurrentEMN = GetCurrentEMN();
				if (pCurrentEMN) {
					CEMR* pCurrentEMR = pCurrentEMN->GetParentEMR();
					if (pCurrentEMR) {
						if (pCurrentEMR->GetEMNCount() > 1) {
							mnu.InsertMenu(n++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
							mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miPrintMultiple, "Print / Preview M&ultiple EMNs...");
						}
					}
					
					// (a.walling 2010-11-12 11:18) - PLID 38135 - Popup preview with all patient EMNs
					mnu.InsertMenu(n++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
					mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miPopupOther, "Popup All Patient EMN Previews...");
				}
			}

			// (z.manning 2009-08-26 09:27) - PLID 33911 - Option to add signature image to topic
			// (a.walling 2009-10-28 12:31) - PLID 35989 - If not on a detail or topic, allow them to insert to the Signature topic (or last topic)
			// (a.walling 2010-03-26 12:44) - PLID 29099 - ignore embedded narrative details
			mnu.InsertMenu(n++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
			if((bIsDetail || bIsTopic) && !bIsMoreInfoTopic && !bIsEmbeddedDetail) {
				mnu.InsertMenu(n++, MF_BYPOSITION|dwDisabled, ContextMenuInfo::miAddSignature, "Add &Signature to Topic");
				mnu.InsertMenu(n++, MF_BYPOSITION|dwDisabled, ContextMenuInfo::miAddOtherUsersSignature, "Add Another &User's Signature to Topic");
			} else if (!bIsEmbeddedDetail) {
				bool bIsSignatureTopic = false;
				CEMRTopic* pSignatureTopic = FindAppropriateSignatureTopic(&bIsSignatureTopic);
				if (pSignatureTopic) {
					if (bIsSignatureTopic) {
						mnu.InsertMenu(n++, MF_BYPOSITION|dwDisabled, ContextMenuInfo::miAddSignature, FormatString("Add &Signature to '%s' Topic", ConvertToControlText(pSignatureTopic->GetName())));
						mnu.InsertMenu(n++, MF_BYPOSITION|dwDisabled, ContextMenuInfo::miAddOtherUsersSignature, FormatString("Add Another &User's Signature to '%s' Topic", ConvertToControlText(pSignatureTopic->GetName())));
					} else {
						mnu.InsertMenu(n++, MF_BYPOSITION|dwDisabled, ContextMenuInfo::miAddSignature, FormatString("Add &Signature to Last Topic ('%s')", ConvertToControlText(pSignatureTopic->GetName())));
						mnu.InsertMenu(n++, MF_BYPOSITION|dwDisabled, ContextMenuInfo::miAddOtherUsersSignature, FormatString("Add Another &User's Signature to Last Topic ('%s')", ConvertToControlText(pSignatureTopic->GetName())));
					}
				}
			}

			// (a.walling 2010-03-26 12:44) - PLID 29099 - ignore embedded narrative details
			if((bIsDetail || bIsTopic) && !bIsMoreInfoTopic && !bIsEmbeddedDetail) {
				// (a.walling 2009-10-29 09:36) - PLID 36089 - Add a text macro to the topic
				mnu.InsertMenu(n++, MF_BYPOSITION|dwDisabled, ContextMenuInfo::miAddTextMacro, "Add Text &Macro to Topic");
			}

			// (c.haag 2009-09-10 15:04) - PLID 35077 - EMR problem options. This applies to all types
			if (!bIsMoreInfoTopic)
			{
				mnu.InsertMenu(n++, MF_BYPOSITION|MF_SEPARATOR, 0, "");		
				if (bIsTopic) {
					// If we get here, the user right-clicked on a topic. Add topic-specific EMR problem editing items.
					CEMRTopic *pTopic = GetTopicFromURL(strUrl);
					if (NULL != pTopic) {
						mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miLinkWithNewProblem, "Link Topic with New Problem");
						mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miLinkWithExistingProblems, "Link Topic with Existing Problems");
						if (pTopic->HasProblems()) {
							mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miUpdateProblemInformation, "Update Topic Problem Information");
						}
					}
				}
				else if (bIsDetail || bIsEmbeddedDetail) {
					// (a.walling 2010-03-26 12:44) - PLID 29099 - Handle embedded narrative details here
					// If we get here, the user right-clicked on a detail. Add detail-specific EMR problem editing items.
					CEMNDetail *pDetail = GetDetailFromURL(strUrl);
					if (NULL != pDetail) {
						mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miLinkWithNewProblem, "Link Detail with New Problem");
						mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miLinkWithExistingProblems, "Link Detail with Existing Problems");
						if (pDetail->HasProblems()) {
							mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miUpdateProblemInformation, "Update Detail Problem Information");
						}
					}
				}
				else {
					// If we get here, the user right-clicked on something that is neither a detail nor a topic. We treat 
					// this as an EMN-level action. Add EMN-specific EMR problem editing items.
					CEMN* pEMN = GetCurrentEMN();
					if (NULL != pEMN) {
						mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miLinkWithNewProblem, "Link EMN with New Problem");
						mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miLinkWithExistingProblems, "Link EMN with Existing Problems");
						if (pEMN->HasProblems()) {
							mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miUpdateProblemInformation, "Update EMN Problem Information");
						}
					}
				}
			}
			else {
				// More info topic. EMR problem items don't apply to it, so don't try to add menu items for editing them.
			}

			// (a.walling 2008-10-22 12:17) - PLID 31795 - Open the 'Find' dialog
			mnu.InsertMenu(n++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
			mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miFind, "&Find...");
		#ifdef _DEBUG
			// (a.walling 2008-08-14 09:42) - PLID 30570 - Removed accelerators on these to lower the possibility of conflicts
			// for new menu items. Besides, I don't think anyone uses them, and these are debug-only.
			mnu.InsertMenu(n++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
			mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miViewSource, "View Source");
			// (a.walling 2007-10-18 14:27) - PLID 25548 - To help debug, you can view the generated source.
			mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miViewGeneratedSource, "View Generated Source");
			mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miProperties, "Properties");
		#endif
			mnu.InsertMenu(n++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
			// (a.walling 2008-10-14 10:57) - PLID 31678 - Configure preview option
			mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miConfigure, "&Configure Preview...");
			mnu.InsertMenu(n++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
			mnu.InsertMenu(n++, MF_BYPOSITION, ContextMenuInfo::miForceRefresh, "Re&fresh");

			//now, default an entry if not -1
			if(nIndexToDefault != -1)
				mnu.SetDefaultItem(nIndexToDefault, TRUE);
		}

		// Show shortcut menu
		long nSelection = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_VERPOSANIMATION,
			contextMenuInfo.pt.x, contextMenuInfo.pt.y, this, NULL);

		if (nSelection > 0) {
			// (a.walling 2009-11-23 11:50) - PLID 24194 - Use the method to get the IOleCommandTarget
			IOleCommandTargetPtr pCmdTarg = GetOleCommandTarget();
			if(pCmdTarg) {
				switch(nSelection) {
					// (j.jones 2007-07-06 10:46) - PLID 25457 - added popup, go to topic, and edit options
					case ContextMenuInfo::miPopup:
						{
							if(bIsDetail) {
								//send a special command to popup the detail
								strUrl.Replace("nexemr://detail","nexemr://popupdetail");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							}
							else if (bIsEmbeddedDetail) {
								// (a.walling 2010-03-26 12:44) - PLID 29099 - simply navigate with our given url
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							}
							else {
								//shouldn't be allowed
								ASSERT(FALSE);
							}
						} break;
					case ContextMenuInfo::miGoToTopic:
					// (z.manning 2009-08-26 10:04) - PLID 33911 - For the add signature option, go to the
					// topic first so we know it's been loaded and it's safe to add the new item.
					case ContextMenuInfo::miAddSignature:
					case ContextMenuInfo::miAddOtherUsersSignature:	// (j.jones 2013-08-07 15:19) - PLID 42958
					case ContextMenuInfo::miAddTextMacro: // (a.walling 2009-10-29 09:36) - PLID 36089 - Support text macros as well
						{
							CEMRTopic *pTopic = NULL;
							if(bIsTopic) {
								pTopic = GetTopicFromURL(strUrl);
								//simply navigate with our given url
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							}
							else if (bIsDetail) {
								CEMNDetail *pDetail = GetDetailFromURL(strUrl);
								if(pDetail != NULL) {
									pTopic = pDetail->m_pParentTopic;
								}
								//send a special command to open the detail's topic
								strUrl.Replace("nexemr://detail","nexemr://topicDT");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							}
							else if (bIsEmbeddedDetail) {					
								// (a.walling 2010-03-26 12:44) - PLID 29099 - Need to figure out the appropriate parent topic and generate our URL
								CEMNDetail *pDetail = GetDetailFromURL(strUrl);
								if(pDetail != NULL) {
									pTopic = pDetail->m_pParentTopic;
									
									if (pTopic) {
										//send a special command to open the detail's topic
										
										long nTopicID = pTopic->GetID();

										BOOL bUnsaved = (nTopicID == -1); // an unsaved topic will have an id of -1!
										if (bUnsaved) {
											nTopicID = reinterpret_cast<long>(this); // cast ourself (CEMRTopic*) to a long pointer.
										}

										CString strPointer = bUnsaved ? "PT" : "ID"; // PT for pointer, ID for ID

										strUrl.Format("nexemr://topic%s/?%li", strPointer, nTopicID);
										_variant_t varUrl;
										varUrl = _bstr_t(strUrl);
										m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);		
									}
								}
							}
							// (j.jones 2013-08-07 15:20) - PLID 42958 - added ability to let another user sign an EMN
							else if (nSelection == ContextMenuInfo::miAddSignature
								|| nSelection == ContextMenuInfo::miAddOtherUsersSignature) {

								// (a.walling 2009-10-28 14:04) - PLID 35989 - Find the best topic for a new signature (either a *signature* topic, or the very last topic on an EMN)
								pTopic = FindAppropriateSignatureTopic(NULL);

								if (pTopic) {
									long nSigTopicID = reinterpret_cast<long>(pTopic);
									strUrl.Format("nexemr://topicPT/?%li", nSigTopicID);
									_variant_t varUrl;
									varUrl = _bstr_t(strUrl);
									m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
								}
							}

							// (z.manning 2009-08-26 10:06) - PLID 33911 - If we want to add a signature item, let's
							// do that now.
							if(pTopic != NULL && pTopic->GetParentEMN() != NULL) {
								if(pTopic->GetParentEMN()->GetInterface() != NULL) {
									// (z.manning 2009-08-26 10:34) - PLID 33911 - Send the message to CEmrTreeWnd so we
									// can ensure the topic wnd exists first
									// (j.jones 2013-08-07 15:33) - PLID 42958 - converted the NXM_INSERT_STOCK_EMR_ITEM
									// message to use the StockEMRItem enumeration
									if (nSelection == ContextMenuInfo::miAddSignature) {
										pTopic->GetParentEMN()->GetInterface()->SendMessage(NXM_INSERT_STOCK_EMR_ITEM, (WPARAM)pTopic, (LPARAM)seiSignatureImage);
									}
									// (j.jones 2013-08-07 15:20) - PLID 42958 - added ability to let another user sign an EMN
									else if (nSelection == ContextMenuInfo::miAddOtherUsersSignature) {
										pTopic->GetParentEMN()->GetInterface()->SendMessage(NXM_INSERT_STOCK_EMR_ITEM, (WPARAM)pTopic, (LPARAM)seiAnotherUsersSignatureImage);
									}
									else if (nSelection == ContextMenuInfo::miAddTextMacro) {
										// (a.walling 2009-10-29 09:36) - PLID 36089 - Create a text macro
										pTopic->GetParentEMN()->GetInterface()->SendMessage(NXM_INSERT_STOCK_EMR_ITEM, (WPARAM)pTopic, (LPARAM)seiTextMacro);
									}
								}
							}

						} break;
					case ContextMenuInfo::miEdit:
						{
							//(e.lally 2010-05-03) PLID 15155 - Check for Writable EMN state instead of Edit Mode now
							if(bIsDetail && IsEmnWritable()) {
								//send a special command to edit the detail
								strUrl.Replace("nexemr://detail","nexemr://editdetail");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							//(e.lally 2010-05-03) PLID 15155 - Check for Writable EMN state instead of Edit Mode now
							} else if (bIsEmbeddedDetail && IsEmnWritable()) {
								// (a.walling 2010-03-26 12:44) - PLID 29099 - Need to figure out the appropriate detail and generate our URL
								CEMNDetail *pDetail = GetDetailFromURL(strUrl);
								//send a special command to edit the detail
								
								long nDetailID = pDetail->GetID();

								BOOL bUnsaved = (nDetailID == -1); // an unsaved topic will have an id of -1!
								if (bUnsaved) {
									nDetailID = reinterpret_cast<long>(this); // cast ourself (CEMRTopic*) to a long pointer.
								}

								CString strPointer = bUnsaved ? "PT" : "ID"; // PT for pointer, ID for ID

								strUrl.Format("nexemr://editdetail%s/?%li", strPointer, nDetailID);

								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							}
							else {
								//shouldn't be allowed
								ASSERT(FALSE);
							}							
						} break;
					case ContextMenuInfo::miFind:  // (a.walling 2008-10-22 12:17) - PLID 31795 - Open the 'Find' dialog
						{
							pCmdTarg->Exec(&CGID_MSHTML, 
									IDM_FIND, 
									OLECMDEXECOPT_DODEFAULT, 
									NULL,
									NULL);
						} break;
					case ContextMenuInfo::miPrint: 
						{
							// (a.walling 2012-03-07 08:36) - PLID 48680 - Moved to previewctrl
							Print();
						} break;
					case ContextMenuInfo::miPrintPreview:
						{
							// (a.walling 2012-03-07 08:36) - PLID 48680 - Moved to previewctrl
							PrintPreview();
						} break;				
					case ContextMenuInfo::miPrintMultiple: // (a.walling 2009-11-24 09:44) - PLID 36418
						{
							PrintMultipleEMNs();
						} break;
					case ContextMenuInfo::miPopupOther:
						{
							PopupAllPatientEMNs(); // (a.walling 2010-11-12 11:18) - PLID 38135 - Popup preview with all patient EMNs
						} break;
					case ContextMenuInfo::miFax:
						{
							if(!m_bInteractive){
								//(e.lally 2009-10-26) PLID 32503 - Fax from outside the interactive editor
								//Load the EMN in memory from our EMN ID
								long nEmnID = GetDisplayedEMNID();
								//Create with a NULL parent EMR so that the parent EMR loads automatically below.
								CEMN* pEMN = new CEMN(NULL);
								ASSERT(pEMN);
								pEMN->LoadFromEmnID(nEmnID);
								
								//(e.lally 2009-10-26) PLID 32503 - Now we can try to fax our preview
								FaxEMNPreview(pEMN);

								//Whether the user actually sent the fax or not, 
								//we have to cleanup our memory
								delete pEMN;
								pEMN = NULL;
							}
							else{
								//(e.lally 2009-10-01) PLID 32503 - Try to fax the preview
								strUrl = "nexemr://faxemnmhtfile?1";
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							}
						} break;
					case ContextMenuInfo::miViewSource:
						{
						// View Source
						pCmdTarg->Exec(&CGID_MSHTML, 
								IDM_VIEWSOURCE, 
								OLECMDEXECOPT_DONTPROMPTUSER, 
								NULL,
								NULL);
						} break;
					case ContextMenuInfo::miViewGeneratedSource:
						{
						// (a.walling 2007-10-18 14:27) - PLID 25548 - To help debug, you can view the generated source.
						// View Generated Source
						strUrl = "nexemr://viewgeneratedsource?1";
						_variant_t varUrl;
						varUrl = _bstr_t(strUrl);
						m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
						} break;
					case ContextMenuInfo::miProperties:
						{
						// Properties
						pCmdTarg->Exec(&CGID_MSHTML, 
								IDM_PROPERTIES, 
								OLECMDEXECOPT_DONTPROMPTUSER, 
								NULL,
								NULL);
						} break;
					case ContextMenuInfo::miCopy:
						{ // (a.walling 2010-03-25 20:34) - PLID 27372
						// Copy
						pCmdTarg->Exec(&CGID_MSHTML, 
								IDM_COPY, 
								OLECMDEXECOPT_DONTPROMPTUSER, 
								NULL,
								NULL);
						} break;
					case ContextMenuInfo::miForceRefresh:
						{
						// Refresh
						strUrl = "nexemr://forcerefresh?1";
						_variant_t varUrl;
						varUrl = _bstr_t(strUrl);
						m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
						} break;
					case ContextMenuInfo::miHideTitle:
						{ // (a.walling 2008-07-01 10:24) - PLID 30570
						// Hide Title
							if (bIsDetail) {
								strUrl.Replace("nexemr://detail","nexemr://detailhidetitle");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							} else if (bIsTopic) {
								strUrl.Replace("nexemr://topic","nexemr://topichidetitle");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							}
						} break;
					case ContextMenuInfo::miHideItem:
						{ // (a.walling 2008-07-01 10:24) - PLID 30570
						// Hide Item
							if (bIsDetail) {
								strUrl.Replace("nexemr://detail","nexemr://detailhideitem");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							} else if (bIsTopic) {
								strUrl.Replace("nexemr://topic","nexemr://topichideitem");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							}
						} break;
					case ContextMenuInfo::miSubDetail:
						{ // (a.walling 2008-10-23 10:38) - PLID 31808
						// Toggle the subdetail display
							ASSERT(bIsDetail);
							strUrl.Replace("nexemr://detail","nexemr://detailtogglesubdetail");
							_variant_t varUrl;
							varUrl = _bstr_t(strUrl);
							m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
						} break;
					case ContextMenuInfo::miHideIfIndirectlyOnNarrative:
						{ // (a.walling 2012-07-13 16:38) - PLID 48896
						// Toggle the hide indirect
							ASSERT(bIsDetail);
							strUrl.Replace("nexemr://detail","nexemr://detailtogglehideifindirect");
							_variant_t varUrl;
							varUrl = _bstr_t(strUrl);
							m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
						} break;
					case ContextMenuInfo::miHideItemOnIPad:
						{
							// (j.armen 2013-01-16 16:42) - PLID 54412
							ASSERT(bIsDetail);
							strUrl.Replace("nexemr://detail","nexemr://detailtogglehideitemonipad");
							_variant_t varUrl;
							varUrl = _bstr_t(strUrl);
							m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
						} break;
					case ContextMenuInfo::miConfigure:
						{
							// (a.walling 2008-10-14 10:22) - PLID 31678 - Configure preview option 
							// (a.walling 2008-11-14 08:38) - PLID 32024 - Warn about limitations when not interactive
							if (!m_bInteractive) {
								MessageBox("Only header and footer formatting can be applied to a saved preview file. To apply any other changes to the EMR Preview configuration, the EMR or EMN must be opened in order to re-generate the preview.", "EMR Preview Configuration", MB_ICONINFORMATION);
							}
							ConfigurePreview();
						} break;						
					case ContextMenuInfo::miColumnOne:
						{ // (a.walling 2009-01-07 15:15) - PLID 32695 - Floating elements support
						// Float Left						
							// (a.walling 2009-07-06 10:26) - PLID 34793 - floatleft/floatright are now colone/coltwo
							if (bIsDetail) {
								strUrl.Replace("nexemr://detail","nexemr://detailcolone");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							} else if (bIsTopic) {
								strUrl.Replace("nexemr://topic","nexemr://topiccolone");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							}
						} break;					
					case ContextMenuInfo::miColumnTwo:
						{ // (a.walling 2009-01-07 15:15) - PLID 32695 - Floating elements support
						// Float Right					
							// (a.walling 2009-07-06 10:26) - PLID 34793 - floatleft/floatright are now colone/coltwo
							if (bIsDetail) {
								strUrl.Replace("nexemr://detail","nexemr://detailcoltwo");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							} else if (bIsTopic) {
								strUrl.Replace("nexemr://topic","nexemr://topiccoltwo");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							}
						} break;							
					case ContextMenuInfo::miPageBreakBefore:
						{
							// (a.walling 2010-08-31 18:20) - PLID 36148 - Page breaks
							if (bIsDetail) {
								strUrl.Replace("nexemr://detail","nexemr://detailpagebreakbefore");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							} else if (bIsTopic) {
								strUrl.Replace("nexemr://topic","nexemr://topicpagebreakbefore");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							}
						} break;								
					case ContextMenuInfo::miPageBreakAfter:
						{
							// (a.walling 2010-08-31 18:20) - PLID 36148 - Page breaks
							if (bIsDetail) {
								strUrl.Replace("nexemr://detail","nexemr://detailpagebreakafter");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							} else if (bIsTopic) {
								strUrl.Replace("nexemr://topic","nexemr://topicpagebreakafter");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							}
						} break;		
					// (a.walling 2009-07-06 10:30) - PLID 34793 - Clearing is deprecated
					/*
					case ContextMenuInfo::miClearLeft:
						{ // (a.walling 2009-01-07 15:15) - PLID 32695 - Floating elements support
						// Clear Left						
							if (bIsDetail) {
								strUrl.Replace("nexemr://detail","nexemr://detailclearleft");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							} else if (bIsTopic) {
								strUrl.Replace("nexemr://topic","nexemr://topicclearleft");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							}
						} break;
					case ContextMenuInfo::miClearRight:
						{ // (a.walling 2009-01-07 15:15) - PLID 32695 - Floating elements support
						// Clear Right						
							if (bIsDetail) {
								strUrl.Replace("nexemr://detail","nexemr://detailclearright");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							} else if (bIsTopic) {
								strUrl.Replace("nexemr://topic","nexemr://topicclearright");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							}
						} break;
					*/
					case ContextMenuInfo::miGroupBegin:
						{ // (a.walling 2009-07-06 12:28) - PLID 34793 - Group at beginning
							if (bIsDetail) {
								ASSERT(FALSE);
							} else if (bIsTopic) {
								strUrl.Replace("nexemr://topic","nexemr://topicgroupbegin");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							}
						} break;
					case ContextMenuInfo::miGroupEnd:
						{ // (a.walling 2009-07-06 12:28) - PLID 34793 - Group at end
							if (bIsDetail) {
								ASSERT(FALSE);
							} else if (bIsTopic) {
								strUrl.Replace("nexemr://topic","nexemr://topicgroupend");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							}
						} break;
					case ContextMenuInfo::miTextRight:
						{ // (a.walling 2009-01-08 14:07) - PLID 32660 - Align text right
						// Text Right						
							if (bIsDetail) {
								strUrl.Replace("nexemr://detail","nexemr://detailtextright");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							} else if (bIsTopic) {
								strUrl.Replace("nexemr://topic","nexemr://topictextright");
								_variant_t varUrl;
								varUrl = _bstr_t(strUrl);
								m_pBrowser->Navigate2(&varUrl, NULL, NULL, NULL, NULL);
							}
						} break;
						
					// (c.haag 2009-09-10 15:10) - PLID 35077 - We can now edit problems from the pane
					case ContextMenuInfo::miLinkWithNewProblem:
					case ContextMenuInfo::miLinkWithExistingProblems:
					case ContextMenuInfo::miUpdateProblemInformation:
						try
						{
							// (a.walling 2010-03-26 12:44) - PLID 29099 - Handle embedded details; just by modifying some conditionals
							// Get the parent tree and determine whether we're editing EMN problems
							CEmrTreeWnd* pTreeWnd = NULL;
							if (NULL != GetCurrentEMN()) {
								pTreeWnd = GetCurrentEMN()->GetInterface();
							}
							BOOL bIsEMN = (!bIsTopic && !bIsDetail && !bIsEmbeddedDetail) ? TRUE : FALSE;

							// If we're at the EMN level, handle EMN problem commands
							if (bIsEMN) {
								if (pTreeWnd) {
									// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
									if (ContextMenuInfo::miLinkWithNewProblem == nSelection) {
										pTreeWnd->PostMessage(WM_COMMAND, IDM_ADD_EMN_PROBLEM);
									}
									else if (ContextMenuInfo::miLinkWithExistingProblems == nSelection) {
										pTreeWnd->PostMessage(WM_COMMAND, IDM_LINK_EMN_PROBLEMS);
									}
									else if (ContextMenuInfo::miUpdateProblemInformation == nSelection) {
										pTreeWnd->EditEmnProblem(GetCurrentEMN());
									}
								}
							}
							// If we get here, we're editing problems for a topic or a detail
							else {
								CEMRTopic* pTopic = NULL;
								CEMNDetail *pDetail = NULL;

								// Get the proper interface windows. The code to actually edit
								// the problems is contained in them, so we will defer.
								if (bIsTopic) {
									pDetail = NULL;
									pTopic = GetTopicFromURL(strUrl);
								}
								else if (bIsDetail || bIsEmbeddedDetail) {
									pDetail = GetDetailFromURL(strUrl);
									pTopic = pDetail->m_pParentTopic;
								}

								if (NULL != pTopic) {
									// We have the topic. Navigate to it to ensure it's fully loaded and that the 
									// interface for it, and the detail if necessary, exists.
									NavigateToTopic(AsString((long)pTopic), "topicPT");
									CEmrItemAdvDlg* pDetailWnd = (NULL == pDetail) ? NULL : pDetail->m_pEmrItemAdvDlg;
									
									// If this is a topic (and we have a valid tree window), then post the problem edit
									// command to the tree as if the user had done it from the tree's right-click menu.
									if (bIsTopic && pTreeWnd) {
										if (ContextMenuInfo::miLinkWithNewProblem == nSelection) {
											pTreeWnd->PostMessage(WM_COMMAND, IDM_ADD_TOPIC_PROBLEM);
										}
										else if (ContextMenuInfo::miLinkWithExistingProblems == nSelection) {
											pTreeWnd->PostMessage(WM_COMMAND, IDM_LINK_TOPIC_PROBLEMS);
										}
										else if (ContextMenuInfo::miUpdateProblemInformation == nSelection) {
											pTreeWnd->PostMessage(WM_COMMAND, IDM_EDIT_TOPIC_PROBLEM);
										}
									}
									// If this is a detail (and we have a valid detail window), then defer to the detail
									// window's problem editing functions.
									else if ((bIsDetail || bIsEmbeddedDetail) && pDetailWnd) {
										if (ContextMenuInfo::miLinkWithNewProblem == nSelection) {
											pDetailWnd->NewProblem(eprtEmrItem);
										}
										else if (ContextMenuInfo::miLinkWithExistingProblems == nSelection) {
											pDetailWnd->LinkProblems(eprtEmrItem);
										}
										else if (ContextMenuInfo::miUpdateProblemInformation == nSelection) {
											pDetailWnd->EditProblem(eprtEmrItem);
										}
									}
									else {
										// Not sure how we could get here, but ignore the command because
										// we must be missing a valid object somewhere
									}

								} // if (NULL != pTopic) {
								else 
								{
									// Not sure how we could get here, but ignore the command because we
									// don't have access to the topic (or the topic containing the detail)
								}
							}
						} 
						NxCatchAll("Error processing EMR problem command from the Preview Pane");
						break;
				}
			}
		}
	} NxCatchAll(__FUNCTION__);
}


////////////////////////////////////////////////////////////////



namespace NxPrintTemplate {

// (a.walling 2012-05-08 16:35) - PLID 50241 - Converts to HTML and sets
DocInfo& DocInfo::SetHeaderText(LPCTSTR szText)
{
	return SetHeaderInnerHTML(_Q(szText));
	return *this;
}

DocInfo& DocInfo::SetFooterText(LPCTSTR szText)
{
	return SetFooterInnerHTML(_Q(szText));
	return *this;
}

// (a.walling 2012-05-08 16:35) - PLID 50241 - Sets direct - no conversion
DocInfo& DocInfo::SetHeaderInnerHTML(LPCTSTR szHtml)
{
	strHeaderHTML = WrapHeaderFooter(szHtml, "headerstyle", "nxheader");
	return *this;
}

DocInfo& DocInfo::SetFooterInnerHTML(LPCTSTR szHtml)
{
	strFooterHTML = WrapHeaderFooter(szHtml, "footerstyle", "nxfooter");
	return *this;
}

// (a.walling 2012-05-08 16:35) - PLID 50241 - Wrap HTML with the necessary elements
CString DocInfo::WrapHeaderFooter(LPCTSTR szHtml, LPCTSTR szClass, LPCTSTR szID)
{
	CString str;
	str.Format(
		"<DIV CLASS='%s' ID='%s'><TABLE STYLE='width:100%%;'><TR><TD STYLE='text-align:center;' COLSPAN=2>"
		"%s"
		"</TD></TR></TABLE></DIV>"
		, szClass
		, szID
		, szHtml
	);

	return str;
}

CString DocInfo::GetDocumentDefinition(long nIndex /*=0*/)
{
	CString strCanonDocumentPath = strDocumentPath;

	if (strCanonDocumentPath != "document") {
		CString strNewPathBuffer;

		DWORD dwChars = INTERNET_MAX_URL_LENGTH;
		HR(UrlCreateFromPath(strDocumentPath, strNewPathBuffer.GetBuffer(INTERNET_MAX_URL_LENGTH + 1), &dwChars, NULL));
		strNewPathBuffer.ReleaseBuffer();

		strCanonDocumentPath = strNewPathBuffer;
	}

	if (strHeaderHTML.IsEmpty()) {
		SetHeaderText("");
	}
	if (strFooterHTML.IsEmpty()) {
		SetFooterText("");
	}

	CString strHeaderHTMLEscaped = strHeaderHTML;
	CString strFooterHTMLEscaped = strFooterHTML;

	strHeaderHTMLEscaped.Replace("\"", "\\\"");
	strFooterHTMLEscaped.Replace("\"", "\\\"");

	CString strDocumentDefinition;
	strDocumentDefinition.Format(
		"\t\tallDocuments[%li] = new Object();\r\n"
		"\t\tallDocuments[%li].source = \"%s\";\r\n"
		"\t\tallDocuments[%li].header = \"%s\";\r\n"
		"\t\tallDocuments[%li].footer = \"%s\";\r\n",
		nIndex,
		nIndex, strCanonDocumentPath,
		nIndex, strHeaderHTMLEscaped,
		nIndex, strFooterHTMLEscaped);

	return strDocumentDefinition;
}

CString DocInfo::PreparePrintTemplate()
{
	CString strDefinition;
	strDefinition.Format("\t\tallDocuments = new Array(%li);\r\n", 1);
	strDefinition += GetDocumentDefinition();

	return PreparePrintTemplate(strDefinition);
}

// (r.gonet 06/13/2013) - PLID 56850 - Added the bDisplayPerDocumentPageCount to control whether or not we reset the page count each document.
CString DocInfo::PreparePrintTemplate(const CString& strDocumentDefinition, bool bDisplayPerDocumentPageCount/*=true*/)
{
	try {
		ASSERT(IDR_PNG_PRINT == 1057); // resource references in print template
		HRSRC rcResource = FindResource(NULL, MAKEINTRESOURCE(IDR_HTML_PRINTTEMPLATE), RT_HTML);
		if (rcResource) {
			HGLOBAL hgResource = LoadResource(NULL, rcResource); // not really an HGLOBAL, wierd.

			if (hgResource) {
				DWORD dwSize = SizeofResource(NULL, rcResource);

				if (dwSize > 0) {
					LPVOID pData = LockResource(hgResource);

					if (pData) {
						CString strFile;
						LPTSTR buf = strFile.GetBuffer(dwSize + 1);

						memcpy(buf, pData, dwSize);
						buf[dwSize] = '\0';

						strFile.ReleaseBuffer();

						FreeResource(hgResource);

						// (a.walling 2009-11-30 16:40) - PLID 36395 - You can comment in this to use an external file for the template instead
						/*
						strFile.Empty();

						CFile fIn("C:\\temp\\NxPrintTemplate.html", CFile::modeRead | CFile::shareCompat);
						long nFileLength = (long)fIn.GetLength();
						LPTSTR szFile = strFile.GetBuffer(nFileLength + 2);
						fIn.Read(szFile, nFileLength);
						szFile[nFileLength + 1] = '\0';
						strFile.ReleaseBuffer();
						*/

						// (a.walling 2009-11-23 11:46) - PLID 36395 - Define the document sources for the print template
						//CString strDocumentDefinition = PrepareDocumentDefinition(listEMNs);
						
						strFile.Replace("//{{NXDOCUMENTDEFINTION}}//", strDocumentDefinition);
						// (r.gonet 06/13/2013) - PLID 56850 - The displayPerDocumentPageCount is a variable on the template, it expects to be set in the initialization javascript function.
						strFile.Replace("//{{NX_SET_DISPLAYPERDOCUMENTPAGECOUNT}}//", FormatString("displayPerDocumentPageCount = %s;", bDisplayPerDocumentPageCount ? "true" : "false"));

						// (a.walling 2008-10-09 14:42) - PLID 31430 - Sometimes we might be PracticeMain, PracticeInternal, PracticeTest, who knows. Make sure the appropriate module name is used to refer to us.
						// (a.walling 2012-04-11 15:39) - PLID 49594 - Replace ://PRACTICE.EXE with ://PracticeMain.exe etc if we are not actually running as practice.exe
						// (a.walling 2012-04-25 17:49) - PLID 49996 - Now we can just use an nxres URL eg nxres://0/[type/]resource
						//strFile = FixupPracticeResModuleName(strFile);

						CString strTemplatePathName;
						HANDLE hFile = CreateNxTempFile("nexemrt_EMNPrintTemplate", "html", &strTemplatePathName, TRUE);

						if (hFile != INVALID_HANDLE_VALUE) {
							DWORD dwWritten = 0;
							::WriteFile(hFile, (LPCTSTR)strFile, strFile.GetLength(), &dwWritten, NULL);
							::CloseHandle(hFile);
							hFile = INVALID_HANDLE_VALUE;

							// mainfrm will delete when CleanupEMRTempFiles is called.

							return strTemplatePathName;
						} else {
							ThrowNxException("Could not create print template file: 0x%08x", GetLastError());
						}
					}
				}
			}
		}
	} NxCatchAll("Error preparing print template");

	ASSERT(FALSE);
	return "";
}

} // namespace NxPrintTemplate