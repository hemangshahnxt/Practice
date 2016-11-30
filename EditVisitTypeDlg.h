#if !defined(AFX_EDITVISITTYPEDLG_H__10CE3DB3_FD94_4B8B_A64C_0EF282A19EE9__INCLUDED_)
#define AFX_EDITVISITTYPEDLG_H__10CE3DB3_FD94_4B8B_A64C_0EF282A19EE9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditVisitTypeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditVisitTypeDlg dialog

class CEditVisitTypeDlg : public CNxDialog
{
// Construction
public:
	CEditVisitTypeDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_pTypeList;

// Dialog Data
	//{{AFX_DATA(CEditVisitTypeDlg)
	enum { IDD = IDD_EDIT_VISIT_TYPE };
	CNxIconButton	m_btnDown;
	CNxIconButton	m_btnUp;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditVisitTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	long m_nSelectedID;

	// Generated message map functions
	//{{AFX_MSG(CEditVisitTypeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnEditingFinishedVisitTypes(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnUp();
	afx_msg void OnDown();
	virtual void OnOK();
	afx_msg void OnRequeryFinishedVisitTypes(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITVISITTYPEDLG_H__10CE3DB3_FD94_4B8B_A64C_0EF282A19EE9__INCLUDED_)
