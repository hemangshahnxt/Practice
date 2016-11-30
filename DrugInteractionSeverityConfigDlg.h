#pragma once

// (j.jones 2013-05-10 14:05) - PLID 55955 - created

// CDrugInteractionSeverityConfigDlg dialog

#include "PracticeRc.h"

struct DrugInteractionDisplayFields;

class CDrugInteractionSeverityConfigDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CDrugInteractionSeverityConfigDlg)

public:
	CDrugInteractionSeverityConfigDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDrugInteractionSeverityConfigDlg();

// Dialog Data
	enum { IDD = IDD_DRUG_INTERACTION_SEVERITY_CONFIG_DLG };

protected:

	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxColor m_bkg;
	NXDATALIST2Lib::_DNxDataListPtr m_SeverityTree;

	void FillSeverityRow(NXDATALIST2Lib::IRowSettingsPtr pChildRow, DrugInteractionDisplayFields eResult, bool bChecked);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnOK();
};
