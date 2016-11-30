#if !defined(AFX_CONTACTTODO_H__CF6A8C91_55AF_11D3_AD64_00104B318376__INCLUDED_)
#define AFX_CONTACTTODO_H__CF6A8C91_55AF_11D3_AD64_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// (c.haag 2008-06-10 11:15) - PLID 11599 - Without this pragma, we would get compiler
// warnings about the new member functions having debug names truncating to 255 chars
#pragma warning(disable:4786)
// ContactTodo.h : header file
//
#include "Client.h"
#include "SharedPermissionUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CContactTodo dialog

class CContactTodo : public CNxDialog
{
// Construction
public:
	CContactTodo(CWnd* pParent);   // standard constructor
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	virtual void RecallDetails();

	void OnModifyItem();

	// (a.walling 2006-08-07 11:35) - PLID 3897 - We need functions to access the protect member vars m_finished, m_unfinished's check state
	bool FinishedChecked();
	bool UnfinishedChecked();

	CTableChecker m_tblCheckToDoList;
// Dialog Data
	//{{AFX_DATA(CContactTodo)
	enum { IDD = IDD_CONTACTS_FOLLOW_UP };
	NxButton	m_btnShowGridlines;
	CNxIconButton	m_viewButton;
	CNxIconButton	m_createButton;
	CNxIconButton	m_deleteButton;
	CNxIconButton	m_editButton;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CContactTodo)
	public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam); // (s.tullis 2014-08-26 11:50) - PLID 63226 -
	void ReflectchangedTodo(CTableCheckerDetails* pDetails);
	
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	// (c.haag 2008-07-03 12:25) - PLID 30615 - This function encapsulates security checking, and saves
	// us from having to repeat a lot of code.
	BOOL CheckAssignToPermissions(long nRow, ESecurityPermissionType type);

	// (c.haag 2008-06-10 15:31) - PLID 11599 - Populates a row in the list with data values
	void PopulateListRow(long nRow, ADODB::FieldsPtr& f);

	// (c.haag 2008-06-10 15:31) - PLID 11599 - Populates a row in the list with data values
	void PopulateListRow(NXDATALISTLib::IRowSettingsPtr& pRow, ADODB::FieldsPtr& f);

	//(c.copits 2011-02-22) PLID 40794 - Permissions for individual todo alarm fields
	BOOL CheckIndividualPermissions(short nCol, CArray<long,long> &arynAssignTo);

protected:
	NXDATALISTLib::_DNxDataListPtr m_list;
	void Requery (long id);
	long m_iRow;
	NxButton m_finished;
	NxButton m_unfinished;
	NxButton m_all;
	void Delete();
	long m_id;		//used instead of GetActiveContactID()

	// (a.walling 2006-08-14 08:51) - PLID 21755
	/* To-do list coloring */
	void ColorizeList(OPTIONAL long nRow = -1); // colorize the todo list accordingly.
	void ColorizeItem(NXDATALISTLib::IRowSettingsPtr &pRow); // colorize a single item
	COLORREF m_colorComplete;
	COLORREF m_colorIncompleteHigh;
	COLORREF m_colorIncompleteMedium;
	COLORREF m_colorIncompleteLow;

	// Generated message map functions
	//{{AFX_MSG(CContactTodo)
	virtual BOOL OnInitDialog();
	afx_msg void OnCompleted();
	afx_msg void OnIncomplete();
	afx_msg void OnAll();
	afx_msg void OnRButtonDownListTodo(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonUpListTodo(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnLButtonDownListTodo(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishingListTodo(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedListTodo(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnView();
	afx_msg void OnDelete();
	afx_msg void OnEdit();
	afx_msg void OnCreateTodo();
	afx_msg void OnEditingStartingListTodo(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnDblClickCellListTodo(long nRowIndex, short nColIndex);
	afx_msg void OnContactsFollowupShowGrid();
	afx_msg void OnRequeryFinishedListTodo(short nFlags);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONTACTTODO_H__CF6A8C91_55AF_11D3_AD64_00104B318376__INCLUDED_)
