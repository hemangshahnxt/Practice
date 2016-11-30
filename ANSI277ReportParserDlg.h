#if !defined(AFX_ANSI277REPORTPARSERDLG_H__C096D015_8176_4C7C_82C4_B46A3D37E234__INCLUDED_)
#define AFX_ANSI277REPORTPARSERDLG_H__C096D015_8176_4C7C_82C4_B46A3D37E234__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ANSI277ReportParserDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CANSI277ReportParserDlg dialog

class CANSI277ReportParserDlg : public CNxDialog
{
// Construction
public:

	CFile m_InputFile,
		  m_OutputFile;

	CString ParseSection(CString &strIn, char chDelimiter);
	CString ParseSegment(CString &strIn);
	CString ParseElement(CString &strIn);
	CString ParseComposite(CString &strIn);

	//these functions make up the 277 file
	BOOL ANSI_ST(CString &strIn);
	void ANSI_SE(CString &strIn);

	void ANSI_BHT(CString &strIn);
	void ANSI_HL(CString &strIn);
	void ANSI_NM1(CString &strIn);
	void ANSI_PER(CString &strIn);
	void ANSI_DMG(CString &strIn);
	void ANSI_TRN(CString &strIn);
	void ANSI_STC(CString &strIn);
	void ANSI_REF(CString &strIn);
	void ANSI_DTP(CString &strIn);
	void ANSI_SVC(CString &strIn);

	//these are the interchange headers
	void ANSI_ISA(CString &strIn);
	void ANSI_IEA(CString &strIn);
	void ANSI_TA1(CString &strIn);
	void ANSI_GS(CString &strIn);
	void ANSI_GE(CString &strIn);

	CString GetStatusCategoryCode(CString strCodeID);
	CString GetStatusCode(long CodeID);

	CANSI277ReportParserDlg(CWnd* pParent);   // standard constructor

	// (j.jones 2008-05-07 10:49) - PLID 29854 - added nxiconbuttons for modernization
// Dialog Data
	//{{AFX_DATA(CANSI277ReportParserDlg)
	enum { IDD = IDD_ANSI_277_REPORTPARSER_DIALOG };
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnParseFile;
	CNxEdit	m_nxeditFileInputName;
	CNxEdit	m_nxeditFileOutputName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CANSI277ReportParserDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CANSI277ReportParserDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowseIn();
	afx_msg void OnBrowseOut();
	afx_msg void OnParse();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANSI277REPORTPARSERDLG_H__C096D015_8176_4C7C_82C4_B46A3D37E234__INCLUDED_)
