// NxTWAIN.h: interface for the CNxTWAIN class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NXTWAIN_H__4CDB9B9C_69C6_47A9_8B1A_DB90FD944C94__INCLUDED_)
#define AFX_NXTWAIN_H__4CDB9B9C_69C6_47A9_8B1A_DB90FD944C94__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CNxTWAIN  
{
public:
	BOOL ActivatedByCamera();
	void ResetCameraActivate(BOOL bActivated = FALSE);
	BOOL Register(const CString& strAppName);
	void Unregister();

	// (a.walling 2008-10-28 13:05) - PLID 31334 - Launched from WIA auto-start
	BOOL ActivatedByWIA();
	void ResetWIAActivate(BOOL bActivated = FALSE);
	BOOL HandleWIAEvent(const CString& strWIADevice);
	CString GetWIADevice();
	long GetWIAInfoFileNumber();

	void OnRequestImport(const CString& strAppName);
	CString GetActiveDeviceName();
	long GetActiveDeviceID();

	CNxTWAIN();
	virtual ~CNxTWAIN();

protected:
	DWORD GetCorrectProcess(const CDWordArray& adwPIDs);
	DWORD GetPID(const CString& strAppName);

protected:
	CString m_strActiveDeviceName;
	long m_nActiveDeviceID;
	BOOL m_bPersistentActivationByCamera;

	// (a.walling 2008-10-28 13:05) - PLID 31334 - Launched from WIA auto-start
	CString m_strWIADevice;
	long m_nWIAInfoFileNumber;
	BOOL m_bPersistentActivationByWIA;
};

#endif // !defined(AFX_NXTWAIN_H__4CDB9B9C_69C6_47A9_8B1A_DB90FD944C94__INCLUDED_)
