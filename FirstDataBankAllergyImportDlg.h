#pragma once

#include <NxUILib/NxStaticIcon.h>
#include "PatientsRc.h"

// (b.savon 2012-07-24 10:50) - PLID 51734 - Created
// CFirstDataBankAllergyImportDlg dialog
class CFirstDataBankAllergyImportDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CFirstDataBankAllergyImportDlg)
private:
	CNxIconButton m_btnImport;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnSearch;

	NXDATALIST2Lib::_DNxDataListPtr m_pSearchList;

	CNxColor m_nxcBack;

	BOOL m_bShouldRequery;

	void LoadFirstDataBankAllergyMasterList();

public:
	CFirstDataBankAllergyImportDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFirstDataBankAllergyImportDlg();
	virtual BOOL OnInitDialog();
	
	inline BOOL ShouldRequery(){ return m_bShouldRequery;	}

// Dialog Data
	enum { IDD = IDD_FDB_ALLERGY_IMPORT_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedImport();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedFdbAllergySearch();
	afx_msg void OnOK();
};
