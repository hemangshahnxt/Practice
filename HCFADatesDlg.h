#if !defined(AFX_HCFADATESDLG_H__05C1EE8A_BDA6_417A_AEDF_8C9DFE23559E__INCLUDED_)
#define AFX_HCFADATESDLG_H__05C1EE8A_BDA6_417A_AEDF_8C9DFE23559E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HCFADatesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHCFADatesDlg dialog

class CHCFADatesDlg : public CNxDialog
{
// Construction
public:

	NXDATALISTLib::_DNxDataListPtr m_Box11a, m_Box9b, m_Box3;

	void SetRadio (CButton &radioTrue, CButton &radioFalse, const bool isTrue);
	void Load();
	CHCFADatesDlg(CWnd* pParent);   // standard constructor

	long m_HCFASetupGroupID;
	CString m_strGroupName;

	// (z.manning, 04/30/2008) - PLID 29860 - Added NxIconButton
// Dialog Data
	//{{AFX_DATA(CHCFADatesDlg)
	enum { IDD = IDD_HCFA_DATES_DLG };
	NxButton	m_checkBox9B_Wide;
	NxButton	m_radioBox9B_4;
	NxButton	m_radioBox9B_2;
	NxButton	m_checkBox31_Wide;
	NxButton	m_radioBox31_4;
	NxButton	m_radioBox31_2;
	NxButton	m_checkBox3_Wide;
	NxButton	m_radioBox3_4;
	NxButton	m_radioBox3_2;
	NxButton	m_checkBox24T_Wide;
	NxButton	m_radioBox24T_4;
	NxButton	m_radioBox24T_2;
	NxButton	m_checkBox24F_Wide;
	NxButton	m_radioBox24F_4;
	NxButton	m_radioBox24F_2;
	NxButton	m_checkBox18_Wide;
	NxButton	m_radioBox18_4;
	NxButton	m_radioBox18_2;
	NxButton	m_checkBox16_Wide;
	NxButton	m_radioBox16_4;
	NxButton	m_radioBox16_2;
	NxButton	m_checkBox15_Wide;
	NxButton	m_radioBox15_4;
	NxButton	m_radioBox15_2;
	NxButton	m_checkBox14_Wide;
	NxButton	m_radioBox14_4;
	NxButton	m_radioBox14_2;
	NxButton	m_checkBox12_Wide;
	NxButton	m_radioBox12_4;
	NxButton	m_radioBox12_2;
	NxButton	m_checkBox11A_Wide;
	NxButton	m_radioBox11A_4;
	NxButton	m_radioBox11A_2;
	NxButton	m_radioBox12UsePrintDate;
	NxButton	m_radioBox12UseBillDate;
	NxButton	m_radioBox31UsePrintDate;
	NxButton	m_radioBox31UseBillDate;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHCFADatesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UpdateTable(CString BoxName, long data);
	void BuildDateCombos();

	// Generated message map functions
	//{{AFX_MSG(CHCFADatesDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelChosenBox3(long nRow);
	afx_msg void OnSelChosenBox9b(long nRow);
	afx_msg void OnSelChosenBox11a(long nRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HCFADATESDLG_H__05C1EE8A_BDA6_417A_AEDF_8C9DFE23559E__INCLUDED_)
