#if !defined(AFX_EDITINSPAYERSDLG_H__DEE99F36_DE3A_41E7_B275_8AC3DC8280D8__INCLUDED_)
#define AFX_EDITINSPAYERSDLG_H__DEE99F36_DE3A_41E7_B275_8AC3DC8280D8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditInsPayersDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditInsPayersDlg dialog

class CEditInsPayersDlg : public CNxDialog
{
// Construction
public:

	NXDATALISTLib::_DNxDataListPtr m_PayerList;

	// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer IDs, and thus removed format type
	//long m_FormatType;

	CEditInsPayersDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditInsPayersDlg)
	enum { IDD = IDD_EDIT_INS_PAYERS_DLG };
	CNxIconButton m_btnOK;
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnDelete;
	CNxIconButton m_btnAdvanced;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditInsPayersDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void RefreshButtons();

	// Generated message map functions
	//{{AFX_MSG(CEditInsPayersDlg)
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	virtual BOOL OnInitDialog();
	afx_msg void OnEditingFinishedInsPayers(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingInsPayers(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnBtnAdvanced();
	afx_msg void OnSelChangedInsPayers(long nNewSel);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITINSPAYERSDLG_H__DEE99F36_DE3A_41E7_B275_8AC3DC8280D8__INCLUDED_)
