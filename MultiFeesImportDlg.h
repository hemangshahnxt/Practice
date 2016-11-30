#if !defined(AFX_MULTIFEESIMPORTDLG_H__6035C209_2F7D_4254_919C_0351EA4E3B0D__INCLUDED_)
#define AFX_MULTIFEESIMPORTDLG_H__6035C209_2F7D_4254_919C_0351EA4E3B0D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MultiFeesImportDlg.h : header file
//


#include "administratorRc.h"
#include "soaputils.h"

/////////////////////////////////////////////////////////////////////////////
// CMultiFeesImportDlg dialog

// (a.walling 2007-02-22 13:07) - PLID 2470 - Dialog allows importing and updating multi fees and standard fees
// from a simple .csv file. m_nFeeGroup should be set to the multifee group, and m_strFeeGroup to that group's name.
// If you set m_nFeeGroup to -1, then we will be modifying the ServiceT prices.

class CMultiFeesImportDlg : public CNxDialog
{
// Construction
public:
	
	CMultiFeesImportDlg(CWnd* pParent);   // standard constructor

	long m_nFeeGroup;
	CString m_strFeeGroup; //description of the fee group

	// (z.manning, 05/01/2008) - PLID 29864 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CMultiFeesImportDlg)
	enum { IDD = IDD_MULTI_FEES_IMPORT };
	NxButton	m_btnShowAllowable;
	NxButton	m_btnShowDesc;
	NxButton	m_btnShowChanges;
	CNxEdit	m_nxeditFeeFile;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxStatic	m_nxstaticFeeGroup;
	CNxIconButton m_btnFormatColumns;
	NxButton m_radioCSVFormat;
	NxButton m_radioXMLFormat;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiFeesImportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString GenerateSaveSql(OUT long &nAuditID);
	void ShowChanges(bool bShow);
	void ShowDescription(bool bShow);
	void ShowAllowed(bool bShow);
	
	// (j.gruber 2009-10-26 12:00) - PLID 35632 - moved some processing so both parsers could use it
	void SetOneLine(CString strCode, CString strFee, CString strAllow);

	CString m_strFile;
	bool m_bDataReady;

	bool m_bStandard; // if we are writing to standard fees and not multifees

	// (j.gruber 2009-10-22 16:57) - PLID 35632 - support XML files
	BOOL m_bProcessCSV;
	BOOL m_bProcessXML;
	BOOL m_bFormatSelected;
	CString m_strCodeField;
	CString m_strFeeField;
	CString m_strAllowableField;
	CString m_strParentNode;

	void ProcessXMLFile();
	CString GetValue(MSXML2::IXMLDOMNodePtr pParent, CString strPath);
	CString GetNodeName(CString strPath, CString &strRemainingPath);


	NXDATALIST2Lib::_DNxDataListPtr m_dlList;

	// Generated message map functions
	//{{AFX_MSG(CMultiFeesImportDlg)
	afx_msg void OnBrowseFeeFile();
	virtual BOOL OnInitDialog();
	afx_msg void OnProcessFeeFile();
	afx_msg void OnFeeShowChanges();
	afx_msg void OnFeeShowDescription();
	afx_msg void OnChangeFeeFile();
	afx_msg void OnSaveChanges();
	afx_msg void OnFeeIncludeAllow();
	afx_msg void OnFeeImportHelp();	
	afx_msg void OnBnClickedFormatColumns();
	afx_msg void OnBnClickedRadioXmlFormat();
	afx_msg void OnBnClickedRadioCsvFormat();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTIFEESIMPORTDLG_H__6035C209_2F7D_4254_919C_0351EA4E3B0D__INCLUDED_)
