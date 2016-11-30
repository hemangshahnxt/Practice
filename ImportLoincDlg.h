#pragma once
#include "AdministratorRc.h"
#include <NxPracticeSharedLib\CSVUtils.h>	// (d.lange 2016-01-11 10:54) - PLID 67829 - Moved to NxPracticeSharedLib

// CImportLoincDlg dialog
// (s.tullis 2015-11-13 16:41) - PLID 54285 - Created
class CImportLoincDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CImportLoincDlg)

	boost::shared_ptr<CCSVRecordSet> m_pcsvRecordSet;

public:
	CImportLoincDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CImportLoincDlg();
	virtual BOOL OnInitDialog();
// Dialog Data

	enum { IDD = IDD_IMPORT_LOINC };


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	NXDATALIST2Lib::_DNxDataListPtr m_dlLOINCImportList;
	CNxIconButton m_btnImportCSV;
	CNxIconButton m_btnSave;
	CNxIconButton m_btnClose;
	CNxIconButton m_btnSelectAll;
	CNxIconButton m_btnDeselectAll;
	CNxIconButton m_btnDelete;
	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedDeleteCodes();
	afx_msg void OnBnClickedSelectAll();
	afx_msg void OnBnClickedDeselectAll();
	afx_msg void OnBnClickedImportLoincFile();
	afx_msg void OnBnClickedCloseCodes();
	void RemoveCheckedCodes();
};
