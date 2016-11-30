// EventSink.cpp : implementation file
//

#include "stdafx.h"
#include "WMPEventSink.h"
#include <afxctl.h>
// (a.walling 2014-04-30 15:19) - PLID 61989 - import a typelibrary rather than a dll
#import "wmp.tlb"

const IID IID_IWMPEventSink = __uuidof(WMPLib::_WMPOCXEvents);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWMPEventSink

IMPLEMENT_DYNCREATE(CWMPEventSink, CCmdTarget)

CWMPEventSink::CWMPEventSink()
{
	EnableAutomation();
	m_dwCookie = 0;
	m_pNotify = NULL;
}

CWMPEventSink::~CWMPEventSink()
{
	ASSERT(0 == m_dwCookie);
}

void CWMPEventSink::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.
	CCmdTarget::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CWMPEventSink, CCmdTarget)
	//{{AFX_MSG_MAP(CWMPEventSink)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CWMPEventSink, CCmdTarget)
	//{{AFX_DISPATCH_MAP(CWMPEventSink)
	DISP_FUNCTION_ID(CWMPEventSink, "PlayStateChange", 0x13ed, OnPlayStateChange, VT_EMPTY, VTS_I4)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

BEGIN_INTERFACE_MAP(CWMPEventSink, CCmdTarget)
	INTERFACE_PART(CWMPEventSink, IID_IWMPEventSink, Dispatch)
END_INTERFACE_MAP()

HRESULT CWMPEventSink::CleanUp()
{
	if (0 != m_dwCookie) {
		//Get a pointer to sinks IUnknown, no AddRef.
		LPUNKNOWN pUnkSink = GetIDispatch(FALSE);

		//Terminate a connection between source and sink.
		//m_pUnkSrc is IUnknown of server obtained by CoCreateInstance().
		//m_dwCookie is a value obtained through AfxConnectionAdvise().
		AfxConnectionUnadvise(m_pWMP, IID_IWMPEventSink, pUnkSink, FALSE, m_dwCookie);
	}
	m_dwCookie = 0;
	m_pWMP = NULL;
	m_pNotify = NULL;
	return S_OK;
}

HRESULT CWMPEventSink::EnsureSink(CWnd* pNotify, LPDISPATCH pWMP)
{
	// If we already have a sink, don't do anything
	if (0 != m_dwCookie)
		return S_OK;

	// Clean up any existing objects
	CleanUp();

	//Get a pointer to sinks IUnknown, no AddRef. CMySink implements only
	//dispinterface and the IUnknown and IDispatch pointers will be same.
	LPUNKNOWN pUnkSink = GetIDispatch(FALSE);
	if (!pUnkSink) return S_FALSE;

	//Establish a connection between source and sink.
	//m_pUnkSrc is IUnknown of server obtained by CoCreateInstance().
	//m_dwCookie is a cookie identifying the connection, and is needed
	//to terminate the connection.
	if (!AfxConnectionAdvise(pWMP, IID_IWMPEventSink, pUnkSink, FALSE, &m_dwCookie))
	{
		return S_FALSE;
	}

	m_pNotify = pNotify;
	m_pWMP = pWMP;
	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// CWMPEventSink message handlers


void CWMPEventSink::OnPlayStateChange(long nNewState)
{
	if (m_pNotify) {
		m_pNotify->PostMessage(NXMEVM_WMP_PLAY_STATE_CHANGE, nNewState);
	}
}
