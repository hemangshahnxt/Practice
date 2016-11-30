#if !defined(AFX_SELECTMASTERDLG_H__5711F5F5_9AE8_4CD4_B7D5_DD7E1A8B7F5F__INCLUDED_)
#define AFX_SELECTMASTERDLG_H__5711F5F5_9AE8_4CD4_B7D5_DD7E1A8B7F5F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectMasterDlg.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CSelectMasterDlg dialog

class CSelectMasterDlg : public CNxDialog
{
// Construction
public:
	CSelectMasterDlg(CWnd* pParent);   // standard constructor

	CString m_strName;
	long m_nMasterID;
	long m_nDetailID;
	BOOL m_bIsTracked;
// Dialog Data
	//{{AFX_DATA(CSelectMasterDlg)
	enum { IDD = IDD_SELECT_MASTER_DLG };
	CNxStatic	m_nxstaticPromptText;
	CNxStatic	m_nxstaticProcWarning;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectMasterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pMasterProcs;

	// Generated message map functions
	//{{AFX_MSG(CSelectMasterDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTMASTERDLG_H__5711F5F5_9AE8_4CD4_B7D5_DD7E1A8B7F5F__INCLUDED_)
