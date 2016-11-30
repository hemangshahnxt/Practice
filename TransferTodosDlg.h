#if !defined(AFX_TRANSFERTODOSDLG_H__E95257E2_3E94_4184_B1AD_53A7D5815EEC__INCLUDED_)
#define AFX_TRANSFERTODOSDLG_H__E95257E2_3E94_4184_B1AD_53A7D5815EEC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TransferTodosDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTransferTodosDlg dialog

class CTransferTodosDlg : public CNxDialog
{
// Construction
public:
	CTransferTodosDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTransferTodosDlg)
	enum { IDD = IDD_TRANSFER_TODOS };
	NxButton	m_btnAllPatients;
	NxButton	m_btnSinglePatient;
	NxButton	m_btnAll;
	NxButton	m_btnUnfinished;
	NxButton	m_btnFromGroupbox;
	NxButton	m_btnToGroupbox;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTransferTodosDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALISTLib::_DNxDataListPtr m_dlTransferFromUserCombo;
	NXDATALISTLib::_DNxDataListPtr m_dlTransferToUserCombo;
	NXDATALISTLib::_DNxDataListPtr m_dlForPatientCombo;

	void BuildWhereClause(CString &strWhere);
	BOOL ConfirmUserPermissions();

	// Generated message map functions
	//{{AFX_MSG(CTransferTodosDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnTransfer();
	afx_msg void OnSinglePatientRadio();
	afx_msg void OnAllPatientsRadio();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRANSFERTODOSDLG_H__E95257E2_3E94_4184_B1AD_53A7D5815EEC__INCLUDED_)
