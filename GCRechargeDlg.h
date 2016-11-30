#if !defined(AFX_GCRECHARGEDLG_H__CBC6CEAF_2923_496B_9374_BA9584BC7D2F__INCLUDED_)
#define AFX_GCRECHARGEDLG_H__CBC6CEAF_2923_496B_9374_BA9584BC7D2F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GCRechargeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGCRechargeDlg dialog

class CGCRechargeDlg : public CNxDialog
{
// Construction
public:
	CGCRechargeDlg(CWnd* pParent);   // standard constructor
	void SetPatient(long nID);
//	void SetService(long nID);
//	void SetAmount(COleCurrency cy);

	long m_nID;
	// (r.gonet 2015-07-10) - PLID 65279 - Save the certificate number as a member variable for callers.
	CString m_strCertNumber;
	bool m_bIsExpired;

// Dialog Data
	//{{AFX_DATA(CGCRechargeDlg)
	enum { IDD = IDD_GC_RECHARGE_DLG };
	NxButton	m_btnIncludeExpired;
	NxButton	m_btnOnlyThisPerson;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGCRechargeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pList;
	long m_nPatientID;

	void UpdateListFilters(); // update the datalist with checkbox options

	// Generated message map functions
	//{{AFX_MSG(CGCRechargeDlg)
	afx_msg void OnRechargeFilter();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRechargeExpired();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GCRECHARGEDLG_H__CBC6CEAF_2923_496B_9374_BA9584BC7D2F__INCLUDED_)
