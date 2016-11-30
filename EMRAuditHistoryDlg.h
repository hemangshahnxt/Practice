#if !defined(AFX_EMRAUDITHISTORYDLG_H__635CC731_A528_4587_A3AB_6F03D15533F5__INCLUDED_)
#define AFX_EMRAUDITHISTORYDLG_H__635CC731_A528_4587_A3AB_6F03D15533F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRAuditHistoryDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEMRAuditHistoryDlg dialog

class CEMRAuditHistoryDlg : public CNxDialog
{
// Construction
public:
	CEMRAuditHistoryDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_List;

	long m_nEMRID;
	long m_nEMNID;
	long m_nEMRTemplateID;

// Dialog Data
	//{{AFX_DATA(CEMRAuditHistoryDlg)
	enum { IDD = IDD_EMR_AUDIT_HISTORY_DLG };
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCopyOutput; // (b.cardillo 2010-01-07 13:26) - PLID 35780
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRAuditHistoryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEMRAuditHistoryDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCopyOutputBtn(); // (b.cardillo 2010-01-07 13:26) - PLID 35780
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRAUDITHISTORYDLG_H__635CC731_A528_4587_A3AB_6F03D15533F5__INCLUDED_)
