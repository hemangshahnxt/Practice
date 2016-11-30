//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_PAYCATDLG_H__B75136C1_45E4_11D4_84D9_00010243175D__INCLUDED_)
#define AFX_PAYCATDLG_H__B75136C1_45E4_11D4_84D9_00010243175D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PayCatDlg.h : header file
//

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CPayCatDlg dialog

class CPayCatDlg : public CNxDialog
{
// Construction
public:
	CPayCatDlg(CWnd* pParent);   // standard constructor

	// (z.manning, 04/30/2008) - PLID 29850 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CPayCatDlg)
	enum { IDD = IDD_ADMIN_PAYCAT_DLG };
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnClose;

	// (j.gruber 2012-11-19 09:16) - PLID 53766
	long m_nDefault;
	//}}AFX_DATA

	NXDATALISTLib::_DNxDataListPtr	m_pCategories;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPayCatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	BOOL IsDefaultSelected(); // (j.gruber 2012-11-16 10:35) - PLID 53766

	// Generated message map functions
	//{{AFX_MSG(CPayCatDlg)
	afx_msg void OnRepeatedLeftClickCategories(const VARIANT FAR& varBoundValue, long iColumn, long nClicks);
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnEditingFinishedCategories(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnSelChangedCategories(long nNewSel);
	afx_msg void OnEditingStartingList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnRButtonDownEditList(long nRow, short nCol, long x, long y, long nFlags); // (j.gruber 2012-11-16 13:57) - PLID 39557
	afx_msg void OnSetDefault();  // (j.gruber 2012-11-15 15:12) - PLID 53766
	afx_msg void OnRemoveDefault(); // (j.gruber 2012-11-15 15:12) - PLID 53766
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAYCATDLG_H__B75136C1_45E4_11D4_84D9_00010243175D__INCLUDED_)
