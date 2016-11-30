#if !defined(AFX_EMRTODOACTIONCONFIGDLG_H__FFDC9F88_AF7F_43C8_9AA8_1A5895415603__INCLUDED_)
#define AFX_EMRTODOACTIONCONFIGDLG_H__FFDC9F88_AF7F_43C8_9AA8_1A5895415603__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRTodoActionConfigDlg.h : header file
//
// (c.haag 2008-05-30 16:37) - PLID 30221 - Initial implementation
//

/////////////////////////////////////////////////////////////////////////////
// CEMRTodoActionConfigDlg dialog

class CEMRTodoActionConfigDlg : public CNxDialog
{
// Construction
public:
	CEMRTodoActionConfigDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEMRTodoActionConfigDlg)
	enum { IDD = IDD_EMR_TODO_ACTION_CONFIG };
	CNxLabel	m_nxlAssignToMultiple;
	CComboBox	m_comboRemindType;
	CComboBox	m_comboDeadlineIntervalType;
	NxButton	m_radioDeadlineImmediate;
	NxButton	m_radioDeadlineAfter;
	NxButton	m_radioRemindImmediate;
	NxButton	m_radioRemindAfter;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	CNxEdit		m_editDeadlineIntervalNumber;
	CNxEdit		m_editRemindIntervalNumber;
	CNxEdit		m_editNotes;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRTodoActionConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL m_bIsNew;

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_dlCategory;
	NXDATALIST2Lib::_DNxDataListPtr m_dlMethod;
	NXDATALIST2Lib::_DNxDataListPtr m_dlPriority;
	NXDATALIST2Lib::_DNxDataListPtr m_dlAssignTo;

protected:
	CArray<long,long> m_anAssignTo;

protected:
	// (c.haag 2008-06-02 14:58) - The bounding rectangle of the multiple
	// assignment static text box
	CRect m_rcAssignToMultiple;

public:
	// (c.haag 2008-06-02 17:20) - Values used for loading and saving
	long m_nCommit_Category;
	CString m_strCommit_Method;
	long m_nCommit_Priority;
	CArray<long,long> m_anCommit_AssignTo;
	CString m_strCommit_Notes;
	long m_nCommit_DeadlineInterval;
	long m_nCommit_DeadlineType;
	long m_nCommit_RemindInterval;
	long m_nCommit_RemindType;

protected:
	// (c.haag 2008-05-30 16:31) - Returns TRUE if this is a new todo action
	BOOL IsNew() const;

protected:
	// (c.haag 2008-05-30 16:33) - Load the default values for a new todo action
	void LoadDefaultValues();
	// (c.haag 2008-05-30 16:36) - Loads a new todo action
	void LoadTodoAction();

protected:
	// (c.haag 2008-06-02 14:56) - based on the current content of m_anAssignTo,
	// update the visibility of the Assign To combo
	void RefreshAssignToCombo();

protected:
	// (c.haag 2008-07-01 09:11) - Moves up a todo date given an interval. Used for validation testing only.
	void MoveTodoDate(COleDateTime& dt, long nType, long nInterval);
	// (c.haag 2008-05-30 16:41) - Returns TRUE if we may save the data
	// for this dialog, or false if not
	BOOL Validate();
	// (c.haag 2008-05-30 16:43) - Called to write the form data to
	// member variables
	void Save();

protected:
	// Generated message map functions
	//{{AFX_MSG(CEMRTodoActionConfigDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDeadlineImmediate();
	afx_msg void OnDeadlineTimeAfter();
	afx_msg void OnRemindImmediate();
	afx_msg void OnRemindAfter();
	afx_msg void OnSelChosenComboTodoAssignTo(LPDISPATCH lpRow);
	afx_msg void OnSelChangingComboTodoAssignTo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangingComboTodoCategory(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangingComboTodoMethod(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangingComboTodoPriority(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelLButtonDown(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRTODOACTIONCONFIGDLG_H__FFDC9F88_AF7F_43C8_9AA8_1A5895415603__INCLUDED_)
