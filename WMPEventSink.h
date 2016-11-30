#if !defined(AFX_EVENTSINK_H__90378EF0_9512_4C7C_A420_58CDAC9D63F3__INCLUDED_)
#define AFX_EVENTSINK_H__90378EF0_9512_4C7C_A420_58CDAC9D63F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EventSink.h : header file
//
#include <afxtempl.h>

#define NXMEVM_BASE							WM_USER + 0x8000
#define NXMEVM_WMP_PLAY_STATE_CHANGE		NXMEVM_BASE + 0x001

enum ENXWMPPlayState {
	eWMP_PlayState_Undefined = 0,
	eWMP_PlayState_Stopped = 1,
	eWMP_PlayState_Paused = 2,
	eWMP_PlayState_Playing = 3,
	eWMP_PlayState_ScanForward = 4,
	eWMP_PlayState_ScanReverse = 5,
	eWMP_PlayState_Buffering = 6,
	eWMP_PlayState_Waiting = 7,
	eWMP_PlayState_MediaEnded = 8,
	eWMP_PlayState_Transitioning = 9,
	eWMP_PlayState_Ready = 10,
	eWMP_PlayState_Reconnecting = 11
};

/////////////////////////////////////////////////////////////////////////////
// CWMPEventSink command target

class CWMPEventSink : public CCmdTarget
{
	DECLARE_DYNCREATE(CWMPEventSink)
	CWMPEventSink();    // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWMPEventSink)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWMPEventSink();

	// Attempts to establish a connection with Outlook
	HRESULT EnsureSink(CWnd* pNotify, LPDISPATCH pWMP);

protected:
	LPDISPATCH m_pWMP; // Dispatch to the Windows Media Player
	CWnd* m_pNotify; // The notification window where messages are posted when events occur
	DWORD m_dwCookie; // The cookie we have a sync for

public:
	// Disconnects from the source
	HRESULT CleanUp();

protected:
	// Generated message map functions
	//{{AFX_MSG(CWMPEventSink)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CWMPEventSink)
	afx_msg void OnPlayStateChange(long NewState);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EVENTSINK_H__90378EF0_9512_4C7C_A420_58CDAC9D63F3__INCLUDED_)
