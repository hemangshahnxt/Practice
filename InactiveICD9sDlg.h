#if !defined(AFX_INACTIVEICD9SDLG_H__9FE1FB95_2BCD_4771_BD6A_AD4C90F86E22__INCLUDED_)
#define AFX_INACTIVEICD9SDLG_H__9FE1FB95_2BCD_4771_BD6A_AD4C90F86E22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InactiveICD9sDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInactiveICD9sDlg dialog

class CInactiveICD9sDlg : public CNxDialog
{
// Construction
public:
	CInactiveICD9sDlg(CWnd* pParent);   // standard constructor

	void RestoreItem(long ID);
	NXDATALISTLib::_DNxDataListPtr m_List;
	// (j.kuziel 2014-03-10) - PLID 61213 - Added column for ICD-10. Let's keep it organized.
	enum InactiveDiagCodeColumns
	{
		eidccID = 0,
		eidccCodeNumber = 1,
		eidccCodeDesc = 2,
		eidccICD10 = 3,
	};


	BOOL m_Changed;

	// (z.manning, 05/01/2008) - PLID 29860 - Added NxIconButton
// Dialog Data
	//{{AFX_DATA(CInactiveICD9sDlg)
	enum { IDD = IDD_INACTIVE_ICD9S_DLG };
	CNxStatic	m_nxstaticInactiveTitleLabel;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInactiveICD9sDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInactiveICD9sDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnLeftClickInactiveIcd9List(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnDblClickCellInactiveIcd9List(long nRowIndex, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void RequeryFinishedInactiveIcd9List(short nFlags);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INACTIVEICD9SDLG_H__9FE1FB95_2BCD_4771_BD6A_AD4C90F86E22__INCLUDED_)
