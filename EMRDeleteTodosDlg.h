#if !defined(AFX_EMRDELETETODOSDLG_H__6C9E8F10_802E_444E_8151_C6BE22FB9A97__INCLUDED_)
#define AFX_EMRDELETETODOSDLG_H__6C9E8F10_802E_444E_8151_C6BE22FB9A97__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRDeleteTodosDlg.h : header file
//
// (c.haag 2008-07-15 09:57) - PLID 30694 - Initial implementation
//

/////////////////////////////////////////////////////////////////////////////
// CEMRDeleteTodosDlg dialog

class CEMRDeleteTodosDlg : public CNxDialog
{
// Construction
public:
	CEMRDeleteTodosDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEMRDeleteTodosDlg)
	enum { IDD = IDD_EMR_DELETE_TODOS };
	CNxIconButton	m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRDeleteTodosDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	CArray<long,long> m_anInputTodoIDs; // Input list of ID's

public:
	CArray<long,long> m_anTodoIDsToDelete; // Output list of ID's to delete

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_dlList;

protected:
	// Generated message map functions
	//{{AFX_MSG(CEMRDeleteTodosDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRDELETETODOSDLG_H__6C9E8F10_802E_444E_8151_C6BE22FB9A97__INCLUDED_)
