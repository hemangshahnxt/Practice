#if !defined(AFX_APPLIEDSUPERBILLSDLG_H__C9AC5645_8A98_493E_8FC7_95BE3B9A39F3__INCLUDED_)
#define AFX_APPLIEDSUPERBILLSDLG_H__C9AC5645_8A98_493E_8FC7_95BE3B9A39F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AppliedSuperbillsDlg.h : header file
//

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CAppliedSuperbillsDlg dialog

class CAppliedSuperbillsDlg : public CNxDialog
{
// Construction
public:
	CAppliedSuperbillsDlg(CWnd* pParent);   // standard constructor

	long m_nCurrentPatientID;
	long m_nCurrentBillID;

// Dialog Data
	//{{AFX_DATA(CAppliedSuperbillsDlg)
	enum { IDD = IDD_APPLIED_SUPERBILL_DLG };
		// NOTE: the ClassWizard will add data members here
	CNxStatic	m_nxstaticBillidText;
	CNxIconButton m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAppliedSuperbillsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALISTLib::_DNxDataListPtr m_listApplies;

	// Generated message map functions
	//{{AFX_MSG(CAppliedSuperbillsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnEditingFinishedApplyList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPLIEDSUPERBILLSDLG_H__C9AC5645_8A98_493E_8FC7_95BE3B9A39F3__INCLUDED_)
