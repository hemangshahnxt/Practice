#if !defined(AFX_MERGELADDERDLG_H__FB6E2554_69F7_48BB_888E_A7706ED949DB__INCLUDED_)
#define AFX_MERGELADDERDLG_H__FB6E2554_69F7_48BB_888E_A7706ED949DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MergeLadderDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMergeLadderDlg dialog

class CMergeLadderDlg : public CNxDialog
{
// Construction
public:
	CMergeLadderDlg(long nLadderID, long nLadderTemplateID,	long nPersonID, CWnd* pParent);   // standard constructor
	CString GetLadderDescription(long nLadderID);

// Dialog Data
	//{{AFX_DATA(CMergeLadderDlg)
	enum { IDD = IDD_MERGE_LADDER_DLG };
		// NOTE: the ClassWizard will add data members here
	CNxEdit	m_nxeditLadderName;
	CNxEdit	m_nxeditLadderStartDate;
	CNxEdit	m_nxeditLadderStatus;
	CNxEdit	m_nxeditLadderAction;
	CNxStatic	m_nxstaticInfo;
	CNxStatic	m_nxstaticProcs;
	CNxStatic	m_nxstaticMergeStart;
	CNxStatic	m_nxstaticStatus;
	CNxStatic	m_nxstaticAction;
	CNxStatic	m_nxstaticSelect;
	CNxIconButton	m_btnMerge;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMergeLadderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	long m_nLadderID;
	long m_nLadderTemplateID;
	long m_nPersonID;
	CBrush m_brush;

	NXDATALIST2Lib::_DNxDataListPtr m_pLadders;
	// Generated message map functions
	//{{AFX_MSG(CMergeLadderDlg)
	afx_msg void OnMerge();
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MERGELADDERDLG_H__FB6E2554_69F7_48BB_888E_A7706ED949DB__INCLUDED_)
