#if !defined(AFX_EMRHEADERDLG_H__CAF82554_A608_44A6_AC22_C2EE9CFC766D__INCLUDED_)
#define AFX_EMRHEADERDLG_H__CAF82554_A608_44A6_AC22_C2EE9CFC766D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrHeaderDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEmrHeaderDlg dialog

class CEmrHeaderDlg : public CNxDialog
{
// Construction
public:
	CEmrHeaderDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEmrHeaderDlg)
	enum { IDD = IDD_EMR_HEADER_DLG };
		// NOTE: the ClassWizard will add data members here
	CNxEdit	m_nxeditHeaderTitle;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrHeaderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pField1, m_pField2, m_pField3, m_pField4;

	// Generated message map functions
	//{{AFX_MSG(CEmrHeaderDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelChosenEmrHeaderField1(long nRow);
	afx_msg void OnSelChosenEmrHeaderField2(long nRow);
	afx_msg void OnSelChosenEmrHeaderField3(long nRow);
	afx_msg void OnSelChosenEmrHeaderField4(long nRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRHEADERDLG_H__CAF82554_A608_44A6_AC22_C2EE9CFC766D__INCLUDED_)
