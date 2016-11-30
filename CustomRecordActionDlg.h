#if !defined(AFX_CUSTOMRECORDACTIONDLG_H__32BDA6A3_1921_4A14_9EF0_C6D470895DFF__INCLUDED_)
#define AFX_CUSTOMRECORDACTIONDLG_H__32BDA6A3_1921_4A14_9EF0_C6D470895DFF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CustomRecordActionDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCustomRecordActionDlg dialog

#include "EmrUtils.h"

class CCustomRecordActionDlg : public CNxDialog
{
// Construction
public:
	CCustomRecordActionDlg(CWnd* pParent);   // standard constructor
	
	EmrActionObject m_SourceType;
	long m_nSourceID;


	// (z.manning, 04/30/2008) - PLID 29852 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CCustomRecordActionDlg)
	enum { IDD = IDD_CUSTOM_RECORD_ACTION_DLG };
	CNxStatic	m_nxstaticActionCaption;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustomRecordActionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALISTLib::_DNxDataListPtr m_pCptCombo, m_pCptList, m_pDiagCombo, m_pDiagList;

	// Generated message map functions
	//{{AFX_MSG(CCustomRecordActionDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelChosenActionCptCombo(long nRow);
	afx_msg void OnSelChosenActionDiagCombo(long nRow);
	afx_msg void OnRButtonUpActionCptList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonUpActionDiagList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRemoveCpt();
	afx_msg void OnRemoveDiag();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUSTOMRECORDACTIONDLG_H__32BDA6A3_1921_4A14_9EF0_C6D470895DFF__INCLUDED_)
