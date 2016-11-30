#if !defined(AFX_ANSIREPORTPARSERDLG_H__B98530D4_EE0F_4FED_B147_E4A4E4D547C2__INCLUDED_)
#define AFX_ANSIREPORTPARSERDLG_H__B98530D4_EE0F_4FED_B147_E4A4E4D547C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CANSIReportParserDlg dialog

class CANSIReportParserDlg : public CNxDialog
{
// Construction
public:

	CFile m_InputFile,
		  m_OutputFile;

	CString ParseSection(CString &strIn, char chDelimiter);
	CString ParseSegment(CString &strIn);
	CString ParseElement(CString &strIn);
	CString ParseComposite(CString &strIn);

	//these functions make up the 997 file
	BOOL ANSI_ST(CString &strIn);
	void ANSI_AK1(CString &strIn);
	void ANSI_AK2(CString &strIn);
	void ANSI_AK3(CString &strIn);
	void ANSI_AK4(CString &strIn);
	void ANSI_AK5(CString &strIn);
	void ANSI_AK9(CString &strIn);
	void ANSI_SE(CString &strIn);

	//these are the interchange headers
	void ANSI_ISA(CString &strIn);
	void ANSI_IEA(CString &strIn);
	void ANSI_TA1(CString &strIn);
	void ANSI_GS(CString &strIn);
	void ANSI_GE(CString &strIn);

	CANSIReportParserDlg(CWnd* pParent);	// standard constructor

	// (j.jones 2008-05-07 10:49) - PLID 29854 - added nxiconbuttons for modernization
// Dialog Data
	//{{AFX_DATA(CANSIReportParserDlg)
	enum { IDD = IDD_ANSIREPORTPARSER_DIALOG };
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnParseFile;
	CNxEdit	m_nxeditFileInputName;
	CNxEdit	m_nxeditFileOutputName;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CANSIReportParserDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CANSIReportParserDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowseIn();
	afx_msg void OnParse();
	afx_msg void OnBrowseOut();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANSIREPORTPARSERDLG_H__B98530D4_EE0F_4FED_B147_E4A4E4D547C2__INCLUDED_)
