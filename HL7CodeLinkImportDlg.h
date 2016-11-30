// (r.gonet 09/27/2011) - PLID 45719 - Added

#pragma once

#include "FinancialRc.h"
#include <NxPracticeSharedLib\CSVUtils.h>		// (d.lange 2016-01-11 10:54) - PLID 67829 - Moved to NxPracticeSharedLib

// CHL7CodeLinkImportDlg dialog

class CHL7CodeLinkImportDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CHL7CodeLinkImportDlg)

	enum EThirdPartyCodeColumn
	{
		etpccID = 0,
		etpccName,
	};

	enum EPracticeIDColumn
	{
		epicID = 0, // epic
		epicName,
	};

	CNxColor m_nxcColor;
	CNxStatic m_nxsFilePath;
	CNxEdit m_nxeImportFilePath;
	CNxIconButton m_btnBrowse;
	NxButton m_checkHasFieldNamesInFirstRow;
	CNxIconButton m_btnLoad;
	CNxStatic m_nxs3rdPartyCode;
	CNxStatic m_nxsPracticeID;
	NXDATALIST2Lib::_DNxDataListPtr m_p3rdPartyCodeCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pPracticeIDCombo;
	CNxIconButton m_btnImport;
	CNxIconButton m_btnCancel;

	// (r.gonet 09/27/2011) - PLID 45719 - the parsed recordset representing a CSV file
	boost::shared_ptr<CCSVRecordSet> m_pcsvRecordSet;
	// (r.gonet 09/27/2011) - PLID 45719 - the column the user selected to contain the third party codes
	long m_n3rdPartyCodeColumn;
	// (r.gonet 09/27/2011) - PLID 45719 - the column the user selected to contain the practice IDs
	long m_nPracticeIDColumn;

public:
	CHL7CodeLinkImportDlg(CWnd* pParent);   // standard constructor
	virtual ~CHL7CodeLinkImportDlg();

// Dialog Data
	enum { IDD = IDD_HL7_CODE_LINK_IMPORT_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedHl7CodeImportBrowseBtn();
	afx_msg void OnBnClickedHl7CodeImportLoadBtn();
	DECLARE_EVENTSINK_MAP()
	void SelChosenHl73rdPartyIdCombo(LPDISPATCH lpRow);
	void SelChangingHl73rdPartyIdCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenHl7PracticeIdCombo(LPDISPATCH lpRow);
	void SelChangingHl7PracticeIdCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg void OnBnClickedImport();
	virtual BOOL OnInitDialog();

public:
	long GetThirdPartyCodeColumn();
	long GetPracticeIDColumn();
	boost::shared_ptr<CCSVRecordSet> GetCSVRecordSet();
};
