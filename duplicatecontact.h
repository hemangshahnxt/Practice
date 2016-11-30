//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_DUPLICATECONTACT_H__AC01EB53_D658_11D2_9340_00104B318376__INCLUDED_)
#define AFX_DUPLICATECONTACT_H__AC01EB53_D658_11D2_9340_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Duplicate.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CDuplicateContact dialog

class CDuplicateContact : public CNxDialog
{
// Construction
public:
	CDuplicateContact(CWnd* pParent =NULL, LPCTSTR strIgnoreBtnText = NULL, LPCTSTR strRetryBtnText = NULL, LPCTSTR strAbortBtnText = NULL, bool bForceSelectionOnIgnore = false);
	bool FindDuplicates(CString first, CString last, CString middle);
	CString m_name;
	
public:
	long m_nSelContactId;

// Dialog Data
	//{{AFX_DATA(CDuplicateContact)
	enum { IDD = IDD_CONTACT_DUPLICATE };
	CNxStatic	m_nxstaticNameLabel;
	CNxIconButton	m_btnSaveContact;
	CNxIconButton	m_btnChangeContactName;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDuplicateContact)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	long GetSelContactId();
	bool m_bForceSelectionOnIgnore;
	CString m_strIgnoreBtnText;
	CString m_strRetryBtnText;
	CString m_strAbortBtnText;
	NXDATALISTLib::_DNxDataListPtr	m_pDupList;

protected:
	CString sql;
	// Generated message map functions
	//{{AFX_MSG(CDuplicateContact)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnBack();
	afx_msg void OnCancel();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DUPLICATECONTACT_H__AC01EB53_D658_11D2_9340_00104B318376__INCLUDED_)
