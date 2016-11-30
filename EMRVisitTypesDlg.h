#if !defined(AFX_EMRVISITTYPESDLG_H__53FA2951_65B2_4472_A412_B8BCE1CF2283__INCLUDED_)
#define AFX_EMRVISITTYPESDLG_H__53FA2951_65B2_4472_A412_B8BCE1CF2283__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRVisitTypesDlg.h : header file
//

// (j.jones 2007-08-16 08:39) - PLID 27054 - created

/////////////////////////////////////////////////////////////////////////////
// CEMRVisitTypesDlg dialog

class CEMRVisitTypesDlg : public CNxDialog
{
// Construction
public:
	CEMRVisitTypesDlg(CWnd* pParent);   // standard constructor

	NXDATALIST2Lib::_DNxDataListPtr m_VisitTypeList;

// Dialog Data
	//{{AFX_DATA(CEMRVisitTypesDlg)
	enum { IDD = IDD_EMR_VISIT_TYPES_DLG };
	NxButton	m_checkShowInactive;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRVisitTypesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEMRVisitTypesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnEditingStartingEmrVisitTypesList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishingEmrVisitTypesList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedEmrVisitTypesList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnCheckShowInactiveVisitTypes();
	afx_msg void OnBtnAddVisitType();
	afx_msg void OnBtnRemoveVisitType();	
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRVISITTYPESDLG_H__53FA2951_65B2_4472_A412_B8BCE1CF2283__INCLUDED_)
