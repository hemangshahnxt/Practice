#pragma once
#include "afxwin.h"
#include "AdministratorRc.h"

// CDiagQuickListCopyFromUserDlg dialog
// (c.haag 2014-02-24) - PLID 60950 - Initial implementation. From here, users can choose which
// other users' QuickLists to copy items from.

class CDiagQuickListCopyFromUserDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CDiagQuickListCopyFromUserDlg)

protected:
	NXDATALIST2Lib::_DNxDataListPtr	m_UserList;

public:
	CStringArray m_astrSelectedUserIDs;

public:
	CDiagQuickListCopyFromUserDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDiagQuickListCopyFromUserDlg();

// Dialog Data
	enum { IDD = IDD_DIAG_QUICKLIST_COPYFROMUSER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
public:
	afx_msg void OnBnClickedOk();
};
