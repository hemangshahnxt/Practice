#if !defined(AFX_FAXSENDDLG_H__62E55DCB_AC8B_4F63_BA4E_45A5B0A7A375__INCLUDED_)
#define AFX_FAXSENDDLG_H__62E55DCB_AC8B_4F63_BA4E_45A5B0A7A375__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FaxSendDlg.h : header file
//
//DRT 6/30/2008 - PLID 30541 - Created.

/////////////////////////////////////////////////////////////////////////////
// CFaxSendDlg dialog

class CFaxSendDlg : public CNxDialog
{
// Construction
public:
	CFaxSendDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFaxSendDlg)
	enum { IDD = IDD_FAX_SEND_DLG };
	CNxEdit	m_editUser;
	CNxEdit	m_editRecipNum;
	CNxEdit	m_editRecipName;
	CNxEdit	m_editPassword;
	CNxEdit	m_editFromName;
	CNxEdit m_editSubject; // (z.manning 2009-09-29 13:03) - PLID 32472
	CNxIconButton	m_btnRemove;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnDown;
	CNxIconButton	m_btnUp;
	CNxIconButton	m_btnSendFax;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA

	//Exposed function for easy sending

	//Use this for a single document
	//(e.lally 2011-10-31) PLID 41195 - Added PersonID and PicID for the mailSent log entry
	int BeginSingleDocFax(CString strNumber, CString strName, CString strDocumentPath, long nPersonID = -1, long nPicID = -1);

	//use this for multiple documents
	//(e.lally 2011-10-31) PLID 41195 - Added PersonID and PicID for the mailSent log entry
	int BeginFax(CString strNumber, CString strName, CStringArray& aryDocumentPaths, long nPersonID = -1, long nPicID = -1);

	//Default values for name, number.  It is preferred you use
	//	BeginFax over filling these manually.
	CString m_strBeginNumber, m_strBeginName;

	//Array of documents to be faxed.  Will be faxed in the order given (user overrideable order)
	CStringArray m_aryDocumentPaths;



// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFaxSendDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//Datalists
	NXDATALIST2Lib::_DNxDataListPtr m_pServiceList;
	NXDATALIST2Lib::_DNxDataListPtr m_pResolutionList;
	NXDATALIST2Lib::_DNxDataListPtr m_pDocumentList;
	//MailSent information
	//(e.lally 2011-10-31) PLID 41195 - Added PersonID and PicID for the mailSent log entry
	long m_nPersonID;
	long m_nPicID;

	//Helper functions
	void LoadDefaultValues();
	void LoadDocuments();
	void AddPathToDocumentList(CString strPath);

	// Generated message map functions
	//{{AFX_MSG(CFaxSendDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSendFax();
	afx_msg void OnSelChosenFaxServiceList(LPDISPATCH lpRow);
	afx_msg void OnFaxDocAdd();
	afx_msg void OnFaxDocRemove();
	afx_msg void OnFaxDocUp();
	afx_msg void OnFaxDocDown();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FAXSENDDLG_H__62E55DCB_AC8B_4F63_BA4E_45A5B0A7A375__INCLUDED_)
