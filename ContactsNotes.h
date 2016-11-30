#if !defined(AFX_CONTACTSNOTES_H__CF6A8C95_55AF_11D3_AD64_00104B318376__INCLUDED_)
#define AFX_CONTACTSNOTES_H__CF6A8C95_55AF_11D3_AD64_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ContactsNotes.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CContactsNotes dialog

class CContactsNotes : public CNxDialog
{
// Construction
public:
	NXDATALISTLib::_DNxDataListPtr m_list;
	CContactsNotes(CWnd* pParent);   // standard constructor
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	virtual void RecallDetails();
	virtual void SecureControls();
// Dialog Data
	//{{AFX_DATA(CContactsNotes)
	enum { IDD = IDD_CONTACTS_NOTES };
	NxButton	m_btnShowGridlines;
	CNxIconButton	m_editButton;
	CNxIconButton	m_deleteButton;
	CNxIconButton	m_addButton;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CContactsNotes)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	long m_iRow;
	void CContactsNotes::Requery(long id);
	long m_id;		//used instead of GetActiveContactID()
	void Add();
	void Delete();
	void EditCategories();
	void EnableAppropriateButtons();
	// Generated message map functions
	//{{AFX_MSG(CContactsNotes)
	virtual BOOL OnInitDialog();
	afx_msg void OnRButtonDownList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishingList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnColumnClickingList(short nCol, BOOL FAR* bAllowSort);
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnEdit();
	afx_msg void OnEditingStartingList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnEditingStartedList(long nRow, short nCol, long nEditType);
	afx_msg void OnSelChangedList(long nNewSel);
	afx_msg void OnContactsNotesShowGrid();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONTACTSNOTES_H__CF6A8C95_55AF_11D3_AD64_00104B318376__INCLUDED_)
