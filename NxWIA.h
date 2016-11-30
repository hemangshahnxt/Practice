// NxWIA.h: WIA Utilities
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NXWIA_H__37E860E7_604E_400B_86DA_ECF331452A46__INCLUDED_)
#define AFX_NXWIA_H__37E860E7_604E_400B_86DA_ECF331452A46__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (push)
#pragma warning (disable : 4146)
// (a.walling 2014-04-30 15:19) - PLID 61989 - import a typelibrary rather than a dll
#import "WIAAut.tlb"
#pragma warning (pop)

#pragma warning(disable:4786)
namespace NxWIA {
	// (a.walling 2008-10-28 13:05) - PLID 31334 - Registered message for when launched from WIA auto-start
	extern const UINT NXM_ACQUIRE_FROM_WIA;

	// (a.walling 2008-09-10 17:42) - PLID 30389 - Various WIA helper functions
	void LogError(_com_error& e, LPCTSTR szNote);

	// (a.walling 2008-09-23 12:53) - PLID 31486 - Added the cameras only option here too
	BOOL IsWIAAvailable(BOOL bCameraOnly = TRUE); // Is WIA available (are any devices connected?)

	// (a.walling 2008-10-28 13:35) - PLID 31334 - Register/UnRegister for WIA persistent events
	HRESULT RegisterForWIAEvent();
	HRESULT UnRegisterForWIAEvent();

	//BOOL GetLaunchStrings(CString& strLaunch, CString& strIcon);

	// (a.walling 2008-09-18 13:48) - PLID 30389 - Added option for cameras only or any device
	WIA::IDevicePtr GetDefaultDevice(BOOL bCameraOnly = TRUE); // Get the default device
	WIA::IDevicePtr GetDeviceFromID(const CString& strDeviceID); // Get device from UUID
	WIA::IItemsPtr GetSelectedItems(WIA::IDevicePtr pDevice); // Get items selected from device's select items dialog
	WIA::ICommonDialogPtr GetCommonDialog(); // Create and return the Common Dialog


	struct CWIAEvent {
		CString EventID;
		CString DeviceID;
		CString ItemID;
	};

	/////////////////////////////////////////////////////////////////////////////
	// CWIAEventSink command target
	// (a.walling 2008-09-10 17:42) - PLID 31334 - Event sink for WIA events

	class CWIAEventSink : public CCmdTarget
	{
		DECLARE_DYNCREATE(CWIAEventSink)
		CWIAEventSink();    // protected constructor used by dynamic creation

	// Attributes
	public:

	// Operations
	public:

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CWIAEventSink)
		public:
		virtual void OnFinalRelease();
		//}}AFX_VIRTUAL

	// Implementation
	public:
		virtual ~CWIAEventSink();

		// Attempts to establish a connection with Outlook
		HRESULT EnsureSink(CWnd* pNotify, LPDISPATCH pWMP);

	protected:
		LPDISPATCH m_pWIA; // Dispatch to WIA
		CWnd* m_pNotify; // The notification window where messages are posted when events occur
		DWORD m_dwCookie; // The cookie we have a sync for

	public:
		// Disconnects from the source
		HRESULT CleanUp();

	protected:
		// Generated message map functions
		//{{AFX_MSG(CWIAEventSink)
			// NOTE - the ClassWizard will add and remove member functions here.
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()
		// Generated OLE dispatch map functions
		//{{AFX_DISPATCH(CWIAEventSink)
		void OnEvent(LPCTSTR EventID, LPCTSTR DeviceID, LPCTSTR ItemID);
		//}}AFX_DISPATCH
		DECLARE_DISPATCH_MAP()
		DECLARE_INTERFACE_MAP()
	};
};


#endif // !defined(AFX_NXWIA_H__37E860E7_604E_400B_86DA_ECF331452A46__INCLUDED_)
