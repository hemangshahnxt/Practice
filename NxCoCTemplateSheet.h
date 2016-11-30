//DRT 10/28/2008 - PLID 31789 - Created for NxCoC Importer
#pragma once


// CNxCoCTemplateSheet dialog
#include "NxCoCImportWizardSheet.h"

class CNxCoCTemplateSheet : public CNxCoCWizardSheet
{
	DECLARE_DYNAMIC(CNxCoCTemplateSheet)

public:
	CNxCoCTemplateSheet(CNxCoCWizardMasterDlg* pParent = NULL);   // standard constructor
	virtual ~CNxCoCTemplateSheet();

// Dialog Data
	enum { IDD = IDD_NXCOC_TEMPLATE_SHEET };

protected:
	//members
	NXDATALIST2Lib::_DNxDataListPtr m_pTemplateList;
	NXDATALIST2Lib::_DNxDataListPtr m_pDuplicateList;

	//General functionality
	void LoadTemplatesToDatalist();
	void SelectSingleColumn(NXDATALIST2Lib::IRowSettingsPtr pRow, short nCol);

	//wizard functionality
	void Load();
	BOOL Validate();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void EditingFinishedNxcocDuplicateTemplateList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void OnBnClickedNxcocSelAllTmpOverwrite();
	afx_msg void OnBnClickedNxcocSelAllTmpRenameNew();
	afx_msg void OnBnClickedNxcocSelAllTmpRenameExisting();
	afx_msg void OnBnClickedNxcocSelAllTmpSkip();
};
