#if !defined(AFX_CONTACTVIEW_H__E6E05066_968F_11D2_80F4_00104B2FE914__INCLUDED_)
#define AFX_CONTACTVIEW_H__E6E05066_968F_11D2_80F4_00104B2FE914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AdminView.h : header file
//

// (j.jones 2013-05-08 08:44) - PLID 56591 - removed the .h files for the child tabs
class CContactsGeneral;
class CContactsNotes;
class CContactTodo;
class CHistoryDlg;
class CAttendanceDlg;

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and ResolveDefaultTab to the modules code

/////////////////////////////////////////////////////////////////////////////
// CContactView view

class CContactView : public CNxTabView
{

public:

public:
	CContactView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CContactView)
// Attributes
public:
// Operations
public:
	BOOL CheckPermissions();
	afx_msg void OnPrint();
	virtual int Hotkey (int key);

public:
	// (k.messina 2010-04-09 17:29) - PLID 37597
	bool CheckViewNotesPermissions();
	bool m_bIsLocked;
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CContactView)
	protected:
	virtual void OnSelectTab(short newTab, short oldTab);//used for the new NxTab
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);// (s.tullis 2014-09-05 13:15) - PLID 63226
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (j.jones 2013-05-08 08:44) - PLID 56591 - changed the child tabs to be declared by reference,
	// which avoids requiring their .h files in this file
	CContactsGeneral	&m_generalSheet;
	CContactsNotes		&m_notesSheet;
	CContactTodo		&m_todoSheet;
	CHistoryDlg			&m_HistorySheet;
	// (z.manning, 11/28/2007) - PLID 28216 - Added a sheet for attendance tracking for users.
	CAttendanceDlg		&m_dlgAttendanceSheet;

	virtual ~CContactView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	// Generated message map functions
protected:
	BOOL PrintTab(bool Preview, CPrintInfo *pInfo = 0);
	//{{AFX_MSG(CContactView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPrintPreview();
	afx_msg void OnUpdateFilePrint(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFilePrintPreview(CCmdUI* pCmdUI);

	// (a.walling 2016-02-15 08:11) - PLID 67826 - Combine Providers
	afx_msg void OnContactCombine();
	afx_msg void OnUpdateContactCombine(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_ADMINVIEW_H__E6E05066_968F_11D2_80F4_00104B2FE914__INCLUDED_)
