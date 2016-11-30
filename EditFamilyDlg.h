#if !defined(AFX_EDITFAMILYDLG_H__1F1BA32D_9B68_4BFB_8BD6_BFA524A13B32__INCLUDED_)
#define AFX_EDITFAMILYDLG_H__1F1BA32D_9B68_4BFB_8BD6_BFA524A13B32__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditFamilyDlg.h : header file
//


/////////////////////////////////////////////////////////////////////////////
//FamilyUtils namespace

namespace FamilyUtils {
	// (a.walling 2006-11-20 12:04) - PLID 22715
	long GetFamilyID(long nPersonID); // return the FamilyID for the given Person, -1 if none.
	long CreateNewFamilyAndRelationship(long nPersonID, long RelativeID); // create a new family and relation and return the family id
	void ClearRelationships(long nPersonID); // remove all relationships for this person
	void InsertRelationship(long nPersonID, long nRelativeID, long nFamilyID, bool bAudit = true); // Create the relationship between these two for this family
	void CleanUp(); // cleans up all family entries, such as empty and single-person families
	long CountRelatives(long nPersonID); // (a.walling 2006-11-22 15:18) - PLID 22715 - Return the number of patients who have this patient marked as their immediate relative
	// (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT - This function was missing NOCOUNT, but was also broken and unreferenced, so removed
	//long UpdateRelatives(long nPersonID, long nNewRelativeID); // (a.walling 2006-11-22 15:21) - PLID 22715 - Updates RelativeID from nPersonID to nNewRelativeID. Returns # of records affected.
}

/////////////////////////////////////////////////////////////////////////////
// CEditFamilyDlg dialog

class CEditFamilyDlg : public CNxDialog
{
// Construction
public:
	CEditFamilyDlg(CWnd* pParent);   // standard constructor

	long m_nFamilyID; // (a.walling 2006-11-16 11:53) - PLID 22715 - Set this before calling DoModal so we know which family to display!
	long m_nPersonID; // (a.walling 2006-11-20 14:19) - PLID 22715 - REQUIRED - Set this to know what to set the relative to when adding from this dialog
	long m_nRelativeID; // (a.walling 2006-11-20 14:19) - PLID 22715 - Set this to the relative so we can choose a new one.
	NXDATALIST2Lib::_DNxDataListPtr m_dlFamily;
	NXDATALIST2Lib::_DNxDataListPtr m_dlPatients;

// Dialog Data
	// (a.walling 2008-04-03 10:29) - PLID 29497 - Added NxStatic controls
	//{{AFX_DATA(CEditFamilyDlg)
	enum { IDD = IDD_EDIT_FAMILY };
	CNxStatic	m_labelMessage;
	CNxStatic	m_labelFamilyCount;
	CNxIconButton	m_btnRemove;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditFamilyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CBrush m_brush;

	void UpdateLabel(); // simply update the text in the label
	CList<long, long> m_listAddMembers;
	CList<long, long> m_listRemoveMembers;

	bool Save(); // save changes to data
	void SetMessage(CString strMessage); // set the IDC_MESSAGE label
	void ResetWhereClause(); // set the where clauses for the lists
	long CalculateBrokenRelationships(); // return the number of patients who are losing their immediate relative

	bool m_bRemoved;
	long m_nRelationRemoved;	// number of patients removed that are immediate relations
	long m_nRelationRemovedCount;	// number of relations that may be broken if moving a person out of the family


	// Generated message map functions
	//{{AFX_MSG(CEditFamilyDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedFamilyMembers(short nFlags);
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnRequeryFinishedPatientList(short nFlags);
	afx_msg void OnDblClickCellPatientList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnDblClickCellFamilyMembers(LPDISPATCH lpRow, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITFAMILYDLG_H__1F1BA32D_9B68_4BFB_8BD6_BFA524A13B32__INCLUDED_)
