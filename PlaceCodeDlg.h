//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_PLACECODEDLG_H__C1928681_317F_11D4_84D9_00010243175D__INCLUDED_)
#define AFX_PLACECODEDLG_H__C1928681_317F_11D4_84D9_00010243175D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlaceCodeDlg.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CPlaceCodeDlg dialog

class CPlaceCodeDlg : public CNxDialog
{
// Construction
public:
	CPlaceCodeDlg(CWnd* pParent);   // standard constructor

	// (z.manning, 05/01/2008) - PLID 29864 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CPlaceCodeDlg)
	enum { IDD = IDD_PLACE_CODE_DLG };
	NXDATALISTLib::_DNxDataListPtr	m_pCodes;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlaceCodeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPlaceCodeDlg)
	afx_msg void OnDelete();
	afx_msg void OnAdd();
	virtual BOOL OnInitDialog();
	afx_msg void OnEditingFinishingPlaceCodes(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedPlaceCodes(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLACECODEDLG_H__C1928681_317F_11D4_84D9_00010243175D__INCLUDED_)
