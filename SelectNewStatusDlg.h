#if !defined(AFX_SELECTNEWSTATUSDLG_H__E8508DCE_8382_4FD2_B56F_AF9D8013A45B__INCLUDED_)
#define AFX_SELECTNEWSTATUSDLG_H__E8508DCE_8382_4FD2_B56F_AF9D8013A45B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectNewStatusDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectNewStatusDlg dialog

class CSelectNewStatusDlg : public CNxDialog
{
// Construction
public:
	CSelectNewStatusDlg(CWnd* pParent);   // standard constructor

	long m_nDoomedID, m_nNewID;

// Dialog Data
	//{{AFX_DATA(CSelectNewStatusDlg)
	enum { IDD = IDD_SELECT_NEW_STATUS };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxStatic	m_nxstaticNewStatusCap;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectNewStatusDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALISTLib::_DNxDataListPtr m_pNewStatusList;
	// Generated message map functions
	//{{AFX_MSG(CSelectNewStatusDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedNewStatuses(short nFlags);
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTNEWSTATUSDLG_H__E8508DCE_8382_4FD2_B56F_AF9D8013A45B__INCLUDED_)
