#pragma once

#include <NxUILib/NxStaticIcon.h>
#include "Patientsrc.h"

// CFirstDataBankImportDlg dialog
// (j.gruber 2010-06-21 10:37) - PLID 39049 - created

class CFirstDataBankImportDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CFirstDataBankImportDlg)

private:
	CNxIconButton m_btnImport;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnSearch;

	CNxColor m_nxcBackground;

	NXDATALIST2Lib::_DNxDataListPtr m_pSearchList;

	void LoadFDBList();
	void ImportMedications(CMap<long, long, CString, LPCTSTR> *pmapFDBs);

public:
	CFirstDataBankImportDlg(CWnd* pParent);   // standard constructor
	virtual ~CFirstDataBankImportDlg();

// Dialog Data
	enum { IDD = IDD_FDB_IMPORT_DLG };

private:


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	

public:
	virtual BOOL OnInitDialog();	
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedFdbSearch();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedChkShowNonActive();
};
