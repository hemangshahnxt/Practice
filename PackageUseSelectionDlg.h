#if !defined(AFX_PACKAGEUSESELECTIONDLG_H__CF3B3301_44EE_411F_BD52_68DD1866FF7C__INCLUDED_)
#define AFX_PACKAGEUSESELECTIONDLG_H__CF3B3301_44EE_411F_BD52_68DD1866FF7C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PackageUseSelectionDlg.h : header file
//

// (j.jones 2008-05-22 11:44) - PLID 28450 - added a proper memory object
struct PackageChargeObject {

	long nPackageChargeID;
	long nServiceID;
	double dblQuantity;
};

/////////////////////////////////////////////////////////////////////////////
// CPackageUseSelectionDlg dialog

class CPackageUseSelectionDlg : public CNxDialog
{
// Construction
public:
	CPackageUseSelectionDlg(CWnd* pParent);   // standard constructor
	~CPackageUseSelectionDlg();

	NXDATALISTLib::_DNxDataListPtr m_PackageList;

	long m_nQuoteID;

	// (j.jones 2008-05-22 11:44) - PLID 28450 - added a new package charge array
	// that replaced the old service & quantity arrays
	CArray<PackageChargeObject*, PackageChargeObject*> m_arypPackageCharges;

	BOOL m_bPackageIsUsedUp;

	// (j.jones 2007-03-26 14:37) - PLID 25287 - used to determine if we need to
	// offset our bill amount to match the remaining package amount
	COleCurrency m_cyMatchRemainingPackageAmount;
	//used with the above to determine which service ID to assign the matched amount to
	// (j.jones 2011-02-03 09:12) - PLID 42291 - changed to calculate the package charge ID, not service ID
	long m_nMatchRemAmountToChargeID;

// Dialog Data
	//{{AFX_DATA(CPackageUseSelectionDlg)
	enum { IDD = IDD_PACKAGE_USE_SELECTION_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPackageUseSelectionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (j.jones 2008-05-22 11:45) - PLID 28450 - this function will properly clear m_arypPackageCharges
	void ClearPackageChargeArray();

	// Generated message map functions
	//{{AFX_MSG(CPackageUseSelectionDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnEditingStartingPackageList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishingPackageList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedPackageList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRequeryFinishedPackageList(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PACKAGEUSESELECTIONDLG_H__CF3B3301_44EE_411F_BD52_68DD1866FF7C__INCLUDED_)
