// NxWIA.cpp: WIA Utilities
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <afxctl.h>
#include "NxWIA.h"
#include "FileUtils.h"

bool GetModuleFilePathName(IN const HMODULE hModule, OUT CString &strModuleFilePathName);

namespace NxWIA {
	// (a.walling 2008-10-28 13:05) - PLID 31334 - Registered message for when launched from WIA auto-start
	const UINT NXM_ACQUIRE_FROM_WIA = ::RegisterWindowMessage("NXM_ACQUIRE_FROM_WIA_{4DC250BA-2C5E-458b-9E6E-273A475D5334}");

	// (a.walling 2008-09-10 17:43) - PLID 30389 - Is WIA available (are any devices connected?)
	
	// the error 0x80210015 seems to mean no devices connected
	// pretty much all COM errors are ignored and a sentinel value (false or NULL depending) is returned.
	// this is mostly because these functions seem to generate exceptions in unpredictable ways, and
	// in ways where you would think they would just silently fail or return NULL, etc. Since there is
	// a lack of documentation, this works out well enough. We just log them.
	void LogError(_com_error& e, LPCTSTR szNote) {
		CString strErrorText;

		strErrorText.Format("NxWIA::%s Error 0x%08x: %s; Description '%s'", szNote, e.Error(), e.ErrorMessage(), _Q((LPCTSTR)e.Description()));

		LogDetail("%s", strErrorText);
	};

	// (a.walling 2008-09-23 12:53) - PLID 31486 - Added the cameras only option here too
	BOOL IsWIAAvailable(BOOL bCameraOnly) {
		try {
			WIA::IDeviceManagerPtr pManager;
			HRESULT hr = pManager.CreateInstance("WIA.DeviceManager");

			if (SUCCEEDED(hr) && pManager != NULL) {
				try {
					WIA::IDeviceInfosPtr pDeviceInfos = pManager->GetDeviceInfos();

					if (pDeviceInfos) {
						if (pDeviceInfos->Count <= 0) {
							return FALSE;
						} else {
							if (bCameraOnly) {
								for (long i = 1; i <= pDeviceInfos->Count; i++) {
									_variant_t varIndex = _variant_t(i);
									WIA::IDeviceInfoPtr pDevice = pDeviceInfos->Item[&varIndex];
									if (pDevice != NULL && pDevice->Type == WIA::CameraDeviceType) {
										return TRUE;
									}
								}

								// no camera types were found
								return FALSE;
							} else {
								return TRUE;
							}
						}
					}
				} catch (_com_error& e) {
					// (a.walling 2008-09-18 13:42) - PLID 30389 - Log any errors here
					ASSERT(FALSE);
					LogError(e, "IsWiaAvailable");
					return FALSE;
				}
			} else {
				return FALSE;
			}
		} catch (_com_error&) {
			// (a.walling 2008-09-18 13:42) - PLID 30389 - Don't log this, it will log every time there is an issue
			// on systems that lack WIA (specifically windows 2000).
			//LogError(e, "IsWIAAvailable");
			return FALSE;
		}

		return FALSE;
	};

	
	// (a.walling 2008-10-28 13:35) - PLID 31334 - Register/UnRegister for WIA persistent events
	HRESULT RegisterForWIAEvent()
	{
		/*
		CString strLaunch, strIcon;
		if (GetLaunchStrings(strLaunch, strIcon)) {
			WIA::IDeviceManagerPtr pManager;
			HRESULT hr = pManager.CreateInstance("WIA.DeviceManager");

			if (SUCCEEDED(hr) && pManager != NULL) {
				CString strName = "NexTech Practice";
				if (!GetSubRegistryKey().IsEmpty()) {
					strName += " (";
					strName += GetSubRegistryKey();
					strName += ")";
				}
				return pManager->RegisterPersistentEvent(_bstr_t(strLaunch), _bstr_t(strName), _bstr_t("Import pictures into history"), _bstr_t(strIcon), _bstr_t(WIA::wiaEventDeviceConnected), _bstr_t(WIA::wiaAnyDeviceID));
			} else {
				if (SUCCEEDED(hr)) {
					return E_FAIL;
				} else {
					return hr;
				}
			}
		}*/

		return E_NOTIMPL;
	}

	HRESULT UnRegisterForWIAEvent()
	{
		/*
		CString strLaunch, strIcon;
		if (GetLaunchStrings(strLaunch, strIcon)) {
			WIA::IDeviceManagerPtr pManager;
			HRESULT hr = pManager.CreateInstance("WIA.DeviceManager");

			if (SUCCEEDED(hr) && pManager != NULL) {
				
				CString strName = "NexTech Practice";
				if (!GetSubRegistryKey().IsEmpty()) {
					strName += " (";
					strName += GetSubRegistryKey();
					strName += ")";
				}
				return pManager->UnregisterPersistentEvent(_bstr_t(strLaunch), _bstr_t(strName), _bstr_t("Import pictures into history"), _bstr_t(strIcon), _bstr_t(WIA::wiaEventDeviceConnected), _bstr_t(WIA::wiaAnyDeviceID));
			} else {
				if (SUCCEEDED(hr)) {
					return E_FAIL;
				} else {
					return hr;
				}
			}
		}
		*/

		return E_NOTIMPL;
	}

	/*
	BOOL GetLaunchStrings(CString& strLaunch, CString& strIcon)
	{		
		CString strModuleFilePath;
		if (GetModuleFilePathName(NULL, strModuleFilePath)) {
			// we have the EXE
			CString strSubkey = GetSubRegistryKey();
			if (!strSubkey.IsEmpty()) {
				strSubkey = CString(" /r:") + strSubkey;
			}
			CString strWorkingPath = GetPracPath(true);
			CString strModuleWorkingPath = FileUtils::GetFilePath(strModuleFilePath);
			strWorkingPath.TrimRight("\\");
			strModuleWorkingPath.TrimRight("\\");

			if (strWorkingPath.CompareNoCase(strModuleWorkingPath) != 0) {
				strWorkingPath = CString(" /dir:\"") + strWorkingPath + "\"";
			} else {
				strWorkingPath.Empty();
			}

			strLaunch.Format("\"%s /wia: %s%s\"", strModuleFilePath, strSubkey, strWorkingPath);

			strIcon.Format("%s,6", strModuleFilePath);

			return TRUE;
		}

		return FALSE;
	}
	*/

	// (a.walling 2008-09-10 17:43) - PLID 30389 - Get the default device
	// (a.walling 2008-09-18 13:48) - PLID 30389 - Added option for cameras only or any device
	WIA::IDevicePtr GetDefaultDevice(BOOL bCameraOnly) {
		try {
			WIA::ICommonDialogPtr pCommonDialog;
			HRESULT hr = pCommonDialog.CreateInstance("WIA.CommonDialog");
			
			if (SUCCEEDED(hr) && pCommonDialog != NULL) {
				return pCommonDialog->ShowSelectDevice(bCameraOnly ? WIA::CameraDeviceType : WIA::UnspecifiedDeviceType, VARIANT_FALSE, VARIANT_FALSE);
			} else {
				return NULL;
			}
		} catch (_com_error& e) {
			LogError(e, "GetDefaultDevice");
			return NULL;
		}
	};

	// (a.walling 2008-09-10 17:43) - PLID 30389 - Get device from UUID
	WIA::IDevicePtr GetDeviceFromID(const CString& strDeviceID) {
		try {
			WIA::IDeviceManagerPtr pManager;
			HRESULT hr = pManager.CreateInstance("WIA.DeviceManager");

			if (SUCCEEDED(hr) && pManager != NULL) {
				WIA::IDeviceInfosPtr pDeviceInfos = pManager->GetDeviceInfos();

				if (pDeviceInfos) {
					if (pDeviceInfos->Count <= 0) {
						return NULL;
					} else {
						_variant_t varDeviceID((LPCTSTR)strDeviceID);
						WIA::IDevicePtr pDevice = pDeviceInfos->Item[&varDeviceID];
					}
				}
			}

			return NULL;
		} catch (_com_error& e) {
			LogError(e, "GetDeviceFromID");
			return NULL;
		}
	};

	// (a.walling 2008-09-10 17:43) - PLID 30389 - Get items selected from device's select items dialog
	WIA::IItemsPtr GetSelectedItems(WIA::IDevicePtr pDevice) {
		try {
			WIA::ICommonDialogPtr pCommonDialog;
			HRESULT hr = pCommonDialog.CreateInstance("WIA.CommonDialog");
			
			if (SUCCEEDED(hr) && pCommonDialog != NULL) {
				return pCommonDialog->ShowSelectItems(pDevice, WIA::UnspecifiedIntent, WIA::MaximizeQuality, VARIANT_FALSE, VARIANT_TRUE, VARIANT_FALSE);
			} else {
				return NULL;
			}
		} catch (_com_error& e) {
			LogError(e, "GetSelectedItems");
			return NULL;
		}
	}

	// (a.walling 2008-09-10 17:43) - PLID 30389 - Create and return the Common Dialog
	WIA::ICommonDialogPtr GetCommonDialog() {
		try {
			WIA::ICommonDialogPtr pCommonDialog;
			HRESULT hr = pCommonDialog.CreateInstance("WIA.CommonDialog");

			if (SUCCEEDED(hr) && pCommonDialog != NULL)
				return pCommonDialog;
			else
				return NULL;
		} catch (_com_error& e) {
			LogError(e, "GetCommonDialog");
			return NULL;
		}
	}



	/////////////////////////////////////////////////////////////////////////////
	// CWIAEventSink
	// (a.walling 2008-09-10 17:42) - PLID 31334 - Event sink for WIA events

	IMPLEMENT_DYNCREATE(CWIAEventSink, CCmdTarget)

	CWIAEventSink::CWIAEventSink()
	{
		EnableAutomation();
		m_dwCookie = 0;
		m_pNotify = NULL;
	}

	CWIAEventSink::~CWIAEventSink()
	{
		ASSERT(0 == m_dwCookie);
	}

	void CWIAEventSink::OnFinalRelease()
	{
		// When the last reference for an automation object is released
		// OnFinalRelease is called.  The base class will automatically
		// deletes the object.  Add additional cleanup required for your
		// object before calling the base class.
		CCmdTarget::OnFinalRelease();
	}


	BEGIN_MESSAGE_MAP(CWIAEventSink, CCmdTarget)
		//{{AFX_MSG_MAP(CWIAEventSink)
			// NOTE - the ClassWizard will add and remove mapping macros here.
		//}}AFX_MSG_MAP
	END_MESSAGE_MAP()

	BEGIN_DISPATCH_MAP(CWIAEventSink, CCmdTarget)
		//{{AFX_DISPATCH_MAP(CWIAEventSink)
		DISP_FUNCTION_ID(CWIAEventSink, "OnEvent",0x1,OnEvent, VT_EMPTY, VTS_BSTR VTS_BSTR VTS_BSTR)
		//}}AFX_DISPATCH_MAP
	END_DISPATCH_MAP()

	BEGIN_INTERFACE_MAP(CWIAEventSink, CCmdTarget)
		INTERFACE_PART(CWIAEventSink, __uuidof(WIA::_IDeviceManagerEvents), Dispatch)
	END_INTERFACE_MAP()

	HRESULT CWIAEventSink::CleanUp()
	{
		if (0 != m_dwCookie) {
			//Get a pointer to sinks IUnknown, no AddRef.
			LPUNKNOWN pUnkSink = GetIDispatch(FALSE);

			//Terminate a connection between source and sink.
			//m_pUnkSrc is IUnknown of server obtained by CoCreateInstance().
			//m_dwCookie is a value obtained through AfxConnectionAdvise().
			AfxConnectionUnadvise(m_pWIA, __uuidof(WIA::_IDeviceManagerEvents), pUnkSink, FALSE, m_dwCookie);
		}
		m_dwCookie = 0;
		m_pWIA = NULL;
		m_pNotify = NULL;
		return S_OK;
	}

	HRESULT CWIAEventSink::EnsureSink(CWnd* pNotify, LPDISPATCH pWIA)
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
		if (!AfxConnectionAdvise(pWIA, __uuidof(WIA::_IDeviceManagerEvents), pUnkSink, FALSE, &m_dwCookie))
		{
			return S_FALSE;
		}

		m_pNotify = pNotify;
		m_pWIA = pWIA;
		return S_OK;
	}


	/////////////////////////////////////////////////////////////////////////////
	// CWIAEventSink message handlers


	void CWIAEventSink::OnEvent(LPCTSTR EventID, LPCTSTR DeviceID, LPCTSTR ItemID)
	{
		CWIAEvent wiaEvent;

		wiaEvent.EventID = (LPCTSTR)EventID;
		wiaEvent.DeviceID = (LPCTSTR)DeviceID;
		wiaEvent.ItemID = (LPCTSTR)ItemID;

#ifdef _DEBUG
		TRACE("NxWIA: Recieved Event %s from %s: %s\n", wiaEvent.EventID, wiaEvent.DeviceID, wiaEvent.ItemID);
#endif

		if (m_pNotify != NULL && ::IsWindow(m_pNotify->GetSafeHwnd())) {
			m_pNotify->SendMessage(NXM_WIA_EVENT, (WPARAM)&wiaEvent, 0);
		}
#ifdef _DEBUG
		else {
			TRACE("NxWIA: Event ignored (no notification window available)");
		}
#endif
	}
};
