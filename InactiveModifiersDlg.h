#if !defined(AFX_INACTIVEMODIFIERSDLG_H__5662C341_DDB3_4AC4_9394_0491B881C252__INCLUDED_)
#define AFX_INACTIVEMODIFIERSDLG_H__5662C341_DDB3_4AC4_9394_0491B881C252__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InactiveModifiersDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInactiveModifiersDlg dialog

// (z.manning, 05/02/2007) - PLID 16623 - Added a dialog to display and allow the reactivation of inactive modifiers.
class CInactiveModifiersDlg : public CNxDialog
{
// Construction
public:
	CInactiveModifiersDlg(CWnd* pParent);   // standard constructor

	// (z.manning, 05/01/2008) - PLID 29860 - Added NxIconButton for Close
// Dialog Data
	//{{AFX_DATA(CInactiveModifiersDlg)
	enum { IDD = IDD_INACTIVE_MODIFIERS };
	CNxStatic	m_nxstaticInactiveTitleLabel;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInactiveModifiersDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALISTLib::_DNxDataListPtr m_pdlList;

	// Generated message map functions
	//{{AFX_MSG(CInactiveModifiersDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblClickCellInactiveModifierList(long nRowIndex, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INACTIVEMODIFIERSDLG_H__5662C341_DDB3_4AC4_9394_0491B881C252__INCLUDED_)
