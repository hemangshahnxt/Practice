#if !defined(AFX_PROCEDUREREPLACEDLG_H__7FDF438E_F616_4773_8121_537ED7C3DFF4__INCLUDED_)
#define AFX_PROCEDUREREPLACEDLG_H__7FDF438E_F616_4773_8121_537ED7C3DFF4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProcedureReplaceDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProcedureReplaceDlg dialog

class CProcedureReplaceDlg : public CNxDialog
{
// Construction
public:
	CProcedureReplaceDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CProcedureReplaceDlg)
	enum { IDD = IDD_PROCEDURE_REPLACE_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcedureReplaceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_findType,
					m_findPurpose,
					m_replaceType,
					m_replacePurpose;

	void RequeryFindPurpose();
	void RequeryReplacePurpose();

	bool GetID(long &id, NXDATALISTLib::_DNxDataListPtr &list);

	// (j.gruber 2008-06-27 09:34) - PLID 26542 - used for auditing
	bool GetName(CString &strName, NXDATALISTLib::_DNxDataListPtr &list);

	// Generated message map functions
	//{{AFX_MSG(CProcedureReplaceDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelChosenFindType(long nRow);
	afx_msg void OnSelChosenReplaceType(long nRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCEDUREREPLACEDLG_H__7FDF438E_F616_4773_8121_537ED7C3DFF4__INCLUDED_)
