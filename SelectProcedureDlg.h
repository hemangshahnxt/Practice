#if !defined(AFX_SELECTPROCEDUREDLG_H__36E313E1_5894_475B_A0A3_8CE04C17A6DB__INCLUDED_)
#define AFX_SELECTPROCEDUREDLG_H__36E313E1_5894_475B_A0A3_8CE04C17A6DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectProcedureDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectProcedureDlg dialog

class CSelectProcedureDlg : public CNxDialog
{
// Construction
public:
	CSelectProcedureDlg(CWnd* pParent);   // standard constructor

	long m_nProcedureID;
	CString m_strProcedureName;

	// (a.walling 2007-03-27 14:46) - PLID 25376 - Modularize this dialog
	BOOL m_bIncludeNoProcedure;
	CString m_strCaption;
	CString m_strBadSelectionWarning;

	// (a.walling 2007-03-27 14:46) - PLID 25376 - Modularize this dialog
	HRESULT Open(CString strCaption, BOOL bIncludeNoProcedure = TRUE, CString strBadSelectionWarning = "Please choose an item from the list.");

// Dialog Data
	//{{AFX_DATA(CSelectProcedureDlg)
	enum { IDD = IDD_SELECT_PROCEDURE };
		// NOTE: the ClassWizard will add data members here
	CNxStatic	m_nxstaticSelectProcedureCaption;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectProcedureDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pProcList;

	// Generated message map functions
	//{{AFX_MSG(CSelectProcedureDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTPROCEDUREDLG_H__36E313E1_5894_475B_A0A3_8CE04C17A6DB__INCLUDED_)
