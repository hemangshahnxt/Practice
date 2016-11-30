#if !defined(AFX_FOLLOWUPDLG_H__083147B4_0F72_11D2_808D_00104B2FE914__INCLUDED_)
#define AFX_FOLLOWUPDLG_H__083147B4_0F72_11D2_808D_00104B2FE914__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000		  
#include "PatientDialog.h"
// (c.haag 2008-06-10 11:15) - PLID 11599 - Without this pragma, we would get compiler
// warnings about the new member functions having debug names truncating to 255 chars
#pragma warning(disable:4786)

// FollowUpDlg.h : header file
//

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;
#include "client.h"

/////////////////////////////////////////////////////////////////////////////
// CFollowUpDlg dialog

class CFollowUpDlg : public CPatientDialog
{
// Construction
public:
	NXDATALISTLib::_DNxDataListPtr m_List;
	CFollowUpDlg(CWnd* pParent);   // standard constructor
	CTableChecker m_tblCheckTask;

	//(e.lally 2009-11-16) PLID 36304 - renamed to ModifyItem
	void ModifyItem();

// Dialog Data
	//{{AFX_DATA(CFollowUpDlg)
	enum { IDD = IDD_FOLLOWUP_DLG };
	NxButton	m_btnShowGrid;
	CNxIconButton	m_editCategoriesButton;
	CNxIconButton	m_deleteCompletedButton;
	CNxIconButton	m_createTodoButton;
	NxButton	m_all;
	NxButton	m_incomplete;
	NxButton	m_completed;
	CNxColor	m_bkg;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFollowUpDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam); // (s.tullis 2014-08-26 11:50) - PLID 63226 -
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (z.manning 2009-09-11 18:05) - PLID 32048
	void GetAssignToArrayByRow(long nRow, CArray<long,long> &arynAssignTo);


	// (c.haag 2008-06-10 10:49) - PLID 11599 - Populates a row in the list with data values
	void PopulateListRow(long nRow, ADODB::FieldsPtr& f);

	// (c.haag 2008-06-10 10:49) - PLID 11599 - Populates a row in the list with data values
	void PopulateListRow(NXDATALISTLib::IRowSettingsPtr& pRow, ADODB::FieldsPtr& f);

	//(c.copits 2011-02-10) PLID 40794 - Permissions for individual todo alarm fields
	BOOL CheckIndividualPermissions(long nRow, short nCol, CArray<long,long> &arynAssignTo);

protected:
	
	// (a.walling 2010-10-13 10:44) - PLID 40977 - Dead code
	//COleVariant m_varSelItem;

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	void SetColor(OLE_COLOR nNewColor);
	long m_nRowSel;
	long m_id;

	// (a.walling 2006-08-14 08:51) - PLID 21755
	/* To-do list coloring */
	void ColorizeList(OPTIONAL long nRow = -1); // colorize the todo list accordingly.
	void ColorizeItem(NXDATALISTLib::IRowSettingsPtr &pRow); // colorize a single item
	COLORREF m_colorComplete;
	COLORREF m_colorIncompleteHigh;
	COLORREF m_colorIncompleteMedium;
	COLORREF m_colorIncompleteLow;

	//(e.lally 2009-11-16) PLID 36304
	void CreateTodo();
	void ReflectChangedTodo(CTableCheckerDetails* pDetails);
	// Generated message map functions
	//{{AFX_MSG(CFollowUpDlg)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL OnInitDialog();
	afx_msg void OnRButtonUpListTodo(long nRow,short nCol, long x, long y, long nFlags);
	afx_msg void OnLButtonDownListTodo(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishedListTodo(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingListTodo(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnRadioAll();
	afx_msg void OnRadioCompleted();
	afx_msg void OnRadioIncomplete();
	afx_msg void OnRequeryFinishedListTodo(short nFlags);
	afx_msg void OnCreateTodo();
	afx_msg void OnTodoList();
	afx_msg void OnDeleteCompleted();
	afx_msg void OnEditCategories();
	afx_msg void OnEditingStartingListTodo(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnDblClickCellListTodo(long nRowIndex, short nColIndex);
	afx_msg void OnFollowupShowGrid();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	// (a.walling 2010-10-13 10:37) - PLID 40977 - Dead code, m_IsEditing is never set anywhere, nor m_IsAdding, nor m_boAllowUpdate
	//BOOL m_IsEditing;
	//BOOL m_IsAdding;
	void Requery();
	//BOOL m_boAllowUpdate;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FOLLOWUPDLG_H__083147B4_0F72_11D2_808D_00104B2FE914__INCLUDED_)
