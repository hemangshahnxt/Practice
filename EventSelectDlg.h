#if !defined(AFX_EVENTSELECTDLG_H__99247EDF_3994_4FDB_A5A6_D8AF2627C89E__INCLUDED_)
#define AFX_EVENTSELECTDLG_H__99247EDF_3994_4FDB_A5A6_D8AF2627C89E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EventSelectDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEventSelectDlg dialog

class CEventSelectDlg : public CNxDialog
{
// Construction
public:
	CEventSelectDlg(CWnd* pParent);   // standard constructor

	PhaseTracking::PhaseAction m_nAction;
	long m_nStepTemplateID;
	long m_nPatientID;
	long m_nLadderID;
	NXDATALISTLib::_DNxDataListPtr m_pEventList;

	//Output.
	long m_nItemID;
	COleDateTime m_dtDate;

// Dialog Data
	//{{AFX_DATA(CEventSelectDlg)
	enum { IDD = IDD_EVENT_SELECT };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEventSelectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEventSelectDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDblClickCellEvents(long nRowIndex, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EVENTSELECTDLG_H__99247EDF_3994_4FDB_A5A6_D8AF2627C89E__INCLUDED_)
