#if !defined(AFX_EYEGRAPHDLG_H__5B67A7F2_3DBA_46DB_A4A9_B1A9A13FD8F1__INCLUDED_)
#define AFX_EYEGRAPHDLG_H__5B67A7F2_3DBA_46DB_A4A9_B1A9A13FD8F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EyeGraphDlg.h : header file
//
#include "EyeGraphCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CEyeGraphDlg dialog

class CEyeGraphDlg : public CNxDialog
{
// Construction
public:
	CEyeGraphDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEyeGraphDlg)
	enum { IDD = IDD_EYE_GRAPH_DLG };
	NxButton	m_btnSEF;
	NxButton	m_btnDEF;
	NxButton	m_btnDatesAll;
	NxButton	m_btnDateRange;
	CEyeGraphCtrl m_EyeGraph;
	COleDateTime	m_dtTo;
	COleDateTime	m_dtFrom;
	CNxEdit	m_nxeditIdealPercent;
	CNxEdit	m_nxeditTotalEyes;
	CNxStatic	m_nxstaticIdealCaption;
	NxButton	m_btnDateGroupbox;
	CNxIconButton	m_btnUpdate;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEyeGraphDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//Strings used to construct the query.
	CString m_strValueCalc, m_strProcWhere, m_strVisitTypeWhere, m_strDateWhere, m_strProvWhere;
	CArray<DataPoint, DataPoint&> m_arPoints;
	NXDATALISTLib::_DNxDataListPtr m_pProcList, m_pVisitTypeList, m_pProvList;

	// Generated message map functions
	//{{AFX_MSG(CEyeGraphDlg)
	afx_msg void OnSef();
	afx_msg void OnDef();
	afx_msg void OnUpdate();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenProcList(long nRow);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelChosenVisitTypeList(long nRow);
	afx_msg void OnAllDates();
	afx_msg void OnEyeDateRange();
	afx_msg void OnDatetimechangeEyeDateFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEyeDateTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelChosenProvList(long nRow);
	afx_msg void OnPrint();
	afx_msg void OnRequeryFinishedVisitTypeList(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EYEGRAPHDLG_H__5B67A7F2_3DBA_46DB_A4A9_B1A9A13FD8F1__INCLUDED_)
