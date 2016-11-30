#if !defined(AFX_LATINPRESCRIPTIONMACROSDLG_H__3E259739_7CFA_46B3_B9B7_6176804A4634__INCLUDED_)
#define AFX_LATINPRESCRIPTIONMACROSDLG_H__3E259739_7CFA_46B3_B9B7_6176804A4634__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LatinPrescriptionMacrosDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLatinPrescriptionMacrosDlg dialog

class CLatinPrescriptionMacrosDlg : public CNxDialog
{
// Construction
public:
	CLatinPrescriptionMacrosDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_Latin_List;

	CDWordArray m_dwaryDeleted;

// Dialog Data
	//{{AFX_DATA(CLatinPrescriptionMacrosDlg)
	enum { IDD = IDD_LATIN_PRESCRIPTION_MACROS_DLG };
	NxButton	m_btnShowConversion;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnAdd;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLatinPrescriptionMacrosDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLatinPrescriptionMacrosDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBtnAddNotation();
	afx_msg void OnBtnDeleteNotation();
	afx_msg void OnEditingFinishingLatinList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedLatinList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRButtonDownLatinList(long nRow, short nCol, long x, long y, long nFlags);
	virtual void OnCancel();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LATINPRESCRIPTIONMACROSDLG_H__3E259739_7CFA_46B3_B9B7_6176804A4634__INCLUDED_)
