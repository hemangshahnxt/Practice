#if !defined(AFX_INPUTLISTDLG_H__C01CA85F_2827_480E_971D_22E058D5CF92__INCLUDED_)
#define AFX_INPUTLISTDLG_H__C01CA85F_2827_480E_971D_22E058D5CF92__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InputListDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInputListDlg dialog

class CInputListDlg : public CDialog
{
// Construction
public:
	CInputListDlg(CWnd* pParent);   // standard constructor

	// (z.manning, 04/30/2008) - PLID 29845 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CInputListDlg)
	enum { IDD = IDD_INPUT_LIST };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInputListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:

public:
	int ZoomList(IN const CString &strSql, IN BOOL bMultiSelect, IN OUT CDWordArray &aryIdList, OUT CString *pstrTextListing = NULL);

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_dlInputList;
	CDWordArray *m_paryIdList;
	CString m_strSql;
	CString *m_pstrTextList;
	BOOL m_bMultiSelect;

	// Generated message map functions
	//{{AFX_MSG(CInputListDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INPUTLISTDLG_H__C01CA85F_2827_480E_971D_22E058D5CF92__INCLUDED_)
