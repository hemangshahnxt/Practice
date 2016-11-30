#if !defined(AFX_VAASCEXPORTDLG_H__058D3ACC_6C06_418B_904B_D6600E637A80__INCLUDED_)
#define AFX_VAASCEXPORTDLG_H__058D3ACC_6C06_418B_904B_D6600E637A80__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VAASCExportDlg.h : header file
//
//{{AFX_INCLUDES()
#include "progressbar.h"
//}}AFX_INCLUDES

// (j.jones 2007-07-02 17:23) - PLID 25493 - created

/////////////////////////////////////////////////////////////////////////////
// CVAASCExportDlg dialog
// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker

class CVAASCExportDlg : public CNxDialog
{
// Construction
public:
	CVAASCExportDlg(CWnd* pParent);   // standard constructor

	NXDATALIST2Lib::_DNxDataListPtr m_POSCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_POS2Combo;

// Dialog Data
	//{{AFX_DATA(CVAASCExportDlg)
	enum { IDD = IDD_VA_ASC_EXPORT_DLG };
	CNxIconButton	m_btnExport;
	CNxIconButton	m_btnExportList;
	CNxIconButton	m_btnClose;
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	CProgressCtrl	m_progressbar;
	CNxEdit	m_nxeditEditHospitalIdentifier;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVAASCExportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CString m_strAllowedCPTCodes;

	CString StripNonNum(CString strIn);
	CString OutputString(CString strIn, long nLength, BOOL bIsNumeric = FALSE);

	void ExportData();

	CStdioFile m_fExport;

	void EnableButtons(BOOL bEnableAll, BOOL bEnableCancel);

	// Generated message map functions
	//{{AFX_MSG(CVAASCExportDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnExport();
	afx_msg void OnBtnExportList();
	afx_msg void OnSelChosenVaAscPosCombo(LPDISPATCH lpRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VAASCEXPORTDLG_H__058D3ACC_6C06_418B_904B_D6600E637A80__INCLUDED_)
