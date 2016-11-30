#if !defined(AFX_NEXWEBPATIENTNOTESDLG_H__9EE7797F_86C7_464C_A59F_26C7040F3C12__INCLUDED_)
#define AFX_NEXWEBPATIENTNOTESDLG_H__9EE7797F_86C7_464C_A59F_26C7040F3C12__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NexWebPatientNotesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNexWebPatientNotesDlg dialog

class CNexWebPatientNotesDlg : public CNxDialog
{
// Construction
public:
	CNexWebPatientNotesDlg(CWnd* pParent);   // standard constructor
	BOOL SaveInfo(long nPersonID = -1);
	BOOL ValidateData();
	void SetPersonID(long nPersonID, BOOL bIsNewPatient);
	long m_nPersonID;
	BOOL m_bIsNewPatient;
	NXDATALIST2Lib::_DNxDataListPtr m_pNoteList;
	void LoadNoteList();
	CString m_strError;

// Dialog Data
	//{{AFX_DATA(CNexWebPatientNotesDlg)
	enum { IDD = IDD_NEXWEB_PATIENT_NOTES_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNexWebPatientNotesDlg)
	protected:
	virtual void OnOK();
	virtual void OnCancel();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNexWebPatientNotesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblClickCellNexwebPatNotes(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnEditingFinishingNexwebPatNotes(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedNexwebPatNotes(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEXWEBPATIENTNOTESDLG_H__9EE7797F_86C7_464C_A59F_26C7040F3C12__INCLUDED_)
