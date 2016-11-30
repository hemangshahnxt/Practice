#if !defined(AFX_EMRMERGECONFLICTDLG_H__01AE492E_3718_41BD_B252_6F2883BEFABA__INCLUDED_)
#define AFX_EMRMERGECONFLICTDLG_H__01AE492E_3718_41BD_B252_6F2883BEFABA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRMergeConflictDlg.h : header file
//
#include "patientsrc.h"

/////////////////////////////////////////////////////////////////////////////
// CEMRMergeConflictDlg dialog

class CEMRMergeConflictDlg : public CNxDialog
{
// Construction
public:
	CEMRMergeConflictDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEMRMergeConflictDlg)
	enum { IDD = IDD_EMR_MERGE_CONFLICT_DLG };
	CNxStatic	m_nxstaticMergeconflictDescription;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRMergeConflictDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetFieldName(const CString& strFieldName);
	void AddDetail(const CString& strCollectionName,
		const CString& strFormattedData,
		long nCollectionID);
	long GetSelectedDetailID();

protected:
	NXDATALISTLib::_DNxDataListPtr m_dlDetails;
	CString m_strFieldName;
	CStringArray m_astrDetails;
	CStringArray m_astrFormattedData;
	CDWordArray m_adwDetailIDs;
	long m_nSelectedDetailID;

	// Generated message map functions
	//{{AFX_MSG(CEMRMergeConflictDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblClickCellListDetailConflicts(long nRowIndex, short nColIndex);
	afx_msg void OnSelChangedListDetailConflicts(long nNewSel);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRMERGECONFLICTDLG_H__01AE492E_3718_41BD_B252_6F2883BEFABA__INCLUDED_)
