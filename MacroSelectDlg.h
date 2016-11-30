#if !defined(AFX_MACROSELECTDLG_H__5F1F78FA_E2FD_4D5F_8836_337FD70EB4CC__INCLUDED_)
#define AFX_MACROSELECTDLG_H__5F1F78FA_E2FD_4D5F_8836_337FD70EB4CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MacroSelectDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMacroSelectDlg dialog

class CMacroSelectDlg : public CNxDialog
{
// Construction
public:
	CMacroSelectDlg(CWnd* pParent);   // standard constructor
	
// Dialog Data
	//{{AFX_DATA(CMacroSelectDlg)
	enum { IDD = IDD_MACRO_SELECT };
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	//}}AFX_DATA

	long m_nSelectedID;

	
	// (a.walling 2011-05-10 10:05) - PLID 41789 - Whether this is for billing notes or not
	bool m_bForBillingNotes;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMacroSelectDlg)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_dlMacroList;

	// Generated message map functions
	//{{AFX_MSG(CMacroSelectDlg)
	afx_msg void OnEditMacros();
	afx_msg void OnDblClickCellMacroList(long nRowIndex, short nColIndex);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelChangedMacroList(long nNewSel);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MACROSELECTDLG_H__5F1F78FA_E2FD_4D5F_8836_337FD70EB4CC__INCLUDED_)
