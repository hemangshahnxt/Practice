#if !defined(AFX_MULTITEMPLATEDLG_H__5CDD2672_AAE3_4D02_9587_5C4A906D3A1B__INCLUDED_)
#define AFX_MULTITEMPLATEDLG_H__5CDD2672_AAE3_4D02_9587_5C4A906D3A1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MultiTemplateDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMultiTemplateDlg dialog
class CMultiTemplateDlg : public CNxDialog
{
// Construction
public:
	CMultiTemplateDlg(CWnd* pParent);   // standard constructor
	// (j.armen 2012-01-23 08:59) - PLID 47707 - Replaced CDWordArray with CArray<long, long>
	CArray<long, long> m_aryTemplateIDs;

	// (z.manning, 05/01/2008) - PLID 29864 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CMultiTemplateDlg)
	enum { IDD = IDD_MULTI_TEMPLATE_DLG };
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnRemove;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiTemplateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pTemplates;

	// Generated message map functions
	//{{AFX_MSG(CMultiTemplateDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddTemplate();
	afx_msg void OnRemoveTemplate();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTITEMPLATEDLG_H__5CDD2672_AAE3_4D02_9587_5C4A906D3A1B__INCLUDED_)
