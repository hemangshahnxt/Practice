#if !defined(AFX_EMRTEMPLATEMANAGERDLG_H__6545216B_DEFE_46D0_B9E3_606BD45BDEEB__INCLUDED_)
#define AFX_EMRTEMPLATEMANAGERDLG_H__6545216B_DEFE_46D0_B9E3_606BD45BDEEB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrTemplateManagerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEmrTemplateManagerDlg dialog
class CPatientEMRDlg;

class CEmrTemplateManagerDlg : public CNxDialog
{
// Construction
public:
	CEmrTemplateManagerDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEmrTemplateManagerDlg)
	enum { IDD = IDD_EMR_TEMPLATE_MANAGER };
	CNxIconButton m_btnNewEmrTemplate;
	CNxIconButton m_btnCopyEmrTemplate;
	CNxIconButton m_btnDeleteEmrTemplate;
	CNxIconButton m_btnEmrTemplateEdit;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnSelectAll;
	CNxIconButton m_btnUnselectAll;
	CNxIconButton m_btnKeywordFilter;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrTemplateManagerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	// (j.jones 2014-08-08 16:59) - PLID 63182 - this function had nothing in it, so I removed it
	//void OnTableChanged(WPARAM wParam, LPARAM lParam);

protected:
	NXDATALISTLib::_DNxDataListPtr m_pTemplateList;

	CWnd *m_pParent;

	void RequeryTemplateList();
	long m_nSavedTopRow;

	//TES 3/2/2009 - PLID 33102 - Needed for keyword filtering
	CString m_strDefaultWhereClause;

	// Generated message map functions
	// (j.gruber 2009-10-30 11:15) - PLID 38506 - added nexweb template functions
	//{{AFX_MSG(CEmrTemplateManagerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangedEmrTemplates(long nNewSel);
	afx_msg void OnEmrTemplateEdit();
	afx_msg void OnDeleteEmrTemplate();
	afx_msg void OnCopyEmrTemplate();
	afx_msg void OnEditingFinishingEmrTemplates(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedEmrTemplates(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnDblClickCellEmrTemplates(long nRowIndex, short nColIndex);
	afx_msg void OnRButtonDownEmrTemplates(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnNewEmrTemplate();
	afx_msg void OnEmnTemplatesSelectAll();
	afx_msg void OnEmnTemplatesUnselectAll();
	afx_msg void OnRequeryFinishedEmrTemplates(short nFlags);
	afx_msg void OnKeywordFilter();
	//(e.lally 2011-05-04) PLID 43537 - changed direct nexweb template management to a goto action
	afx_msg void OnGotoNexWebListDisplayManager();	
	afx_msg void OnEditCustomPreviewLayouts(); // (c.haag 2012-01-17) - PLID 54593
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRTEMPLATEMANAGERDLG_H__6545216B_DEFE_46D0_B9E3_606BD45BDEEB__INCLUDED_)
