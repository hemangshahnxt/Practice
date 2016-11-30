#pragma once
#include "PracticeRc.h"

// (j.gruber 2013-11-05 12:46) - PLID 59323 - created for
// CSummaryofCareExportDlg dialog

struct CCDAExport
{
	CString strPatientName;
	std::vector<int> emnIDs;
};

class CSummaryofCareExportDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CSummaryofCareExportDlg)


public:
	CSummaryofCareExportDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSummaryofCareExportDlg();

// Dialog Data
	enum { IDD = IDD_SUMMARY_OF_CARE_EXPORT_DLG };

protected:

	CNxIconButton m_btnExport;
	CNxIconButton m_btnOK;


	NXDATALIST2Lib::_DNxDataListPtr m_pExportList;
	void GetCheckedEMNs(NXDATALIST2Lib::IRowSettingsPtr pPatientRow, CCDAExport &ccdaExport);
	BOOL FillExportList();
	bool DetermineSavePath(CString &strExportPath);

	std::map<int, CCDAExport> m_mapExports;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedExportSummaries();
	DECLARE_EVENTSINK_MAP()
	void EditingFinishedPatientsToExportList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
};
