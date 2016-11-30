// GBenericLink.h: interface for the CGenericLink class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GENERICLINK_H__76B7AA5C_0FD7_48F4_9577_1B96BEE3769C__INCLUDED_)
#define AFX_GENERICLINK_H__76B7AA5C_0FD7_48F4_9577_1B96BEE3769C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CGenericLink : public CNxDialog  
{
protected:
	//////////////////////////////////////
	// Data lists for the dialog
	NXDATALISTLib::_DNxDataListPtr m_dlPractice;
	NXDATALISTLib::_DNxDataListPtr m_dlRemote;
	NXDATALISTLib::_DNxDataListPtr m_dlPracticeSelected;
	NXDATALISTLib::_DNxDataListPtr m_dlRemoteSelected;

	//////////////////////////////////////
	// Connection pointer to remote DB
	// and functions to access it.
	ADODB::_ConnectionPtr m_pConRemote;
	void EnsureRemoteData();
	void EnsureNotRemoteData();

	//////////////////////////////////////
	// Status bar
	CStatusBarCtrl m_statusBar;

	//////////////////////////////////////
	// Remote path information
	CString m_strRemotePath;
	BOOL m_bGotRemotePath;
	CString m_strRemotePathProperty;
	CString m_strRemotePassword;
	CString BrowseRemotePath();
	CButton	m_btnRemotePath;
	void OnBtnRemotePath();

	//////////////////////////////////////
	// List color related
	virtual void RefreshColors();

	//////////////////////////////////////
	// General functions
	virtual unsigned long RefreshData();
	void OnPracAdd();
	void OnPracRemove();
	void OnPracRemoveAll();
	void OnRemoteAdd();
	void OnRemoteRemove();
	void OnRemoteRemoveAll();
	void OnRequeryFinishedNextech(short nFlags);
	void OnRequeryFinishedRemote(short nFlags);
	void OnUnlink();	
	ADODB::_RecordsetPtr CreateRecordSet(CString strQuery);
	//////////////////////////////////////
	// Overloads
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	//////////////////////////////////////
	// Other Variables
	BOOL m_bLinkEnabled; // The link can be temporarily disabled if the database
						// is inaccessible or invalid, and the user needs to open
						// the configuration dialog.
	CString m_strLinkName; // The name of the link (United, Mirror, etc)

public:
	CNxIconButton	m_remoteRemAll;
	CNxIconButton	m_remoteRem;
	CNxIconButton	m_remoteAdd;
	CNxIconButton	m_pracRemAll;
	CNxIconButton	m_pracRem;
	CNxIconButton	m_pracAdd;
	CNxIconButton	m_btnImport;
	CNxIconButton	m_btnExport;

	virtual CString GetRemotePath() = 0;
	virtual CString GetRemotePassword() { return m_strRemotePassword; }
	virtual void SetRemotePath(const CString &path) = 0;
	virtual BOOL TestConnection(CString strRemotePath) = 0; // Returns true on success
	virtual LPDISPATCH GetRemoteData();
	inline BOOL IsLinkEnabled() { return m_bLinkEnabled; }
	inline void EnableLink(BOOL bEnable) { m_bLinkEnabled = bEnable; }

	virtual void Unlink(long lNexTechID) = 0;
	

	CGenericLink(CWnd *pParent);
	CGenericLink(int IDD, CWnd *pParent);
	virtual ~CGenericLink();

	void OpenDlg();
	void CloseDlg();	
};

#endif // !defined(AFX_GENERICLINK_H__76B7AA5C_0FD7_48F4_9577_1B96BEE3769C__INCLUDED_)
