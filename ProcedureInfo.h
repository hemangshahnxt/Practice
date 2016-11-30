#pragma once

// ProcedureInfo.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CProcedureInfo dialog

class CProcedureInfo : public CNxDialog
{
// Construction
public:
	CProcedureInfo(CWnd* pParent);   // standard constructor
	CArray<int, int> m_arProcIDs;

	// (d.thompson 2009-02-17) - PLID 3843 - Enable this if you want the preview functionality
	bool m_bAllowPreview;

// Dialog Data
	//{{AFX_DATA(CProcedureInfo)
	enum { IDD = IDD_PROCEDURE_INFO };
	CRichEditCtrl	m_text;
	CNxStatic	m_nxstaticProcCaption;
	CNxIconButton	m_nxbuttonPreview;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcedureInfo)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool m_closing;
	NXDATALISTLib::_DNxDataListPtr m_pProcSelect;
	// Generated message map functions
	//{{AFX_MSG(CProcedureInfo)
	afx_msg BOOL OnNcActivate(BOOL bActive);
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSelChosenProcSelect(long nRow);
	afx_msg void OnRequeryFinishedProcSelect(short nFlags);
	afx_msg void OnPreviewCheatSheetRpt();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};