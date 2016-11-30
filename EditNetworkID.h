#if !defined(AFX_EDITNETWORKID_H__B4CA71A5_1551_4359_83E5_96D25F8235F9__INCLUDED_)
#define AFX_EDITNETWORKID_H__B4CA71A5_1551_4359_83E5_96D25F8235F9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditNetworkID.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditNetworkID dialog

class CEditNetworkID : public CNxDialog
{
// Construction
public:
	CString m_strInsCo;
	NXDATALISTLib::_DNxDataListPtr m_ComboProviders;
	long m_iInsuranceCoID;
	long m_iProviderID;
	void LoadNetworkID();
	void SaveNetworkID();
	CEditNetworkID(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditNetworkID)
	enum { IDD = IDD_EDIT_NETWORKID };
	CString	m_NetworkID;
	CNxEdit	m_nxeditEditNetworkid;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnAdvNetworkIDEdit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditNetworkID)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditNetworkID)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenComboProviders(long nRow);
	afx_msg void OnRequeryFinishedComboProviders(short nFlags);
	afx_msg void OnKillfocusEditNetworkid();
	afx_msg void OnAdvNetworkidEdit();
	virtual void OnOK();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITNETWORKID_H__B4CA71A5_1551_4359_83E5_96D25F8235F9__INCLUDED_)
