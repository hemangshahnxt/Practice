#if !defined(AFX_PURPOSES_H__32939A75_FD12_4ED2_99F3_829A7B1A5A3B__INCLUDED_)
#define AFX_PURPOSES_H__32939A75_FD12_4ED2_99F3_829A7B1A5A3B__INCLUDED_



#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Purposes.h : header file
//


#include "afxtempl.h"
#include "globalutils.h"

/////////////////////////////////////////////////////////////////////////////
// CPurposes dialog

class CPurposes : public CNxDialog
{
// Construction
public:
	void OK();
	void PUpdate();
	long GetUserID();
	void SetUserID(long ID);

// Dialog Data
	//{{AFX_DATA(CPurposes)
	enum { IDD = IDD_PURPOSESET };
	NxButton	m_btnAllowDelete;
	NxButton	m_btnAppendNote;
	CNxIconButton	m_btnRemoveAll;
	CNxIconButton	m_btnRemove;
	CNxIconButton	m_btnAddAll;
	CNxIconButton	m_btnAdd;
	CNxEdit	m_editDays;
	CListBox	m_listSelected;
	CListBox	m_listAvail;
	BOOL	m_bAppendNoteToDesc;
	BOOL	n_bAllowApptCancelling;
	NxButton	m_btnNoteGroupbox;
	//}}AFX_DATA

	CPurposes(CWnd* pParent);   // standard constructor
	void NxParse();
	void PopulateSelected();
	void PopulateAvail();
	
	long m_nPCount;
	long m_nParseIndex;

	ADODB::_RecordsetPtr m_p;
	
	bool m_bFirst;

	//CArray<CString, int> m_aryPurposes;
	CStringArray m_aryPurposes;
	CString m_TextParam;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPurposes)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	long m_nPUserID;

	void OnOK();
	void OnCancel();
	// Generated message map functions
	//{{AFX_MSG(CPurposes)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonAddAll();
	afx_msg void OnButtonRemove();
	afx_msg void OnButtonRemoveAll();
	afx_msg void OnDblclkListAvail();
	afx_msg void OnDblclkListSelected();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PURPOSES_H__32939A75_FD12_4ED2_99F3_829A7B1A5A3B__INCLUDED_)
