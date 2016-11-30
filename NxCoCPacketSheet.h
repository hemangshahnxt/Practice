//DRT 10/28/2008 - PLID 31789 - Created for NxCoC Importer
#pragma once


// CNxCoCPacketSheet dialog
#include "NxCoCImportWizardSheet.h"

class CNxCoCPacketSheet : public CNxCoCWizardSheet
{
	DECLARE_DYNAMIC(CNxCoCPacketSheet)

public:
	CNxCoCPacketSheet(CNxCoCWizardMasterDlg* pParent = NULL);   // standard constructor
	virtual ~CNxCoCPacketSheet();

// Dialog Data
	enum { IDD = IDD_NXCOC_PACKET_SHEET };

protected:
	//Members
	NXDATALIST2Lib::_DNxDataListPtr m_pPacketList;
	NXDATALIST2Lib::_DNxDataListPtr m_pDuplicateList;

	//General functionality
	void LoadPacketsToDatalist();
	void SelectSingleColumn(NXDATALIST2Lib::IRowSettingsPtr pRow, short nCol);

	//Wizard functionality
	void Load();
	BOOL Validate();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg BOOL OnInitDialog();


	DECLARE_MESSAGE_MAP()

public:
	DECLARE_EVENTSINK_MAP()
	void EditingFinishedNxcocDuplicatePacketList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void OnBnClickedNxcocSelAllPacketOverwrite();
	afx_msg void OnBnClickedNxcocSelAllPacketRenameNew();
	afx_msg void OnBnClickedNxcocSelAllPacketRenameExisting();
	afx_msg void OnBnClickedNxcocSelAllPacketSkip();
};
