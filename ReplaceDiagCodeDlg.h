#pragma once

// (j.jones 2014-12-22 14:06) - PLID 64489 - created

#include "PatientsRc.h"
#include "DiagCodeInfoFwd.h"

// CReplaceDiagCodeDlg dialog

class CReplaceDiagCodeDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CReplaceDiagCodeDlg)

public:
	CReplaceDiagCodeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CReplaceDiagCodeDlg();

	virtual int DoModal(long nBkgColor, DiagCodeInfoPtr oldDiagCode);

	DiagCodeInfoPtr m_newDiagCode;

// Dialog Data
	enum { IDD = IDD_REPLACE_DIAG_CODE_DLG };

protected:

	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxColor		m_bkg;	
	NXDATALIST2Lib::_DNxDataListPtr	m_DiagSearch;
	NXDATALIST2Lib::_DNxDataListPtr	m_OldCodeList;
	NXDATALIST2Lib::_DNxDataListPtr	m_NewCodeList;
	
	long m_nBkgColor;
	DiagCodeInfoPtr m_oldDiagCode;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	virtual BOOL OnInitDialog();	
	void OnSelChosenDiagSearch(LPDISPATCH lpRow);
	afx_msg void OnOK();
};
