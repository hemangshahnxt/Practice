#if !defined(AFX_FAXCHOOSERECIPIENTDLG_H__AD968635_C24F_4A71_B7BA_D815B7A1C52C__INCLUDED_)
#define AFX_FAXCHOOSERECIPIENTDLG_H__AD968635_C24F_4A71_B7BA_D815B7A1C52C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FaxChooseRecipientDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFaxChooseRecipientDlg dialog
//DRT 7/2/2008 - PLID 30601 - Created.

class CFaxChooseRecipientDlg : public CNxDialog
{
// Construction
public:
	CFaxChooseRecipientDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFaxChooseRecipientDlg)
	enum { IDD = IDD_FAX_CHOOSE_RECIPIENT_DLG };
	CNxEdit	m_editOtherNum;
	CNxEdit	m_editOtherName;
	NxButton	m_btnPerson;
	NxButton	m_btnRefPhys;
	NxButton	m_btnPCP;
	NxButton	m_btnInsCo;
	NxButton	m_btnContact;
	NxButton	m_btnLocation;
	NxButton	m_btnOther;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA

	//Exposed member variables
	//
	//	Input
	//
	//The PersonID that the fax is intended toward.  Leave as <= 0 if you are not launching this
	//	for a person.
	long m_nPersonID;

	//
	//	Output
	//
	//The fax recipient name to use.
	CString m_strName;
	//The fax recipient number to send to.
	CString m_strNumber;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFaxChooseRecipientDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//Controls
	NXDATALIST2Lib::_DNxDataListPtr m_pInsList;
	NXDATALIST2Lib::_DNxDataListPtr m_pContactList;
	NXDATALIST2Lib::_DNxDataListPtr m_pLocList;

	//Member functions
	void EnsureControls();
	void LoadData();

	//These members are only valid after LoadData() and before OnOK() is finished
	CString m_strPatName, m_strPatNum;
	CString m_strRefPhysName, m_strRefPhysNum;
	CString m_strPCPName, m_strPCPNum;

	// Generated message map functions
	//{{AFX_MSG(CFaxChooseRecipientDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnRecipPerson();
	afx_msg void OnRecipRefphys();
	afx_msg void OnRecipPcp();
	afx_msg void OnRecipInsco();
	afx_msg void OnRecipContact();
	afx_msg void OnRecipLocation();
	afx_msg void OnRecipOther();
	afx_msg void OnRequeryFinishedRecipInscoList(short nFlags);
	afx_msg void OnRequeryFinishedRecipContactList(short nFlags);
	afx_msg void OnRequeryFinishedRecipLocationList(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FAXCHOOSERECIPIENTDLG_H__AD968635_C24F_4A71_B7BA_D815B7A1C52C__INCLUDED_)
