#if !defined(AFX_YAKGROUPSELECT_H__A3158DB0_8461_4216_8A48_1BBF8954F194__INCLUDED_)
#define AFX_YAKGROUPSELECT_H__A3158DB0_8461_4216_8A48_1BBF8954F194__INCLUDED_

#include "SelectRecipientsDlg.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// YakGroupSelect.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CYakGroupSelect dialog

class CYakGroupSelect : public CNxDialog
{
// Construction
public:
	CYakGroupSelect(CWnd* pParent);   // standard constructor
	//CSelectRecipientsDlg m_dlgSelectRecipients; // this was not used
	CDWordArray m_dwGroupsAry;
	CStringArray m_strGroupsAry;
	


// Dialog Data
	//{{AFX_DATA(CYakGroupSelect)
	enum { IDD = IDD_YAK_GROUP_SELECT };
	CNxIconButton	m_btnAddYakGroup;
	CNxIconButton	m_btnEditYakGroup;
	CNxIconButton	m_btnDeleteYakGroup;
	CNxIconButton	m_btnOk;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CYakGroupSelect)
	public:
	virtual int DoModal(long nMode = 0);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_groupList;

	bool IsGroupNameValid(CString strGropuNameToValidate, CString strOldGroupName = "");
	long m_nMode;

	// Generated message map functions
	//{{AFX_MSG(CYakGroupSelect)
	afx_msg void OnAddGroup();
	afx_msg void OnEditGroup();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnDeleteYakGroup();
	afx_msg void OnDblClickCellYakGroupList(long nRowIndex, short nColIndex);
	afx_msg void OnEditingFinishingYakGroupList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_YAKGROUPSELECT_H__A3158DB0_8461_4216_8A48_1BBF8954F194__INCLUDED_)
