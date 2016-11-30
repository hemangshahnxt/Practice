// (r.gonet 09/27/2011) - PLID 45717 - Added

#pragma once

#include "FinancialRc.h"

// CLabCorpInsCoExportDlg dialog

class CLabCorpInsCoExportDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CLabCorpInsCoExportDlg)

	enum ELabCorpGroupColumns
	{
		elcgcID = 0,
		elcgcName,
	};

	CNxColor m_nxcColor;
	CNxStatic m_nxsHeader;
	NXDATALIST2Lib::_DNxDataListPtr m_pGroups;
	CNxStatic m_nxsExportFilter;
	NxButton m_radioAllCompanies;
	NxButton m_radioCompaniesWithoutCode;
	CNxIconButton m_nxbExport;
	CNxIconButton m_nxbCancel;

	CString m_strFilePath;
	CString m_strFolderPath;

	CString EscapeCSV(CString strValue);

public:
	CLabCorpInsCoExportDlg(CWnd* pParent);   // standard constructor
	virtual ~CLabCorpInsCoExportDlg();

// Dialog Data
	enum { IDD = IDD_LABCORP_INSCO_EXPORT_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	DECLARE_EVENTSINK_MAP()
	void SelChosenLabcorpExportHl7groupsList(LPDISPATCH lpRow);
	void SelChangingLabcorpExportHl7groupsList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);

public:
	CString GetExportFolderPath();
	CString GetExportFilePath();
};
