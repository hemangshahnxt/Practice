#if !defined(AFX_EDITDRAWERSDLG_H__30BCF32B_6289_4873_A82B_10BB500C94E2__INCLUDED_)
#define AFX_EDITDRAWERSDLG_H__30BCF32B_6289_4873_A82B_10BB500C94E2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditDrawersDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditDrawersDlg dialog

class CEditDrawersDlg : public CNxDialog
{
// Construction
public:
	CEditDrawersDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditDrawersDlg)
	enum { IDD = IDD_EDIT_DRAWERS_DLG };
	NxButton	m_btnCurrentLoc;
	NxButton	m_btnAllLoc;
	NxButton	m_btnIncludeClosed;
	CNxIconButton m_btnNewDrawer;
	CNxIconButton m_btnCloseDrawer;
	CNxIconButton m_btnEditDrawer;
	CNxIconButton m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditDrawersDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pList;

	// Generated message map functions
	//{{AFX_MSG(CEditDrawersDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnNewDrawer();
	afx_msg void OnCloseDrawer();
	afx_msg void OnEditingFinishingDrawerList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedDrawerList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnIncludeClosed();
	afx_msg void OnEditDrawer();
	afx_msg void OnSelChosenDrawerList(long nRow);
	afx_msg void OnSelChangedDrawerList(long nNewSel);
	// (j.gruber 2007-08-09 12:31) - PLID 25119 - add locations to cash drawers
	afx_msg void OnCurrentLocation();
	afx_msg void OnAllLocations();
	// (a.walling 2007-09-28 12:01) - PLID 27468 - Open physical cash drawer
	afx_msg void OnOpenPhysicalCashDrawer();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITDRAWERSDLG_H__30BCF32B_6289_4873_A82B_10BB500C94E2__INCLUDED_)
