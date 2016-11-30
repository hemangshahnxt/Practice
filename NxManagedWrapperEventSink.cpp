// (c.haag 2010-06-22 16:57) - PLID 39295 - Moved class into its own source/header pair
#include "stdafx.h"
#include <afxctl.h>
#include "NxManagedWrapperEventSink.h"
#include "Filter.h"
#include "Groups.h"
#include "NexPhotoDlg.h"

#import "NxManagedWrapper.tlb"

IMPLEMENT_DYNCREATE(CNxManagedWrapperEventSink, CCmdTarget)

// (c.haag 2010-03-30 17:13) - PLID 36327 - We now support handling events fired from the NxManagedWrapper.
// At present, we only have OnGoToPatient.
CNxManagedWrapperEventSink::CNxManagedWrapperEventSink()
{
	EnableAutomation();
	m_pNxPhotoTabSink = NULL;
	m_dwNxPhotoTabCookie = 0;
	m_pNxPhotoImportFormSink = NULL;
	m_dwNxPhotoImportFormCookie = 0;
}

CNxManagedWrapperEventSink::~CNxManagedWrapperEventSink()
{
}

void CNxManagedWrapperEventSink::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.
	CCmdTarget::OnFinalRelease();
}

BEGIN_MESSAGE_MAP(CNxManagedWrapperEventSink, CCmdTarget)
	//{{AFX_MSG_MAP(CNxManagedWrapperEventSink)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CNxManagedWrapperEventSink, CCmdTarget)
	//{{AFX_DISPATCH_MAP(CNxManagedWrapperEventSink)
	DISP_FUNCTION_ID(CNxManagedWrapperEventSink, "GoToPatient", 0x0001, PhotoTab_OnGoToPatient, VT_EMPTY, VTS_I4 VTS_PBOOL)
	DISP_FUNCTION_ID(CNxManagedWrapperEventSink, "OnConvertFilterStringToClause", 0x0002, PhotoTab_OnConvertFilterStringToClause, VT_EMPTY, VTS_I4 VTS_BSTR VTS_PBOOL VTS_PBSTR VTS_PBSTR)
	DISP_FUNCTION_ID(CNxManagedWrapperEventSink, "OnShowImportForm", 0x0003, PhotoTab_OnShowImportForm, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION_ID(CNxManagedWrapperEventSink, "VisibilityChanged", 0x0004, PhotoImportForm_OnVisibilityChanged, VT_EMPTY, VTS_BOOL)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

BEGIN_INTERFACE_MAP(CNxManagedWrapperEventSink, CCmdTarget)
	INTERFACE_PART(CNxManagedWrapperEventSink, __uuidof(NxManagedWrapperLib::_INxPhotoTabEvents), Dispatch)
	INTERFACE_PART(CNxManagedWrapperEventSink, __uuidof(NxManagedWrapperLib::_INxPhotoImportFormEvents), Dispatch)
END_INTERFACE_MAP()

// (c.haag 2010-03-30 17:13) - PLID 36327 - Attempts to establish a connection with the photo tab object in the managed wrapper
void CNxManagedWrapperEventSink::EnsureSink(NxManagedWrapperLib::INxPhotoTabPtr pNxPhotoTab)
{
	if (NULL == pNxPhotoTab) {
		ThrowNxException("Called CNxManagedWrapperEventSink::EnsureSink with a NULL photo tab");
	}

	// If we already have a sink, don't do anything
	if (m_dwNxPhotoTabCookie)
		return;

	//Get a pointer to sinks IUnknown, no AddRef. CMySink implements only
	//dispinterface and the IUnknown and IDispatch pointers will be same.
	LPUNKNOWN pUnkSink = GetIDispatch(FALSE);
	if (!pUnkSink) {
		ThrowNxException("Call to GetIDispatch failed in CNxManagedWrapperEventSink::EnsureSink");
	}

	//Establish a connection between source and sink.
	//m_dwNxPhotoTabCookie is a cookie identifying the connection, and is needed
	//to terminate the connection
	if (!AfxConnectionAdvise(pNxPhotoTab, __uuidof(NxManagedWrapperLib::_INxPhotoTabEvents), GetIDispatch(FALSE),
		FALSE, &m_dwNxPhotoTabCookie))
	{
		ThrowNxException("Call to AfxConnectionAdvise failed in CNxManagedWrapperEventSink::EnsureSink");
	}
	m_pNxPhotoTabSink = pNxPhotoTab;
	m_pNxPhotoTabSink->AddRef();
}

// (c.haag 2010-06-22 13:37) - PLID 39295 - We now support sinks for import objects
void CNxManagedWrapperEventSink::EnsureSink(NxManagedWrapperLib::INxPhotoImportFormPtr pNxPhotoImportForm)
{
	if (NULL == pNxPhotoImportForm) {
		ThrowNxException("Called CNxManagedWrapperEventSink::EnsureSink with a NULL photo import form");
	}

	// If we already have a sink, don't do anything
	if (m_dwNxPhotoImportFormCookie)
		return;

	//Get a pointer to sinks IUnknown, no AddRef. CMySink implements only
	//dispinterface and the IUnknown and IDispatch pointers will be same.
	LPUNKNOWN pUnkSink = GetIDispatch(FALSE);
	if (!pUnkSink) {
		ThrowNxException("Call to GetIDispatch failed in CNxManagedWrapperEventSink::EnsureSink");
	}

	//Establish a connection between source and sink.
	//m_dwNxPhotoTabCookie is a cookie identifying the connection, and is needed
	//to terminate the connection
	if (!AfxConnectionAdvise(pNxPhotoImportForm, __uuidof(NxManagedWrapperLib::_INxPhotoImportFormEvents), GetIDispatch(FALSE),
		FALSE, &m_dwNxPhotoImportFormCookie))
	{
		ThrowNxException("Call to AfxConnectionAdvise failed in CNxManagedWrapperEventSink::EnsureSink");
	}
	m_pNxPhotoImportFormSink = pNxPhotoImportForm;
	m_pNxPhotoImportFormSink->AddRef();
}

// (c.haag 2010-03-30 17:13) - PLID 36327 - Disconnect the event sink
void CNxManagedWrapperEventSink::CleanUp()
{
	if (m_pNxPhotoTabSink != NULL)
	{
		//Get a pointer to sinks IUnknown, no AddRef.
		LPUNKNOWN pUnkSink = GetIDispatch(FALSE);

		//Terminate a connection between source and sink.
		//m_dwNxPhotoTabCookie is a value obtained through AfxConnectionAdvise().
		AfxConnectionUnadvise(m_pNxPhotoTabSink, __uuidof(NxManagedWrapperLib::_INxPhotoTabEvents), pUnkSink, FALSE, m_dwNxPhotoTabCookie);
		m_pNxPhotoTabSink->Release();
		m_pNxPhotoTabSink = NULL;
	}
	m_dwNxPhotoTabCookie = 0;

	// (c.haag 2010-06-22 13:37) - PLID 39295 - We now support sinks for import objects
	if (m_pNxPhotoImportFormSink != NULL) 
	{
		//Get a pointer to sinks IUnknown, no AddRef.
		LPUNKNOWN pUnkSink = GetIDispatch(FALSE);

		//Terminate a connection between source and sink.
		//m_dwNxPhotoTabCookie is a value obtained through AfxConnectionAdvise().
		AfxConnectionUnadvise(m_pNxPhotoImportFormSink, __uuidof(NxManagedWrapperLib::_INxPhotoImportFormEvents), pUnkSink, FALSE, m_dwNxPhotoImportFormCookie);
		m_pNxPhotoImportFormSink->Release();
		m_pNxPhotoImportFormSink = NULL;
	}
	m_dwNxPhotoImportFormCookie = 0;
}

// (c.haag 2010-03-30 17:13) - PLID 36327 - This event is fired, ultimately, when the user chooses "Go to Patient"
// from the NexPhoto global search.
void CNxManagedWrapperEventSink::PhotoTab_OnGoToPatient(long nPersonID, BOOL* pbSuccess)
{
	try {
		//Set the active patient
		CMainFrame *pMainFrame;
		pMainFrame = GetMainFrame();
		if (pMainFrame != NULL) {

			if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPersonID)) {
				if(IDNO == AfxMessageBox("This patient is not in the current lookup. \n"
					"Do you wish to reset the lookup to include all patients?",MB_ICONQUESTION|MB_YESNO)) {
					// If we get here, do not dismiss the global search
					*pbSuccess = FALSE;
					return;
				}
			}
			
			//TES 1/7/2010 - PLID 36761 - This function may fail now
			if(pMainFrame->m_patToolBar.TrySetActivePatientID(nPersonID)) {

				//Now just flip to the patient's module and set the active Patient
				pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
				CNxTabView *pView = pMainFrame->GetActiveView();
				if(pView)
					pView->UpdateView();

				*pbSuccess = TRUE;
			}
			else {
				// If it does fail, don't dismiss the global search
				*pbSuccess = FALSE;
			}

		}//end if MainFrame
		else {
			MsgBox(MB_ICONSTOP|MB_OK, "ERROR - NexPhotoDlg.cpp: Cannot Open Mainframe");
			*pbSuccess = FALSE;
		}//end else pMainFrame
	}
	NxCatchAllCall(__FUNCTION__, if (pbSuccess) { *pbSuccess = FALSE; } );
}

// (c.haag 2010-04-16 11:57) - PLID 36457 - This event is fired (ultimately from the global search) when NexPhoto
// needs to run a query filtered on a letter writing filter. Practice will return a success flag, the From, and the Where
// clauses such that a filter can be built in the form "PersonT.ID IN (SELECT ID FROM [from] WHERE [where])"
void CNxManagedWrapperEventSink::PhotoTab_OnConvertFilterStringToClause(long nFilterID, LPCTSTR strFilter, BOOL* pbSuccess, BSTR* bstrFrom, BSTR* bstrWhere)
{
	try {
		CString strFrom, strWhere;

		ASSERT(NULL == *bstrFrom);
		ASSERT(NULL == *bstrWhere);

		*pbSuccess = CFilter::ConvertFilterStringToClause(nFilterID, strFilter, fboPerson, &strWhere, &strFrom, NULL, NULL, TRUE) ? TRUE : FALSE;
		*bstrFrom = strFrom.AllocSysString();
		*bstrWhere = strWhere.AllocSysString();
	}
	NxCatchAllCall(__FUNCTION__, if (pbSuccess) { *pbSuccess = FALSE; } );
}

// (c.haag 2010-06-08 11:29) - PLID 38898 - This event is fired when the user requested to show the import form
// from the NexPhoto Tab 
void CNxManagedWrapperEventSink::PhotoTab_OnShowImportForm()
{
	try 
	{
		// (c.haag 2015-07-08) - PLID 65912 - Only log verbose NexPhoto managed operations if this is not commented out
#ifdef LOG_NEXPHOTO_MANAGED_ACTIONS
		Log("(NexPhoto) User requested to view Import window. Attempting to open now.");
#endif
		CMainFrame* pFrame = GetMainFrame();
		if (NULL != pFrame) 
		{
			pFrame->ShowNexPhotoImportForm(TRUE);
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-06-22 13:37) - PLID 39295 - This even is fired when the NexPhoto Import form is displayed or hidden
void CNxManagedWrapperEventSink::PhotoImportForm_OnVisibilityChanged(BOOL bVisible)
{
	try {
		// When the import form is hidden (closed, if you will), we need to update the patients module view
		// because the History and NexPhoto tabs may have changed (and possibly General 1 if thumbs are
		// visible).
		if (!bVisible) {
			CNxView* pView = GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
			if (NULL != pView) {
				pView->UpdateView();
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}
