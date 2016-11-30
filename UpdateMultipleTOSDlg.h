#if !defined(AFX_UPDATEMULTIPLETOSDLG_H__1454371E_279B_468B_A529_AF1C3A104084__INCLUDED_)
#define AFX_UPDATEMULTIPLETOSDLG_H__1454371E_279B_468B_A529_AF1C3A104084__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UpdateMultipleTOSDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUpdateMultipleTOSDlg dialog

class CUpdateMultipleTOSDlg : public CNxDialog
{
// Construction
public:
	CUpdateMultipleTOSDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUpdateMultipleTOSDlg)
	enum { IDD = IDD_UPDATE_MULTIPLE_TOS_DLG };
	CNxIconButton	m_btnUnselOne;
	CNxIconButton	m_btnUnselAll;
	CNxIconButton	m_btnSelAll;
	CNxIconButton	m_btnSelOne;
	CNxEdit	m_nxeditTosCode;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnUpdate;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpdateMultipleTOSDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pAvail;
	NXDATALISTLib::_DNxDataListPtr m_pSelected;

	// Generated message map functions
	//{{AFX_MSG(CUpdateMultipleTOSDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelOne();
	afx_msg void OnSelAll();
	afx_msg void OnUnselOne();
	afx_msg void OnUnselAll();
	afx_msg void OnUpdateTos();
	afx_msg void OnDblClickCellAvailableTosList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectedTosList(long nRowIndex, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPDATEMULTIPLETOSDLG_H__1454371E_279B_468B_A529_AF1C3A104084__INCLUDED_)
