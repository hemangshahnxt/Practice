#pragma once

#include "PatientDialog.h"
#import "NxManagedWrapper.tlb"
// CNexPhotoDlg dialog
// (c.haag 2009-08-19 12:39) - PLID 35231 - This class encapsulates the NexPhoto C# control as
// a tab for the patients module

// (c.haag 2015-07-08) - PLID 65912 - Only log verbose NexPhoto managed operations if this is not commented out
//#define LOG_NEXPHOTO_MANAGED_ACTIONS

class CNexPhotoDlg : public CPatientDialog
{
	DECLARE_DYNAMIC(CNexPhotoDlg)

private:
	NxManagedWrapperLib::INxPhotoTabPtr m_pNxPhotoTab;

private:
	CNxManagedWrapperEventSink m_EventSink;

public:
	CNexPhotoDlg(CWnd* pParent);   // standard constructor
	virtual ~CNexPhotoDlg();

// Dialog Data
	enum { IDD = IDD_NEXPHOTO };

public:
	// (a.walling 2014-04-25 16:00) - VS2013 - We definitely don't support Win2k any longer

public:
	virtual void SetColor(OLE_COLOR nNewColor);

public:
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnDestroy();
	
	DECLARE_MESSAGE_MAP()
	// (r.gonet 2016-05-24 15:54) - NX-100732 - Handle when the system name changes, because
	// NexPhoto stores it and queries for machine specific preferences.
	afx_msg LRESULT OnSystemNameUpdated(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	
};
