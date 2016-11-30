#if !defined(AFX_EMRCHOOSETEMPLATEDLG_H__664CF125_5B30_4CCD_9D4A_E30A5A667D11__INCLUDED_)
#define AFX_EMRCHOOSETEMPLATEDLG_H__664CF125_5B30_4CCD_9D4A_E30A5A667D11__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRChooseTemplateDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEMRChooseTemplateDlg dialog

class CEMRChooseTemplateDlg : public CNxDialog
{
// Construction
public:
	CEMRChooseTemplateDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEMRChooseTemplateDlg)
	enum { IDD = IDD_EMR_CHOOSE_TEMPLATE_DLG };
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	//}}AFX_DATA

	void AddProcedureFilter(long nProcedureID);
	long m_nSelectedTemplateID;

	BOOL m_bSilentReload;//If there are no templates, don't pop up that message.


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRChooseTemplateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//Members
	NXDATALIST2Lib::_DNxDataListPtr m_pList;
	CArray<long, long> m_aryFilteredProcedures;
	BOOL m_bFilterOn;	//Are we filtered on specific procedures?  Default TRUE
	BOOL m_bNoFilteredTemplates;	//Are there no filtered templates for the given procedure(s)?  Default FALSE

	//Functions
	void Reload();


	// Generated message map functions
	//{{AFX_MSG(CEMRChooseTemplateDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnRequeryFinishedTemplateList(short nFlags);
	afx_msg void OnDblClickCellTemplateList(LPDISPATCH lpRow, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRCHOOSETEMPLATEDLG_H__664CF125_5B30_4CCD_9D4A_E30A5A667D11__INCLUDED_)
