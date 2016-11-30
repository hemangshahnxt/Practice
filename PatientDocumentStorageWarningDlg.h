// (d.thompson 2013-07-02) - PLID 13764 - This dialog is a warning dialog for the PersonDocumentStorage warning
//	exception.  This exception happens when you try to access the patient's document folder but are unable to
//	create it (generally due to permissions or such).
#pragma once
#include "PracticeRc.h"

// CPatientDocumentStorageWarningDlg dialog

class CPatientDocumentStorageWarningDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CPatientDocumentStorageWarningDlg)

public:
	CPatientDocumentStorageWarningDlg(CString strPath, CWnd* pParent = NULL);   // standard constructor
	virtual ~CPatientDocumentStorageWarningDlg();

// Dialog Data
	enum { IDD = IDD_PERSON_DOCUMENT_STORAGE_WARNING_DLG };

	//public properties
	CString m_strPath;

protected:
	//controls
	CNxLabel m_nxlLink;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	void OnWarningLink();

	DECLARE_MESSAGE_MAP()
};
