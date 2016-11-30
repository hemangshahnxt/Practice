#if !defined(AFX_INACTIVETYPESDLG_H__9AA26B24_FE5E_4BF2_BEEB_D1178F6163E1__INCLUDED_)
#define AFX_INACTIVETYPESDLG_H__9AA26B24_FE5E_4BF2_BEEB_D1178F6163E1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InactiveTypesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInactiveTypesDlg dialog
class CInactiveTypesDlg : public CNxDialog
{
// Construction
public:
	CInactiveTypesDlg(CWnd* pParent);   // standard constructor

	// (z.manning, 05/01/2008) - PLID 29860 - Added NxIconButton
// Dialog Data
	//{{AFX_DATA(CInactiveTypesDlg)
	enum { IDD = IDD_INACTIVE_TYPES_DLG };
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInactiveTypesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pInactiveTypes;

	// Generated message map functions
	//{{AFX_MSG(CInactiveTypesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblClickCellInactiveTypes(long nRowIndex, short nColIndex);
	virtual void OnOK();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INACTIVETYPESDLG_H__9AA26B24_FE5E_4BF2_BEEB_D1178F6163E1__INCLUDED_)
