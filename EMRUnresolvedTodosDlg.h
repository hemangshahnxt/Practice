#if !defined(AFX_EMRUNRESOLVEDTODOSDLG_H__0A5052E4_7A93_460E_A124_325ABB18B5FF__INCLUDED_)
#define AFX_EMRUNRESOLVEDTODOSDLG_H__0A5052E4_7A93_460E_A124_325ABB18B5FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRUnresolvedTodosDlg.h : header file
//
// (c.haag 2008-07-10 12:13) - PLID 30648 - Initial implementation
// (c.haag 2008-07-14 15:49) - PLID 30696 - Changed scope from only spawned/unspawned
// todo's to manually created/deleted todos
//
class EMNTodo;

/////////////////////////////////////////////////////////////////////////////
// CEMRUnresolvedTodosDlg dialog

class CEMRUnresolvedTodosDlg : public CNxDialog
{
// Construction
public:
	CEMRUnresolvedTodosDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEMRUnresolvedTodosDlg)
	enum { IDD = IDD_EMR_UNRESOLVED_TODOS };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRUnresolvedTodosDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (c.haag 2008-07-14 10:52) - PLID 30696 - The formerly "spawned todo"
	// list now includes EMN-specific todos that were manually added by the
	// user. So, a name change is in order.
	NXDATALIST2Lib::_DNxDataListPtr m_dlCreatedTodos;
	NXDATALIST2Lib::_DNxDataListPtr m_dlDeletedTodos;

public:
	// (c.haag 2008-07-14 10:57) - PLID 30696 - These lists used to apply to just
	// spawned/unspawned todo's; now they also apply to manually created/deleted todos
	CArray<EMNTodo*,EMNTodo*> m_apCreatedEMNTodos;
	CArray<EMNTodo*,EMNTodo*> m_apDeletedEMNTodos;
	
protected:
	// (c.haag 2008-07-10 10:22) - Adds a row to either list
	void AddTodoRow(NXDATALIST2Lib::_DNxDataListPtr& pList, EMNTodo* pTodo);

protected:
	void OnOK();

	// Generated message map functions
	//{{AFX_MSG(CEMRUnresolvedTodosDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRUNRESOLVEDTODOSDLG_H__0A5052E4_7A93_460E_A124_325ABB18B5FF__INCLUDED_)
